#include "EspNowHelper.h"

EspNowHelper::EspNowHelper() : receiverAddress(nullptr) {
  message.deviceId = 0;
  message.month = 9;
  message.day = 21;
  message.year = 2006;
}

void EspNowHelper::begin(uint8_t* hubMacAddress, int dateId) {
  receiverAddress = hubMacAddress;
  deviceId = dateId;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.printf("Date MAC Address: %s\n", WiFi.macAddress().c_str());

  Serial.println("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(handleESPNowDataSent);

  Serial.println("Adding ESP-NOW Peers...");
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, hubMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add HUB peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add HUB peer");
    return;
  }
}

void EspNowHelper::handleESPNowDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("  ✓ Delivery confirmed");
  } else {
    Serial.println("  ✗ Delivery failed");
  }
  Serial.println("------------------------");
}

void EspNowHelper::sendConnected() {
  Serial.println("Sending Connected Message...");

  message.deviceId = deviceId;
  message.deviceType = DEVICE_TYPE_DATE;
  message.messageType = MSG_TYPE_CONNECT;

  sendMessage();
}

void EspNowHelper::updateDate(uint8_t month, uint8_t day, uint16_t year) {
  if (month != message.month || day != message.day || year != message.year) {
    Serial.println("Sending Data Message...");

    message.deviceId = deviceId;
    message.deviceType = DEVICE_TYPE_DATE;
    message.messageType = MSG_TYPE_DATA;
    message.month = month;
    message.day = day;
    message.year = year;

    sendMessage();
  }
}

void EspNowHelper::sendMessage() {
  Serial.println("  → Preparing message:");
  Serial.print("      Device ID: ");
  Serial.println(message.deviceId);
  Serial.print("      Device Type: ");
  Serial.println(message.deviceType);
  Serial.print("      Message Type: ");
  Serial.println(message.messageType);

  Serial.print("      Date: ");
  Serial.print(message.month);
  Serial.print("/");
  Serial.print(message.day);
  Serial.print("/");
  Serial.println(message.year);

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t*)&message, sizeof(message));

  if (result == ESP_OK) {
    Serial.println("  → Message queued for sending");
  } else {
    Serial.print("  ✗ Error queueing message: ");
    switch (result) {
      case ESP_ERR_ESPNOW_NOT_INIT:
        Serial.println("ESP_ERR_ESPNOW_NOT_INIT: ESP-NOW not initialized");
        break;
      case ESP_ERR_ESPNOW_ARG:
        Serial.println("ESP_ERR_ESPNOW_ARG: Invalid argument");
        break;
      case ESP_ERR_ESPNOW_INTERNAL:
        Serial.println("ESP_ERR_ESPNOW_INTERNAL: Internal error");
        break;
      case ESP_ERR_ESPNOW_NO_MEM:
        Serial.println("ESP_ERR_ESPNOW_NO_MEM: Out of memory");
        break;
      case ESP_ERR_ESPNOW_NOT_FOUND:
        Serial.println("ESP_ERR_ESPNOW_NOT_FOUND: Peer not found");
        break;
      case ESP_ERR_ESPNOW_IF:
        Serial.println("ESP_ERR_ESPNOW_IF: WiFi interface error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
}
