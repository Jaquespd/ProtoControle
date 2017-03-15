#pragma once

#include "Arduino.h"
#include <ArduinoJson.h>
#include <Programming.h>

#define N_RELAY 4


typedef struct Relay_t {
  int id;
  int state;
}Relay;


class Relays {
  private:
    Relay _relays[4];
    int PORT_RELAYS[N_RELAY]={5,4,14,12};
  public:

    Relays();
    void begin();
    void write(int id, int state, int i);
    void serialize (char* json, const int SIZE_JSON);
    void read();
    bool deserialize(char* json);
    void updateState();
    void print();
    void checkReaction(Programming programming, unsigned long getEpochTime);


};
