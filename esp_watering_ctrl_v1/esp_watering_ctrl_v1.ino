#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
//#include <Adafruit_GFX.h> // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <PubSubClient.h>
//#include "BluetoothSerial.h"
#include <Preferences.h>
//#include "TOD.h"
#include "WateringPlan.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

#define PREFS "my-app"
#define MANAUSGMT -14400 // MANAUS GMT TIME = -4

#define SECURE_CONNECTION 0

// maquina de estados do loop principal
#define CHECK_CONNECTION_STATE 0 
#define SELECT_NETWORKS_STATE 1
#define BROKER_CONNECTION_STATE 2 
#define TIME_CHECK_STATE 3
#define SENSOR_CHECK_STATE 4
#define PERFORMING_WATERING_STATE 5 
#define NEW_PROGRAM_RECEIVED 6

int state = CHECK_CONNECTION_STATE; // inicial state 

Preferences preferences;

//test network: Os Silva Wi Fi
//test pass: c8906d2932
//Pauliina's net : "TW-EAV510v28483";
//Pauliina's net pwd: "58d19c6fa81fe5e6";

String network = "Os Silva Wi Fi";
String password= "f0r@c0ntr0l";
bool conncted = false;

// certificado para acesso usando TLS
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDPzCCAicCFAgB/9E66qY666QwZU3U+uX/sk7QMA0GCSqGSIb3DQEBCwUAMFwx\n" \
"CzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl\n" \
"cm5ldCBXaWRnaXRzIFB0eSBMdGQxFTATBgNVBAMMDG1hcWlhdHRvLmNvbTAeFw0y\n" \
"MDA1MDMxNTU4NTZaFw0yMTA0MjgxNTU4NTZaMFwxCzAJBgNVBAYTAkFVMRMwEQYD\n" \
"VQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBM\n" \
"dGQxFTATBgNVBAMMDG1hcWlhdHRvLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
"ADCCAQoCggEBAPQUxGPAcCiAhIDvNWGh2UFhtVAEu9JiadWRzBPaWgwePvFfHtNN\n" \
"isiqLpPB39+IsOEaya9W6izpF9dNdEsLZX0oKKv1JzBYFuBskQOb4oLqEOztnxf7\n" \
"jq15mYTpim/6rL6d4VzwfHDH02TxBgM8cVPPJIjN8qfymWXB1Rb8phT6q1wT8tey\n" \
"TgOHwm8BmCk+7UX6j5gT9LB5nDCZOmjcK9m/wYE9xQBmxevGbYlFt5woXjpE+BTa\n" \
"DeXVZMum5gqkJ1CvviwoNkFbAUJ5e619gGJfL2HOre7ckHPLRjplCWacNb6eHe3R\n" \
"0tZFNaDphGL8hyUnPK+CnFG+B4d7JqKliq8CAwEAATANBgkqhkiG9w0BAQsFAAOC\n" \
"AQEAA+V0DyO/RDqCEWBrPDByEzNSharBL/ig8Mr9z+AoU0dLmmb8kwpPO1hJUTdh\n" \
"uARNHg8HyeoHlPqEDlfP2+KDqvHZmWRx5GSBHMPFYvaDLk9237nueEmTPM3cdVFh\n" \
"ZQPI7gwlYxO6hvqtHpHb4erOLXpQSHl06tE75akM0BS9ve1t0rvDBk/YbB7sIUei\n" \
"30NF8LdIgDZPBDOPvGRvXm3ib20S8fkHTgNMEsGuPjrk41glH6nONlDzQ+8NbXwo\n" \
"B9AWDQG54xUYDpOwM/LTRdvYfFMH9+U8nXCRO+xD89R80BQkeFz9On5tf6y2D8Pl\n" \
"79gL077l7yk/UUsZukhkpzFnrg==\n" \
"-----END CERTIFICATE-----\n";


volatile int interruptCounter = 0;

// pinos dos sensores de nivel de agua
int level1 = 14;
int level2 = 27;
int level3 = 26;

int waterLevel = 0;

int serverLed = 25;
int countingLed = 33;
int dataLed = 32;

// pump pin
int pumpPin = 12;

// ******* SOFT AP VARIABLES **************
const char* softApSSID = "ed_system";
const char *softApPWD = "riptide";
IPAddress ipAddr;
String recvMsg, ip;
bool isSoftApConnected = false; 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalClient = NULL;
bool msgAvailable = false;
bool sendInfo = false;

// *******/SOFT AP VARIABLES **************

// ******* MQTT VARIABLEs *****************
bool secureConnect = false;
// mqtt broker currently running on maqiatto
const char* mqttServer = "maqiatto.com";
const int mqttPort= 1883; // tcp port 
const int secureMqttPort= 3883; // tls port
const char* mqttUser = "netosilvan78@gmail.com";
const char* mqttPassword = "y4NW1jgJi3b7";

