#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi.h"
WiFiClient espClient;
PubSubClient client(espClient);
int maxtry = 30;
bool WiFiConnected = false;

#include <ArduinoJson.h>
DynamicJsonDocument CMD(2048);

#include <PxMatrix.h>
#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#define matrix_width 32
#define matrix_height 16

PxMATRIX display(32, 16, P_LAT, P_OE, P_A, P_B, P_C);
uint8_t display_draw_time = 60; //30-70 is usually fine

int R = 0;
int G = 0;
int B = 0;
int X = 0;
int Y = 0;

void setup() {
  Serial.begin(115200);
  display.begin(8);
  display.setRotate(true);
  display.clearDisplay();
  display_ticker.attach(0.004, display_updater);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Settings(topic, messageTemp);
}

void loop() {
  MQTT_loop();
  if (Serial.available()) {
    String input = Serial.readString();
    Settings(TopicCMD, input);
    Serial.read();
  }
}

void Settings(String topic, String messageTemp) {
  Serial.println(messageTemp);
  if (topic == TopicCMD) {
    deserializeJson(CMD, messageTemp);
    if (CMD.containsKey("CMD")) {
      String command = CMD["CMD"];
      if (command == "ALL") {
        R = CMD["R"];
        G = CMD["G"];
        B = CMD["B"];
        display.fillRect(0, 0, 16, 32,display.color565(R, G, B));
        return;
      }
    }
    if (CMD.containsKey("R")) {
      R = CMD["R"];
    }
    if (CMD.containsKey("G")) {
      G = CMD["G"];
    }
    if (CMD.containsKey("B")) {
      B = CMD["B"];
    }

    if (CMD.containsKey("X")) {
      X = CMD["X"];
    }
    if (CMD.containsKey("Y")) {
      Y = CMD["Y"];
    }
    display.drawPixel(X, Y, display.color565(R, G, B));
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HostName);
  WiFi.begin(ssid, password);
  for (int i = 0; i < maxtry; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      display.drawPixel(0, 0, display.color565(0, 255, 0));
      WiFiConnected = true;
      return;
    }
    display.drawPixel(0, 0, display.color565(255, 0, 0));
    delay(250);
    Serial.print(".");
    display.drawPixel(0, 0, display.color565(0, 0, 0));
    delay(250);
  }
  WiFiConnected = false;
  display.drawPixel(0, 0, display.color565(255, 0, 0));
  Serial.println("WiFi not connected");
  Serial.println("Serial Only mode");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(HostName)) {
      Serial.println("connected");
      client.subscribe(TopicCMD);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      display.drawPixel(0, 0, display.color565(255, 0, 0));
      delay(5000);
    }
  }
}

void MQTT_loop() {
  if (WiFiConnected == true) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}

void display_updater()
{
  display.display(display_draw_time);
}
