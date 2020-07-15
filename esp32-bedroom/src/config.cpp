#include "config.h"
#include "filesys.h"

char configFile[] = "/config.json";

// const char *Config::MQTT_HOST = "192.168.1.15";
// String Config::HOST = "http://192.168.1.15:4000/dv";

const char *Config::MQTT_HOST = "mxyue.com";
String Config::HOST = "http://mxyue.com/dv";

int Config::FB_SUCCESS = 2;
int Config::FB_FAILED = 3;

DJSON Config::Data(512);

void Config::Setup(){
  String datastr = Filesys::GetContent(configFile);
  Config::Data = JSON::StrToObj(datastr);
}

SJSON Config::GetHttpConfig(){
  SJSON doc;
  doc["username"] = Config::Data["username"];
  doc["password"] = Config::Data["password"];
  return doc;
}

DJSON Config::GetMqttConfig(){
  DJSON doc(256);
  String houseId = Config::Data["houseId"];
  String deviceNo = Config::Data["deviceNo"];
  doc["clientId"] = houseId + "-" + deviceNo;
  doc["username"] = Config::Data["username"];
  doc["password"] = Config::Data["password"];

  JsonArray subscribes = doc.createNestedArray("subscribes");
  subscribes.add("iot");
  subscribes.add("heart");
  // String houseTopic = "houses/"+houseId;
  subscribes.add("houses/"+houseId + "/devices/"+deviceNo+"");
  
  return doc;
}

JsonArray Config::GetWifiConfig(){
  return Config::Data["wifi"];
}

SJSON Config::GetApConfig(){
  return Config::Data["ap"];
}