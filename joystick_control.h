#ifndef JOYSTICK_CONTROL_H
#define JOYSTICK_CONTROL_H
#include <Arduino.h>
#include "motor_control.h"

//inputs: (x,y) from html (sent by ino)
//action: calculates speed
//output: sends speed to motor control 
void runJoystick(int x, int y);  

#endif