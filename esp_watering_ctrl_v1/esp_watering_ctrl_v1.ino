#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <PubSubClient.h>
//#include "BluetoothSerial.h"
#include <Preferences.h>
#include "TOD.h"
#include "WateringPlan.h"

#define PREFS "my-app"
#define MANAUSGMT -14400 // MANAUS GMT TIME = -4

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

// certificado para acesso usando TLS
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFdDCCBFygAwIBAgIQJ2buVutJ846r13Ci/ITeIjANBgkqhkiG9w0BAQwFADBv\n" \
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n" \
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n" \
"eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n" \
"gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n" \
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\n" \
"VQQDEyJDT01PRE8gUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjANBgkq\n" \
"hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAkehUktIKVrGsDSTdxc9EZ3SZKzejfSNw\n" \
"AHG8U9/E+ioSj0t/EFa9n3Byt2F/yUsPF6c947AEYe7/EZfH9IY+Cvo+XPmT5jR6\n" \
"2RRr55yzhaCCenavcZDX7P0N+pxs+t+wgvQUfvm+xKYvT3+Zf7X8Z0NyvQwA1onr\n" \
"ayzT7Y+YHBSrfuXjbvzYqOSSJNpDa2K4Vf3qwbxstovzDo2a5JtsaZn4eEgwRdWt\n" \
"4Q08RWD8MpZRJ7xnw8outmvqRsfHIKCxH2XeSAi6pE6p8oNGN4Tr6MyBSENnTnIq\n" \
"m1y9TBsoilwie7SrmNnu4FGDwwlGTm0+mfqVF9p8M1dBPI1R7Qu2XK8sYxrfV8g/\n" \
"vOldxJuvRZnio1oktLqpVj3Pb6r/SVi+8Kj/9Lit6Tf7urj0Czr56ENCHonYhMsT\n" \
"8dm74YlguIwoVqwUHZwK53Hrzw7dPamWoUi9PPevtQ0iTMARgexWO/bTouJbt7IE\n" \
"IlKVgJNp6I5MZfGRAy1wdALqi2cVKWlSArvX31BqVUa/oKMoYX9w0MOiqiwhqkfO\n" \
"KJwGRXa/ghgntNWutMtQ5mv0TIZxMOmm3xaG4Nj/QN370EKIf6MzOi5cHkERgWPO\n" \
"GHFrK+ymircxXDpqR+DDeVnWIBqv8mqYqnK8V0rSS527EPywTEHl7R09XiidnMy/\n" \
"s1Hap0flhFMCAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTvA73g\n" \
"JMtUGjAdBgNVHQ4EFgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQD\n" \
"AgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1UdHwQ9\n" \
"MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVy\n" \
"bmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0dHA6\n" \
"Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAGS/g/FfmoXQ\n" \
"zbihKVcN6Fr30ek+8nYEbvFScLsePP9NDXRqzIGCJdPDoCpdTPW6i6FtxFQJdcfj\n" \
"Jw5dhHk3QBN39bSsHNA7qxcS1u80GH4r6XnTq1dFDK8o+tDb5VCViLvfhVdpfZLY\n" \
"Uspzgb8c8+a4bmYRBbMelC1/kZWSWfFMzqORcUx8Rww7Cxn2obFshj5cqsQugsv5\n" \
"B5a6SE2Q8pTIqXOi6wZ7I53eovNNVZ96YUWYGGjHXkBrI/V5eu+MtWuLt29G9Hvx\n" \
"PUsE2JOAWVrgQSQdso8VYFhH2+9uRv0V9dlfmrPb2LjkQLPNlzmuhbsdjrzch5vR\n" \
"pu/xO28QOG8=\n" \
"-----END CERTIFICATE-----\n";




String network = "Os Silva Wi Fi";
String password= "c8906d2932";
bool conncted = false;

volatile int interruptCounter = 0;

// pinos de interrupção dos sensores
int level1Interrupt = 14;
int level2Interrupt = 27;
int level3Interrupt = 26;

int serverLed = 25;
int countingLed = 33;
int dataLed = 32;

// pump pin
int pumpPin = 12;

// ******* MQTT VARIABLEs *****************
// mqtt broker currently running on maqiatto
const char* mqttServer = "maqiatto.com";
const int mqttPort= 1883; // tcp port 
//const int mqttPort= 3883; // tls port
const char* mqttUser = "netosilvan78@gmail.com";
const char* mqttPassword = "y4NW1jgJi3b7";

