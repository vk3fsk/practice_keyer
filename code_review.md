# Code Review: main.cpp - Maintainability & Best Practices

**Project:** Practice Keyer - ESP32 Morse Code Memory Keyer  
**File Reviewed:** `src/main.cpp` (431 lines)  
**Review Date:** May 22, 2026  
**Reviewer:** Cline (AI Code Analyst)

---

## Executive Summary

The code is **functional and demonstrates good basic practices**, but has several maintainability and real-time operation issues that should be addressed before hardware testing. The main concerns are blocking delays that hurt responsiveness, monolithic structure that limits scalability, extensive global state, and minimal error handling.

**Overall Maintainability Score: 6/10**

---

## ✅ Strengths

### Good Practices Found:

1. **Clear file header** with project description and features (lines 1-10)
2. **Organized structure** with enums, structs, function prototypes, and setup/loop pattern
3. **Good use of enums** for element types (`ElementType`) and system states (`SystemState`)
4. **Function prototypes** declared upfront (lines 64-78) for readability
5. **Serial debugging** throughout for troubleshooting and development
6. **Reasonable separation** of concerns with dedicated functions for hardware, storage, and input handling
7. **Consistent naming** conventions (camelCase for variables/functions)
8. **State machine pattern** for managing system states (IDLE/RECORDING/SAVE_PROMPT/PLAYING)

---

## 🔴 Critical Issues

### 1. **Blocking `delay()` Calls in Main Loop**
**Severity: HIGH** - Breaks real-time responsiveness

**Problem Locations:**
```cpp
delay(200);  // Lines 229, 237, 257, 273, 280
delay(ditDuration);  // Line 332
```

