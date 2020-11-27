// Touch sensing variables
const int threshold = 30;
const long activateFor = 5000;
long activatedAt = -1;

// LED state machine
const byte LED_STATE_OFF = 0;
const byte LED_STATE_ON  = 1;
byte ledState = LED_STATE_OFF;

// Pins
const int LED_PIN = 16;
const int TOUCH_PIN = T2;

// Turn off the LED, track state, and reset timer
void turnOff() {
  digitalWrite(LED_PIN, LOW);
  ledState = LED_STATE_OFF;
  activatedAt = -1;
}

// Turn on the LED, track state, and update timer
void turnOn() {
  digitalWrite(LED_PIN, HIGH);
  ledState = LED_STATE_ON;
  activatedAt = millis();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  // Test the LED
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }

  // Attach a touch interrupt
  touchAttachInterrupt(TOUCH_PIN, turnOn, threshold);
}

void loop() {
  long t = millis();
  if (ledState == LED_STATE_ON && t > activatedAt + activateFor) {
    turnOff();
  }
}
