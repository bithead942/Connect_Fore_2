/*  Connect Fore Ball Dispenser

    This sketch is for an Arduino Uno

    Accepts command from remote computer and takes action
    Input             Description                                             Output
    <SOC>0<EOC>       Turn off lights                                         <ACK>
    <SOC>1<EOC>       Dispense Blue Ball                                      <ACK>
    <SOC>2<EOC>       Blue Open                                               <ACK>
    <SOC>3<EOC>       Blue Close                                              <ACK>
    <SOC>4<EOC>       LASER On                                                <ACK>
    <SOC>5<EOC>       LASER Off                                               <ACK>
    <SOC>6<EOC>       Launch Motors On                                        <ACK>
    <SOC>7<EOC>       Launch Motors Off                                       <ACK>
    <SOC>8?<EOC>      Aim  robot                                              <ACK>
    <SOC>9?<EOC>      Aim & Fire robot                                        <ACK>

    This sketch runs as a state machine so it can be interrruped at any time

    This sketch relies on the Adafruit Motor Shield V2 to drive 2 DC motors to release the balls.
    This sketch also relies on 2 strips of NeoPixels, with ?? LEDs each.

    Pin     Function
    0,1     Serial
    4       Blue Strip LEDs
    5       LASER
    7       Power Relay for Motor Controller
    9       Sabertooth S1 Motor
    10      Sabertooth S2 Motor
    A4      SDA (I2C)
    A5      SCL (I2C)


 ************************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>

#define SOC 40  //  value = (
#define EOC 41  //  value = )
#define ACK 6
#define NAK 21
#define LED_COUNT 80  // How many NeoPixels are attached to the Arduino?
#define BRIGHTNESS 50
#define LED_PIN 4
#define LASER_PIN 5
#define RELAY_PIN 7
#define S1_PIN 9
#define S2_PIN 10

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *blueDispenser = AFMS.getMotor(1);
Adafruit_StepperMotor *aimRobot = AFMS.getStepper(200, 2);

//Servo library used to control Sabertooth Motor controller
//Range 1000 - 2000;  1500 = stop;  1000 = full reverse;  2000 = full forward;
Servo S1;
Servo S2;

Adafruit_NeoPixel blueStrip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); 

#define BLUE  blueStrip.Color(0  , 0  , 255)

int Command = -1;
int Hole = -1;

void setup() {
  pinMode(LASER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200);
  AFMS.begin();  // create with the default frequency 1.6KHz
  blueDispenser->run(BACKWARD);
  blueDispenser->setSpeed(255);
  blueDispenser->run(RELEASE);
  delay(250);
  aimRobot->setSpeed(255);  // 0 - 255

  blueStrip.begin();           // INITIALIZE NeoPixel strip object 
  blueStrip.show();            // Turn OFF all pixels ASAP
  blueStrip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)

  digitalWrite(LASER_PIN, LOW);
  
  digitalWrite(RELAY_PIN, HIGH);  //Turn on Motor Controller (stops start-up spin)
  S1.attach(S1_PIN, 1000, 2000);
  S1.write(1500);  //stop
  S2.attach(S2_PIN, 1000, 2000);
  S2.write(1500);  //stop

  flushBuffer();
  Serial.println(char(ACK));  //Ready to receive commands
}

void loop() {
  getCommand();
  switch (Command) {
    case 0:   //Lights Off
      stripClear();
      Command = -1;
      break;

    case 1:   //Dispense Blue Ball, Blue Lights On
      growBlue(10); 
      blueDispense();
      flashBlue(15, 100); //Flash Reps, Wait
      stripClear();
      Command = -1;  //Loop until stopped
      break;

    case 2:   //Blue Open
      blueOpen();
      Command = -1;
      break;

    case 3:   //Blue Close
      blueClose();
      Command = -1;
      break;

    case 4:   // LASER ON
      digitalWrite(LASER_PIN, HIGH);
      break;

    case 5:   // LASER OFF
      digitalWrite(LASER_PIN, LOW);
      break;

    case 6:   // Launch Motors On
      launcherOn();
      break;
      
    case 7:   // Launch Motors Off
      launcherOff();
      break;

    case 8:   // Aim robot
      aim_Robot(Hole);
      growBlue(10); 
      flashBlue(15, 100); //Flash Reps, Wait
      stripClear();
      center_Robot(Hole);
      Command = -1;
      Hole = -1;
      break;
      
    case 9:   // Aim & fire robot
      digitalWrite(LASER_PIN, HIGH);
      launcherOn();  
      aim_Robot(Hole);
      growBlue(10); 
      blueDispense();
      flashBlue(15, 100); //Flash Reps, Wait
      stripClear();
      digitalWrite(LASER_PIN, LOW);
      launcherOff();
      center_Robot(Hole);
      Command = -1;
      Hole = -1;
      break;
      
    case -1:   // Idle, do nothing
      break;

    default:
      Command = -1;
      Hole = -1;
      break;
  }
  delay(10);

}

/***************************

    Serial Interface Functions

 **************************/

