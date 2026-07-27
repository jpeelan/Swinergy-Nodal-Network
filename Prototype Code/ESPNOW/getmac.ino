#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("This ESP32 MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}