const char* subscribeCtrlPath = "netosilvan78@gmail.com/system/control/dispense";
const char* subscribeTimingPath = "netosilvan78@gmail.com/system/api/timing";
const char* subscribeAmountPath = "netosilvan78@gmail.com/system/app/amount";
const char* publishTempPath = "netosilvan78@gmail.com/system/hardware/temp";
const char* publishLevelPath = "netosilvan78@gmail.com/system/hardware/level";
const char* publishReqNextSchedule = "netosilvan78@gmail.com/system/hardware/nextWSchedule";
const char* setupWateringProgram = "netosilvan78@gmail.com/system/hardware/setupProgram";
const char* publishReport = "netosilvan78@gmail.com/system/report";
const char* pingPath = "netosilvan78@gmail.com/system/ping";

const char* wateredMessage = "JUST_WATERED";
const char* lowLvMessage = "LOW_WATER";


WiFiClient espClient;
//WiFiClientSecure espClient;
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


//****** SETUP OF RTC MODULE ***************

template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } //Sets up serial streaming Serial<<"text"<<variable;
TOD RTC; //Instantiate Time of Day class TOD as RTC
uint8_t lastminute=0;
uint8_t lastsecond=0;
char printbuffer[50];
bool TODvalid=false;

//****** /SETUP OF RTC MODULE ***************

// ***** SCHEDULE & WATERING VARIABLES *******
// TODO :  remove these time variables when the new wprogram is implemented
int current_interval = 0;
uint8_t deadline_hour = 0;
uint8_t deadline_minute = 0;
long int newAmount = 0;
volatile unsigned int waterLevel = 0; // modified by level interruptions
// variables to track the sensors 
bool lv1 = false;
bool lv2 = false;
bool lv3 = false;

// TODO: remove this reference when the new program 
// is implemented
Program currProgram;

WateringProgram wprogram;

// ***** /SCHEDULE & WATERING VARIABLES *******

int availableNets = 0;
String inData = "";


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux =  portMUX_INITIALIZER_UNLOCKED;
bool toggleLed = 0;

// function to execute when interruption triggers
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux); // lock main thread for executing critical code
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

//BluetoothSerial SerialBT;

void IRAM_ATTR levelChange1(){

  // toggle the related debouncer 
  // and change level
  if(!debounce[0]){

    lv1 = !lv1;

    // if water above the level 1 sensor
    // then it's about < 50
    // if below then it's < 40
    if(lv1) waterLevel = 50;
    else waterLevel = 40;
    
    debounce[0] = !debounce[0];

    Serial.print("level: ");
    Serial.print(waterLevel);
    Serial.print(" | debouncer: ");
    Serial.println(debounce[0]);
    
    
  }
  
 
  
}

void IRAM_ATTR levelChange2(){

  // toggle the related debouncer 
  // and change level
  if(!debounce[1]){

    lv2 = !lv2;
    
    // if water above the level 2 sensor
    // then it's about < 70
    // if below then it's < 50
    if(lv2) waterLevel = 70;
    else waterLevel = 50;
    
    debounce[1] = !debounce[1];
    
  }

  
}

void IRAM_ATTR levelChange3(){

  // toggle the related debouncer 
  // and change level
  if(!debounce[2]){

    lv3 = !lv3;
    
    // if water above the level 2 sensor
    // then it's about < 90
    // if below then it's < 70
    if(lv3) waterLevel = 90;
    else waterLevel = 70;
    
    debounce[1] = !debounce[1];
    
  }

}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("Initialization...");
    
    //pinMode(LED_BUILTIN, OUTPUT);
    pinMode(pumpPin, OUTPUT);

    // configuring interruption pins
    pinMode(level1Interrupt, INPUT_PULLUP);
    pinMode(level2Interrupt, INPUT_PULLUP);
    pinMode(level3Interrupt, INPUT_PULLUP);
    
    // attaching interruptions linked to the water sensors
    attachInterrupt(level1Interrupt, levelChange1, CHANGE);
    attachInterrupt(level2Interrupt, levelChange2, CHANGE);
    attachInterrupt(level3Interrupt, levelChange3, CHANGE);

    pinMode(dataLed, OUTPUT);
    pinMode(serverLed, OUTPUT);
    pinMode(countingLed, OUTPUT);
    

    // try to get watering program from memory
    setWProgramFromMemory();

    // try to connect to the network 
    connectToSavedNetwork();
    
    // mqtt settings 
    //espClient.setCACert(ca_cert); // SSL/TLS certificate
    client.setServer(mqttServer, mqttPort);
    client.setCallback(dataCallback);
    currProgram = getMemProgram();
    printProgram();

    // TODO: customize this so we get the gmt from the loaded program.
    timeClient.begin();
    timeClient.setTimeOffset(wprogram.gmtTimezone);

    
}

