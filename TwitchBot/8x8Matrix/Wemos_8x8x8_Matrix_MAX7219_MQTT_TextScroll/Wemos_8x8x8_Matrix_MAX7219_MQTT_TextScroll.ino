#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi.h"
WiFiClient espClient;
PubSubClient client(espClient);

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 2; // Attach CS to this pin, DIN to MOSI and CLK to SCK
int numberOfHorizontalDisplays = 8;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 25; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  for ( int i = 0 ; i < width * messageTemp.length() + matrix.width() - 1 - spacer; i++ ) {
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < messageTemp.length() ) {
        matrix.drawChar(x, y, messageTemp[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait);
  }

  matrix.fillScreen(LOW);
  matrix.write();
}

void setup() {

  matrix.setIntensity(15); // Use a value between 0 and 15 for brightness

  matrix.setPosition(0, 7, 0);
  matrix.setPosition(1, 6, 0);
  matrix.setPosition(2, 5, 0);
  matrix.setPosition(3, 4, 0);
  matrix.setPosition(4, 3, 0);
  matrix.setPosition(5, 2, 0);
  matrix.setPosition(6, 1, 0);
  matrix.setPosition(7, 0, 0);

  matrix.setRotation(0, 3);    
  matrix.setRotation(1, 3);
  matrix.setRotation(2, 3);
  matrix.setRotation(3, 3);
  matrix.setRotation(4, 3);
  matrix.setRotation(5, 3);
  matrix.setRotation(6, 3);
  matrix.setRotation(7, 3);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(LED_BUILTIN, OUTPUT);
  client.setCallback(callback);

  matrix.fillScreen(LOW);
  matrix.write();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HostName);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(HostName)) {
      Serial.println("connected");
      digitalWrite(LED_BUILTIN, HIGH);
      client.subscribe(TopicDOWN);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(LED_BUILTIN, LOW);
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
