#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

//-------------------------CONFIGURAÇÃO--------------------------------

#define N_RELE 4
#define PORTA_RELE_1 5
#define PORTA_RELE_2 4
#define PORTA_RELE_3 14
#define PORTA_RELE_4 12
#define N_PROGRAMACOES 10
//Tempos para checar se a programacao deve ativar ou não os reles.
#define TEMPO_VERIFICAR_PROGRAMACAO 5000
#define TEMPO_VERIFICAR_CLIENTE 1000
#define TEMPO_ATUALIZACAO_HORA_INICIAL 300000
#define TEMPO_ATUALIZACAO_HORA 21600000

//---------------------DECLARAÇÃO DE FUNÇÕES---------------------

unsigned long sendNTPpacket(IPAddress&);
void retornePortas();
void atualizarRelogioManual (String);
void retorneHora();
void ativarPortas (String);
void atualizarRelogioNTP ();
void programacao (String);
void retorneProgramacao();
void retorneHttpOK();
void relogio();
JsonObject& criarObjetoProgramacao(int, JsonBuffer&);
JsonArray& prepareResponseProgram(JsonBuffer&);
JsonObject& prepareResponseTime(JsonBuffer&);
void writeResponseObject(WiFiClient&, JsonObject&);
void checkJson();
void checkProgramacao();

//---------------------VARIAVEIS DE CONTROLE---------------------

const int RELE[N_RELE] = {PORTA_RELE_1,PORTA_RELE_2,PORTA_RELE_3,PORTA_RELE_4};
//variaveis onde serão armazenada a programação. IdRele = qual rele ativar.
unsigned long int idRele[10] = {0,0,0,0,0,0,0,0,0,0};
unsigned long int horaLigar[10] = {0,0,0,0,0,0,0,0,0,0};
unsigned long int horaDesligar[10] = {0,0,0,0,0,0,0,0,0,0};

//-------------------------WIFI--------------------------------

const char* ssid = "Jaques";
const char* password = "Harien22";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client = server.available();

//-------------------------RELOGIO--------------------------------

//Variaveis utilizadas para construir um relogio no controlador e atualiza-lo manualmente e pela internet
unsigned long relogioInterno = 0;
unsigned long relogioAtualizado = 0;
unsigned long millisref = 0;
int horaInt=0;
int minutoInt=0;
int segundoInt=0;
//Variaveis utilizadas para checar as condiçoes em periodos determinados. (Sem delay)
unsigned long currentmillisProgramacao=0;
unsigned long currentmillisCliente=0;
unsigned long currentmillisNTP=0;
unsigned long currentmillisNTPi=0;

//-------------------------NTP--------------------------------

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

//----------------------------------------------------------------

void setup() {
//configuração dos pinos do rele como saida
  for (int i = 0; i<N_RELE; i++){
    pinMode(RELE[i], OUTPUT);
    }
//inicialização da serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  delay(10);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  Serial.println("Aplicacao Iniciada com Sucesso! ");

}

void loop() {
//rotinas de periodo para verificação das ações do controlador.

//verifica se o cliente (app do celular) enviou alguma requisição a cada 1 segundo
  if (millis()-currentmillisCliente>=TEMPO_VERIFICAR_CLIENTE) {
    currentmillisCliente=millis();
    checkJson();
    }
//atualiza a hora pela internet apenas uma vez após 5 minutos de inicialização
  if (millis()-currentmillisNTPi>=TEMPO_ATUALIZACAO_HORA_INICIAL) {
    currentmillisNTPi=millis();
    atualizarRelogioNTP();
    }
//atualiza a hora pela internet a cada x horas
  if (millis()-currentmillisNTP>=TEMPO_ATUALIZACAO_HORA) {
    currentmillisNTP=millis();
    atualizarRelogioNTP();
    }
//verifica se o controlador deve ativar ou desativar os reles pela programacao a cada 5 segundos
  if (millis()-currentmillisProgramacao>=TEMPO_VERIFICAR_PROGRAMACAO) {
    currentmillisProgramacao=millis();
    checkProgramacao();
    }

  }

