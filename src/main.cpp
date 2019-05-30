// ============================================================
// Project:     Athaan Clock
// ============================================================
// Author:      Habeeb Surooprajally
// Project start date: 25.05.2019
// ============================================================
// Description:
// The final result will be a highly configurable device which
// will play the islamic call to prayer at the appropriate time
// day.
// ============================================================

// Just a couple libraries //
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <stdio.h>
#include <string.h>

// Initilising tha RTC //
RTC_DS1307 RTC;

// Initilising tha Display //
U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 22, 3, 4, 5, 6, 7, 8, 9, 13, U8X8_PIN_NONE, 12, 11);

// Some cool Variables that you can access anywhere //
volatile boolean TurnDetected;
volatile boolean up;
const int pinA = 2;              // Used for generating interrupts using CLK signal
const int pinB = 24;             // Used for reading DT signal
const int PinSW = 26;            // Used for the push button switch
static long virtualPosition = 1; // without STATIC it does not count correctly!!!
static long lastVirtualPosition = 1;
const int buttonPin = 28; // the number of the pushbutton pin
const int ledPin = 13;    // the number of the LED pin
int buttonState = 0; // variable for reading the pushbutton status
int currentSelection = 0;

// Function Declarations //
int mainMenuOption(int menuPos, int virtualPos, int lastvirtualPos);

// what? you never seen an interrupt routine before? //
void isr0()
{
  up = (digitalRead(pinA) == digitalRead(pinB));
  TurnDetected = true;
}

void setup(void)
{
  pinMode(buttonPin, INPUT);
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(PinSW, INPUT);
  attachInterrupt(0, isr0, CHANGE); // interrupt 0 is always connected to pin 2 on Arduino UNO
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);
  Serial.println("Start");

  u8g2.begin();
  delay(1000);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tf);
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
  u8g2.setFont(u8g2_font_ncenB10_tf);
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
  DateTime now = RTC.now();

  while (currentSelection == 0)
  {

    if ((!(digitalRead(PinSW)) && (virtualPosition != 0)))
    {
      // This has to be here for some reason idk why dont ask me //
      if (lastVirtualPosition == 3)
      {
        currentSelection = 3;
        break;
      }
      lastVirtualPosition = virtualPosition;
      virtualPosition = 0;      // if YES, then reset counter to ZERO
      Serial.print("Reset = "); // Using the word RESET instead of COUNT here to find out a buggy encoder
      Serial.println(virtualPosition);
    }

    if (TurnDetected)
    { // do this only if rotation was detected
      if (up)
      {
        if (virtualPosition < 3)
        {
          virtualPosition++;
          lastVirtualPosition = virtualPosition;
        }
      }
      if (!up)
      {
        if (virtualPosition > 1)
        {
          virtualPosition--;
          lastVirtualPosition = virtualPosition;
        }
        TurnDetected = false; // do NOT repeat IF loop until new rotation detected
        Serial.print("Count = ");
        Serial.println(virtualPosition);
      }
    }
    int menuPosition = virtualPosition;

    switch (menuPosition)
    {
    case 1:
      menuPosition = 33;

      break;
    case 2:
      menuPosition = 43;

      break;
    case 3:
      menuPosition = 53;

      break;
    default:
      break;
    }

    currentSelection = mainMenuOption(menuPosition, virtualPosition, lastVirtualPosition);
  }

  int integerHour;
  enum
  {
    BufSize = 6
  }; // If a is short use a smaller number, eg 5 or 6
  char charHour[BufSize];

  int integerMinute;
  char charMinute[BufSize];

  int integerSecond;
  char charSecond[BufSize];
  

  switch (currentSelection)
  {
  case 1:
  {
    integerHour = now.hour();
    snprintf(charHour, BufSize, "%02d", integerHour);
    integerMinute = now.minute();
    snprintf(charMinute, BufSize, "%02d", integerMinute);
    integerSecond = now.second();
    snprintf(charSecond, BufSize, "%02d", integerSecond);

    char timeStr[15];
    strcpy (timeStr,charHour);
    strcat (timeStr,":");
    strcat (timeStr,charMinute);
    strcat (timeStr,":");
    strcat (timeStr,charSecond);
    puts (timeStr);

    u8g2.clearBuffer();

    // i basically just invented time //
    u8g2.setFont(u8g2_font_crox5hb_tn);
    u8g2.drawStr(((64 - (u8g2.getStrWidth(timeStr) / 2))-3), 35, timeStr);

    // 'the time is' pretext //
    u8g2.setFont(u8g2_font_blipfest_07_tr);
    u8g2.drawStr(((64 - (u8g2.getStrWidth("THE TIME IS:") / 2))), 15, "THE TIME IS:");

    u8g2.setFont(u8g2_font_blipfest_07_tn);
    int moveValtext = ((u8g2.getStrWidth(" 04:20")) / 2);

    u8g2.setFont(u8g2_font_baby_tf);
    
    u8g2.drawStr(((64 - (u8g2.getStrWidth("FAJR PRAYER IS AT ") / 2)) - moveValtext), 54, "FAJR PRAYER IS AT ");

    u8g2.setFont(u8g2_font_baby_tf);
    int moveValprayer = (64 - (u8g2.getStrWidth("FAJR PRAYER IS AT ") / 2) + (u8g2.getStrWidth("FAJR PRAYER IS AT ")));
    u8g2.setFont(u8g2_font_blipfest_07_tn);
    u8g2.drawStr(moveValprayer - moveValtext, 53, " 04:20");
    
    u8g2.sendBuffer();
  }
  break;
  case 2:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tf);

    u8g2.drawStr(30, 40, "ayo number 2");
    u8g2.sendBuffer();
  }
  break;
  case 3:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tf);

    u8g2.drawStr(30, 40, "ayo number tree");
    u8g2.sendBuffer();
  }
  break;
  default:
    break;
  }

  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH)
  {
    // turn LED on:
    currentSelection = 0;
    virtualPosition = 1;
  }
}


