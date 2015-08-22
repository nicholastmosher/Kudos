#include <XBOXRECV.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <LSM303.h>
#include "PID.h"
#include "TecBot.h"
#include "Timer.h"

const int XBOX_HAT_MAX = 32767;
const int XBOX_HAT_MIN = -32768;

const int MOTOR_LEFT_PIN = 9;
const int MOTOR_RIGHT_PIN = 10;

void enable();
void disable();
void disEnable();

int16_t getLeftYStick();
int16_t getRightXStick();

void updateY();

double compassSource();
void pidOutput(double output);

USB Usb;
XBOXRECV Xbox(&Usb);
LSM303 compass;

Servo leftMotor;
Servo rightMotor;

TecBot kudos(&leftMotor, &rightMotor);

PIDController<double> steerController(1.0, 0.0, 0.0, compassSource, pidOutput);

bool enabled = false;
double xDecayFactor = 0.99;
double xSmooth = 0.0;
double initialHeading = 0.0;

int driveMagnitude = 90;
int steerMagnitude = 90;

void setup() 
{
  Serial.begin(57600);
  Wire.begin();
  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  if (Usb.Init() == -1) 
  {
    Serial.println(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.println(F("\r\nXbox Wireless Receiver Library Started"));

  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-32767, -32767, -32767};
  compass.m_max = (LSM303::vector<int16_t>){+32767, +32767, +32767};
  Serial.println("Compass Initialized");
  compass.read();
  Serial.println("Compass read()");
  initialHeading = compassSource();
  Serial.println("Setup Compass");

  steerController.setOutputBounds(-90.0, 90.0);
  steerController.setFeedbackWrapBounds(0.0, 360.0);
  steerController.registerTimeFunction(millis);
  steerController.setTarget(initialHeading);

  Serial.println("Finished setup()");
}

void loop() 
{
  Usb.Task();
  steerController.tick();
  
  if(Xbox.XboxReceiverConnected) 
  {
    if(Xbox.Xbox360Connected[0]) 
    {
      if(Xbox.getButtonClick(XBOX, 0)) 
      {
        disEnable();
      } 
    }
  }
  updateY();

  if(enabled)
  {
    kudos.driveArcade(driveMagnitude, steerMagnitude);
  }
  else
  {
    kudos.stop();
  }

  Serial.print("Target: " + String(steerController.getTarget()));
  Serial.print("\tFeedback: " + String(steerController.getFeedback()));
  Serial.print("\tError: " + String(steerController.getError()));
  Serial.print("\tPComp: " + String(steerController.getProportionalComponent()));
  Serial.print("\tIComp: " + String(steerController.getIntegralComponent()));
  Serial.print("\tDComp: " + String(steerController.getDerivativeComponent()));
  Serial.print("\tOutput: " + String(steerController.getOutput()));
  Serial.println();
}

void enable() 
{
  if(!enabled) 
  {
    Xbox.setLedOn(LED1, 0);
    leftMotor.attach(MOTOR_LEFT_PIN);
    rightMotor.attach(MOTOR_RIGHT_PIN);
    enabled = true;
    Serial.println("Enabled!");
  } 
  else 
  {
    Serial.println("Already enabled!");
  }
}

void disable() 
{
  if(enabled) 
  {
    Xbox.setLedMode(ALTERNATING, 0);
    leftMotor.detach();
    rightMotor.detach();
    enabled = false;
    Serial.println("Disabled!");
  } 
  else 
  {
    Serial.println("Already disabled!");
  }
}

void disEnable() 
{
  if(enabled) 
  {
    disable();
  } 
  else 
  {
    enable();
  }
}

double compassSource()
{
  compass.read();
  xSmooth = (xSmooth * xDecayFactor) + ((1-xDecayFactor) * compass.heading());
  return xSmooth;
}

void pidOutput(double output)
{
  
}

void updateY()
{
  if(Xbox.XboxReceiverConnected) 
  {
    if(Xbox.Xbox360Connected[0]) 
    {
      if(enabled) 
      {
        driveMagnitude = map(getLeftYStick(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180);
      }
      else
      {
        driveMagnitude = 90;
      }
    }
  }
}

const int DEADBAND = 12000;
int16_t getLeftYStick() 
{
  if(Xbox.getAnalogHat(LeftHatY, 0) > DEADBAND || Xbox.getAnalogHat(LeftHatY, 0) < -DEADBAND) 
  {
    return Xbox.getAnalogHat(LeftHatY, 0);
  } 
  else 
  {
    return 0.0;
  }
}

int16_t getRightXStick() 
{
  if(Xbox.getAnalogHat(RightHatX, 0) > DEADBAND || Xbox.getAnalogHat(RightHatX, 0) < -DEADBAND) 
  {
    return Xbox.getAnalogHat(RightHatX, 0);
  } 
  else 
  {
    return 0.0;
  }
}
