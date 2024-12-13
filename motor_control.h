#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
#include <Arduino.h>

//sets up motor pins 
void setUpMotors(); 

//inputs: gets left motor speed, right motor speed, and direction from joystick, wall following, or autonomous
//action: controls the motors (gives speed and PID)
//output: nothing 
void updateMotorControls(float targetSpeed1, float targetSpeed2, int direction1, int direction2); 

float* motorLoop(float* inputArray);

void readEncoder();

void updateMotorsPID();

void resetPID();

void readEncoder2();

void updateMotorsPID2();

void resetPID2();



// // declare functions here in this format: void name(input args & type);
// void setup_motors(uint8_t pwm_pin, uint8_t dir1_pin, uint8_t dir2_pin); 
// void setup_encoders(uint8_t encA_pin, uint8_t encB_pin); 
// void run_motors(void); 
// void update_motors_pid(void);  
// void reset_pid(void); 
// void read_encoder(uint8_t encB_pin, uint8_t motor_number); // encoder interrupt 



#endif