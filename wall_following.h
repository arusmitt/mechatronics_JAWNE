#ifndef WALL_FOLLOWING_H
#define WALL_FOLLOWING_H
#include <Arduino.h>
#include "motor_control.h"
#include "Adafruit_VL53L0X.h"

void setup_wall_following(void); 
void read_TOF(void); 
void check_wall_close(void); 
void check_front_collision(void); 
void wall_following_loop(void); 

#endif



