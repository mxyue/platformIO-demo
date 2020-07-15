
#ifndef COLLROOM_H
#define COLLROOM_H

class CollRoom{
private:
  static void publishRoomState(String state);  

public:
  static void Setup();
  static void Loop();
};

#endif
