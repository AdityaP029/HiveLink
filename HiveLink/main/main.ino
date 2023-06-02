#include "DHT.h"
#include <Servo.h>
#define LDR_PIN D8
#define SERVO_PIN D4
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define LDR_PIN D8
#define SERVO_PIN D4
// Depth Sensor pins
#define sensorPower D7
#define sensorPin A0
Servo servo;

// Value for storing sugar quantity level
int val = 0;

void setup() {
  servo.attach(SERVO_PIN);
  pinMode(LDR_PIN, INPUT);

  // Set D7 as an OUTPUT
  pinMode(sensorPower, OUTPUT);
  // Set to LOW so no power flows through the sensor
  digitalWrite(sensorPower, LOW);
  Serial.begin(9600);
  dht.begin();
  delay(1500);
}

void loop() {
  int ldrValue = digitalRead(LDR_PIN);
  // If the LDR sensor is off
  if (ldrValue == LOW) {
    // Rotate the servo to position 1
    servo.write(180);
    delay(500);
  }
  // If the LDR sensor is on
  else {
    // Rotate the servo to position 2
    servo.write(90);
    delay(500);
  }
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
  digitalWrite(sensorPower, HIGH);    // Turn the sensor ON
  delay(10);                          // wait 10 milliseconds
  float val = analogRead(sensorPin);  // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);     // Turn the sensor OFF
  return val;                         // send current reading
}
