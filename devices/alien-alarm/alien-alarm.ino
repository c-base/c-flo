#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>

#include "FastLED.h" //two alien-eyes, ws2812B leds
#define NUM_LEDS 2
#define DATA_PIN D3

struct Config {
  const String prefix = "";
  const String role = "alien-alarm";

  const int builtinLed = D4;
  const int button = D1;
  const int door = D2;

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
msgflo::OutPort *doorPort;
msgflo::InPort *ledPort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

long nextDoorCheck = 0;
long resetAlienAlarmLed = 0;

auto participant = msgflo::Participant("c-base/AlienAlarm", cfg.role);

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Green;
  FastLED.show();
  
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
  doorPort = engine->addOutPort("door", "boolean", cfg.prefix+cfg.role+"/door");
  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in != "1" && in != "true");
      digitalWrite(cfg.builtinLed, on);
  });
  

  Serial.printf("Led pin: %d\r\n", cfg.builtinLed);
  Serial.printf("Button pin: %d\r\n", cfg.button);

  pinMode(cfg.button, INPUT);
  pinMode(cfg.builtinLed, OUTPUT);

  // Start with LED off
  digitalWrite(cfg.builtinLed, true);
}

bool pressedStateButton = true;
bool pressedStateDoor = false;

void checkDoor() {
  // Read door state every 50ms, send if changed
  if (millis() > nextDoorCheck) {
    const bool pressed = digitalRead(cfg.door);
    if (pressedStateDoor != pressed){
      doorPort->send(pressed ? "false" : "true");
      pressedStateDoor = pressed;
      Serial.write("Door pressed\n");
    } else if (millis() > nextPeriodicUpdate) {
      // Also send current state once per minute
      doorPort->send(pressedStateDoor ? "false" : "true");
      nextPeriodicUpdate += 60*1000;
    }
    nextDoorCheck += 50;
  }
}

void loop() {
  static bool connected = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      Serial.printf("Wifi connected: ip=%s\r\n", WiFi.localIP().toString().c_str());
    }
    connected = true;
    
    engine->loop();
    checkButtons();
    checkDoor();
    updateLed();
  } else {
    if (connected) {
      connected = false;
      Serial.println("Lost wifi connection.");
    }
  }
}
void updateLed() {
  if (resetAlienAlarmLed < millis()) {
      leds[0] = CRGB::Green;
      leds[1] = CRGB::Green;
      FastLED.show();
  }
}
void checkButtons() {
  // Read button state every 50ms, send if changed
  if (millis() > nextButtonCheck) {
    const bool pressed = digitalRead(cfg.button);
    if (pressedStateButton != pressed){
        if (pressed) {
          leds[0] = CRGB::Red;
          leds[1] = CRGB::Red;
          resetAlienAlarmLed = millis()+10*1000;
          FastLED.show();
      }
      alarmPort->send(pressed ? "false" : "true");
      
      pressedStateButton = pressed;
      Serial.write("Button pressed\n");
    } else if (millis() > nextPeriodicUpdate) {
      // Also send current state once per minute
      alarmPort->send(pressedStateButton ? "false" : "true");
      nextPeriodicUpdate += 60*1000;
    }
    nextButtonCheck += 50;
  }
}
