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
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include "PrayerTimes.h"
#include <ctype.h>

// Initilising tha Display //
U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 22, 3, 4, 5, 6, 7, 8, 9, 13, U8X8_PIN_NONE, 12, 11);

// Some cool Variables that you can access anywhere //
volatile boolean TurnDetected;
volatile boolean up;
const int pinA = 2;              // Used for generating interrupts using CLK signal
const int pinB = 24;             // Used for reading DT signal
const int PinSW = 18;            // Used for the push button switch
static long virtualPosition = 1; // without STATIC it does not count correctly!!!
static long lastVirtualPosition = 1;
const int buttonPin = 28; // the number of the pushbutton pin
const int ledPin = 13;    // the number of the LED pin
int buttonState = 0;      // variable for reading the pushbutton status
int currentSelection = 0;
double times[sizeof(TimeName) / sizeof(char *)];
//boolean nextmenu = false;
volatile int clickDetect;
volatile int clickLast;
volatile int viewAltMenu = 0;
volatile int displayCount = 0;

    // Function Declarations //
    int
    mainMenuOption(int menuPos, int virtualPos, int lastvirtualPos);
void p(char *fmt, ...);
char *getNextPTimeName(double &pTime, char *pTimeName);
double &getNextPTime(double &pTime, char *pTimeName);

// Variable Function Declarations? a lickle invention of mine. //
char *pTimeName1 = getNextPTimeName(times[NULL], TimeName[NULL]);
double &pTime1 = getNextPTime(times[NULL], TimeName[NULL]);

// what? you never seen an interrupt routine before? //
void isr0()
{
  up = (digitalRead(pinA) == digitalRead(pinB));
  TurnDetected = true;
}

