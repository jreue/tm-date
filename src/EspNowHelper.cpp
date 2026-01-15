#include "EspNowHelper.h"

EspNowHelper::EspNowHelper() : receiverAddress(nullptr) {
}

void EspNowHelper::begin(uint8_t* hubMacAddress, int dateId) {
  receiverAddress = hubMacAddress;
  deviceId = dateId;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());

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

void EspNowHelper::sendMessage(EspNowHeader& message, size_t messageSize) {
  Serial.println("  → Preparing message:");
  Serial.print("      Device ID: ");
  Serial.println(message.deviceId);
  Serial.print("      Device Type: ");
  Serial.println(message.deviceType);
  Serial.print("      Message Type: ");
  Serial.println(message.messageType);

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t*)&message, messageSize);

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

void EspNowHelper::sendDateConnected() {
  Serial.println("Sending Date Connected Message...");

  DateMessage message;
  message.deviceId = deviceId;
  message.deviceType = DEVICE_TYPE_DATE;
  message.messageType = MSG_TYPE_CONNECT;
  message.month = 0;
  message.day = 0;
  message.year = 0;

  sendMessage((EspNowHeader&)message, sizeof(message));
}

void EspNowHelper::sendDateUpdated(uint8_t month, uint8_t day, uint16_t year) {
  Serial.println("Sending Date Updated Message...");

  DateMessage message;
  message.deviceId = deviceId;
  message.deviceType = DEVICE_TYPE_DATE;
  message.messageType = MSG_TYPE_DATA;
  message.month = month;
  message.day = day;
  message.year = year;

  Serial.print("  → Date: ");
  Serial.print(month);
  Serial.print("/");
  Serial.print(day);
  Serial.print("/");
  Serial.println(year);
  sendMessage((EspNowHeader&)message, sizeof(message));
}
