// Board: esp8266 > NodeMCU 1.0(ESP-12E Module)


#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>


// Shift Register SN74HC595N 8-bit SIPO
#define SRCLK D5 
#define SER_DATA D6 
#define LATCH D7
#define DOTpin D8
uint8_t digits[10] = {126, 48, 109, 121, 51, 91, 95, 112, 127, 123};  // 0 -> 9 from binary
uint8_t celsiusUnit[2] = {0b01100011, 0b01001110};
// 0b 0 X X X X X X X
//      a b c d e f g 


// DS3230
#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
// D1 -> SCL, D2 -> SDA
#define SquareWave_pin D0 // D0 -> SQW
uint8_t SquareWave;


const char* ssid = "TP-Link_93C4";                           
const char* password = "47299238";  

int16_t pullData_hour = 80; // Max 49 Day , 1176 hours
bool connection;
bool onePullQuota = true;

int timezone = 7 * 3600;
int dst = 0;                            
WiFiUDP ntp;
NTPClient timeClient(ntp, "europe.pool.ntp.org", timezone);

unsigned long previousPullDataTimes;
unsigned long currentTimes;
unsigned long oldLoadTimes;
unsigned long newLoadTimes;

void setup() {
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
  // Serial.begin(9600);
 
  //Set WIFI
  timeClient.begin();
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  //Set DS3231
  Rtc.Begin();
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock); //Sets pin mode
  Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz); //Sets frequency 
  checkStateRTC();
  pinMode(SquareWave_pin, INPUT);

  // Set 74595
  pinMode(LATCH, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SER_DATA, OUTPUT);
  pinMode(DOTpin, OUTPUT);

  TestStart();
}

void TestStart() {
  for(int i=9;i>=0;i--) {
    digitalWrite(LATCH, 0);
    shiftOut(SER_DATA, SRCLK, LSBFIRST, digits[i]);
    digitalWrite(DOTpin, !digitalRead(DOTpin));
    digitalWrite(LATCH, 1);
    digitalWrite(LATCH, 0);
    delay(100);
    digitalWrite(DOTpin, LOW);
  }
  for(int i=0;i<=4;i++) {
    digitalWrite(LATCH, 0);
    shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
    digitalWrite(LATCH, 1);
    digitalWrite(LATCH, 0);
    delay(100);
  }

  LatchData(0b01000000, 0b00000000, 0b00000000, 0b00000000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b00000000, 0b00000000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01000000, 0b00000000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01000000, 0b01000000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01000000, 0b01100000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01000000, 0b01110000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01000000, 0b01111000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01000000, 0b01001000, 0b01111000, 126, 126); delay(100);
  LatchData(0b01000000, 0b01001000, 0b01001000, 0b01111000, 126, 126); delay(100);
  LatchData(0b01001000, 0b01001000, 0b01001000, 0b01111000, 126, 126); delay(100);
  LatchData(0b01001100, 0b01001000, 0b01001000, 0b01111000, 126, 126); delay(100);
  LatchData(0b01001110, 0b01001000, 0b01001000, 0b01111000, 126, 126); delay(200);
  // display_SET(); delay(300);
  ResetDisplay(); delay(50);
}

void display_SET() {
  digitalWrite(DOTpin, LOW);
  digitalWrite(LATCH, 0);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b01100011);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00001111);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b01001111);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b01011011);
  digitalWrite(LATCH, 1);
  digitalWrite(LATCH, 0);
  delay(1000);
}


// Wifi ESP8266 > below
void checkStatusWifi() {
  if(WiFi.status() == WL_CONNECTED) {
    autoPullData();
  }
  else{
    onePullQuota = true;
  }
}


// Real Time Clock > below
void checkStateRTC() {
  Rtc.Enable32kHzPin(false);
}

void autoPullData() {
  if (onePullQuota) {
    updateDate();
    onePullQuota = false;
    // previousPullDataTimes = 0;
    display_SET();
  }

  currentTimes = millis();
  if( (currentTimes - previousPullDataTimes) > (pullData_hour * 3600000)) {
    previousPullDataTimes = currentTimes;
    updateDate();
    display_SET();
  }

  if (currentTimes < previousPullDataTimes) {previousPullDataTimes = 0;}
}

