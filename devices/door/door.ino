 
#include "config.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "";
  const String role = "arboretum/door";

  const int ledPin = LED_BUILTIN;
  const int rightdoorPin = D5;
  const int leftdoorPin = D6;

  const char *wifiSsid = WIFI_SSID;
  const char *wifiPassword = WIFI_PASSWORD;

  const char *mqttHost = "c-beam.cbrp3.c-base.org";
  const int mqttPort = 1883;
  const char *mqttUsername = NULL;
  const char *mqttPassword = NULL;
} cfg;

WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::OutPort *rightdoorPort;
msgflo::OutPort *leftdoorPort;
msgflo::InPort *ledPort;
long nextDoorCheck = 0;
long nextDoorSend = 60000;
int rightDoorState = LOW;
int leftDoorState = LOW;
int latestRightDoorState = LOW;
int latestLeftDoorState = LOW;

auto participant = msgflo::Participant("c-base/DoorStatus", cfg.role);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  participant.icon = "sign-out";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  rightdoorPort = engine->addOutPort("rightdoor", "any", cfg.prefix+cfg.role+"/rightdooropen");
  leftdoorPort = engine->addOutPort("leftdoor", "any", cfg.prefix+cfg.role+"/leftdooropen");

  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.ledPin, on);
  });

  Serial.printf("Led pin: %d\r\n", cfg.ledPin);
  Serial.printf("right door pin: %d\r\n", cfg.rightdoorPin);
  Serial.printf("left door pin: %d\r\n", cfg.leftdoorPin);
  pinMode(cfg.rightdoorPin, INPUT_PULLUP);
  pinMode(cfg.leftdoorPin, INPUT_PULLUP);
  pinMode(cfg.ledPin, OUTPUT);
}

void loop() {
  static bool connected = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      Serial.printf("Wifi connected: ip=%s\r\n", WiFi.localIP().toString().c_str());
    }
    connected = true;
    engine->loop();
  } else {
    if (connected) {
      connected = false;
      Serial.println("Lost wifi connection.");
    }
  }

  if (connected && millis() > nextDoorCheck) {
    latestRightDoorState = digitalRead(cfg.rightdoorPin);
    latestLeftDoorState = digitalRead(cfg.leftdoorPin);
    if (latestRightDoorState == HIGH) {
      if (rightDoorState == LOW) {
        rightdoorPort->send("true");
        rightDoorState = HIGH;
        nextDoorSend += 60000;
      }
    } else {
      if (rightDoorState == HIGH) {
        rightdoorPort->send("false");
        rightDoorState = LOW;
        nextDoorSend += 60000;
      }
    }
    if (latestLeftDoorState == HIGH) {
      if (leftDoorState == LOW) {
        leftdoorPort->send("true");
        leftDoorState = HIGH;
        nextDoorSend += 60000;
      }
    } else {
      if (leftDoorState == HIGH) {
        leftdoorPort->send("false");
        leftDoorState = LOW;
        nextDoorSend += 60000;
      }
    }
    nextDoorCheck += 100;
  }
  if (connected && millis() > nextDoorSend) {
    rightdoorPort->send(rightDoorState ? "true" : "false");
    leftdoorPort->send(leftDoorState ? "true" : "false");
    nextDoorSend += 60000;
  }
}
