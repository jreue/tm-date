#pragma once

#include <DIYables_4Digit7Segment_74HC595.h>

#include "hardware_config.h"

class DateController {
  public:
    DateController();

    void begin();
    void loop();
    void showDate(int month, int day, int year);

  private:
    DIYables_4Digit7Segment_74HC595 display1;
    DIYables_4Digit7Segment_74HC595 display2;
    void showMonthAndDay(int month, int day);
    void showYear(int year);
};