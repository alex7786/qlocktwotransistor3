// SMD Variant !!!

#include <Wire.h>
//#include "RTClib.h"
#include <DS3232RTC.h>
#include <Time.h> 

#define DEBUG false //set to true for Serial Information
#define SHOWTIME false //set to true for Serial clock
#define SHOWLDR false //set to true for LDR Information

#define SINGLELEDMAT false  //set to true for 1 LED at a time or false for whole word multiplexing

#define ALLONTESTDELAY false
#define ALLONTESTDELAYLENGTH 100

const int hourplus = 12;
const int minplus = 11;

const int LDR = A0;      //LDR min = 0; LDR max = 1023
const int dimPin = 10;
const int dimPinOuterLEDS = 9;
const int pwmtable_8[32]=
{
    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
};

const int mrshiftreset = 2;
const int datapin = 6;
const int clockpin = 5;
const int latch = 7;

const int upperright =3;
const int downright =8;
const int downleft =13; 
const int upperleft =4; 

//help variables for offset calc
bool fuenf = false;
bool zehn = false;
bool viertel = false;
bool dreiviertel = false;
bool vor = false;
bool nach = false;
bool halb = false;
bool seccorrhelp = false;

int seccorrday = 0;      //seconds correction

int minutebuff;
int hourbuff;

// Shift Register Order : SR1: (S1S2S3S4S5S6S7S8) SR2: (S9S10S11R2R1R10R9R8) SR3: (R7R6R5R4R3)
const byte qlockarstd[3] = {0b11111111, 0b11100000, 0b00000000};
byte qlockar[3];

const int onboardled = 13;

time_t myTime;
//RTC_DS1307 RTC;

void settimeswitch(int hourplus, int minplus);
void ldrdrim(int ldrin, int ldrdimout, int minutes);
void qlocksetman(byte qlockset[3]);

void minutes(int minutes);
void esist();
void uhr(int minutes);
void houroffset(int minutes);
void hours(int hours, int minutes);
void secondcorr(int hours, int minutes,int seconds, bool helpvar, int seccor);
void allon();
void alloff();


void setup () {
  Serial.begin(57600);
  Wire.begin();
  //RTC.begin();
  pinMode(hourplus, INPUT_PULLUP);
  pinMode(minplus, INPUT_PULLUP);
  
  pinMode(mrshiftreset, OUTPUT);
  digitalWrite(mrshiftreset, LOW);
  digitalWrite(mrshiftreset, HIGH);
  pinMode(datapin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(latch, OUTPUT);
  
  pinMode(upperright, OUTPUT);
  pinMode(downright, OUTPUT);
  pinMode(downleft, OUTPUT);
  pinMode(upperleft, OUTPUT);
  pinMode(onboardled, OUTPUT);
  pinMode(dimPin, OUTPUT);
  
  alloff();

  /*
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //RTC.adjust(DateTime(__DATE__, __TIME__));      //sets the RTC to the date & time this sketch was compiled
  }
  */

}

void loop () {
  myTime = RTC.get();
  //DateTime now = RTC.now();
  int minutevar = (int)minute(myTime);
  int hourvar = (int)hour(myTime);
  if(hourvar < 25 && minutevar < 60)
  {
    minutebuff = minutevar;
    hourbuff = hourvar;
  }
  if(minutevar > 60)
    {
      minutevar = minutebuff;
    }
    
    if(hourvar > 25)
    {
      hourvar = hourbuff;
    }
    
  for(int i = 0; i < 500; i++)
  {
        
    settimeswitch(hourplus, minplus);
    ldrdrim(LDR, dimPin, minutevar);
    
    //allon();
  
    minutes(minutevar);  
    
    uhr(minutevar);
    alloff();
    hours(hourvar, minutevar);
    alloff();
    houroffset(minutevar);
    alloff();
    esist();
    alloff();
    secondcorr(hourvar, minutevar, (int)second(myTime), seccorrhelp, seccorrday);
  }
}

void esist()
{
  if(!SINGLELEDMAT)
  {
    qlockar[0] = 0b11011100;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar);
  }
  else
  {
    qlockar[0] = 0b10000000;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar);
    qlockar[0] = 0b01000000;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar);
    qlockar[0] = 0b00010000;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar);
    qlockar[0] = 0b00001000;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar);
    qlockar[0] = 0b00000100;
    qlockar[1] = 0b00001000;
    qlockar[2] = 0b00000000;
    qlocksetman(qlockar); 
  }
}


