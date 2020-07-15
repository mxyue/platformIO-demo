#include "pin.h"
#include "ircontrol.h"
#include <IRremote.h>

IRsend irsend;
//接收的引脚定义在 Arduino-IRremote 的 boarddefs.h里的 TIMER_PWM_PIN, esp32 的引脚默认为GPIO5
// https://github.com/ExploreEmbedded/Arduino-IRremote/blob/master/boarddefs.h

void IRControl::IRSendNEC(String data){
  Serial.printf("ir will send key: %s \n", data.c_str());
  irsend.sendNEC(strtol(data.c_str(), NULL, 16), 32);
}

void IRControl::Setup(){
  pinMode(IRSEND_PIN, OUTPUT);
  digitalWrite(IRSEND_PIN, LOW);
}

void IRControl::MqttHandler(DJSON doc){
  String function = doc["function"];
  if(function == "sendKey"){
    String key = doc["key"];
    IRControl::IRSendNEC(key);
  }
}