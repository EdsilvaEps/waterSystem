#include <WiFi.h>
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <ArduinoJson.h> // library for parsing json
#include <PubSubClient.h>
//#include "BluetoothSerial.h"
#include <Preferences.h>
#include "TOD.h"
#include "WateringPlan.h"

#define PREFS "my-app"

Preferences preferences;

//test network: Os Silva Wi Fi
//test pass: c8906d2932
//Pauliina's net : "TW-EAV510v28483";
//Pauliina's net pwd: "58d19c6fa81fe5e6";


String network = "TW-EAV510v28483";
String password= "58d19c6fa81fe5e6";
bool conncted = false;

volatile int interruptCounter = 0;

int level1Interrupt = 14;
int level2Interrupt = 27;
int level3Interrupt = 26;

int serverLed = 25;
int countingLed = 33;
int dataLed = 32;

int pumpPin = 12;

// ******* MQTT VARIABLEs *****************
// mqtt broker currently running on Heroku
const char* mqttServer = "soldier-01.cloudmqtt.com";
const int mqttPort= 18488;
const char* mqttUser = "ehvtsbga";
const char* mqttPassword = "5n7anyeMnoLR";

const char* subscribeCtrlPath = "system/control/dispense";
const char* subscribeTimingPath = "system/api/timing";
const char* subscribeAmountPath = "system/app/amount";
const char* publishTempPath = "system/hardware/temp";
const char* publishLevelPath = "system/hardware/level";
const char* publishReqNextSchedule = "system/hardware/nextWSchedule";
const char* publishReport = "system/report";
const char* testPath = "system/ping";

const char* wateredMessage = "JUST_WATERED";
const char* lowLvMessage = "LOW_WATER";


WiFiClient espClient;
PubSubClient client(espClient);

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


/*void *debouncer(void *arguments){

  int i = 0;
  while(i < DEBOUNCE_TIME_MINS * 60){

    sleep(1);
    
  }

  debounce[0] = false;
  debounce[1] = false;
  debounce[2] = false;

  return NULL;

}*/

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
int current_interval = 0;
uint8_t deadline_hour = 0;
uint8_t deadline_minute = 0;
long int newAmount = 0;
volatile unsigned int waterLevel = 0; // modified by level interruptions
// variables to track the sensors 
bool lv1 = false;
bool lv2 = false;
bool lv3 = false;

Program currProgram;



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
    
    
    

    connectToSavedNetwork();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    currProgram = getMemProgram();
    printProgram();
    

    
}

// ***************** LOOP *********************************
void loop() {

  if(interruptCounter > 0){
    // since the ocunter is shared with ISR, we should
    // change it inside a critical section:
    portENTER_CRITICAL(&timerMux); 
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    toggleLed = !toggleLed;
    //digitalWrite(LED_BUILTIN, toggleLed);
    
    
  }

  
  if(Serial.available() > 0){
    char input = Serial.read(); 
    char inp[12]= {input};
    int in = atoi(inp); // convert from ascII to int
    getInput(in);
    
  }

  checkNetStatus();
  countTime(); // perform continous check of time
  if(!client.connected()){
    
    if(reconnectToBroker()){
      digitalWrite(serverLed, HIGH);
    }

    else digitalWrite(serverLed,LOW);
    
  } 

  if(TODvalid) digitalWrite(countingLed, HIGH);

  client.loop();
  countTime(); // perform continous check of time

  

  // making a lil testing
  //if(publishMsg(testPath, "1000")){
  //  Serial.println("test message published");
  //}
  

  
  
}
// ***************** /LOOP *********************************

// ***************** PROGRAM FUNCTIONS ************************
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

// TODO: make this better
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
    return true;
  }

  else{

    Serial.println("cliente não conectado");
    return false;
    
  }

  // todo: use threads to do this delay
  digitalWrite(dataLed, HIGH);
  delay(300);
  digitalWrite(dataLed, LOW);
  
  
}

// *********************************************************

// ***** CALLBACK FOR SUBSCRIPTIONS FROM MQTT BROKER ********

