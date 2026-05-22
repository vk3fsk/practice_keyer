/**
 * Practice Keyer - ESP32 Morse Code Memory Keyer
 * 
 * Features:
 * - Iambic paddle keyer with sidetone
 * - Variable speed control (5-40 WPM via potentiometer)
 * - 4 memory slots for recording/playback (100+ characters each)
 * - Playback at current pot speed setting
 * - Persistent NVS storage
 */

#include <Arduino.h>
#include <Preferences.h>
#include "pins.h"

// Morse element types
enum ElementType : uint8_t {
  ELEMENT_DIT = 0,
  ELEMENT_DAH = 1,
  ELEMENT_GAP = 2,
  ELEMENT_WORD_GAP = 3
};

// System states
enum SystemState {
  STATE_IDLE,
  STATE_RECORDING,
  STATE_SAVE_PROMPT,
  STATE_PLAYING
};

// Memory slot structure
struct MessageSlot {
  bool isEmpty;
  uint16_t elementCount;
  uint8_t elements[MAX_ELEMENTS];
};

// Global variables
MessageSlot slots[NUM_SLOTS];
Preferences preferences;
SystemState currentState = STATE_IDLE;
int currentWPM = 20;
unsigned long ditDuration = 60;  // milliseconds
int recordingSlot = -1;
int playbackSlot = -1;
unsigned long lastButtonPress = 0;
unsigned long savePromptStartTime = 0;
int lastUsedSlot = 0;

// Paddle state for iambic keyer
bool ditPressed = false;
bool dahPressed = false;
bool lastWasDit = false;
unsigned long elementStartTime = 0;
bool isSending = false;

// Playback state
unsigned int playbackIndex = 0;
unsigned long playbackElementStart = 0;
bool playbackToneOn = false;

// Function prototypes
void initHardware();
void initStorage();
void updateSpeed();
void handlePaddleInput();
void handleButtons();
void processRecording();
void processPlayback();
void toneOn();
void toneOff();
void ledOn();
void ledOff();
void ledBlink(int times);
void saveSlot(int slotNum);
void loadSlots();
bool isButtonPressed(int pin);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Practice Keyer Starting ===");
  
  initHardware();
  initStorage();
  loadSlots();
  
  Serial.println("System ready!");
}

void loop() {
  updateSpeed();
  handleButtons();
  
  switch (currentState) {
    case STATE_IDLE:
      handlePaddleInput();
      break;
      
    case STATE_RECORDING:
      handlePaddleInput();
      processRecording();
      break;
      
    case STATE_SAVE_PROMPT:
      // Wait for slot button press or timeout
      if (millis() - savePromptStartTime > 3000) {
        // Timeout - save to last used slot
        saveSlot(lastUsedSlot);
        currentState = STATE_IDLE;
        ledOff();
        Serial.println("Auto-saved to slot " + String(lastUsedSlot + 1));
      }
      break;
      
    case STATE_PLAYING:
      processPlayback();
      break;
  }
}

void initHardware() {
  // Configure paddle inputs
  pinMode(PIN_DIT_PADDLE, INPUT_PULLUP);
  pinMode(PIN_DAH_PADDLE, INPUT_PULLUP);
  
  // Configure button inputs
  pinMode(PIN_RECORD_BTN, INPUT_PULLUP);
  pinMode(PIN_SLOT1_BTN, INPUT_PULLUP);
  pinMode(PIN_SLOT2_BTN, INPUT_PULLUP);
  pinMode(PIN_SLOT3_BTN, INPUT_PULLUP);
  pinMode(PIN_SLOT4_BTN, INPUT_PULLUP);
  
  // Configure LED output
  pinMode(PIN_STATUS_LED, OUTPUT);
  ledOff();
  
  // Configure PWM for buzzer
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(PIN_BUZZER, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);  // Start silent
  
  Serial.println("Hardware initialized");
}

