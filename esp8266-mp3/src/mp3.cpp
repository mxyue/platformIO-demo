#include "mp3.h"

#include <Arduino.h>
#include "mqtt.h"
#include "time.h"
#include "json.h"
#include "pin.h"
#include "request.h"
#include "config.h"

String currentTaskId = "";

void sendFeedBack(int status){
  if(currentTaskId != ""){
    Request::Feedback(currentTaskId, status);
    currentTaskId = "";
  }
}

void publishMp3PlayState(String state){
  SJSON doc;
  doc["T"] = "V";
  JsonArray data = doc.createNestedArray("V");
  JsonObject field1 = data.createNestedObject();
  field1["F"] = "playState";
  field1["E"] = state;

  Request::Upload(JSON::ObjToStr(doc));
}


//获取喇叭电平的均方差,大于1表示有音频输出
unsigned long preAudioTimestamp; //最近有声音的时间戳
unsigned long cmdTimestamp; //命令发起时间
DJSON mp3Data(2048); //记录当前命令的状态
// cmd字段, 对应的值有 play,pause, power-on, power-off
String CMD_play = "play";
String CMD_pause = "pause";
String CMD_power_on = "power-on";
String CMD_power_off = "power-off";

//subcmd 字段,表示有些前置条件,比如 play,需要 power-on (保留)
//status 表示命令进行的阶段, do, done , audit, [completed, error] 
String mp3Power = "on"; 
String jsonNull = "null";

extern unsigned long timestamp;


