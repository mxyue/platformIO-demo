#ifndef JSON_H
#define JSON_H

#include <ArduinoJson.h>

typedef StaticJsonDocument<200> SJSON;
typedef DynamicJsonDocument DJSON;

class JSON{
public:
  static String ObjToStr(DJSON obj);
  static DJSON StrToObj(String str);
};

#endif 