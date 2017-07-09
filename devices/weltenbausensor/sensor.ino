#include "config.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

// MsgFlo communications
#include <PubSubClient.h>
#include <Msgflo.h>

// Environment sensor
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN            D1
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

struct Config {
  const String prefix = "sensor/";
  const String role = "weltenbausensor";

  const int pinMotion = D0;

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
msgflo::OutPort *tempPort;
msgflo::OutPort *humPort;
msgflo::OutPort *motionPort;

auto participant = msgflo::Participant("c-base/WeltenbauSensor", cfg.role);
long nextMotionCheck = 0;
long nextEnvCheck = 0;
int pirState = LOW;
int latestPirState = LOW;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(cfg.pinMotion, INPUT);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  participant.icon = "microchip";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
  humPort = engine->addOutPort("humidity", "number", cfg.prefix+cfg.role+"/humidity");
  motionPort = engine->addOutPort("motion", "boolean", cfg.prefix+cfg.role+"/motion");
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

  if (millis() > nextEnvCheck) {
    // Read DHT
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (!isnan(event.temperature)) {
      tempPort->send(String(event.temperature));
    }
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity)) {
      humPort->send(String(event.relative_humidity));
    }


    nextEnvCheck += 30000;
  }

  if (connected && millis() > nextMotionCheck) {
    // Read motion sensor
    latestPirState = digitalRead(cfg.pinMotion);
    if (latestPirState == HIGH) {
      if (pirState == LOW) {
        Serial.println("Motion detected!");
        motionPort->send("true");
        pirState = HIGH;
      }
    } else {
      if (pirState == HIGH) {
        Serial.println("Motion ended!");
        motionPort->send("false");
        pirState = LOW;
      }
    }
    nextMotionCheck += 10;
  }
}

