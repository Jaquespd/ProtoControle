#include <Arduino.h>
#include <Relays.h>
#include <Programming.h>

Relays relays;
Programming programming;
char json[1000];

void setup()
{

  Serial.begin(115200);
  //relays.write(2, 1, 0);
  relays.read();
  //relays.serialize(json);
  //relays.print();
  programming.begin();
  programming.print();
  programming.random(62987);
  programming.print();


}

void loop ()
{




}
