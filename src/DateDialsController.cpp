#include "DateDialsController.h"

#include "hardware_config.h"

DateDialsController::DateDialsController() {
  lastMonth = -1;
  lastDay = -1;
  lastYear = -1;
}

int DateDialsController::readAveraged(uint8_t pin, uint8_t samples) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(50);
  }
  return sum / samples;
}

int DateDialsController::mapToSelection(int raw, int minVal, int maxVal) {
  return map(raw, 0, 4095, minVal, maxVal);
}

DateFromDials DateDialsController::readDate() {
  int monthRaw = readAveraged(MONTH_POT_PIN);
  int month = mapToSelection(monthRaw, 1, 12);

  int dayRaw = readAveraged(DAY_POT_PIN);
  int day = mapToSelection(dayRaw, 1, 31);

  int yearHighRaw = readAveraged(YEAR_HIGH_POT_PIN);
  int yearHigh = mapToSelection(yearHighRaw, 10, 30);

  int yearLowRaw = readAveraged(YEAR_LOW_POT_PIN);
  int yearLow = mapToSelection(yearLowRaw, 0, 99);

  int year = yearHigh * 100 + yearLow;

  bool changed = (month != lastMonth || day != lastDay || year != lastYear);

  if (changed) {
    lastMonth = month;
    lastDay = day;
    lastYear = year;
  }

  return {month, day, year, changed};
}
