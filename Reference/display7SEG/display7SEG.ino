#include <Arduino.h>
#include <TM1637Display.h>

#define CLK D6
#define DIO D7

TM1637Display display(CLK, DIO);

uint8_t a = 1;
uint8_t b = 1;
uint8_t c = 0;
uint8_t d = 0;
uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
uint8_t dot = 64; // 0b01000000
void setup() {
  // put your setup code here, to run once:
  display.setBrightness(0);
}

int processDate(uint8_t a2, uint8_t b2, uint8_t c2, uint8_t d2) {
  String data = String(a2) + String(b2) + String(c2) + String(d2);
  return data.toInt();
}

void loop() {
  // put your main code here, to run repeatedly:
  display.showNumberDecEx(1213, dot);
  delay(1000);
  display.showNumberDecEx(1214, dot);
  delay(1000);
  display.showNumberDecEx(1215, dot);
  delay(1000);
  display.showNumberDecEx(processDate(a, b, c, d), dot);
  delay(1000);

}
