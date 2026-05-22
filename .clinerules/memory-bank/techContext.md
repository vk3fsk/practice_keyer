# Technical Context

## Technology Stack

### Platform & Framework
- **Hardware**: ESP32 microcontroller (DevKit/WROOM)
- **Framework**: Arduino (via PlatformIO)
- **Language**: C++ (Arduino dialect)
- **Build System**: PlatformIO
- **Flash Tool**: esptool.py (automatic via PlatformIO)

## Development Environment

### Prerequisites
- **PlatformIO**: Installed via VS Code extension or CLI
- **USB Drivers**: CP210x or CH340 (for ESP32 USB connection)
- **Code Editor**: VS Code with PlatformIO IDE extension
- **Git**: For version control
- **Python 3**: Required by PlatformIO (usually bundled)

### Setup Instructions
```bash
# Initialize PlatformIO project
pio project init --board esp32dev

# Build project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

## Technical Constraints

### Hardware Limitations
- **RAM**: 520KB SRAM (using ~2KB for message slots)
- **Flash**: 4MB typical (NVS partition for persistent storage)
- **GPIO**: Limited safe pins (avoid 0,2,12,15 during boot)
- **ADC**: 12-bit resolution, non-linear response needs calibration

### Performance Requirements
- **Timing Precision**: ±1ms for accurate Morse code
- **Real-time Operation**: No blocking delays in main loop
- **PWM Audio**: 600Hz tone, 8-bit duty cycle resolution
- **Button Debounce**: 50ms minimum to prevent false triggers

### Audio Requirements
- Standard Morse code timing (PARIS standard)
- 600Hz tone frequency (standard CW sidetone)
- Adjustable WPM: 5-40 WPM via potentiometer
- PWM-generated square wave (passive buzzer)

## Dependencies

### Core Libraries (Arduino Framework)
- **Arduino.h**: Core ESP32 Arduino framework
- **Preferences.h**: ESP32 NVS (Non-Volatile Storage) wrapper
- **driver/ledc.h**: Low-level PWM control (LED Control peripheral)

### PlatformIO Configuration
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

### No External Libraries Required
- All functionality uses built-in ESP32 Arduino libraries
- Lightweight, minimal dependencies

## Development Setup

### Project Structure
```
practice_keyer/
├── platformio.ini          # Build configuration
├── src/
│   ├── main.cpp            # Main entry point
│   ├── keyer.cpp/h         # Iambic keyer logic
│   ├── recorder.cpp/h      # Recording system
│   ├── player.cpp/h        # Playback engine
│   ├── storage.cpp/h       # NVS persistence
│   └── hardware.cpp/h      # GPIO/PWM/ADC abstraction
├── include/
│   └── pins.h              # Pin definitions
└── .pio/                   # PlatformIO build artifacts (ignored)
```

## Build & Deployment

### Development Workflow
```bash
# Clean build
pio run --target clean

# Compile firmware
pio run

# Upload to ESP32
pio run --target upload

# Serial monitor
pio device monitor
```

### Firmware Upload
- **Method**: USB serial (UART0)
- **Protocol**: esptool.py automatic bootloader
- **Speed**: 921600 baud (upload), 115200 (monitor)
- **Partition**: Default ESP32 partition scheme

### OTA Updates (Future)
- Could add Over-The-Air updates via WiFi
- Not required for initial implementation

## Testing Strategy

### Hardware Testing
- **Paddle Input**: Test dit/dah detection and timing
- **Button Debouncing**: Verify no false triggers
- **ADC Reading**: Calibrate potentiometer response
- **PWM Output**: Verify buzzer tone quality
- **Timing Accuracy**: Measure with oscilloscope/logic analyzer

### Software Testing
- **State Machine**: Test all state transitions
- **Recording**: Verify element capture accuracy
- **Playback**: Confirm timing reproduction
- **NVS Storage**: Test save/load persistence
- **Edge Cases**: Empty slots, overflow, corrupt data

## Tool Usage Patterns

### Development Workflow
1. Edit code in VS Code with PlatformIO extension
2. Build with `pio run` (Ctrl+Alt+B)
3. Upload with `pio run --target upload` (Ctrl+Alt+U)
4. Monitor serial output (Ctrl+Alt+S)
5. Commit changes with Git

### Debugging
- **Serial Monitor**: `Serial.print()` debugging (115200 baud)
- **JTAG Debugger**: ESP-PROG or similar (advanced)
- **Logic Analyzer**: For timing analysis (optional)
- **Oscilloscope**: PWM waveform verification (optional)

## Version Control
- Git repository initialized
- Commit messages should be descriptive
- Branch strategy: main + feature branches
- Ignore `.pio/` build directory

## Known Technical Limitations
- **ADC Non-linearity**: ESP32 ADC not perfectly linear (use lookup table if needed)
- **PWM Resolution**: 8-bit adequate but not true sine wave
- **Boot Pin Restrictions**: GPIO 0,2,12,15 have boot mode functions
- **Flash Wear**: NVS flash has ~100,000 write cycle limit per sector
- **No Floating Point Hardware**: ESP32 has FPU, but stick to integer math where possible

## ESP32-Specific Notes
- **CPU**: Dual-core Xtensa LX6 @ 240MHz (overkill for this project)
- **Free RTOS**: Arduino abstracts this, but underlying OS is FreeRTOS
- **Watchdog Timer**: May need feeding in long operations
- **Deep Sleep**: Not used in this application (always-on keyer)
