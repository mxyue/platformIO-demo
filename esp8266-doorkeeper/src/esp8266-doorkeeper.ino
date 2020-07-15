#include <Arduino.h>

#include "filesys.h"
#include "config.h"
#include "wifiap.h"
#include "mqtt.h"
#include "collroom.h"

void setup() {
  Serial.begin(115200);
  Filesys::Setup();
  Config::Setup();
  WifiAP::Setup();
  MQTT::Setup();
  CollRoom::Setup();
}

void loop() {
  WifiAP::Loop();
  MQTT::Loop();
  CollRoom::Loop();
}
