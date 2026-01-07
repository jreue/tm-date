#include "DateEncoder.h"

DateEncoder::DateEncoder(int dataPin, int clockPin, int switchPin)
    : encoder(nullptr),
      dataPin(dataPin),
      clockPin(clockPin),
      switchPin(switchPin),
      currentMonth(1),
      currentDay(1),
      currentYear(2000),
      currentField(FIELD_MONTH),
      lastEncoderValue(0),
      lastStableButtonState(HIGH),
      lastButtonReading(HIGH),
      lastDebounceTime(0),
      dateChanged(false),
      buttonWasPressed(false) {
  // Encoder will be created in begin()
}

DateEncoder::~DateEncoder() {
  if (encoder) {
    delete encoder;
  }
}

void DateEncoder::begin(int month, int day, int year) {
  // Create the encoder here, after GPIO system is initialized
  encoder = new Encoder(dataPin, clockPin);

  // Setup button with internal pullup
  pinMode(switchPin, INPUT_PULLUP);

  // Store initial date
  currentMonth = constrain(month, MONTH_MIN, MONTH_MAX);
  currentDay = constrain(day, DAY_MIN, DAY_MAX);
  currentYear = constrain(year, YEAR_MIN, YEAR_MAX);

  // Start in month editing mode
  currentField = FIELD_MONTH;
  updateEncoderRange();
}

void DateEncoder::updateEncoderRange() {
  int value;

  switch (currentField) {
    case FIELD_MONTH:
      value = currentMonth;
      break;
    case FIELD_DAY:
      value = currentDay;
      break;
    case FIELD_YEAR:
      value = currentYear;
      break;
  }

  encoder->write(value * COUNTS_PER_DETENT);
  lastEncoderValue = value;
}

void DateEncoder::switchField() {
  if (currentField == FIELD_MONTH) {
    currentField = FIELD_DAY;
  } else if (currentField == FIELD_DAY) {
    currentField = FIELD_YEAR;
  } else {
    currentField = FIELD_MONTH;
  }
  updateEncoderRange();
}

void DateEncoder::update() {
  readRotaryEncoder();
  readSwitch();
}

void DateEncoder::readRotaryEncoder() {
  dateChanged = false;
  // Read and process encoder
  long rawPosition = encoder->read();
  int encoderValue = rawPosition / COUNTS_PER_DETENT;

  // Apply min/max constraints based on current field
  int minVal, maxVal;
  switch (currentField) {
    case FIELD_MONTH:
      minVal = MONTH_MIN;
      maxVal = MONTH_MAX;
      break;
    case FIELD_DAY:
      minVal = DAY_MIN;
      maxVal = DAY_MAX;
      break;
    case FIELD_YEAR:
      minVal = YEAR_MIN;
      maxVal = YEAR_MAX;
      break;
  }

  // Clamp and write back
  if (encoderValue < minVal) {
    encoderValue = minVal;
    encoder->write(minVal * COUNTS_PER_DETENT);
  } else if (encoderValue > maxVal) {
    encoderValue = maxVal;
    encoder->write(maxVal * COUNTS_PER_DETENT);
  }

  // Check if value changed
  if (encoderValue != lastEncoderValue) {
    switch (currentField) {
      case FIELD_MONTH:
        currentMonth = encoderValue;
        break;
      case FIELD_DAY:
        currentDay = encoderValue;
        break;
      case FIELD_YEAR:
        currentYear = encoderValue;
        break;
    }
    lastEncoderValue = encoderValue;
    dateChanged = true;
  }
}

void DateEncoder::readSwitch() {
  buttonWasPressed = false;

  // Handle button with debouncing
  bool buttonReading = digitalRead(switchPin);

  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = buttonReading;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (buttonReading != lastStableButtonState) {
      if (buttonReading == LOW) {
        buttonWasPressed = true;
        switchField();
      }
      lastStableButtonState = buttonReading;
    }
  }
}

bool DateEncoder::hasChanged() {
  return dateChanged;
}

bool DateEncoder::wasButtonPressed() {
  return buttonWasPressed;
}

int DateEncoder::getMonth() const {
  return currentMonth;
}

int DateEncoder::getDay() const {
  return currentDay;
}

int DateEncoder::getYear() const {
  return currentYear;
}

int DateEncoder::getMonthDay() const {
  return currentMonth * 100 + currentDay;
}

DateField DateEncoder::getCurrentField() const {
  return currentField;
}

const char* DateEncoder::getFormattedDate() {
  snprintf(formattedDateBuffer, sizeof(formattedDateBuffer), "%02d-%02d-%04d", currentMonth,
           currentDay, currentYear);
  return formattedDateBuffer;
}