const char* subscribeCtrlPath = "netosilvan78@gmail.com/system/control/dispenseWater";
const char* subscribeTimingPath = "netosilvan78@gmail.com/system/api/timing";
const char* subscribeAmountPath = "netosilvan78@gmail.com/system/app/amount";
const char* publishTempPath = "netosilvan78@gmail.com/system/hardware/temp";
const char* publishLevelPath = "netosilvan78@gmail.com/system/hardware/level";
const char* publishReqNextSchedule = "netosilvan78@gmail.com/system/hardware/nextWSchedule";
const char* setupWateringProgram = "netosilvan78@gmail.com/system/hardware/setupProgram";
const char* publishReport = "netosilvan78@gmail.com/system/report";
const char* pingPath = "netosilvan78@gmail.com/system/ping";
const char* powerPath = "netosilvan78@gmail.com/power";

const char* wateredMessage = "JUST_WATERED";
const char* lowLvMessage = "LOW_WATER";

//#ifdef SECURE_CONNECTION
//WiFiClientSecure espClient;
//#else
WiFiClient espClient;
//#endif

PubSubClient client(espClient);



// NTP Client for time variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// ********************* ******************

// ********** DEBOUNCE ******************************
// the following variables take care of the debouncing of the hardware.
// The level sensors work like relays, closing the connection between 
// two points (namely GPIO port and GND) there should a noise area while 
// the tank is being filled or emptied where the rippling of the water 
// might disturb this reading, so we need to setup a long debouncing time
// to only allow the registering of new information by a single sensor after
// the waters have calmed. 

#define DEBOUNCE_TIME_MINS 5

bool debounce[3] = {false, false, false}; // debounce vars for the 3 lvl sensors 

// ************************************************

// ***** SCHEDULE & WATERING VARIABLES *******

// amount of the times the esp32 will retry the update the clock
// before restarting 
const int maxAttemptsAtScheduling = 3;
int attemptsAtScheduling = 0;

WateringProgram wprogram;

int soilSensorPower = 35; // GPIO 35
int soilSensorPin = 34; // GPIO 34 (Analog ADC1_CH6) 
int drySoilThreshold = 3800;
int soilHumidity = 0;

int lastWateredHour = -1; // last hour the plans have been watered
int lastSoilMCheckedMinute = -1;

// ***** /SCHEDULE & WATERING VARIABLES *******

int availableNets = 0;
String inData = "";

bool isWatering = false;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux =  portMUX_INITIALIZER_UNLOCKED;
bool toggleLed = 0;

