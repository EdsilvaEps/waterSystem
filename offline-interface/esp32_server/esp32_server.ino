#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

const char* network = "Os Silva Wi Fi";
const char* password= "f0r@c0ntr0l3";

AsyncWebServer server(80);

String ip = "";
IPAddress ipAddr;

String processor(const String& var){

  if(var == "PLACEHOLDER")
    return ip;

  return String();
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if(!SPIFFS.begin()){
    Serial.println("An error has ocurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(network, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  char buf[20];
  ipAddr = WiFi.localIP();
  sprintf(buf,"%d.%d.%d.%d", ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
  ipAddr = WiFi.localIP();
 
  ip = String(buf);
  Serial.println(buf);
  Serial.println(ip);
  Serial.println(WiFi.localIP());

  server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

}
