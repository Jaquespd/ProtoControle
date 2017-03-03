#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>



class RestServer {
  private:
    const char* _ssid = "Jaques";
    const char* _password = "Harien22";
//    WiFiServer _server(80);
    WiFiClient _client;
//    WiFiServer server(80);


  public:
    RestServer();
    int begin(const char* ssid, const char* password);
    void clientResponse(char* json);
/*    void clientResponse();
    void CheckClientRequest();
    //void ListReactionRequest(String request);
*/
};
