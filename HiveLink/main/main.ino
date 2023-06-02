#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  dht.begin();
  delay(1500);
}


void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.println("Humidity: ");
  Serial.println(h);
  Serial.println("Temperature: ");
  Serial.println(t);
  delay(3000);
}