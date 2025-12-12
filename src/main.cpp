#include <Arduino.h>

#include "DateController.h"
#include "DateDialsController.h"

DateController dateController;
DateDialsController dateDialsController;

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  dateController.begin();
  dateController.showDate(12, 25, 1975);

  Serial.println("Date Selector Initialized");
}

// Reminder - Remove the Serial.println statements from DIYables_4Digit7Segment_74HC595.cpp
// as it print each character in printInt()
void loop() {
  dateController.loop();

  DateFromDials date = dateDialsController.readDate();

  if (date.changed) {
    Serial.println(String(date.month) + "/" + String(date.day) + "/" + String(date.year));
    dateController.showDate(date.month, date.day, date.year);
  }
}