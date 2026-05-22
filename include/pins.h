/**
 * Practice Keyer - GPIO Pin Definitions
 * ESP32 Morse Code Memory Keyer
 */

#ifndef PINS_H
#define PINS_H

// Paddle Inputs (with pull-ups)
#define PIN_DIT_PADDLE    34  // Dit paddle input
#define PIN_DAH_PADDLE    35  // Dah paddle input

// Speed Control
#define PIN_SPEED_POT     36  // Potentiometer ADC input (GPIO36 = ADC1_CH0)

// Button Inputs (with pull-ups)
#define PIN_RECORD_BTN    32  // Record button
#define PIN_SLOT1_BTN     33  // Slot 1 playback button
#define PIN_SLOT2_BTN     25  // Slot 2 playback button
#define PIN_SLOT3_BTN     26  // Slot 3 playback button
#define PIN_SLOT4_BTN     27  // Slot 4 playback button

// Outputs
#define PIN_BUZZER        23  // PWM buzzer output (600Hz tone)
#define PIN_STATUS_LED    22  // Status LED (recording indicator)

// PWM Configuration
#define PWM_CHANNEL       0   // LEDC channel for buzzer
#define PWM_FREQUENCY     600 // Buzzer frequency in Hz (standard CW sidetone)
#define PWM_RESOLUTION    8   // 8-bit resolution (0-255)

// Timing Constants
#define DEBOUNCE_MS       50  // Button debounce time in milliseconds
#define WPM_MIN           5   // Minimum words per minute
#define WPM_MAX           40  // Maximum words per minute

// Memory Configuration
#define NUM_SLOTS         4   // Number of memory slots
#define MAX_ELEMENTS      500 // Maximum morse elements per slot (~100 characters)

#endif // PINS_H
