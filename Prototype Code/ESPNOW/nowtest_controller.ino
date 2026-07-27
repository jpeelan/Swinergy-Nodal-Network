#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

//MAC address of ESP32M1
uint8_t receiverMAC[] = {
  0x84, 0x1F, 0xE8, 0xEE, 0xC4, 0x48
};

struct Message {
  int messageNumber;
  char text[50];
};

Message outgoingMessage;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Force both ESP32s to use Wi-Fi channel 1
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  Serial.println();
  Serial.println("ESP32 1: SENDER");
  Serial.print("My MAC address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  esp_now_peer_info_t peerInfo = {};

  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add receiver!");
    return;
  }

  Serial.println("ESP-NOW ready.");
}

void loop() {
  static int counter = 1;

  outgoingMessage.messageNumber = counter;

  snprintf(
    outgoingMessage.text,
    sizeof(outgoingMessage.text),
    "Hello from ESP32 1"
  );

  esp_err_t result = esp_now_send(
    receiverMAC,
    (uint8_t *)&outgoingMessage,
    sizeof(outgoingMessage)
  );

  if (result == ESP_OK) {
    Serial.print("Sent message #");
    Serial.println(counter);
  } else {
    Serial.print("Send error: ");
    Serial.println(result);
  }

  counter++;

  delay(1000);
}