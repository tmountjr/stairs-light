#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

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
 * Login page
 */

const char* loginIndex = R"EOF(
<form name-"loginForm">
  <table width="20%" bgcolor="A09F9F" align="center">
    <tr>
      <td colspan=2>
        <center><font size=4><b>ESP32 Login Page</b></font></center>
        <br/>
      </td>
      <br/>
      <br/>
    </tr>
    <tr>
      <td>Username:</td>
      <td>
        <input type="text" size=25 name="userid">
        <br/>
      </td>
    </tr>
    <br/>
    <br/>
    <tr>
      <td>Password:</td>
      <td>
        <input type="password" size=25 name="pwd">
        <br/>
      </td>
    </tr>
    <tr>
      <td><input type="submit" onclick="check(this.form)" value="Login"></td>
    </tr>
  </table>
</form>
<script>
  function check(form) {
    if (form.userid.value == "admin" && form.pwd.value == "admin") {
      window.open("/serverIndex");
    } else {
      alert("Username/Password do not match.");
    }
  }
</script>
)EOF";
 
/*
 * Server Index Page
 */
 
const char* serverIndex = R"EOF(
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
  <input type='file' name='update'>
  <input type='submit' value='Update'>
</form>
<div id='prg'>progress: 0%</div>
<script>
  $('form').submit(function (e) {
    e.preventDefault();
    var form = $('#upload_form')[0];
    var data = new FormData(form);
    $.ajax({
      url: '/update',
      type: 'POST',
      data: data,
      contentType: false,
      processData: false,
      xhr: function () {
        var xhr = new window.XMLHttpRequest();
        xhr.upload.addEventListener('progress', function (evt) {
          if (evt.lengthComputable) {
            var per = evt.loaded / evt.total;
            $('#prg').html('progress: ' + Math.round(per * 100) + '%');
          }
        }, false);
        return xhr;
      },
      success: function (d, s) {
        console.log('success!')
      },
      error: function (a, b, c) {}
    });
  });
</script>
)";

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
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
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
        Update.printError(Serial)l
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
