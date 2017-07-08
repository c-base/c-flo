 
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
long nextButtonCheck = 0;

auto participant = msgflo::Participant("c-base/DoorStatus", cfg.role);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  participant.icon = "toggle-on";

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

  // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    const bool rightdoorpressed = digitalRead(cfg.rightdoorPin);
    const bool leftdoorpressed = digitalRead(cfg.leftdoorPin);
    
    rightdoorPort->send(rightdoorpressed ? "true" : "false");
    leftdoorPort->send(leftdoorpressed ? "true" : "false");
    nextButtonCheck += 1000;
  }
}
