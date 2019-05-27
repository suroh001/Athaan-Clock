#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

//U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 22, 3, 4, 5, 6, 7, 8, 9, 13, U8X8_PIN_NONE, 12, 11);

volatile boolean TurnDetected;
volatile boolean up;

const int pinA = 2;   // Used for generating interrupts using CLK signal
const int pinB = 24;  // Used for reading DT signal
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
  up = (digitalRead(pinA) == digitalRead(pinB));
  TurnDetected = true;
}

void setup(void)
{
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(PinSW, INPUT);
  attachInterrupt(0, isr0, CHANGE); // interrupt 0 is always connected to pin 2 on Arduino UNO
  Serial.begin(9600);
  Serial.println("Start");

  u8g2.begin();
  delay(1000);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  //Serial.println(u8g2.getStrWidth("Welcome!"));
  u8g2.drawStr((64 - (u8g2.getStrWidth("Habeeb's") / 2)), 37, "Habeeb's");
  u8g2.sendBuffer();

  delay(1000);

  for (int i = 64; i <= ((u8g2.getStrWidth("Habeeb's") / 2) + 64); i++)
  {
    u8g2.drawHLine(i, 40, 1);
    u8g2.drawHLine((128 - i), 40, 1);
    u8g2.sendBuffer();
    delay(1);
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  //Serial.println(u8g2.getStrWidth("Welcome!"));
  u8g2.drawStr((64 - (u8g2.getStrWidth("Athaan Clock") / 2)), 37, "Athaan Clock");
  u8g2.sendBuffer();

  delay(1000);

  for (int i = 64; i <= ((u8g2.getStrWidth("Athaan Clock") / 2) + 64); i++)
  {
    u8g2.drawHLine(i, 40, 1);
    u8g2.drawHLine((128 - i), 40, 1);
    u8g2.sendBuffer();
    delay(1);
  }
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

  int integerPosition = int(virtualPosition);
  enum
  {
    BufSize = 6
  }; // If a is short use a smaller number, eg 5 or 6
  char positionString[BufSize];
  snprintf(positionString, BufSize, "%d", integerPosition);

  const char *string_list =
      "Altocumulus\n"
      "Altostratus\n"
      "Cirrocumulus\n"
      "Cirrostratus\n"
      "Cirrus\n"
      "Cumulonimbus\n"
      "Cumulus\n"
      "Nimbostratus\n"
      "Stratocumulus\n"
      "Stratus";

  u8g2.clearBuffer();
  u8g2.drawHLine(0, 15, 128);
  u8g2.setFont(u8g2_font_victoriabold8_8u);
  u8g2.setFontPosCenter();
  u8g2.drawStr((64 - (u8g2.getStrWidth("MAIN MENU") / 2)), 8, "MAIN MENU");
  u8g2.drawStr(110, 8, positionString);
  u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
  u8g2.drawStr(0, 50, "o");
  u8g2.sendBuffer();
}
