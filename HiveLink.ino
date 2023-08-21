#include "DHT.h"
#include <HX711_ADC.h>
// Additional includes based on platform
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#include <Servo.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#endif

// Define pin assignments for sensors and components
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define sensorPower D1
#define sensorPin A0
#define LDR_PIN D2
#define SERVO_PIN D5
Servo servo;

unsigned long previousSendMillis = 0;
unsigned long previousSerialMillis = 0;

const unsigned long sendInterval = 10000;  // 10 seconds in milliseconds
const unsigned long serialdelay = 4000;
// WiFi credentials
const char* ssid = "TP-Link_3018";
const char* password = "75285654";

// IFTTT and Google Script host URLs
const char* host = "maker.ifttt.com";
const char* host2 = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;

// Google Apps Script API Key
String GAS_ID = "AKfycbyE2lkfeReTAE8mgIgqP8RMjMHQkLO1EgE1PPgj9exYe9GO_Gi-rGXJLHVAGea4nPQLug";

// Load cell amplifier pins
const int HX711_dout = D6;  // MCU > HX711 dout pin
const int HX711_sck = D7;   // MCU > HX711 sck pin

// HX711 constructor
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// Calibration value EEPROM address
const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  // Initialize components and sensors
  servo.attach(SERVO_PIN);
  pinMode(LDR_PIN, INPUT);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);

  // Initialize serial communication
  Serial.begin(9600);

  // Initialize DHT sensor
  dht.begin();
  Serial.println("Email from Node Mcu");
  delay(100);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize secure client for HTTPS
  client.setInsecure();

  // Initialize load sensors
  LoadCell.begin();

  // Delay for serial monitor to open
  delay(1500);

  // Calibration value for load cell
  float calibrationValue = 10.35;

  // Stabilization time and tare for load cell
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1)
      ;
  } else {
    LoadCell.setCalFactor(calibrationValue);  // Set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  float hiveWeight = 0;
  // Read LDR sensor value and control servo
  int ldrValue = digitalRead(LDR_PIN);
  if (ldrValue == LOW) {
    servo.write(180);
  } else {
    servo.write(90);
  }
  // Read temperature, humidity, and syrup level
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int level = readSensor();
  //  unsigned long te = 0;

  // Temperature, humidity, and syrup level notifications
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  if (h >= 75) {
    // Construct and send IFTTT URL for humidity alert
    String url1 = "/trigger/Humidity_Alert/with/key/oa0dl1Qs8AOLd0Cm5kI1AiLEDoYNWxh4oNixF6_HSYu";
    Serial.print("Requesting URL: ");
    Serial.println(url1);
    client.print(String("GET ") + url1 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    delay(10000);
  }

  if (t >= 35) {
    // Construct and send IFTTT URL for temperature alert
    Serial.println("Temperature is raising!");
    String url2 = "/trigger/Temp_Alert/with/key/cYh4K8NGokabGpIyRhvi-K";
    Serial.print("Requesting URL: ");
    Serial.println(url2);
    client.print(String("GET ") + url2 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    delay(10000);
  }

  if (level <= 100) {
    // Construct and send IFTTT URL for low syrup level
    Serial.println("Lower Suppliment");
    String url3 = "/trigger/Low_supplement/with/key/oa0dl1Qs8AOLd0Cm5kI1AiLEDoYNWxh4oNixF6_HSYu";
    Serial.print("Requesting URL: ");
    Serial.println(url3);
    client.print(String("GET ") + url3 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    delay(10000);
  }


  // Weight measurement
  static boolean newDataReady = 0;
  //  const int serialPrintInterval = 2000;  // Increase value to slow down serial print activity

  // Check for new data/start next conversion
  if (LoadCell.update()) newDataReady = true;
  // Get smoothed value from the dataset
  if (newDataReady) {
    //   if (millis() > te + serialPrintInterval) {
    hiveWeight = LoadCell.getData();
    //      Serial.print("Weight: ");
    //      Serial.println(hiveWeight);
    newDataReady = 0;
    //      te = millis();
    //   }
  }

  // Tare operation through serial input
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 'te') LoadCell.tareNoDelay();
  }

  // Check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  // Print sensor data
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  String Syrup = "Syrup Quantity : " + String(level);
  String Weight = "Weight : " + String(hiveWeight);
  if (currentMillis - previousSerialMillis >= serialdelay) {
    previousSerialMillis = currentMillis;
    Serial.println(Temp);
    Serial.println(Humi);
    Serial.println(Syrup);
    Serial.println(Weight);
    Serial.println("-----------------------------------------");
  }

  if (currentMillis - previousSendMillis >= sendInterval) {
    previousSendMillis = currentMillis;  // Save the last time you sent data

    // Send sensor data to Google Apps Script
    sendData(t, h, level, hiveWeight);
  }
}

// Read analog sensor
int readSensor() {
  digitalWrite(sensorPower, HIGH);    // Turn the sensor ON
  delay(10);                          // Wait 10 milliseconds
  float val = analogRead(sensorPin);  // Read the analog value from the sensor
  digitalWrite(sensorPower, LOW);     // Turn the sensor OFF
  return val;                         // Return current reading
}

// Send data to Google Apps Script
void sendData(float tem, int hum, int syr, float wei) {
  Serial.println("==========");
  Serial.print("Connecting to ");
  Serial.println(host2);

  // Connect to Google Apps Script
  if (!client.connect(host2, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  // Prepare data strings
  String string_temperature = String(tem);
  String string_humidity = String(hum, DEC);
  String string_syrup = String(syr);
  String string_weight = String(wei);

  // Construct and send URL for Google Apps Script
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&syrup=" + string_syrup + "&weight=" + string_weight;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host2 + "\r\n" + "User-Agent: ESP8266\r\n" + "Connection: close\r\n\r\n");
  Serial.println("Request sent");

  // Wait for response and process
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  // Check success status in response
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Data sending failed");
  }
  Serial.print("Reply was : ");
  Serial.println(line);
  Serial.println("Closing connection");
  Serial.println("==========");
  Serial.println();
}