// function to execute when interruption triggers
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux); // lock main thread for executing critical code
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("Initialization...");
    Serial.setDebugOutput(true);

    //pinMode(LED_BUILTIN, OUTPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(soilSensorPin, INPUT);
    pinMode(soilSensorPower, OUTPUT);

    // configuring level sensor pins
    pinMode(level1, INPUT);
    pinMode(level2, INPUT);
    pinMode(level3, INPUT);

    pinMode(dataLed, OUTPUT);
    pinMode(serverLed, OUTPUT);
    pinMode(countingLed, OUTPUT);


    // catching and logging different wifi events
    WiFi.onEvent(WifiStaConnected, SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(WifiScanDone, SYSTEM_EVENT_SCAN_DONE);
    WiFi.onEvent(WifiStaDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent(WifiStaStart, SYSTEM_EVENT_STA_START);
    WiFi.onEvent(WifiStaStop, SYSTEM_EVENT_STA_STOP);
    WiFi.onEvent(WifiAPStart, SYSTEM_EVENT_AP_START);
    WiFi.onEvent(WifiAPStop, SYSTEM_EVENT_AP_STOP);
    

    if(!SPIFFS.begin()){
      Serial.println("An error has ocurred while mounting SPIFFS");
      return;
    }

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
    
    // try to get watering program from memory
    wprogram = setWProgramFromMemory();

    scanNets();
  
    // try to connect to the network 
    connectToSavedNetwork();

    if(!isConnected()){
      delay(1000);
      scanNets();
      createSoftApConnection();
    }

    // mqtt settings 
    //espClient.setCACert(ca_cert); // SSL/TLS certificate
    //client.setServer(mqttServer, mqttPort);
    //client.setCallback(dataCallback);
    //currProgram = getMemProgram();
    printProgram();

    timeClient.begin();
    timeClient.setTimeOffset(wprogram.gmtTimezone);

}

// ***************** LOOP *********************************
void loop() {

  int currMins = 0;
  
  // STATE MACHINE PROCESS 
  switch(state){

    case CHECK_CONNECTION_STATE:
      //Serial.println("check connection state");
      if(isConnected()){
        
        state = TIME_CHECK_STATE;
        //Serial.println("WiFi conectado! Verificando proximas tarefas.");
        break;
      } 
      
      else {

        state = SELECT_NETWORKS_STATE;
        //if(!connectToSavedNetwork()){
        //  Serial.println("WiFi não conectado! Vamos tentar outra rede");
        //  state = SELECT_NETWORKS_STATE;
          //scanNets(); 
        //}
          
      }
      break;

    case SELECT_NETWORKS_STATE:
    
      // create soft ap connection
      // for selection of network
      createSoftApConnection();
      if(sendInfo){ // this could be solved with posix
        exportInfo(); 
        sendInfo = false;
      }

      // process user selected network.
      // todo: make function to eventually end soft ap connection 
      if (isSoftApConnected && msgAvailable){
        processSelectedNet(recvMsg);
        msgAvailable = false;
      }

      if (isConnected()) state = TIME_CHECK_STATE;
      
      break;

    case TIME_CHECK_STATE:
      //Serial.println("time check state");
      state = BROKER_CONNECTION_STATE; // next state
      
      if(!timeClient.update()){
        
        timeClient.forceUpdate();

        Serial.println(timeClient.getHours());
        Serial.println(timeClient.getMinutes());
        Serial.println(timeClient.getSeconds());
        Serial.println(timeClient.getDay());

        // if the clock cant be updated after some tries, restart board
        attemptsAtScheduling++;
        if(attemptsAtScheduling==maxAttemptsAtScheduling){
          ESP.restart();
        }

              
      } else attemptsAtScheduling = 0;

      if(wprogram.automaticWatering) break;

      if(isDeadline()){
        state = PERFORMING_WATERING_STATE;
        break;
      }

      break;

    case BROKER_CONNECTION_STATE:
    
      //Serial.println("broker connection state");
      
      if(!client.connected()){
        
        if(reconnectToBroker()){
          digitalWrite(serverLed, HIGH);
          state = SENSOR_CHECK_STATE;
          break;
        }
        
        digitalWrite(serverLed, LOW);
        state = CHECK_CONNECTION_STATE;
        break;
        
      }

      //Serial.println("Broker ok..");
      client.loop();
      state = SENSOR_CHECK_STATE;
      break;

    

    case SENSOR_CHECK_STATE:
      // here be sensor checking (humidity) related actions
      // TODO: instead of using interruptions, we can check the state 
      // of the other level sensors in previous times to assure if tank
      // is going up or down.

      checkWaterLevel();

      /* if automatic watering is enabed, the "verifySoilHumidty" function
         shall water the plants when above a certain dryness threshold, otherwise the 
         values of soil sensor will be simply registered on a global var.
      */
      currMins = timeClient.getMinutes();
      // we only check soil moisture in 5 minutes intervals
      if(wprogram.automaticWatering == true && currMins != lastSoilMCheckedMinute && isMinute5Multiple()){
        // if soil is dry and plants havent been watered in the current hour
        Serial.println("soil humidity checking");
        if(isSoilDry() && (getLastWateredStatus() != timeClient.getHours())){
          pumpAction(wprogram.amountWater);
        }
        
      } 

      state = CHECK_CONNECTION_STATE;
      //delay(500);

      break;

    case PERFORMING_WATERING_STATE:
     // here watering will happen when conditions are met
     // this part is skipped for now
     Serial.println("realizando regagem.");
     //publishMsg(pingPath, "okei! Vou jogar uma aguinha"); 
     testWateringSendMsg();
     pumpAction(wprogram.amountWater);
     state = CHECK_CONNECTION_STATE; // restart cicle
      
     break;
     
    default:
      state = CHECK_CONNECTION_STATE;
    

    
  }

  
}



// ***************** /LOOP *********************************

//TODO: make this a generic function to send
// all types of status messages
void testWateringSendMsg(){
   StaticJsonDocument<200> jsonMsg;
   JsonObject root = jsonMsg.to<JsonObject>();
   root["type"] = "wateringNow";
   root["waterVolume"] = wprogram.amountWater;

   char msgToSend[80];
   serializeJson(root, msgToSend);
   Serial.println("Sending status msg:");
   Serial.println(msgToSend);
   Serial.println("");
   publishMsg(pingPath, msgToSend);
}

//****************** LEVEL SENSORS *************************

void checkWaterLevel(){

  int absLevel = 0; // local level

  int l1, l2, l3;
  l1 = digitalRead(level1);
  l2 = digitalRead(level2);
  l3 = digitalRead(level3); // this one works
  
  //Serial.print("Level 1: ");
  //Serial.println(l2);
  //Serial.print("Level 2: ");
  //Serial.println(l2);
  //Serial.print("Level 3: ");
  //Serial.println(l3);
  

  if(l1) absLevel = 30;
  if(l2 && l1) absLevel = 50;
  if(l3 && l2 && l1) absLevel = 75;
  if(l3 && !l1 || l3 && !l2 && l1 || l2 && !l1) absLevel = -1; // malfunctioning sensor

  if(absLevel != waterLevel){
    waterLevel = absLevel;
    setLevel(absLevel);
    broadcastWaterLevel();
  }

  
}

void setLevel(int level){
  Serial.println("[FUNCTION] setLevel()");

  preferences.begin(PREFS, false);
  preferences.putInt("waterLevel", level);
  preferences.end();

  
}

int getLevel(){
  Serial.println("[FUNCTION] getLevel()");

  preferences.begin(PREFS, false);
  int level = preferences.getInt("waterLevel", 0);
  preferences.end();

  return level;
  
}

void broadcastWaterLevel(){
  Serial.println("[FUNCTION] broadcastWaterLevel()");

  StaticJsonDocument<200> jsonMsg;
  JsonObject root = jsonMsg.to<JsonObject>();
  //JsonObject& root = jsonMsg.createObject();
  root["type"] = "levelSensor";
  root["waterLevel"] = waterLevel;

  char msgToSend[80];
  serializeJson(root, msgToSend);
  Serial.println("Sending status msg:");
  Serial.println(msgToSend);
  Serial.println("");
  publishMsg(pingPath,msgToSend);
  
}

//****************** /LEVEL SENSORS *************************


//****************** OFFLINE INTERFACE *********************

/*
 * Since should not hardcode an IP address for the webpage
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
 * create a soft ap server connetion
 * for selecting the wifi network.
*/
void createSoftApConnection(){


  if(isSoftApConnected){
    return;
    
  } else{
    //scanNets();
    Serial.println("opening soft connection");

    WiFi.softAP(softApSSID, softApPWD);
    ipAddr = WiFi.softAPIP();
    server.begin();

    isSoftApConnected = true;

    // formatting the ip address into a suitable string
    // to be sent to the webpage
    char buf[20];  
    sprintf(buf,"%d.%d.%d.%d", ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
    ip = String(buf);
  
    //Serial.println(buf);
    //Serial.println(ip);
    //Serial.println(WiFi.localIP());
  
    Serial.println("Soft AP connection created");
    Serial.print("Navigate to: ");
    Serial.print("http://");
    Serial.print(ip);
    Serial.println("/html");

    
    
  }
  
  
  
}

void endSoftApConnection(){
  
  // TODO: research the method "beginSecure()" of server
  // TODO: research enabling SSL security
  server.end(); // end server connection
  WiFi.softAPdisconnect(); // remove AP server
  isSoftApConnected = false;
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
    sendInfo = true;
    
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
    
  }
  
}


/*
 * After the user selects the desired wifi network from
 * the interface, its data need to be processed (unpacked)
 * and connection attempted.
*/
void processSelectedNet(String msg){
 
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
  
  connectToSelectedNet2(netname, pwd);
  //endSoftApConnection();
    
  
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

    delay(2000);

  }
  
}

