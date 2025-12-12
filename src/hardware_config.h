#pragma once

// ====================
// UART Configuration
// ====================
#define UART_BAUD_RATE 9600
#define UART_RX_PIN 19
#define UART_TX_PIN 21

// ====================
// 4-Digit 7-Segment Display
// ====================
// Display Module 1 MM.DD
#define D1_DIO_PIN 4
#define D1_RCLK_PIN 2
#define D1_SCLK_PIN 15

// Display Module 2 YYYY
#define D2_DIO_PIN 5
#define D2_RCLK_PIN 17
#define D2_SCLK_PIN 16

// MM DD YYYY Potentiometers
#define MONTH_POT_PIN 35
#define DAY_POT_PIN 34
#define YEAR_HIGH_POT_PIN 39
#define YEAR_LOW_POT_PIN 36