#include "EspNowHelper.h"

EspNowHelper::EspNowHelper() : receiverAddress(nullptr) {
  message.id = 0;
  message.month = 9;
  message.day = 21;
  message.year = 2006;
}

void EspNowHelper::begin(uint8_t* hubMacAddress, uint8_t deviceId) {
  receiverAddress = hubMacAddress;
  message.id = deviceId;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.println("Device starting with ESP-NOW");
  Serial.print("Device MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Device ID: ");
  Serial.println(deviceId);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(handleDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, hubMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void EspNowHelper::sendConnected() {
  Serial.println("[Device Connected]");
  message.messageType = MSG_TYPE_CONNECT;
  sendMessage();
}

void EspNowHelper::updateDate(uint8_t month, uint8_t day, uint16_t year) {
  if (month != message.month || day != message.day || year != message.year) {
    Serial.println("Date changed...");
    message.month = month;
    message.day = day;
    message.year = year;
    message.messageType = MSG_TYPE_DATE_UPDATE;
    sendMessage();
  }
}

void EspNowHelper::handleDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("  ✓ Delivery confirmed");
  } else {
    Serial.println("  ✗ Delivery failed");
  }
  Serial.println("------------------------");
}

void EspNowHelper::sendMessage() {
  Serial.println("  → Preparing message:");
  Serial.print("      Device ID: ");
  Serial.println(message.id);
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
