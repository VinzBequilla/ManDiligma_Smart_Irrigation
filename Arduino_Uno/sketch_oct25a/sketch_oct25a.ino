#include <DHT.h>
#include <Arduino_KNN.h>

// ===================== Pin definitions =====================
#define SOIL_PIN A0
#define PUMP_PIN 3
#define DHT_PIN 2
#define DHT_TYPE DHT22

// ===================== Relay configuration =====================
// Change to true if your relay module activates when pin is LOW
#define RELAY_ACTIVE_LOW true     

// ===================== Pump cooldown =====================
const unsigned long PUMP_COOLDOWN = 5UL * 60 * 1000; // 5 minutes

// ===================== k-NN parameters =====================
const int NUM_FEATURES = 3;
const int K = 3;
const int NUM_SAMPLES = 100;

// ===================== Dataset =====================
float dataset[NUM_SAMPLES][NUM_FEATURES] = {
  {0.2360, 0.3805, 0.6063}, {0.0562, 0.1829, 1.0000},
  {0.3034, 0.3797, 0.6061}, {0.3034, 0.3801, 0.6064},
  {0.2247, 0.3810, 0.6066}, {0.6742, 0.3801, 0.6082},
  {0.7865, 0.3799, 0.6087}, {0.8315, 0.2236, 0.9693},
  {0.9663, 0.7525, 0.0345}, {0.3596, 0.3800, 0.6065},
  {0.4494, 0.3792, 0.6065}, {0.9888, 0.3782, 0.6086},
  {0.4157, 0.3800, 0.6069}, {0.5955, 0.3799, 0.6077},
  {0.0899, 0.3802, 0.6053}, {0.0449, 0.3812, 0.6058},
  {0.5955, 0.3798, 0.6076}, {0.0674, 0.1829, 1.0000},
  {0.0225, 0.3814, 0.6058}, {0.9551, 0.3782, 0.6084},
  {0.3483, 0.3503, 0.7032}, {0.5618, 0.3796, 0.6073},
  {0.3820, 0.2650, 0.9349}, {0.5506, 0.3792, 0.6070},
  {0.7079, 0.6846, 0.1279}, {0.1348, 0.3805, 0.6058},
  {0.7978, 0.3798, 0.6087}, {1.0000, 0.3794, 0.6094},
  {0.6854, 0.3791, 0.6076}, {0.5056, 0.3798, 0.6072},
  {0.5506, 0.3799, 0.6075}, {0.7753, 0.3797, 0.6085},
  {0.7978, 0.3797, 0.6086}, {0.5730, 0.3802, 0.6078},
  {0.8876, 0.3792, 0.6087}, {1.0000, 0.2915, 0.2413},
  {0.8202, 0.3786, 0.6080}, {0.6404, 0.2347, 0.8589},
  {0.0899, 0.3804, 0.6055}, {0.3708, 0.3800, 0.6066},
  {0.3258, 0.3796, 0.6062}, {0.5843, 0.2117, 0.8051},
  {0.8090, 0.3787, 0.6080}, {0.4270, 0.3794, 0.6066},
  {0.9551, 0.3788, 0.6088}, {0.7191, 0.3797, 0.6082},
  {0.7978, 0.4330, 0.6690}, {0.3933, 0.3803, 0.6069},
  {0.7191, 0.3788, 0.6076}, {0.1124, 0.3804, 0.6056},
  {0.6742, 0.3787, 0.6073}, {0.8090, 0.3791, 0.6082},
  {0.2135, 0.3804, 0.6061}, {0.8652, 0.3797, 0.6089},
  {0.9888, 0.3780, 0.6085}, {0.6854, 0.3798, 0.6081},
  {0.2472, 0.3806, 0.6064}, {0.3820, 0.3809, 0.6073},
  {0.4831, 0.3797, 0.6070}, {0.9326, 0.3785, 0.6085},
  {0.1461, 0.3812, 0.6063}, {0.1348, 0.3811, 0.6062},
  {0.2697, 0.3807, 0.6066}, {0.8764, 0.3793, 0.6087},
  {0.1910, 0.2003, 1.0000}, {0.5618, 0.3796, 0.6074},
  {0.5618, 0.3798, 0.6075}, {0.5955, 0.3795, 0.6074},
  {0.5169, 0.3798, 0.6072}, {0.9775, 0.3781, 0.6085},
  {0.6067, 0.3550, 0.2283}, {0.3596, 0.3802, 0.6067},
  {0.8652, 0.3790, 0.6085}, {0.0337, 0.7368, 0.0380},
  {0.4045, 0.3795, 0.6065}, {0.6180, 0.3799, 0.6078},
  {0.0674, 0.3803, 0.6053}, {0.5169, 0.3801, 0.6075},
  {0.2135, 0.3200, 0.7043}, {0.6629, 0.3801, 0.6081},
  {0.1573, 0.3811, 0.6063}, {1.0000, 0.3786, 0.6089},
  {0.2247, 0.2234, 1.0000}, {0.3708, 0.2292, 0.9798},
  {0.2247, 0.3798, 0.6058}, {0.0000, 0.2135, 1.0000},
  {0.9101, 0.3795, 0.6090}, {0.1573, 0.3809, 0.6061},
  {0.4831, 0.3805, 0.6075}, {0.2921, 0.3806, 0.6067},
  {0.9438, 0.3789, 0.6088}, {0.7978, 0.3784, 0.6077},
  {0.5281, 0.3796, 0.6072}, {0.5618, 0.3798, 0.6075},
  {0.8315, 0.3792, 0.6084}, {0.7865, 0.3799, 0.6087},
  {0.1798, 0.3805, 0.6060}, {0.4270, 0.3801, 0.6070},
  {0.6742, 0.3791, 0.6076}, {0.4382, 0.3801, 0.6070}
};

