#include "motor_control.h"
#include <Arduino.h>

// encoder definitions
#define ENCA 4 // yellow (encoder)
#define ENCB 5 // white (encoder)
#define ENCA2 36 // (yellow)
#define ENCB2 35 // (white)
#define REV_COUNT 120  

volatile int pos1 = 0;
unsigned long lastTime1 = 0;
float motorRPM1 = 0;
float actual_PWM1 = 0;
volatile int pos2 = 0;
unsigned long lastTime2 = 0;
float motorRPM2 = 0;
float actual_PWM2 = 0;
// end of encoder definitions

// start PID variables
int mtargetSpeed1 = 0;
int dir1 = 0;
//int speed_prev1 = 0;
//int dir_prev1 = 0;
float Kp1 = 1.75;
float Ki1 = 0.0;
float Kd1 = 0.01;
float pidConstant1 = 4000; 
float error1 = 0;
float previousError1 = 0;
float integral1 = 0;
float derivative1 = 0;
float controlSignal1 = 0;
unsigned long previousTime1 = 0;

int mtargetSpeed2 = 0;
int dir2 = 0;
//int speed_prev2 = 0;
//int dir_prev2 = 0;
float Kp2 = 1.75;
float Ki2 = 0.0;
float Kd2 = 0.01;
float pidConstant2 = 4000; 
float error2 = 0;
float previousError2 = 0;
float integral2 = 0;
float derivative2 = 0;
float controlSignal2 = 0;
unsigned long previousTime2 = 0;
// end PID variables

//pin naming
//motor 1
const int pwmPin = 42;  //pin controlling pwm going to H bridge
const int topIn = 40; //voltage input closest to the top of H bridge(set as either 0 or 5)
const int bottomIn = 41; //voltage input closest to bottom of H bridge(set as either 0 or 5)
//motor 2
const int pwmPin2 = 39;  //pin controlling pwm going to H bridge
const int topIn2 = 38; //voltage input closest to the top of H bridge(set as either 0 or 5)
const int bottomIn2 = 37; //voltage input closest to bottom of H bridge(set as either 0 or 5)

//pwm set up
const int pwmResolution = 12; //12 bit
float pwmFrequency = 120;    //frequency that allows motor to move
int dutyCycle = 0;   //starts as 0% cycle (off)
int dutyCycle2 = 0;   //starts as 0% cycle (off)




void setUpMotors() { 
  //set pins as outputs to go into H bridge
  pinMode(pwmPin, OUTPUT);
  pinMode(topIn, OUTPUT);
  pinMode(bottomIn, OUTPUT);
  pinMode(pwmPin2, OUTPUT);
  pinMode(topIn2, OUTPUT);
  pinMode(bottomIn2, OUTPUT);

  //set up pwm pin to send out specific dutyCycle to impact speed
  ledcAttach(pwmPin, pwmFrequency, pwmResolution);
  ledcWrite(pwmPin, mtargetSpeed1);
  //topIn and bottomIn must be opposite HIGH LOW, switching changes direction
  digitalWrite(topIn, LOW);
  digitalWrite(bottomIn, HIGH);

  // same thing for motor 2
   //set up pwm pin to send out specific dutyCycle to impact speed
  ledcAttach(pwmPin2, pwmFrequency, pwmResolution);
  ledcWrite(pwmPin2, mtargetSpeed2);
  //topIn and bottomIn must be opposite HIGH LOW, switching changes direction
  digitalWrite(topIn2, LOW);
  digitalWrite(bottomIn2, HIGH);
  
  // encoder setup
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
  lastTime1 = millis(); //initialize the time

  // encoder setup for motor 2
  pinMode(ENCA2, INPUT);
  pinMode(ENCB2, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA2),readEncoder2,RISING);
  lastTime2 = millis(); //initialize the time
}

void updateMotorControls(float targetSpeed1, float targetSpeed2, int dir1, int dir2) {
  digitalWrite(topIn, dir1);
  digitalWrite(bottomIn, !dir1);
  ledcWrite(pwmPin, targetSpeed1);
 
  digitalWrite(topIn2, dir2);
  digitalWrite(bottomIn2, !dir2);
  ledcWrite(pwmPin2, targetSpeed2);
  
  mtargetSpeed1 = targetSpeed1; 
  mtargetSpeed2 = targetSpeed2; 

  resetPID(); 
  resetPID2(); 
  

}

float stateArray[8] = {0.0};

float* motorLoop(float* inputArray) {

  //taking necessary pid values to stay consistent 
  integral1 = inputArray[0]; 
  integral2 = inputArray[1]; 
  previousError1 = inputArray[2]; 
  previousError2 = inputArray[3]; 
  mtargetSpeed1 = inputArray[4]; 
  mtargetSpeed2 = inputArray[5]; 
  dir1 = inputArray[6]; 
  dir2 = inputArray[7]; 

  unsigned long currentTime = millis();
  float timeBetween = currentTime - lastTime1;
  // calculate the RPM every 100 ms
  if (timeBetween >= 100){
    float revs = pos1/float(REV_COUNT); // convert counts to revolutions
    // calculate the RPM
    motorRPM1 = (revs/0.1) * 60.0; // revolutions per minute
    actual_PWM1 = motorRPM1 * 4095.0 / 450.0;
    actual_PWM1 = abs(actual_PWM1);
    // update vars for next calculation
    pos1 = 0;
    lastTime1 = currentTime;
    updateMotorsPID();
    // // print the RPM
    // Serial.print("Motor Speed1 (RPM):");
    // Serial.print(motorRPM1);

    // encoder loop for motor 2
  unsigned long currentTime2 = millis();
  float timeBetween2 = currentTime2 - lastTime2;
  // calculate the RPM every 100 ms
  if (timeBetween2 >= 100){
    float revs2 = pos2/float(REV_COUNT); // convert counts to revolutions
    // calculate the RPM
    motorRPM2 = (revs2/0.1) * 60.0; // revolutions per minute
    actual_PWM2 = motorRPM2 * 4095.0 / 450.0;
    actual_PWM2 = abs(actual_PWM2);
    // update vars for next calculation
    pos2 = 0;
    lastTime2 = currentTime2;
    updateMotorsPID2();
    //print the RPM
    // Serial.print("           Motor Speed2 (RPM): ");
    // Serial.println(motorRPM2);
    // delay(100); 
  }
 
}
  stateArray[0] = integral1;
  stateArray[1] = integral2; 
  stateArray[2] = previousError1; 
  stateArray[3] = previousError2; 
  stateArray[4] = mtargetSpeed1; 
  stateArray[5] = mtargetSpeed2; 
  stateArray[6] = dir1; 
  stateArray[7] = dir2; 
  return stateArray; 
}

