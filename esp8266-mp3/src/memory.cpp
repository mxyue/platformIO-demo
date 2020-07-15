#include "memory.h"
#include "request.h"

extern "C" {
  #include "user_interface.h"
}

void Memory::pushValue(){
  SJSON doc;
  uint32_t free = system_get_free_heap_size();
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field = data.createNestedObject();
  field["F"] = "freeRam";
  field["E"] = free;
  Serial.printf("memory: %d\n", free);
  Request::Upload(JSON::ObjToStr(doc));
}

void Memory::MqttHandler(DJSON doc){
  String function = doc["function"];
  if(function == "getValue"){
    Memory::pushValue();
  }
}