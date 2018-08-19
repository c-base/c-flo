#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "config.h"

#include <PubSubClient.h>
#include <Msgflo.h>

#define MQTT_MAX_PACKET_SIZE 1024


#define BRIGHTNESS 30
#define NUM_LEDS 8
#define DATA_PIN    D5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

struct Config {
  const String prefix = "";
  const String role = "drinks-status";

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
msgflo::InPort *replicatorPort;
msgflo::InPort *barPort;

auto participant = msgflo::Participant("c-base/DrinksStatusLight", cfg.role);

const String clientId = cfg.role + WiFi.macAddress();

// 0 = unknown
// 1 = red
// 2 = green
// 2 = blue
int statuses[NUM_LEDS];

void updateLights() {
  for (int i=0; i<NUM_LEDS; i++) {
    if (statuses[i] == 3) {
      leds[i] = CRGB::Blue;
    } else if (statuses[i] == 2) {
      leds[i] = CRGB::Green;
    }
    else if (statuses[i] == 1) {
      leds[i] = CRGB::Red;
    }
    else if (statuses[i] == 0) {
      leds[i] = CRGB::White;
    }
  }
  FastLED.show();
}

boolean extractStatusFromJson(const JsonObject& json, const char* pKey) {
  return json[pKey];
}

// Called when replicator status updates
void replicatorCallback(byte *payload, int length) {
  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success()) {
    Serial.println("Parsing replicator status failed");
    return;
  }

  Serial.println("Updated replicator status.");
  bool replicator[7];
  replicator[1]  = extractStatusFromJson(root, " 1 Club-Mate 0,5 ");
  replicator[2] = extractStatusFromJson(root, " 2 Berliner 0,5 ");
  replicator[3] = extractStatusFromJson(root, " 3 Berliner 0,5 ");
  replicator[4] = extractStatusFromJson(root, " 4 Flora-Mate 0,5 ");
  replicator[5] = extractStatusFromJson(root, " 5 Premium Cola 0,5 ");
  replicator[6] = extractStatusFromJson(root, " 6 Spezi 0,5 ");
  replicator[7] = extractStatusFromJson(root, " 7 Kraftmalz 0,5 ");
  for (int i=1; i<8; i++) {
    if (replicator[i]) {
      statuses[i] = 2;
    } else {
      statuses[i] = 1;      
    }
  }
  
  updateLights();  
}

// Called when bar status updates
void barCallback(byte *payload, int length) {
  const std::string in((char *)payload, length);
  const boolean open = (in == "open" || in == "opening");

  Serial.printf("Updated bar status: %s.\r\n", in.c_str());
  if (open) {
    statuses[0] = 2;
  } else{
    statuses[0] = 1;
  }
  updateLights();
}

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
  participant.icon = "lightbulb-o";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);
  // FIXME: Switch to correct port in production
  // replicatorPort = engine->addInPort("replicator", "object", cfg.prefix+cfg.role+"/replicator", replicatorCallback);
  // barPort = engine->addInPort("bar", "string", cfg.prefix+cfg.role+"/bar", barrCallback);
  replicatorPort = engine->addInPort("replicator", "object", "Replicator/out", replicatorCallback);
  barPort = engine->addInPort("bar", "string", "bar/state", barCallback);
 
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  for (int i=0; i< NUM_LEDS; i++) {
    // Start everything false
    statuses[i] = 0;
  }

  FastLED.show();
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
}
