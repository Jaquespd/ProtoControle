#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Relays.h>
#include <Programming.h>
#include <RestServer.h>

#define UPDATE_NTP 3600000 //in milliseconds
#define TIMEZONE -10800 //in milliseconds
// #define SSID "Jaques" //in milliseconds
// #define PASSWORD "Harien22" //in milliseconds


Relays relays;
Programming programming;
WiFiUDP ntpUDP;
RestServer restserver;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).

NTPClient timeClient(ntpUDP, "time.nist.gov", TIMEZONE, UPDATE_NTP);

char json[1000];

//-------------------------WIFI--------------------------------

const char* ssid = "Jaques";
const char* password = "Harien22";
// const char* ssid = SSID;
// const char* password = PASSWORD;

uint32_t freeMemory = ESP.getFreeHeap();

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client = server.available();

void listReactionRequest(String request);
void debugMode();

void setup()
{
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
  relays.read();
  programming.random(timeClient.getEpochTime());
  relays.print();
  programming.print();
  //delay(10000);
}

void loop ()
{
  timeClient.update();
  // client = server.available();
  listReactionRequest(restserver.checkClientRequest(client = server.available()));
  relays.checkReaction(programming, timeClient.getEpochTime());
}

void debugMode()
{
      Serial.print("Memory loss before alguma coisa:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
}

void listReactionRequest(String request)
{
  bool returnJson=false;
  if(request.indexOf("PORTAS") != -1){
    relays.read();
    relays.serialize(json);
    returnJson=true;
    }
  if(request.indexOf("ATIVAR") != -1){
    relays.deserialize(json);
    relays.updateState();
    returnJson=false;
    }
  if(request.indexOf("PROG") != -1){
    programming.serialize(json);
    returnJson=true;
    }
  if(request.indexOf("QXT") != -1){
    programming.deserialize(json);
    //pode se pedir o checkReaction aqui
    returnJson=false;
    }

  if (returnJson){
    restserver.clientResponse(client,json);
  } else {
    restserver.clientResponse(client);
  }
}
