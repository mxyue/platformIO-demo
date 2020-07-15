#ifndef MP3_H
#define MP3_H
#include "json.h"

class Mp3{

public:
  static void Setup();
  static void Loop();
  static void MqttHandler(DJSON doc);
};

#endif