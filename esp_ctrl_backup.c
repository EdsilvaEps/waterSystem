#include <WebSocketsServer.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>

#include "WiFi.h"
#include "BluetoothSerial.h"
#include <Preferences.h>

Preferences preferences;

//test network: Os Silva Wi Fi
//test pass: c8906d2932
String network = "---";
String password= "---";
bool conncted = false;

volatile int interruptCounter = 0;

int availableNets = 0;
String inData = "";


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux =  portMUX_INITIALIZER_UNLOCKED;
bool toggleLed = 0;

 // TEMPORARY FOR WEB SERVING                               
                                                            
 // set web server port number to 80                        
  WiFiServer server(80);
  WebSocketsServer webSocket = WebSocketsServer(81);                                     
 // Variable to store the HTTP request                     
 String header;                                             


// function to execute when interruption triggers
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux); // lock main thread for executing critical code
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

BluetoothSerial SerialBT;


char webpage[] PROGMEM = R"=====(
<html>
  <head>
    <script>
      var Socket;
      function init(){
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        Socket.onmessage = function(event){
          document.getElementById("rxConsole").value += event.data;
        }
      }
    </script>
  </head>
  <body onload="javascritpt:init()">
    <textaread id="rxConsole"></textarea>
  </body>
</html>

)=====";



void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    
    // enable bluetooth with device name as argument 
    SerialBT.begin("ESP32CTRL"); 

    pinMode(LED_BUILTIN, OUTPUT);

    connectToSavedNetwork();
    /*getLastSavedCredentials(); // get details from last saved network
    if(network != "---" && password != "---"){ // if we have connection details in store
      char net[60];
      network.toCharArray(net, 60);
      if(password.length() > 0){
        char pass[60];
        password.toCharArray(pass,60);
        WiFi.begin(net, pass);
      } else WiFi.begin(net);

      checkConnection(network,password);
    }*/

    
    
}

void loop() {

  if(interruptCounter > 0){
    // since the ocunter is shared with ISR, we should
    // change it inside a critical section:
    portENTER_CRITICAL(&timerMux); 
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    toggleLed = !toggleLed;
    digitalWrite(LED_BUILTIN, toggleLed);
    
    
  }

  if(Serial.available() > 0){
    char input = Serial.read(); 
    char inp[12]= {input};
    int in = atoi(inp); // convert from ascII to int
    getInput(in);
    
  }

  checkNetStatus();
  //clientAvailable();
  server.handleClient();
  webSocket.loop();
  if(Serial.available()> 0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }

}

// START OF WEB SERVER
// Print local IP address and start web server
void startServer(){
  if(conncted){
    Serial.println("IP address:");
    Serial.println(WiFi.localIP());

    server.on("/", [](){ // websocket
      server.send_P(200,"text/html", webpage)
    });
    server.begin();
    
    webSocket.begin();
  }
  
}

void clientAvailable(){
  
  if(conncted){
    WiFiClient client= server.available(); // Listen for incoming clients 

    if (client) {                            // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            //client.println("Connection: close");
            client.println();
            client.println(webpage);

            /*
            // Do something here:
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26 
            /* 
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } */
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            /*
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }*/
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }*/
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

    
 }
  
  
}
// END OF WEB SERVER

// START CHECKING CONNECTION STATUS
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
      digitalWrite(LED_BUILTIN, HIGH); 
      
      
      // if we have our timer enabled, disable it 
      if(timer != NULL){
        timerAlarmDisable(timer);
        timerDetachInterrupt(timer);
      }
      saveCredentials(net, pwd); // if connection successful, save details to memory

      startServer(); // start web server
      


         
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
// END CHECKING CONNECTION STATUS

void checkNetStatus(){
  // if we are still connected just light the builtin led
  // since not everytime the connecting routine lights it up
  if(conncted){
    
    if(WiFi.status() == WL_CONNECTED){
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      // if not, turn it off and attempt to connect to previously
      // connected network, then check connection again
      conncted = false;
      digitalWrite(LED_BUILTIN, LOW);
      connectToSavedNetwork();
    }
    
  }
  
  
}


// START SCAN FOR NETWORKS 
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
// END SCAN FOR NETWORKS

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
  
    getLastSavedCredentials(); // get details from last saved network
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
