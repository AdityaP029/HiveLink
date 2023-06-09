#include "thingProperties.h"
#include "DHT.h"
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#include <Servo.h>

#endif
#define DHTPIN D3
#define DHTTYPE DHT11
#define sensorPower D7
#define sensorPin A0
#define LDR_PIN D8
#define SERVO_PIN D4
DHT dht(DHTPIN, DHTTYPE);
Servo servo;

//pins:
const int HX711_dout = D2; //mcu > HX711 dout pin
const int HX711_sck = D1; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  servo.attach(SERVO_PIN);
  pinMode(LDR_PIN, INPUT);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  servo.attach(SERVO_PIN);
  pinMode(LDR_PIN, INPUT);

  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  dht.begin();
  LoadCell.begin();
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);
  float calibrationValue;
  calibrationValue = 105.35;
#if defined(ESP8266)|| defined(ESP32)
#endif
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}
void loop() {
  ArduinoCloud.update();
  // Your code here
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
  unsigned long te = 0;
  syrupQuantity = level;
  temperature = t;
  humidity = h;
  //weight measurement
  static boolean newDataReady = 0;
  const int serialPrintInterval = 2000; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > te + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Weight: ");
      Serial.println(i);
      weight = i;
      newDataReady = 0;
      te = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 'te') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}

int readSensor() {
  digitalWrite(sensorPower, HIGH);	    // Turn the sensor ON
  delay(10);							              // wait 10 milliseconds
  float val = analogRead(sensorPin);		// Read the analog value form sensor
  digitalWrite(sensorPower, LOW);		    // Turn the sensor OFF
  return val;							              // send current reading
}
