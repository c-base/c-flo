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


// Variables for controlling the state
bool previousState = false; // What was the state before this state
bool currentState = false;  // What is the current state. true == 'bar open'
unsigned long lastStateChange = 0;    // When did it last change. 0 means we didn't see a change
bool animationPhase = false; // Which phase of animation we are, the 'real' state color or the alternate
unsigned long lastLoop = 0;           // Time of the previous loop execution
unsigned long loopInterval = 1000;    // How often to run the animation (lastLoop + 1000 >= millis())
bool showingPrimary = true; // In animation, are we showing the primary or secondary color
int showAnimationFor = 2 * 60 * 1000;
CRGB::HTMLColorCode openSideCurrent = CRGB::White;
CRGB::HTMLColorCode closeSideCurrent = CRGB::White;

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
    currentState = true;
  } else if (inputPayload == "closed") {
    digitalWrite(D4, HIGH);  // Turn the BUILTIN_LED off by making the voltage HIGH
    currentState = false; 
  }
  if (lastStateChange == 0) {
    // First state received => no animation
    previousState = currentState;
  }
  lastStateChange = millis();

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


// Pseudo-code
// in callback: just set current state to true/false (DONE)


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

// in loop:
//   if lastStateChange is 0 and both currentState and previousState are false
//     Initial state, go blue
//   if currentState is same as previousState
//     Set lights according to the current state (green or red)
//   if currentState is different from previousState
//     if lastStateChange is 0
//       Set lights according to current state (green or red)
//     if lastLoop is less than a second ago, skip
//     if lastStateChange is less than two minutes ago
//       Do the bar opening/closing animation depending which is the current state is
//       Hint: keep animation true/false boolean in global state, flip it every cycle of the loop
//     if lastStateChange is greater than two minutes ago
//       Set lastStateChange to current time minus two minutes
//       Set previousState to value of currentState
//       Set lights according to current state (green or red)
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (lastStateChange == 0 && currentState == false && previousState == false) {
    // Initial state,go blue
    setColors(CRGB::Blue, CRGB::White);
    lastLoop = millis();
    return;
  }
  if (currentState == previousState) {
    // Set lights according to the current state (green or red);
    if (currentState == true) {
      setColors(CRGB::Green, CRGB::White);
    } else {
      setColors(CRGB::White, CRGB::Red);
    }
    lastLoop = millis();
    return;
  }
  if ((lastLoop + loopInterval) > millis()) {
    // Skip since the last animation step was less than a second ago
    return;
  }
  if ((lastStateChange + showAnimationFor) > millis()) {
    // Bar status changed less than two minutes ago, we should be animating
    // Do the bar opening/closing animation depending which is the current state is
    if (showingPrimary == true) {
      // Show yellow or blue
      showingPrimary = false;
      Serial.println("Show alternate color");
      if (currentState == true) {
        setColors(CRGB::Blue, CRGB::White);
      } else {
        setColors(CRGB::White, CRGB::Yellow);
      }
    } else {
      // Show red or green
      Serial.println("Show primary color");
      showingPrimary = true;
      if (currentState == true) {
        setColors(CRGB::Green, CRGB::White);
      } else {
        setColors(CRGB::White, CRGB::Red );
      }
    }
    lastLoop = millis();
    return;
  }
  // Bar status changed more than two minutes ago, go stable
  previousState = currentState;
  // Set lights according to the current state (green or red)
  if (currentState == true) {
    Serial.println("Stable state: open");
    setColors(CRGB::Green, CRGB::White);
  } else {
    Serial.println("Stable state: closed");
    setColors(CRGB::White, CRGB::Red);
  }
  lastLoop = millis();
}

void setColors(CRGB::HTMLColorCode openSide, CRGB::HTMLColorCode closeSide) {
  setOpenSide(openSide);
  setCloseSide(closeSide);
  if (closeSideCurrent != closeSide || openSideCurrent != openSide) {
    // Only change lights if values change
    closeSideCurrent = closeSide;
    openSideCurrent = openSide;
    FastLED.show();
  }
}

void setOpenSide(CRGB::HTMLColorCode color) {
  for (int i = 0; i < 21; i++) {
    leds[i] = color;
  }
  for (int i = 104; i < 165; i++) {
    leds[i] = color;
  }
}

void setCloseSide(CRGB::HTMLColorCode color) {
  for (int i = 21; i < 104; i++) {
    leds[i] = color;
  }
}
