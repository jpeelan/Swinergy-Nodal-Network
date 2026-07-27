#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>

struct Message {
  int messageNumber;
  char text[50];
};

Message incomingMessage;

// Callback for newer Arduino ESP32 core versions
#if ESP_ARDUINO_VERSION_MAJOR >= 3

void onDataReceived(
  const esp_now_recv_info_t *info,
  const uint8_t *data,
  int length
) {
  if (length != sizeof(incomingMessage)) {
    Serial.println("Wrong message size received.");
    return;
  }

  memcpy(&incomingMessage, data, sizeof(incomingMessage));

  Serial.println("--------------------");
  Serial.print("Received message #");
  Serial.println(incomingMessage.messageNumber);

  Serial.print("Message: ");
  Serial.println(incomingMessage.text);

  Serial.print("Signal strength: ");
  Serial.print(info->rx_ctrl->rssi);
  Serial.println(" dBm");
}

// Callback for older Arduino ESP32 core versions
#else

void onDataReceived(
  const uint8_t *senderMAC,
  const uint8_t *data,
  int length
) {
  if (length != sizeof(incomingMessage)) {
    Serial.println("Wrong message size received.");
    return;
  }

  memcpy(&incomingMessage, data, sizeof(incomingMessage));

  Serial.println("--------------------");
  Serial.print("Received message #");
  Serial.println(incomingMessage.messageNumber);

  Serial.print("Message: ");
  Serial.println(incomingMessage.text);
}

#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Must match the sender's channel
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  Serial.println();
  Serial.println("ESP32 2: RECEIVER");
  Serial.print("My MAC address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  esp_now_register_recv_cb(onDataReceived);

  Serial.println("Waiting for messages...");
}

void loop() {
  // Nothing needed here.
  // Messages are handled by onDataReceived().
}