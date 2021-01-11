#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Touch sensing variables
const float THRESHOLD = 7;
const long ACTIVATE_FOR = 60000;
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

// OTA Update
const char* ssid = "ihatecomputers";
const char* password = R"=====(
wifi password here
)=====";

// Turn off the LED, track state, and reset timer.
void turnOff() {
  //Serial.println("TURN OFF");
  digitalWrite(LED_PIN, LED_LEVEL_OFF);
  ledState = LED_STATE_OFF;
  activatedAt = -1;
  deactivatedAt = millis();
}

// Turn on the LED, track state, and update timer.
void turnOn() {
  //Serial.println("TURN ON");
  digitalWrite(LED_PIN, LED_LEVEL_ON);
  ledState = LED_STATE_ON;
  activatedAt = millis();
  deactivatedAt = -1;
}

// Generic event handler that allows a second touch event to turn the LED off and
// stop the timer.
void handleTouchEvent() {
  //Serial.println("Touch Event");
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
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000)
    ESP.restart();
  }

  ArduinoOTA.setHostname("stairs_led");

  ArduinoOTA
    .onStart([]() {
      // Disable the touch interrupts so that they don't fire during the update.
      touch_pad_intr_disable();
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {
        type = "filesystem";
      }
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

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
  //Serial.println(touchRead(TOUCH_PIN));
  ArduinoOTA.handle();
  long t = millis();
  if (ledState == LED_STATE_ON && t > activatedAt + ACTIVATE_FOR) {
    turnOff();
  }
}
