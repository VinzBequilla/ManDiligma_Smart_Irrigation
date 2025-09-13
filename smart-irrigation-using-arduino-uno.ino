#include <DHT.h>

#define SOIL_PIN A0     // Soil moisture sensor analog pin
#define PUMP_PIN 3      // Pump relay digital pin
#define DHT_PIN 2       // DHT22 data pin for temperature/humidity
#define DHT_TYPE DHT22  // Sensor type

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);  // Pump off
  Serial.begin(9600);
  dht.begin();
  delay(2000);
  Serial.println("Arduino ready.");
}

void loop() {
  // Read sensors
  int raw_moisture = analogRead(SOIL_PIN);
  float moisture = 100 - (raw_moisture / 1023.0 * 100);  // Convert to dryness % (high = dry)
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.print(moisture);
  Serial.print(",");
  Serial.print(isnan(temp) ? 25.0 : temp);  // Default 25°C if error
  Serial.print(",");
  Serial.println(isnan(hum) ? 60.0 : hum);  // Default 60% if error

  // Listen for pump command from Python (e.g., "PUMP:15")
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("PUMP:")) {
      int runtime_s = command.substring(5).toInt();
      if (runtime_s > 0) {
        digitalWrite(PUMP_PIN, HIGH);  // Start pump
        delay(runtime_s * 1000);       // Run for exact seconds
        digitalWrite(PUMP_PIN, LOW);   // Stop pump
        Serial.println("PUMP_DONE");   // Acknowledge
      }
    }
  }

  delay(5000);  // Send data every 5 seconds
}