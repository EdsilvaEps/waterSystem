//#include <Preferences.h>


#include "WateringPlan.h"



Program getProgram(char deadlineHours[CHAR_SIZE], char deadlineMinutes[CHAR_SIZE], long int amount){

  Program n;

  n.deadlineHours = atoi(deadlineHours);
  n.deadlineMinutes = atoi(deadlineMinutes);
  n.amount = amount;
  
  return n;
  
}


/*Program getMemProgram(char preferences[CHAR_SIZE]){

  Program n;
  
  Preferences prefs;
  prefs.begin(preferences, false);
  n.deadlineInMinutes = prefs.getInt("deadline", 0);
  n.amount = prefs.getInt("amount",0);
  prefs.end();

  return n;
  
}*/

/*void saveProgram(Program program, char preferences[CHAR_SIZE]){
  
  Preferences prefs;
  prefs.begin(preferences, false);
  prefs.putInt("deadline",n.deadlineInMinutes);
  prefs.putInt("amount", n.amount);
  prefs.end();
  
  
}*/



/*

preferences.begin("my-app",false);
  preferences.putString("network",net);
  delay(500);
  preferences.putString("password", pwd);
  delay(500);
  preferences.end();


preferences.begin("my-app",false);
  network = preferences.getString("network","---");
  password = preferences.getString("password","---");
  Serial.print("last saved net: ");
  Serial.println(network);
  Serial.print("last saved password: ");
  Serial.println(password);
  preferences.end();
*/
