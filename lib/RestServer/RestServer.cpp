#include "RestServer.h"

RestServer::RestServer() {
  //WiFiServer server(80);
}

// int RestServer::begin(const char* ssid, const char* password) {}

void RestServer::clientResponse(WiFiClient& client, char* json)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    //Serial.println(json);
    client.println(json);
    delay(1);
    client.stop();
    //Serial.println("Client disonnected");
}

void RestServer::clientResponse(WiFiClient& client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  client.stop();
  //Serial.println("Client disonnected");

}

String RestServer::checkClientRequest(WiFiClient& client, char* json, const int SIZE_JSON)
{
    // Check if a client has connected
    //client = server.available();
    if (!client) {
      return "-1";
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
    msgReceive.toCharArray(json, SIZE_JSON);
    Serial.print("msgRecebida: ");
    Serial.println(msgReceive);
    // Serial.println(json);

      }else{
        if(req.indexOf("{")!= -1){
        beginJson = req.indexOf("{");
        endJson = req.indexOf("}");
        msgReceive = req.substring(beginJson,endJson+1);
        msgReceive.toCharArray(json, SIZE_JSON);
        Serial.print("msgRecebida: ");
        Serial.println(msgReceive);
        // Serial.println(json);
          }
        }
    //identifica o que a requisição pediu e encaminha para a rotina que vai realizar a ação
    return req;
}
