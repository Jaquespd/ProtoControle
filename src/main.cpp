#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define N_PROGRAMMING 10
#define N_RELAY 4
#define maxSizeJson 1000
#define UPDATE_NTP 3600000 //in milliseconds
#define TIMEZONE -10800 //in milliseconds
const int PORT_RELAYS[N_RELAY]={5,4,14,12};


WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "time.nist.gov", TIMEZONE, UPDATE_NTP);

//NTPClient timeClient(ntpUDP);


typedef struct Relay_t {
  int id;
  int state;
}Relay;

typedef struct Programming_t {
  int id;
  unsigned long timeOn;
  unsigned long timeOff;
}Programming;

Relay relay[N_RELAY];
Programming programming[N_PROGRAMMING];

char json[maxSizeJson];
//char* json="s";


//-------------------------WIFI--------------------------------

const char* ssid = "Jaques";
const char* password = "Harien22";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client = server.available();

void serializeRelay (Relay relay[], char* json);
bool deserializeRelay(Relay relay[], char* json);
void clientResponse(WiFiClient& client, char* json);
void clientResponse(WiFiClient& client);
void CheckClientRequest();
void ListReactionRequest(String request);
void serializeProgramming (Programming programming[], char* json);
bool deserializeProgramming(Programming programming[], char* json);
void updateStateRelays();
void readStateRelays();
void testProtoControl();
void checkReactionProgramming();
void printProgramming ();
void printRelays ();


int a = 0;
void setup()
{
  for (int i = 0; i<N_RELAY; i++){
    pinMode(PORT_RELAYS[i], OUTPUT);
    }

  Serial.begin(115200);
  timeClient.begin();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  timeClient.forceUpdate();
  readStateRelays();
  testProtoControl();
  printRelays();
  printProgramming();

  }

void loop ()
{
    timeClient.update();
    CheckClientRequest();
    checkReactionProgramming();


  }

void serializeRelay (Relay relay[], char* json)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_RELAY;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = relay[i].id;
      nested["estado"] = relay[i].state;
    }
    array.printTo(json,maxSizeJson);
    array.printTo(Serial);

  }

void serializeProgramming (Programming programming[], char* json)
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_PROGRAMMING;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = programming[i].id;
      nested["HoraLigar"] = timeClient.convertFormattedTime(programming[i].timeOn);
      nested["HoraDesligar"] = timeClient.convertFormattedTime(programming[i].timeOff);
    }
    array.printTo(json,maxSizeJson);
    array.printTo(Serial);

}

bool deserializeRelay(Relay relay[], char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    for (int i=0;i<N_RELAY;i++){
      relay[i].id = array[i]["id"];
      relay[i].state = array[i]["estado"];
    }
    return array.success();
}

bool deserializeProgramming(Programming programming[], char* json)
{
    DynamicJsonBuffer jsonBuffer;
    Serial.println(json);
    JsonArray& array = jsonBuffer.parseArray(json);
    for(int i=0;i<N_PROGRAMMING;i++){
      programming[i].id = array[i]["id"];//talvez precise transformar em int
      programming[i].timeOn=timeClient.convertEpochTime(array[i]["HoraLigar"]);
      programming[i].timeOff=timeClient.convertEpochTime(array[i]["HoraDesligar"]);
      }
    printProgramming();

    return array.success();
}

void clientResponse(WiFiClient& client, char* json)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    //Serial.println(json);
    client.println(json);
    delay(1);
    client.stop();
    Serial.println("Client disonnected");
  }

void clientResponse(WiFiClient& client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    client.stop();
    Serial.println("Client disonnected");
  }

void CheckClientRequest()
{
    // Check if a client has connected
    client = server.available();
    if (!client) {
      return;
      }

    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()){
      delay(1);
      }

    // Read the first line of the request
    String req = client.readString();
    Serial.println(req);
    client.flush();

    int beginJson = 0;
    int endJson = 0;
    String msgReceive;
    //a rotina abaixo verifica se é um objeto json ou array, e faz a separação do json do resto da requisição
    if(req.indexOf("[")!= -1){
    beginJson = req.indexOf("[");
    endJson = req.indexOf("]");
    msgReceive = req.substring(beginJson,endJson+1);
    Serial.print("msgRecebida: ");
    Serial.println(msgReceive);
      }else{
        if(req.indexOf("{")!= -1){
        beginJson = req.indexOf("{");
        endJson = req.indexOf("}");
        msgReceive = req.substring(beginJson,endJson+1);
        Serial.print("msgRecebida: ");
        Serial.println(msgReceive);
          }
        }
    //identifica o que a requisição pediu e encaminha para a rotina que vai realizar a ação
    ListReactionRequest(req);

}

void ListReactionRequest(String request)
{
  bool returnJson=false;
  if(request.indexOf("PORTAS") != -1){
    readStateRelays();
    serializeRelay(relay, json);
    returnJson=true;
    }
  if(request.indexOf("ATIVAR") != -1){
    deserializeRelay(relay, json);
    updateStateRelays();
    returnJson=false;
    }
  if(request.indexOf("PROG") != -1){
    serializeProgramming(programming, json);
    returnJson=true;
    }


  if (returnJson){
    clientResponse(client,json);
  } else {
    clientResponse(client);
  }

}

void updateStateRelays()
{
  for (int i=0;i<N_RELAY;i++){
    digitalWrite(PORT_RELAYS[i],relay[i].state);
    }
}

void readStateRelays()
{
  for (int i=0;i<N_RELAY;i++){
    if (digitalRead(PORT_RELAYS[i])==HIGH){
      relay[i].state = 1;
      }else{
        relay[i].state = 0;
        }
  }
}

void checkReactionProgramming()
{
  unsigned now = timeClient.getEpochTime();
  for (int i=0; i<N_PROGRAMMING; i++) {
    if (programming[i].timeOn<programming[i].timeOff) {
      if (programming[i].timeOn<= now && now <= programming[i].timeOff) {
        relay[programming[i].id].state = 1;
      } else {
        relay[programming[i].id].state = 0;
      }
    }
    if (programming[i].timeOn>programming[i].timeOff) {
      if (programming[i].timeOn<= now || now <= programming[i].timeOff) {
        relay[programming[i].id].state = 1;
      } else {
        relay[programming[i].id].state = 0;
      }
    }
  }
  updateStateRelays();
}

void testProtoControl()
{
  unsigned long now = timeClient.getEpochTime();
  for (int i=0; i<N_PROGRAMMING; i++) {
    programming[i].id = (i%4)+1;
    programming[i].timeOn = now + ((i+1)*5);
    programming[i].timeOff = programming[i].timeOn + 2;
  }
}

void printProgramming ()
{
  Serial.println("");
  Serial.print("Hora atual: ");
  Serial.println(timeClient.getFormattedTime());
  for (int i = 0; i<N_PROGRAMMING; i++) {
    Serial.println("");
    Serial.print("Programação n: ");
    Serial.print(i+1);
    Serial.print("; Rele: ");
    Serial.print(programming[i].id);
    Serial.print("; Hora de Ligar: ");
    Serial.print(timeClient.convertFormattedTime(programming[i].timeOn));
    Serial.print("; Hora de Desligar: ");
    Serial.print(timeClient.convertFormattedTime(programming[i].timeOff));
    Serial.println(".");
  }
}

void printRelays ()
{
  for (int i = 0; i<N_RELAY; i++) {
    Serial.println("");
    Serial.print("Rele n: ");
    Serial.print(relay[i].id);
    Serial.print("; Estado da porta: ");
    Serial.println(relay[i].state);
  }
}
