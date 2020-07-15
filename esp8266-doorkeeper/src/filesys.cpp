#include <FS.h>
#include "filesys.h"

String nullStr = "";

// 获取文件内容
String Filesys::GetContent(char filepath[]){
  bool exist = SPIFFS.exists(filepath);
  if (!exist) {
    Serial.print(filepath);
    Serial.println(" --- file not present");
    return nullStr;
  }
  File f = SPIFFS.open(filepath, "r");
  if (!f) {
    Serial.printf("%s file open failed \n", filepath);
    return nullStr;
  }
  String content = f.readString();
  f.close();
  Serial.printf("[%s]-->size(%d)\n", filepath, f.size());
  return content;
}

// 设置文件内容
void Filesys::SetContent(char filepath[], String content){
  File f = SPIFFS.open(filepath, "w");
  f.print(content);
  f.close();
}

void Filesys::Setup(){
  bool fsok = SPIFFS.begin();
  if (fsok) {
    Serial.println("FS ok");
  }
}
  