#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>



class RestServer {
  private:

  public:
    RestServer();
    // int begin(const char* ssid, const char* password);
    void clientResponse(WiFiClient& client, char* json);
    void clientResponse(WiFiClient& client);
    String checkClientRequest(WiFiClient& client, char* json, const int SIZE_JSON);

};