void houroffset(int minutes)
{
  if(!SINGLELEDMAT)
  {
      if(minutes >= 5 && minutes <= 9 || minutes >= 25 && minutes <= 29 || minutes >= 35 && minutes <= 39 || minutes >= 55 && minutes <= 59 )  //FÜNF
     {
      fuenf = true;
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b11101000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       fuenf = false;
     }
     
     if(minutes >= 10 && minutes <= 14 || minutes >= 20 && minutes <= 24 || minutes >= 40 && minutes <= 44 || minutes >= 50 && minutes <= 54)  //ZEHN
     {
      zehn = true;
      qlockar[0] = 0b11110000;
      qlockar[1] = 0b00010000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       zehn = false;
     }
     
     if(minutes >= 15 && minutes <= 19)  //VIERTEL
     {
      viertel = true;
      qlockar[0] = 0b00001111;
      qlockar[1] = 0b11100000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       viertel = false;
     }
     
     if(minutes >= 45 && minutes <= 49)  //DREIVIERTEL
     {
      dreiviertel = true;
      qlockar[0] = 0b11111111;
      qlockar[1] = 0b11100000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       dreiviertel = false;
     }
     
     if(minutes >= 20 && minutes <= 29 || minutes >= 50 && minutes <= 59)  //VOR
     {
      vor = true;
      qlockar[0] = 0b11100000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       vor = false;
     }
     
     if(minutes >= 5 && minutes <= 14 || minutes >= 35 && minutes <= 44)  //NACH
     {
      nach = true;
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b11100000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       nach = false;
     }
     
     if(minutes >= 20 && minutes <= 44)  //HALB
     {
      halb = true;
      qlockar[0] = 0b11110000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00100000;
      qlocksetman(qlockar);
      alloff();
     }
     else
     {
       halb = false;
     }
  }
  
  else
  {
     if(minutes >= 5 && minutes <= 9 || minutes >= 25 && minutes <= 29 || minutes >= 35 && minutes <= 39 || minutes >= 55 && minutes <= 59 )  //FÜNF
     {
      fuenf = true;
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b00001000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b10001000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b01001000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b00101000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
     }
     else
     {
       fuenf = false;
     }
     
     if(minutes >= 10 && minutes <= 14 || minutes >= 20 && minutes <= 24 || minutes >= 40 && minutes <= 44 || minutes >= 50 && minutes <= 54)  //ZEHN
     {
      zehn = true;
      qlockar[0] = 0b10000000;
      qlockar[1] = 0b00010000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b01000000;
      qlockar[1] = 0b00010000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00100000;
      qlockar[1] = 0b00010000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00010000;
      qlockar[1] = 0b00010000;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
     }
     else
     {
       zehn = false;
     }
     
     if(minutes >= 15 && minutes <= 19)  //VIERTEL
     {
      viertel = true;
      qlockar[0] = 0b00001000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000100;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000010;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b10000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b01000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b00100000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
     }
     else
     {
       viertel = false;
     }
     
     if(minutes >= 45 && minutes <= 49)  //DREIVIERTEL
     {
      dreiviertel = true;
      qlockar[0] = 0b10000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b01000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00100000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00010000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
       
      qlockar[0] = 0b00001000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000100;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000010;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b10000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b01000000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b00100000;
      qlockar[2] = 0b00001000;
      qlocksetman(qlockar);
     }
     else
     {
       dreiviertel = false;
     }
     
     if(minutes >= 20 && minutes <= 29 || minutes >= 50 && minutes <= 59)  //VOR
     {
      vor = true;
      qlockar[0] = 0b10000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      qlockar[0] = 0b01000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00100000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
     }
     else
     {
       vor = false;
     }
     
     if(minutes >= 5 && minutes <= 14 || minutes >= 35 && minutes <= 44)  //NACH
     {
      nach = true;
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b00100000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b01000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b10000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000001;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00010000;
      qlocksetman(qlockar);
     }
     else
     {
       nach = false;
     }
     
     if(minutes >= 20 && minutes <= 44)  //HALB
     {
      halb = true;
      qlockar[0] = 0b10000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00100000;
      qlocksetman(qlockar);
      qlockar[0] = 0b01000000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00100000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00100000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00100000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00010000;
      qlockar[1] = 0b00000000;
      qlockar[2] = 0b00100000;
      qlocksetman(qlockar);
     }
     else
     {
       halb = false;
     }
  }
}

