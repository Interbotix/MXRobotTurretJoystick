 /* 
   MX-Turret Code For Joystick
  
   Connect the joystick on pins analog pins 0 and 1 of the turret.
   
   -DEADBANDHIGH and DEADBANDLOW can be adjusted to the deadband of your particular joystick if needed
   -*_LIMIT defines are hard limits for pan and tilt to avoid the turret from damagint/over torque-ing itself
   
   
   http://www.trossenrobotics.com/p/WidowX-robot-turret.aspx
   http://www.trossenrobotics.com/p/ScorpionX-RX-64-robot-turret.aspx
   http://learn.trossenrobotics.com/interbotix/robot-turrets/widowx-turret
   http://learn.trossenrobotics.com/interbotix/robot-turrets/scorpionx-turret


  ArbotiX Firmware - Commander Extended Instruction Set Example
  Copyright (c) 2008-2010 Vanadium Labs LLC.  All right reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the Vanadium Labs LLC nor the names of its 
        contributors may be used to endorse or promote products derived 
        from this software without specific prior written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL VANADIUM LABS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 
 
 
//define pan and tilt servo IDs
#define PAN    1
#define TILT   2

// the H101 'C' bracket attached to the tilt servo creates a physical limitation to how far
// we can move the tilt servo. This software limit will ensure that we don't jam the bracket into the servo.
#define TILT_UPPER_LIMIT 3300 
#define TILT_LOWER_LIMIT 796

//Upper/Lower limits for the pan servo - by defualt they are the normal 0-4095 (0-360) positions for the servo
#define PAN_UPPER_LIMIT 4095 
#define PAN_LOWER_LIMIT 0

//Default/Home position. These positions are used for both the startup position as well as the 
//position the servos will go to when they lose contact with the commander
#define DEFAULT_PAN 2048
#define DEFAULT_TILT 2048

//define analog pins that will be connected to the joystick pins
#define JOYPAN 0
#define JOYTILT 1

//generic deadband limits - not all joystics will center at 512, so these limits remove 'drift' from joysticks that are off-center.
#define DEADBANDLOW 480
#define DEADBANDHIGH 540

//Include necessary Libraries to drive the DYNAMIXEL servos  
#include <ax12.h>
#include <BioloidController.h>



/* Hardware Construct */
BioloidController bioloid = BioloidController(1000000);  //create a bioloid object at a baud of 1MBps

int pan;    //current position of the pan servo
int tilt;   //current position of the tilt servo  
int panMod =1;  //modifier for pan speed on left joystick - increase this to reduce the turret's speed
int tiltMod =1;  //modifier for tilt speed on left joystick - increase this to reduce the turret's speed
int joyPanVal = 0;//current value of the pan joystick (analog 0)
int joyTiltVal = 0;//current value of the tilt joystick (analog 1)
int joyTiltMapped =0;//tilt joystick value, mapped from 1-1023 to -500-500
int joyPanMapped =0;//pan joystick value, mapped from 1-1023 to -500-500
int speed = 50;//increase this to increase the speed of the movement


void setup(){
  
  // setup LED
  pinMode(0,OUTPUT);
  
  
  // setup interpolation, slowly raise turret to a 'home' positon. 2048 are the 'center' positions for both servos
  pan = DEFAULT_PAN;//load default pan value for startup
  tilt = DEFAULT_TILT;//load default tilt value for startup
  delay(1000);
  bioloid.poseSize = 2;//2 servos, so the pose size will be 2
  bioloid.readPose();//find where the servos are currently
  bioloid.setNextPose(PAN,pan);//prepare the PAN servo to the centered position, pan/2048
  bioloid.setNextPose(TILT,tilt);//preprare the tilt servo to the centered position, tilt/2048
  bioloid.interpolateSetup(2000);//setup for interpolation from the current position to the positions set in setNextPose, over 2000ms
  while(bioloid.interpolating > 0)  //until we have reached the positions set in setNextPose, execute the instructions in this loop
  {
    bioloid.interpolateStep();//move servos 1 'step
    delay(3);
  }
}


 
void loop(){
   //read analog values from joysticks
   joyPanVal = analogRead(JOYPAN);
   joyTiltVal = analogRead(JOYTILT);
      
   
   //deadzone for pan jotystick - only change the pan value if the joystick value is outside the deadband
   if(joyPanVal > DEADBANDHIGH || joyPanVal < DEADBANDLOW)
   {
     joyPanMapped = map(joyPanVal, 0, 1023, -speed, speed);
     pan += floor(joyPanMapped);
   }
    
    //deadzone for tilt jotystick - only change the pan value if the joystick value is outside the deadband   
   if(joyTiltVal > DEADBANDHIGH || joyTiltVal < DEADBANDLOW)
   {
     joyTiltMapped = map(joyTiltVal, 0, 1023, -speed, speed);
     tilt += floor(joyTiltMapped);
   }
     
  
      //enforce upper/lower limits for tilt servo
      if (tilt < TILT_LOWER_LIMIT)
      {
        tilt =TILT_LOWER_LIMIT;
    
      }  
    
      else if (tilt > TILT_UPPER_LIMIT)
      {
        tilt =TILT_UPPER_LIMIT;
    
      }
    
    
      //enforce upper/lower limits for pan servo
      if (pan < PAN_LOWER_LIMIT)
      {
        pan =PAN_LOWER_LIMIT;
    
      }  
    
      else if (pan > PAN_UPPER_LIMIT)
      {
        pan =PAN_UPPER_LIMIT;
    
      }
      
      //send pan and tilt goal positions to the pan/tilt servos 
      SetPosition(PAN,pan);
      SetPosition(TILT,tilt);
    
    delay(10); //delay to allow the analog-to-digital converter to settle before the next reading
  

}

