#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "NatchPai";                      
const char* password = "powerpay4";                 

int timezone = 7 * 3600;                 
int dst = 0;   

WiFiUDP ntp;
NTPClient timeClient(ntp, "pool.ntp.org", timezone);

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);                   
  WiFi.begin(ssid, password);
  //Set AutoReconnect
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  timeClient.begin();
}

void loop() {  
  timeClient.update();
  Serial.print(timeClient.getHours() + 200);
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  delay(1000);
}