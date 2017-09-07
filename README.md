Kudos
=======

![Kudos Photo](http://nicholastmosher.com/assets/Kudos/KudosFinished.jpg)

Originally Designed, built and programmed by Nick Mosher ([@nicholastmosher](https://github.com/nicholastmosher)). He wrote a long detailed [blog post](http://nicholastmosher.com/Kudos) about building it.

Kudos is a 4 wheel, XBOX controller controlled Robot. The system is build using Arduino as the main control.

Currently uses the [`Kudos360`](https://github.com/ComputerScienceHouse/Kudos/tree/master/Kudos360) code to control it.

How to Use it
-------------
- First turn on the battery pack in the clear housing. This will power the arduino and its components.
- Power up and pair the Xbox Controller. You should see the controller displaying as player one when paired.
- Flip the black reset switch on the front left corner of the frame. This powers on the motors.
- Have fun. Try not to blast any shins.


### Xbox Controller

Currently Kudos requires the use of an Xbox 360 Wireless controller. 

#### Q: Can I use a wired Xbox 360 Controller?

##### A: Not currently. In the current configuration a wired controller pulls too much power from the system due to a broken voltage regulator on the Arduino.


#### Q: Can I use an XBOX One Controller?

##### A: Not currently. We use XBOX 360 specific packages