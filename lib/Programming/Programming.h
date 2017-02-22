#pragma once

#include "Arduino.h"
#include <ArduinoJson.h>

#define N_PROGRAMMING 4
#define maxSizeJson 1000


typedef struct Programming_t {
  int id;
  unsigned long timeOn;
  unsigned long timeOff;
}ProgrammingList;


class Programming {
  private:
    ProgrammingsList _programming[10];
    
    
  public:

    Programming();
    void begin();
    void write(int id, unsigned long timeOn, unsigned long timeOff);
    void serialize (char* json);
    bool deserialize(char* json);
    //void checkReaction();
    void random();
    void programming();

};
