/*  Connect Fore Serial Test

    Accepts command from remote computer and takes action
    Input             Description                                             Output
    <SOC>0<EOC>       Turn off lights                                         <ACK>
    <SOC>1<EOC>       Start new game (turn on white lights)                   <ACK>
    <SOC>214<EOC>     Blue wins, winning rows 1-4 (horizontal or diag win)    <ACK>
    <SOC>331<EOC>     Red wins, winning rows 3 (vertical win)                 <ACK>
    <SOC>4<EOC>       Request Game Board                                      (7x6 matrix in JSON format)
    <SOC>5<EOC>       Rumble Motor On                                         <ACK>
    <SOC>6<EOC>       Rumble Motor Off                                        <ACK>

    This sketch runs as a state machine so it can be interrruped at any time

    This sketch relies on a gameboard with 42 TCS34725 colour sensors arranged in 7 columns of 6 each
    This sketch also relies on 7 strips of NeoPixels, with 15 LEDs each.  DIO pins 3-9 control these 7 NeoPixel strips

 ************************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <avr/pgmspace.h>

#define SOC 40
#define EOC 41
#define ACK 6
#define NAK 21
#define RUMBLE_PIN 12

#define LED_COUNT 15  // How many NeoPixels are attached to the Arduino?
#define STRIP_COUNT 7

#define BLUE  strip[0].Color(0  , 0  , 255)
#define RED  strip[0].Color(255  , 0  , 0)
#define WHITE  strip[0].Color(120 , 120 , 120)

#define TCAADDR1 0x70
#define TCAADDR2 0x71
#define TCAADDR3 0x72
#define TCAADDR4 0x73
#define TCAADDR5 0x74
#define TCAADDR6 0x75
#define TCAADDR7 0x76
#define NUMCOLS 7
#define NUMROWS 6

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip1(LED_COUNT, 3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(LED_COUNT, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4(LED_COUNT, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip5(LED_COUNT, 7, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip6(LED_COUNT, 8, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip7(LED_COUNT, 9, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strip[STRIP_COUNT] {
  strip1,
  strip2,
  strip3,
  strip4,
  strip5,
  strip6,
  strip7
};

//Column 1 Sensors
Adafruit_TCS34725 tcs00 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs01 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs02 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs03 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs04 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs05 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 2 Sensors
Adafruit_TCS34725 tcs06 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs07 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs08 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs09 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs10 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs11 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 3 Sensors
Adafruit_TCS34725 tcs12 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs13 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs14 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs15 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs16 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs17 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 4 Sensors
Adafruit_TCS34725 tcs18 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs19 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs20 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs21 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs22 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs23 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 5 Sensors
Adafruit_TCS34725 tcs24 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs25 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs26 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs27 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs28 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs29 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 6 Sensors
Adafruit_TCS34725 tcs30 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs31 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs32 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs33 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs34 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs35 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

//Column 7 Sensors
Adafruit_TCS34725 tcs36 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs37 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs38 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs39 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs40 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs41 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

Adafruit_TCS34725 ColumnSensor[NUMCOLS][NUMROWS] = {
  {tcs00, tcs01, tcs02, tcs03, tcs04, tcs05},
  {tcs06, tcs07, tcs08, tcs09, tcs10, tcs11},
  {tcs12, tcs13, tcs14, tcs15, tcs16, tcs17},
  {tcs18, tcs19, tcs20, tcs21, tcs22, tcs23},
  {tcs24, tcs25, tcs26, tcs27, tcs28, tcs29},
  {tcs30, tcs31, tcs32, tcs33, tcs34, tcs35},
  {tcs36, tcs37, tcs38, tcs39, tcs40, tcs41},
};

char ColumnVal[NUMCOLS][NUMROWS];
unsigned char Multiplex[NUMCOLS] = { TCAADDR1, TCAADDR2, TCAADDR3, TCAADDR4, TCAADDR5, TCAADDR6, TCAADDR7 };

int Command = -1;
uint8_t StartCol = 0;
uint8_t NumCols = 0;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  //Startup Column 1 Sensors
  for (int i = 0; i <= NUMCOLS - 1; i++) {
    for (int j = 0; j <= NUMROWS - 1; j++) {
      tcaselect(Multiplex[i], j);
      if (ColumnSensor[i][j].begin(41)) {
        //Serial.println("Found sensor in Column " + String(i) + ": " + String(j));
      }
      else {
        Serial.print(F("Can't find sensor in Column "));
        Serial.println(String(i) + ": " + String(j));
        while (1); // halt!
      }
    }
  }

  flushBuffer();
  Serial.println(char(ACK));  //Ready to receive commands

  pinMode(RUMBLE_PIN, OUTPUT);
  digitalWrite(RUMBLE_PIN, LOW);  //Turn off Rumble

  for (int i = 0; i < STRIP_COUNT; i++) {
    strip[i].begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip[i].show();            // Turn OFF all pixels ASAP
    strip[i].setBrightness(130); // Set BRIGHTNESS (max = 255),  170 = overcurrent
  }

  initializeVals();
}

void loop() {
  getCommand();
  switch (Command) {
    case 0:   //Lights Off
      stripClear();
      initializeVals();
      Command = -1;
      break;

    case 1:   //New Game
      colorWipeAll(WHITE, 10);
      Command = -1;
      break;

    case 2:   //Blue Wins
      winner(BLUE, 10, 10); //Flash Color, Reps, Wait
      stripClear();
      colorWipe(BLUE, StartCol, NumCols, 15);  // Highlight winning cols
      delay(2000);
      //Command = -1;  //Loop until stopped
      break;

    case 3:   //Red Wins
      winner(RED, 10, 10); //Flash color, Reps, Wait
      stripClear();
      colorWipe(RED, StartCol, NumCols, 15);  // Highlight winning cols
      delay(2000);
      //Command = -1;  //Loop until stopped
      break;

    case 4:   //Get Board
      getColours();
      Command = -1;
      break;

    case 5:  //Rumble On
      digitalWrite(RUMBLE_PIN, HIGH);  //Turn on Rumble
      break;

    case 6:  //Rumble Off
      digitalWrite(RUMBLE_PIN, LOW);  //Turn off Rumble
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
      if (temp >= 48 && temp <= 54) { //Valid command code, ASCII 48 = 0
        Command = temp - 48;
      }
      else {
        ProtocolError = true;
      }
      if (!ProtocolError && (temp == 50 || temp == 51)) {  //Additional parameters expected for command 2 and 3
        param = Serial.read();
        //Serial.println(param);
        if (param >= 49 && param <= 55) {  //Valid first param (1-7)
          StartCol = param - 49;
          param = Serial.read();
          if (param == 49 || param == 52) {  //valid second param (1 or 4)
            NumCols = param - 49;
          }
          else {
            ProtocolError = true;  // Bad second param
          }
        }
        else {
          ProtocolError = true;  //  Bad first param
        }

        temp = Serial.read();
        if (!ProtocolError && temp == EOC) { // Valid termination of command
        }
        else {
          ProtocolError = true;
        }
      }
      if (ProtocolError) {
        Command = -1;
        StartCol = 0;
        NumCols = 0;
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

void colorWipeAll(uint32_t color, int wait) {
  stripClear();
  for (int i = 0; i < LED_COUNT; i++) { // For each pixel in strip...
    for (int j = 0; j < STRIP_COUNT; j++) {
      strip[j].setPixelColor(i, color);         //  Set pixel's color (in RAM)
      strip[j].show();                          //  Update strip to match
      delay(wait);                           //  Pause for a moment
    }
  }
}

void colorWipe(uint32_t color, uint8_t startCol, uint8_t numCols, int wait) {
  stripClear();
  for (int i = 0; i < LED_COUNT; i++) { // For each pixel in strip...
    for (int j = startCol; j < startCol + numCols + 1; j++) {
      strip[j].setPixelColor(i, color);         //  Set pixel's color (in RAM)
      strip[j].show();                          //  Update strip to match
      delay(wait);                           //  Pause for a moment
    }
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void winner(uint32_t color, int reps, int wait) {
  stripClear();
  for (int a = 0; a < reps; a++) { // Repeat x times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      for (int x = 0; x < STRIP_COUNT; x++) {  // For each strip
        strip[x].clear();         //   Set all pixels in RAM to 0 (off)
        // 'c' counts up from 'b' to end of strip in steps of 3...
        for (int c = b; c < LED_COUNT; c += 3) {
          strip[x].setPixelColor(c, color); // Set pixel 'c' to value 'color'
        }
        strip[x].show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
      }
    }
  }
}

void stripClear() {
  for (int i = 0; i < STRIP_COUNT; i++) {
    strip[i].clear();
    strip[i].show();
  }
}

/***************************

   Colour Sensor Functions

 **************************/

