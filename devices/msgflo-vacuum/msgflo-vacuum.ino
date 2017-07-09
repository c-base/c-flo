/**
 * Createa msgflo participant for a vacuum clean based on an ESP8266. 
 */
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


struct Config {
  const String prefix = "c-base/";
  const String role = "vacuum";

  //const int ledPin = LED_BUILTIN;
  //const int buttonPin = D5;

  const char *wifiSsid = WIFI_SSID;
  const char *wifiPassword = "WIFI_PASSWORD";

  const char *mqttHost = "c-beam.cbrp3.c-base.org";
  const int mqttPort = 1883;

  const char *mqttUsername = NULL;
  const char *mqttPassword = NULL;
} cfg;

char topic[] = "c-base/vacuum/on"; // cfg.prefix+cfg.role+"/on";

WiFiClient wifiClient;
PubSubClient mqttClient;
msgflo::Engine *engine;
msgflo::OutPort *alivePort;
long nextMessage = 0;


void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect

    Serial.print("Topic is ");
    Serial.println(topic);
    if (mqttClient.connect("c-base-vacuum", topic, 1, 1, "false")) {
      Serial.println("connected");
      // Once connected, publish an announcement...  
      Serial.println("Sending 1st alive msg done");
      // ... and resubscribe
        // TODO: check for statechange. If changed, send right away. Else only send every 3 seconds or so
      while(mqttClient.connected()) {
        if (millis() > nextMessage) {
          Serial.printf("Sending alive msg");
          //const bool pressed = digitalRead(cfg.buttonPin);
          mqttClient.publish(topic, "true");
          nextMessage += 5000;
        }
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  delay(100); 
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Configuring wifi: %s\r\n", cfg.wifiSsid);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  // Provide a Font Awesome (http://fontawesome.io/icons/) icon for the component
  //participant.icon = "mixcloud";

  mqttClient.setServer(cfg.mqttHost, cfg.mqttPort);
  mqttClient.setClient(wifiClient);

  String clientId = cfg.role;
  clientId += WiFi.macAddress();

  //engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);

  //alivePort = engine->addOutPort("on", "any", cfg.prefix+cfg.role+"/on");

  // Serial.printf("Led pin: %d\r\n", cfg.ledPin);
  // Serial.printf("Button pin: %d\r\n", cfg.buttonPin);

}

void loop() {
  static bool connected = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      Serial.printf("Wifi connected: ip=%s\r\n", WiFi.localIP().toString().c_str());
    }
    connected = true;
    
    if (!mqttClient.connected()) {
      reconnect();
    }
    mqttClient.loop();
    
  } else {
    if (connected) {
      connected = false;
      Serial.println("Lost wifi connection.");
    }
  }


}
