#include "Relays.h"

Relays::Relays() {
  for (int i = 0; i<N_RELAY; i++){
    pinMode(PORT_RELAYS[i], OUTPUT);
    }
}

void Relays::begin() {
  Serial.println("Deu certo!");
}

void Relays::write(int id, int state, int i) {
  this->_relays[i].id = id;
  this->_relays[i].state = state;
  Serial.println(this->_relays[i].id);
}

void Relays::read()
{
  for (int i=0;i<N_RELAY;i++){
    this->_relays[i].id = i+1;
    if (digitalRead(PORT_RELAYS[i])==HIGH){
      this->_relays[i].state = 1;
      }else{
        this->_relays[i].state = 0;
        }
  }
}

void Relays::serialize (char* json)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_RELAY;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = this->_relays[i].id;
      nested["estado"] = this->_relays[i].state;
    }
    array.printTo(json,maxSizeJson);
    array.printTo(Serial);

}

bool Relays::deserialize(char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    for (int i=0;i<N_RELAY;i++){
      this->_relays[i].id = array[i]["id"];
      this->_relays[i].state = array[i]["estado"];
    }
    return array.success();
}

void Relays::updateState()
{
  for (int i=0;i<N_RELAY;i++){
    digitalWrite(PORT_RELAYS[i],this->_relays[i].state);
    }
}

void Relays::print()
{
  for (int i = 0; i<N_RELAY; i++) {
    Serial.println("");
    Serial.print("Rele n: ");
    Serial.print(this->_relays[i].id);
    Serial.print("; Estado da porta: ");
    Serial.println(this->_relays[i].state);
  }
}
