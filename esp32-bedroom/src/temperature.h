
#ifndef TEMPERATURE_H
#define TEMPERATURE_H

class Temperature{
private:
  static void publishTemperature(int temp);

public:
  static void Loop();
};

#endif 