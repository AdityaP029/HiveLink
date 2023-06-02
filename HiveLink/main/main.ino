#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
// Depth Sensor pins
#define sensorPower D7
#define sensorPin A0

// Value for storing sugar quantity level
int val = 0;

void setup() {
  // Set D7 as an OUTPUT
  pinMode(sensorPower, OUTPUT);
  // Set to LOW so no power flows through the sensor
  digitalWrite(sensorPower, LOW);
  Serial.begin(9600);
  dht.begin();
  delay(1500);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int level = readSensor();
  Serial.println("Humidity: ");
  Serial.println(h);
  Serial.println("Temperature: ");
  Serial.println(t);
  Serial.print("Water level: ");
  Serial.println(level);
  delay(2000);
}

int readSensor() {
  digitalWrite(sensorPower, HIGH);	    // Turn the sensor ON
  delay(10);							              // wait 10 milliseconds
  float val = analogRead(sensorPin);		// Read the analog value form sensor
  digitalWrite(sensorPower, LOW);		    // Turn the sensor OFF
  return val;							              // send current reading
}
