# System Patterns

## Architecture Overview
ESP32-based embedded firmware for a hardware Morse code memory keyer.

### High-Level Architecture
```
┌─────────────────────────────────────────────────────────┐
│                     ESP32 Firmware                       │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌────────────┐  ┌──────────────┐  ┌─────────────┐    │
│  │  Hardware  │  │    Keyer     │  │  Recording  │    │
│  │   Layer    │  │    Engine    │  │   System    │    │
│  └─────┬──────┘  └──────┬───────┘  └──────┬──────┘    │
│        │                 │                  │            │
│  ┌─────┴─────────────────┴──────────────────┴──────┐   │
│  │           State Machine Controller              │   │
│  └─────────────────────┬────────────────────────────┘   │
│                        │                                 │
│  ┌─────────────────────┴────────────────────────────┐   │
│  │         Storage Manager (NVS Flash)              │   │
│  └──────────────────────────────────────────────────┘   │
│                                                          │
└─────────────────────────────────────────────────────────┘
         │                    │                   │
    ┌────┴────┐          ┌───┴───┐          ┌────┴─────┐
    │ Buttons │          │Paddle │          │  Buzzer  │
    │   LED   │          │ Inputs│          │   (PWM)  │
    │   Pot   │          │       │          │          │
    └─────────┘          └───────┘          └──────────┘
```

## Key Technical Decisions

### Finalized Decisions
1. **Platform**: ESP32 microcontroller
2. **Framework**: Arduino framework via PlatformIO
3. **Language**: C++
4. **Audio**: PWM tone generation (600Hz buzzer)
5. **Storage**: ESP32 NVS (Non-Volatile Storage) in flash
6. **Memory Architecture**: Normalized element storage (dit/dah/gap)
7. **Keyer Mode**: Iambic paddle support

## Design Patterns in Use

### Core Patterns
- **State Machine**: Main control flow (IDLE → RECORDING → PLAYING)
- **Hardware Abstraction**: Pin definitions and I/O wrappers
- **Singleton**: Audio/PWM manager, Storage manager
- **Circular Buffer**: For input debouncing
- **Strategy Pattern**: Iambic keyer modes (A/B)

## Component Relationships

```
Hardware Layer:
  GPIO Inputs → Debounce → Input Handler
  ADC (Pot) → Speed Calculator → Timing Engine
  
Keyer Engine:
  Paddle Inputs → Iambic Logic → Element Generator → Buzzer (PWM)
  
Recording:
  Element Generator → Buffer → Slot Storage (RAM) → NVS Flash
  
Playback:
  NVS Flash → Slot Buffer → Speed Scaler → Element Player → Buzzer
  
State Control:
  Button Events → State Machine → Mode Handler
```

## Critical Implementation Paths

### Core Functionality Path
1. **Hardware Init**: GPIO setup, PWM init, ADC config
2. **Input Processing**: Paddle debouncing, button detection
3. **Keyer Logic**: Iambic timing, element generation
4. **Audio Output**: PWM tone generation (600Hz)
5. **Recording**: Element capture, normalization, storage
6. **Playback**: Slot retrieval, speed scaling, reproduction
7. **Persistence**: NVS save/load on demand and boot

### Data Flow
```
KEYING:
  Paddle → Debounce → Iambic Engine → Element Type → 
  → PWM Buzzer + Recording Buffer (if recording)

RECORDING:
  Elements → Normalize → RAM Buffer → (on save) → NVS Flash

PLAYBACK:
  NVS Flash → Load Slot → Scale to Current Speed → 
  → Timeline Player → PWM Buzzer
  
SPEED CONTROL:
  Pot → ADC Read → WPM Calc (5-40) → Dit Duration → All Timing
```

## Technical Constraints
- Real-time timing requirements (1ms precision for CW)
- Limited RAM: 520KB available, using ~2KB for slots
- Flash wear: Limit NVS writes to necessary saves only
- GPIO limitations: Avoid boot-sensitive pins (0,2,12,15)
- PWM frequency range: 50Hz - 40kHz (buzzer optimized)
- ADC accuracy: 12-bit (0-4095), needs smoothing

## Code Organization Principles

### Project Structure
```
practice_keyer/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main program loop
│   ├── keyer.cpp/h         # Iambic keyer engine
│   ├── recorder.cpp/h      # Recording system
│   ├── player.cpp/h        # Playback system
│   ├── storage.cpp/h       # NVS persistence
│   └── hardware.cpp/h      # GPIO/PWM abstractions
└── include/
    └── pins.h              # Pin definitions
```

### Coding Principles
- Hardware abstraction for testability
- State machines for control flow
- Minimal dynamic allocation (stack-based)
- Interrupt-safe buffer access
- Clear pin naming constants

## Performance Considerations
- **Timing Precision**: Use `micros()` for Morse timing (1µs resolution)
- **PWM Frequency**: 600Hz tone, 8-bit resolution adequate
- **ADC Sampling**: Read pot every 100ms (smoothed average)
- **Debouncing**: 50ms software debounce for buttons/paddle
- **Memory**: Static allocation, no heap fragmentation
- **Flash Writes**: Batch NVS writes, minimize wear cycles

## Security & Privacy
- All data stored locally in ESP32 NVS flash
- No network connectivity required
- No personal data collection
- Recordings stay on device unless manually extracted

## Hardware Requirements
- ESP32 development board (DevKit, WROOM, etc.)
- 10kΩ potentiometer for speed control
- 2× pushbuttons for paddle (or iambic paddle)
- 5× pushbuttons (RECORD + 4 SLOTs)
- 1× passive buzzer (requires PWM)
- 1× LED for status indication
- Pull-up resistors or use internal pull-ups

---

**GPIO Pin Assignments:**
| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| DIT Paddle | 34 | Input | Pull-up |
| DAH Paddle | 35 | Input | Pull-up |
| Speed Pot | 36 | ADC | Analog input |
| RECORD Btn | 32 | Input | Pull-up |
| SLOT 1 Btn | 33 | Input | Pull-up |
| SLOT 2 Btn | 25 | Input | Pull-up |
| SLOT 3 Btn | 26 | Input | Pull-up |
| SLOT 4 Btn | 27 | Input | Pull-up |
| Buzzer | 23 | PWM Output | 600Hz tone |
| Status LED | 22 | Digital Out | Recording indicator |
