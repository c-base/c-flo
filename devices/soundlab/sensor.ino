// Note: This Lolin ver 0.1 board needs to be flashed as "nodemcuv2", not "nodemcu" 
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
#define DHTPIN            D5
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

struct Config {
  const String prefix = "sensor/";
  const String role = "soundlab";

  const int pinAdc = A0;
  const int pinLed = 2;

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
msgflo::OutPort *soundPort;
msgflo::OutPort *tempPort;
msgflo::OutPort *humPort;
msgflo::InPort *ledPort;

auto participant = msgflo::Participant("c-base/SoundlabSensor", cfg.role);
const int numReadings = 1000;
long nextSoundCheck = 0;
long nextSoundSend = 0;
long nextEnvCheck = 0;
int readIndex = 0;
int soundReadings[numReadings];
int soundTotal = 0;
float soundAverage = 0;
const String clientId = cfg.role + WiFi.macAddress();

void setup() {
  Serial.begin(115200);
  dht.begin();
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    soundReadings[thisReading] = 0;
  }
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  participant.icon = "soundcloud";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  soundPort = engine->addOutPort("sound", "number", cfg.prefix+cfg.role+"/sound");
  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
  humPort = engine->addOutPort("humidity", "number", cfg.prefix+cfg.role+"/humidity");
  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.pinLed, on);
  });

  Serial.printf("Led pin: %d\r\n", cfg.pinLed);
  Serial.printf("Sound pin: %d\r\n", cfg.pinAdc);
  pinMode(cfg.pinLed, OUTPUT);
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
      Serial.printf("Sending temperature %d\r\n", event.temperature);
      tempPort->send(String(event.temperature));
    } else {
      Serial.println("DHT returned NaN");
    }
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity)) {
      humPort->send(String(event.relative_humidity));
    }
    nextEnvCheck += 30000;
  }

  if (millis() > nextSoundCheck) {
    // Read sound sensor
    soundTotal = soundTotal - soundReadings[readIndex];
    soundReadings[readIndex] = analogRead(cfg.pinAdc);
    soundTotal = soundTotal + soundReadings[readIndex];
    soundAverage = ((float) soundTotal / numReadings);
    if (millis() > nextSoundSend) {
      soundPort->send(String(soundAverage));
      nextSoundSend += 10000;
    }

    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }
    nextSoundCheck += 10;
  }
}
