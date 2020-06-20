

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>


#ifndef WATERING_PLAN
#define WATERING_PLAN

#define CHAR_SIZE 3

/* expected json format for WateringProgram
 * {
  "amountWater": 1000,
  "gmtTimezone": -4,
  "deadlineHour": 12,
  "deadlineMinute": 0,
  "deadlineDays": [
    0,
    2,
    4,
    -1,
    -1,
    -1,
    -1
  ],
  "automaticWatering": false
}
*/

// structure representing a Program object  
// it contains the dealine until next watering in minutes 
// and the amount of water in mililiters (to be converted in time
// in which the pump motor will keep on).
typedef struct{
  int deadlineHours;
  int deadlineMinutes;
  long int amount;
  
} Program;

typedef struct{
  long int amountWater;
  int gmtTimezone;
  int deadlineHour;
  int deadlineMinute;
  int deadlineDays[7];
  bool automaticWatering;
} WateringProgram;

// takes strings for deadline and amount of water and returns a program object 
Program getProgram(char deadlineHours[CHAR_SIZE], char deadlineMinutes[CHAR_SIZE], long int amount);

// retrieves the lastly used program from memory
//Program getMemProgram(char preferences[CHAR_SIZE]);

// saves the current program in memory, in case power issues happen
//void saveProgram(Program program, char preferences[CHAR_SIZE]);



#endif

#ifdef __cplusplus
}
#endif