**Why This Is Bad:**
- Blocks ALL processing during delay (buttons unresponsive)
- Ruins timing precision for a real-time Morse keyer
- Prevents concurrent operations (can't check for stop during delay)
- User experience degraded (sluggish button response)

**Impact:** Critical for a timing-sensitive embedded device. This is the #1 issue to fix.

**Solution:**
Replace blocking delays with non-blocking timestamp checks using `millis()` and state variables.

**Example Fix:**
```cpp
// Current (BAD):
if (isButtonPressed(PIN_RECORD_BTN)) {
  // ... do stuff ...
  delay(200);  // BLOCKS EVERYTHING
}

// Improved (GOOD):
struct ButtonState {
  unsigned long lastPressTime;
  bool wasPressed;
};

ButtonState recordButton = {0, false};

void handleRecordButton() {
  bool pressed = isButtonPressed(PIN_RECORD_BTN);
  unsigned long now = millis();
  
  if (pressed && !recordButton.wasPressed && 
      (now - recordButton.lastPressTime > DEBOUNCE_MS)) {
    // Button just pressed - take action
    recordButton.lastPressTime = now;
    // ... do stuff ...
  }
  recordButton.wasPressed = pressed;
}
```

---

### 2. **Magic Numbers Everywhere**
**Severity: MEDIUM** - Reduces maintainability and readability

**Problem Examples:**
```cpp
if (millis() - savePromptStartTime > 3000)  // Line 107 - What is 3000?
if (millis() - lastUpdate < 100)  // Line 198 - Why 100?
ledcWrite(PWM_CHANNEL, 128);  // Line 409 - What does 128 mean?
delay(100);  // Lines 427, 428 - Magic timing values
ditDuration * 3;  // Line 318, 377 - DAH is 3× DIT (document this!)
ditDuration * 7;  // Line 400 - Word gap is 7× DIT
```

**Why This Is Bad:**
- Intent unclear ("what does 3000 mean?")
- Hard to maintain (need to search entire file to change values)
- Easy to introduce bugs when copying code
- No central definition

**Solution:**
Extract all magic numbers to named constants at the top of the file:

```cpp
// Timing constants
constexpr unsigned long SAVE_PROMPT_TIMEOUT_MS = 3000;
constexpr unsigned long SPEED_UPDATE_INTERVAL_MS = 100;
constexpr unsigned long LED_BLINK_DURATION_MS = 100;
constexpr unsigned long BUTTON_DEBOUNCE_DELAY_MS = 200;

// Morse timing ratios (PARIS standard)
constexpr unsigned int DAH_DIT_RATIO = 3;      // DAH is 3× DIT duration
constexpr unsigned int WORD_GAP_DIT_RATIO = 7; // Word gap is 7× DIT

// PWM constants
constexpr uint8_t PWM_DUTY_CYCLE_50_PERCENT = 128;  // 50% of 256 (8-bit)
constexpr uint8_t PWM_DUTY_CYCLE_OFF = 0;
```

---

### 3. **All Code in One File**
**Severity: MEDIUM** - Poor maintainability at scale

**Problem:**
431 lines in `main.cpp` mixing concerns:
- Hardware abstraction (GPIO, PWM, ADC)
- State machine logic
- Storage management (NVS)
- Keyer timing algorithms
- Recording system
- Playback engine

**Why This Is Bad:**
- Hard to navigate (scroll through entire file to find anything)
- Testing difficult (can't unit test modules independently)
- Multiple developers would cause merge conflicts
- Violates Single Responsibility Principle
- Doesn't match documented architecture in `systemPatterns.md`

**Solution:**
Split into modules as documented in Memory Bank:

```
src/
├── main.cpp            # Main program loop, state machine coordination
├── keyer.cpp/h         # Iambic keyer engine
├── recorder.cpp/h      # Recording system
├── player.cpp/h        # Playback system
├── storage.cpp/h       # NVS persistence
└── hardware.cpp/h      # GPIO/PWM/ADC abstractions

include/
├── pins.h              # Pin definitions (already exists)
└── config.h            # System constants and configuration
```

**Benefits:**
- Each module ~100-150 lines (manageable)
- Clear responsibilities
- Independent testing
- Better reusability
- Easier to understand

---

### 4. **Global Variable Soup**
**Severity: MEDIUM** - Namespace pollution, hard to track state

**Problem:**
15+ global variables scattered throughout (lines 40-61):

```cpp
MessageSlot slots[NUM_SLOTS];
Preferences preferences;
SystemState currentState = STATE_IDLE;
int currentWPM = 20;
unsigned long ditDuration = 60;
int recordingSlot = -1;
int playbackSlot = -1;
unsigned long lastButtonPress = 0;
unsigned long savePromptStartTime = 0;
int lastUsedSlot = 0;
bool ditPressed = false;
bool dahPressed = false;
bool lastWasDit = false;
unsigned long elementStartTime = 0;
bool isSending = false;
unsigned int playbackIndex = 0;
unsigned long playbackElementStart = 0;
bool playbackToneOn = false;
```

**Why This Is Bad:**
- Any function can modify any global (debugging nightmare)
- No clear ownership or lifecycle
- Name collisions possible
- Hard to track dependencies
- Difficult to pass state to helper functions
- Can't easily reset subsystems

**Solution:**
Encapsulate related variables into logical structs:

```cpp
// Keyer state
struct KeyerState {
  bool ditPressed;
  bool dahPressed;
  bool lastWasDit;
  unsigned long elementStartTime;
  bool isSending;
  int currentWPM;
  unsigned long ditDuration;
};

// Recording state
struct RecordingState {
  int slot;
  unsigned long savePromptStartTime;
  int lastUsedSlot;
};

// Playback state
struct PlaybackState {
  int slot;
  unsigned int index;
  unsigned long elementStart;
  bool toneOn;
};

// System state
struct SystemContext {
  SystemState state;
  KeyerState keyer;
  RecordingState recording;
  PlaybackState playback;
  MessageSlot slots[NUM_SLOTS];
  unsigned long lastButtonPress;
};

SystemContext sys;  // Single global context
```

**Benefits:**
- Clear grouping of related data
- Easy to pass entire context to functions
- Can reset subsystems by zeroing struct
- Better IDE autocomplete
- Self-documenting relationships

---

### 5. **Insufficient Debouncing**
**Severity: MEDIUM** - Can cause false triggers

**Problem Location:**
```cpp
void handleButtons() {
  unsigned long now = millis();
  if (now - lastButtonPress < DEBOUNCE_MS) return;  // Line 215
  
  // Single global lastButtonPress for ALL buttons!
```

**Why This Is Bad:**
- Single global `lastButtonPress` timestamp for ALL buttons
- Pressing one button debounces **all other buttons**
- If you press SLOT1, you can't press SLOT2 for DEBOUNCE_MS
- Not true debouncing (doesn't track individual button state changes)

**Example Bug:**
1. User presses RECORD button
2. System sets `lastButtonPress = millis()`
3. User immediately presses SLOT1 (within debounce window)
4. SLOT1 press ignored because global debounce active
5. Confused user!

**Solution:**
Individual debounce tracking per button:

```cpp
struct ButtonDebounce {
  unsigned long lastChangeTime;
  bool lastState;
  bool currentState;
};

ButtonDebounce buttons[6];  // RECORD + 4 SLOTs + (paddle if needed)

bool readDebouncedButton(int buttonIndex, int pin) {
  bool rawState = (digitalRead(pin) == LOW);
  unsigned long now = millis();
  
  if (rawState != buttons[buttonIndex].lastState) {
    buttons[buttonIndex].lastChangeTime = now;
  }
  
  if ((now - buttons[buttonIndex].lastChangeTime) > DEBOUNCE_MS) {
    buttons[buttonIndex].currentState = rawState;
  }
  
  buttons[buttonIndex].lastState = rawState;
  return buttons[buttonIndex].currentState;
}
```

---

### 6. **Code Duplication in Playback**
**Severity: LOW** - DRY principle violation

**Problem Location:**
Lines 362-388 have nearly identical code for DIT and DAH cases:

```cpp
case ELEMENT_DIT:
  elementDuration = ditDuration;
  if (!playbackToneOn) {
    toneOn();
    playbackToneOn = true;
    playbackElementStart = now;
  } else if (now - playbackElementStart >= elementDuration) {
    toneOff();
    playbackToneOn = false;
    playbackIndex++;
    playbackElementStart = now;
  }
  break;
  
case ELEMENT_DAH:
  elementDuration = ditDuration * 3;  // Only difference!
  if (!playbackToneOn) {             // Duplicate code
    toneOn();
    playbackToneOn = true;
    playbackElementStart = now;
  } else if (now - playbackElementStart >= elementDuration) {
    toneOff();
    playbackToneOn = false;
    playbackIndex++;
    playbackElementStart = now;
  }
  break;
```

**Solution:**
Extract common playback logic:

```cpp
void processPlaybackElement(unsigned long duration) {
  unsigned long now = millis();
  
  if (!playbackToneOn) {
    toneOn();
    playbackToneOn = true;
    playbackElementStart = now;
  } else if (now - playbackElementStart >= duration) {
    toneOff();
    playbackToneOn = false;
    playbackIndex++;
    playbackElementStart = now;
  }
}

// In processPlayback():
case ELEMENT_DIT:
  processPlaybackElement(ditDuration);
  break;
case ELEMENT_DAH:
  processPlaybackElement(ditDuration * DAH_DIT_RATIO);
  break;
```

---

### 7. **Rudimentary Iambic Keyer**
**Severity: LOW** - Feature completeness issue

**Current Implementation:**
Basic paddle detection without true iambic features (lines 286-335).

**Missing Features:**
- **Squeeze keying**: When both paddles pressed, alternates dit-dah-dit-dah
- **Iambic Mode A vs B**: Different memory behavior
- **Paddle memory**: Remembers paddle pressed during current element
- **Curtis keyer modes**: Industry-standard keyer algorithms

**Current Code:**
```cpp
if (ditPressed || dahPressed) {
  // Takes first pressed, ignores second
  if (ditPressed) {
    lastWasDit = true;
    // ...
  } else {
    lastWasDit = false;
    // ...
  }
}
```

**What a True Iambic Keyer Does:**
- Detects both paddles during element sending
- Automatically alternates on release
- Implements Curtis mode A or B timing

**Note:** This is acceptable for MVP testing, but should be enhanced for a production "iambic" keyer. The current implementation is more of a "straight key with dit/dah buttons."

---

### 8. **Missing Error Handling**
**Severity: MEDIUM** - Robustness issue

**Problem Locations:**

```cpp
// Line 147: No check for NVS initialization failure
preferences.begin("practice-keyer", false);

// Line 192: No verification write succeeded
preferences.putBytes(key.c_str(), buffer, 2 + count);

// Line 165: No bounds checking before memcpy
preferences.getBytes(key.c_str(), buffer, len);
memcpy(slots[i].elements, &buffer[2], count);

// No flash full detection
// No corrupt data recovery
```

**Potential Issues:**
- NVS partition not initialized → silent failure
- Flash full → write fails, user loses data
- Corrupt data in NVS → memcpy garbage, system crash
- Buffer overflow if `len` exceeds array size

**Solution:**
Add defensive checks:

```cpp
void initStorage() {
  if (!preferences.begin("practice-keyer", false)) {
    Serial.println("ERROR: NVS initialization failed!");
    ledBlink(5);  // Error indicator
    // Could set error state, use defaults, etc.
  }
  // ...
}

void saveSlot(int slotNum) {
  if (slotNum < 0 || slotNum >= NUM_SLOTS) {
    Serial.println("ERROR: Invalid slot number");
    return;
  }
  
  String key = "slot_" + String(slotNum);
  uint16_t count = slots[slotNum].elementCount;
  
  if (count > MAX_ELEMENTS) {
    Serial.println("ERROR: Element count exceeds MAX_ELEMENTS");
    return;
  }
  
  uint8_t buffer[2 + count];
  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8) & 0xFF;
  memcpy(&buffer[2], slots[slotNum].elements, count);
  
  size_t written = preferences.putBytes(key.c_str(), buffer, 2 + count);
  if (written != 2 + count) {
    Serial.println("ERROR: NVS write failed!");
    ledBlink(3);  // Error indicator
  } else {
    Serial.println("Saved slot " + String(slotNum + 1));
  }
}

void loadSlots() {
  for (int i = 0; i < NUM_SLOTS; i++) {
    String key = "slot_" + String(i);
    size_t len = preferences.getBytesLength(key.c_str());
    
    if (len > 0 && len <= (2 + MAX_ELEMENTS)) {  // Sanity check
      uint8_t buffer[len];
      preferences.getBytes(key.c_str(), buffer, len);
      
      uint16_t count = buffer[0] | (buffer[1] << 8);
      
      // Validate count before memcpy
      if (count > 0 && count <= MAX_ELEMENTS && count <= (len - 2)) {
        slots[i].isEmpty = false;
        slots[i].elementCount = count;
        memcpy(slots[i].elements, &buffer[2], count);
        Serial.println("Loaded slot " + String(i + 1));
      } else {
        Serial.println("WARNING: Corrupt data in slot " + String(i + 1));
        slots[i].isEmpty = true;
      }
    }
  }
}
```

---

### 9. **Hardcoded Pin Mapping in Button Handler**
**Severity: LOW** - Code smell, brittle

**Problem Location:**
Lines 243-245:
```cpp
int pin = (i == 0) ? PIN_SLOT1_BTN :
          (i == 1) ? PIN_SLOT2_BTN :
          (i == 2) ? PIN_SLOT3_BTN : PIN_SLOT4_BTN;
```

**Why This Is Bad:**
- Nested ternary operators (hard to read)
- If slots increase to 5, need to modify this line
- Brittle (easy to make off-by-one error)
- No compile-time checking

**Solution:**
Use array lookup (defined near top of file):

```cpp
const int SLOT_BUTTON_PINS[NUM_SLOTS] = {
  PIN_SLOT1_BTN,
  PIN_SLOT2_BTN,
  PIN_SLOT3_BTN,
  PIN_SLOT4_BTN
};

// In handleButtons():
for (int i = 0; i < NUM_SLOTS; i++) {
  int pin = SLOT_BUTTON_PINS[i];  // Simple, clear, maintainable
  // ...
}
```

**Benefits:**
- Declarative (intent clear)
- Easy to add more slots
- No magic indices
- Compile-time size checking

---

### 10. **No Watchdog Handling**
**Severity: LOW** - Could cause unexpected resets

**Problem:**
ESP32 has watchdog timers that reset the system if not fed regularly. Long operations or infinite loops can trigger watchdog reset.

**Current Code:**
No `yield()`, `delay(0)`, or explicit watchdog feeding.

**Potential Issues:**
- If `loop()` takes too long, watchdog triggers
- Blocking operations in ISRs (if added later)
- User sees random reboots

**Solution:**
Add periodic watchdog feeding:

```cpp
void loop() {
  updateSpeed();
  handleButtons();
  
  // Feed watchdog periodically
  yield();  // Or: esp_task_wdt_reset();
  
  switch (currentState) {
    // ...
  }
}
```

---

## 📊 Detailed Maintainability Scoring

| Category | Score | Notes |
|----------|-------|-------|
| **Organization** | 7/10 | Good basic structure (enums, prototypes, setup/loop), but monolithic file limits scalability |
| **Readability** | 7/10 | Clear function names and comments, but too many globals and magic numbers reduce clarity |
| **Modularity** | 4/10 | Everything in one file, no separation of concerns, hard to test independently |
| **Error Handling** | 3/10 | Minimal defensive programming, no NVS failure checks, no bounds validation |
| **Real-time Safety** | 5/10 | Blocking delays problematic, but core timing logic reasonable |
| **Testability** | 4/10 | Hardware coupling makes unit tests hard, no dependency injection |
| **Documentation** | 6/10 | Good file header, but missing inline comments for complex logic (timing calculations) |
| **Performance** | 7/10 | Reasonable efficiency, but could optimize ADC reads and reduce String allocations |

**Overall: 6/10** - Functional prototype with room for improvement before production use.

---

## 🎯 Recommended Improvements (Priority Order)

### **High Priority** (Fix before hardware testing):

1. ✅ **Remove all blocking `delay()` calls**
   - Replace with non-blocking timestamp checks
   - Use state machines for button debouncing
   - Critical for responsive user experience

2. ✅ **Fix debouncing logic**
   - Per-button debounce tracking
   - Prevents one button blocking others
   - Improves reliability

3. ✅ **Extract magic numbers to named constants**
   - Define all timing values at top
   - Document Morse timing ratios
   - Improves maintainability

4. ✅ **Add error handling for NVS operations**
   - Check return values
   - Validate data before memcpy
   - Handle flash full condition
   - Improves robustness

### **Medium Priority** (Refactor for maintainability):

5. ✅ **Modularize code into separate files**
   - Split into keyer/recorder/player/storage modules
   - Each file ~100-150 lines
   - Matches documented architecture
   - Easier to maintain and test

6. ✅ **Encapsulate state in structs**
   - Group related globals
   - Pass context to functions
   - Reduces namespace pollution
   - Better organization

7. ✅ **Reduce code duplication**
   - Extract common playback logic
   - DRY principle
   - Easier to maintain

### **Low Priority** (Nice to have):

8. ✅ **Use pin arrays instead of nested ternary**
   - Cleaner code
   - Easier to extend
   - Minor improvement

9. ✅ **Add inline comments for complex logic**
   - Document timing calculations
   - Explain state transitions
   - Better understanding

10. ✅ **Implement true iambic keyer modes**
    - Add squeeze keying
    - Curtis mode A/B
    - Paddle memory
    - Feature completeness

---

## 💡 Code Examples: Before & After

### Example 1: Non-blocking Delays

**Before (blocking):**
```cpp
void handleButtons() {
  if (isButtonPressed(PIN_RECORD_BTN)) {
    // ... do stuff ...
    delay(200);  // ❌ BLOCKS EVERYTHING
  }
}
```

**After (non-blocking):**
```cpp
struct ButtonState {
  unsigned long lastPressTime;
  bool wasPressed;
};

ButtonState recordBtn = {0, false};

void handleRecordButton() {
  bool pressed = isButtonPressed(PIN_RECORD_BTN);
  unsigned long now = millis();
  
  // Detect button press edge
  if (pressed && !recordBtn.wasPressed) {
    if (now - recordBtn.lastPressTime > DEBOUNCE_MS) {
      recordBtn.lastPressTime = now;
      // ... do stuff ...
    }
  }
  
  recordBtn.wasPressed = pressed;
}
```

### Example 2: Extract Magic Numbers

**Before:**
```cpp
if (millis() - savePromptStartTime > 3000)  // ❌ What is 3000?
  ditDuration * 3;  // ❌ Why 3?
  ledcWrite(PWM_CHANNEL, 128);  // ❌ What does 128 mean?
```

**After:**
```cpp
constexpr unsigned long SAVE_PROMPT_TIMEOUT_MS = 3000;
constexpr unsigned int DAH_DIT_RATIO = 3;
constexpr uint8_t PWM_DUTY_CYCLE_50_PERCENT = 128;

if (millis() - savePromptStartTime > SAVE_PROMPT_TIMEOUT_MS)  // ✅ Clear!
  ditDuration * DAH_DIT_RATIO;  // ✅ Self-documenting
  ledcWrite(PWM_CHANNEL, PWM_DUTY_CYCLE_50_PERCENT);  // ✅ Intent clear
```

### Example 3: Encapsulate Globals

**Before:**
```cpp
// ❌ 15+ globals scattered everywhere
int currentWPM = 20;
unsigned long ditDuration = 60;
bool ditPressed = false;
bool dahPressed = false;
bool lastWasDit = false;
unsigned long elementStartTime = 0;
bool isSending = false;
// ... many more
```

**After:**
```cpp
// ✅ Organized into logical groups
struct KeyerState {
  bool ditPressed;
  bool dahPressed;
  bool lastWasDit;
  unsigned long elementStartTime;
  bool isSending;
  int currentWPM;
  unsigned long ditDuration;
};

struct SystemContext {
  SystemState state;
  KeyerState keyer;
  RecordingState recording;
  PlaybackState playback;
  MessageSlot slots[NUM_SLOTS];
};

SystemContext sys;  // Single global context
```

---

## 📁 Suggested File Structure

If refactoring to separate modules:

```
practice_keyer/
├── platformio.ini
├── include/
│   ├── pins.h                # GPIO pin definitions (exists)
│   ├── config.h              # System constants (NEW)
│   ├── types.h               # Common types/enums (NEW)
│   ├── hardware.h            # Hardware abstraction (NEW)
│   ├── keyer.h               # Keyer engine (NEW)
│   ├── recorder.h            # Recording (NEW)
│   ├── player.h              # Playback (NEW)
│   └── storage.h             # NVS persistence (NEW)
└── src/
    ├── main.cpp              # Main loop, state coordination
    ├── hardware.cpp          # GPIO/PWM/ADC implementation
    ├── keyer.cpp             # Iambic keyer logic
    ├── recorder.cpp          # Recording system
    ├── player.cpp            # Playback engine
    └── storage.cpp           # NVS read/write
```

**Benefits:**
- Each module ~100-150 lines (manageable)
- Clear responsibilities
- Can unit test modules independently
- Better IDE navigation
- Matches architecture documentation

---

## 🔍 Additional Observations

### Performance Considerations:

1. **String concatenation in loops** (minor):
   ```cpp
   String key = "slot_" + String(i);  // Creates temporary objects
   Serial.println("Saved to slot " + String(i + 1));
   ```
   
   Consider using `snprintf()` for better performance:
   ```cpp
   char key[16];
   snprintf(key, sizeof(key), "slot_%d", i);
   ```

2. **ADC reads every 100ms** (good):
   Current throttling is appropriate. ESP32 ADC is noisy, could add smoothing filter.

3. **No sleep modes** (acceptable):
   Always-on keyer doesn't need deep sleep, but could reduce power if needed.

### Security/Safety:

- No buffer overflow protection on element array
- No rate limiting on button presses (unlikely to be an issue)
- NVS namespace collision possible with other apps (unlikely)

### Future Enhancements:

- Add UART commands for remote control
- Implement beacon mode (auto-repeat)
- Add adjustable sidetone frequency
- Multiple keyer personalities (bug vs straight key)
- Text-to-Morse conversion

---

## ✅ Testing Recommendations

Before deploying to hardware:

1. **Unit Tests** (if modularized):
   - Test timing calculations (WPM → dit duration)
   - Test element encoding/decoding
   - Test slot save/load logic
   - Mock hardware for pure logic testing

2. **Integration Tests**:
   - Test all button combinations
   - Test state transitions
   - Test edge cases (buffer full, empty slots)
   - Test NVS persistence across reboots

3. **Hardware Tests**:
   - Verify paddle timing accuracy with oscilloscope
   - Test button debouncing reliability
   - Verify PWM buzzer frequency (should be 600Hz)
   - Test ADC potentiometer linearity
   - Long-term reliability testing (hours of operation)

4. **User Acceptance Tests**:
   - Record and playback test messages
   - Verify speed changes work as expected
   - Test all LED feedback patterns
   - Ensure responsiveness during all operations

---

## 🎓 Learning Resources

For improving embedded best practices:

- **Real-time patterns**: [FreeRTOS patterns](https://www.freertos.org/a00125.html)
- **State machines**: [Embedded State Machine Design](https://barrgroup.com/embedded-systems/how-to/state-machines)
- **Debouncing**: [Elliot Williams Debouncing Guide](https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/)
- **Morse keyer algorithms**: [Curtis Keyer (K5BCQ)](http://www.k5bcq.com/cw.html)

---

## 📝 Summary

### What Works Well:
- ✅ Core functionality is sound
- ✅ Good basic structure and organization
- ✅ State machine pattern appropriate
- ✅ Serial debugging helpful

### What Needs Improvement:
- ❌ Blocking delays (critical issue)
- ❌ Global state management
- ❌ Monolithic file structure
- ❌ Missing error handling
- ❌ Magic numbers throughout

### Recommended Actions:
1. **Immediate**: Remove blocking delays, fix debouncing, add constants
2. **Short-term**: Add error handling, improve keyer logic
3. **Long-term**: Modularize codebase, add unit tests

**Overall Assessment:** Solid foundation with clear improvement path. Code demonstrates understanding of embedded concepts but needs real-time refinement before hardware testing.

---

**End of Review**
