#include "Relays.h"

Relays::Relays() {
  for (int i = 0; i<N_RELAY; i++){
    pinMode(PORT_RELAYS[i], OUTPUT);
    }
}

void Relays::begin() {
  Serial.println("Deu certo Relay!");
}

void Relays::write(int id, int state, int i) {
  this->_relays[i-1].id = id;
  this->_relays[i-1].state = state;
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

void Relays::serialize (char* json, const int SIZE_JSON)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_RELAY;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = this->_relays[i].id;
      nested["state"] = this->_relays[i].state;
    }
    array.printTo(json,SIZE_JSON);
    array.printTo(Serial);

}

bool Relays::deserialize(char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    array.success() == true ? Serial.println("Parse Ok") : Serial.println("erro");
    for (int i=0;i<N_RELAY;i++){
      this->_relays[i].id = array[i]["id"];
      this->_relays[i].state = array[i]["state"];
      Serial.println("Teste: ");
      array[i]["id"].printTo(Serial);
      array[i]["state"].printTo(Serial);
      delay(2000);
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

void Relays::checkReaction(Programming programming, unsigned long getEpochTime)
{
  unsigned long now = getEpochTime;
  for (int i = 0; i<N_RELAY; i++) {
    this->write(i+1, 0, i+1);
  }
  for (int i=0; i<N_PROGRAMMING; i++) {
    if (programming._programming[i].timeOn < programming._programming[i].timeOff) {
      if (programming._programming[i].timeOn <= now && now <= programming._programming[i].timeOff) {
        this->write(programming._programming[i].id, 1, programming._programming[i].id);
      }
    }
    if (programming._programming[i].timeOn > programming._programming[i].timeOff) {
      if (programming._programming[i].timeOn <= now || now <= programming._programming[i].timeOff) {
        this->write(programming._programming[i].id, 1, programming._programming[i].id);
      }
    }
  }
  this->updateState();
}