//------------------------VERIFICAÇÕES-----------------------------
//verifica se há requisição do cliente (app celular) e encaminha para a ação a ser tomada
void checkJson() {
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

  int inicioJson = 0;
  int fimJson = 0;
  String msgRecebida = "";
//a rotina abaixo verifica se é um objeto json ou array, e faz a separação do json do resto da requisição
  if(req.indexOf("[")!= -1){
  inicioJson = req.indexOf("[");
  fimJson = req.indexOf("]");
  Serial.print("inicioJson: ");
  Serial.println(inicioJson);
  Serial.print("fimJson: ");
  Serial.println(fimJson);
  msgRecebida = req.substring(inicioJson,fimJson+1);
  Serial.print("msgRecebida: ");
  Serial.println(msgRecebida);
    }else{
      if(req.indexOf("{")!= -1){
      inicioJson = req.indexOf("{");
      fimJson = req.indexOf("}");
      Serial.print("inicioJson: ");
      Serial.println(inicioJson);
      Serial.print("fimJson: ");
      Serial.println(fimJson);
      msgRecebida = req.substring(inicioJson,fimJson+1);
      Serial.print("msgRecebida: ");
      Serial.println(msgRecebida);
        }
      }
//identifica o que a requisição pediu e encaminha para a rotina que vai realizar a ação
  if(req.indexOf("PORTAS") != -1){
  retornePortas();
    }

  if(req.indexOf("HORA") != -1){
  atualizarRelogioManual (msgRecebida);
    }

  if(req.indexOf("RELOGIO") != -1){
  retorneHora();
    }

  if(req.indexOf("ATIVAR") != -1){
  ativarPortas (msgRecebida);
    }

  if(req.indexOf("NTP") != -1){
  atualizarRelogioNTP ();
    }

  if(req.indexOf("PROG") != -1){
  programacao (msgRecebida);
    }

  if(req.indexOf("QXT") != -1){
  retorneProgramacao();
    }

  if(req.indexOf("QXT") == -1 || req.indexOf("RELOGIO") == -1 || req.indexOf("PORTAS") == -1 || req.indexOf("HORA") == -1 || req.indexOf("ATIVAR") == -1 || req.indexOf("NTP") == -1 || req.indexOf("PROG") == -1){
  retorneHttpOK();
    }
}
//verifica se algum rele deve ser ativado ou desativado baseado no relogio e na programacao. E já toma a ação
void checkProgramacao() {
  //idRele[10] = {0,0,0,0,0,0,0,0,0,0};
  //horaLigar[10] = {0,0,0,0,0,0,0,0,0,0};
  //horaDesligar[10] = {0,0,0,0,0,0,0,0,0,0};

  relogio();
  for (int i=0; i<N_PROGRAMACOES; i++) {
    if (horaLigar[i] < horaDesligar[i]) {
      if (horaLigar[i] <= relogioInterno && relogioInterno <= horaDesligar[i]) {
        digitalWrite(idRele[i],HIGH);
      }else{
        digitalWrite(idRele[i],LOW);
      }
    }
    if (horaLigar[i] > horaDesligar[i]) {
      if (horaLigar[i] <= relogioInterno || relogioInterno <= horaDesligar[i]) {
        digitalWrite(idRele[i],HIGH);
      }else{
        digitalWrite(idRele[i],LOW);
      }
    }
  }
}

//------------------------REPOSTAS JSON-----------------------------
//está rotina prepara a resposta json do estado das portas do rele e retorna ao cliente

  JsonObject& criarObjetoEstadoPortas(int i,JsonBuffer& jsonBuffer2){
            JsonObject& root1 = jsonBuffer2.createObject();
            root1["id"] = i+1;
            if (digitalRead(RELE[i])==HIGH){
              root1["estado"] = 1;
              }else{
                root1["estado"] = 0;
                }

            // Json criado: {"id" : 1 , "estado" : 1}

            root1.printTo(Serial);
            return root1;
    }

//está rotina prepara o array da resposta a função anterior (retorne portas)
JsonArray& prepareResponsePorts(JsonBuffer& jsonBuffer) {
      JsonArray& array = jsonBuffer.createArray();
      DynamicJsonBuffer jsonBuffer2;

      for (int i=0;i<N_RELE;i++){
        array.add(criarObjetoEstadoPortas(i,jsonBuffer2));
        }

      //Json criado: [{"id":value,"estado":value},{"id":value,"estado":value},{"id":value,"estado":value}]

      Serial.println("");
      array.printTo(Serial);
      Serial.println("");
      return array;
  }

  //está rotina envia o cabeçalho html e escreve (envia) para o cliente um array. (utilizado para retornar a programacao e o estado das portas)
  void writeResponseArray(WiFiClient& client, JsonArray& msgEnviar) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();

      msgEnviar.printTo(client);
    }
