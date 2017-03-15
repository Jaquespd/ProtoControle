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
#define SSID "Jaques"
#define PASSWORD "Harien22"
#define SIZE_JSON 1000 // in bytes


Relays relays;
Programming programming;
WiFiUDP ntpUDP;
RestServer restserver;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client = server.available();

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).

NTPClient timeClient(ntpUDP, "time.nist.gov", TIMEZONE, UPDATE_NTP);


//-------------------------WIFI--------------------------------

//const char* ssid = "Jaques";
//const char* password = "Harien22";
const char* ssid = SSID;
const char* password = PASSWORD;
char json[SIZE_JSON];
bool MODE_AUTO = true;

//uint32_t freeMemory = ESP.getFreeHeap();

void listReactionRequest(String request);
void wifibegin();
//void debugMode();

void setup()
{
  Serial.begin(115200);
  timeClient.begin();
  wifibegin();
  timeClient.forceUpdate();
  relays.read();
  programming.random(timeClient.getEpochTime());

  // Linhas para teste
  // relays.print();
  // programming.print();
  // delay(10000);
}

void loop ()
{
  timeClient.update();
  listReactionRequest(restserver.checkClientRequest(client = server.available(), json, SIZE_JSON));
  if (MODE_AUTO == true) {
    relays.checkReaction(programming, timeClient.getEpochTime());
  }
}

void listReactionRequest(String request)
{
  bool returnJson=false;
  if(request.indexOf("GETPORTS") != -1){
    relays.read();
    relays.serialize(json, SIZE_JSON);
    returnJson=true;
    }
  if(request.indexOf("SETPORTS") != -1){
    relays.deserialize(json);
    relays.updateState();
    MODE_AUTO = false;
    returnJson=false;
    }
  if(request.indexOf("GETPROGRAMMING") != -1){
    programming.serialize(json,SIZE_JSON);
    returnJson=true;
    }
  if(request.indexOf("SETPROGRAMMING") != -1){
    programming.deserialize(json);
    // se ativar aqui pode dar problema nas views, pois vai passar para automatico sem avisar o app
    // relays.checkReaction(programming, timeClient.getEpochTime());
    returnJson=false;
    }
  if(request.indexOf("CLOCK") != -1){
    timeClient.serialize(json, SIZE_JSON);
    returnJson=true;
    }
  if(request.indexOf("MODEAUTO") != -1){
    MODE_AUTO = true;
    returnJson = false;
    }

  if (returnJson){
    restserver.clientResponse(client,json);
  } else {
    restserver.clientResponse(client);
  }
}

void wifibegin ()
{
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

}

/*
void debugMode()
{
      Serial.print("Memory loss before alguma coisa:");
      Serial.println(freeMemory - ESP.getFreeHeap(),DEC);
      freeMemory = ESP.getFreeHeap();
}
*/
