
// int digits [10][8]{
//   {0,1,1,1,1,1,1,0}, // digit 0 0b01111110
//   {0,0,1,1,0,0,0,0}, // digit 1 0b00110000
//   {0,1,1,0,1,1,0,1}, // digit 2 0b01101101
//   {0,1,1,1,1,0,0,1}, // digit 3 0b01111001
//   {0,0,1,1,0,0,1,1}, // digit 4 0b00110011
//   {0,1,0,1,1,0,1,1}, // digit 5 0b01011011
//   {0,1,0,1,1,1,1,1}, // digit 6 0b01011111
//   {0,1,1,1,0,0,0,0}, // digit 7 0b01110000
//   {0,1,1,1,1,1,1,1}, // digit 8 0b01111111
//   {0,1,1,1,1,0,1,1}};  // digit 9 0b01111011


#define DATA 3
#define CLK 4
#define LATCH 5

uint8_t digits[10] = {126, 48, 109, 121, 51, 91, 95, 112, 127, 123};  // 0 -> 9 from binary
uint8_t ick = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
}

void loop() {
    digitalWrite(LATCH, 0);
    shiftOut(DATA, CLK, LSBFIRST, digits[8]);

    digitalWrite(LATCH, 1);
    digitalWrite(LATCH, 0);
    delay(500);

}
