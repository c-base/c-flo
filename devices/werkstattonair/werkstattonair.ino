#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "FastLED.h"

// MsgFlo communications
#include <PubSubClient.h>
#include <Msgflo.h>
#define NUM_LEDS 10
#define DATA_PIN 3

const int BUTTON_PIN = D7;

// Define the array of leds
CRGB leds[NUM_LEDS];

// Variables will change:
int ledState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


struct Config {
  const String prefix = "c-base/";
  const String role = "werkstattonair";

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
msgflo::OutPort *onAirPort;

auto participant = msgflo::Participant("c-base/werkstattonair", cfg.role);

long nextButtonCheck = 0;

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
  participant.icon = "video-camera";
  participant.label = "Werkstatt ON AIR";


  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  onAirPort = engine->addOutPort("onair", "boolean", cfg.prefix+cfg.role+"/onair");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);

}

void setLeds(int state) {
  // Turn the LED on, then pause
  for (int i=0; i<NUM_LEDS; i++) {
    if (state == LOW) {
      leds[i] = CRGB::Red;
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}

void loop() {
  static bool connected = false;
  static int buttonState = LOW;
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


  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        setLeds(ledState);
      }
    }
  }  
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;

  // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
  if (millis() > nextButtonCheck) {
    // note: ledState is inverted, i.e. true means light is off
    onAirPort->send(ledState ? "false" : "true");
    nextButtonCheck += 500;
  }
  
}



