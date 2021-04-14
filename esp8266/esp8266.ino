#include <FS.h>          // this needs to be first, or it all crashes and burns...
#ifdef ESP32
  #include <SPIFFS.h>
#endif
#if defined(ESP8266)
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library         
#else
#include <WiFi.h>      //ESP32 Core WiFi Library    
#endif
//needed for library
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal ( https://github.com/zhouhan0126/DNSServer---esp32 )
#if defined(ESP8266)
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#else
#include <WebServer.h> //Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#endif
#include <WiFiManager.h>   // WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> https://github.com/tzapu/WiFiManager (ORIGINAL)
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson foi usado a versao 5.x, ao usar 6.x gera erros
#include "ThingSpeak.h" 
#include <SoftwareSerial.h>


SoftwareSerial NodeMCU(D6, D5);

WiFiClient client;

unsigned long idCanalString; //ADICIONADO ID CANAL
const char* chaveEscritaThingspeak = ""; // variavel a ser usada na requisicao http


//defina valores padroes aqui, se existirem valores diferentes no config.json eles serão sobrescritos
char writekey[17]; //a ser usado para valor vindo do portal de configuração
char idCanal[8]; //ADICIONADO ID CANAL


//flag para salvemento de dados
bool shouldSaveConfig = false;

//callback que notifica de que precisa salvar as configurações
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupSpiffs(){
  SPIFFS.format();

  Serial.println("mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          
          /
          strcpy(writekey, json["writekey"]);
          strcpy(idCanal, json["idCanal"]);
          
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}




void setup(){
  
  Serial.begin(115200);
  NodeMCU.begin(115200);
  ThingSpeak.begin(client);
  
  setupSpiffs();
  WiFiManager wm;

  //callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
  wm.setSaveConfigCallback(saveConfigCallback);

  // Setar parametros extras que aparecerao no portal(192.168.4.1)
  WiFiManagerParameter custom_writekey("writekey", "Digite a chave de escrita do ThingSpeak:","57VI9AW70QRCTTLH", 17);//chave já aparecerá no portalAP para facilitar testes do codigo
  WiFiManagerParameter custom_idCanal("idCanal", "Digite o id do canal do ThingSpeak:","", 8);//ADICIONADO ID CANAL
  
  wm.addParameter(&custom_writekey);
  wm.addParameter(&custom_idCanal);
 
  if (!wm.autoConnect("Tomada_SUPER","12345678")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

   wm.setConfigPortalTimeout(60);
   wm.startConfigPortal("Tomada_SUPER","12345678");

  Serial.println("Conectado (STA) ");

  strcpy(writekey, custom_writekey.getValue());
  strcpy(idCanal, custom_idCanal.getValue());
  
  if (shouldSaveConfig) {
    Serial.println("salvando configuração");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    
    json["writekey"] = writekey;
    json["idCanal"]  = idCanal;
   
  
    File configFile = SPIFFS.open("/config.json", "w"); 
    if (!configFile) {
      Serial.println("falha ao abrir arquivo para escrita");
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    shouldSaveConfig = false;
  }

  chaveEscritaThingspeak = writekey;
  idCanalString = (unsigned long )(idCanal);

}


void loop(){

  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(NodeMCU);
  if (root == JsonObject::invalid())
    return;
  root.prettyPrintTo(Serial);
  float stensao = root["tensao"];
  float scorrente = root["corrente"];
  float potencia = root["potencia"];

  if(stensao < 0.4){
    stensao = 0.0;
    scorrente = 0.0;
    potencia = 0.0;
  }
  else{
    ThingSpeak.setField(1, stensao);
    ThingSpeak.setField(2, scorrente);
    ThingSpeak.setField(3, potencia);
  }
  
  int x = ThingSpeak.writeFields(idCanalString, chaveEscritaThingspeak);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(16000);
}
