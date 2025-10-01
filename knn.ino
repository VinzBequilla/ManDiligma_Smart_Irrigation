#include <DHT.h>
#include <math.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <avr/pgmspace.h> // For PROGMEM

// Wi-Fi credentials
#define WIFI_SSID "PLDTFIBRAR1"
#define WIFI_PASSWORD "Spiderman.416"

// Firebase configuration
#define DATABASE_URL "mandiligma-16e1c-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DATABASE_SECRET "FfG5XWSYCZ0LUQbF8NSuvuhFZY4tGHVlPU2Alw6x" // Replace with your Firebase database secret

// Pin definitions
#define SOIL_PIN A0
#define PUMP_PIN 3
#define DHT_PIN 2
#define DHT_TYPE DHT22

// Relay configuration (true for active-low, false for active-high)
#define RELAY_ACTIVE_LOW true

DHT dht(DHT_PIN, DHT_TYPE);
RTC_DS3231 rtc; // Real-time clock

// Wi-Fi and HTTP client
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, DATABASE_URL, 443); // Firebase uses HTTPS (port 443)

// k-NN parameters and dataset
const int NUM_SAMPLES = 200;
const int NUM_FEATURES = 4;
const int K = 3;
const float dataset[NUM_SAMPLES][NUM_FEATURES] PROGMEM = {
    {0.2360, 0.5364, 0.3805, 0.6063}, {0.5618, 0.0091, 0.3789, 0.6069}, /* ... your full dataset ... */
    // Add the remaining 198 samples here (omitted for brevity)
    {0.9775, 0.8182, 0.3792, 0.6092}
};
const int labels[NUM_SAMPLES] PROGMEM = {
    1, 1, 1, 1, 0, 1, 1, 0, 0, 0, /* ... your full labels ... */
    // Add the remaining 190 labels here (omitted for brevity)
    1
};
const float mins[NUM_FEATURES] = {1.0, 0.0, 11.22, 0.59};
const float maxs[NUM_FEATURES] = {90.0, 110.0, 45.56, 96.0};

// Timing constants
const unsigned long WATERING_COOLDOWN = 24UL * 60 * 60 * 1000; // 24 hours
const unsigned long UPLOAD_INTERVAL = 5UL * 60 * 60 * 1000; // 5 hours
unsigned long lastWateringTime = 0;
unsigned long sendDataPrevMillis = 0;
bool pump_activated = false;
static unsigned long cycleStart = 0;
const unsigned long CYCLE_LENGTH = 110UL * 60 * 1000; // 110 minutes

void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW); // Ensure pump is OFF
  Serial.begin(9600);
  dht.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC failed, using uptime-based timestamp");
    // Continue without RTC, using millis() for timestamps
  } else if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Connect to Wi-Fi
  connectWiFi();
  Serial.println("Arduino ready!");
}

