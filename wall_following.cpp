#include <Wire.h>
#include <Arduino.h>
#include "wall_following.h"
#include "Adafruit_VL53L0X.h"
#include "motor_control.h"

// defining TOF objects
Adafruit_VL53L0X sensor_1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor_2 = Adafruit_VL53L0X();

// TOF pins
#define SDA_PIN 18
#define SCL_PIN 19 
#define FRONT_TOF_PIN 14
#define RIGHT_TOF_PIN 13

//States 
#define GO_STRAIGHT 111
#define TURN_RIGHT 222
#define TURN_LEFT 333
#define INCREASE_LEFT_MOTOR 444
#define INCREASE_RIGHT_MOTOR 555
#define INCREASE_RIGHT_MOTOR_ALOT 666

int prevState = 1; 
int state = 0; 

uint32_t turn_counter = 0; 
//uint32_t straight_counter = 0; 
// variables for distance from wall 
long front_TOF_distance = 0; 
long right_TOF_distance = 0; 

// variables for chosen distances
int front_collision = 90;
int good_wall_distance = 85;
int wall_tolerance = 0;

float wfstateArray[8] = {0.0};

void setup_wall_following(void) {
  pinMode(FRONT_TOF_PIN, OUTPUT); 
  pinMode(RIGHT_TOF_PIN, OUTPUT); 
  Wire.begin(SDA_PIN, SCL_PIN); // initialize i2c on custom pins 

  // setting up TOF IP addresses 
  digitalWrite(FRONT_TOF_PIN, LOW); // reset
  digitalWrite(RIGHT_TOF_PIN, LOW); 
  delay(10); 
  digitalWrite(FRONT_TOF_PIN, HIGH); 
  digitalWrite(RIGHT_TOF_PIN, HIGH); 

  digitalWrite(RIGHT_TOF_PIN, LOW); 
  if(!sensor_1.begin(0x30)) {
    Serial.println(F("failed to boot front TOF")); 
  }

  digitalWrite(RIGHT_TOF_PIN, HIGH); 
  if(!sensor_2.begin(0x31)) {
    Serial.println(F("failed to boot right TOF")); 
  }
}

void read_TOF(void) {
  VL53L0X_RangingMeasurementData_t measure_1; 
  VL53L0X_RangingMeasurementData_t measure_2; 
  sensor_1.rangingTest(&measure_1, false); // pass in true to get debug data printout
  if (measure_1.RangeStatus != 4) { // phase failures have incorrect data 
    front_TOF_distance = measure_1.RangeMilliMeter;
    // Serial.println("front.  "); 
    // Serial.println(front_TOF_distance); 
    // Serial.println("FRONT TOF:     "); 
    // Serial.print(front_TOF_distance);
  } //else { 
    // Serial.println("front TOF out of range"); 
  //}
  sensor_2.rangingTest(&measure_2, false); //
  if (measure_2.RangeStatus != 4) { // phase failures have incorrect data 
    right_TOF_distance = measure_2.RangeMilliMeter;
    // Serial.println("right.  "); 
    // Serial.println(right_TOF_distance); 
    //   Serial.println("              Right TOF:     "); 
    // Serial.print(right_TOF_distance);
    // delay(1000); 
  } 
  //else { 
    // Serial.print("Right TOF out of range"); 
  //}
 // delay(100); 
}

void check_wall_close(void) {
  if (abs(right_TOF_distance - good_wall_distance) <= wall_tolerance) {
     // Serial.println("go straight"); 
      state = GO_STRAIGHT; 
  } 
  else if ((right_TOF_distance - good_wall_distance > 0)) {
        if ((right_TOF_distance - good_wall_distance > 200)) {
            // Serial.println("turn right"); 
             state = TURN_RIGHT; 
             
           
        } else {
          //Serial.println("increase left motor"); 
          state = INCREASE_LEFT_MOTOR;
        // delay(100); 
        }
  } 
  
  else if ((right_TOF_distance - good_wall_distance  < 0)) {
    if((right_TOF_distance - good_wall_distance) < (-40)) {
      state = INCREASE_RIGHT_MOTOR_ALOT; 
    } else {
      state = INCREASE_RIGHT_MOTOR; 
    }
  }
}

