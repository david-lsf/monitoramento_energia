# monitoramento_energia

Desenvolvimento de um protótipo de tomada inteligente para monitoramento de energia elétrica residencial.
Para o funcionamento do mesmo, é necessário a calibração correta dos sensores e download das seguuintes bibliotecas:

EmonLib.h > Leitura e tratamento de dados dos sensores
SoftwareSerial.h > Para comunicação serial
ArduinoJson.h > Envio de dados atraves da serial
<ESP8266WiFi.h > Utilização do WIFI do módulo ESP8266
ESP8266WebServer.h> Criar o modo de comunicação do ESP8266
ThingSpeak.h > Plataforma em nuvem utilizada