int mainMenuOption(int menuPos, int virtualPos, int lastVirtualPos)
{
  int userOption = 0;
  if (virtualPos == 0)
  {
    userOption = lastVirtualPos;

    return userOption;
  }

  int integerPosition = int(virtualPos);
  enum
  {
    BufffSize = 6
  }; // If a is short use a smaller number, eg 5 or 6
  char positionString[BufffSize];
  snprintf(positionString, BufffSize, "%d", integerPosition);
  /*
*/
  u8g2.setFontMode(1);  /* activate transparent font mode */
  u8g2.setDrawColor(1); /* color 1 for the box */

  u8g2.clearBuffer();

  // SELECTION BOX //
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, menuPos - 2, 128, 10);

  // HORIZONTAL RULE //
  u8g2.drawHLine(0, 27, 128);

  // MAIN MENU TITLE //
  u8g2.setFont(u8g2_font_victoriabold8_8u);
  u8g2.setFontPosCenter();
  u8g2.drawStr(((64 - (u8g2.getStrWidth("MAIN  MENU") / 2)) + 3), 20, "MAIN");
  u8g2.drawStr(((64 - (u8g2.getStrWidth("MAIN  MENU") / 2)) - 1), 20, "      MENU");

  // TOP LEVEL TITLE //
  u8g2.setFont(u8g2_font_blipfest_07_tr);
  u8g2.drawStr((64 - (u8g2.getStrWidth("HABEEB'S ATHAAN CLOCK") / 2)), 8, "HABEEB'S ATHAAN CLOCK");

  // MAIN MENU ICON MID //
  u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
  u8g2.drawStr(((64 - (u8g2.getStrWidth("U") / 2))), 20, "U");
  /*
  // ENCODER COUNTER (DEBUGGING) //
  u8g2.setFont(u8g2_font_blipfest_07_tr);
  u8g2.drawStr(110, 8, positionString);
  */
  // ARROW //
  u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
  u8g2.setDrawColor(2);
  u8g2.drawStr(0, (menuPos + 3), "o");

  // START OPTION //
  u8g2.setFont(u8g2_font_profont10_tf);
  u8g2.drawStr((64 - (u8g2.getStrWidth("START") / 2)), 37, "START");

  // CONFIGURE OPTION //
  u8g2.setFont(u8g2_font_profont10_tf);
  u8g2.drawStr((64 - (u8g2.getStrWidth("CONFIGURE") / 2)), 47, "CONFIGURE");

  // ABOUT OPTION //
  u8g2.setFont(u8g2_font_profont10_tf);
  u8g2.drawStr((64 - (u8g2.getStrWidth("ABOUT") / 2)), 57, "ABOUT");

  u8g2.sendBuffer();

  return userOption;
}
