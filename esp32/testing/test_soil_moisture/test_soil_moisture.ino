// Define calibration values - you'll need to adjust these based on your sensor readings
const int dryValue = 4095;   // Value when sensor is dry (in air)
const int wetValue = 1800;   // Value when sensor is fully submerged in water

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(4);
  
  // Map the sensor value from dry-wet range to 0-100% range
  // Note: We use 0% for wet and 100% for dry since higher values typically mean drier soil
  int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);
  
  // Constrain the value to 0-100% to handle out-of-range readings
  moisturePercent = constrain(moisturePercent, 0, 100);

  // Alternative mapping if your sensor works inversely (higher values = more moisture):
  // int moisturePercent = map(sensorValue, wetValue, dryValue, 100, 0);
  // moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");
  
  delay(500);
}