// function to decode the encryption type codes.
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

//******************/OFFLINE INTERFACE *********************

//****************** MOISTURE SENSOR ***********************
// is the current minute a multiple of five?
bool isMinute5Multiple(){
  return (timeClient.getMinutes() % 5 == 0) ;
}


/* Simple function to verify the soil humidity.
   (test the threshold values with sensor inside plant */
bool isSoilDry(){

  digitalWrite(soilSensorPower, HIGH);
  delay(500);
  soilHumidity = analogRead(soilSensorPin);
  lastSoilMCheckedMinute = timeClient.getMinutes();

  sendHSensorStatusMessage(soilHumidity);

  if(soilHumidity > drySoilThreshold){
    Serial.print("Soil is dry | Dryness: ");
    Serial.println(soilHumidity);
    return true;
    
  } else{
    Serial.print("Soil is not dry | Dryness: ");
    Serial.println(soilHumidity);
    return false;
  }

  digitalWrite(soilSensorPower, LOW);
  

}

void sendHSensorStatusMessage(int humidity){
  Serial.println("[FUNCTION] sendHSensorStatusMessage()");

  String statusMsg;
  String humidityStr = String(humidity);
  statusMsg.reserve(30);

  //statusMsg = "Dryness: ";
  statusMsg = humidityStr;

  StaticJsonDocument<200> jsonMsg;
  JsonObject root = jsonMsg.to<JsonObject>();
  //JsonObject& root = jsonMsg.createObject();
  root["type"] = "moistureSensor";
  root["lastWatered"] = getLastWateredStatus();
  root["Dryness"] = statusMsg;

  char msgToSend[80];
  serializeJson(root, msgToSend);
  Serial.println("Sending status msg:");
  Serial.println(msgToSend);
  Serial.println("");
  publishMsg(pingPath,msgToSend);

  
  
}


//******************/MOISTURE SENSOR ***********************

// ***************** PROGRAM FUNCTIONS ************************

