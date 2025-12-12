#pragma once

#include <Arduino.h>

struct DateFromDials {
    int month;
    int day;
    int year;
    bool changed;
};

class DateDialsController {
  public:
    DateDialsController();
    DateFromDials readDate();

  private:
    int lastMonth;
    int lastDay;
    int lastYear;

    int readAveraged(uint8_t pin, uint8_t samples = 5);
    int mapToSelection(int raw, int minVal, int maxVal);
};
