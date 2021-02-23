#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>


const char* network = "Os Silva Wi Fi";
const char* password= "f0r@c0ntr0l3";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

AsyncWebSocketClient * globalClient = NULL;

String ip = "";
IPAddress ipAddr;

/*
 * Since we cannot hardcode an IP address for the webpage
 * to connect to the board server, this function replaces 
 * a tag inside the webpg code with the ip address currently
 * used by the board.
 * 
*/
String processor(const String& var){

  if(var == "PLACEHOLDER")
    return ip;

  return String();
  
}

/*
 * Since we need to handle information coming and going between client and server
 * this function takes care of events like "client connected" or disconnected. 
 * TODO: add event handler for when client sends a message to server.
*/
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len){

  if(type == WS_EVT_CONNECT){

    Serial.println("Websocket client connection received");
    globalClient = client;
  }

  else if(type == WS_EVT_DISCONNECT){
    Serial.println("Websocket client connection finished");
    globalClient = NULL;
  }
  
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

  // formatting the ip address into a suitable string
  // to be sent to the webpage
  char buf[20];
  ipAddr = WiFi.localIP();
  sprintf(buf,"%d.%d.%d.%d", ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
  ip = String(buf);
  Serial.println(buf);
  Serial.println(ip);
  Serial.println(WiFi.localIP());

  // binding the handler function with websocket and 
  // websocket to the webserver.
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

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

  // here we send the information we need to the page
  // TODO1: make a dedicated function
  // TODO2: use a FreeRTOS function to schedule sending
  // TOOD3: use real network information
  if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
    String mocknet;
    StaticJsonDocument<200> jsonMsg;
    JsonObject root = jsonMsg.to<JsonObject>();
    root["netname"] = "Os Silva WiFi";
    root["encryption"] = "WPA-PSK";
    root["openNet"] = "No";
    serializeJson(root, mocknet);
    globalClient->text(mocknet);
  }
  delay(4000);

}

  
