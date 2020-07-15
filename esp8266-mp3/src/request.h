#ifndef REQUEST_H
#define REQUEST_H
#include "json.h"

class Request{
private:
  static void login();
  static String get(String path);
  static String post(String path, String data);
  // static bool checkNeedLogin(String payload);

public:
  static String NOAUTH;
  static String ERROR;
  static bool ResInvalid(String res);
  static String Get(String path);
  static String Post(String path, String upDoc);
  static String Upload(String upDoc);
  static void Feedback(String taskId, int status);
};

#endif
