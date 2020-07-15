#ifndef MEMORY_H
#define MEMORY_H
#include "json.h"

class Memory{

private:
  static void pushValue();

public:
  static void MqttHandler(DJSON doc);

};

#endif 
