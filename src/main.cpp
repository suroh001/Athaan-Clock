#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

//U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 22, 3, 4, 5, 6, 7, 8, 9, 13, U8X8_PIN_NONE, 12, 11);

volatile boolean TurnDetected;
volatile boolean up;

const int PinCLK = 2; // Used for generating interrupts using CLK signal
const int PinDT = 24; // Used for reading DT signal
const int PinSW = 26; // Used for the push button switch

//U8X8_PIN_NONE
/*
void setup(void) {
  u8g2.begin();
}

void loop(void) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0,24,"Hello World!");
  } while ( u8g2.nextPage() );
}
*/

void isr0()
{
  up = (digitalRead(PinCLK) == digitalRead(PinDT));
  TurnDetected = true;
}

void setup(void)
{
  pinMode(PinCLK, INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PinSW, INPUT);
  attachInterrupt (0,isr0,CHANGE); // interrupt 0 is always connected to pin 2 on Arduino UNO
  Serial.begin(9600);
  Serial.println("Start");
  u8g2.begin();
}

void loop(void)
{
  static long virtualPosition = 0; // without STATIC it does not count correctly!!!

  if (!(digitalRead(PinSW)) && (virtualPosition != 0)) 
  {                           // check if pushbutton is pressed
    virtualPosition = 0;      // if YES, then reset counter to ZERO
    Serial.print("Reset = "); // Using the word RESET instead of COUNT here to find out a buggy encoder

    Serial.println(virtualPosition);
  }

  if (TurnDetected)
  { // do this only if rotation was detected
    if (up)
      virtualPosition++;
    else
      virtualPosition--;
    TurnDetected = false; // do NOT repeat IF loop until new rotation detected
    Serial.print("Count = ");
    Serial.println(virtualPosition);
  }

  enum {BufSize=6}; // If a is short use a smaller number, eg 5 or 6 
  char positionString[BufSize];
  snprintf (positionString, BufSize, "%d", virtualPosition);
  u8g2.drawStr(33, 33, positionString);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 20, positionString);
  u8g2.drawStr(0, 50, "ay iss workin'");
  u8g2.sendBuffer();
}