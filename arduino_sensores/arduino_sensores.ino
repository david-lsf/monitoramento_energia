#include <SoftwareSerial.h>
#include "EmonLib.h"
#include <ArduinoJson.h>

#define VOLT_CAL 202.6
#define VOLT_PIN A0
#define CURR_PIN A2

SoftwareSerial ArduinoUno(5,6);
EnergyMonitor emon1;
 
void setup(){
  ArduinoUno.begin(115200);
  Serial.begin(115200);
  emon1.voltage(VOLT_PIN, VOLT_CAL, 1.7);
  emon1.current(CURR_PIN, 17.5);
}

void loop(){
  emon1.calcVI(17,2000);
  double scorrente = emon1.calcIrms(1480);
  double stensao = emon1.Vrms;
  double potencia = emon1.realPower;
    
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["corrente"] = scorrente;
  root["tensao"] = stensao;
  root["potencia"] = potencia;
  
  root.prettyPrintTo(Serial);
  if(ArduinoUno.available() > 0) {
    root.printTo(ArduinoUno);
   delay(1000);
  }
}