/* Since after after the first program setting we'll save it to memory 
 * we'll need to retrieve it after every new startup of the system. This 
 * function gets the json string from memory and makes it default in the 
 * system.
*/
WateringProgram setWProgramFromMemory(){

  Serial.println("[FUNCTION] setWProgramFromMemory()");

  WateringProgram w;
  preferences.begin(PREFS, false);
  w.amountWater = preferences.getInt("amountWater", 0);
  w.gmtTimezone = preferences.getInt("gmtTimeZone", MANAUSGMT);
  w.deadlineHour = preferences.getInt("deadlineHour", 0);
  w.deadlineMinute = preferences.getInt("deadlineMinute", 0);
  w.deadlineSecond = preferences.getInt("deadlineSecond", 0);
  w.deadlineDays[0] = preferences.getInt("day0",-1);
  w.deadlineDays[1] = preferences.getInt("day1",-1);
  w.deadlineDays[2] = preferences.getInt("day2",-1);
  w.deadlineDays[3] = preferences.getInt("day3",-1);
  w.deadlineDays[4] = preferences.getInt("day4",-1);
  w.deadlineDays[5] = preferences.getInt("day5",-1);
  w.deadlineDays[6] = preferences.getInt("day6",-1);
  w.automaticWatering = preferences.getInt("automaticWatering", 0);

  return w;
  
}

void saveProgramToMemory(WateringProgram w){
  Serial.println("[FUNCTION] saveProgramToMemory()");
  
  preferences.begin(PREFS, false);
  preferences.putInt("deadlineHour",w.deadlineHour);
  preferences.putInt("deadlineMinute",w.deadlineMinute);
  preferences.putInt("deadlineSecond", w.deadlineSecond);
  preferences.putInt("amountWater", w.amountWater);
  preferences.putInt("gmtTimezone", w.gmtTimezone);
  if(w.automaticWatering) {
    preferences.putInt("automaticWatering", 1);
  } else preferences.putInt("automaticWatering", 0);
  
  Serial.print("Auto watering: ");
  Serial.println(w.automaticWatering);
  preferences.putInt("day0", w.deadlineDays[0]);
  Serial.println(w.deadlineDays[0]);
  preferences.putInt("day1", w.deadlineDays[1]);
  Serial.println(w.deadlineDays[1]);
  preferences.putInt("day2", w.deadlineDays[2]);
  Serial.println(w.deadlineDays[2]);
  preferences.putInt("day3", w.deadlineDays[3]);
  Serial.println(w.deadlineDays[3]);
  preferences.putInt("day4", w.deadlineDays[4]);
  Serial.println(w.deadlineDays[4]);
  preferences.putInt("day5", w.deadlineDays[5]);
  Serial.println(w.deadlineDays[5]);
  preferences.putInt("day6", w.deadlineDays[6]);
  Serial.println(w.deadlineDays[6]);
  preferences.end();
  
  
}


void pumpAction(long int quantity){

  /*
   * Starting from inicial calculations assuming the ration
   * pumping/Voltage is linear and proportional we calculate
   * that under 9.0V we'll pump 49.5mL/s (test this).
   * 
   */

  // quantity is the value in mL to be pumped.
  if(!isWatering){
    Serial.println("realizando regagem");
    int pumpDelay = (quantity/49)*1000;
    isWatering = true;
    //lastWateredHour = timeClient.getHours();
    setLastWateredStatus(timeClient.getHours());
    

    // create an async task for pumping water
    xTaskCreate(
                  pumpTask,           // task function
                  "pumpTask",         // string w/ name of task
                  10000,              // stack size in bytes
                  (void *)&pumpDelay, // parameter passed as input
                  1,                  // priority of the task
                  NULL);              // task handle

        
  }
  else Serial.println("irrigation in progress");
  
}

void pumpTask(void *delayTime){
  int dt = *((int *) delayTime); 
  digitalWrite(pumpPin, HIGH);
  delay(dt);
  digitalWrite(pumpPin, LOW);
  //Serial.println("irrigation finished");
  //Serial.println(dt);
  isWatering = false;
  vTaskDelete( NULL );
}

// just prints the current program to the serial
void printProgram(){
    Serial.println("CURRENT PROGRAM: [");
    Serial.print("IRRIGATION TIME: ");
    Serial.print(wprogram.deadlineHour);
    Serial.print(":");
    Serial.print(wprogram.deadlineMinute);
    Serial.print(":");
    Serial.println(wprogram.deadlineSecond);
    Serial.print("AMOUNT OF WATER: ");
    Serial.println(wprogram.amountWater);
    Serial.print("TIMEZONE: ");
    Serial.println(wprogram.gmtTimezone);
    Serial.print("Automatic watering: ");
    Serial.println(wprogram.automaticWatering);
    Serial.print("Watering days:");
    for(int i = 0; i < 7; i++){
      Serial.print(" ");
      Serial.print(wprogram.deadlineDays[i]);
    }
    Serial.println(" ");
}

