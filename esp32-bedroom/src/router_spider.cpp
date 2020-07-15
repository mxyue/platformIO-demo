#include "router_spider.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "filesys.h"
#include "request.h"
#include "config.h"

unsigned long preSpierMillis;

char inHouseMacPath[] = "/in-house-mac.txt";
String inHouseMac = "";

int RouterSpider::loopDelayTime = 10;

void RouterSpider::setDefaultDelayTime(){
  RouterSpider::loopDelayTime = 10;
}

void RouterSpider::publishInHouse(String status)
{
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "house";
  field1["E"] = status;

  Request::Upload(JSON::ObjToStr(doc));
}

int getDeviceNum(String content)
{
  String deviceNumKey = "var DeviceNum =";
  int startLine = content.indexOf(deviceNumKey) + deviceNumKey.length();
  int newLine = content.indexOf(";\n", startLine);
  String numStr = content.substring(startLine, newLine);
  numStr.trim();
  return numStr.toInt();
}

String getKeyValue(String content, String key, int index)
{
  String prefix = key + "[" + index + "] = '";
  int prefixIndex = content.indexOf(prefix) + prefix.length();
  int endIndex = content.indexOf("';\n", prefixIndex);
  if (endIndex - prefixIndex < 1)
  {
    return "";
  }
  String valueStr = content.substring(prefixIndex, endIndex);
  valueStr.trim();
  return valueStr;
}

void RouterSpider::uploadAddresses(String taskId, String payload)
{
  int count = getDeviceNum(payload);
  if(count == 0){
    Serial.println("没有拉取到连接设备");
    Request::Feedback(taskId, Config::FB_FAILED);
    return;
  }
  DJSON doc(1024);
  doc["T"] = "A";
  JsonArray data = doc.createNestedArray("V");

  for (int i = 0; i < count; i++)
  {
    String host = getKeyValue(payload, "PortHostName", i);
    String ip = getKeyValue(payload, "PortIp", i);
    String mac = getKeyValue(payload, "PortMac", i);
    // Serial.printf("host: %s, ip: %s, mac: %s \n", host.c_str(), ip.c_str(), mac.c_str());
    JsonObject field = data.createNestedObject();
    field["name"] = host;
    field["ip"] = ip;
    field["mac"] = mac;
  }
  // Serial.print("address:");
  // Serial.println(JSON::ObjToStr(doc));
  String res = Request::Upload(JSON::ObjToStr(doc));
  Serial.print("up address res:");
  Serial.println(res);

  if(Request::ResInvalid(res)){
    Serial.println("spider返回格式错误");
    Request::Feedback(taskId, Config::FB_FAILED);
    return;
  }
  Request::Feedback(taskId, Config::FB_SUCCESS);
}

int RouterSpider::login()
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://192.168.1.1/cgi-bin/index2.asp");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Cookie", "UID=user; PSW=fe82187542608474d000ae6152a246c8; SESSIONID=boasid169f59a9");

  int httpCode = http.POST("Username=user&Password=jc24f");

  if (httpCode > 0)
  {
    if (httpCode != HTTP_CODE_OK)
    {
      Serial.printf("[HTTP] POST to login failed, code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] POSt... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return httpCode;
}

int RouterSpider::getIndex()
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://192.168.1.1/cgi-bin/content.asp");
  http.addHeader("Cookie", "UID=user; PSW=fe82187542608474d000ae6152a246c8; SESSIONID=boasid169f59a9");

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    if (httpCode != HTTP_CODE_OK)
    {
      Serial.printf("[HTTP] GET index code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return httpCode;
}

String RouterSpider::fetchPayload()
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://192.168.1.1/cgi-bin/lanHostInfo.cgi");
  http.addHeader("Cookie", "SESSIONID=boasid169f59a9");
  String payload;

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
      // Serial.println(payload);
    }
    else
    {
      Serial.println("fetch keyword error");
      Serial.printf("[HTTP] GET client ip list code: %d\n", httpCode);
      if (httpCode == 404)
      {
        RouterSpider::loopDelayTime = 60 * 10; //10分钟
      }
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return payload;
}

bool RouterSpider::checkMacPresent(String payload)
{
  if (inHouseMac == "")
  {
    return false;
  }
  return payload.indexOf(inHouseMac) != -1;
}

String RouterSpider::AutoGetPayload()
{
  int httpCode = RouterSpider::getIndex();
  if (httpCode != HTTP_CODE_OK)
  {
    int loginCode = RouterSpider::login();
    if ( loginCode != HTTP_CODE_OK)
    {
      if (loginCode == 404)
      {
        //登录接口不存在,增加登录周期的时间
        RouterSpider::loopDelayTime = 60 * 1000;
      }
      return "";
    }
    RouterSpider::getIndex();
    return ""; //等待下一个周期
  }
  return RouterSpider::fetchPayload();
}

void RouterSpider::Setup()
{
  inHouseMac = Filesys::GetContent(inHouseMacPath);
  Serial.printf("inHouseMac: %s\n", inHouseMac.c_str());
}

void RouterSpider::Loop()
{
  if (millis() - preSpierMillis < RouterSpider::loopDelayTime * 1000 || WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  preSpierMillis = millis();
  if (inHouseMac == "")
  {
    return;
  }

  String payload = RouterSpider::AutoGetPayload();

  if (payload == "")
  {
    return;
  }
  String status = RouterSpider::checkMacPresent(payload) ? "in" : "out";
  Serial.printf("house %s \n", status.c_str());
  RouterSpider::publishInHouse(status);
}

void RouterSpider::MqttHandler(DJSON doc)
{
  String function = doc["function"];
  String taskId = doc["ID"];
  if (function == "setHouseMac")
  {
    String value = doc["value"];
    inHouseMac = value;
    Filesys::SetContent(inHouseMacPath, value);
  }
  else if (function == "uploadAddresses")
  {
    Serial.println("----uploadAddresses----");
    String payload = RouterSpider::AutoGetPayload();
    if (payload == "")
    {
      Serial.println("spider拉取没数据");
      Request::Feedback(taskId, Config::FB_FAILED);
      return;
    }
    RouterSpider::uploadAddresses(taskId, payload);
  }
}