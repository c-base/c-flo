#include "config.h"
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

  // To safely configure WIFI_SSID and WIFI_PASSWORD, create a new tab in the Arduino IDE
  // (drop-down menu on the top right of the editor), name if config.h and add the following 
  // two lines to the newly created file:
  // #define WIFI_SSID "<put your SSID here>"
  // #define WIFI_PASSWORD "<put your WIFI PASSWORD here>"
  
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

auto participant = msgflo::Participant("c-base/SpreeSensor", cfg.role);
long nextTempCheck = 30000;
const String clientId = cfg.role + WiFi.macAddress();

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
  participant.icon = "ship";
  participant.label = "Spree sensor";


  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

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

  if (connected && (millis() > nextTempCheck)) {
    // Read temp sensor
    dtostrf(getTemperature(), 6, 2, result); // Leave room for too large numbers!
    Serial.print("millis = ");
    Serial.println(millis());
    
    Serial.print("temp = ");
    Serial.println(result);
    tempPort->send(result);
    
    nextTempCheck += 30000;
  }
  
}
