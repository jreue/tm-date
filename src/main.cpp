#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>
#include <shared_hardware_config.h>

#include "DateEncoder.h"
#include "EspNowHelper.h"
#include "hardware_config.h"

// Forward declarations
void handleEncoderChange();
void handleEncoderPressed();

Adafruit_7segment mmddDisplay = Adafruit_7segment();
Adafruit_7segment yyyyDisplay = Adafruit_7segment();
DateEncoder dateEncoder(ENCODER_PIN_DT, ENCODER_PIN_CLK, ENCODER_PIN_SW);

bool mmddDisplayConnected = false;
bool yyyyDisplayConnected = false;

uint8_t hubAddress[] = HUB_MAC_ADDRESS;
EspNowHelper espNowHelper;

void setup() {
  Serial.begin(115200);

  Serial.println("Date Device Starting...");

  dateEncoder.begin(9, 21, 2006);

  Serial.print("Initial Date: ");
  Serial.println(dateEncoder.getFormattedDate());

  Serial.println("Initializing Displays...");
  mmddDisplayConnected = mmddDisplay.begin(MMDD_DISPLAY_I2C_ADDR);
  yyyyDisplayConnected = yyyyDisplay.begin(YYYY_DISPLAY_I2C_ADDR);

  if (mmddDisplayConnected) {
    Serial.println("  ✓ MMDD Display connected");
    mmddDisplay.setBrightness(DISPLAY_BRIGHTNESS);
    mmddDisplay.print(dateEncoder.getMonthDay());
    mmddDisplay.writeDisplay();
  } else {
    Serial.println("  ✗ MMDD Display not found");
  }

  if (yyyyDisplayConnected) {
    Serial.println("  ✓ YYYY Display connected");
    yyyyDisplay.setBrightness(DISPLAY_BRIGHTNESS);
    yyyyDisplay.print(dateEncoder.getYear());
    yyyyDisplay.writeDisplay();
  } else {
    Serial.println("  ✗ YYYY Display not found");
  }

  espNowHelper.begin(hubAddress, DEVICE_ID);
  espNowHelper.sendDateConnected();
}

void loop() {
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
  switch (dateEncoder.getCurrentField()) {
    case FIELD_MONTH:
      Serial.print("Month Changed: ");
      if (mmddDisplayConnected) {
        mmddDisplay.print(dateEncoder.getMonthDay());
        mmddDisplay.writeDisplay();
      }
      break;
    case FIELD_DAY:
      Serial.print("Day Changed: ");
      if (mmddDisplayConnected) {
        mmddDisplay.print(dateEncoder.getMonthDay());
        mmddDisplay.writeDisplay();
      }
      break;
    case FIELD_YEAR:
      Serial.print("Year Changed: ");
      if (yyyyDisplayConnected) {
        yyyyDisplay.print(dateEncoder.getYear());
        yyyyDisplay.writeDisplay();
      }
      break;
  }

  Serial.println(dateEncoder.getFormattedDate());

  espNowHelper.sendDateUpdated(dateEncoder.getMonth(), dateEncoder.getDay(), dateEncoder.getYear());
}

void handleEncoderPressed() {
  Serial.print("Button Pressed: ");
  Serial.println(dateEncoder.getFormattedDate());
}
