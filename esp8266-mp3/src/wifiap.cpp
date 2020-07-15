#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "filesys.h"
#include "json.h"
#include "wifiap.h"
#include "config.h"

ESP8266WiFiMulti wifiMulti;

bool wifiFirstConnected = false;


void WifiAP::Setup(){

  //配置WiFi
  JsonArray stationArr = Config::GetWifiConfig();
  Serial.println("wifi connect list:");
  for(JsonVariant item : stationArr) {
    serializeJson(item, Serial);
    Serial.println("");
    const char* ssid = item["ssid"].as<char*>();
    const char* password = item["password"].as<char*>();
    wifiMulti.addAP(ssid, password);
  }

}


void WifiAP::Loop(){
  if(wifiMulti.run() == WL_CONNECTED && wifiFirstConnected == false) {
    Serial.printf("wifi connected, local ip is: %s \n", WiFi.localIP().toString().c_str());
    
    wifiFirstConnected = true;
  }
  
}

