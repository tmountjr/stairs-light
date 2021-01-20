#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const long VERSION = 1.4;

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
const char* password = R"=====(wifi-password-here)=====";
const char* host = "stairs_led";

WebServer server(80);

/*
 * Server Index Page
 */
const char *serverIndex = R"EOF(
<!doctype html>
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Upload Sketch</title>
  <link rel="stylesheet" href="https://unpkg.com/purecss@1.0.1/build/base-min.css">
  <link rel="stylesheet" href="https://unpkg.com/purecss@2.0.3/build/pure-min.css">
</head>

<body>
  <div id="app"></div>

  <script src="https://vuejs.org/js/vue.js"></script>
  <script src="https://cdn.jsdelivr.net/gh/tmountjr/stairs-light@1.4.0/js/dist/main.js"></script>
</body>

</html>
)EOF";

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

  Serial.print("Current version: ");
  Serial.println(VERSION);

  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  if (!MDNS.begin(host)) {
    Serial.println("Error setting up MDNS responder!");
    ESP.restart();
  }

  server.on("/", []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success! Total size: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

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
  server.handleClient();
  delay(1);
  long t = millis();
  if (ledState == LED_STATE_ON && t > activatedAt + ACTIVATE_FOR) {
    turnOff();
  }
}
