#ifndef CONFIG_H
#define CONFIG_H

#include "json.h"

class Config{
public:
  static String HOST;
  static const char * MQTT_HOST;
  static int FB_SUCCESS;
  static int FB_FAILED;
  static int FB_TIMEOUT;
  
  static DJSON Data;
  static void Setup();
  static DJSON GetMqttConfig();
  static SJSON GetHttpConfig();
  static JsonArray GetWifiConfig();
  static SJSON GetApConfig();

};
#endif 