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
int buttonStateLast = 1;
int currentSelection = 0;
double times[sizeof(TimeName) / sizeof(char *)];
//boolean nextmenu = false;
volatile int clickDetect;
volatile int clickLast;
volatile int viewAltMenu = 0;
volatile int displayCount = 0;
volatile int displayCount2 = 0;
char calcMethod;
char AsrMethod;
char HighLatAdjust;

// Function Declarations //
int mainMenuOption(int menuPos, int virtualPos, int lastvirtualPos);
void p(char *fmt, ...);
void softReset();
char *getNextPTimeName(/*double &pTime,*/ char *pTimeName);
double &getNextPTime(double &pTime, char *pTimeName);

// Variable Function Declarations? a little invention of mine. //
char *pTimeName1 = getNextPTimeName(/*double &pTime,*/ TimeName[NULL]);
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

  //set_calc_method(ISNA);
  //set_asr_method(Shafii);
  //set_high_lats_adjust_method(OneSeventh);

  //get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 1, times);

  u8g2.begin();
  /*
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
  */

  char calcMethod = "ISNA";
  char AsrMethod = "Shafii";
  char HighLatAdjust = "OneSeventh";
}

void loop(void)
{
  set_calc_method(calcMethod);
  set_asr_method(AsrMethod);
  set_high_lats_adjust_method(HighLatAdjust);

  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 1, times);

  pTimeName1 = getNextPTimeName(/*double &pTime,*/ TimeName[NULL]);

  while (currentSelection == 0)
  {

    clickDetect = digitalRead(PinSW);
    //Serial.println(clickDetect);

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

  // Let's figure out when the next prayer is //
  double currTime = hour() + minute() / 60.0;
  int h;

  for (h = 0; h < sizeof(times) / sizeof(double); h++)
  {
    if (times[h] >= currTime)
      break;
  }
  if ((times[h] - currTime) < 0)
  {
    h = 0;
    //pTime = times[i];
  }

  // NAME OF NEXT PRAYER //
  String upperPTimeName = String(TimeName[h]);
  upperPTimeName.toUpperCase();
  char charPtime[10];
  upperPTimeName.toCharArray(charPtime, 20);
  //Serial.println(charPtime);
  char nextTimePrayerText[20];
  strcpy(nextTimePrayerText, charPtime);
  strcat(nextTimePrayerText, " IS AT ");
  puts(nextTimePrayerText);

  // TIME OF NEXT PRAYER //
  int pTimeHour = (int)times[h];
  int pTimeMinute = (times[h] - pTimeHour) * 60;
  char charPtimeH[BufSize];
  char charPtimeM[BufSize];

  snprintf(charPtimeH, BufSize, "%02d", pTimeHour);
  snprintf(charPtimeM, BufSize, "%02d", pTimeMinute);
  char pTimeHourMin[20];
  strcpy(pTimeHourMin, charPtimeH);
  strcat(pTimeHourMin, ":");
  strcat(pTimeHourMin, charPtimeM);
  puts(pTimeHourMin);

  // FAJR //
  int fajrTimeHour = (int)times[0];
  int fajrTimeMinute = (times[0] - fajrTimeHour) * 60;
  char fajrtimeH[BufSize];
  char fajrtimeM[BufSize];
  snprintf(fajrtimeH, BufSize, "%02d", fajrTimeHour);
  snprintf(fajrtimeM, BufSize, "%02d", fajrTimeMinute);
  char fajrTimeHourMin[12];
  strcpy(fajrTimeHourMin, fajrtimeH);
  strcat(fajrTimeHourMin, ":");
  strcat(fajrTimeHourMin, fajrtimeM);
  puts(fajrTimeHourMin);

  // DHUHR //
  int dhuhrTimeHour = (int)times[2];
  int dhuhrTimeMinute = (times[2] - dhuhrTimeHour) * 60;
  char dhuhrtimeH[BufSize];
  char dhuhrtimeM[BufSize];
  snprintf(dhuhrtimeH, BufSize, "%02d", dhuhrTimeHour);
  snprintf(dhuhrtimeM, BufSize, "%02d", dhuhrTimeMinute);
  char dhuhrTimeHourMin[12];
  strcpy(dhuhrTimeHourMin, dhuhrtimeH);
  strcat(dhuhrTimeHourMin, ":");
  strcat(dhuhrTimeHourMin, dhuhrtimeM);
  puts(dhuhrTimeHourMin);

  // ASR //
  int asrTimeHour = (int)times[3];
  int asrTimeMinute = (times[3] - asrTimeHour) * 60;
  char asrtimeH[BufSize];
  char asrtimeM[BufSize];
  snprintf(asrtimeH, BufSize, "%02d", asrTimeHour);
  snprintf(asrtimeM, BufSize, "%02d", asrTimeMinute);
  char asrTimeHourMin[12];
  strcpy(asrTimeHourMin, asrtimeH);
  strcat(asrTimeHourMin, ":");
  strcat(asrTimeHourMin, asrtimeM);
  puts(asrTimeHourMin);

  // MAGHRIB //
  int maghribTimeHour = (int)times[4];
  int maghribTimeMinute = (times[4] - maghribTimeHour) * 60;
  char maghribtimeH[BufSize];
  char maghribtimeM[BufSize];
  snprintf(maghribtimeH, BufSize, "%02d", maghribTimeHour);
  snprintf(maghribtimeM, BufSize, "%02d", maghribTimeMinute);
  char maghribTimeHourMin[12];
  strcpy(maghribTimeHourMin, maghribtimeH);
  strcat(maghribTimeHourMin, ":");
  strcat(maghribTimeHourMin, maghribtimeM);
  puts(maghribTimeHourMin);

  // ISHA //
  int ishaTimeHour = (int)times[6];
  int ishaTimeMinute = (times[6] - ishaTimeHour) * 60;
  char ishatimeH[BufSize];
  char ishatimeM[BufSize];
  snprintf(ishatimeH, BufSize, "%02d", ishaTimeHour);
  snprintf(ishatimeM, BufSize, "%02d", ishaTimeMinute);
  char ishaTimeHourMin[12];
  strcpy(ishaTimeHourMin, ishatimeH);
  strcat(ishaTimeHourMin, ":");
  strcat(ishaTimeHourMin, ishatimeM);
  puts(ishaTimeHourMin);

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
  //softReset();

  if (viewAltMenu == 1)
  {
    currentSelection = 4;
  }

  //Serial.println(times[0]);

  //Serial.println(times[0]);

  switch (currentSelection)
  {
  case 4:
  {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.drawStr(17, 13, "FAJR: ");
    u8g2.drawStr(87, 13, fajrTimeHourMin);
    u8g2.drawStr(17, 23, "DHUHR: ");
    u8g2.drawStr(87, 23, dhuhrTimeHourMin);
    u8g2.drawStr(17, 33, "ASR: ");
    u8g2.drawStr(87, 33, asrTimeHourMin);
    u8g2.drawStr(17, 43, "MAGHRIB: ");
    u8g2.drawStr(87, 43, maghribTimeHourMin);
    u8g2.drawStr(17, 53, "ISHA: ");
    u8g2.drawStr(87, 53, ishaTimeHourMin);

    if (displayCount2 == 0)
    {
      /*
      u8g2.setDrawColor(1);

      for (int i = 64; i <= 114; i++)
      {
        u8g2.drawHLine(i, 4, 1);
        u8g2.drawHLine(i, 59, 1);
        u8g2.drawHLine((128 - i), 59, 1);
        u8g2.drawHLine((128 - i), 4, 1);

        u8g2.sendBuffer();
        delay(1);
      }
      for (int i = 4; i <= 35; i++)
      {
        u8g2.drawVLine(13, i, 1);
        u8g2.drawVLine(114, i, 1);
        u8g2.drawVLine(13, (63 - i), 1);
        u8g2.drawVLine(114, (63 - i), 1);

        u8g2.sendBuffer();
        delay(1);
      }
      */
    }

    u8g2.setDrawColor(1);
    u8g2.drawHLine(13, 4, 101);
    u8g2.drawHLine(13, 59, 101);
    u8g2.drawVLine(13, 4, 56);
    u8g2.drawVLine(114, 4, 56);
    u8g2.setDrawColor(2);
    u8g2.sendBuffer();
    displayCount2++;
  }
  break;
  case 1:
  {

    // LOADING ANIME //
    if (displayCount == 0)
    {
      /*
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_blipfest_07_tr);
      u8g2.drawStr(((64 - (u8g2.getStrWidth("INTIALISING") / 2))), 34, "INTIALISING");

      for (int i = ((64 - (u8g2.getStrWidth("INTIALISING") / 2))); i <= ((64 + (u8g2.getStrWidth("INTIALISING") / 2))); i++)
      {
        u8g2.drawHLine(i, 38, 1);
        u8g2.sendBuffer();
        delay(1);
      }
      */
    }

    u8g2.clearBuffer();

    // anime time thingy //
    u8g2.setFont(u8g2_font_crox5hb_tn);
    u8g2.drawStr(((64 - (u8g2.getStrWidth(timeStr) / 2)) - 2), 35, timeStr);

    // 'CLICK TO VIEW MORE' subheading //
    u8g2.setFont(u8g2_font_baby_tf);
    u8g2.drawStr(((64 - (u8g2.getStrWidth("CLICK TO VIEW MORE") / 2))), 54, "CLICK TO VIEW MORE");

    // time of tha prayer formatting //
    u8g2.setFont(u8g2_font_blipfest_07_tn);
    moveValtext = ((u8g2.getStrWidth(pTimeHourMin)) / 2);

    // name of tha prayer //
    u8g2.setFont(u8g2_font_baby_tf);
    u8g2.drawStr(((64 - (u8g2.getStrWidth(nextTimePrayerText) / 2)) - moveValtext), 16, nextTimePrayerText);
    //u8g2.drawStr(((64 - (u8g2.getStrWidth((upperPTime + " PRAYER IS AT ")) / 2)) - moveValtext), 54, (upperPTime + " PRAYER IS AT "));

    // time of tha prayer //
    u8g2.setFont(u8g2_font_baby_tf);
    moveValprayer = (64 - (u8g2.getStrWidth(nextTimePrayerText) / 2) + (u8g2.getStrWidth(nextTimePrayerText)));
    u8g2.setFont(u8g2_font_blipfest_07_tn);
    u8g2.drawStr((moveValprayer - moveValtext), 15, pTimeHourMin);

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
  if (buttonState && !buttonStateLast)
  {
    displayCount = 0;
    displayCount2 = 0;
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
  buttonStateLast = buttonState;
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
  char tmp[64]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, 64, fmt, args);
  va_end(args);
  //Serial.print(tmp);
}

char *getNextPTimeName(/*double &pTime,*/ char *pTimeName)
{

  double currTime = hour() + minute() / 60.0;
  int i;

  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(OneSeventh);

  //get_prayer_times(year(), month(), day(), 46.9500, 7.4458, 1, times);
  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 1, times);
  for (i = 0; i < sizeof(times) / sizeof(double); i++)
  {
    if (times[i] >= currTime)
      break;
  }
  if ((times[i] - currTime) < 0)
  {
    i = 0;
    //pTime = times[i];
  }
  //pTime = times[i];
  //sprintf(pTimeName, "%s", TimeName[i]);
  //Serial.println(pTimeName);
  //Serial.println( TYPE_NAME(pTimeName) );
  return pTimeName;
}

double &getNextPTime(double &pTime, char *pTimeName)
{

  double currTime = hour() + minute() / 60.0;
  int i;

  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(OneSeventh);

  //get_prayer_times(year(), month(), day(), 46.9500, 7.4458, 1, times);
  get_prayer_times(year(), month(), day(), 51.373176, -0.210757, 1, times);
  for (i = 0; i < sizeof(times) / sizeof(double); i++)
  {
    if (times[i] >= currTime)
      break;
  }
  if ((times[i] - currTime) < 0)
  {
    i = 0;
    //pTime = times[i];
  }
  //pTime = times[i];
  //sprintf(pTimeName, "%s", TimeName[i]);
  //Serial.println(pTimeName);
  return pTime;
}

void softReset()
{
  asm volatile("  jmp 0");
}