/*  Sometimes no guidelines for watering programs are provided, in those 
 *  cases we'll just set the settings to half a liter of water dispensed and 
 *  automatic watering (dry soil). This function returns a watering program 
 *  with only basic values.
*/
WateringProgram getBasicProgram(){
  Serial.println("getting the basic wateringsystem program");
  
  WateringProgram basic;
  basic.amountWater = 500;
  basic.gmtTimezone = MANAUSGMT;
  basic.deadlineHour = 0;
  basic.deadlineHour = 0;
  basic.deadlineMinute = 0;
  basic.deadlineSecond = 0;
  for (int i; i < 7; i++){
    basic.deadlineDays[i] = -1;
  }
  basic.automaticWatering = true;

  return basic;
  
}

void setLastWateredStatus(int lStatus){

    Serial.println("[FUNCTION] setLastWateredStatus()");

    preferences.begin(PREFS, false);
    preferences.putInt("lastWatered", lStatus);
    preferences.end();

}


int getLastWateredStatus(){

    Serial.println("[FUNCTION] getLastWateredStatus()");

    preferences.begin(PREFS, false);
    int lStatus = preferences.getInt("lastWatered", -1);
    preferences.end();

    return lStatus;

}


// ***************** /PROGRAM FUNCTIONS ************************


// ************** PUBLISH TO MQTT BROKER *******************

/*
 publish message on the specified topic in the MQTT broker
 return false if client isnt connected
*/
bool publishMsg(const char* path, const char* msg){
  
  if(client.connected()){

    client.publish(path, msg);
    Serial.print("Mensagem enviada: ");
    Serial.println(msg);
    Serial.print("Para: ");
    Serial.println(path);
    return true;
  }

  else{

    Serial.println("cliente não conectado");
    return false;
    
  }

  digitalWrite(dataLed, HIGH);
  delay(300);
  digitalWrite(dataLed, LOW);
  
  
}




// *********************************************************

// ***** CALLBACK FOR SUBSCRIPTIONS FROM MQTT BROKER ********

// since we needed to simplify the code for better understanding
// this simple function returns if the topic and the path are the same
bool isPath(char* topic, const char* path){
  return strcmp(topic, path) == 0;
}

// since we need to control the hardware scheduling and actions
// through user input using the mqtt and android app, we have this callback.
// this function is called when a message from the mqtt broker arrives.
void callback(char* topic, byte* payload, unsigned int length){

  Serial.println("Callback called");

  char payloadStr[length + 2];
  memset(payloadStr, 0, length + 1);
  strncpy(payloadStr, (char*)payload, length);
  Serial.printf("Data    : dataCallback. Topic : [%s]\n", topic);
  Serial.printf("Data    : dataCallback. Payload : %s\n", payloadStr);

  /*char mqttMessage[100];  // TODO: payloadStr already does this line
  for (int i=0;i<length;i++) {
    mqttMessage[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }*/
  
  if(isPath(topic, subscribeCtrlPath)) {
    state = PERFORMING_WATERING_STATE;
    publishMsg(pingPath, "okei! Vou jogar uma aguinha"); 
    pumpAction(wprogram.amountWater);
  }

  if(isPath(topic, setupWateringProgram)) changeDefaultProgram(payloadStr);

  if(isPath(topic, publishReport)){
    char level[3];
    sprintf(level, "%d", waterLevel);
    publishMsg(publishLevelPath, level); 
  } 
  
}

// Since this code uses a "default" watering program, a 
// global variable object that contains the data of the 
// watering program currently being used, we have this
// function that changes this default object if requested 
// by the user through the network. After everything, it saves 
// the json string in memory. If no json is provided, the default 
// program is set for the basic values.
//void changeDefaultProgram(char message[100]){
void changeDefaultProgram(char* message){

  
  Serial.println("[FUNCTION]  changeDefaultProgram()"); 

  const size_t capacity = JSON_ARRAY_SIZE(7) + JSON_OBJECT_SIZE(6) + 100;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, message);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.c_str());
    wprogram = getBasicProgram();
    return;
  }
  

  wprogram.amountWater = doc["amountWater"]; 
  wprogram.gmtTimezone = doc["gmtTimezone"];
  wprogram.deadlineHour = doc["deadlineHour"]; 
  wprogram.deadlineMinute = doc["deadlineMinute"]; 
  wprogram.deadlineSecond = doc["deadlineSecond"];
  JsonArray dataDays = doc["deadlineDays"];
  for (int i = 0; i < 7; i++){
    Serial.println((int)dataDays[i]);
    wprogram.deadlineDays[i] = dataDays[i]; 
  }
  wprogram.automaticWatering = doc["automaticWatering"];

  Serial.println("default program changed:");
  saveProgramToMemory(wprogram);
  printProgram();
  

}