int labels[NUM_SAMPLES] = {
  1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0,
  1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
  1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
  0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0,
  0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0
};

const float mins[NUM_FEATURES] = {1.00, 11.22, 0.59};
const float maxs[NUM_FEATURES] = {90.00, 45.56, 96.00};

// ===================== Global variables =====================
DHT dht(DHT_PIN, DHT_TYPE);
KNNClassifier knn(NUM_FEATURES);
unsigned long lastPumpTime = 0;

// ===================== SETUP =====================
void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW); // Pump OFF

  Serial.begin(9600);
  dht.begin();

  for (int i = 0; i < NUM_SAMPLES; i++) {
    knn.addExample(dataset[i], labels[i]);
  }

  Serial.println("System Initialized");
}

// ===================== LOOP =====================
void loop() {
  float moisture, temp, hum;
  if (!readSensors(&moisture, &temp, &hum)) {
    Serial.println("Sensor read failed, retrying...");
    delay(3000);
    return;
  }

  float new_point[NUM_FEATURES];
  normalizeInputs(moisture, temp, hum, new_point);

  int prediction = knn.classify(new_point, K);
  Serial.print("Predicted Status: ");
  Serial.println(prediction ? "ON" : "OFF");

  controlPump(prediction);
  Serial.println("-----------------------------------");
  delay(5000);
}

// ===================== SENSOR READ =====================
bool readSensors(float* moisture, float* temp, float* hum) {
  int raw_moisture = analogRead(SOIL_PIN);
  Serial.print("Raw moisture: "); Serial.println(raw_moisture);

  // adjust this depending on your sensor’s behavior
  *moisture = 100 - (raw_moisture / 1023.0 * 100);

  *temp = dht.readTemperature();
  *hum = dht.readHumidity();

  if (isnan(*temp) || isnan(*hum)) {
    Serial.println("DHT failed, using fallback values");
    *temp = 25.0;
    *hum = 60.0;
  }

  Serial.print("Moisture(%): "); Serial.print(*moisture);
  Serial.print(" | Temp(°C): "); Serial.print(*temp);
  Serial.print(" | Hum(%): "); Serial.println(*hum);

  return true;
}

// ===================== NORMALIZATION =====================
void normalizeInputs(float moisture, float temp, float hum, float* new_point) {
  float norm_moisture = (moisture - mins[0]) / (maxs[0] - mins[0]);
  float norm_temp = (temp - mins[1]) / (maxs[1] - mins[1]);
  float norm_hum = (hum - mins[2]) / (maxs[2] - mins[2]);

  new_point[0] = constrain(norm_moisture, 0, 1);
  new_point[1] = constrain(norm_temp, 0, 1);
  new_point[2] = constrain(norm_hum, 0, 1);

  Serial.print("Normalized: ");
  for (int i = 0; i < NUM_FEATURES; i++) {
    Serial.print(new_point[i], 4); Serial.print(" ");
  }
  Serial.println();
}

// ===================== PUMP CONTROL =====================
void controlPump(int prediction) {
  if (prediction == 1 && (millis() - lastPumpTime >= PUMP_COOLDOWN)) {
    Serial.println("Activating pump...");
    digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    delay(6000);
    digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    Serial.println("Pump OFF (after 6s)");
    lastPumpTime = millis();
  } else if (prediction == 0) {
    digitalWrite(PUMP_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  }
}