void getCommand() {
  uint8_t temp = 0;
  uint8_t param = 0;
  bool ProtocolError = false;

  if (Serial.available() > 0) {
    temp = Serial.read();
    if (temp == SOC) {  //Valid start of new command string
      temp = Serial.read();
      //Serial.println(temp);
      if (temp >= 48 && temp <= 57) { //Valid command code, ASCII 48 = 0, ASCII 57 = 9
        Command = temp - 48;
      }
      else {
        ProtocolError = true;
      }

      if (!ProtocolError && (Command == 8 || Command == 9))  //A second parameter is expected
      {
        temp = Serial.read();
        if (temp - 48 >= 1 && temp - 48 <= 7)
        {
          Hole = temp - 48;
        }
        else 
        {
          ProtocolError = true;
        }
      }

      temp = Serial.read();
      if (!ProtocolError && temp == EOC) { // Valid termination of command
        }
      else {
        ProtocolError = true;
      }
      if (ProtocolError) {
        Command = -1;
        Serial.println(char(NAK));
        flushBuffer();
      }
      else {
        Serial.println(char(ACK));
      }
    }
  }
}

void flushBuffer() {
  while (Serial.available() > 0) {
    uint8_t temp = Serial.read();
  }
}

/***************************

   LED Strip Functions

 **************************/

// Theater-marquee-style chasing lights.
void flashBlue(int reps, int wait) {
  stripClear();
  for (int a = 0; a < reps; a++) { // Repeat x times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      blueStrip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < LED_COUNT; c += 3) {
        blueStrip.setPixelColor(c, BLUE); // Set pixel 'c' to value 'color'
      }
      blueStrip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void growBlue(int wait) {
  stripClear();
  for (int i = LED_COUNT; i >= 0; i--)
  {
    blueStrip.setPixelColor(i, BLUE);
    blueStrip.show();
    delay(wait);
  }
}

void stripClear() {
    blueStrip.clear();
    blueStrip.show();
}

/***************************

    Motor Control Functions

 **************************/
void blueDispense() {
  blueDispenser->run(FORWARD);
  delay(250);
  blueDispenser->run(BACKWARD);
  delay(500);
  blueDispenser->run(RELEASE);
}

void blueOpen() {
  blueDispenser->run(FORWARD);
  delay(500);
  blueDispenser->run(RELEASE);
}

void blueClose() {
  blueDispenser->run(BACKWARD);
  delay(500);
  blueDispenser->run(RELEASE);
}

/***************************

    Robot Control Functions

 **************************/

void launcherOn() {
  S1.write(2000);
  S2.write(2000);
  delay(2000);  //wait for max speed
}

void launcherOff() {
  S1.write(1500);
  S2.write(1500);
}

void aim_Robot(int Hole){
     switch (Hole) {
        case 1:
          aimRobot->step(3000, FORWARD, DOUBLE); 
          aimRobot->step(1, FORWARD, SINGLE); //reduce holding current
          break;

        case 2:
          aimRobot->step(2000, FORWARD, DOUBLE); 
          aimRobot->step(1, FORWARD, SINGLE); //reduce holding current
          break;
       
        case 3:
          aimRobot->step(1000, FORWARD, DOUBLE); 
          aimRobot->step(1, FORWARD, SINGLE); //reduce holding current
          break;

        case 4:
          //do nothing, already centered
          break;

        case 5:
          aimRobot->step(1000, BACKWARD, DOUBLE); 
          aimRobot->step(1, BACKWARD, SINGLE);   //reduce holding current
          break;

        case 6:
          aimRobot->step(2000, BACKWARD, DOUBLE); 
          aimRobot->step(1, BACKWARD, SINGLE);   //reduce holding current
          break;

        case 7:
          aimRobot->step(3000, BACKWARD, DOUBLE); 
          aimRobot->step(1, BACKWARD, SINGLE);   //reduce holding current
          break;

        default:
          break;
      }
}

void center_Robot(int Hole){
     switch (Hole) {
        case 1:
          aimRobot->step(3000, BACKWARD, DOUBLE); 
          aimRobot->release();   //reduce holding current
          break;

        case 2:
          aimRobot->step(2000, BACKWARD, DOUBLE); 
          aimRobot->release(); //reduce holding current
          break;
       
        case 3:
          aimRobot->step(1000, BACKWARD, DOUBLE); 
          aimRobot->release(); //reduce holding current
          break;

        case 4:
          //do nothing, already centered
          break;

        case 5:
          aimRobot->step(1000, FORWARD, DOUBLE); 
          aimRobot->release(); //reduce holding current
          break;

        case 6:
          aimRobot->step(2000, FORWARD, DOUBLE); 
          aimRobot->release(); //reduce holding current
          break;

        case 7:
          aimRobot->step(3000, FORWARD, DOUBLE); 
          aimRobot->release(); //reduce holding current
          break;

        default:
          break;
      }
}
