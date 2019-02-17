// include libraries and config.h
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include "config.h"

//defines for FastLED
#define NUM_LEDS 165
#define DATA_PIN D3
#define LED_TYPE WS2812B
#define BRIGHTNESS 255
#define COLOR_ORDER GRB
#define FRAMES_PER_SECOND 60

// Define the array of leds
CRGB leds[NUM_LEDS];

//MQTT defines
#define MQTT_HOST "c-beam.cbrp3.c-base.org"


// Setup WIFI Connection as defined in config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Setup MQTT Server
const char* mqtt_server = MQTT_HOST;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to your WIFI Network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String inputPayload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inputPayload += (char)payload[i];
  }
  Serial.println();

  Serial.println();
  Serial.print("debugPayload: ");
  Serial.print(inputPayload);
  Serial.println();
  
  if (inputPayload == "open") {
    digitalWrite(D4, LOW);   // Turn the BUILTIN_LED on by making the voltage LOW
    baropen();
    
  } else {
    digitalWrite(D4, HIGH);  // Turn the BUILTIN_LED off by making the voltage HIGH
    barclose();
    
  }

}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("bar/state");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Setup FastLED LEDs and Brightness
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS );
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // Initialize D4 / BUILTIN_LED as an output
  pinMode(D4, OUTPUT);

  // Setup Serial
  Serial.begin(115200);

  // Execute WIFI-Setup
  setup_wifi();

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Startup-Blue to LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
    }
    FastLED.show();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void baropen() {
    // Tell FastLED to turn close side White
for (int i = 21; i < 104; i++) {
    leds[i] = CRGB::White;
    }
    delay(50);

    
    // Tell FastLED to turn the open side GREEN
    for (int t = 0; t < 10; t++) {
    
    for (int i = 0; i < 21; i++) {
    leds[i] = CRGB::Blue;
    }
    delay(50);
    for (int i = 104; i < 165; i++) {
    leds[i] = CRGB::Blue;
    }
    FastLED.show();
    delay(500);
    
    for (int i = 0; i < 21; i++) {
    leds[i] = CRGB::Green;
    }
    delay(50);
    for (int i = 104; i < 165; i++) {
    leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(500);
  }
}

void barclose() {

    // Tell FastLED to turn open side White
    for (int i = 0; i < 21; i++) {
    leds[i] = CRGB::White;
    }
    delay(50);
    for (int i = 104; i < 165; i++) {
    leds[i] = CRGB::White;
    }
    delay(50);


    // Tell FastLED to turn the close side RED
for (int t = 0; t < 10; t++) {
  
    for (int i = 21; i < 104; i++) {
    leds[i] = CRGB::Yellow;
    }
    FastLED.show();
    delay(500);

  for (int i = 21; i < 104; i++) {
    leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
  }    
}