// prints messages retrieved from MQTT BROKER
// TODO: do something more useful with those messages
void callback(char* topic, byte* payload, unsigned int length) {

  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char sPayload[10]; // save the payload into a char array for simpler use
  for (int i=0;i<length;i++) {
    sPayload[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // lets do something with those messages
  if(strcmp(topic, subscribeCtrlPath) == 0){
    // if we received a control message to start pumpin'
    Serial.println("ctrl message");
    pumpAction(currProgram.amount);
    
  } 

  else if(strcmp(topic, subscribeAmountPath) == 0){
    // if we received a new amount message we might soon
    // receive a new deadline message, if we just selected
    // a new a program on the front end, save the amount message 
    Serial.println("amount message");

    //unsigned char* sAmount = (char)payload;
    char sAmount[10];
    for (int i=0;i<length;i++) {
      sAmount[i] = (char)payload[i];
      //strcat(sAmount,(char)payload[i]);
 
      
    }
    newAmount = atoi(sAmount);
    
    
  }

  else if(strcmp(topic, subscribeTimingPath) == 0){
    // if we receive a new timing message then we should 
    // reload the current watering program with the new data 
    // including the previously saved amount message 

    char sHours[3] = {"0"};
    char sMins[3] = {"0"};

    // the code below is a "technical arrangement"
    // also known in Brazil as "gambiarra". A hack im making 
    // because the messages from the server either have 3 char (h:m),
    // 4 chars (h:mm / hh:m) or they have 5 (hh:mm). 
    // And since ive made the server code 
    // im feeling quite dumb right now.

    switch(length){
      case(5):
        //standard case hh:mm
        sHours[0] = (char)payload[0];
        sHours[1] = (char)payload[1];
        sMins[0] = (char)payload[3];
        sMins[1] = (char)payload[4];
   
        break;

      case(4):
        // it's either h:mm or hh:m
        if((char)payload[1] == ':'){
          // if h:mm
          sHours[0] = (char)payload[0];
          sMins[0] = (char)payload[2];
          sMins[1] = (char)payload[3];
        
          
        } else if((char)payload[2] == ':'){
          // if hh:m
          sHours[0] = (char)payload[0];
          sHours[1] = (char)payload[1];
          sMins[0] = (char)payload[3];
          
          
        }
        break;

      case(3):
        // case h:m
        sHours[0] = (char)payload[0];
        sMins[0] = (char)payload[2];
        break;

      default: break;
    }
    
  
   

    // replace the current program if the message was valid
    if(length > 2){
      currProgram = getProgram(sHours, sMins, newAmount);
      saveProgram(currProgram);
      printProgram();  
    }
    
    
  }

  // request from the app to check status
  // send back level and temperatura (when available)
  else if(strcmp(topic, publishReport) == 0){

    char level[3];
    sprintf(level, "%d", waterLevel);
    publishMsg(publishLevelPath, level);


  }

  
  // todo: use threads to do this delay
  digitalWrite(dataLed, HIGH);
  delay(300);
  digitalWrite(dataLed, LOW);
  
  

  
}


// **********************************************************



// ************** CONNECTION TO MQTT BROKER ****************

// TODO: FINISH BUILDING THE MQTT CLIENT
// link to tutorial: https://www.arduinoecia.com.br/2019/02/enviando-mensagens-mqtt-modulo-esp32-wifi.html
bool reconnectToBroker(){
  // attempt to connection or reconnection to the broker

  int attemptsAcc = 0; // attempts to connect
  client.setServer(mqttServer, mqttPort);

  // set function to handle the callback from subscription route
  client.setCallback(callback);

  /* code will make 15 attempts to connect to MQTT broker
     (this is because we'll be using the CloudMQTT service
     of Heroku and thus, it goes to sleep when it isnt used for
     30 mins, causing slow waking. Upon successful connection with
     server, client will subscribe to the global topics for 
     timing and control.
  */
  while(!client.connected() && (attemptsAcc < 30)){
    
    Serial.print("Conectando com o broker ");
    Serial.println(mqttServer);
    
    if(client.connect("ESP32Client",mqttUser,mqttPassword)){
      
      Serial.println("Conectado ao Broker!");
      client.subscribe(subscribeTimingPath);
      client.subscribe(subscribeCtrlPath);
      client.subscribe(subscribeAmountPath);
      client.subscribe(publishReport);
      return true;
      
    } else {
      
      Serial.print("Estado cliente: ");
      Serial.println(client.state());
      return false;
       
    }
    
    
    delay(100);
    attemptsAcc++;
    
  }

  
}

// **************************************************


// ***************** RTC TIME REGISTERING AND PUMP CONTROL **********


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
void checkConnection(String net, String pwd){
    int attemptsAcc = 0; // attempts to connect
  
    while((WiFi.status() != WL_CONNECTED) && (attemptsAcc < 10)){
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
    if(WiFi.status() == WL_CONNECTED){
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
      timerAttachInterrupt(timer, &onTimer, true);
      timerAlarmWrite(timer,500000, true); // generate an interrupt every 0.5 second
      timerAlarmEnable(timer); // enable interruption 
      scanNets();
      scanNets();
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
      connectToSavedNetwork();
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

void getInput(int input){

  // here we send some report through the bt
  if(input == 9){
    sendReport(); 
  }
  
  /* if we arent connected the input is handling
  connection */
  if(WiFi.status() != WL_CONNECTED){
    if(input <= availableNets){
      
      //if the selected network is open, try connect straight away
      if(WiFi.encryptionType(input-1) == WIFI_AUTH_OPEN){
        WiFi.begin(WiFi.SSID(input-1).c_str());
        checkConnection(WiFi.SSID(input-1), ""); // check if connection attempt is successful 
      
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
        checkConnection(net, pass); // check if connection is successful 
        if(WiFi.scanComplete() > -2){ // if WiFi.scanComplete() is -2 then it hanst been triggered
          WiFi.scanDelete();
        }
             
      }
           
    }
  }
  // later on we'll add more handling to this 
  
  
  
}

// START DISCONNECT
void disconnection(){
  if(WiFi.status() == WL_CONNECTED){
    WiFi.disconnect();
    Serial.println("disconnected from network");
  } else Serial.println("device is not connected to any network");
}
// END DISCONNECT


// START CONNECTING TO MEMORY-SAVED NETWORK
void connectToSavedNetwork(){ 
    // using home connection
    //getLastSavedCredentials(); // get details from last saved network
    if(network != "---" && password != "---"){ // if we have connection details in store
      char net[60];
      network.toCharArray(net, 60);
      if(password.length() > 0){
        char pass[60];
        password.toCharArray(pass,60);
        WiFi.begin(net, pass);
      } else WiFi.begin(net);

      checkConnection(network,password);
    }
  
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

// START SENDING STATUS REPORT THROUGH SERIAL
void sendReport(){
  // in the future this report might be sent online
  Serial.print("Connected:");
  Serial.println(conncted);
  
}
// END SENDING STATUS REPORT THROUGH SERIAL

// code for interruption: https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
