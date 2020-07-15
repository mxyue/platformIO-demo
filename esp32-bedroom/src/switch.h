#ifndef Switch_H
#define Switch_H
#include "json.h"

struct Task;

class Switch
{
private:
  static String FUNC_PI;
  static String FUNC_LIGHT1;
  static String VALUE_ON;
  static String VALUE_OFF;

  static void switchLight();
  static void openLight1(String taskId);
  static void closeLight1(String taskId);
  static void checkTask(Task task);

public:
  static void Setup();
  static void Loop();
  static void MqttHandler(DJSON doc);
};

#endif