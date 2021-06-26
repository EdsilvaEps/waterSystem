#!/bin/sh


mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/ping &
mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/power &
mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/hardware/temp &
mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/control/dispenseWater &
mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/hardware/setupProgram &

# future topics(name change)
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/ping &
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/connected &
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/sensor/soil &
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/sensor/level &
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/control/dispenseWater &
# mosquitto_sub -h 59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud -p 8883 -u avalon -P WZ71o80U6PzG -t system/control/setupProgram &

