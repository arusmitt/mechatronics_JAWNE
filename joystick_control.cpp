#include "joystick_control.h"
#include <Arduino.h>
#include "motor_control.h"


int setdir1 = 0; 
int setdir2 = 0;
float settargetSpeed1 = 0; 
float settargetSpeed2 = 0; 
int calcSpeed = 0;
int prevX = 1000; 
int prevY = 1000; 

float jsstateArray[8] = {0.0};


void runJoystick(int x, int y) {
    if ((x != prevX) || (y != prevY)) {
          const int deadzone = 2; // Adjust this value as needed
        if (abs(x) <= deadzone && abs(y) <= deadzone) {
          settargetSpeed1 = 0;
          settargetSpeed2 = 0;
        } else {
          if (y >= 0) { // if above or equal to zero, go forward
            setdir1 = 0;
            setdir2 = 0;
          } else { // if below zero, go backwards
            setdir1 = 1;
            setdir2 = 1;
          }

          calcSpeed = map(static_cast<int>(sqrt(x*x + y*y)), 0, 100, 0, 4095);

          float steeringFactor = 1 - abs(x) / 100.0;
          if (x >= 0) {
            settargetSpeed1 = calcSpeed;
            settargetSpeed2 = static_cast<int>(calcSpeed * steeringFactor);
          } else {
            settargetSpeed2 = calcSpeed;
            settargetSpeed1 = static_cast<int>(calcSpeed * steeringFactor);
          }
          //change ARRAY
        }
        
        updateMotorControls(settargetSpeed1, settargetSpeed2, setdir1, setdir2); 
        jsstateArray[4] = settargetSpeed1; 
        jsstateArray[5] = settargetSpeed2; 
        jsstateArray[6] = setdir1; 
        jsstateArray[7] = setdir2; 
        // Serial.println(settargetSpeed1); 
        // Serial.print("                           "); 
        // Serial.print(settargetSpeed1); 
        prevX = x; 
        prevY = y; 
    } else {
        float* jsupdatedStateArray = motorLoop(jsstateArray); // Pass array and get updates
      // If needed, copy updated values back
      for (int i = 0; i < 8; i++) {
          jsstateArray[i] = jsupdatedStateArray[i];
      }
    }
    
    
    
    
   

}

