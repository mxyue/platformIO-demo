#include "filesys.h"
#include "config.h"
#include "wifiap.h"
#include "mqtt.h"
#include "mp3.h"
#include "time.h"


void setup(){
  Serial.begin(115200);
  Filesys::Setup();
  Config::Setup();
  WifiAP::Setup();
  MQTT::Setup();
  Mp3::Setup();
  Time::Setup();
}

void loop(){
  WifiAP::Loop();
  MQTT::Loop();
  Mp3::Loop();
  Time::Loop();
}
