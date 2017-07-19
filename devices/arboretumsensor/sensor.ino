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
long nextMotionCheck = 0;
long nextSoundCheck = 0;
long nextEnvCheck = 0;
bool bmpOk = true;
int pirState = LOW;
int latestPirState = LOW;

void setup() {
  Serial.begin(115200);
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    bmpOk = false;
  }
  pinMode(cfg.pinMotion, INPUT);
  pinMode(cfg.pinLed, OUTPUT);
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

  if (millis() > nextSoundCheck) {
    // Read sound sensor
    long sum = 0;
    for(int i=0; i<32; i++)
    {
      sum += analogRead(cfg.pinAdc);
    }
    sum >>= 5;
    soundPort->send(String(sum));
    nextSoundCheck += 10000;
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

  if (connected && millis() > nextMotionCheck) {
    // Read motion sensor
    latestPirState = digitalRead(cfg.pinMotion);
    if (latestPirState == HIGH) {
      digitalWrite(cfg.pinLed, LOW);
      if (pirState == LOW) {
        Serial.println("Motion detected!");
        motionPort->send("true");
        pirState = HIGH;
      }
    } else {
      digitalWrite(cfg.pinLed, HIGH);
      if (pirState == HIGH) {
        Serial.println("Motion ended!");
        motionPort->send("false");
        pirState = LOW;
      }
    }
    nextMotionCheck += 10;
  }
}
