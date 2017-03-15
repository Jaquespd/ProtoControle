#pragma once

#include "Arduino.h"
#include <ArduinoJson.h>


#define N_PROGRAMMING 10


typedef struct Programming_t {
  int id;
  unsigned long timeOn;
  unsigned long timeOff;
}ProgrammingList;


class Programming {
  private:




  public:
    ProgrammingList _programming[10];
    Programming();
    void begin();
    void write(int id, unsigned long timeOn, unsigned long timeOff);
    void serialize (char* json, const int SIZE_JSON);
    bool deserialize(char* json);
    void random(unsigned long getEpochTime);
    void print();
    String convertFormattedTime(unsigned long userTime);
    unsigned long convertEpochTime(String userTime);

};
