# Active Context

## Current Focus
**Status**: Implementation Complete - Ready for Hardware Testing
**Phase**: Firmware Build Successful

This is an **ESP32-based hardware Morse code memory keyer** - NOT a web application. Complete firmware implementation finished and compiled successfully.

## Recent Changes
- Memory Bank initialized (May 22, 2026)
- Core documentation created
- **MAJOR PIVOT**: Project is embedded firmware for ESP32, not web app
- Complete hardware pin layout designed
- Memory structure optimized for 4-slot message storage
- All feature specifications finalized
- **ALL FIRMWARE IMPLEMENTED** (May 22, 2026)
- Build successful: 23.9KB RAM, 287KB Flash

## Next Immediate Steps
1. ✅ ~~Initialize PlatformIO project for ESP32~~
2. ✅ ~~Set up project structure (src/, include/, platformio.ini)~~
3. ✅ ~~Create pin definitions and hardware abstraction~~
4. ✅ ~~Implement iambic paddle keyer engine~~
5. ✅ ~~Add buzzer/PWM tone generation~~
6. ✅ ~~Build recording system with 4 memory slots~~
7. ✅ ~~Implement playback with variable speed~~
8. ✅ ~~Add NVS persistent storage~~
9. **Connect ESP32 hardware and test with real components**
10. Upload firmware and validate all functionality
11. Create wiring diagram and user documentation

## Active Decisions
- **Platform**: ESP32 with Arduino framework via PlatformIO ✓
- **Memory Slots**: 4 slots, 100+ characters each ✓
- **Storage Format**: Normalized elements (dit/dah/gap) for speed-independent playback ✓
- **Speed Control**: Potentiometer on GPIO 36 (ADC), 5-40 WPM range ✓
- **Playback Speed**: Uses current pot setting (not recorded speed) ✓
- **Recording**: Captures normalized elements, ignores original timing ✓

## Important Patterns & Preferences
- **Hardware-first design**: Physical buttons and paddle inputs
- **Simple user experience**: One record button, 4 slot buttons
- **Variable practice speed**: Playback at current pot setting for learning
- **Normalized storage**: Store elements, not timing (speed-independent)
- **Embedded best practices**: Efficient memory use, debouncing, state machines

## Current Learnings
- Project is an **embedded hardware device** (ESP32 microcontroller)
- Designed for ham radio operators and CW enthusiasts
- Memory keyer functionality: record and replay Morse messages
- Emphasis on simple physical interface (buttons/paddle/pot)
- Speed-adjustable playback for learning/practice
- Must handle real-time timing-critical operations
- Storage must persist across power cycles (NVS flash)

## Questions to Resolve
None - firmware implementation complete.

## Dependencies & Blockers
- Hardware setup required for testing:
  - ESP32 DevKit board
  - 2× paddle buttons (or iambic paddle)
  - 5× pushbuttons (RECORD + 4 SLOT buttons)
  - 10kΩ potentiometer
  - Passive buzzer (PWM-capable)
  - LED + resistor
  - Breadboard and wires

## Working Notes
- Keep physical interface simple (minimal buttons)
- Buzzer tone quality is critical for CW practice (600Hz standard)
- Memory efficiency important for embedded device (2KB for 4 slots)
- Timing precision critical for Morse code accuracy
- Debouncing essential for reliable button/paddle input
- LED feedback provides user confirmation without screen
- NVS storage ensures recordings survive power cycles
- Potentiometer allows real-time speed adjustment during practice
