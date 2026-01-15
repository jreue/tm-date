#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>

#include "DateEncoder.h"
#include "EspNowHelper.h"
#include "hardware_config.h"

// Forward declarations
void handleEncoderChange();
void handleEncoderPressed();

Adafruit_7segment mmddDisplay = Adafruit_7segment();
Adafruit_7segment yyyyDisplay = Adafruit_7segment();
DateEncoder dateEncoder(ENCODER_PIN_DT, ENCODER_PIN_CLK, ENCODER_PIN_SW);

uint8_t hubAddress[] = HUB_MAC_ADDRESS;
EspNowHelper espNowHelper;

void setup() {
  Serial.begin(115200);

  Serial.println("Date Encoder with ESP-NOW Starting...");

  dateEncoder.begin(9, 21, 2006);

  Serial.print("Initial Date: ");
  Serial.println(dateEncoder.getFormattedDate());

  mmddDisplay.begin(MMDD_DISPLAY_I2C_ADDR);
  yyyyDisplay.begin(YYYY_DISPLAY_I2C_ADDR);

  mmddDisplay.setBrightness(DISPLAY_BRIGHTNESS);
  yyyyDisplay.setBrightness(DISPLAY_BRIGHTNESS);

  mmddDisplay.print(dateEncoder.getMonthDay());
  yyyyDisplay.print(dateEncoder.getYear());

  mmddDisplay.writeDisplay();
  yyyyDisplay.writeDisplay();

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
      mmddDisplay.print(dateEncoder.getMonthDay());
      mmddDisplay.writeDisplay();
      break;
    case FIELD_DAY:
      Serial.print("Day Changed: ");
      mmddDisplay.print(dateEncoder.getMonthDay());
      mmddDisplay.writeDisplay();
      break;
    case FIELD_YEAR:
      Serial.print("Year Changed: ");

      yyyyDisplay.print(dateEncoder.getYear());
      yyyyDisplay.writeDisplay();
      break;
  }
  Serial.println(dateEncoder.getFormattedDate());

  espNowHelper.sendDateUpdated(dateEncoder.getMonth(), dateEncoder.getDay(), dateEncoder.getYear());
}

void handleEncoderPressed() {
  Serial.print("Button Pressed: ");
  Serial.println(dateEncoder.getFormattedDate());
}
