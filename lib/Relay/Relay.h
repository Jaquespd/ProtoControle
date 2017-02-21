#pragma once

#include "Arduino.h"


typedef struct Relay_t {
  int id;
  int state;
}Relay;


class Relays {
  private:


  public:

    Relays();
    void begin(Relay _relays, char* json);


};
