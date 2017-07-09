#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>



#include <Wire.h>
#include "rgb_lcd.h"

#include <ArduinoJson.h>



struct Config {
  const String prefix = "";
  const String role = "c-interface";

  const int blueButton = D4;

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

WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::InPort *farbgeberPort;
msgflo::InPort *textPort;
msgflo::OutPort *bluePort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

auto participant = msgflo::Participant("c-base/screen", cfg.role);

rgb_lcd lcd;



void setLedGroupColor(Color color) {
    // Print a message to the LCD.
    Serial.println(color.red);
    Serial.println(color.green);
    Serial.println(color.blue);
    lcd.setRGB(color.red, color.green, color.blue);
}

void callbackText(byte *payload, int length) {
    lcd.clear();
   // Print a message to the LCD.
    std::string message((const char *)(payload),length);    
    lcd.setCursor(0, 0);
    std::string line = message.substr(0, _min(16,message.size()));
    lcd.print(line.c_str());
    if(message.size() >16){
       lcd.setCursor(0, 1);
       std::string line2 = message.substr(16, _min(32, message.size()));
       lcd.print(line2.c_str());
    }
 
}
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  participant.icon = "toggle-on";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);
  farbgeberPort = engine->addInPort("colors", "object", cfg.prefix+cfg.role+"/colors", callback);
  textPort = engine-> addInPort("text", "object", cfg.prefix+cfg.role+"/text", callbackText);
  bluePort = engine->addOutPort("blue", "boolean", cfg.prefix+cfg.role+"/blueButton");

 pinMode(cfg.blueButton, INPUT_PULLUP);

   // set up the LCD's number of columns and rows:
    //  input wire is sda = D2;
  //  input wire scl = D1;

    lcd.begin(16, 2);
}



 bool pressedStateBlueButton = true;

void loop() {
  static bool connected = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      Serial.printf("Wifi connected: ip=%s\r\n", WiFi.localIP().toString().c_str());
    }
    connected = true;
    
    engine->loop();
   checkButtons();
  } else {
    if (connected) {
      connected = false;
      Serial.println("Lost wifi connection.");
    }
  }

}

 void checkButtons(){
   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    const bool pressedBlue = digitalRead(cfg.blueButton);
    if(pressedStateBlueButton != pressedBlue){
      bluePort->send(pressedBlue ? "false" : "true");
      pressedStateBlueButton = pressedBlue;
    }


    
    nextButtonCheck += 50;
  }


   // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextPeriodicUpdate) {
    const bool pressedBlue = digitalRead(cfg.blueButton);
    bluePort->send(pressedBlue ? "false" : "true");

   
    nextPeriodicUpdate += 60*1000;
  }
  }
