#include "pin.h"
#include "request.h"
#include "collroom.h"

unsigned long preCollRoomTime;
int roomCheckCount = 0;


void CollRoom::publishRoomState(String state){
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "bedroom";
  field1["E"] = state;

  Request::Upload(JSON::ObjToStr(doc));
}

void CollRoom::Setup(){
  pinMode(ROOM_CHECK_PIN, INPUT);
}

void CollRoom::Loop(){

  char roomOut = digitalRead(ROOM_CHECK_PIN);
  if(roomOut == 0){
    return;
  }
  if(millis() < 10*1000 ||  millis() - preCollRoomTime < 5*1000 ){
    return;
  }
  Serial.println("roomOut");

  preCollRoomTime = millis();

  CollRoom::publishRoomState("out");

}
