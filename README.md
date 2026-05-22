# Practice Keyer

A hardware-based Morse code memory keyer built on ESP32 for ham radio operators and CW enthusiasts.

## Overview

Practice Keyer is a dedicated device that allows you to:
- Key Morse code with an iambic paddle
- Record messages to 4 memory slots (100+ characters each)
- Play back recordings at adjustable speeds (5-40 WPM)
- Practice at different speeds for skill building
- Store recordings persistently across power cycles

## Hardware

- **Platform**: ESP32 microcontroller
- **Inputs**: Iambic paddle, 5 buttons (RECORD + 4 slots), speed potentiometer
- **Outputs**: PWM buzzer (600Hz sidetone), status LED
- **Framework**: Arduino via PlatformIO

## Quick Start

```bash
# Build firmware
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Documentation

For complete project details, see the memory bank documentation:

- **[Project Brief](.clinerules/memory-bank/projectbrief.md)** - Core purpose, goals, and success criteria
- **[Product Context](.clinerules/memory-bank/productContext.md)** - Why this exists, user experience, target audience
- **[Active Context](.clinerules/memory-bank/activeContext.md)** - Current focus, recent changes, next steps
- **[System Patterns](.clinerules/memory-bank/systemPatterns.md)** - Architecture, design patterns, technical decisions
- **[Tech Context](.clinerules/memory-bank/techContext.md)** - Technology stack, dependencies, development setup
- **[Progress](.clinerules/memory-bank/progress.md)** - Implementation status, known issues, milestones

## Status

✅ Firmware implementation complete  
✅ Build successful (23.9KB RAM, 287KB Flash)  
🔄 Ready for hardware testing

## Hardware Requirements

- ESP32 development board
- 2× paddle buttons (or iambic paddle)
- 5× pushbuttons (RECORD + 4 SLOT buttons)
- 10kΩ potentiometer
- Passive buzzer (PWM-capable)
- LED + resistor
- Breadboard and wires

## Features

- **Iambic Keyer**: Authentic paddle keying experience
- **Variable Speed**: 5-40 WPM via potentiometer
- **4 Memory Slots**: Record and store messages
- **Speed-Independent Playback**: Practice at any speed
- **Persistent Storage**: ESP32 NVS flash
- **Simple Interface**: Physical buttons, no screen required

## License

[Add your license here]

## Author

VK3FSK
