#include "Programming.h"

Programmings::Programming() {
  for (int i = 0; i<N_PROGRAMMING; i++){
    //vazio
    }
}

void Programming::begin() {
  Serial.println("Deu certo!");
}

void serialize (char* json)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_PROGRAMMING;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = this->_programming[i].id;
      //nested["HoraLigar"] = timeClient.convertFormattedTime(this->_programming[i].timeOn);
      //nested["HoraDesligar"] = timeClient.convertFormattedTime(this->_programming[i].timeOff);
    }
    array.printTo(json,1000);
    array.printTo(Serial);

}

bool deserialize(char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    for(int i=0;i<N_PROGRAMMING;i++){
      this->_programming[i].id = array[i]["id"];//talvez precise transformar em int
      //this->_programming[i].timeOn=timeClient.convertEpochTime(array[i]["HoraLigar"]);
      //this->_programming[i].timeOff=timeClient.convertEpochTime(array[i]["HoraDesligar"]);
      }
    printProgramming();

    return array.success();
}

void checkReaction()
{
  //unsigned now = timeClient.getEpochTime();
  for (int i=0; i<N_PROGRAMMING; i++) {
    if (this->_programming[i].timeOn<this->_programming[i].timeOff) {
      if (this->_programming[i].timeOn<= now && now <= this->_programming[i].timeOff) {
        //relays.write(this->_programming[i].id, 1, i);
      } else {
        //relays.write(this->_programming[i].id, 0, i);
      }
    }
    if (this->_programming[i].timeOn>this->_programming[i].timeOff) {
      if (this->_programming[i].timeOn<= now || now <= this->_programming[i].timeOff) {
        //relays.write(programming[i].id, 1, i);
      } else {
        //relays.write(programming[i].id, 0, i);
      }
    }
  }
  //relays.updateState();
}

void random()
{
  //unsigned long now = timeClient.getEpochTime();
  for (int i=0; i<N_PROGRAMMING; i++) {
    this->_programming[i].id = (i%4)+1;
    this->_programming[i].timeOn = now + ((i+1)*5);
    this->_programming[i].timeOff = this->_programming[i].timeOn + 2;
  }
}

void print ()
{
  Serial.println("");
  Serial.print("Hora atual: ");
  //Serial.println(timeClient.getFormattedTime());
  for (int i = 0; i<N_PROGRAMMING; i++) {
    Serial.println("");
    Serial.print("Programação n: ");
    Serial.print(i+1);
    Serial.print("; Rele: ");
    Serial.print(this->_programming[i].id);
    Serial.print("; Hora de Ligar: ");
    //Serial.print(timeClient.convertFormattedTime(this->_programming[i].timeOn));
    Serial.print("; Hora de Desligar: ");
    //Serial.print(timeClient.convertFormattedTime(this->_programming[i].timeOff));
    Serial.println(".");
  }
}
