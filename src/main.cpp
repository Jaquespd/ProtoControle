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
void clientResponse(WiFiClient& client, char* json);
void clientResponse(WiFiClient& client);
void CheckClientRequest();
void reactionRequest(String request);


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
    for (int i=0;i<N_RELAY;i++){
      relay[i]=relay2;
    }

    CheckClientRequest();


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
    Serial.print("inicioJson: ");
    Serial.println(beginJson);
    Serial.print("fimJson: ");
    Serial.println(endJson);
    msgReceive = req.substring(beginJson,endJson+1);
    Serial.print("msgRecebida: ");
    Serial.println(msgReceive);
      }else{
        if(req.indexOf("{")!= -1){
        beginJson = req.indexOf("{");
        endJson = req.indexOf("}");
        Serial.print("inicioJson: ");
        Serial.println(beginJson);
        Serial.print("fimJson: ");
        Serial.println(endJson);
        msgReceive = req.substring(beginJson,endJson+1);
        Serial.print("msgRecebida: ");
        Serial.println(msgReceive);
          }
        }
    //identifica o que a requisição pediu e encaminha para a rotina que vai realizar a ação
    reactionRequest(req);

}

void reactionRequest(String request)
{
  bool returnJson=false;
  if(request.indexOf("PORTAS") != -1){
    serializeRelay(relay, json);
    returnJson=true;
    }
  if(request.indexOf("ATIVAR") != -1){
    deserializeRelay(relay, json);
    returnJson=false;
    }
  if (returnJson){
    clientResponse(client,json);
  } else {
    clientResponse(client);
  }
  
}