void setup(void)
{
  //nextmenu = false;
  viewAltMenu = 0;
  clickLast = 1;

  pinMode(buttonPin, INPUT);
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(PinSW, INPUT);
  attachInterrupt(0, isr0, CHANGE); // interrupt 0 is always connected to pin 2 on Arduino UNO
  Serial.begin(9600);

  while (!Serial)
    ;                       // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  Serial.println("Start");

  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(OneSeventh);

  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 0, times);

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
  pTimeName1 = getNextPTimeName(times[NULL], TimeName[NULL]);

  while (currentSelection == 0)
  {

    clickDetect = digitalRead(PinSW);
    Serial.println(clickDetect);

    if ((clickDetect && !clickLast) && (virtualPosition != 0))
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
    clickLast = clickDetect;

    //while (digitalRead(PinSW));

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

  // FORMATTING VARIABLES //
  int moveValtext;
  int moveValprayer;

  // ANIMATED CLOCK //
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
  integerHour = hour();
  snprintf(charHour, BufSize, "%02d", integerHour);
  integerMinute = minute();
  snprintf(charMinute, BufSize, "%02d", integerMinute);
  integerSecond = second();
  snprintf(charSecond, BufSize, "%02d", integerSecond);
  char timeStr[15];
  strcpy(timeStr, charHour);
  strcat(timeStr, ":");
  strcat(timeStr, charMinute);
  strcat(timeStr, ":");
  strcat(timeStr, charSecond);
  puts(timeStr);

  // NAME OF NEXT PRAYER //
  String upperPTimeName = String(pTimeName1);
  upperPTimeName.toUpperCase();
  char charPtime[26];
  upperPTimeName.toCharArray(charPtime, 25);
  //Serial.println(charPtime);
  char nextTimePrayerText[50];
  strcpy(nextTimePrayerText, charPtime);
  strcat(nextTimePrayerText, " PRAYER IS AT ");
  puts(nextTimePrayerText);

  // TIME OF NEXT PRAYER //
  int pTimeHour = (int)pTime1;
  int pTimeMinute = (pTime1 - pTimeHour) * 60;
  char charPtimeH[BufSize];
  char charPtimeM[BufSize];
  snprintf(charPtimeH, BufSize, "%02d", pTimeHour);
  snprintf(charPtimeM, BufSize, "%02d", pTimeMinute);
  char pTimeHourMin[20];
  strcpy(pTimeHourMin, charPtimeH);
  strcat(pTimeHourMin, ":");
  strcat(pTimeHourMin, charPtimeM);
  puts(pTimeHourMin);
  /*
  // SWITCH ROUTINE //
  if (digitalRead(PinSW) == 0)
  {

    delay(10);
    if (digitalRead(PinSW) == 0)
    {
      nextmenu = !nextmenu;
      Serial.println("Next Detent");
      Serial.println(nextmenu);
      while (digitalRead(PinSW) == 0)
        ;
    }
  }
*/

  if (viewAltMenu == 1)
  {
    currentSelection = 4;
  }

  switch (currentSelection)
  {
  case 4:
  {
    u8g2.clearBuffer();
    /*
    for (int i = 64; i <= ((u8g2.getStrWidth("display ting") / 2) + 64); i++)
    {
      u8g2.drawHLine(i, 40, 1);
      u8g2.drawHLine((128 - i), 20, 1);
      u8g2.drawHLine((128 - i), 40, 1);
      u8g2.sendBuffer();
      delay(1);
    }

    for (int i = 64; i <= ((u8g2.getStrWidth("display ting") / 2) + 64); i++)
    {
      u8g2.drawHLine(i, 40, 1);
      u8g2.drawHLine((128 - i), 20, 1);
      u8g2.drawHLine((128 - i), 40, 1);
      u8g2.sendBuffer();
      delay(1);
    }
*/
    u8g2.setFont(u8g2_font_ncenB10_tf);
    u8g2.drawStr(((64 - (u8g2.getStrWidth("display ting") / 2))), 27, "display ting");

    u8g2.sendBuffer();
  }
  break;
  case 1:
  {

    if (displayCount == 0)
      {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_blipfest_07_tr);
        u8g2.drawStr(((64 - (u8g2.getStrWidth("INTIALISING") / 2))), 34, "INTIALISING");

        for (int i = ((64 - (u8g2.getStrWidth("INTIALISING") / 2))); i <= ((64 + (u8g2.getStrWidth("INTIALISING") / 2))); i++)
        {
          u8g2.drawHLine(i, 38, 1);
          u8g2.sendBuffer();
          delay(1);
        }
      }

    u8g2.clearBuffer();

    // anime time thingy //
    u8g2.setFont(u8g2_font_crox5hb_tn);
    u8g2.drawStr(((64 - (u8g2.getStrWidth(timeStr) / 2)) - 2), 35, timeStr);

    // 'the time is' pretext //
    u8g2.setFont(u8g2_font_blipfest_07_tr);
    u8g2.drawStr(((64 - (u8g2.getStrWidth("HABEEB'S ATHAAN CLOCK") / 2))), 15, "HABEEB'S ATHAAN CLOCK");

    // time of tha prayer formatting //
    u8g2.setFont(u8g2_font_blipfest_07_tn);
    moveValtext = ((u8g2.getStrWidth(pTimeHourMin)) / 2);

    // name of tha prayer //
    u8g2.setFont(u8g2_font_baby_tf);
    u8g2.drawStr(((64 - (u8g2.getStrWidth(nextTimePrayerText) / 2)) - moveValtext), 54, nextTimePrayerText);
    //u8g2.drawStr(((64 - (u8g2.getStrWidth((upperPTime + " PRAYER IS AT ")) / 2)) - moveValtext), 54, (upperPTime + " PRAYER IS AT "));

    // time of tha prayer //
    u8g2.setFont(u8g2_font_baby_tf);
    moveValprayer = (64 - (u8g2.getStrWidth(nextTimePrayerText) / 2) + (u8g2.getStrWidth(nextTimePrayerText)));
    u8g2.setFont(u8g2_font_blipfest_07_tn);
    u8g2.drawStr((moveValprayer - moveValtext), 53, pTimeHourMin);

    u8g2.sendBuffer();
    displayCount++;
  }
  break;
  case 2:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tf);
    u8g2.drawStr(30, 40, pTimeHourMin);
    u8g2.sendBuffer();
  }
  break;
  case 3:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tf);

    u8g2.drawStr(30, 40, TimeName[1]);
    u8g2.sendBuffer();
  }
  break;
  default:
    break;
  }

  clickDetect = digitalRead(PinSW);
  if ((clickDetect && !clickLast) && (currentSelection == 1))
  {
    viewAltMenu = 1;
  }
  clickLast = clickDetect;

  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH)
  {
    displayCount = 0;
    if (viewAltMenu == 1)
    {
      viewAltMenu = 0;
      currentSelection = 1;
    }
    else if (viewAltMenu != 1)
    {
      currentSelection = 0;
      virtualPosition = 1;
    }
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

void p(char *fmt, ...)
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, 128, fmt, args);
  va_end(args);
  //Serial.print(tmp);
}

char *getNextPTimeName(double &pTime, char *pTimeName)
{
  double times[sizeof(TimeName) / sizeof(char *)];
  double currTime = hour() + minute() / 60.0;
  int i;

  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(OneSeventh);

  //get_prayer_times(year(), month(), day(), 46.9500, 7.4458, 1, times);
  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 0, times);
  for (i = 0; i < sizeof(times) / sizeof(double); i++)
  {
    if (times[i] >= currTime)
      break;
  }
  if ((times[i] - currTime) < 0)
  {
    i = 0;
  }
  pTime = times[i];
  sprintf(pTimeName, "%s", TimeName[i]);
  //Serial.println(pTimeName);
  //Serial.println( TYPE_NAME(pTimeName) );
  return pTimeName;
}

double &getNextPTime(double &pTime, char *pTimeName)
{
  double times[sizeof(TimeName) / sizeof(char *)];
  double currTime = hour() + minute() / 60.0;
  int i;

  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(OneSeventh);

  //get_prayer_times(year(), month(), day(), 46.9500, 7.4458, 1, times);
  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 0, times);
  for (i = 0; i < sizeof(times) / sizeof(double); i++)
  {
    if (times[i] >= currTime)
      break;
  }
  if ((times[i] - currTime) < 0)
  {
    i = 0;
  }
  pTime = times[i];
  sprintf(pTimeName, "%s", TimeName[i]);
  //Serial.println(pTimeName);
  return pTime;
}
