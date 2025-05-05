#include <WiFi.h>
#include <ezTime.h>

#include "arduino_secrets.h"
#include "Font_Data.h"

#include <MD_Parola.h>
#include <MD_MAX72xx.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;


#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 16 
#define MAX_ZONES 2
#define ZONE_SIZE (MAX_DEVICES / MAX_ZONES)

#define CLK_PIN 9
#define DATA_PIN 13
#define CS_PIN 11

MD_Parola parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
Timezone myTZ;

WiFiServer server(80);

int countdown = 0;  
int milestone = 10; 
unsigned long lastUpdate = 0;
String milestoneMessage = "Milestone!";

void setup() {
  Serial.begin(115200);
  
  parola.begin(MAX_ZONES);
  parola.setIntensity(0);
  parola.displayClear();

  parola.setZone(0, 0, ZONE_SIZE - 1);
  parola.setZone(1, ZONE_SIZE, MAX_DEVICES - 1);
  parola.setFont(0, BigFontBottom);
  parola.setFont(1, BigFontUp);
 
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  waitForSync();
	myTZ.setLocation("Europe/Prague");

  server.begin();
  Serial.println("Server started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  String ipStr = WiFi.localIP().toString();
  parola.displayZoneText(0, ipStr.c_str(), PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  parola.displayZoneText(1, ipStr.c_str(), PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  parola.synchZoneStart();

  while (!parola.getZoneStatus(0) || !parola.getZoneStatus(1)) {
    parola.displayAnimate();
  }

  delay(2000);
  parola.displayClear();

}

void loop() {

  WiFiClient client = server.available();
  if (client) {
    handleClientRequest(client);
  }

  time_t now = myTZ.now();

  int currentHour = hour(now);
  int currentMinute = minute(now);
  
  char timeBuffer[9];
  sprintf(timeBuffer, "%02d:%02d", currentHour, currentMinute);
  parola.displayZoneText(0, timeBuffer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  parola.displayZoneText(1, timeBuffer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  parola.synchZoneStart();

  if (countdown > 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= 1000) { 
      lastUpdate = currentMillis;
      countdown--;

      
      char countdownBuffer[10];
      sprintf(countdownBuffer, "%d", countdown);
      parola.displayZoneText(0, countdownBuffer, PA_CENTER, 50, 50, PA_SCROLL_UP, PA_SCROLL_DOWN);
      parola.displayZoneText(1, countdownBuffer, PA_CENTER, 50, 50, PA_SCROLL_UP, PA_SCROLL_DOWN);
      parola.synchZoneStart();
      while (!parola.getZoneStatus(0) || !parola.getZoneStatus(1)) {
        parola.displayAnimate();
      }
    }

    if (countdown == milestone) {
      parola.displayClear();
      
      textEffect_t effect = (milestoneMessage.length() > (ZONE_SIZE * 8 / 6)) ? PA_SCROLL_LEFT : PA_WIPE;
      parola.displayZoneText(0, milestoneMessage.c_str(), PA_CENTER, 50, 50, effect, PA_MESH);
      parola.displayZoneText(1, milestoneMessage.c_str(), PA_CENTER, 50, 50, effect, PA_MESH);
      parola.synchZoneStart();
      while (!parola.getZoneStatus(0) || !parola.getZoneStatus(1)) {
        parola.displayAnimate();
      }
    }

    if (countdown == 0) {
      parola.displayClear();
      parola.displayZoneText(0, "Time's Up!", PA_CENTER, 50, 50, PA_WIPE, PA_MESH);
      parola.displayZoneText(1, "Time's Up!", PA_CENTER, 50, 50, PA_WIPE, PA_MESH);
      parola.synchZoneStart();
      while (!parola.getZoneStatus(0) || !parola.getZoneStatus(1)) {
        parola.displayAnimate();
      }
    }
    
  }
  delay(1000);
  parola.displayAnimate();
}

void handleClientRequest(WiFiClient client) {
  String request = "";
  while (client.available()) {
    request += char(client.read()); 
  }
  client.flush();
  Serial.println("Full request:");
  Serial.println(request);

  if (request.indexOf("POST") >= 0) {
    int contentIndex = request.indexOf("countdown=");
    if (contentIndex != -1) {
      int countdownIndex = request.indexOf("countdown=") + 10;
      int milestoneIndex = request.indexOf("milestone=") + 10;
      int milestoneTextIndex = request.indexOf("milestoneText=") + 14;

      countdown = request.substring(countdownIndex, request.indexOf("&", countdownIndex)).toInt()+1;
      milestone = request.substring(milestoneIndex, request.indexOf("&", milestoneIndex)).toInt();

      int endIndex = request.indexOf("&", milestoneTextIndex);
      if (endIndex == -1) endIndex = request.length();
      milestoneMessage = request.substring(milestoneTextIndex, endIndex);
      milestoneMessage.replace('+', ' ');
      milestoneMessage = decodeURIComponent(milestoneMessage);

      Serial.println("Updated countdown: " + String(countdown));
      Serial.println("Updated milestone: " + String(milestone));
      Serial.println("Milestone message: " + milestoneMessage);
    }

    String response = "Settings updated. <a href='/'>Go back</a>";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(response);
  } else {
    String html = "<html><body><h2>Countdown Timer</h2>";
    html += "<form action='/setCountdown' method='POST'>";
    html += "Countdown (seconds): <input type='number' name='countdown' value='" + String(countdown) + "'><br>";
    html += "Milestone (seconds): <input type='number' name='milestone' value='" + String(milestone) + "'><br>";
    html += "Milestone text: <input type='text' name='milestoneText' value='" + milestoneMessage + "'><br>";
    html += "<input type='submit' value='Set Timer'>";
    html += "</form></body></html>";
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);
  }

  delay(1);
  client.stop();
}

String decodeURIComponent(String input) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = input.length();
  for (unsigned int i = 0; i < len; i++) {
    if (input[i] == '%') {
      if (i + 2 < len) {
        temp[2] = input[i + 1];
        temp[3] = input[i + 2];
        decoded += char(strtol(temp, NULL, 16));
        i += 2;
      }
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

