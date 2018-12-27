  s #include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "";
  const String role = "c-rash";

const int buttonPin = D4;

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
msgflo::OutPort *buttonPort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

const String clientId = cfg.role + WiFi.macAddress();

auto participant = msgflo::Participant("c-base/crashbutton", cfg.role);

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

  buttonPort = engine->addOutPort("pressed", "boolean", cfg.prefix+cfg.role+"/pressed");
  
  Serial.printf("Button pin: %d\r\n", cfg.buttonPin);

  pinMode(cfg.buttonPin, INPUT_PULLUP);
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
 void checkButtons(){
   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    const bool pressed = digitalRead(cfg.buttonPin);
    if (pressedStateButton != pressed){
      buttonPort->send(pressed ? "false" : "true");
      pressedStateButton = pressed;
     }
    nextButtonCheck += 50;
  }


   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextPeriodicUpdate) {
    const bool pressed = digitalRead(cfg.buttonPin);
    buttonPort->send(pressed ? "false" : "true");
    nextPeriodicUpdate += 60*1000;
  }
}
