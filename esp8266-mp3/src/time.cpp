#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "time.h"
#include "filesys.h"
#include "request.h"

unsigned long timestamp; //服务器和本地运算得来的以秒为单位的时间戳

unsigned long preTimeLoop;    //秒为单位
unsigned long preSyncMillis;    //上次同步的系统时间
unsigned long preSyncTimestamp; //毫秒为单位
char currentTimestampFile[] = "/current_timestamp.txt";

void Time::Setup()
{
  String content = Filesys::GetContent(currentTimestampFile);
  if (content != "")
  {
    timestamp = content.toInt();
    Serial.printf("init timestamp file time: %ld \n", timestamp);
  }
}

void asyncTime()
{
  if(millis() - preTimeLoop < 1000){
    return;
  }
  preTimeLoop = millis();
  int duration = millis() - preSyncMillis;
  if (duration < 60000 || preSyncTimestamp == 0)
  { //小于60秒,则自己本地更新
    timestamp = preSyncTimestamp + (duration / 1000);
    if (preSyncTimestamp != 0)
    {
      return; //如果初次已经更新,则不走下面流程
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  String payload = Request::Get("/timestamp/seconds");
  if (payload== "")
  {
    return;
  }
  Filesys::SetContent(currentTimestampFile, payload);
  timestamp = payload.toInt();
  preSyncTimestamp = timestamp;
  preSyncMillis = millis();
}

void Time::Loop()
{
  asyncTime();
}