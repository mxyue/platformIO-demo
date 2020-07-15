#include "pin.h"
#include "illumination.h"
#include "request.h"
#include "config.h"

unsigned long preIlluminationMillis;
unsigned long preLoopTime;
int preLuxInner;

const unsigned int MAX_VALUE = 4095;

void Illumination::pushValue(int outer, int inner){
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "luxOuter";
  field1["E"] = outer;

  JsonObject field2 = data.createNestedObject();
  field2["F"] = "luxInner";
  field2["E"] = inner;

  Request::Upload(JSON::ObjToStr(doc));
}

int Illumination::getInnerLux(){
  return MAX_VALUE - analogRead(ILLUMINATION_INNER_PIN);
}

int Illumination::getOuterLux(){
  return (MAX_VALUE - analogRead(ILLUMINATION_OUTER_PIN)) * 0.83;
}

void Illumination::Setup(){
  pinMode(ILLUMINATION_OUTER_PIN, INPUT);
  pinMode(ILLUMINATION_INNER_PIN, INPUT);
}

void Illumination::Loop(){
  if(millis() - preLoopTime < 1000){
    return;
  }
  preLoopTime = millis();
  
  int inner = Illumination::getInnerLux();
  if(millis() - preIlluminationMillis < 60*1000){
    if(abs(preLuxInner - inner) < 300 ){
      return;
    }
  }
  preLuxInner = inner;
  preIlluminationMillis = millis();
  int outer = Illumination::getOuterLux();

  Serial.printf("outer: %d, inner: %d\n", outer, inner);
  Illumination::pushValue(outer, inner);
}


void Illumination::MqttHandler(DJSON doc){
  String function = doc["function"];
  String taskId = doc["ID"];
  if(function == "getValue"){
    int inner = Illumination::getInnerLux();
    int outer = Illumination::getOuterLux();
    Illumination::pushValue(outer, inner);
    Request::Feedback(taskId, Config::FB_SUCCESS);
  }
}