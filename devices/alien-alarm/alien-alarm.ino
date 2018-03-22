#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "";
  const String role = "alien-alarm";

  const int builtinLed = D4;
  const int button = D2;

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
msgflo::OutPort *alarmPort;
msgflo::InPort *ledPort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

auto participant = msgflo::Participant("c-base/AlienAlarm", cfg.role);

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
  participant.icon = "reddit-alien";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);
  alarmPort = engine->addOutPort("alarm", "boolean", cfg.prefix+cfg.role+"/alarm");
  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in != "1" && in != "true");
      digitalWrite(cfg.builtinLed, on);
  });
  

  Serial.printf("Led pin: %d\r\n", cfg.builtinLed);
  Serial.printf("Button pin: %d\r\n", cfg.button);

  pinMode(cfg.button, INPUT_PULLUP);
  pinMode(cfg.builtinLed, OUTPUT);

  // Start with LED off
  digitalWrite(cfg.builtinLed, true);
}

bool pressedStateButton = true;

void loop() {
  static bool connected = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      Serial.printf("Wifi connected: ip=%s\r\n", WiFi.localIP().toString().c_str());
    }
    connected = true;
    
    engine->loop();
    checkButtons();
  } else {
    if (connected) {
      connected = false;
      Serial.println("Lost wifi connection.");
    }
  }
}

void checkButtons() {
  // Read button state every 50ms, send if changed
  if (millis() > nextButtonCheck) {
    const bool pressed = digitalRead(cfg.button);
    if (pressedStateButton != pressed){
      alarmPort->send(pressed ? "false" : "true");
      pressedStateButton = pressed;
    } else if (millis() > nextPeriodicUpdate) {
      // Also send current state once per minute
      alarmPort->send(pressedStateButton ? "false" : "true");
      nextPeriodicUpdate += 60*1000;
    }
    nextButtonCheck += 50;
  }
}
