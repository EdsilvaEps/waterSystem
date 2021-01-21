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
String password= "f0r@c0ntr0l3";
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

// ***** SCHEDULE & WATERING VARIABLES *******

// amount of the times the esp32 will retry the update the clock
// before restarting 
const int maxAttemptsAtScheduling = 3;
int attemptsAtScheduling = 0;

volatile unsigned int waterLevel = 0; // modified by level interruptions
// variables to track the sensors 
bool lv1 = false;
bool lv2 = false;
bool lv3 = false;

WateringProgram wprogram;

int soilSensorPin = 34; // GPIO 34 (Analog ADC1_CH6) 
int drySoilThreshold = 3800;
int soilHumidity = 0;

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
    Serial.setDebugOutput(true);

    //pinMode(LED_BUILTIN, OUTPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(soilSensorPin, INPUT);

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
    wprogram = setWProgramFromMemory();

    // try to connect to the network 
    connectToSavedNetwork();
    
    // mqtt settings 
    //espClient.setCACert(ca_cert); // SSL/TLS certificate
    //client.setServer(mqttServer, mqttPort);
    //client.setCallback(dataCallback);
    //currProgram = getMemProgram();
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
      //Serial.println("check connection state");
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
      //Serial.println("select nets state");
      if(Serial.available() > 0){
        char input = Serial.read(); 
        char inp[12]= {input};
        int in = atoi(inp); // convert from ascII to int
        connectToSelectedNet(in);

        if (isConnected()) state = TIME_CHECK_STATE;
        
      }
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

      /* if automatic watering is enabed, the "verifySoilHumidty" function
         shall water the plants when above a certain dryness threshold, otherwise the 
         values of soil sensor will be simply registered on a global var.
      */
      if(wprogram.automaticWatering == true){
        //verifySoilHumidity();
        //soilHumidity = analogRead(soilSensorPin);
        //Serial.println(soilHumidity);
      } 

      else{
        
        soilHumidity = analogRead(soilSensorPin);
        //Serial.println(soilHumidity);
      }
      
      state = CHECK_CONNECTION_STATE;
      //delay(500);

      break;

    case PERFORMING_WATERING_STATE:
     // here watering will happen when conditions are met
     // this part is skipped for now
     Serial.println("realizando regagem.");
     publishMsg(pingPath, "okei! Vou jogar uma aguinha"); 
     pumpAction(wprogram.amountWater);
     state = CHECK_CONNECTION_STATE; // restart cicle
      
     break;
     
    default:
      state = CHECK_CONNECTION_STATE;
    

    
  }

  
  

  
  
}
// ***************** /LOOP *********************************

/* Simple function to verify the soil humidity, if above a certain
   threshold, meaning a dry soil, water will be pumped into the plant.
   (test the threshold values with sensor inside plant */
void verifySoilHumidity(){

  soilHumidity = analogRead(soilSensorPin);
  Serial.print("Soil humidity: ");
  Serial.println(soilHumidity);

  if(soilHumidity > drySoilThreshold){

    pumpAction(wprogram.amountWater);
    
  }

}

// ***************** PROGRAM FUNCTIONS ************************

// TODO: make sure the entire program from the app gets saved

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
  w.automaticWatering = preferences.getBool("automaticWatering", true);

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
  preferences.putBool("automaticWatering", w.automaticWatering);
  preferences.putInt("day0", w.deadlineDays[0]);
  Serial.println(w.deadlineDays[1]);
  preferences.putInt("day1", w.deadlineDays[1]);
  Serial.println(w.deadlineDays[2]);
  preferences.putInt("day2", w.deadlineDays[2]);
  Serial.println(w.deadlineDays[3]);
  preferences.putInt("day3", w.deadlineDays[3]);
  Serial.println(w.deadlineDays[4]);
  preferences.putInt("day4", w.deadlineDays[4]);
  Serial.println(w.deadlineDays[5]);
  preferences.putInt("day5", w.deadlineDays[5]);
  Serial.println(w.deadlineDays[6]);
  preferences.putInt("day6", w.deadlineDays[6]);
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
  Serial.println("irrigation finished");
  Serial.println(dt);
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
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

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
