#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define N_PROGRAMMING 10
#define N_RELAY 4
#define maxSizeJson 100


typedef struct Relay_t {
  int id;
  int state;
}Relay;

Relay relay[N_RELAY];

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


void setup()
{

  Serial.begin(115200);
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

void loop ()
{
    Relay relay2;
    relay2.id=0;
    relay2.state=1;

    relay[0]=relay2;
    relay[1]=relay2;
    relay[2]=relay2;
    relay[3]=relay2;

    serializeRelay(relay, json);
    delay(3000);
    deserializeRelay(relay,json);
    delay(3000);
  }

void serializeRelay (Relay relay[], char* json)
{
    //StaticJsonBuffer<200> jsonBuffer;
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    for (int i=0;i<N_RELAY;i++){
      JsonObject& nested = array.createNestedObject();
      nested["id"] = relay[i].id;
      nested["estado"] = relay[i].state;
    }
    //size_t len = root.measureLength();
    //size_t size = len+1;
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
      //json=[{id:1,estado:1},{id:2,estado:1},{id:3,estado:1},{id:4,estado:1}]
      //Serial.println(relay[i].id);
      //Serial.println(relay[i].state);
    }
    return array.success();
    //return true;

}
