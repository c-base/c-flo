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
#define DHTPIN            14
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// Pressure sensor
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

struct Config {
  const String prefix = "sensor/";
  const String role = "arboretum";

  const int pinAdc = A0;
  const int pinMotion = 12;
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
msgflo::OutPort *pressurePort;
msgflo::OutPort *motionPort;

auto participant = msgflo::Participant("c-base/ArboretumSensor", cfg.role);
const int numReadings = 1000;
long nextMotionCheck = 0;
long nextMotionSend = 0;
long nextEnvCheck = 0;
bool bmpOk = true;
int readIndex = 0;
int pirReadings[numReadings];
int pirTotal = 0;
float pirAverage = 0;
int soundReadings[numReadings];
int soundTotal = 0;
float soundAverage = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin(4, 5);
  if (!bmp.begin(1)) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    bmpOk = false;
  }
  pinMode(cfg.pinMotion, INPUT);
  pinMode(cfg.pinLed, OUTPUT);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    pirReadings[thisReading] = 0;
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
  participant.icon = "tree";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  soundPort = engine->addOutPort("sound", "number", cfg.prefix+cfg.role+"/sound");
  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
  humPort = engine->addOutPort("humidity", "number", cfg.prefix+cfg.role+"/humidity");
  pressurePort = engine->addOutPort("pressure", "number", cfg.prefix+cfg.role+"/pressure");
  motionPort = engine->addOutPort("motion", "boolean", cfg.prefix+cfg.role+"/motion");

  Serial.printf("Sound pin: %d\r\n", cfg.pinAdc);
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

    // Read pressure
    if (bmpOk) {
      pressurePort->send(String(bmp.readPressure()));
    } else {
      Serial.println("No pressure sensor found, skipping measurement");
    }

    nextEnvCheck += 30000;
  }

  if (millis() > nextMotionCheck) {
    // Read motion sensor
    pirTotal = pirTotal - pirReadings[readIndex];
    pirReadings[readIndex] = digitalRead(cfg.pinMotion);
    pirTotal = pirTotal + pirReadings[readIndex];
    pirAverage = ((float) pirTotal / numReadings);
    if (millis() > nextMotionSend) {
      Serial.printf("PIR state is %d (total %d), latest value %d\r\n", pirAverage, pirTotal, pirReadings[readIndex]);
      if (pirAverage < 0.80) {
        motionPort->send("0.00");
      } else {
        motionPort->send(String(pirAverage));
      }
      nextMotionSend += 10000;
    }
    // Read sound sensor
    soundTotal = soundTotal - soundReadings[readIndex];
    soundReadings[readIndex] = analogRead(cfg.pinAdc);
    soundTotal = soundTotal + soundReadings[readIndex];
    soundAverage = ((float) soundTotal / numReadings);
    if (millis() > nextMotionSend) {
      soundPort->send(String(soundAverage));
    }

    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }
    nextMotionCheck += 10;
  }
}
