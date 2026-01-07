#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Message types
#define MSG_TYPE_CONNECT 0
#define MSG_TYPE_STATUS 1
#define MSG_TYPE_DISCONNECT 2
#define MSG_TYPE_DATE_UPDATE 3

typedef struct DateMessage {
    uint8_t id;
    uint8_t messageType;  // MSG_TYPE_CONNECT, MSG_TYPE_STATUS, MSG_TYPE_DISCONNECT,
                          // MSG_TYPE_DATE_UPDATE
    uint8_t month;
    uint8_t day;
    uint16_t year;
} DateMessage;

class EspNowHelper {
  public:
    EspNowHelper();

    void begin(uint8_t* hubMacAddress, uint8_t deviceId);
    void sendConnected();
    void updateDate(uint8_t month, uint8_t day, uint16_t year);

  private:
    uint8_t* receiverAddress;
    DateMessage message;

    void sendMessage();
    static void handleDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
};
