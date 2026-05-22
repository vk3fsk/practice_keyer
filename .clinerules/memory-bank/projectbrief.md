# Project Brief: Practice Keyer

## Project Name
Practice Keyer - ESP32 Morse Code Memory Keyer

## Core Purpose
A hardware-based Morse code memory keyer for ham radio operators and CW enthusiasts to record, store, and replay Morse code messages at variable speeds for practice and learning.

## Primary Goals
1. Enable users to practice Morse code with authentic paddle keying
2. Record and store Morse code messages for repeated practice
3. Allow playback at adjustable speeds (5-40 WPM) for skill building
4. Provide a simple, reliable, standalone hardware device
5. Persist recordings across power cycles

## Key Features
- Iambic paddle keyer with sidetone (600Hz buzzer)
- Variable speed control via potentiometer (5-40 WPM)
- 4 memory slots for message storage (100+ characters each)
- One-button recording with LED indicator
- Playback at current speed setting (speed-independent practice)
- Persistent storage in ESP32 NVS flash
- Simple physical button interface (no screen/UI required)

## Target Users
- Amateur radio operators (ham radio)
- CW (continuous wave) operators learning/maintaining proficiency
- Radio contesters practicing standard exchanges
- Emergency communications personnel

## Success Criteria
- Reliable paddle input detection with accurate timing
- Smooth speed adjustment from 5-40 WPM
- 100+ character message capacity per slot
- Recordings persist across power cycles
- Simple, intuitive button operation
- Clear buzzer tone quality (600Hz)

## Out of Scope (Initial Version)
- WiFi/network connectivity
- LCD/OLED display
- Multiple keyer modes beyond iambic
- Message editing or text-to-Morse conversion
- Advanced features like beacon mode

## Technical Constraints
- ESP32 microcontroller platform
- Real-time timing requirements (±1ms precision)
- Limited RAM: ~2KB for 4 message slots
- Flash wear: minimize NVS write cycles
- GPIO pin limitations (avoid boot-sensitive pins)

## Timeline
Initial MVP to be developed iteratively

## Project Start Date
May 22, 2026