void minutes(int minutes)
{
  //DateTime now = RTC.now();
  
  switch(minutes % 5){
    
    case 1: digitalWrite(upperleft, HIGH);
            digitalWrite(upperright, LOW);
            digitalWrite(downright, LOW);
            digitalWrite(downleft, LOW);
            break;
    case 2: digitalWrite(upperleft, HIGH);
            digitalWrite(upperright, HIGH);
            digitalWrite(downright, LOW);
            digitalWrite(downleft, LOW);
            break;
    case 3: digitalWrite(upperleft, HIGH);
            digitalWrite(upperright, HIGH);
            digitalWrite(downright, HIGH);
            digitalWrite(downleft, LOW);
            break;
    case 4: digitalWrite(upperleft, HIGH);
            digitalWrite(upperright, HIGH);
            digitalWrite(downright, HIGH);
            digitalWrite(downleft, HIGH);
            break;
    default:digitalWrite(upperleft, LOW);
            digitalWrite(upperright, LOW);
            digitalWrite(downright, LOW);
            digitalWrite(downleft, LOW);
  }
}

void hours(int hours, int minutes)
{
  hours = hours%12;
  if(halb || viertel || dreiviertel || zehn && vor || fuenf && vor)
     {
       hours++; 
     } 
  
  if(!SINGLELEDMAT)
  {
  switch(hours){
    case 1: if(minutes >= 0 && minutes <= 4)  //EIN UHR
            {
              qlockar[0] = 0b11100000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              break;
            }
            else                              //EINS
            {
              qlockar[0] = 0b11110000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              break;
            }
    case 2: qlockar[0] = 0b00000001;
            qlockar[1] = 0b11100000;
            qlockar[2] = 0b01000000;
            qlocksetman(qlockar);
            break;
    case 3: qlockar[0] = 0b11110000;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            break;
    case 4: qlockar[0] = 0b00000001;
            qlockar[1] = 0b11100000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            break;
    case 5: qlockar[0] = 0b00000001;
            qlockar[1] = 0b11100000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            break;
    case 6: qlockar[0] = 0b11111000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 7: qlockar[0] = 0b11111100;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 8: qlockar[0] = 0b00000001;
            qlockar[1] = 0b11100001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;    
    case 9: qlockar[0] = 0b00011110;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 10:qlockar[0] = 0b11110000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 11:qlockar[0] = 0b00000111;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            break;
     default:  //ZWÖLF
            qlockar[0] = 0b00000011;
            qlockar[1] = 0b11100010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
    }
  }
            
   else
   {
     switch(hours){
    case 1: if(minutes >= 0 && minutes <= 4)
            {
              qlockar[0] = 0b10000000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              qlockar[0] = 0b01000000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              qlockar[0] = 0b00100000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              break;
            }
            else
            {
              qlockar[0] = 0b10000000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              qlockar[0] = 0b01000000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              qlockar[0] = 0b00100000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              qlockar[0] = 0b00010000;
              qlockar[1] = 0b00000000;
              qlockar[2] = 0b01000000;
              qlocksetman(qlockar);
              break;
            }
    case 2: qlockar[0] = 0b00000000;
            qlockar[1] = 0b00100000;
            qlockar[2] = 0b01000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b01000000;
            qlockar[2] = 0b01000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b10000000;
            qlockar[2] = 0b01000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b01000000;
            qlocksetman(qlockar);
            break;
    case 3: qlockar[0] = 0b10000000;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b01000000;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00100000;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00010000;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            break;
    case 4: qlockar[0] = 0b00000000;
            qlockar[1] = 0b00100000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b01000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b10000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b10000000;
            qlocksetman(qlockar);
            break;
    case 5: qlockar[0] = 0b00000000;
            qlockar[1] = 0b00100000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b01000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b10000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            break;
    case 6: qlockar[0] = 0b10000000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b01000000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00100000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00010000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00001000;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 7: qlockar[0] = 0b10000000;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b01000000;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00100000;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00010000;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00001000;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000100;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 8: qlockar[0] = 0b00000000;
            qlockar[1] = 0b00100001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b01000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b10000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000001;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;    
    case 9: qlockar[0] = 0b00010000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00001000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000100;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000010;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 10:qlockar[0] = 0b10000000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b01000000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00100000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00010000;
            qlockar[1] = 0b00000100;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            break;
    case 11:qlockar[0] = 0b00000100;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000010;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000000;
            qlockar[2] = 0b00100000;
            qlocksetman(qlockar);
            break;
     default:  //ZWÖLF
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b00100010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b01000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000000;
            qlockar[1] = 0b10000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000001;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
            qlockar[0] = 0b00000010;
            qlockar[1] = 0b00000010;
            qlockar[2] = 0b00000000;
            qlocksetman(qlockar);
     }
  }
}

void uhr(int minutes)
{
  if(minutes >= 0 && minutes <= 4)
  {
    if(!SINGLELEDMAT)
    {
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b11100100;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
    }
    
    else
    {
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b10000100;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b01000100;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
      qlockar[0] = 0b00000000;
      qlockar[1] = 0b00100100;
      qlockar[2] = 0b00000000;
      qlocksetman(qlockar);
    }
  }
}

