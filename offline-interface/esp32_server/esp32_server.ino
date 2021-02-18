#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

const char* network = "Os Silva Wi Fi";
const char* password= "f0r@c0ntr0l3";

AsyncWebServer server(80);

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

  Serial.println(WiFi.localIP());

  server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

}
