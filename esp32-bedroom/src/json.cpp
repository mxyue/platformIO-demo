#include "json.h"

typedef StaticJsonDocument<200> SJSON;
typedef DynamicJsonDocument DJSON;

// json object to string 
String JSON::ObjToStr(DJSON obj){
  String resStr;
  serializeJson(obj, resStr);
  return resStr;
}

//需要clear
DJSON JSON::StrToObj(String str){
  DJSON doc(512);
  DeserializationError error = deserializeJson(doc, str);
  if (error) {
    Serial.print("strToObj deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  return doc;
}

