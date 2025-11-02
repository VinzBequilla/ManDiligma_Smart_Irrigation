#include <DHT.h>
#include <math.h>

// ---------------- Sensors & Relay ----------------
#define SOIL_PIN A0
#define RELAY_PIN 3
#define DHTPIN 2
#define DHTTYPE DHT22

// Calibrate these values with your sensor
#define AIR_VALUE 850    // raw sensor value in dry air
#define WATER_VALUE 400  // raw sensor value in water

DHT dht(DHTPIN, DHTTYPE);

// ---------------- Dataset ----------------
struct DataPoint {
  int soil;   // Soil moisture %
  int temp;   // Air temp °C
  int hum;    // Air humidity %
  int label;  // 0 = OFF, 1 = ON
};

DataPoint trainingSet[] = {
  {74, 42, 47, 1}, {75, 40, 30, 1}, {26, 32, 24, 1}, {12, 31, 22, 1}, {79, 57, 49, 1},
  {30, 56, 6, 1},  {31, 54, 42, 1}, {11, 22, 96, 1}, {27, 66, 56, 1}, {27, 45, 77, 1},
  {85, 53, 1, 0},  {73, 38, 12, 0}, {61, 54, 92, 0}, {32, 29, 79, 0}, {80, 41, 96, 0},
  {67, 43, 8, 0},  {54, 61, 61, 0}, {66, 58, 19, 0}, {53, 30, 96, 0}, {63, 60, 6, 0}
};

int trainingSize = sizeof(trainingSet) / sizeof(trainingSet[0]);

// ---------------- KNN Functions ----------------
float distance(int soil, int temp, int hum, DataPoint d) {
  return sqrt(pow((soil - d.soil), 2) +
              pow((temp - d.temp), 2) +
              pow((hum - d.hum), 2));
}

int predict(int soil, int temp, int hum) {
  int k = 3;
  float distances[trainingSize];
  int labels[trainingSize];

  for (int i = 0; i < trainingSize; i++) {
    distances[i] = distance(soil, temp, hum, trainingSet[i]);
    labels[i] = trainingSet[i].label;
  }

  // Bubble sort (distances + labels)
  for (int i = 0; i < trainingSize - 1; i++) {
    for (int j = i + 1; j < trainingSize; j++) {
      if (distances[j] < distances[i]) {
        float tmpD = distances[i]; distances[i] = distances[j]; distances[j] = tmpD;
        int tmpL = labels[i]; labels[i] = labels[j]; labels[j] = tmpL;
      }
    }
  }

  int countOn = 0, countOff = 0;
  for (int i = 0; i < k; i++) {
    if (labels[i] == 1) countOn++;
    else countOff++;
  }

  return (countOn > countOff) ? 1 : 0;
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

// ---------------- Main Loop ----------------
void loop() {
  // --- Soil Sensor Reading ---
  int soilRaw = analogRead(SOIL_PIN);
  int soilMoisture = map(soilRaw, AIR_VALUE, WATER_VALUE, 0, 100);
  soilMoisture = constrain(soilMoisture, 0, 100);

  // --- DHT Sensor Reading ---
  int airTemp = (int)dht.readTemperature();
  int airHum = (int)dht.readHumidity();

  // --- Prediction ---
  int decision = predict(soilMoisture, airTemp, airHum);

  // --- Debug Info ---
  Serial.print("RawSoil: "); Serial.print(soilRaw);
  Serial.print(" | SoilMoisture(%): "); Serial.print(soilMoisture);
  Serial.print(" | AirTemp: "); Serial.print(airTemp);
  Serial.print(" | AirHum: "); Serial.print(airHum);
  Serial.print(" | KNN: "); Serial.println(decision ? "ON" : "OFF");

  // --- Pump Control ---
  // Pump runs only if soil is below threshold and KNN suggests ON
  if (decision == 1 && soilMoisture < 30) {
    Serial.println(">>> Pump ON for 6s <<<");
    digitalWrite(RELAY_PIN, HIGH);
    delay(6000); // Pump ON
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Pump OFF");
  }

  delay(5000); // Wait 5s before next reading
}
