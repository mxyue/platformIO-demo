#include "request.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "config.h"

WiFiClient client;

String token;
String Request::NOAUTH = "noauth";
String Request::ERROR = "error";

bool Request::ResInvalid(String res){
  if(res == Request::ERROR || res == Request::NOAUTH){
    return true;
  }
  return false;
}


void Request::login()
{
  Serial.println("http login");

  HTTPClient http;
  String url = Config::HOST + "/session";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  SJSON postDoc = Config::GetHttpConfig();
  String data = JSON::ObjToStr(postDoc);
  int httpCode = http.POST(data);
  if (httpCode == 200)
  {
    DJSON res(256);
    res = JSON::StrToObj(http.getString());
    int code = res["code"];
    if (code == 0)
    {
      token = res["data"]["token"].as<String>();
      Serial.printf("token: %s\n", token.c_str());
    }
  }
  else
  {
    Serial.printf("login err: %d \n", httpCode);
  }
  http.end();
}

// bool Request::checkNeedLogin(String payload){
//   DJSON data(256);
//   data = JSON::StrToObj(payload);
//   int code = data["code"];
//   if(code == 901 || code == 902 || code == 903){
//     return true;
//   }
//   return false;
// }

String Request::get(String path)
{
  HTTPClient http;
  String url = Config::HOST + path;
  http.begin(client, url);
  Serial.printf("GET url: %s \n", url.c_str());
  http.addHeader("token", token);

  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  if (httpCode == 401)
  {
    return Request::NOAUTH;
  }
  else if (httpCode != 200)
  {
    Serial.printf("get res err, http code: %d \n", httpCode);
    return Request::ERROR;
  }

  return payload;
}

String Request::post(String path, String data)
{
  HTTPClient http;
  String url = Config::HOST + path;
  Serial.printf("POST url: %s \n", url.c_str());
  Serial.printf("POST body: %s \n", data.c_str());
  http.begin(client, url);
  http.addHeader("token", token);
  // http.addHeader("Content-Type", "text/plain");
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(data);
  String payload = http.getString();
  http.end();
  if (httpCode == 401)
  {
    return Request::NOAUTH;
  }
  else if (httpCode != 200)
  {
    Serial.printf("post res err, http code: %d \n", httpCode);
    return Request::ERROR;
  }

  return payload;
}

String Request::Get(String path)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return "";
  }
  String res = Request::get(path);
  if (res == Request::NOAUTH)
  {
    Request::login();
    return Request::get(path);
  }
  return res;
}

String Request::Post(String path, String data)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return "";
  }
  String res = Request::post(path, data);
  if (res == Request::NOAUTH)
  {
    Request::login();
    return Request::post(path, data);
  }
  return res;
}

String Request::Upload(String upDoc)
{
  String houseId = Config::Data["houseId"];
  String deviceNo = Config::Data["deviceNo"];
  return Request::Post("/houses/" + houseId + "/devices/" + deviceNo + "/upload", upDoc);
}

void Request::Feedback(String taskId, int status)
{
  SJSON upDoc;
  upDoc["T"] = "B";
  upDoc["ID"] = taskId;
  upDoc["status"] = status;
  String postData = JSON::ObjToStr(upDoc);
  Request::Upload(postData);
}