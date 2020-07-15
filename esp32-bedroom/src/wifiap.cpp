#include <WiFi.h>
#include <WiFiMulti.h>
#include "filesys.h"
#include "json.h"
#include "wifiap.h"
#include "config.h"

WiFiMulti wifiMulti;

unsigned long wifiPreLoopTime;
bool wifiFirstConnected = false;

extern char wifiConfigFile[];
bool softApOpen = false;

void WifiAP::Setup(){

  //配置热点
  SJSON apConfig = Config::GetApConfig();
  const char* apSSID = apConfig["ssid"].as<const char*>();
  const char* apPWD = apConfig["password"].as<const char*>();

  Serial.printf("soft ap ssid: %s, password: %s\n",apSSID, apPWD);
  bool success = WiFi.softAP(apSSID, apPWD );
  softApOpen = success;
  Serial.println(success ? "SOFT AP Ready" : "SOFT AP Failed!");
  Serial.print("Soft-AP IP address: ");
  Serial.println(WiFi.softAPIP());

  //配置WiFi
  JsonArray stationArr = Config::GetWifiConfig();
  Serial.println("wifi connect list:");
  for(JsonVariant item : stationArr) {
    serializeJson(item, Serial);
    const char* ssid = item["ssid"].as<char*>();
    const char* password = item["password"].as<char*>();
    wifiMulti.addAP(ssid, password);
  }

}

unsigned long preprint;

void WifiAP::Loop(){
  if(wifiMulti.run() == WL_CONNECTED && wifiFirstConnected == false) {
    Serial.printf("wifi connected, local ip is: %s \n", WiFi.localIP().toString().c_str());
    
    wifiFirstConnected = true;
  }
  
  //检查连接状况,如果3分钟后WiFi已经连接,则关闭热点
  if(softApOpen == false ||  millis() - wifiPreLoopTime < 5000 ){
    return;
  }
  wifiPreLoopTime = millis();
  if(WiFi.status() != WL_CONNECTED){
    Serial.printf("wifi connected failed, status: %d \n", WiFi.status());
  }

  //wifi连接成功 3分钟后关闭AP
  if(millis() > 3*60*1000 && WiFi.status() == WL_CONNECTED ){
    Serial.println("close soft ap!");
    // softApOpen = !WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.printDiag(Serial);
    softApOpen = false;
  }
}

