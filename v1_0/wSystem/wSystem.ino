#include <RTClib.h>
#include <Wire.h>


char data = 0;
double wTime = 20; // pump for the smaller plants (20 secs ~= 340mL)
double wTime_2 = 88; // pump for the bigger plants (1.20 min ~= 1.5L)
//  the pumps have a capacity of approx 17 ml/s
int pumpPin = 13;
int pumpPin_2 = 12;

#define SUMMER_MODE 3
#define WINTER_MODE 4
#define HOUR_MODE 5
#define MINUTES_MODE 6
#define NO_MODE 8
#define SHOW_MODE 9
#define SUMMER_MOD_DAYS 2 
#define WINTER_MOD_DAYS 7
#define DAY_HOURS 24
#define HOUR_MINUTES 60
#define MINUTE_SECONDS 60

#define WINTER_MOD_SECS 604800
#define SUMMER_MOD_SECS 172800
#define HOUR_TEST 3600
#define FIVE_MIN_TEST 300

int mode = 0;
long timer = 0; // for counting seconds
long curMode = 0; // for storing current mode's seconds 
int fineTimer = 0; // for couting microseconds
int aux = 0; // for helping count microsseconds




void setup() {

  Serial.begin(9600); 
  pinMode(pumpPin, OUTPUT); 
  pinMode(pumpPin_2, OUTPUT);

}

void loop() {
  //---------------------------<Bluetooth>---------------------------------------------//
  if(Serial.available() > 0){ // send data when you receive data
      
      data = Serial.read(); // Read the incoming data and store it into variable data


      if(data == '1'){
        // pump some water
        pump(pumpPin, wTime);
        pump(pumpPin_2, wTime_2);
        Serial.println("n");
      } 
      else if(data == 't'){
        // checking how much water we're pumping
        int val = wTime;
        int val2 = wTime_2;
        Serial.println(val);
        Serial.println(val2);
      }
      else if(data == 'c'){
        // we change the duration of first pump time
        int i = 0;
        while(i == 0){
          // wait for input to be given
          if(Serial.available() > 0){
             data = Serial.read();
             i = data - '0';
             wTime = i;
             Serial.println(data);
             Serial.println(i);
            
          } 
        }  
      }

      else if(data == 'd'){
        // we change the duration of second pump time
        int i = 0;
        while(i == 0){
          // wait for input to be given
          if(Serial.available() > 0){
             data = Serial.read();
             i = data - '0';
             wTime_2 = i;
             Serial.println(data);
             Serial.println(i);
            
          } 
        }  
      }

      else if(data == '3'){
        // set for summer mode pumping (every two days)
        mode = SUMMER_MODE;
        timer = SUMMER_MOD_SECS;
        curMode = SUMMER_MOD_SECS;
        Serial.println("n");
      }

      else if(data == '4'){
        // set for winter mode pumping (weekly)
        mode = WINTER_MODE;
        timer = WINTER_MOD_SECS;
        curMode = WINTER_MOD_SECS;
        Serial.println("n");
      }

      else if(data == '5'){
        // set for hourly test
        mode = HOUR_MODE;
        timer = HOUR_TEST;
        curMode = HOUR_TEST;
        Serial.println("n");
      }

      else if(data == '6'){
        // set for five minutes test
        mode = MINUTES_MODE;
        timer = FIVE_MIN_TEST;
        curMode = FIVE_MIN_TEST;
        Serial.println("n);
      }

      else if(data == '8'){
        // set for manual mode
        mode = 0;
        timer = 0;
        curMode = 0;
        Serial.println('n');
      }

      else if(data == '9'){
        // visualize remaining time
        Serial.println(mode);
        Serial.println(timer);
        Serial.println("n");
      }



      data = '0'; // clear variable to avoid errors
     
  }
  //---------------------------</Bluetooth>---------------------------------------------//
  countTime();

}



void pump(int pin, int pumpTime){
  digitalWrite(pin, HIGH); 
  showProgress(pumpTime, pin);
  digitalWrite(pin, LOW); 
}


void showProgress(int wtime, int pin){
    int i = wtime;
    while(i > 0){
       Serial.println(i);
       i--;
       delay(1000);
    }
    
  
}


void countTime(){
    
  if(mode > 0){ // if we have some mode going on
   
    if(fineTimer < 10000){
      
      fineTimer += 4; // increase the fineTimer of 4 microsseconds (change that if too slow)
    
    } else {
      if(aux < 100){
        fineTimer = 0;
        aux += 1; // increase the auxiliar counter
      
      } else{
        
        aux = 0;
        if(timer > 0){ // decrease one second of deadline
        timer--;
        } else{
          // when deadline comes, pump some water   
          pump(pumpPin, wTime);
          pump(pumpPin_2, wTime_2);
          timer = curMode; // and restart timer
          
        }
      }      
    }
  }

}

