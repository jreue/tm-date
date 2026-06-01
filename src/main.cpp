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
void renderTravelEffect(unsigned long durationMs);

Adafruit_7segment mmddDisplay = Adafruit_7segment();
Adafruit_7segment yyyyDisplay = Adafruit_7segment();
DateEncoder dateEncoder(ENCODER_PIN_DT, ENCODER_PIN_CLK, ENCODER_PIN_SW);

bool mmddDisplayConnected = false;
bool yyyyDisplayConnected = false;
bool isCleared = false;
bool travelEffectActive = false;

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
  if (travelEffectActive) {
    travelEffectActive = false;
    renderTravelEffect(18000);
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

  travelEffectActive = true;
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

// ============================================================
// Travel Effect
// ============================================================

static const uint8_t DIGIT_POS[4] = {0, 1, 3, 4};

// Outer ring segments (A, B, C, D, E, F) — clockwise comet path
static const uint8_t COMET_RING[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};

// Accumulating segment masks for cascade fill; last entry clears
static const uint8_t CASCADE_MASKS[8] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0x00};

static int computeFrameDelay(float phase) {
  const int SLOW_DELAY = 200;
  const int FAST_DELAY = 15;
  const float RAMP_UP_END = 0.222f;
  const float WIND_DOWN_START = 0.778f;

  if (phase < RAMP_UP_END) {
    float t = phase / RAMP_UP_END;
    return SLOW_DELAY + (int)((FAST_DELAY - SLOW_DELAY) * t);
  } else if (phase < WIND_DOWN_START) {
    return FAST_DELAY;
  } else {
    float t = (phase - WIND_DOWN_START) / (1.0f - WIND_DOWN_START);
    return FAST_DELAY + (int)((SLOW_DELAY - FAST_DELAY) * t);
  }
}

// Each digit shows one rotating outer-ring segment, phase-shifted by digit index
static void effectComet(uint32_t step) {
  for (int i = 0; i < 4; i++) {
    uint8_t seg = COMET_RING[(step + i) % 6];
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitRaw(DIGIT_POS[i], seg);
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitRaw(DIGIT_POS[i], seg);
  }
  if (mmddDisplayConnected)
    mmddDisplay.writeDisplay();
  if (yyyyDisplayConnected)
    yyyyDisplay.writeDisplay();
}

// Random 0–9 on every digit position each frame
static void effectNumberScramble(uint32_t step) {
  (void)step;
  for (int i = 0; i < 4; i++) {
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitNum(DIGIT_POS[i], (uint8_t)random(0, 10));
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitNum(DIGIT_POS[i], (uint8_t)random(0, 10));
  }
  if (mmddDisplayConnected)
    mmddDisplay.writeDisplay();
  if (yyyyDisplayConnected)
    yyyyDisplay.writeDisplay();
}

// One fully-lit column sweeps left-to-right across all 8 digit positions (4 mmdd + 4 yyyy)
static void effectColumnChase(uint32_t step) {
  uint8_t activeCol = (uint8_t)(step % 8);

  if (mmddDisplayConnected) {
    mmddDisplay.clear();
    if (activeCol < 4) {
      mmddDisplay.writeDigitRaw(DIGIT_POS[activeCol], 0x7F);
    }
    mmddDisplay.writeDisplay();
  }

  if (yyyyDisplayConnected) {
    yyyyDisplay.clear();
    if (activeCol >= 4) {
      yyyyDisplay.writeDigitRaw(DIGIT_POS[activeCol - 4], 0x7F);
    }
    yyyyDisplay.writeDisplay();
  }
}

// Cycles A→B→C→D→E→F→G, lighting that one segment across all 8 digit positions at once
static const uint8_t SEG_WAVE[7] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};
static void effectSegmentWave(uint32_t step) {
  uint8_t seg = SEG_WAVE[step % 7];
  for (int i = 0; i < 4; i++) {
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitRaw(DIGIT_POS[i], seg);
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitRaw(DIGIT_POS[i], seg);
  }
  if (mmddDisplayConnected)
    mmddDisplay.writeDisplay();
  if (yyyyDisplayConnected)
    yyyyDisplay.writeDisplay();
}

// Segments accumulate one-by-one across all digits, then reset
static void effectCascadeFill(uint32_t step) {
  uint8_t mask = CASCADE_MASKS[step % 8];
  for (int i = 0; i < 4; i++) {
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitRaw(DIGIT_POS[i], mask);
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitRaw(DIGIT_POS[i], mask);
  }
  if (mmddDisplayConnected)
    mmddDisplay.writeDisplay();
  if (yyyyDisplayConnected)
    yyyyDisplay.writeDisplay();
}

// Left 2 digits vs right 2 digits alternate on both displays each step
static void effectAlternatingHalf(uint32_t step) {
  uint8_t leftVal = (step % 2 == 0) ? 0x7F : 0x00;
  uint8_t rightVal = (step % 2 == 0) ? 0x00 : 0x7F;
  for (int i = 0; i < 2; i++) {
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitRaw(DIGIT_POS[i], leftVal);
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitRaw(DIGIT_POS[i], leftVal);
  }
  for (int i = 2; i < 4; i++) {
    if (mmddDisplayConnected)
      mmddDisplay.writeDigitRaw(DIGIT_POS[i], rightVal);
    if (yyyyDisplayConnected)
      yyyyDisplay.writeDigitRaw(DIGIT_POS[i], rightVal);
  }
  if (mmddDisplayConnected)
    mmddDisplay.writeDisplay();
  if (yyyyDisplayConnected)
    yyyyDisplay.writeDisplay();
}

void renderTravelEffect(unsigned long totalDurationMs) {
  unsigned long start = millis();
  uint32_t step = 0;

  while (true) {
    unsigned long elapsed = millis() - start;
    if (elapsed >= totalDurationMs)
      break;

    float phase = (float)elapsed / (float)totalDurationMs;
    int frameDelay = computeFrameDelay(phase);
    uint8_t currentEffect = (uint8_t)((elapsed / 1500UL) % 6);

    switch (currentEffect) {
      case 0:
        effectComet(step);
        break;
      case 1:
        effectNumberScramble(step);
        break;
      case 2:
        effectColumnChase(step);
        break;
      case 3:
        effectSegmentWave(step);
        break;
      case 4:
        effectCascadeFill(step);
        break;
      case 5:
        effectAlternatingHalf(step);
        break;
    }

    delay(frameDelay);
    step++;
  }

  clear();
}
