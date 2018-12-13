#include "wificonfig.h"

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
  const int numReadings = 10;

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
const int numReadings = 1000;
long nextMotionCheck = 0;
long nextMotionSend = 0;
long nextEnvCheck = 0;
int readings[numReadings];
int readIndex = 0;
int total = 0;
float average = 0;
const String clientId = cfg.role + WiFi.macAddress();

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(cfg.pinMotion, INPUT);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
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

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
  humPort = engine->addOutPort("humidity", "number", cfg.prefix+cfg.role+"/humidity");
  motionPort = engine->addOutPort("motion", "float", cfg.prefix+cfg.role+"/motion");
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
    total = total - readings[readIndex];
    readings[readIndex] = digitalRead(cfg.pinMotion);
    total = total + readings[readIndex];
    average = ((float) total / numReadings);
    if (millis() > nextMotionSend) {
      Serial.printf("PIR state is %d (total %d), latest value %d\r\n", average, total, readings[readIndex]);
      if (average < 0.80) {
        motionPort->send("0.00");
      } else {
        motionPort->send(String(average));
      }
      nextMotionSend += 10000;
    }
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }
    nextMotionCheck += 10;
  }
}