//está rotina prepara os objetos json para a rotina anterior criar o array de objetos.
void retornePortas(){
      StaticJsonBuffer<100> jsonBuffer;
      JsonArray& msgEnviar = prepareResponsePorts(jsonBuffer);
      writeResponseArray(client, msgEnviar);
      delay(1);
      client.stop();
      Serial.println("Client disonnected");
  }


//está rotina prepara a resposta json do valor do relogio interno e retorna ao cliente
void retorneHora(){
      DynamicJsonBuffer jsonBuffer;
      JsonObject& msgEnviar = prepareResponseTime(jsonBuffer);
      writeResponseObject(client, msgEnviar);
      delay(1);
      client.stop();
      Serial.println("Client disonnected");

  }
//está rotina atualiza o relogio baseado no millis() e cria o objeto json para ser enviado
JsonObject& prepareResponseTime(JsonBuffer& jsonBuffer) {

      JsonObject& root = jsonBuffer.createObject();
      relogio(); //Chama função para atualizar as variaveis abaixo.
      root["hora"] = horaInt;
      root["minuto"] = minutoInt;
      root["segundo"] = segundoInt;

      //Json criado: {hora:11,minuto:23,segundo:52}

      root.printTo(Serial);
      return root;
  }
//está rotina envia o cabeçalho html e escreve (envia) para o cliente um objeto. (utilizado para retornar o relogio)
void writeResponseObject(WiFiClient& client, JsonObject& msgEnviar) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    //  json.prettyPrintTo(client);

    msgEnviar.printTo(Serial);
    msgEnviar.printTo(client);

  }
//está rotina prepara a resposta json da programação salva e retorna ao cliente
void retorneProgramacao(){

      StaticJsonBuffer<1300> jsonBuffer;
      JsonArray& msgEnviar = prepareResponseProgram(jsonBuffer);
      writeResponseArray(client, msgEnviar);
      delay(1);
      client.stop();
      Serial.println("Client disonnected");
  }
//está rotina prepara o array da resposta a função anterior (retorne programacao)
JsonArray& prepareResponseProgram(JsonBuffer& jsonBuffer) {
    JsonArray& array = jsonBuffer.createArray();
    DynamicJsonBuffer jsonBuffer2;

    for (int i=0;i<N_PROGRAMACOES;i++){
      array.add(criarObjetoProgramacao(i, jsonBuffer2));
      }

    //msgEnviar: [{id:1,HoraLigar:"10:20:11",HoraDesligar:"10:50:11"},{id:2,HoraLigar:"11:20:11",HoraDesligar:"11:50:11"}]
    //idRele[10] = {0,0,0,0,0,0,0,0,0,0};
    //horaLigar[10] = {0,0,0,0,0,0,0,0,0,0};
    //horaDesligar[10] = {0,0,0,0,0,0,0,0,0,0};

    Serial.println("");
    array.printTo(Serial);
    Serial.println("");
    return array;
  }
//está rotina prepara os objetos json para a rotina anterior criar o array de objetos.
JsonObject& criarObjetoProgramacao(int i, JsonBuffer& jsonBuffer2){
    JsonObject& root = jsonBuffer2.createObject();
    root["id"] = idRele[i];

    String _horaLigar = String(horaLigar[i]/3600);
    String _minutoLigar = String((horaLigar[i]%3600)/60);
    String _segundoLigar = String(horaLigar[i]%60);
    //Aqui utilizei "n" inves de ":" para separar hora:minuto:segundo pois estava apresentando um problema
    root["HoraLigar"] = _horaLigar + "n" +_minutoLigar + "n" +_segundoLigar;

    String _horaDesligar = String(horaDesligar[i]/3600);
    String _minutoDesligar = String((horaDesligar[i]%3600)/60);
    String _segundoDesligar = String(horaDesligar[i]%60);
    root["HoraDesligar"] = _horaDesligar + "n" +_minutoDesligar + "n" +_segundoDesligar;

    Serial.println("");
    root["id"].printTo(Serial);
    Serial.println("");
    root["HoraLigar"].printTo(Serial);
    Serial.println("");
    root["HoraDesligar"].printTo(Serial);

    return root;
  }
//retorna o cabeçalho html quando a requisição não pede nenhuma informação (POST)
void retorneHttpOK() {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/http");
  client.println("Connection: close");
  client.println();

  delay(1);
  client.stop();
  Serial.println("Client disonnected");
  }

