
#ifndef MQTT_H
#define MQTT_H

#include "json.h"

class MQTT{
private:
  static void callback(char *topic, byte *payload, uint length);
  static void reconnect();
  static bool feedback(String topic, DJSON doc);
  static void handler(DJSON doc);

public:
  static void Setup();
  static void Loop();
  // static void Publish(DJSON data);
  // static void publishNumber(String field, int value);
};

#endif