void updateDate() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *) &epochTime);

  uint16_t year = ptm -> tm_year + 1900;
  uint8_t month = ptm -> tm_mon + 1;
  uint8_t dayMonth = ptm -> tm_mday;
  uint8_t hour = timeClient.getHours();
  uint8_t min = timeClient.getMinutes();
  uint8_t sec = timeClient.getSeconds() + 1;  //Fixed time delay

  Rtc.SetDateTime(RtcDateTime(year, month, dayMonth, hour, min, sec));

}


// Shift Register > below
uint8_t rawSec, rawMinute, rawHour;
uint8_t second_First, second_End;
uint8_t minute_First, minute_End;
uint8_t hour_First, hour_End;

uint8_t partitionFirstDigit(uint8_t data) {
  if( int(data) >= 10) return data / 10;
  else return 0;
}

uint8_t partitionEndDigit(uint8_t data) {
  return int(data) % 10;
}

void analyzeData(RtcDateTime now) {
  rawSec = now.Second();
  rawMinute = now.Minute();
  rawHour = now.Hour();
  // partitionDigit below
  second_First = partitionFirstDigit(rawSec);
  second_End = partitionEndDigit(rawSec);
  minute_First = partitionFirstDigit(rawMinute);
  minute_End = partitionEndDigit(rawMinute);
  hour_First = partitionFirstDigit(rawHour);
  hour_End = partitionEndDigit(rawHour);
}


// SHIFT DATA Below

// LatchData(hour_first, hour_last, minute_first, minute_last, second_first, second_last);
void LatchData(uint8_t Q5, uint8_t Q4, uint8_t Q3, uint8_t Q2, uint8_t Q1, uint8_t Q0) {
  digitalWrite(LATCH, 0);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q0);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q1);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q2);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q3);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q4);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, Q5);
  digitalWrite(LATCH, 1);
  digitalWrite(LATCH, 0);
}

void ResetDisplay() {
  digitalWrite(DOTpin, 0);
  digitalWrite(LATCH, 0);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
  digitalWrite(LATCH, 1);
  digitalWrite(LATCH, 0);
}


// Auto Mode Set Below
uint8_t mode = 1;
uint8_t oldStat;
unsigned int countSquare;

void setMode() {
  SquareWave = digitalRead(SquareWave_pin);
  // 1 Hz SquareWave to 1s.
  if (SquareWave == 1) {
    if(oldStat == 0) {
      countSquare++;
      oldStat = 1;
    }
  }
  else if(SquareWave == 0) {
    oldStat = 0;
  }

  // 1 Hz SquareWave to 500ms.
  // if (SquareWave != oldStat) {
  //   countSquare++;
  //   oldStat = SquareWave;
  // }

  if (mode == 1) {
    // 32 Seconds
    if(countSquare == (35) ) { 
      countSquare = 0;
      digitalWrite(DOTpin, 0);
      mode = 2;
    }
  }

  else if(mode == 2) {
    // 8 Seconds
    if(countSquare == (8) ) {
      countSquare = 0;
      digitalWrite(DOTpin, 0);
      mode = 1;
    }
  }
}


void blinkDot() { 
  if (SquareWave == 1) {
    digitalWrite(DOTpin, 0);
  }
  else {
    digitalWrite(DOTpin, 1);
  }
}


// MAIN below
void loop() {
  // Load time every 50 ms. 
  // Approximate 20 Frame per second
  newLoadTimes = millis();
  if (newLoadTimes - oldLoadTimes >= 50) {
    oldLoadTimes = newLoadTimes;
    checkStatusWifi();
    setMode();
    if (mode == 1) {
      blinkDot();
      RtcDateTime now = Rtc.GetDateTime();
      analyzeData(now);
      // LATCH DATA
      LatchData(digits[hour_First], digits[hour_End], digits[minute_First], digits[minute_End], digits[second_First], digits[second_End]);
    }
    else if (mode == 2) {
      RtcTemperature temp = Rtc.GetTemperature();

      uint8_t temp_F = partitionFirstDigit(temp.AsFloatDegC());
      uint8_t temp_E = partitionEndDigit(temp.AsFloatDegC());
      // LATCH DATA
      digitalWrite(LATCH, 0);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, 0b00000000);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, celsiusUnit[1]);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, celsiusUnit[0]);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, digits[temp_E]);
      shiftOut(SER_DATA, SRCLK, LSBFIRST, digits[temp_F]);
      digitalWrite(LATCH, 1);
      digitalWrite(LATCH, 0);
    }
  }
  if (newLoadTimes < oldLoadTimes) {oldLoadTimes = 0;}
}