void loop() {
  // Ensure pump is OFF unless explicitly activated
  digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);

  // Read sensors
  int raw_moisture = analogRead(SOIL_PIN);
  if (raw_moisture < 0 || raw_moisture > 1023) {
    Serial.println("Invalid moisture reading, skipping.");
    delay(5000);
    return;
  }
  float moisture = 100 - (raw_moisture / 1023.0 * 100); // Convert to dryness %
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT sensor error, using defaults.");
    temp = 25.0;
    hum = 60.0;
  }

  // Validate readings
  if (moisture < 0 || moisture > 100 || temp < -40 || temp > 80 || hum < 0 || hum > 100) {
    Serial.println("Sensor error, skipping.");
    delay(5000);
    return;
  }

  // Calculate time_value (overflow-safe)
  if (millis() - cycleStart >= CYCLE_LENGTH) {
    cycleStart += CYCLE_LENGTH;
  }
  float time_value = (millis() - cycleStart) / 60000.0; // Minutes since cycle start

  // Print data
  Serial.print("Moisture:"); Serial.print(moisture);
  Serial.print(",Temp:"); Serial.print(temp);
  Serial.print(",Humidity:"); Serial.print(hum);
  Serial.print(",Time:"); Serial.println(time_value);

  // Normalize for k-NN
  float new_point[NUM_FEATURES];
  new_point[0] = constrain((moisture - mins[0]) / (maxs[0] - mins[0]), 0, 1);
  new_point[1] = constrain((time_value - mins[1]) / (maxs[1] - mins[1]), 0, 1);
  new_point[2] = constrain((temp - mins[2]) / (maxs[2] - mins[2]), 0, 1);
  new_point[3] = constrain((hum - mins[3]) / (maxs[3] - mins[3]), 0, 1);

  // Debug normalized inputs
  Serial.print("Normalized inputs: ");
  for (int i = 0; i < NUM_FEATURES; i++) {
    Serial.print(new_point[i], 4); Serial.print(" ");
  }
  Serial.println();

  // Run k-NN
  int prediction = knn_predict(new_point);
  Serial.print("Predicted Status: ");
  Serial.println(prediction ? "ON" : "OFF");

  // Pump control (non-blocking)
  if (prediction == 1 && (millis() - lastWateringTime >= WATERING_COOLDOWN)) {
    Serial.println("Activating pump...");
    digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    unsigned long pumpStart = millis();
    while (millis() - pumpStart < 6000) {
      if (WiFi.status() != WL_CONNECTED) connectWiFi();
    }
    digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    Serial.println("Pump state: OFF (after 6s)");
    lastWateringTime = millis();
    pump_activated = true;
  }

  // Upload sensor data every 5 hours
  if (millis() - sendDataPrevMillis > UPLOAD_INTERVAL) {
    sendDataPrevMillis = millis();
    String timestamp = String(millis());
    String json = "{\"moisture\":" + String(moisture, 1) +
                  ",\"temp\":" + String(temp, 1) +
                  ",\"humidity\":" + String(hum, 1) +
                  ",\"time\":" + String(time_value, 1) +
                  ",\"timestamp\":\"" + getTimestamp() + "\"}";

    String path = "/mungbean/sensors/" + timestamp + ".json?auth=" + String(DATABASE_SECRET);
    if (uploadData(path, json)) {
      Serial.println("Sensor data uploaded!");
    } else {
      Serial.println("Sensor upload failed.");
    }
  }

  // Immediate pump log upload
  if (pump_activated) {
    String timestamp = String(millis());
    String pump_json = "{\"pump_activated\":true,\"duration\":6,\"timestamp\":\"" + getTimestamp() + "\"}";
    String pump_path = "/mungbean/pump_log/" + timestamp + ".json?auth=" + String(DATABASE_SECRET);
    if (uploadData(pump_path, pump_json)) {
      Serial.println("Pump log uploaded!");
    } else {
      Serial.println("Pump log failed.");
    }
    pump_activated = false;
  }

  // Serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("PUMP:")) {
      int runtime_s = command.substring(5).toInt();
      if (runtime_s > 0 && runtime_s <= 10) {
        Serial.println("Manual pump activation...");
        digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
        unsigned long pumpStart = millis();
        while (millis() - pumpStart < runtime_s * 1000) {
          if (WiFi.status() != WL_CONNECTED) connectWiFi();
        }
        digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
        Serial.println("Pump state: OFF (manual)");
        pump_activated = true;
      } else {
        Serial.println("Invalid pump duration (1-10s allowed)");
      }
    }
  }

  delay(5000); // Check every 5 seconds
}

// Wi-Fi connection handler
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.disconnect();
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

// Upload data to Firebase with retry logic
bool uploadData(String path, String json) {
  int maxRetries = 3;
  int retryCount = 0;
  while (retryCount < maxRetries) {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
    client.beginRequest();
    client.post(path);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", json.length());
    client.beginBody();
    client.print(json);
    client.endRequest();

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    client.stop(); // Release resources
    if (statusCode == 200) {
      return true;
    } else {
      Serial.print("Upload failed (attempt "); Serial.print(retryCount + 1);
      Serial.print("): "); Serial.println(response);
      retryCount++;
      delay(1000 * retryCount); // Exponential backoff
    }
  }
  return false;
}

// Get timestamp (RTC or uptime-based)
String getTimestamp() {
  if (rtc.begin()) {
    DateTime now = rtc.now();
    char buf[] = "YYYY-MM-DDThh:mm:ssZ";
    return String(now.toString(buf));
  } else {
    return "1970-01-01T" + String(millis() / 1000) + "Z"; // Uptime-based
  }
}

// Optimized k-NN prediction
int knn_predict(float* new_point) {
  struct Neighbor {
    float distance;
    int index;
  };
  Neighbor neighbors[K];
  for (int i = 0; i < K; i++) {
    neighbors[i].distance = 1e9;
    neighbors[i].index = -1;
  }

  for (int i = 0; i < NUM_SAMPLES; i++) {
    float dist = 0.0;
    for (int j = 0; j < NUM_FEATURES; j++) {
      float value = pgm_read_float(&dataset[i][j]);
      dist += pow(new_point[j] - value, 2);
    }
    dist = sqrt(dist);

    for (int j = 0; j < K; j++) {
      if (dist < neighbors[j].distance) {
        for (int k = K - 1; k > j; k--) {
          neighbors[k] = neighbors[k - 1];
        }
        neighbors[j].distance = dist;
        neighbors[j].index = i;
        break;
      }
    }
  }

  int on_count = 0;
  for (int i = 0; i < K; i++) {
    if (neighbors[i].index != -1) {
      int label = pgm_read_word(&labels[neighbors[i].index]);
      if (label == 1) on_count++;
    }
  }
  return (on_count > K / 2) ? 1 : 0;
}