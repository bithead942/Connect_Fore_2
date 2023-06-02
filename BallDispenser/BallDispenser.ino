/*  Connect Fore Ball Dispenser

    This sketch is for an Arduino Uno

    Accepts command from remote computer and takes action
    Input             Description                                             Output
    <SOC>0<EOC>       Turn off lights                                         <ACK>
    <SOC>1<EOC>       Dispense Red Ball                                       <ACK>
    <SOC>2<EOC>       Red Open                                                <ACK>
    <SOC>3<EOC>       Red Close                                               <ACK>
    <SOC>4<EOC>       Dispense Blue Ball                                      <ACK>
    <SOC>5<EOC>       Blue Open                                               <ACK>
    <SOC>6<EOC>       Blue Close                                              <ACK>

    This sketch runs as a state machine so it can be interrruped at any time

    This sketch relies on the Adafruit Motor Shield V2 to drive 2 DC motors to release the balls.
    This sketch also relies on 2 strips of NeoPixels, with ?? LEDs each.

    Pin     Function
    0,1     Serial
    3       Red Strip
    4       Blue Strip
    A4      SDA (I2C)
    A5      SCL (I2C)


 ************************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MotorShield.h>

#define SOC 40  //  value = (
#define EOC 41  //  value = )
#define ACK 6
#define NAK 21
#define LED_COUNT 80  // How many NeoPixels are attached to the Arduino?
#define BRIGHTNESS 70

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *redDispenser = AFMS.getMotor(1);
Adafruit_DCMotor *blueDispenser = AFMS.getMotor(2);

Adafruit_NeoPixel redStrip(LED_COUNT, 3, NEO_GRB + NEO_KHZ800);  //Pin 3
Adafruit_NeoPixel blueStrip(LED_COUNT, 4, NEO_GRB + NEO_KHZ800); //Pin 4

#define BLUE  blueStrip.Color(0  , 0  , 255)
#define RED  redStrip.Color(255  , 0  , 0)

int Command = -1;

void setup() {
  Serial.begin(115200);
  AFMS.begin();  // create with the default frequency 1.6KHz
  blueDispenser->setSpeed(255);
  blueDispenser->run(BACKWARD);
  delay(200);
  blueDispenser->run(RELEASE);
  delay(250);
  redDispenser->setSpeed(255);
  redDispenser->run(BACKWARD);
  delay(200);
  redDispenser->run(RELEASE);
  delay(250);
  
  redStrip.begin();           // INITIALIZE NeoPixel strip object 
  redStrip.show();            // Turn OFF all pixels ASAP
  redStrip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)

  blueStrip.begin();           // INITIALIZE NeoPixel strip object 
  blueStrip.show();            // Turn OFF all pixels ASAP
  blueStrip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)

  lightCheck();

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

    case 1:   //Dispense Red Ball, Red Lights On
      growRed(10); 
      redDispense();
      flashRed(15, 100); //Flash Reps, Wait
      stripClear();
      Command = -1;
      break;

    case 2:   //Red Open
      redOpen();
      Command = -1;
      break;

    case 3:   //Red Close
      redClose();
      Command = -1;
      break;

    case 4:   //Dispense Blue Ball, Blue Lights On
      growBlue(10); 
      blueDispense();
      flashBlue(15, 100); //Flash Reps, Wait
      stripClear();
      Command = -1;  //Loop until stopped
      break;

    case 5:   //Blue Open
      blueOpen();
      Command = -1;
      break;

    case 6:   //Blue Close
      blueClose();
      Command = -1;
      break;
      
    case -1:   // Idle, do nothing
      break;

    default:
      Command = -1;
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
      if (temp >= 48 && temp <= 54) { //Valid command code, ASCII 48 = 0, ASCII 54 = 6
        Command = temp - 48;
      }
      else {
        ProtocolError = true;
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

void lightCheck() {
  stripClear();
   for (int c = 0; c < LED_COUNT; c++) {
     blueStrip.setPixelColor(c, BLUE); // Set pixel 'c' to value 'color'
     redStrip.setPixelColor(c, RED); // Set pixel 'c' to value 'color'
   }
   blueStrip.show();
   redStrip.show();
   delay(250);
   stripClear();
}

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

// Theater-marquee-style chasing lights.
void flashRed(int reps, int wait) {
  stripClear();
  for (int a = 0; a < reps; a++) { // Repeat x times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      redStrip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < LED_COUNT; c += 3) {
        redStrip.setPixelColor(c, RED); // Set pixel 'c' to value 'color'
      }
      redStrip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void growRed(int wait) {
  stripClear();
  for (int i = LED_COUNT; i >= 0; i--)
  {
    redStrip.setPixelColor(i, RED);
    redStrip.show();
    delay(wait);
  }
}

void stripClear() {
    redStrip.clear();
    redStrip.show();
    blueStrip.clear();
    blueStrip.show();
}

/***************************

    Motor Control Functions

 **************************/
void blueDispense() {
  blueDispenser->run(FORWARD);
  delay(270);
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

void redDispense() {
  redDispenser->run(FORWARD);
  delay(250);
  redDispenser->run(BACKWARD);
  delay(500);
  redDispenser->run(RELEASE);
}

void redOpen() {
  redDispenser->run(FORWARD);
  delay(500);
  redDispenser->run(RELEASE);
}

void redClose() {
  redDispenser->run(BACKWARD);
  delay(500);
  redDispenser->run(RELEASE);
}