void initStorage() {
  preferences.begin("practice-keyer", false);
  
  // Initialize slots as empty
  for (int i = 0; i < NUM_SLOTS; i++) {
    slots[i].isEmpty = true;
    slots[i].elementCount = 0;
  }
  
  Serial.println("Storage initialized");
}

void loadSlots() {
  for (int i = 0; i < NUM_SLOTS; i++) {
    String key = "slot_" + String(i);
    size_t len = preferences.getBytesLength(key.c_str());
    
    if (len > 0) {
      uint8_t buffer[len];
      preferences.getBytes(key.c_str(), buffer, len);
      
      // First 2 bytes are element count (little-endian)
      uint16_t count = buffer[0] | (buffer[1] << 8);
      
      if (count > 0 && count <= MAX_ELEMENTS) {
        slots[i].isEmpty = false;
        slots[i].elementCount = count;
        memcpy(slots[i].elements, &buffer[2], count);
        Serial.println("Loaded slot " + String(i + 1) + ": " + String(count) + " elements");
      }
    }
  }
}

void saveSlot(int slotNum) {
  if (slotNum < 0 || slotNum >= NUM_SLOTS) return;
  
  String key = "slot_" + String(slotNum);
  uint16_t count = slots[slotNum].elementCount;
  
  // Create buffer: 2 bytes for count + element data
  uint8_t buffer[2 + count];
  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8) & 0xFF;
  memcpy(&buffer[2], slots[slotNum].elements, count);
  
  preferences.putBytes(key.c_str(), buffer, 2 + count);
  Serial.println("Saved slot " + String(slotNum + 1) + ": " + String(count) + " elements");
}

void updateSpeed() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 100) return;  // Update every 100ms
  lastUpdate = millis();
  
  int adcValue = analogRead(PIN_SPEED_POT);
  currentWPM = map(adcValue, 0, 4095, WPM_MIN, WPM_MAX);
  
  // Calculate dit duration: WPM = PARIS standard (50 dits per word)
  // Formula: dit_ms = 1200 / WPM
  ditDuration = 1200 / currentWPM;
}

bool isButtonPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void handleButtons() {
  unsigned long now = millis();
  if (now - lastButtonPress < DEBOUNCE_MS) return;
  
  // Record button
  if (isButtonPressed(PIN_RECORD_BTN)) {
    lastButtonPress = now;
    
    if (currentState == STATE_IDLE) {
      // Start recording
      recordingSlot = 0;  // Temporary slot
      slots[recordingSlot].isEmpty = false;
      slots[recordingSlot].elementCount = 0;
      currentState = STATE_RECORDING;
      ledOn();
      Serial.println("Recording started");
      delay(200);  // Prevent double-trigger
      
    } else if (currentState == STATE_RECORDING) {
      // Stop recording and prompt for slot
      currentState = STATE_SAVE_PROMPT;
      savePromptStartTime = now;
      ledBlink(2);
      Serial.println("Recording stopped - select slot (1-4)");
      delay(200);
    }
  }
  
  // Slot buttons
  for (int i = 0; i < NUM_SLOTS; i++) {
    int pin = (i == 0) ? PIN_SLOT1_BTN :
              (i == 1) ? PIN_SLOT2_BTN :
              (i == 2) ? PIN_SLOT3_BTN : PIN_SLOT4_BTN;
    
    if (isButtonPressed(pin)) {
      lastButtonPress = now;
      
      if (currentState == STATE_SAVE_PROMPT) {
        // Save recording to selected slot
        saveSlot(i);
        lastUsedSlot = i;
        currentState = STATE_IDLE;
        ledOff();
        Serial.println("Saved to slot " + String(i + 1));
        delay(200);
        
      } else if (currentState == STATE_IDLE) {
        // Play back slot
        if (!slots[i].isEmpty) {
          playbackSlot = i;
          playbackIndex = 0;
          playbackElementStart = now;
          playbackToneOn = false;
          currentState = STATE_PLAYING;
          Serial.println("Playing slot " + String(i + 1));
        } else {
          // Empty slot - blink LED
          ledBlink(2);
          Serial.println("Slot " + String(i + 1) + " is empty");
        }
        delay(200);
        
      } else if (currentState == STATE_PLAYING && playbackSlot == i) {
        // Stop playback
        currentState = STATE_IDLE;
        toneOff();
        Serial.println("Playback stopped");
        delay(200);
      }
    }
  }
}

