
#ifndef COLLBED_H
#define COLLBED_H
#include <Arduino.h>

class CollBed{
private:
  static void publishBedState(String state);  

public:
  static void Setup();
  static void Loop();
};

#endif
