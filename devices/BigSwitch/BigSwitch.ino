
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "button/";
  const String role = "bigswitch";

  const int ledPin = LED_BUILTIN;

  const int redLedPin = D5;
  const int blueLedPin = D6;
  const int greenLedPin = D7;

  const int buttonPin = D4;
  

  const char *wifiSsid = WIFI_SSID;
  const char *wifiPassword = WIFI_PASSWORD;

  const char *mqttHost = "c-beam.cbrp3.c-base.org";
  const int mqttPort = 1883;

  const char *mqttUsername = NULL;
  const char *mqttPassword = NULL;
} cfg;

typedef struct Color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

Color extractColorFromJson(const JsonObject& json, const char* pKey) {
  Color c;

  c.red   = json[pKey][0];
  c.green = json[pKey][1];
  c.blue  = json[pKey][2];

  return c;
}



WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::OutPort *buttonPort;
msgflo::InPort *ledPort;
msgflo::InPort *ledStripPort;

long nextButtonCheck = 0;

auto participant = msgflo::Participant("c-base/BigSwitch", cfg.role);


void callback(byte *payload, int length) {

  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  Color base;
  Color variant[5];
  Color contrast;

  base       = extractColorFromJson(root, "b");
  variant[0] = extractColorFromJson(root, "v1");
  variant[1] = extractColorFromJson(root, "v2");
  variant[2] = extractColorFromJson(root, "v3");
  variant[3] = extractColorFromJson(root, "v4");
  variant[4] = extractColorFromJson(root, "v5");
  contrast   = extractColorFromJson(root, "c");
  setLedGroupColor(contrast);

}

void setLedGroupColor(Color color) {
  analogWrite(cfg.blueLedPin, color.blue * 4);
  analogWrite(cfg.redLedPin, color.red * 4);
  analogWrite(cfg.greenLedPin, color.green * 4);

}

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
  participant.icon = "bomb";
  participant.label = "IP-Lounge Big Switch";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  buttonPort = engine->addOutPort("state", "boolean", cfg.prefix+cfg.role+"/on");

  ledPort = engine->addInPort("led", "boolean", cfg.prefix+cfg.role+"/led",
  [](byte *data, int length) -> void {
      const std::string in((char *)data, length);
      const boolean on = (in == "1" || in == "true");
      digitalWrite(cfg.ledPin, on);
  });

  ledStripPort = engine->addInPort("colors", "object", cfg.prefix+cfg.role+"/colors", callback);


  Serial.printf("Led pin: %d\r\n", cfg.ledPin);
  Serial.printf("Button pin: %d\r\n", cfg.buttonPin);

  pinMode(cfg.buttonPin, INPUT_PULLUP);
  pinMode(cfg.ledPin, OUTPUT);
  
  pinMode(cfg.blueLedPin, OUTPUT);
  pinMode(cfg.redLedPin, OUTPUT);
  pinMode(cfg.greenLedPin, OUTPUT);

  //analogWrite(cfg.redLedPin, 516);
  //
  //digitalWrite(cfg.blueLedPin,HIGH);
  //digitalWrite(cfg.redLedPin,HIGH);
  //digitalWrite(cfg.greenLedPin,HIGH);
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

  // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    const bool pressed = digitalRead(cfg.buttonPin);
    buttonPort->send(pressed ? "true" : "false");
    nextButtonCheck += 500;
  }
}

