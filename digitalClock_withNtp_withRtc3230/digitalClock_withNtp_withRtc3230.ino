#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//7-SEGMENT
#include <Arduino.h>
#include <TM1637Display.h>
#define CLK D6
#define DIO D7
TM1637Display display(CLK, DIO);
uint8_t dot = 64; // 0b01000000
uint8_t BRIGHT = 0;

const uint8_t SEG_SET[] = {
	SEG_A | SEG_C | SEG_D | SEG_G | SEG_F,           // S
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
	SEG_A | SEG_B | SEG_C,                           // t
	SEG_A   // _
	};

// DS3230
#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

const char* ssid = "NatchPai";                           
const char* password = "powerpay4";  

int16_t pullData_hour = 6;
bool connection;
bool onePullQuota = true;

int timezone = 7 * 3600;
int dst = 0;                            
WiFiUDP ntp;
NTPClient timeClient(ntp, "pool.ntp.org", timezone);

unsigned long previousPullDataTimes;
unsigned long previousTimes;
unsigned long previousTimes2;

unsigned long currentTimes;

void setup() {
  Serial.begin(9600);
  display.setBrightness(BRIGHT);
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
 
  //Set AutoReconnect
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  Rtc.Begin();
  checkStateRTC();

  timeClient.begin();
}


// Wifi ESP8266 > below
void checkStatusWifi() {
  if(WiFi.status() == WL_CONNECTED) {
    // Serial.println("Connect");
    autoPullData();
  }
  else{
    // Serial.println("...Disconnect");
    onePullQuota = true;
  }
}



// Real Time Clock > below
void checkStateRTC() {
  // ปิดการทำงาน RTC ถ้าปิดอยู่
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

void autoPullData() {
  if (onePullQuota) {
    display.setSegments(SEG_SET);
    updateDate();
    onePullQuota = false;
    previousPullDataTimes = 0;
  }

  currentTimes = millis();
  if( (currentTimes - previousPullDataTimes) > (pullData_hour * 3600000)) {
    previousPullDataTimes = currentTimes;
    updateDate();
  }

  if (currentTimes < previousTimes) {previousPullDataTimes = 0;}
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
  uint8_t sec = timeClient.getSeconds() + 1;  //Fix delay

  Rtc.SetDateTime(RtcDateTime(year, month, dayMonth, hour, min, sec));

}


// 7 Segment > below
String editDate(uint8_t data) {
  if (data >= 0 && data <= 9) {
    return '0' + String(data);
  }
  else {
    return String(data);
  }
}

void SEGDisplayYOYO(RtcDateTime now) {

  uint8_t second = now.Second();
  uint8_t minute = now.Minute();
  uint8_t hour = now.Hour();
  
  String DATE = editDate(hour) + editDate(minute);
  display.showNumberDecEx(DATE.toInt(), dot, true);
}


uint8_t mode = 1;
void setMode() {
  if (mode == 1) {
    currentTimes = millis();
    if(currentTimes - previousTimes > 20000) {
      previousTimes = currentTimes;
      mode = 2;
    }
  }
  else if(mode == 2) {
    currentTimes = millis();
    if(currentTimes - previousTimes > 6000) {
      previousTimes = currentTimes;
      mode = 1;
    }
  }

   if (currentTimes < previousTimes) {previousTimes = 0;}
}


void loop() {
  // put your main code here, to run repeatedly: 
  checkStatusWifi();
  setMode();
  if (mode == 1) {
    RtcDateTime now = Rtc.GetDateTime();
    SEGDisplayYOYO(now);
  }
  else if (mode == 2) {
    RtcTemperature temp = Rtc.GetTemperature();
    display.showNumberDec(temp.AsFloatDegC());
  }

}