// ***************** LOOP *********************************
void loop() {

  // STATE MACHINE PROCESS 
  switch(state){

    case CHECK_CONNECTION_STATE:

      if(isConnected()){
        state = TIME_CHECK_STATE;
        //Serial.println("WiFi conectado! Verificando proximas tarefas.");
        break;
      } 
      
      else {

        if(!connectToSavedNetwork()){
          Serial.println("WiFi não conectado! Vamos tentar outra rede");
          state = SELECT_NETWORKS_STATE;
          scanNets(); 
        }
          
      }
      break;

    case SELECT_NETWORKS_STATE:
     
      if(Serial.available() > 0){
        char input = Serial.read(); 
        char inp[12]= {input};
        int in = atoi(inp); // convert from ascII to int
        connectToSelectedNet(in);

        if (isConnected()) state = TIME_CHECK_STATE;
        
      }
      break;

    case TIME_CHECK_STATE:

      state = BROKER_CONNECTION_STATE; // next state
      
      if(!timeClient.update()){
        
        timeClient.forceUpdate();

        Serial.println(timeClient.getHours());
        Serial.println(timeClient.getMinutes());
        Serial.println(timeClient.getSeconds());
        Serial.println(timeClient.getDay());
      
      } //else Serial.println("relogio alinhado...");

      if(wprogram.automaticWatering) break;

      if(isDeadline){
        state = PERFORMING_WATERING_STATE;
        break;
      }

      break;

    case BROKER_CONNECTION_STATE:
     
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
      state = CHECK_CONNECTION_STATE;
      //delay(500);

      break;

    case PERFORMING_WATERING_STATE:
     // here watering will happen when conditions are met
     // this part is skipped for now
     Serial.println("realizando regagem.");
     pumpAction(wprogram.amountWater);
     state = CHECK_CONNECTION_STATE; // restart cicle
      
     break;
     
    default:
      state = CHECK_CONNECTION_STATE;
    

    
  }

  
  

  
  
}
// ***************** /LOOP *********************************

// ***************** PROGRAM FUNCTIONS ************************

// TODO: we need to cover the "multiple days" avenue
// TODO: make sure the entire program from the app gets saved

/* Since after after the first program setting we'll save it to memory 
 * we'll need to retrieve it after every new startup of the system. This 
 * function gets the json string from memory and makes it default in the 
 * system.
*/
void setWProgramFromMemory(){

  WateringProgram w;
  preferences.begin(PREFS, false);
  String jsonProgram = preferences.getString("watering_program", "");
  preferences.end();  

  char buf[100];
  jsonProgram.toCharArray(buf,100); 
  changeDefaultProgram(buf);
  
  /*StaticJsonDocument<200> doc;
  DeserializationError err =  deserializeJson(doc, jsonProgram);

  if(err){
    Serial.print(F("Deserializacao falhou: "));
    Serial.println(err.c_str());
  } else{

    w.amountWater = doc["amountWater"];
    w.gmtTimezone = doc["gmtTimezone"]; //preferences.getInt("gmtTimezone", MANAUSGMT);
    w.deadlineHour = doc["deadlineHour"]; //preferences.getInt("deadlineHour", 12);
    w.deadlineMinute = doc["deadlineMinute"];  //preferences.getInt("deadlineMinute", 0);
    w.deadlineDays = doc["deadlineDays"]; //preferences.getInt("deadlineDay", 0); // sunday
    w.automaticWatering = doc["automaticWatering"]; //preferences.getBool("automaticWatering", true);
    
  }
  
  return w;*/

}


Program getMemProgram(){

  Program n;
  
  preferences.begin(PREFS, false);
  n.deadlineHours = preferences.getInt("deadlineHours", 0);
  n.deadlineMinutes = preferences.getInt("deadlineMinutes", 0);
  n.amount = preferences.getInt("amount",0);
  preferences.end();

  return n;
  
}

void saveProgram(Program program){
  
  preferences.begin(PREFS, false);
  preferences.putInt("deadlineHours",program.deadlineHours);
  preferences.putInt("deadlineMinutes",program.deadlineMinutes);
  preferences.putInt("amount", program.amount);
  preferences.end();
  
  
}

