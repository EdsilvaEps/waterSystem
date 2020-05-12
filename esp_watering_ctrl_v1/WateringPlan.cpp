//#include <Preferences.h>


#include "WateringPlan.h"



Program getProgram(char deadlineHours[CHAR_SIZE], char deadlineMinutes[CHAR_SIZE], long int amount){

  Program n;

  n.deadlineHours = atoi(deadlineHours);
  n.deadlineMinutes = atoi(deadlineMinutes);
  n.amount = amount;
  
  return n;
  
}
