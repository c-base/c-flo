//#include "config.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// MsgFlo communications
#include <PubSubClient.h>
#include <Msgflo.h>

// Environment sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D1

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

struct Config {
  const String prefix = "c-base/";
  const String role = "spreesensor";

  const char *wifiSsid = "c-base-botnet";
  const char *wifiPassword = "xxxxxxx";

  const char *mqttHost = "c-beam.cbrp3.c-base.org";
  const int mqttPort = 1883;

  const char *mqttUsername = NULL;
  const char *mqttPassword = NULL;
} cfg;

WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::OutPort *tempPort;

auto participant = msgflo::Participant("c-base/SpreeSensor", cfg.role);
long nextTempCheck = 5000;


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
  participant.icon = "microchip";
  participant.label = "Spree temperature sensor";


  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  tempPort = engine->addOutPort("temperature", "number", cfg.prefix+cfg.role+"/temperature");
}

float getTemperature() {
  float temp;
  
  for (int i = 0; i < 100; i++) {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
    if ((temp != 85.0) && (temp != -127.0)) {
      break;
    }
  }
  
  return temp;
}

// store temperature as string
char result[8]; 

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

  if (connected && millis() > nextTempCheck) {
    // Read temp sensor
    dtostrf(getTemperature(), 6, 2, result); // Leave room for too large numbers!
    Serial.println("temp = ");

    Serial.println(result);
    tempPort->send(result);
    
    nextTempCheck += 5000;
  }
  
}
