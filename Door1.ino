#include <SoftwareSerial.h>
#include <RDM6300.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <JsonParser.h>
#include <SimpleTimer.h>
SimpleTimer heartbeatTimer;

JsonParser<32> parser;

#define USE_SERIAL Serial
SoftwareSerial RFID(2, 3); // RX and TX

#define RST_PIN         D3        //NFC
#define SS_PIN          D8        //NFC
MFRC522 mfrc522(SS_PIN, RST_PIN);  // NFC MFRC522 

int inputSwitch = D1;
int Led=13;
uint8_t Payload[6]; // RFID
char payload[150];

RDM6300 RDM6300(Payload);

void setup()
{
    Serial.begin(57600); 
    pinMode(inputSwitch, INPUT);

// Send a bootup message to the server to say the device is online
    bootMessage();
    heartbeatTimer.setInterval(60000, heartbeatMessage);
       
// Serial
    USE_SERIAL.begin(57600);
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
  
// RID 125hz
   pinMode(Led, OUTPUT);
   RFID.begin(9600);
  
// WIFI Manager  
   WiFiManager wifiManager;
   wifiManager.autoConnect("AutoConnectAP");
   Serial.println("connected...yeey :)");

// Check is WIFI connected
    while (WiFi.status() != WL_CONNECTED) {
    }

// NFC
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  
}// end setup

void loop()
{
// Need to add in RFID's
  if (!digitalRead(inputSwitch)) {
  sendMessage();
   }
  
// Heatbeat
  heartbeatTimer.run();

// RFID
   if(RFID.available() > 0) 
  {
    digitalWrite(Led, HIGH);
    uint8_t c = RFID.read();
    if (RDM6300.decode(c)) {
      for (int i=0; i < 12; i++){
        Serial.print(Payload[i], HEX);
        Serial.print(" ");
      } 
    }
  }
   
 // NFC
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  String rfidUid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
  rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
  rfidUid += String(mfrc522.uid.uidByte[i], HEX);
    Serial.println(rfidUid);
}

}//endloop

void sendMessage() {
    HTTPClient http;
    http.begin("http://46.101.78.57/acs"); //HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("service=status&tag=70005AAA5CD1&device=esp-test&message=lookup");
    if(httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String tmpPayload = http.getString();
            tmpPayload.toCharArray(payload, http.getSize()+1);
            JsonHashTable hashTable = parser.parseHashTable(payload);
            if (!hashTable.success()) {
                Serial.println("Error parsing json");
                Serial.println(payload);
            }
            String name = hashTable.getString("member");
            Serial.println(name);
            Serial.println(tmpPayload);
        } else if (httpCode == 404) {
            Serial.println("Member not found");
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
    http.end();
}

void bootMessage() {
    HTTPClient http;

    http.begin("http://46.101.78.57/acs"); //HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("service=status&device=esp-test&message=boot");
    if(httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {

        } else if (httpCode == 404) {
            Serial.println("Member not found");
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
    http.end();
}

void heartbeatMessage() {
    HTTPClient http;

    http.begin("http://46.101.78.57/acs"); //HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("service=status&device=esp-test&message=heartbeat");
    if(httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
        } else if (httpCode == 404) {
            Serial.println("Member not found");
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
    http.end();   
}

