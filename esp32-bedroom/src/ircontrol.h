#ifndef IRREMOTE_H
#define IRREMOTE_H
#include "json.h"

class IRControl{

public:
  static void Setup();
  static void MqttHandler(DJSON doc);
  static void IRSendNEC(String data);
};

#endif 