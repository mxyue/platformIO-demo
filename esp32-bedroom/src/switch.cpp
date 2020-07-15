#include "switch.h"
#include "pin.h"
#include "request.h"
#include "ircontrol.h"
#include "illumination.h"
#include "config.h"
#include <ArduinoQueue.h>

struct Task
{
  String ID;
  String function;
  String value;
  unsigned long milliTime;
  int preNumber;
};

String Switch::FUNC_PI = "pi";
String Switch::FUNC_LIGHT1 = "light1";
String Switch::VALUE_ON = "on";
String Switch::VALUE_OFF = "off";

ArduinoQueue<Task> taskQueue(5);

void Switch::switchLight()
{
  IRControl::IRSendNEC("FF02FD");
}

void Switch::openLight1(String taskId)
{
  int preLux = Illumination::getInnerLux();
  if (Illumination::getOuterLux() < preLux)
  {
    Serial.println("light1 is opened");
    Request::Feedback(taskId, Config::FB_SUCCESS);
    return;
  }
  Switch::switchLight();
  Task task = {taskId, Switch::FUNC_LIGHT1, Switch::VALUE_ON, millis() + 1000, preLux};
  taskQueue.enqueue(task);
}

void Switch::closeLight1(String taskId)
{
  int innerLux = Illumination::getInnerLux();
  if(innerLux == 0){
    Serial.println("light1 is closed");
    Request::Feedback(taskId, Config::FB_SUCCESS);
    return;
  }
  Switch::switchLight();
  Task task = {taskId, Switch::FUNC_LIGHT1, Switch::VALUE_OFF, millis() + 1000, innerLux};
  taskQueue.enqueue(task);
}

void Switch::checkTask(Task task)
{
  if (task.function == Switch::FUNC_LIGHT1)
  {
    // Serial.printf("light1 taskId %s, millstime: %ld, now: %ld, preNum: %d;, noNum: %d, ", task.ID.c_str(), task.milliTime, millis(), task.preNumber, Illumination::getInnerLux());
    bool success = false;
    if(task.value == Switch::VALUE_ON){
      success = Illumination::getInnerLux() - task.preNumber > 100;
    }else if(task.value == Switch::VALUE_OFF){
      success = Illumination::getInnerLux() - task.preNumber < 100;
    }
    if (success)
    {
      // Serial.printf("%s success\n", task.value.c_str());
      Request::Feedback(task.ID, Config::FB_SUCCESS);
    }
    else
    {
      // Serial.printf("%s failed\n", task.value.c_str());
      Request::Feedback(task.ID, Config::FB_FAILED);
    }
  }
  taskQueue.dequeue();
}

void Switch::Setup()
{
  pinMode(PI_PIN, OUTPUT);
  digitalWrite(PI_PIN, LOW);
}

void Switch::Loop()
{
  if (!taskQueue.isEmpty())
  {
    Task task = taskQueue.getHead();
    if (task.milliTime > millis())
    {
      return;
    }
  
    Switch::checkTask(task);
  }
}

void Switch::MqttHandler(DJSON doc)
{
  String function = doc["function"];
  String taskId = doc["ID"];
  String value = doc["value"];

  if (function == Switch::FUNC_PI)
  {
    if (value == Switch::VALUE_ON)
    {
      Serial.println("打开 pi");
      digitalWrite(PI_PIN, HIGH);
      Request::Feedback(taskId, digitalRead(PI_PIN) == HIGH ? Config::FB_SUCCESS : Config::FB_FAILED);
    }
    else if (value == Switch::VALUE_OFF)
    {
      Serial.println("关闭 pi");
      digitalWrite(PI_PIN, LOW);
      Request::Feedback(taskId, digitalRead(PI_PIN) == LOW ? Config::FB_SUCCESS : Config::FB_FAILED);
    }
  }
  else if (function == Switch::FUNC_LIGHT1)
  {
    if (value == Switch::VALUE_ON)
    {
      Switch::openLight1(taskId);
    }
    else if (value == Switch::VALUE_OFF)
    {
      Switch::closeLight1(taskId);
    }
  }
}