// **********************************************************



// ************** CONNECTION TO MQTT BROKER ****************

// link to tutorial: https://www.arduinoecia.com.br/2019/02/enviando-mensagens-mqtt-modulo-esp32-wifi.html
// Function that connects to the global-directed broker and returns a boolean indicating 
// if the connection was successful or not so we can send and receive information from 
// the other peripherals in the network.
bool reconnectToBroker(){

  int reconnectionDelay = 1000;
  int connectionAttempts = 0;
  int mqttPortInfo;


  Serial.print("Tentando conexão com o broker ");
  if(secureConnect){
    //espClient.setCACert(ca_cert);
    //client.setServer(mqttServer, secureMqttPort);
    //Serial.print("(TLS) ");
    //mqttPortInfo = secureMqttPort;
  } 
  else{
    client.setServer(mqttServer, mqttPort);
    mqttPortInfo = mqttPort;  
  }
  
  client.setCallback(callback);

  
  Serial.println(mqttServer);
  Serial.print("Usuario: ");
  Serial.println(mqttUser);
  Serial.print("Porta: ");
  Serial.println(mqttPortInfo);

  // power off will message
  StaticJsonDocument<200> jsonMsg;
  JsonObject root = jsonMsg.to<JsonObject>();
  root["type"] = "powerStatus";
  root["powerOn"] = false;
  char willMsg[80];
  serializeJson(root, willMsg);

  
  while(!client.connected() && (connectionAttempts < 30)){
    
    Serial.print("tentativas ");
    Serial.println(connectionAttempts);
    
    if(client.connect("EddiesHomeWS",mqttUser,mqttPassword, powerPath, 2, true, willMsg, true)){
      
      Serial.println("Conectado ao Broker!");
      client.subscribe(subscribeTimingPath);
      client.subscribe(subscribeCtrlPath);
      client.subscribe(subscribeAmountPath);
      client.subscribe(publishReport);
      client.subscribe(setupWateringProgram);
      //client.subscribe(powerPath);
      //publishMsg(pingPath, "Olá! Dispositvo SmartFarm conectado!{}");

      //StaticJsonDocument<200> jsonMsg;
      //JsonObject root = jsonMsg.to<JsonObject>();
      root["type"] = "powerStatus";
      root["powerOn"] = true;

      char msgToSend[80];
      serializeJson(root, msgToSend);
      Serial.println("Sending status msg:");
      Serial.println(msgToSend);
      Serial.println("");
      //publishMsg(pingPath, msgToSend);
      client.publish(powerPath, msgToSend, true);
      return true;
      
    } 
    
    delay(reconnectionDelay);
    connectionAttempts++;
    
  }

  Serial.print("Não conectado - estado cliente: ");
  Serial.println(client.state());
  return false;

  
}

// **************************************************


// ***************** RTC TIME REGISTERING AND PUMP CONTROL **********

// Since we need to check for the deadline of the watering action
// this function checks if current day, hour, minute and second match with 
// the default watering program deadline.
bool isDeadline(){
  int weekDay = timeClient.getDay();
  bool isDay = false;
  for(int i=0; i<7; i++){

    if(weekDay == wprogram.deadlineDays[i]) isDay = true;
    
  }

  if(!isDay) return false;
  
  if ((wprogram.deadlineHour == timeClient.getHours()) && (wprogram.deadlineMinute == timeClient.getMinutes())){
    if(wprogram.deadlineSecond == timeClient.getSeconds()) return true;
  }

  return false;
}



// ***************** /RTC TIME REGISTERING AND PUMP CONTROL **********




// ***************** CONNECTION STATUS ******************************
bool isConnected(){
  return WiFi.status() == WL_CONNECTED;
}

// function counts a timeout for connecting to a network 
bool connectedAfterTimeout(String net, String pwd){
  int attemptsAcc = 0; // attempts to connect
  
  while(!isConnected() && (attemptsAcc < 20)){
    delay(500);
    Serial.print("connectando com wifi ");
    Serial.println(net);
    attemptsAcc++;
      
  }

  if(isConnected()){
      
     Serial.print("Connectado com a rede ");
     Serial.println(net);

     saveCredentials(net, pwd); // if connection successful, save details to memory
     return true;
  } 
  else{
    
     Serial.println("Failed to connect to network");
     return false;
  }

}



void checkNetStatus(){
  // if we are still connected just light the builtin led
  // since not everytime the connecting routine lights it up
  if(conncted){
    
    if(WiFi.status() == WL_CONNECTED){
      //digitalWrite(LED_BUILTIN, HIGH); // uncomment if we are on ESP32 DOIT
    } else {
      // if not, turn it off and attempt to connect to previously
      // connected network, then check connection again
      conncted = false;
      //digitalWrite(LED_BUILTIN, LOW); // uncomment if we are on ESP32 DOIT
      



    }
    
  }
  
  
}

