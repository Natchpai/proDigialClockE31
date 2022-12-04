#define CLK D5
#define DAT D4
#define RST D2

#include <ThreeWire.h>
#include <RtcDS1302.h>

// DAT, CLK, RST
ThreeWire wire(DAT, CLK, RST);
RtcDS1302<ThreeWire> Rtc(wire);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Rtc.Begin();
  checkStateRTC();
  RtcDateTime DefaultDateTime = RtcDateTime(2022, 12, 20, 0, 0, 0);
  Rtc.SetDateTime(DefaultDateTime);
  
}

void checkStateRTC() {
  //แก้ป้องกันการเขียนข้อมูลทับ
  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
    // ค่าเวลามาตราฐาน
    Rtc.SetDateTime(DefaultDateTime);
  }
  // ปิดการทำงาน RTC ถ้าปิดอยู่
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

}

void setRTC(uint16_t year, uint8_t month, uint8_t dayMonth, uint8_t hour, uint8_t min, uint8_t sec) {
  Rtc.SetDateTime(RtcDateTime(year, month, dayMonth, hour, min, sec));
}

void readRTC() {

}


void loop() {

  RtcDateTime now = Rtc.GetDateTime();
  Serial.print(now.Minute());
  Serial.print(" ");
  Serial.println(now.Second());
  delay(1000);
}
