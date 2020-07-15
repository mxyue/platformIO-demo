#ifndef ILLUMINATION_H
#define ILLUMINATION_H
#include "json.h"

class Illumination{
private:
static void pushValue(int outer, int inner);

public:
  static void Setup();
  static void Loop();
  static void MqttHandler(DJSON doc);
  static int getInnerLux();
  static int getOuterLux();

};

#endif