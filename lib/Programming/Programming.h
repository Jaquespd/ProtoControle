#pragma once

#include "Arduino.h"
#include <ArduinoJson.h>
#include <Relays.h>

#define N_PROGRAMMING 10
//#define maxSizeJson 1000


typedef struct Programming_t {
  int id;
  unsigned long timeOn;
  unsigned long timeOff;
}ProgrammingList;


class Programming {
  private:
    ProgrammingList _programming[10];



  public:

    Programming();
    void begin();
    void write(int id, unsigned long timeOn, unsigned long timeOff);
    void serialize (char* json);
    bool deserialize(char* json);
    void checkReaction(Relays relays, unsigned long getEpochTime);
    void random(unsigned long getEpochTime);
    void print();
    String convertFormattedTime(unsigned long userTime);
    unsigned long convertEpochTime(String userTime);

};
