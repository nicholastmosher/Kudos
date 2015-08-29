#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Servo.h>;

StaticJsonBuffer<200> jsonBuffer;
SoftwareSerial bluetoothSerial(10, 11);
Servo leftMotor;
Servo rightMotor;

const long WATCHDOG_BUFFER = 1000;

bool enabled = false;
long lastUpdate;

void enable();
void disable();
void disEnable();

void setup() {
  Serial.begin(9600);
  #if !defined(__MIPSEL__)
    //while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif

  bluetoothSerial.begin(9600);
  
  enable();
}

void loop() {

  char json[100]; //Receive from bluetooth SoftwareSerial
  char temp = -1;
  int index = 0;

  if(bluetoothSerial.available() > 0) {
    while(bluetoothSerial.available() > 0 && (index < sizeof(json) - 1)) {
      temp = bluetoothSerial.read();
      json[index] = temp;
      index++;
      json[index] = '\0';
      delay(1);
    }
  
    Serial.print("Received Data: ");
    Serial.print(json);
  
    JsonObject& root = jsonBuffer.parseObject(json);
  
    if(!root.success()) {
      Serial.print(" Parsing failed!");
    } else {
      Serial.print(" Parsing successful!");

      const char* mName = root["mName"];
      if(strcmp(mName, "KudosEnable") == 0) { //KUDOS ENABLE PACKET
        bool kEnabled = root["mData"];
        Serial.print(" KudosEnable is ");
        Serial.print(kEnabled);
        
      } else if(strcmp(mName, "KudosDrive") == 0) { //KUDOS DRIVE PACKET
        double kX = root["mData"][0];
        double kY = root["mData"][1];
        Serial.print(" KudosDrive is [");
        Serial.print(kX);
        Serial.print(", ");
        Serial.print(kY);
        Serial.print("]");
      }

      lastUpdate = millis();
    }

    //if(lastUpdate
    
    Serial.println();
  }
}

void enable() {
  if(!enabled) {
    leftMotor.attach(2);
    rightMotor.attach(3);
    enabled = true;
    Serial.println("Enabled!");
  } else {
    Serial.println("Already enabled!");
  }
}

void disable() {
  if(enabled) {
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
