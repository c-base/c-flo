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
#define DHTPIN            2
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// Gravity sensor
#include "MMA7660.h"
MMA7660 accelemeter;

struct Config {
  const String prefix = "sensor/";
  const String role = "workshop";

  const int pinAdc = A0;
  const int pinMotion = 14;
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
msgflo::OutPort *soundPort;
msgflo::OutPort *tempPort;
msgflo::OutPort *humPort;
msgflo::OutPort *gravityPort;
msgflo::OutPort *motionPort;

auto participant = msgflo::Participant("c-base/WorkshopSensor", cfg.role);
const int numReadings = 1000;
long nextMotionCheck = 0;
long nextMotionSend = 0;
long nextSoundCheck = 0;
long nextEnvCheck = 0;
int readings[numReadings];
int readIndex = 0;
int total = 0;
float average = 0;
float prevAverage = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  accelemeter.init();
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
  participant.icon = "wrench";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  soundPort = engine->addOutPort("sound", "number", cfg.prefix+cfg.role+"/sound");
  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
  humPort = engine->addOutPort("humidity", "number", cfg.prefix+cfg.role+"/humidity");
  gravityPort = engine->addOutPort("gravity", "array", cfg.prefix+cfg.role+"/gravity");
  motionPort = engine->addOutPort("motion", "float", cfg.prefix+cfg.role+"/motion");

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

    // Read accelerometer
    //float ax,ay,az;
    //accelemeter.getAcceleration(&ax,&ay,&az);
    //gravityPort->send('[' + String(ax) + ',' + String(ay) + ',' + String(az) + ']');
    int8_t x;
    int8_t y;
    int8_t z;
    accelemeter.getXYZ(&x,&y,&z);
    gravityPort->send('[' + String(x) + ',' + String(y) + ',' + String(z) + ']');

    nextEnvCheck += 30000;
  }

  if (connected && millis() > nextMotionCheck) {
    // Read motion sensor
    total = total - readings[readIndex];
    readings[readIndex] = digitalRead(cfg.pinMotion);
    total = total + readings[readIndex];
    average = ((float) total / numReadings);
    if (average != prevAverage && millis() > nextMotionSend) {
      Serial.printf("PIR state is %d (total %d), latest value %d\r\n", average, total, readings[readIndex]);
      motionPort->send(String(average));
      prevAverage = average;
      nextMotionSend += 5000;
    }
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }
    nextMotionCheck += 10;
  }
}
