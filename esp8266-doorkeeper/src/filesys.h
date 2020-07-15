#ifndef FILESYS_H
#define FILESYS_H

#include <Arduino.h>

class Filesys{
public:
  static void Setup();
  static void SetContent(char filepath[], String content);
  static String GetContent(char filepath[]);
};

#endif