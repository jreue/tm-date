#pragma once

// ====================
// Rotary Encoder Pins
// ====================
#define ENCODER_PIN_SW 35   // GREEN
#define ENCODER_PIN_DT 34   // WHITE
#define ENCODER_PIN_CLK 39  // YELLOW

// ====================
// Adafruit 7-Segment Display I2C Addresses
// ====================
#define MMDD_DISPLAY_I2C_ADDR 0x70
#define YYYY_DISPLAY_I2C_ADDR 0x71

// ====================
// Adafruit 7-Segment Display Brightness
// ====================
#define DISPLAY_BRIGHTNESS 2  // 0 (min) to 15 (max)

// ====================
// ESP-NOW Configuration
// ====================
#define DEVICE_ID 20
#define HUB_MAC_ADDRESS {0xE0, 0x8C, 0xFE, 0x5C, 0x04, 0x58}
