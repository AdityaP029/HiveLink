#include <ESP8266WiFi.h>
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11
#define sensorPower D7
#define sensorPin A0
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Galaxy F41";
const char* password = "@bcd1234";
const char* host = "maker.ifttt.com";

void setup() {
  Serial.begin(115200);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  dht.begin();
  Serial.println("Email from Node Mcu");
  delay(100);
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
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int level = readSensor();
  Serial.println("Humidity: ");
  Serial.println(h);
  Serial.println("Temperature: ");
  Serial.println(t);
  Serial.println("Syrup Quantity: ");
  Serial.println(level);
  delay(3000);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  if (h >= 75) {
    String url1 = "/trigger/Humidity_Alert/with/key/oa0dl1Qs8AOLd0Cm5kI1AiLEDoYNWxh4oNixF6_HSYu";
    Serial.print("Requesting URL: ");
    Serial.println(url1);
    client.print(String("GET ") + url1 + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(20000);
  }
  else if (t >= 35) {
      Serial.println("Temperature is raising!");
      String url2 = "/trigger/Temp_Alert/with/key/cYh4K8NGokabGpIyRhvi-K";
      Serial.print("Requesting URL: ");
      Serial.println(url2);
      client.print(String("GET ") + url2 + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
    delay(20000);
    }
  else if (level <= 100) {
    Serial.println("lower Suppliment");
    String url3 = "/trigger/Low_supplement/with/key/oa0dl1Qs8AOLd0Cm5kI1AiLEDoYNWxh4oNixF6_HSYu";
    Serial.print("Requesting URL: ");
    Serial.println(url3);
    client.print(String("GET ") + url3 + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(20000);
  }
  delay(1000);
}

int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);        // wait 10 milliseconds
  float val = analogRead(sensorPin);    // Read the analog value form sensor
  digitalWrite(sensorPower, LOW); // Turn the sensor OFF
  return val;       // send current reading
}
