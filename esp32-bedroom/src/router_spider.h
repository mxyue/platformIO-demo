
#ifndef ROUTER_SPIDER_H
#define ROUTER_SPIDER_H
#include <Arduino.h>
#include "json.h"

class RouterSpider{
private:
  static void publishInHouse(String status);
  static int login();
  static int getIndex();
  static String fetchPayload();
  static String AutoGetPayload();
  static bool checkMacPresent(String payload);
  static void uploadAddresses(String taskId, String payload);
  static int loopDelayTime;
  static void setDefaultDelayTime();

public:
  static void MqttHandler(DJSON doc);
  static void Setup();
  static void Loop();
};

#endif 