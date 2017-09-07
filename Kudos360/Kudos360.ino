#include <XBOXRECV.h>
#include <SPI.h>
#include <Servo.h>
#include <PID.h>
#include <NewPing.h>

USB Usb;
XBOXRECV Xbox(&Usb);

Servo leftMotor;
Servo rightMotor;

const int TRIGGER_PIN = 12;
const int ECHO_PIN = 11;
const int MAX_DISTANCE = 200;

const int PIN_MOTOR_LEFT = 5;
const int PIN_MOTOR_RIGHT = 6;

const int XBOX_HAT_MAX = 32767;
const int XBOX_HAT_MIN = -32768;

int mOutput = 0;

bool enabled = false;

const int DRIVE_MODES = 3;
int driveMode = 0;

const int CONTROL_MODES = 2;
int controlMode = 0;
int controlTap = 0;

const int MAX_TARGET = 100;
const int MIN_TARGET = 20;
int target = 50;

void enable();

void disable();

void disEnable();

int pidSource();

void pidOutput(int output);

int16_t getLeftY();

int16_t getRightX();

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
PIDController<int> distanceController(4.0, 0.001, 100, pidSource, pidOutput);

void setup() {
    Serial.begin(9600);
    Serial.println("Started setup()");
    #if !defined(__MIPSEL__)
    //while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
    #endif
    if (Usb.Init() == -1) {
        Serial.print(F("\r\nOSC did not start"));
        while (1); //halt
    }
    Serial.print(F("\r\nXbox Wireless Receiver Library Started"));
    distanceController.registerTimeFunction(millis);
    distanceController.setOutputBounds(-90, 90);
    distanceController.setTarget(target);
    //enable();
    Serial.println("Finished setup()");
}

void loop() {
    Usb.Task();
    if (Xbox.XboxReceiverConnected) {
        if (Xbox.Xbox360Connected[0]) {

            if (Xbox.getButtonClick(XBOX, 0)) {
                disEnable();
            }

            if (Xbox.getButtonClick(Y, 0)) {
                cycleDriveInput();
            }

            if (Xbox.getButtonClick(X, 0)) {
                controlTap = (controlTap + 1) % 2;
                if (controlTap == 0) {
                    cycleControlMode();
                }
            }

            if (Xbox.getButtonClick(UP, 0)) {
                if (target < MAX_TARGET) {
                    target += 10;
                    distanceController.setTarget(target);
                }
            }

            if (Xbox.getButtonClick(DOWN, 0)) {
                if (target > MIN_TARGET) {
                    target -= 10;
                    distanceController.setTarget(target);
                }
            }

            if (controlMode == 0) {

                if (driveMode == 0) {
                    arcadeDrive(map(getLeftY(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180),
                                map(getRightX(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180));
                } else if (driveMode == 1) {
                    arcadeDrive(map(getLeftY(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180),
                                map(getLeftX(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180));
                } else if (driveMode == 2) {
                    arcadeDrive(map(getRightY(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180),
                                map(getRightX(), XBOX_HAT_MIN, XBOX_HAT_MAX, 0, 180));
                }

            } else if (controlMode == 1) {
                distanceController.tick();
                int rightOutput = mOutput + 90;
                int leftOutput = (-mOutput) + 90;
                if (enabled) {
                    leftMotor.write(leftOutput);
                    rightMotor.write(rightOutput);
                }
            }

        } else {
            disable();
        }
    } else {
        disable();
    }
    Serial.println();
}

void cycleDriveInput() {
    driveMode = (driveMode + 1) % DRIVE_MODES;
}

void cycleControlMode() {
    controlMode = (controlMode + 1) % CONTROL_MODES;
}

void enable() {
    if (!enabled) {
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
    if (enabled) {
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
    if (enabled) {
        disable();
    } else {
        enable();
    }
}

// Deadband can also be considered sensitivity. Setting to 1500 allows for it to ease into speeds.
// IE: Slightly touching the stick allows for slow movement
const int DEADBAND = 1500;

int16_t getLeftX() {
    if (Xbox.getAnalogHat(LeftHatX, 0) > DEADBAND || Xbox.getAnalogHat(LeftHatX, 0) < -DEADBAND) {
        return Xbox.getAnalogHat(LeftHatX, 0);
    } else {
        return 0.0;
    }
}

int16_t getLeftY() {
    if (Xbox.getAnalogHat(LeftHatY, 0) > DEADBAND || Xbox.getAnalogHat(LeftHatY, 0) < -DEADBAND) {
        return Xbox.getAnalogHat(LeftHatY, 0);
    } else {
        return 0.0;
    }
}

int16_t getRightY() {
    if (Xbox.getAnalogHat(RightHatY, 0) > DEADBAND || Xbox.getAnalogHat(RightHatY, 0) < -DEADBAND) {
        return Xbox.getAnalogHat(RightHatY, 0);
    } else {
        return 0.0;
    }
}

int16_t getRightX() {
    if (Xbox.getAnalogHat(RightHatX, 0) > DEADBAND || Xbox.getAnalogHat(RightHatX, 0) < -DEADBAND) {
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
    if (leftPower > 180) {
        leftPower = 180;
    } else if (leftPower < 0) {
        leftPower = 0;
    }

    //Trim range for right
    if (rightPower > 180) {
        rightPower = 180;
    } else if (rightPower < 0) {
        rightPower = 0;
    }
    rightPower = 180 - rightPower;

    Serial.print(", Left power: ");
    Serial.print(leftPower);
    Serial.print(", Right power: ");
    Serial.print(rightPower);

    if (enabled) {
        leftMotor.write(leftPower);
        rightMotor.write(rightPower);
    }
}

int pidSource() {
    return sonar.ping_cm();
}

void pidOutput(int output) {
    mOutput = output;
}

