#include "config.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <Msgflo.h>


#include <SoftwareSerial.h>



#include <Wire.h>
#include "rgb_lcd.h"

#include <ArduinoJson.h>

#define ARDUINO_RX 13  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 15  //connect to RX of the module

SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY


/************ Command byte **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f


/************ Opitons **************************/
#define DEV_TF            0X02



struct Config {
  const String prefix = "";
  const String role = "c-gravel-lcd";

  const int blueButton = D4;

  const int mp3rx = D7;
  const int mp3tx = D8;

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
msgflo::InPort *soundPort;
msgflo::OutPort *bluePort;
long nextButtonCheck = 0;
long nextPeriodicUpdate = 0;

long nextPeriodicUpdateMp3 = 0;

auto participant = msgflo::Participant("c-base/screen", cfg.role);

boolean notplaying;


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
    lcd.print(message.c_str());

}


void callbackSound(byte *payload, int length) {
 std::string message((const char *)(payload),length);
 int num = atoi(message.c_str());
 sendCommand(CMD_PLAY_W_INDEX, num);
}
void setup() {
  Serial.swap();
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

  sendCommand(CMD_SEL_DEV, DEV_TF);

  engine = msgflo::pubsub::createPubSubClientEngine(participant, &mqttClient, clientId.c_str(), cfg.mqttUsername, cfg.mqttPassword);
  farbgeberPort = engine->addInPort("colors", "object", cfg.prefix+cfg.role+"/colors", callback);
  textPort = engine-> addInPort("text", "object", cfg.prefix+cfg.role+"/text", callbackText);
  soundPort = engine-> addInPort("sound", "object", cfg.prefix+cfg.role+"/sound", callbackSound);
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


/********************************************************************************/
/*Function: Send command to the MP3                                         */
/*Parameter:-int8_t command                                                     */
/*Parameter:-int16_ dat  parameter for the command                              */

void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e;   //
  Send_buf[1] = 0xff;   //
  Send_buf[2] = 0x06;   // Len
  Send_buf[3] = command;//
  Send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  Send_buf[5] = (int8_t)(dat >> 8);  //datah
  Send_buf[6] = (int8_t)(dat);       //datal
  Send_buf[7] = 0xef;   //
 
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]) ;
  }
  
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                 */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */


String sbyte2hex(uint8_t b)
{
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}
