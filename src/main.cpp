#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>
#include <shared_hardware_config.h>

#include "DateEncoder.h"
#include "EspNowHelper.h"
#include "hardware_config.h"

// Forward declarations
void handleEncoderChange();
void handleEncoderPressed();
void handleDateMessage(const DateMessage& msg);

void clear();
void setMonthDay(int monthDay);
void setYear(int year);

Adafruit_7segment mmddDisplay = Adafruit_7segment();
Adafruit_7segment yyyyDisplay = Adafruit_7segment();
DateEncoder dateEncoder(ENCODER_PIN_DT, ENCODER_PIN_CLK, ENCODER_PIN_SW);

bool mmddDisplayConnected = false;
bool yyyyDisplayConnected = false;
bool isCleared = false;
bool shouldClear = false;

uint8_t hubAddress[] = HUB_MAC_ADDRESS;
EspNowHelper espNowHelper;

void setup() {
  Serial.begin(115200);

  Serial.println("Date Device Starting...");

  dateEncoder.begin(6, 26, 2056);

  Serial.print("Initial Date: ");
  Serial.println(dateEncoder.getFormattedDate());

  Serial.println("Initializing Displays...");
  mmddDisplayConnected = mmddDisplay.begin(MMDD_DISPLAY_I2C_ADDR);
  yyyyDisplayConnected = yyyyDisplay.begin(YYYY_DISPLAY_I2C_ADDR);

  if (mmddDisplayConnected) {
    Serial.println("  ✓ MMDD Display connected");
    mmddDisplay.setBrightness(DISPLAY_BRIGHTNESS);
  } else {
    Serial.println("  ✗ MMDD Display not found");
  }

  if (yyyyDisplayConnected) {
    Serial.println("  ✓ YYYY Display connected");
    yyyyDisplay.setBrightness(DISPLAY_BRIGHTNESS);
  } else {
    Serial.println("  ✗ YYYY Display not found");
  }

  clear();

  espNowHelper.begin(DEVICE_ID);
  espNowHelper.addPeer(hubAddress);
  espNowHelper.registerDateMessageHandler(handleDateMessage);
  espNowHelper.sendDateConnected(hubAddress);
}

void loop() {
  if (shouldClear) {
    Serial.println("Clearing Displays...");
    clear();
    shouldClear = false;
  }

  dateEncoder.update();
  if (dateEncoder.hasChanged()) {
    handleEncoderChange();
  }
  if (dateEncoder.wasButtonPressed()) {
    handleEncoderPressed();
  }
  delay(10);
}

void handleEncoderChange() {
  if (isCleared) {
    setMonthDay(dateEncoder.getMonthDay());
    setYear(dateEncoder.getYear());
    isCleared = false;
  } else {
    switch (dateEncoder.getCurrentField()) {
      case FIELD_MONTH:
        Serial.print("Month Changed: ");
        setMonthDay(dateEncoder.getMonthDay());
        break;
      case FIELD_DAY:
        Serial.print("Day Changed: ");
        setMonthDay(dateEncoder.getMonthDay());
        break;
      case FIELD_YEAR:
        Serial.print("Year Changed: ");
        setYear(dateEncoder.getYear());
        break;
    }
  }

  Serial.println(dateEncoder.getFormattedDate());

  espNowHelper.sendDateUpdated(hubAddress, dateEncoder.getMonth(), dateEncoder.getDay(),
                               dateEncoder.getYear());
}

void handleEncoderPressed() {
  Serial.print("Button Pressed: ");
  Serial.println(dateEncoder.getFormattedDate());
}

void handleDateMessage(const DateMessage& msg) {
  Serial.print("Received Date Message: ");
  Serial.print(msg.month);
  Serial.print("-");
  Serial.print(msg.day);
  Serial.print("-");
  Serial.println(msg.year);

  shouldClear = true;
}

void clear() {
  setMonthDay(0);
  setYear(0);
  isCleared = true;
}

void setMonthDay(int monthDay) {
  if (mmddDisplayConnected) {
    if (monthDay == 0) {
      // Render ---- for cleared state
      mmddDisplay.clear();
      mmddDisplay.writeDigitRaw(0, 0x40);
      mmddDisplay.writeDigitRaw(1, 0x40);
      mmddDisplay.writeDigitRaw(3, 0x40);
      mmddDisplay.writeDigitRaw(4, 0x40);
    } else {
      mmddDisplay.print(monthDay);
    }
    mmddDisplay.writeDisplay();
  }
}

void setYear(int year) {
  if (yyyyDisplayConnected) {
    if (year == 0) {
      // Render ---- for cleared state
      yyyyDisplay.clear();
      yyyyDisplay.writeDigitRaw(0, 0x40);
      yyyyDisplay.writeDigitRaw(1, 0x40);
      yyyyDisplay.writeDigitRaw(3, 0x40);
      yyyyDisplay.writeDigitRaw(4, 0x40);
    } else {
      yyyyDisplay.print(year);
    }
    yyyyDisplay.writeDisplay();
  }
}
