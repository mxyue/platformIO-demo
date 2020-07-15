#include "pin.h"
#include "collbed.h"
#include "request.h"
#include "json.h"

unsigned long preCollBedTime;
unsigned long inBedMillis;
String preBedState;

void CollBed::publishBedState(String state){
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "bed";
  field1["E"] = state;

  Request::Upload(JSON::ObjToStr(doc));
}

void CollBed::Setup(){
  pinMode(BED_CHECK_PIN, INPUT);
}

void CollBed::Loop(){
  if(millis() < 10*1000){
    return;
  }
  if(millis() - preCollBedTime < 5*1000 ){
    return;
  }
  preCollBedTime = millis();
  char bedIn = digitalRead(BED_CHECK_PIN);
  if(bedIn == 1){
    inBedMillis = millis();
  }

  String bedState = "out";
  if(millis() - inBedMillis < 10*1000){
    bedState = "in";
  }
  Serial.printf("bed state: %s \n", bedState.c_str());
  
  if(preBedState == bedState){
    return;
  }
  preBedState = bedState;
  CollBed::publishBedState(bedState);
}
