#include <Arduino.h>
#include <Relays.h>
#include <Programming.h>

#include "RestServer.h"
#include <ESP8266WiFi.h>


//WiFiServer server(80);
//WiFiClient client = server.available();
//const char* ssid="Jaques";
//const char* password="Harien22";

Relays relays;
Programming programming;
RestServer comunicacao;
char json[1000];

void setup()
{



  Serial.begin(115200);
  comunicacao.begin("Jaques","Harien22");

/*  WiFi.begin(ssid,password);

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

  Serial.println("Deu certo CBhttp!");
*/


  //relays.write(2, 1, 0);
  relays.read();
  //relays.serialize(json);
  //relays.print();
  programming.begin();
  programming.print();
  programming.random(62987);
  programming.print();
  relays.checkReaction(programming, 62987);


}

void loop ()
{




}