void pumpAction(long int quantity){

  /*
   * Parting from inicial calculations assuming the ration
   * pumping/Voltage is linear and proportional we calculate
   * that under 9.0V we'll pump 49.5mL/s (test this).
   * 
   */

  // quantity is the value in mL to be pumped.
  digitalWrite(pumpPin, HIGH);
  delay((quantity/49)*1000); // rounded to 49
  digitalWrite(pumpPin, LOW);
}

// just prints the current program to the serial
void printProgram(){
    Serial.println("CURRENT PROGRAM: [");
    Serial.print("TIME UNTIL WATERING: ");
    Serial.print(currProgram.deadlineHours);
    Serial.print(".");
    Serial.println(currProgram.deadlineMinutes);
    Serial.print("AMOUNT OF WATER: ");
    Serial.println(currProgram.amount);
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
void dataCallback(char* topic, byte* payload, unsigned int length){

  char payloadStr[length + 1];
  memset(payloadStr, 0, length + 1);
  strncpy(payloadStr, (char*)payload, length);
  Serial.printf("Data    : dataCallback. Topic : [%s]\n", topic);
  Serial.printf("Data    : dataCallback. Payload : %s\n", payloadStr);

  char mqttMessage[100];  // TODO: payloadStr already does this line
  for (int i=0;i<length;i++) {
    mqttMessage[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }

  if(isPath(topic, subscribeCtrlPath)) {
    state = PERFORMING_WATERING_STATE;
    publishMsg(pingPath, "okei! Vou jogar uma aguinha"); 
  }

  if(isPath(topic, setupWateringProgram)) changeDefaultProgram(mqttMessage);

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
// the json string in memory.
void changeDefaultProgram(char message[100]){

  const size_t capacity = JSON_ARRAY_SIZE(7) + JSON_OBJECT_SIZE(6) + 100;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, message);

  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  wprogram.amountWater = doc["amount"]; 
  wprogram.gmtTimezone = doc["gmtTimezone"];
  wprogram.deadlineHour = doc["deadlineHour"]; 
  wprogram.deadlineMinute = doc["deadlineMinute"]; 
  JsonArray dataDays = doc["deadlineDays"];
  for (int i = 0; i < 7; i++){
    wprogram.deadlineDays[i] = dataDays[i]; 
  }
  wprogram.automaticWatering = doc["automaticWatering"];

  preferences.begin(PREFS, false);
  preferences.putString("watering_program", message);
  preferences.end();

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
  client.setServer(mqttServer, mqttPort);
  client.setCallback(dataCallback);

  Serial.print("Tentando conexão com o broker ");
  Serial.println(mqttServer);
  Serial.print("Usuario: ");
  Serial.println(mqttUser);
  Serial.print("Porta: ");
  Serial.println(mqttPort);
  
  while(!client.connected() && (connectionAttempts < 30)){
    
    Serial.print("tentativas ");
    Serial.println(connectionAttempts);
    
    if(client.connect("EddieHomeWS",mqttUser,mqttPassword)){
      
      Serial.println("Conectado ao Broker!");
      client.subscribe(subscribeTimingPath);
      client.subscribe(subscribeCtrlPath);
      client.subscribe(subscribeAmountPath);
      client.subscribe(publishReport);
      client.subscribe(setupWateringProgram);
      publishMsg(pingPath, "Olá! Dispositvo SmartFarm conectado!");
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
// this function checks if current hour and minute match with 
// the default watering program deadline.
bool isDeadline(){
  if ((wprogram.deadlineHour == timeClient.getHours()) && (wprogram.deadlineMinute == timeClient.getMinutes())){
    return true;
  }

  return false;
}


/**
 * Continuously checks the time to see if the deadline
 * for the next irrigation has been reached. If so, sends
 * info data to the server, gets newest data, performs
 * irrigation and sets next irrigation deadline.
 * 
 * returns void
 */
void countTime(){

  if(RTC.second()!=lastsecond && TODvalid) //We want to perform this loop on the second, each second
  {
    lastsecond=RTC.second();
    sprintf(printbuffer,"   UTC Time:%02i:%02i:%02i.%03i\n", RTC.hour(), RTC.minute(),RTC.second(),RTC.millisec());
    Serial<<printbuffer; 
  }

  // every minute check the schedule
  if(RTC.minute() != lastminute && TODvalid){
    lastminute=RTC.minute();

    // if any debounce is on, release them
    if(debounce[0] || debounce[1] || debounce[2]){

      char level[3];
      sprintf(level, "%d", waterLevel);
      publishMsg(publishLevelPath, level);

      debounce[0] = false;
      debounce[1] = false;
      debounce[2] = false;

    }
      
    
    decrementCounter();
    printProgram();
    
  }
    
  if(!TODvalid) //if we still havent got a valid TOD, hit the NIST time server 
  {
     char ssid[20];
     char pwd[20];
     network.toCharArray(ssid,20);
     password.toCharArray(pwd,20);
     if(RTC.begin(ssid,pwd))TODvalid=true; 
   }
}

/**
 * handles the decrementing of the hour and minute counters 
 * as well as what to do after they reach 0
 */
void decrementCounter(){

  int remMins = currProgram.deadlineMinutes;
  int remHours = currProgram.deadlineHours;

  
  if(remMins > 0){

    currProgram.deadlineMinutes = remMins - 1;
    
  } else {

    if(remHours > 0){
      // decrement hours and reset minutes counter
      currProgram.deadlineHours = remHours - 1;
      currProgram.deadlineMinutes = 60;
      
    }

    else {
      /*
       * if 0 hours and 0 minutes remaining we :
       * - turn the pump on
       * - reset the counter with the last saved data
       * - request the next watering time from server
       * - send status data to the server
      */

      pumpAction(currProgram.amount);
      currProgram = getMemProgram();
    
      if(publishMsg(publishReqNextSchedule, "1")){

        char level[3];
        sprintf(level, "%d", waterLevel);
        publishMsg(publishLevelPath, level);
        publishMsg(publishReport, wateredMessage);
        
      }
            
    }
    
  }
 
  
}
  

// ***************** /RTC TIME REGISTERING AND PUMP CONTROL **********




// ***************** CONNECTION STATUS ******************************
bool isConnected(){
  return WiFi.status() == WL_CONNECTED;
}

// function counts a timeout for connecting to a network 
bool connectedAfterTimeout(String net, String pwd){
  int attemptsAcc = 0; // attempts to connect
  
  while(!isConnected() && (attemptsAcc < 10)){
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

// DEPRECATED
/*
void checkConnection(String net, String pwd){  
    int attemptsAcc = 0; // attempts to connect
  
    while(!isConnected() && (attemptsAcc < 10)){
      delay(500);
      Serial.print("connectando com wifi ");
      Serial.println(net);
      attemptsAcc++;
      
    }
  
    /* the following code lights up the esp's built-in led if it manages to connect
    to a wireless network, otherwise, it'll start an interruption routine that blinks
    the led every half second, announcing that we cant connect, the blinking it assynchronous
    because if we cant connect to wifi, we'll start bluetooth service to let the user input
    data for connecting to another network*/
    /*
    if(isConnected()){
      conncted = true;

      Serial.print("Connectado com a rede ");
      Serial.println(net);
      //digitalWrite(LED_BUILTIN, HIGH); // Uncomment if we are on ESP32 DOIT 
      
      
      // if we have our timer enabled, disable it 
      if(timer != NULL){
        timerAlarmDisable(timer);
        timerDetachInterrupt(timer);
      }
      saveCredentials(net, pwd); // if connection successful, save details to memory
      
      char ssid[20];
      char pwd[20];
      network.toCharArray(ssid,20);
      password.toCharArray(pwd,20);
      if(RTC.begin(ssid,pwd))TODvalid=true;   //.begin(ssid,password) requires SSID and password of your home access point
                                               //All the action is in this one function.  It connects to access point, sends
                                               //an NTP time query, and receives the time mark. Returns TRUE on success.
      lastminute=RTC.minute();

         
    } else{
      conncted = false;
      Serial.println("Failed to connect to network");
      timer = timerBegin(0,80,true); // prescaler value
      /* below attaching timer interrupt with timer variable as first arg
      onTimer function as second ard and true for interrupt 
      generated on edge type */
      /*
      timerAttachInterrupt(timer, &onTimer, true);
      timerAlarmWrite(timer,500000, true); // generate an interrupt every 0.5 second
      timerAlarmEnable(timer); // enable interruption 
      scanNets();
      scanNets();
    }
  
}
*/

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
void scanNets(){
  
  // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    availableNets = n;
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
      
}
// ***************** /SCAN FOR NETWORKS *******************************


// **************** CONNECT TO SELECTED NETWORK ***********************
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
    //getLastSavedCredentials(); // get details from last saved network
    char net[60];
    char pass[60];
    
    if(network != "---" && password != "---"){ // if we have connection details in store
      network.toCharArray(net, 60);
      
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

// code for interruption: https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
