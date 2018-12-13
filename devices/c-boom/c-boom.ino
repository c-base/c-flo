#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "";
  const String role = "c-boom";

  const int nitroPin = D1;
  const int bigPin = D4;
  const int boomLed = D2;
  const int vibra = D5;

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
msgflo::OutPort *bigPort;
msgflo::OutPort *nitroPort;
msgflo::InPort *vibraPort;
msgflo::InPort *ledPort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

const String clientId = cfg.role + WiFi.macAddress();

auto participant = msgflo::Participant("c-base/buttonpanel", cfg.role);

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
  participant.icon = "toggle-on";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  nitroPort = engine->addOutPort("ignition", "any", cfg.prefix+cfg.role+"/ignition");
  bigPort = engine->addOutPort("start", "any", cfg.prefix+cfg.role+"/start");
  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.boomLed, on);
  });
  vibraPort = engine->addInPort("vibra", "boolean", cfg.prefix+cfg.role+"/vibra",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.vibra, on);
  });

  Serial.printf("Led pin: %d\r\n", cfg.boomLed);
  Serial.printf("Led pin: %d\r\n", cfg.vibra);
  Serial.printf("Big pin: %d\r\n", cfg.bigPin);
  Serial.printf("Nitro pin: %d\r\n", cfg.nitroPin);

  pinMode(cfg.nitroPin, INPUT_PULLUP);
  pinMode(cfg.bigPin, INPUT_PULLUP);
  pinMode(cfg.boomLed, OUTPUT);
  pinMode(cfg.vibra, OUTPUT);
}

 bool pressedStateBig = true;
 bool pressedStateNitro = true;

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
 void checkButtons(){
   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    const bool pressed = digitalRead(cfg.bigPin);
    if(pressedStateBig != pressed){
      bigPort->send(pressed ? "false" : "true");
      pressedStateBig = pressed;
      }
    
    const bool pressed2 = digitalRead(cfg.nitroPin);
     if(pressedStateNitro != pressed2){
      nitroPort->send(pressed2 ? "false" : "true");
      pressedStateNitro = pressed2;
      }
    
    nextButtonCheck += 50;
  }


   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextPeriodicUpdate) {
    const bool pressed = digitalRead(cfg.bigPin);
    const bool pressed2 = digitalRead(cfg.nitroPin);
    bigPort->send(pressed ? "false" : "true");
    nitroPort->send(pressed2 ? "false" : "true");
    nextPeriodicUpdate += 60*1000;
  }
  }
