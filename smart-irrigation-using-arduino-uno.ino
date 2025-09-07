// Smart Irrigation System (fixed)

int sensor_pin = A0;     // Soil moisture sensor pin
int pump_pin = 3;        // Relay/SSR control pin
int output_value;

void setup() {
  pinMode(pump_pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Reading from the Moisture sensor...");
  delay(2000);
}

void loop() {
  output_value = analogRead(sensor_pin);              // read raw value (0–1023)
  output_value = (100 - ((output_value / 1023.0) * 100));  // convert to %

    Serial.print("Moisture:");
  Serial.print(output_value);
  Serial.println("%");

  if (output_value<0)
  {
    delay(1000);
    digitalWrite(3, HIGH);
  }

  else
  {
    delay(1000);
    digitalWrite (3,LOW);  
  }

  delay (1000);
}