void getColours() {
  float red, green, blue;

  for (int i = 0; i <= NUMCOLS - 1 ; i++) {
    for (int j = 0; j <= NUMROWS - 1; j++) {
      tcaselect(Multiplex[i], j);
      ColumnSensor[i][j].getRGB(&red, &green, &blue);

      if (red >= 147 && red <= 197 && green >= 22 && green <= 62 && blue >= 32 && blue <= 72)
      {
        ColumnVal[i][j] = 'R';  //Red Golf Ball
      }
      else if (red >= 16 && red <= 56 && green >= 58 && green <= 98 && blue >= 105 && blue <= 155)
      {
        ColumnVal[i][j] = 'B';  //Blue Golf Ball
      }
      else
      {
        ColumnVal[i][j] = '-';
      }
    }
  }
  printVals2();
}

void tcaselect(unsigned char TCAADDR, uint8_t i) {
  if (i > NUMCOLS) return;

  for (int j = 0; j <= NUMCOLS - 1; j++) {   //turn off all multiplexers
    Wire.beginTransmission(Multiplex[j]);
    Wire.write(0);
    Wire.endTransmission();
  }

  /*if (i = 0)
  {
    Wire.beginTransmission(Multiplex[NUMCOLS - 1]);
    Wire.write(0);
    Wire.endTransmission();
  }
  else
  {
    Wire.beginTransmission(Multiplex[i - 1]);
    Wire.write(0);
    Wire.endTransmission();
  }*/

  Wire.beginTransmission(TCAADDR);  //Turn on selected multiplexer
  Wire.write(1 << i);
  Wire.endTransmission();
}

void printVals2() {

  Serial.print(F("{ \n  \"GameBoard\": { \n   \"rows\": [\n"));
  for (int i = NUMROWS - 1; i >= 0; i--) {
    Serial.print(F("  ["));
    for (int j = 0; j <= NUMCOLS - 1; j++) {
      Serial.print(F(" \""));
      Serial.print(ColumnVal[j][i]);
      Serial.print(F("\" "));
      if (j < NUMCOLS - 1) Serial.print(F(","));
    }
    Serial.print(" ]");
    if (i > 0) Serial.print(F(",\n"));
    else Serial.print(F("\n"));
  }
  Serial.println(F("  ]\n } \n }"));
}

void initializeVals() {
  for (int i = 0; i <= NUMCOLS - 1; i++) {
    for (int j = 0; j <= NUMROWS - 1; j++) {
      ColumnVal[i][j] = '-';
    }
  }
}
