#include "Programming.h"

Programming::Programming() {
  //for (int i = 0; i<N_PROGRAMMING; i++){
    //vazio
    //}
}

void Programming::begin() {
  Serial.println("Deu certo Programming!");
}

void Programming::serialize (char* json)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_PROGRAMMING;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = this->_programming[i].id;
      nested["HoraLigar"] = Programming::convertFormattedTime(this->_programming[i].timeOn);
      nested["HoraDesligar"] = Programming::convertFormattedTime(this->_programming[i].timeOff);
    }
    array.printTo(json,1000);
    array.printTo(Serial);

}

bool Programming::deserialize(char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    for(int i=0;i<N_PROGRAMMING;i++){
      this->_programming[i].id = array[i]["id"];//talvez precise transformar em int
      this->_programming[i].timeOn=Programming::convertEpochTime(array[i]["HoraLigar"]);
      this->_programming[i].timeOff=Programming::convertEpochTime(array[i]["HoraDesligar"]);
      }
    Programming::print();

    return array.success();
}

void Programming::random(unsigned long getEpochTime)
{
  unsigned long now = getEpochTime;
  for (int i=0; i<N_PROGRAMMING; i++) {
    this->_programming[i].id = (i%4)+1;
    this->_programming[i].timeOn = now + ((i+1)*5);
    this->_programming[i].timeOff = this->_programming[i].timeOn + 2;
  }
}

void Programming::print ()
{
  for (int i = 0; i<N_PROGRAMMING; i++) {
    Serial.println("");
    Serial.print("Programação n: ");
    Serial.print(i+1);
    Serial.print("; Rele: ");
    Serial.print(this->_programming[i].id);
    Serial.print("; Hora de Ligar: ");
    Serial.print(Programming::convertFormattedTime(this->_programming[i].timeOn));
    Serial.print("; Hora de Desligar: ");
    Serial.print(Programming::convertFormattedTime(this->_programming[i].timeOff));
    Serial.println(".");
  }
}

String Programming::convertFormattedTime(unsigned long userTime) {
  unsigned long rawTime = userTime;
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

unsigned long Programming::convertEpochTime(String userTime) {
  if (userTime.length()==7){
    userTime = "0" + userTime;
  }
  if (userTime.length()!=8){
    return 0;
  }
  String hoursStr = userTime.substring(0,2);
  String minuteStr = userTime.substring(3,5);
  String secondStr = userTime.substring(6,8);
  return 3600*hoursStr.toInt()+60*minuteStr.toInt()+secondStr.toInt();
}
