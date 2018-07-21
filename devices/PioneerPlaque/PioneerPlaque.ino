#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <PubSubClient.h>
#include <Msgflo.h>

#define MQTT_MAX_PACKET_SIZE 1024
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include <config.h>

struct Config {
  const String prefix = "case/";
  const String role = "pioneer";

  const int ledPin = 2;
  const int fastLedBrightness = 40;

  const char *wifiSsid = WIFI_SSID;
  const char *wifiPassword = WIFI_PASSWORD; 

  const char *mqttHost = "c-beam.cbrp3.c-base.org";
  const int mqttPort = 1883;

  const char *mqttUsername = NULL;
  const char *mqttPassword = NULL;
} cfg;

#define LED_TYPE NEOPIXEL
#define NUM_LEDS 5
#define FASTLED_PIN 5
CRGB leds[NUM_LEDS];

WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::InPort *ledPort;
msgflo::InPort *ledStripPort;

auto participant = msgflo::Participant("c-base/PioneerPlaque", cfg.role);

void setColorsFromJson(const JsonObject& json, const char* pKey) {
  for (int i=0; i < NUM_LEDS; i++) {
    leds[i].red   = json[pKey][0];
    leds[i].green = json[pKey][1];
    leds[i].blue  = json[pKey][2];
  }
  FastLED.show();
}

void callback(byte *payload, int length) {
  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  setColorsFromJson(root, "c");
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  participant.icon = "lightbulb-o";
  participant.label = "c-base Pioneer Plaque";

  FastLED.addLeds<LED_TYPE, FASTLED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(cfg.fastLedBrightness);

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.ledPin, on);
  });

  ledStripPort = engine->addInPort("colors", "object", cfg.prefix+cfg.role+"/colors", callback);

  pinMode(cfg.ledPin, OUTPUT);

}

// the loop function runs over and over again forever
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
