
#include "filesys.h"
#include "wifiap.h"
#include "mqtt.h"
#include "router_spider.h"
#include "collbed.h"
#include "collroom.h"
#include "illumination.h"
#include "ircontrol.h"
#include "temperature.h"
#include "config.h"
#include "switch.h"

void setup() {
  Serial.begin(115200);
  Filesys::Setup();
  Config::Setup();
  WifiAP::Setup();
  MQTT::Setup();
  CollBed::Setup();
  CollRoom::Setup();
  Illumination::Setup();
  RouterSpider::Setup();
  IRControl::Setup();
  Switch::Setup();
}

void loop() {
  WifiAP::Loop();
  MQTT::Loop();
  RouterSpider::Loop();
  CollBed::Loop();
  CollRoom::Loop();
  Illumination::Loop();
  RouterSpider::Loop();
  Temperature::Loop();
  Switch::Loop();
}