// encoder interrupt
void readEncoder() {
  int b = digitalRead(ENCB);
  if(b>0) {
    pos1++;
  }
  else{
    pos1--;
  }
 
}

// this loop continuously calculates and writes the signal to the send to the motors based on the target speed and current speed
void updateMotorsPID(){
  //Serial.println("PID ERROR");
  // calculate time difference
  unsigned long currentTime1 = millis();
  float deltaTime1 = (currentTime1 - previousTime1)/1000.0;
  previousTime1 = currentTime1;
  //calculate error
  // Serial.print("targetSpeed1: ");
  // Serial.print(targetSpeed1);
  // Serial.println(" ");

  // Serial.print("actual_PWM1 ");
  // Serial.print(actual_PWM1);
  // Serial.println(" ");


  error1 = mtargetSpeed1 - actual_PWM1;
  // Serial.print("error1 ");
  // Serial.print(error1);
  // Serial.println(" ");

  // calculate integral
  integral1 += error1 * deltaTime1;
  // Serial.print("integral1 ");
  // Serial.print(integral1);
  // Serial.println(" ");
  //calculate derivative
  derivative1 = (error1 - previousError1)/deltaTime1;
  // Serial.print("derivative1 ");
  // Serial.print(derivative1);
  // Serial.println(" ");
  //calculate control signal (PID output)
    controlSignal1 = Kp1 * error1 + Ki1 * integral1 + Kd1 * derivative1 + pidConstant1;
    //controlSignal1 = Kp1 * error1;
  // Serial.println("unconstrained control1 ");
  // Serial.print(controlSignal1);

  controlSignal1 = constrain(controlSignal1, 0, 4095);
  // write the updated speed to motor
  if (mtargetSpeed1 == 0) { 
    ledcWrite(pwmPin, 0);
  }
  else { 
    ledcWrite(pwmPin, controlSignal1);
  }
  previousError1 = error1;
  // Serial.print("constrained ");
  // Serial.print(controlSignal1);
}

void resetPID(){
  // implement a method to reset all of the PID variables every time the website is updated (so there is a speed or direction change for either motor)
  error1 = 0;
  previousError1 = 0;
  integral1 = 0;
  derivative1 = 0;
  controlSignal1 = 0;
}


// encoder interrupt
void readEncoder2() {
  int b2 = digitalRead(ENCB2);
  if(b2>0) {
    pos2++;
  }
  else{
    pos2--;
  }
 
}

// this loop continuously calculates and writes the signal to the send to the motors based on the target speed and current speed
void updateMotorsPID2(){
  //Serial.println("PID ERROR");
  // calculate time difference
  unsigned long currentTime2 = millis();
  float deltaTime2 = (currentTime2 - previousTime2)/1000.0;
  previousTime2 = currentTime2;
  //calculate error
  // Serial.print("targetSpeed2: ");
  // Serial.print(targetSpeed2);
  // Serial.println(" ");

  // Serial.print("actual_PWM2 ");
  // Serial.print(actual_PWM2);
  // Serial.println(" ");


  error2 = mtargetSpeed2 - actual_PWM2;
  // Serial.print("error2 ");
  // Serial.print(error2);
  // Serial.println(" ");

  // calculate integral
  integral2 += error2 * deltaTime2;
  // Serial.print("integral2 ");
  // Serial.print(integral2);
  // Serial.println(" ");
  //calculate derivative
  derivative2 = (error2 - previousError2)/deltaTime2;
  // Serial.print("derivative2 ");
  // Serial.print(derivative2);
  // Serial.println(" ");
  //calculate control signal (PID output)
    controlSignal2 = Kp2 * error2 + Ki2 * integral2 + Kd2 * derivative2 + pidConstant2;
  //controlSignal2 = Kp2 * error2;
  // Serial.print("            unconstrained control2 ");
  // Serial.print(controlSignal2);

  controlSignal2 = constrain(controlSignal2, 0, 4095);
  // write the updated speed to motor
  if (mtargetSpeed2 == 0) {
    ledcWrite(pwmPin2, 0);
  }
  else {
     ledcWrite(pwmPin2, controlSignal2);
  }
 

  previousError2 = error2;
  // Serial.print("constrained ");
  // Serial.print(controlSignal2);
}

void resetPID2(){
  // implement a method to reset all of the PID variables every time the website is updated (so there is a speed or direction change for either motor)
  error2 = 0;
  previousError2 = 0;
  integral2 = 0;
  derivative2 = 0;
  controlSignal2 = 0;
}