//按下按键
void downMp3Btn(int16_t pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
//释放按键
void upMp3Btn(int16_t pin){
  pinMode(pin, INPUT);
}
//点击按键
void clickMp3Btn(int16_t pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(210);
  pinMode(pin, INPUT);
}

//开启音响电源
void subwooferOpen(){
  // Serial.print("pin value:");
  // Serial.println(digitalRead(subwooferPin));
  if(digitalRead(subwooferPin) == LOW){
    Serial.println("开启音响");
    digitalWrite(subwooferPin, HIGH);
  }
  sendFeedBack(digitalRead(subwooferPin) == HIGH ? Config::FB_SUCCESS : Config::FB_FAILED);
 
}

//关闭音响电源
void subwooferClose(){
  if( digitalRead(subwooferPin) == HIGH){
    Serial.println("关闭音响");
    digitalWrite(subwooferPin, LOW);
  }
  sendFeedBack(digitalRead(subwooferPin) == LOW ? Config::FB_SUCCESS : Config::FB_FAILED);

}

int *getVoicePinValue(){
  int count = 8;
  int ele[count];
  
  int total = 0;
  for(int i = 0; i < count; i++){
    int voiceValue = analogRead(inputVoicePin);
    delay(10);
    total += voiceValue; 
    ele[i] = voiceValue;
  }
  int mean = total/count;
  int sum = 0;
  for(int i = 0; i < count; i++){
    sum += pow(ele[i] - mean, 2);
  }
  int meanSquare = sum/count;
  // Serial.printf("voice mean square: %d\n", meanSquare);
  static int result[2];
  result[0] = mean;
  result[1] = meanSquare > 25;
  // Serial.printf("-------mean: %d, meanSquare: %d \n", mean, meanSquare);
  if(result[1] == true){ //将最近的声音时间记起来
    preAudioTimestamp = timestamp;
  }
  return result;
}

int8 hasAudio(){
  int* res = getVoicePinValue();
  if((bool)res[1]){
    return 1;
  }
  return 0;
}

//更新最近的声音时间
void refreshVoiceTime(){
  getVoicePinValue();
  // {"type":"value","data":[{"to":"device","field":"playState","value": "play"}]}
  
  if(timestamp % 5 == 0){ //大概5秒上传一次 音乐播放状态
    String state = timestamp - preAudioTimestamp <= 2 ? "play" : "stop";
    if(state == "play"){
      subwooferOpen(); //有声音,检查音响是否打开
    }
    publishMp3PlayState(state);
  }
}

//下一首
void map3NextAudio(){
  Serial.println("下一首");
  clickMp3Btn(nextPin);
  sendFeedBack(Config::FB_SUCCESS);
}

//播放
void playMusic(){
  String status = mp3Data["status"];

  if(status == "start"){
    if(hasAudio() == 1){
      //已经在播放, 过1秒钟再检查
      mp3Data["status"] = "audit";
      mp3Data["preDoActionMillis"] = millis();
      Serial.println("已经在播放, 过1秒钟再检查");
    }else if(timestamp - preAudioTimestamp <= 2){
      //之前还有声音,再等待
      // mp3Data["status"] = "wait";
      // return; 
    }else if(timestamp - preAudioTimestamp >= 4){
      clickMp3Btn(startPin); //点击
      Serial.println("点击播放");
      mp3Data["status"] = "done";
      mp3Data["preDoActionMillis"] = millis();
      return;
      //进入下阶段,如果有声音,则状态改成audit,没声音则进行开机
    }
  }else if(status == "done"){
    if(timestamp - preAudioTimestamp > 3){ //3秒内无声音
      Serial.println(" 进行播放前开机");
      mp3Data["subcmd"] = CMD_power_on; //进行开机
      mp3Data["status"] = "start";
      mp3Data["preDoActionMillis"] = millis();
      return;
    }
  }else if(status == "audit"){
    if(timestamp - preAudioTimestamp <= 2){
      Serial.println("审核完成,已经在播放");
      mp3Data["cmd"] = jsonNull;
      mp3Data["preDoActionMillis"] = 0;
      Serial.println("play action completed");
      sendFeedBack(Config::FB_SUCCESS);
    }
  }
}

//暂停
void pauseMusic(){
  String status = mp3Data["status"];
  if(status == "start"){
    if(hasAudio() == 1 || timestamp - preAudioTimestamp <= 1){
      //已经在播放, 点击暂停
      mp3Data["status"] = "done";
      mp3Data["preDoActionMillis"] = millis();
      clickMp3Btn(startPin);
      Serial.println("点击暂停");
      return;
    }
  }
  if(timestamp - preAudioTimestamp > 2){
    // 2秒内没有声音
    if(mp3Data["status"] == "done"){ 
      //经过点击 出现2秒内无声音,估计已经暂停
      mp3Data["cmd"] = jsonNull;
      return;
    }
    mp3Data["status"] = "audit"; //将状态切为状态确认
    mp3Data["preDoActionMillis"] = millis();
  }
  if(timestamp - preAudioTimestamp <=4){
    //如果4秒内出现了声音,将状态切回 start,重新执行
    mp3Data["status"] = "start"; 
    mp3Data["preDoActionMillis"] = millis();
    return;
  }
  
  if(timestamp - preAudioTimestamp > 4){
    // 4秒内没有声音
    mp3Data["cmd"] = jsonNull;
    sendFeedBack(Config::FB_SUCCESS);
  }
}

//开机
void map3PowerOn(String cmdkey){
  if(hasAudio() == 1){
    //已经开机
    Serial.println("已经开机");
    upMp3Btn(startPin);
    mp3Data[cmdkey] = jsonNull;
    mp3Data["preDoActionMillis"] = millis();
    mp3Data["status"] = "start";
    mp3Power = "on";
    return;
  }
  String status = mp3Data["status"];
  
  if(status == "start"){
    mp3Data["preDoActionMillis"] = millis();
    if(timestamp - preAudioTimestamp <= 1){
      //最近1秒内有声音,等待1秒再处理
      mp3Data["status"] = "audit";
      Serial.println("最近有声音,进入检查阶段");
      return;
    }
    downMp3Btn(startPin);
    Serial.println("开机按下");
    mp3Data["preDoActionMillis"] = millis();
    mp3Data["status"] = "done";
    return;
  }else if(status == "done"){
    if(cmdkey == "subcmd" && timestamp - preAudioTimestamp <= 3){
      //已经开机, 清除subcmd,执行之前的操作
      Serial.println("已经开机,释放按钮");
      upMp3Btn(startPin);
      mp3Data["subcmd"] = jsonNull;
      mp3Data["status"] = "start";
      return;
    }
    unsigned long preDoActionMillis = mp3Data["preDoActionMillis"];
    if(millis() - preDoActionMillis > 6000 ){ //6秒后
      upMp3Btn(startPin); //开机完成释放
      mp3Data[cmdkey] = jsonNull;
      mp3Power = "on";
      //释放后需要等3秒才能操作
      mp3Data["status"] = "start";
      mp3Data["preDoActionMillis"] = millis() + 3000;
      Serial.println("开机完成"); 
      // serializeJson(mp3Data, Serial);
      // Serial.println(""); 
    }
  }else if(status == "audit"){
    if(timestamp - preAudioTimestamp <= 1){
      //最近1秒内有声音,等待1秒再处理
      mp3Data["status"] = "audit";
      mp3Data[cmdkey] = jsonNull;
      mp3Data["preDoActionMillis"] = millis();
      mp3Power = "on";
      sendFeedBack(Config::FB_SUCCESS);
      return;
    }
  }
}

//关机
void map3PowerOff(){
  subwooferClose(); //关闭音响
  String status = mp3Data["status"];
  if(status == "start"){
    downMp3Btn(startPin);
    mp3Data["preDoActionMillis"] = millis();
    mp3Data["status"] = "done";
    return;
  }
  if(status == "done"){
    unsigned long preDoActionMillis = mp3Data["preDoActionMillis"];
    if(millis() - preDoActionMillis > 4000 ){ //4秒后
      upMp3Btn(startPin); //关机完成释放
      mp3Data["cmd"] = jsonNull;
      Serial.println("关机完");
      publishMp3PlayState("stop");
      mp3Data["preDoActionMillis"] = millis();
      mp3Power = "off";
      sendFeedBack(Config::FB_SUCCESS);
    }
  }
}

//设置播放,暂停,开机,关机
void setMp3Action(String action){
  Serial.printf("set mp3 action: %s \n", action.c_str());
  mp3Data["cmd"].set(action);
  mp3Data["status"].set("start");
  mp3Data["subcmd"].set(jsonNull);
  // serializeJson(mp3Data, Serial);
}

void Mp3::MqttHandler(DJSON doc){
  String function = doc["function"];
  Serial.printf("mp3 function: %s \n", function.c_str());
  String taskId = doc["ID"].as<String>();
  
  if(currentTaskId != ""){
    if(timestamp - cmdTimestamp < 15){
      Request::Feedback(taskId, Config::FB_FAILED);
      Serial.println("还有任务没执行完,直接返回");
      return;
    }else{
      //15秒后,如果前一个命令还没执行完,将前一个命令设置为超时
      sendFeedBack(Config::FB_TIMEOUT);
    }
  }
  currentTaskId = taskId;
  cmdTimestamp = timestamp;

  if(function == "audio-next"){
    map3NextAudio();
  }else if(function == "play"){
    setMp3Action(CMD_play);
  }else if(function == "pause"){
    setMp3Action(CMD_pause);
  }else if(function == "power-on"){
    setMp3Action(CMD_power_on);
  }else if(function == "power-off"){
    setMp3Action(CMD_power_off);
  }else if(function == "subwoofer-on"){
    subwooferOpen();
  }else if(function == "subwoofer-off"){
    subwooferClose();
  }
}


void Mp3::Setup(){
  pinMode(subwooferPin, OUTPUT);
}

unsigned long preLoopMillis;
unsigned int autoCloseMp3Second = 5*60; //5分钟没声音则关机

void Mp3::Loop(){
  if(millis() - preLoopMillis < 1000){ 
    return;
  }
  preLoopMillis = millis();
  
  //大于1秒进入声音采集, 长时间没有声音也不采集
  if(mp3Power == "on"){
    refreshVoiceTime();
  }
  
  String cmd = mp3Data["cmd"];
  if(cmd == jsonNull){
    if(mp3Power == "on" && timestamp - cmdTimestamp  > autoCloseMp3Second && millis() > autoCloseMp3Second*1000 && timestamp - preAudioTimestamp > autoCloseMp3Second){
      //开机超过5分钟,且5分钟无操作, 5分钟没声音,则执行mp3关机命令
      Serial.println("5分钟无声音关机");
      setMp3Action(CMD_power_off);
    }
    return; //没命令,直接返回
  }

  if(timestamp - cmdTimestamp > 20 ){ //超过20秒的命令,不再执行
    if(cmd == jsonNull){
      mp3Data["cmd"] = jsonNull;
    }
    return;
  }

  unsigned long preDoActionMillis = mp3Data["preDoActionMillis"]; //上次行动的时间,
  if(millis() < preDoActionMillis || millis() - preDoActionMillis < 1000 ){ //1秒内不频繁操作,直接返回
    return;
  }

  String subcmd = mp3Data["subcmd"];
  if(subcmd == CMD_power_on){
    map3PowerOn("subcmd");
    return;
  }else if(subcmd == jsonNull){
    if(cmd == CMD_play){
      playMusic();
    }else if(cmd == CMD_pause){
      pauseMusic();
    }else if(cmd == CMD_power_on){
      map3PowerOn("cmd");
    }else if(cmd == CMD_power_off){
      map3PowerOff();
    }
  }
}
