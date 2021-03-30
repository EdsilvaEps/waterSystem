
#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>


String network = "Os Silva Wi Fi";
String password= "f0r@c0ntr0l";
const char* softApSSID = "Edson_System";
const char* softApPwd = "riptide";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

AsyncWebSocketClient * globalClient = NULL;

bool connecting = false;

char net[60];
char pass[60];


String ip = "";
IPAddress ipAddr;

String recvMsg;
bool msgAvailable = false;
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
    exportInfo();
  }

  else if(type == WS_EVT_DISCONNECT){
    Serial.println("Websocket client connection finished");
    globalClient = NULL;
  }

  else if(type == WS_EVT_DATA){
    Serial.println("Information received!");
    char dataMsg[len];

    for(int i = 0; i < len; i++){
      Serial.print((char) data[i]);
      dataMsg[i] = (char) data[i];
    }

    recvMsg = String(dataMsg);
    Serial.println(recvMsg);
    msgAvailable = true;

    /*if(recvMsg == "refresh"){
      exportInfo();
    } else {
      processSelectedNet(dataMsg);  
    }*/
    
  }
  
}


void processSelectedNet(String msg){
  // TODO: save net data before trying connection.

  const size_t capacity = JSON_OBJECT_SIZE(2) + 200;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, msg);
  if(error){
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  String netname = doc["netname"];
  String pwd = doc["pwd"];

  Serial.print("Nome da rede: ");
  Serial.println(netname);
  Serial.print("senha rede: ");
  Serial.println(pwd);

  //---- there's already a function for this in the main program ----//
  
  netname.toCharArray(net,60);
  pwd.toCharArray(pass,60);
  // TODO: research the method "beginSecure()" of server
  // TODO: research enabling SSL security
  //server.end(); // end server connection
  //WiFi.softAPdisconnect(); // remove AP server
  delay(1000);
  WiFi.begin(net, pass);
  connectedAfterTimeout();
  
  
}

bool isConnected(){
  return WiFi.status() == WL_CONNECTED;
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if(!SPIFFS.begin()){
    Serial.println("An error has ocurred while mounting SPIFFS");
    return;
  }

  /*WiFi.begin(network, password);
   * while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
   }
   ipAddr = WiFi.localIP();
   */

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
    
  scanNets();

  connectToNetwork();

  if(!isConnected()){
    delay(1000);
    scanNets();
    startServer();
    
  }

  
}

void startServer(){
  WiFi.softAP(softApSSID, softApPwd);
  ipAddr = WiFi.softAPIP();

  server.begin();
  // formatting the ip address into a suitable string
  // to be sent to the webpage
  char buf[20];  
  sprintf(buf,"%d.%d.%d.%d", ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
  ip = String(buf);
  Serial.print("go to page http://");
  Serial.print(buf);
  Serial.println("/html");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (msgAvailable){
    processSelectedNet(recvMsg);
    msgAvailable = false;
  }


}

// here we send the information we need to the page
void exportInfo(){
  // TODO2: use a FreeRTOS function to schedule sending
  Serial.println("[FUNCTION] exportInfo()");
  if(globalClient != NULL && globalClient->status() == WS_CONNECTED){

    int n = scanNets();
    if(n > 0){
      size_t capacity = JSON_ARRAY_SIZE(n) + n*JSON_OBJECT_SIZE(n);

      DynamicJsonDocument jsonBuffer(capacity);
      JsonArray arr = jsonBuffer.createNestedArray();

      for(int i = 0; i < n ; i++){
        
        JsonObject root = arr.createNestedObject();
        root["netname"] = String(WiFi.SSID(i));
        root["encryption"] = translateEncryptionType(WiFi.encryptionType(i));
        (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)? root["opennet"] = "Yes" : root["opennet"] = "No"; 
      }

      String nets;
      serializeJson(arr, nets);
      Serial.println(nets);
      globalClient->text(nets);
      
    } 

  }
  
}


int scanNets(){
  delay(1000);
  // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    //availableNets = n;
    if(n == WIFI_SCAN_FAILED) Serial.println("Scan Failed!");
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Serial.println("");
    return n;
      
}

void starte(){
  scanNets();
  
}

void connectToNetwork(){
  char net[60];
  char pass[60];
    
  if(network != "---" && password != "---"){ // if we have connection details in store
    network.toCharArray(net, 60);
      
    if(password.length() > 0){
      password.toCharArray(pass,60);
      WiFi.begin(net, pass);
    } else WiFi.begin(net);

    connectedAfterTimeout();
      
  }


}

// function counts a timeout for connecting to a network 
bool connectedAfterTimeout(){
  int attemptsAcc = 0; // attempts to connect
  
  while(!isConnected() && (attemptsAcc < 10)){
    delay(500);
    Serial.println("connectando com wifi ");
    attemptsAcc++;
      
  }

  if(isConnected()){
      
     Serial.println("Connectado com a rede ");

     return true;
  } 
  else{
    
     Serial.println("Failed to connect to network");
     //WiFi.mode(WIFI_STA);
     
     WiFi.disconnect();
     return false;
  }

}

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "Open";
    case (1):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (3):
      return "WPA2_PSK";
    case (4):
      return "WPA_WPA2_PSK";
    case (5):
      return "WPA2_ENTERPRISE";
    default:
      return "UNKNOWN";
    }
  }
