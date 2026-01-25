#pragma once

#include <Encoder.h>

enum DateField { FIELD_MONTH, FIELD_DAY, FIELD_YEAR };

class DateEncoder {
  private:
    Encoder* encoder;
    int dataPin;
    int clockPin;
    int switchPin;

    // Date values
    int currentMonth;
    int currentDay;
    int currentYear;

    // State tracking
    DateField currentField;
    int lastEncoderValue;
    bool lastStableButtonState;
    bool lastButtonReading;
    unsigned long lastDebounceTime;
    bool dateChanged;
    bool buttonWasPressed;

    char formattedDateBuffer[12];  // Buffer for "MM-DD-YYYY\0"

    static const int COUNTS_PER_DETENT = 4;
    static const unsigned long DEBOUNCE_DELAY = 50;

    // Date field constraints
    static const int MONTH_MIN = 1;
    static const int MONTH_MAX = 12;
    static const int DAY_MIN = 1;
    static const int DAY_MAX = 31;
    static const int YEAR_MIN = 1900;
    static const int YEAR_MAX = 2100;

    void updateEncoderRange();
    void switchField();
    void readRotaryEncoder();
    void readSwitch();

  public:
    DateEncoder(int dataPin, int clockPin, int switchPin);
    ~DateEncoder();

    void begin(int month, int day, int year);
    void reset(int month, int day, int year);
    void update();

    bool hasChanged();
    bool wasButtonPressed();

    int getMonth() const;
    int getDay() const;
    int getYear() const;
    int getMonthDay() const;
    DateField getCurrentField() const;

    const char* getFormattedDate();
};