// ***************** /CONNECTION STATUS ******************************


// ***************** SCAN FOR NETWORKS *******************************
int scanNets(){

  delay(1000);
  // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    availableNets = n;
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
// ***************** /SCAN FOR NETWORKS *******************************


// **************** CONNECT TO SELECTED NETWORK ***********************

// simple function to connect using given credentials
void connectToSelectedNet2(String netname, String pwd){

  char net[60];
  char pass[60];

  netname.toCharArray(net,60);
  pwd.toCharArray(pass,60);

  WiFi.begin(net, pass);
  connectedAfterTimeout(net, pass);
  
}

void connectToSelectedNet(int input){
  
  /* if we arent connected the input is handling
  connection */
  if(!isConnected()){
    if(input <= availableNets){
      
      //if the selected network is open, try connect straight away
      if(WiFi.encryptionType(input-1) == WIFI_AUTH_OPEN){
        WiFi.begin(WiFi.SSID(input-1).c_str());
       
      }
      
      // or else, we'll need the password to connect to it 
      else {
        Serial.print("Password for network: ");
        Serial.println(WiFi.SSID(input-1));
        // wait for password 
        String pwd = "";
        while(pwd == ""){
          //delay(10);
          pwd = Serial.readString();
           
        }
        
        char net[60];
        char pass[60]; 
        pwd.toCharArray(pass, 60);
        WiFi.SSID(input-1).toCharArray(net, 60);
        Serial.print("password input was: ");
        Serial.println(pass);
        WiFi.begin(net, pass); // mock attempt to check
        
        if(WiFi.scanComplete() > -2){ // if WiFi.scanComplete() is -2 then it hanst been triggered
          WiFi.scanDelete();
        }

        connectedAfterTimeout(net,pass);

             
      }
           
    }
  }
  // later on we'll add more handling to this 
  
  
  
}

// **************** /CONNECT TO SELECTED NETWORK **********************


// START DISCONNECT
void disconnection(){
  if(WiFi.status() == WL_CONNECTED){
    WiFi.disconnect();
    Serial.println("disconnected from network");
  } else Serial.println("device is not connected to any network");
}
// END DISCONNECT


// START CONNECTING TO MEMORY-SAVED NETWORK
bool connectToSavedNetwork(){ 
    // using home connection
    getLastSavedCredentials(); // get details from last saved network
    char net[60];
    char pass[60];

    WiFi.disconnect(); // disconnect before trying to connect
    
    if(network != "---" && password != "---"){ // if we have connection details in store

      Serial.print("Connecting to network ");
      Serial.print(network);
      Serial.println("...");

      network.toCharArray(net, 60);
      
      WiFi.mode(WIFI_STA);
      if(password.length() > 0){
        password.toCharArray(pass,60);
        WiFi.begin(net, pass);
      } else WiFi.begin(net);

      return connectedAfterTimeout(net, pass);
      
    }

    return false;

    
  
}
// END CONNECTING TO MEMORY-SAVED NETWORK


// START SAVING NETWORK CREDENTIALS ON MEMORY
void saveCredentials(String net, String pwd){
  Serial.print("saving ");
  Serial.print(net);
  Serial.println(" to memory");
  
  preferences.begin("my-app",false);
  preferences.putString("network",net);
  delay(500);
  preferences.putString("password", pwd);
  delay(500);
  preferences.end();

  Serial.println("Netowork saved");

  
}
// END SAVING NETWORK CREDENTIALS ON MEMORY

// START GETTING LAST SAVED CREDENTIALS FROM MEMORY
void getLastSavedCredentials(){
  preferences.begin("my-app",false);
  network = preferences.getString("network","---");
  password = preferences.getString("password","---");
  Serial.print("last saved net: ");
  Serial.println(network);
  Serial.print("last saved password: ");
  Serial.println(password);
  preferences.end();
}
// END GETTING LAST SAVED CREDENTIALS FROM MEMORY

//***************** LOGGING *****************************//
void WifiScanDone(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("SCAN DONE");
}

void WifiNoAPFound(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("NO AP FOUND");
}

void WifiStaStart(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("STA START");
  // makeshift fix for a problem with the connection where 
  // the system stays locked on STA_START
  if(!isConnected()) WiFi.disconnect();
  
}

void WifiReadyLog(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WIFI READY");
}

void WifiStaConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("STA CONNECTED");
  if(isSoftApConnected) endSoftApConnection();
}

void WifiStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("STA DISCONNECTED");
  conncted = false;
}

void WifiStaStop(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("STA STOP");
}

void WifiAPStart(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("AP START");
}

void WifiAPStop(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("AP STOP");
}

// code for interruption: https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