void handlePaddleInput() {
  ditPressed = isButtonPressed(PIN_DIT_PADDLE);
  dahPressed = isButtonPressed(PIN_DAH_PADDLE);
  
  unsigned long now = millis();
  
  if (!isSending) {
    // Not currently sending - check for new input
    if (ditPressed || dahPressed) {
      isSending = true;
      elementStartTime = now;
      toneOn();
      
      if (ditPressed) {
        lastWasDit = true;
        if (currentState == STATE_RECORDING) {
          if (slots[recordingSlot].elementCount < MAX_ELEMENTS) {
            slots[recordingSlot].elements[slots[recordingSlot].elementCount++] = ELEMENT_DIT;
          }
        }
      } else {
        lastWasDit = false;
        if (currentState == STATE_RECORDING) {
          if (slots[recordingSlot].elementCount < MAX_ELEMENTS) {
            slots[recordingSlot].elements[slots[recordingSlot].elementCount++] = ELEMENT_DAH;
          }
        }
      }
    }
  } else {
    // Currently sending - check for timing
    unsigned long elapsed = now - elementStartTime;
    unsigned long targetDuration = lastWasDit ? ditDuration : ditDuration * 3;
    
    if (elapsed >= targetDuration) {
      // Element complete
      toneOff();
      isSending = false;
      
      // Add inter-element gap
      if (currentState == STATE_RECORDING) {
        if (slots[recordingSlot].elementCount < MAX_ELEMENTS) {
          slots[recordingSlot].elements[slots[recordingSlot].elementCount++] = ELEMENT_GAP;
        }
      }
      
      delay(ditDuration);  // Inter-element space
    }
  }
}

void processRecording() {
  // Check for overflow
  if (slots[recordingSlot].elementCount >= MAX_ELEMENTS) {
    Serial.println("Recording buffer full!");
    ledBlink(3);
    currentState = STATE_SAVE_PROMPT;
    savePromptStartTime = millis();
  }
}

void processPlayback() {
  unsigned long now = millis();
  
  if (playbackIndex >= slots[playbackSlot].elementCount) {
    // Playback complete
    currentState = STATE_IDLE;
    toneOff();
    Serial.println("Playback complete");
    return;
  }
  
  uint8_t element = slots[playbackSlot].elements[playbackIndex];
  unsigned long elementDuration;
  
  switch (element) {
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
      elementDuration = ditDuration * 3;
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
      
    case ELEMENT_GAP:
      // Inter-element gap (1 dit duration)
      if (now - playbackElementStart >= ditDuration) {
        playbackIndex++;
        playbackElementStart = now;
      }
      break;
      
    case ELEMENT_WORD_GAP:
      // Word gap (7 dit durations)
      if (now - playbackElementStart >= ditDuration * 7) {
        playbackIndex++;
        playbackElementStart = now;
      }
      break;
  }
}

void toneOn() {
  ledcWrite(PWM_CHANNEL, 128);  // 50% duty cycle
}

void toneOff() {
  ledcWrite(PWM_CHANNEL, 0);
}

void ledOn() {
  digitalWrite(PIN_STATUS_LED, HIGH);
}

void ledOff() {
  digitalWrite(PIN_STATUS_LED, LOW);
}

void ledBlink(int times) {
  for (int i = 0; i < times; i++) {
    ledOn();
    delay(100);
    ledOff();
    delay(100);
  }
}
