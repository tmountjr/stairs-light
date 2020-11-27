// Touch sensing variables
const int THRESHOLD = 30;
const long ACTIVATE_FOR = 5000;
const long DEBOUNCE = 500;
long activatedAt = -1;
long deactivatedAt = -1;

// LED state machine
const byte LED_STATE_OFF = 0;
const byte LED_STATE_ON = 1;
byte ledState = LED_STATE_OFF;
const int LED_LEVEL_ON = 255;
const int LED_LEVEL_OFF = 0;

// Pins
const int LED_PIN = 16;
const int TOUCH_PIN = T2;

// Turn off the LED, track state, and reset timer.
void turnOff() {
  digitalWrite(LED_PIN, LED_LEVEL_OFF);
  ledState = LED_STATE_OFF;
  activatedAt = -1;
  deactivatedAt = millis();
}

// Turn on the LED, track state, and update timer.
void turnOn() {
  digitalWrite(LED_PIN, LED_LEVEL_ON);
  ledState = LED_STATE_ON;
  activatedAt = millis();
  deactivatedAt = -1;
}

// Generic event handler that allows a second touch event to turn the LED off and
// stop the timer.
void handleTouchEvent() {
  long t = millis();
  if (ledState == LED_STATE_OFF) {
    if (t > deactivatedAt + DEBOUNCE) {
      turnOn();
    }
  } else {
    if (t > activatedAt + DEBOUNCE) {
      turnOff();
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  // Test the LED.
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LED_LEVEL_ON);
    delay(500);
    digitalWrite(LED_PIN, LED_LEVEL_OFF);
    delay(500);
  }

  // Attach a touch interrupt.
  touchAttachInterrupt(TOUCH_PIN, handleTouchEvent, THRESHOLD);

  // Start with the LED off.
  digitalWrite(LED_PIN, LED_LEVEL_OFF);
}

void loop() {
  long t = millis();
  if (ledState == LED_STATE_ON && t > activatedAt + ACTIVATE_FOR) {
    turnOff();
  }
}