//-------------------REQUISIÇÕES JSON-----------------------------
//Esta rotina ativa as portas do rele atraves de requisição do cliente. (ativação manual) depois melhorar pq esta funcao precisa inibiar a programacao automatica
void ativarPortas (String msgRecebida) {
  retorneHttpOK();

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(msgRecebida);
  if (!root.success()) {
    Serial.println("parseArray() failed");
    return;
  }

  //msgRecebida = [{id:1,value:1},{id:2,value:1},{id:3,value:1},{id:4,value:1}]

  for (int i=0;i<N_RELE;i++){
    //int estado = root[i]["estado"];
    digitalWrite(RELE[i],root[i]["estado"]);
    }
}
//Esta rotina atualiza a programacao do controlador
void programacao (String msgRecebida) {
  retorneHttpOK();

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(msgRecebida);
  if (!root.success()) {
    Serial.println("parseArray() failed");
    return;
  }

  //msgRecebida: [{id: 1, HoraLigar: "10:20:11", HoraDesligar: "10:50:11"},{id: 2, HoraLigar: "11:20:11", HoraDesligar: "11:50:11"},{id: 1, HoraLigar: "12:22:12", HoraDesligar: "10:50:11"}]

  for(int i=0;i<N_PROGRAMACOES;i++){
      String _idRele = root[i]["id"];
      idRele[i]=_idRele.toInt();

      String _horaLigar = root[i]["HoraLigar"];
      _horaLigar = _horaLigar.substring(0,2);

      String _minutoLigar = root[i]["HoraLigar"];
      _minutoLigar = _minutoLigar.substring(3,5);

      String _segundoLigar = root[i]["HoraLigar"];
      _segundoLigar = _segundoLigar.substring(6,8);

      horaLigar[i]=3600*_horaLigar.toInt()+60*_minutoLigar.toInt()+_segundoLigar.toInt();

      String _horaDesligar = root[i]["HoraDesligar"];
      _horaDesligar = _horaDesligar.substring(0,2);

      String _minutoDesligar = root[i]["HoraDesligar"];
      _minutoDesligar = _minutoDesligar.substring(3,5);

      String _segundoDesligar = root[i]["HoraDesligar"];
      _segundoDesligar = _segundoDesligar.substring(6,8);

      horaDesligar[i]=3600*_horaDesligar.toInt()+60*_minutoDesligar.toInt()+_segundoDesligar.toInt();

      Serial.println("");
      Serial.print("Programacao n: ");
      Serial.print(i);
      Serial.print(": O rele ");
      Serial.print(idRele[i]);
      Serial.print(" sera acionado em: ");
      Serial.print(horaLigar[i]);
      Serial.print(" e desligado em: ");
      Serial.println(horaDesligar[i]);

    }
}
//está rotina atualiza o relogio interno atraves de requisição do cliente para ambientes sem internet. (ativação manual)
void atualizarRelogioManual (String msgRecebida) {
  retorneHttpOK();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msgRecebida);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  //msgRecebida = {hora:11,minuto:24,segundo:34}

  String _hora = root["hora"];
  String _minuto = root["minuto"];
  String _segundo = root["segundo"];

  // In the first 10 seconds of each minute, we'll want a leading '0'
  unsigned long _epochManual = _hora.toInt()*3600 + _minuto.toInt()*60 + _segundo.toInt();
  relogioAtualizado = _epochManual;
  millisref=millis();
}

//---------------------------RELOGIO------------------------------
//Rotina que cria o relogio interno baseado no millis()
void relogio() {

  relogioInterno = relogioAtualizado+((millis()-millisref)/1000);
  horaInt = (relogioInterno  % 86400L) / 3600;
  minutoInt = (relogioInterno  % 3600) / 60;
  segundoInt = relogioInterno % 60;

  if ( ((relogioInterno % 86400L) / 3600) < 10 ) {
    // In the first 10 horas of each day, we'll want a leading '0'
    Serial.print('0');
    }
  Serial.print((relogioInterno  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  Serial.print(':');

  if ( ((relogioInterno % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
    }
  Serial.print((relogioInterno  % 3600) / 60); // print the minute (3600 equals secs per minute)
  Serial.print(':');

  if ( (relogioInterno % 60) < 10 ) {
    Serial.print('0');
    }
  Serial.println(relogioInterno % 60); // print the second
}
//rotina atualiza o relogio pela internet
void atualizarRelogioNTP () {
  retorneHttpOK();

  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    relogioAtualizado = epoch-10800; //10800 é equivalente a 3 horas do GTM -3
    millisref=millis();

  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
