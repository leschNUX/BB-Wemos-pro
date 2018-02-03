#include <SoftwareSerial.h>
#include <RDM6300.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>

SoftwareSerial RFID(2, 3); // RX and TX

int Led=13;
uint8_t Payload[6]; // used for read comparisons

RDM6300 RDM6300(Payload);

void setup()
{
  pinMode(Led, OUTPUT);
  RFID.begin(9600);
  Serial.begin(9600);  
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
}

void loop()
{
  while (RFID.available() > 0) 
  {
    digitalWrite(Led, HIGH);
    uint8_t c = RFID.read();
    //Serial.println(c,HEX);
    if (RDM6300.decode(c)) {
      for (int i=0; i < 12; i++){
        Serial.print(Payload[i], HEX);
        Serial.print(" ");
      } 
      Serial.println();
    }  
  }
  digitalWrite(Led, LOW);   
  delay(100);
}
