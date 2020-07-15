#include <PubSubClient.h>
#include <WiFi.h>
#include "filesys.h"
#include "json.h"
#include "mqtt.h"
#include "ircontrol.h"
#include "request.h"
#include "config.h"
#include "router_spider.h"
#include "illumination.h"
#include "switch.h"


WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool MQTT::feedback(String topic, DJSON doc){
  // -------heart---------
  if (topic == "heart")
  {
    SJSON upDoc;
    upDoc["T"] = "H";
    upDoc["E"] = doc["key"];
    String postData = JSON::ObjToStr(upDoc);
    Request::Upload(postData);
    return true;
  }else{
    String taskId = doc["ID"];
    Serial.printf("taskId: %s \n", taskId.c_str());
    if( taskId != "null"){
      // SJSON upDoc;
      // upDoc["T"] = "B";
      // upDoc["ID"] = doc["ID"];
      // String postData = JSON::ObjToStr(upDoc);
      // Request::Upload(postData);
      Request::Feedback(doc["ID"], 1);
    }
  }
  return false;
}

void MQTT::handler(DJSON doc){

  String scope = doc["scope"];
  if (scope == "IRControl")
  {
    IRControl::MqttHandler(doc);
  }
  else if (scope == "RouterSpider")
  {
    RouterSpider::MqttHandler(doc);
  }
  else if(scope == "Illumination"){
    Illumination::MqttHandler(doc);
  }
  else if(scope == "Switch"){
    Switch::MqttHandler(doc);
  }
}

void MQTT::callback(char *topicChar, byte *payload, uint length)
{
  Serial.printf("mqtt receive [%s]", topicChar);
  
  String topic(topicChar);
  DJSON doc(512);
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error)
  {
    Serial.printf("mqtt deserialize json err: %s \n", error.c_str());
    return;
  }
  Serial.println(JSON::ObjToStr(doc));
  
  if(MQTT::feedback(topic, doc)){
    return;
  }

  MQTT::handler(doc);
}

void MQTT::reconnect()
{
  if (mqttClient.connected())
  {
    return;
  }

  Serial.print("MQTT connection...");
  DJSON config(512);
  config = Config::GetMqttConfig();
  const char *clientId = config["clientId"];
  const char *username = config["username"];
  const char *password = config["password"];
  if (mqttClient.connect(clientId, username, password))
  {
    Serial.println("connected");
    Serial.println("mqtt sub topic: ");
    JsonArray subscribes = config["subscribes"].as<JsonArray>();
    for (JsonVariant topic : subscribes)
    {
      String topicStr = topic;
      Serial.print(topicStr);
      Serial.print(", ");
      mqttClient.subscribe(topicStr.c_str(), 1); //只支持0和1 (最少一次)
    }
    Serial.println("");
  }
  else
  {
    Serial.print("mqtt 连接失败, 状态码=");
    Serial.println(mqttClient.state());
  }
}

void MQTT::Setup()
{
  mqttClient.setServer(Config::MQTT_HOST, 1883);
  mqttClient.setCallback(MQTT::callback);
}

unsigned long preMqttLoopMillis;

void MQTT::Loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }
  if (millis() - preMqttLoopMillis > 2000)
  {
    if (!mqttClient.connected())
    {
      MQTT::reconnect();
    }
    preMqttLoopMillis = millis();
  }
  mqttClient.loop();
}