void qlocksetman(byte qlockset[3])
{
  /* All off:
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b00000 000;
  qlocksetman(qlockar);
  */
  
  digitalWrite(latch, LOW);
  for(int i = 2; i >= 0; i--)
  {
    shiftOut(datapin, clockpin, LSBFIRST, qlockset[i]); 
    if(DEBUG)
    {
      Serial.print("SR number: ");
      Serial.println(i);
      Serial.println(qlockset[i]);
    }
  }
  digitalWrite(latch, HIGH);
}

void ldrdrim(int ldrin, int ldrdimout, int minutes)
{
  int outterdim = pwmtable_8[25 - ((analogRead(ldrin)/50))];
  if(minutes % 5 == 1)
  {
    outterdim = pwmtable_8[(25 - ((analogRead(ldrin)/50)))/2];
  }
  else if(minutes % 5 == 4)
  {
    outterdim = pwmtable_8[(25 - ((analogRead(ldrin)/50)))*2];
  }
  if(outterdim < 1)
  {
     outterdim = 1; 
  }
  //int innerdim = pwmtable_8[(analogRead(ldrin)/100)];
  int innerdim = 0;
  
  if(SHOWLDR)
  {
    Serial.print("LDR Value: ");
    Serial.println(analogRead(ldrin));
    Serial.print("LDR Value Inner: ");
    Serial.println(innerdim);
    Serial.print("LDR Value Outer: ");
    Serial.println(outterdim);
  }
  
  //analogWrite(ldrdimout, (analogRead(ldrin)/dimfactor));    // Value between 0 and 255
  analogWrite(ldrdimout, innerdim); 
  analogWrite(dimPinOuterLEDS, outterdim);
}

void settimeswitch(int hourplus, int minplus)
{    
  //DateTime now = RTC.now();
  
  if(SHOWTIME)
  {
    Serial.print("Current Time set: ");
    Serial.print(hour(myTime), DEC);
    Serial.print(':');
    Serial.print(minute(myTime), DEC);
    Serial.print(':');
    Serial.print(second(myTime), DEC);
    Serial.println();
    Serial.println();
    
    Serial.print("Current Button State: ");
    Serial.print(digitalRead(hourplus));
    Serial.print(digitalRead(minplus));
    Serial.println();
    Serial.println();
  }
  
  if(!digitalRead(hourplus))
  {
    //RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour()+1,now.minute(),0));
    setTime(hour(myTime)+1,minute(myTime),second(myTime),day(myTime),month(myTime),year(myTime));
    myTime = now();
    RTC.set(myTime);
    delay(300);
    if(hour(myTime) >= 24)
    {
      //RTC.adjust(DateTime(now.year(),now.month(),now.day(),0,now.minute(),0));
      setTime(0,minute(myTime),second(myTime),day(myTime),month(myTime),year(myTime));
      myTime = now();
      RTC.set(myTime);
    }
  }
  
  if(!digitalRead(minplus))
  {
    //RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour(),now.minute()+1,0));
    setTime(hour(myTime),minute(myTime)+1,0,day(myTime),month(myTime),year(myTime));
    myTime = now();
    RTC.set(myTime);
    delay(300);
    if(minute(myTime) >= 60)
    {
      //RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour(),0,0));
      setTime(hour(myTime),0,0,day(myTime),month(myTime),year(myTime));
      myTime = now();
      RTC.set(myTime);
    }
  }
}

void allon()
{  
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11110000;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11101000;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100100;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100010;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100001;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b10000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b01000000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b00100000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b00010000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
  qlockar[0] = 0b11111111;
  qlockar[1] = 0b11100000;
  qlockar[2] = 0b00001000;
  qlocksetman(qlockar);
  if(ALLONTESTDELAY)
  {
    delay(ALLONTESTDELAYLENGTH);
  }
}

void alloff()
{
  qlockar[0] = 0b00000000;
  qlockar[1] = 0b00000000;
  qlockar[2] = 0b00000000;
  qlocksetman(qlockar);
}

void secondcorr(int hours, int minutes, int seconds, bool helpvar, int seccor)
{
  //DateTime now = RTC.now();
  
  if(hours == 0 && minutes == 0 && seconds == 0 && helpvar == false)
  {
    helpvar = true;
    //RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second() + seccor));
    setTime(hour(myTime),minute(myTime),second(myTime) + seccor,day(myTime),month(myTime),year(myTime));
    myTime = now();
    RTC.set(myTime);
  }
  
  if(hours == 1)
  {
    helpvar = false;
  }
}