void check_front_collision(void) {
  if(front_TOF_distance < front_collision) {
   //Serial.println("turn left"); 
   state = TURN_LEFT; 
    
  }
}

void move(){
  if (state == prevState) {
    if (state == TURN_RIGHT) {
      turn_counter++;
    }
    // if (state == GO_STRAIGHT) {
    //   straight_counter++; 
    // }
    if(turn_counter == 30) {
        state == GO_STRAIGHT; 
        for (int i = 0; i < 10; i++) {
        updateMotorControls(1, 1, 0, 0);
        wfstateArray[4] = 1; 
        wfstateArray[5] = 1;  
        wfstateArray[6] = 0; 
        wfstateArray[7] = 0; 
        }
        turn_counter = 0; 
    }

    //   if(straight_counter == 30) { 
    //     for (int i = 0; i < 20; i++) {
    //     updateMotorControls(1000, 1000, 0, 0);
    //     wfstateArray[4] = 1000; 
    //     wfstateArray[5] = 1000;  
    //     wfstateArray[6] = 0; 
    //     wfstateArray[7] = 0; 
    //     }
    //     straight_counter = 0; 
    // }
     //Serial.println("NO CHANGE"); 
      //delay(5000); 
      float* updatedStateArray = motorLoop(wfstateArray); // Pass array and get updates
    // If needed, copy updated values back
    for (int i = 0; i < 8; i++) {
        wfstateArray[i] = updatedStateArray[i];
    }
  } else {
    prevState = state; 
    turn_counter = 0; 
    //straight_counter = 0; 
    if (state == GO_STRAIGHT) { 
      // Serial.print("go straight"); 
      //  delay(500); 
      //updateMotorControls(4000, 4000, 1, 1); 
         updateMotorControls(5, 5, 0, 0); 
         wfstateArray[4] = 5; 
         wfstateArray[5] = 5; 
         wfstateArray[6] = 0; 
         wfstateArray[7] = 0;
    }
    else if (state == TURN_RIGHT) {
      // Serial.print("turn right"); 
      //  delay(100); 
     // updateMotorControls(4000, 4000, 1, 1); 
        updateMotorControls(1, 1, 0, 1); 
         wfstateArray[4] = 1; 
         wfstateArray[5] = 1; 
         wfstateArray[6] = 0; 
         wfstateArray[7] = 1;
        
    }
    else if (state == TURN_LEFT) { 
      // Serial.print("turn left"); 
      //  delay(100); 
      //updateMotorControls(4000, 4000, 1, 1); 
       updateMotorControls(1, 1, 1, 0); 
       wfstateArray[4] = 1; 
       wfstateArray[5] = 1; 
       wfstateArray[6] = 1; 
       wfstateArray[7] = 0;
    }
    else if (state == INCREASE_LEFT_MOTOR) { 
      // Serial.print("increase left motor"); 
      //  delay(100); 
      //updateMotorControls(4000, 4000, 1, 1); 
          updateMotorControls(2, 1, 0, 0);  
          wfstateArray[4] = 2; 
          wfstateArray[5] = 1; 
          wfstateArray[6] = 0; 
          wfstateArray[7] = 0;
    }
    else if (state == INCREASE_RIGHT_MOTOR) { 
      // Serial.print("increase right motor");
      // delay(100); 
     // updateMotorControls(4000, 4000, 1, 1); 
       updateMotorControls(1, 2, 0, 0);
        wfstateArray[4] = 1; 
        wfstateArray[5] = 2;  
        wfstateArray[6] = 0; 
        wfstateArray[7] = 0; 
      
      
      
    } else if (state == INCREASE_RIGHT_MOTOR_ALOT) {
      updateMotorControls(0, 5, 0, 0); 
      wfstateArray[4] = 0; 
      wfstateArray[5] = 5;  
      wfstateArray[6] = 0; 
      wfstateArray[7] = 0;
    }
  }
}


// loop code
void wall_following_loop(void) {
  read_TOF(); 
  check_wall_close(); 
  check_front_collision(); 
  move();
}