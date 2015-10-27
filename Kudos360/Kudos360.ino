#include <XBOXRECV.h>
#include <SPI.h>
#include <Servo.h>

USB Usb;
XBOXRECV Xbox(&Usb);

Servo leftMotor;
Servo rightMotor;

const int PIN_MOTOR_LEFT = 5;
const int PIN_MOTOR_RIGHT = 6;

const int XBOX_HAT_MAX = 32767;
const int XBOX_HAT_MIN = -32768;

bool enabled = false;

void enable();
void disable();
void disEnable();

int16_t getLeftY();
int16_t getRightX();

void setup() {
  Serial.begin(9600);
  #if !defined(__MIPSEL__)
    //while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nXbox Wireless Receiver Library Started"));
  enable();
}

void loop() {
  Usb.Task();
  if(Xbox.XboxReceiverConnected) {
    if(Xbox.Xbox360Connected[0]) {
      
      if(Xbox.getButtonClick(XBOX, 0)) {
        disEnable();
      }
      
      if(enabled) {
        arcadeDrive(map(getLeftY(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180), map(getRightX(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180));
      }
    }
  }
  Serial.println();
}

void enable() {
  if(!enabled) {
    Xbox.setLedOn(LED1, 0);
    leftMotor.attach(PIN_MOTOR_LEFT);
    rightMotor.attach(PIN_MOTOR_RIGHT);
    enabled = true;
    Serial.println("Enabled!");
  } else {
    Serial.println("Already enabled!");
  }
}

void disable() {
  if(enabled) {
    Xbox.setLedMode(ALTERNATING, 0);
    leftMotor.detach();
    rightMotor.detach();
    enabled = false;
    Serial.println("Disabled!");
  } else {
    Serial.println("Already disabled!");
  }
}

void disEnable() {
  if(enabled) {
    disable();
  } else {
    enable();
  }
}

const int DEADBAND = 12000;
int16_t getLeftY() {
  if(Xbox.getAnalogHat(LeftHatY, 0) > DEADBAND || Xbox.getAnalogHat(LeftHatY, 0) < -DEADBAND) {
    return Xbox.getAnalogHat(LeftHatY, 0);
  } else {
    return 0.0;
  }
}

int16_t getRightX() {
  if(Xbox.getAnalogHat(RightHatX, 0) > DEADBAND || Xbox.getAnalogHat(RightHatX, 0) < -DEADBAND) {
    return Xbox.getAnalogHat(RightHatX, 0);
  } else {
    return 0.0;
  }
}

//Input on a scale of 0-180
void arcadeDrive(int16_t y, int16_t x) {
  Serial.print("Y power: ");
  Serial.print(y);
  Serial.print(", X power: ");
  Serial.print(x);
  
  //Set left and right to y power
  int leftPower = y;
  int rightPower = y;
  
  //Balance x around 0 (range -90 to 90).
  x -= 90;
  leftPower -= x;
  rightPower += x;
  
  //Trim range for left
  if(leftPower > 180) {
    leftPower = 180;
  } else if(leftPower < 0) {
    leftPower = 0;
  }
  
  //Trim range for right
  if(rightPower > 180) {
    rightPower = 180;
  } else if(rightPower < 0) {
    rightPower = 0;
  }
  rightPower = 180 - rightPower;
  
  Serial.print(", Left power: ");
  Serial.print(leftPower);
  Serial.print(", Right power: ");
  Serial.print(rightPower);
  
  if(enabled) {
    leftMotor.write(leftPower);
    rightMotor.write(rightPower);
  }
}
