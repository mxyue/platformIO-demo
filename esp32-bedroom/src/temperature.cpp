#include "temperature.h"
#include <Arduino.h>
#include "json.h"
#include "request.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

unsigned long preTempMillis;

void Temperature::publishTemperature(int temp)
{
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "boardTemp";
  field1["E"] = temp;

  Serial.print(temp);
  Serial.println(" C");
  Request::Upload(JSON::ObjToStr(doc));
}

void Temperature::Loop(){
  if(millis() - preTempMillis < 30*1000){
    return;
  }
  preTempMillis = millis();
  int temp = (temprature_sens_read() - 32) / 1.8;
  Temperature::publishTemperature(temp);
}
