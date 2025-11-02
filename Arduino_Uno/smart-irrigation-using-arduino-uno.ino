/* Change these values based on your calibration values */
#include "DHT.h"
#define soilWet 500  // Define max value we consider soil 'wet'
#define soilDry 750  // Define min value we consider soil 'dry'

// Sensor pins
#define sensorPower 7
#define sensorPin A0

#define DHTPIN 2 
// Pump pin
#define pumpPin 3

#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
void setup() {
  pinMode(sensorPower, OUTPUT);
  pinMode(pumpPin, OUTPUT); // Configure pump pin as an output

  // Initially keep the sensor and pump OFF
  digitalWrite(sensorPower, LOW);
  digitalWrite(pumpPin, HIGH);

  Serial.begin(9600);
  dht.begin();
}

void loop() {

  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  
  //get the reading from the function below and print it
  int moisture = readSensor();
  Serial.print("Analog Output: ");
  Serial.println(moisture);

  // Determine status of our soil and control the pump
  if (moisture < soilWet) {
    Serial.println("Status: Soil is too wet");
    digitalWrite(pumpPin, HIGH); // Turn the pump OFF
  } else if (moisture >= soilWet && moisture < soilDry) {
    Serial.println("Status: Soil moisture is perfect");
    digitalWrite(pumpPin, HIGH); // Turn the pump OFF
  } else {
    Serial.println("Status: Soil is too dry - time to water!");
    digitalWrite(pumpPin, LOW); // Turn the pump ON
  }

  delay(1000); // Take a reading every second for testing
               // Normally you should take reading perhaps once or twice a day
  Serial.println();
}


// This function returns the analog soil moisture measurement
int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(300);                       // Allow power to settle
  int val = analogRead(sensorPin);  // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
  return val;                       // Return analog moisture value
}