#include <Arduino.h>

#ifndef FILESYS_H
#define FILESYS_H

class Filesys{
public:
  static void Setup();
  static void SetContent(char filepath[], String content);
  static String GetContent(char filepath[]);
};

#endif