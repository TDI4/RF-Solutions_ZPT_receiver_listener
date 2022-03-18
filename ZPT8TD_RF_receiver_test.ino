/*---------------------------------------------------------------------------------------------------------------------------------- -
Author:   Tóth Viktor
Release:  2022.03.18
Description: This code listen for packets on serial port from Rf-Solution's "FOB-OEM" ZPT-xxx modul, and decodes the received data
             into structure for further processing. Optional parts are used to read status of the digital outputs on the module.
             Output is HIGH only if the receiver learnt the transmitter, and the button is pressed.
Hardware: This code was tested on Arduino Uno, you might need to change pinout on different board.
-----------------------------------------------------------------------------------------------------------------------------------*/
#include <SoftwareSerial.h>
//-----------------------------------------------------------------------------------------------------------------------------------
#define button4 A0
#define button3 A1
#define button2 A2
#define button1 A3
//-----------------------------------------------------------------------------------------------------------------------------------
          //optional!******************************************
          bool button1state = false, button2state = false, button3state = false, button4state = false, prevButton1state = false, prevButton2state = false, prevButton3state = false, prevButton4state = false;
          //***************************************************

byte incomingDataContent[10], incomingDataLength = 10;
struct ZPT8RDdata { long serialNo; byte buttonPressed; bool lowBatteryAlert; bool transmitterLearnt; byte rssiPercentage; };
ZPT8RDdata transmitterData;
//-----------------------------------------------------------------------------------------------------------------------------------
byte traceLevel = 3;           // 0=High 1=Medium 2=Low 3=Off
SoftwareSerial rfSerial(8, 9); // RX, TX
//-----------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  while (!Serial);
  Serial.println("Arduino connected.");
  rfSerial.begin(19200);
  while(!rfSerial);
  Serial.println("Serial connection between host and RF receiver established.");

          //optional!******************************************
          pinMode(button1, INPUT);
          pinMode(button2, INPUT);
          pinMode(button3, INPUT);
          pinMode(button4, INPUT);
          //***************************************************
}//setup
//-----------------------------------------------------------------------------------------------------------------------------------
void loop() {

  if (rfSerial.available()) {
    transmitterData = handleZPT8RDreceiver(traceLevel);
  }

          //optional!******************************************
          button1state = digitalRead(button1);
          button2state = digitalRead(button2);
          button3state = digitalRead(button3);
          button4state = digitalRead(button4);

          if (button1state != prevButton1state) {
            Serial.print("Button1 state: ");
            Serial.println(button1state);
            prevButton1state = button1state;
          }
          if (button2state != prevButton2state) {
            Serial.print("Button2 state: ");
            Serial.println(button2state);
            prevButton2state = button2state;
          }
          if (button3state != prevButton3state) {
            Serial.print("Button3 state: ");
            Serial.println(button3state);
            prevButton3state = button3state;
          }
          if (button4state != prevButton4state) {
            Serial.print("Button4 state: ");
            Serial.println(button4state);
            prevButton4state = button4state;
          }
          //***************************************************
}//loop
//-----------------------------------------------------------------------------------------------------------------------------------
ZPT8RDdata handleZPT8RDreceiver(byte traceLevel) {
  ZPT8RDdata tempZPT8RDdata;
  byte tempByte;

  while (rfSerial.available()) {
    rfSerial.readBytes(incomingDataContent, incomingDataLength);

    //Extract serial number
    tempZPT8RDdata.serialNo = 0;
    tempZPT8RDdata.serialNo += (long)incomingDataContent[2];
    tempZPT8RDdata.serialNo += (long)incomingDataContent[1] << 8;
    tempZPT8RDdata.serialNo += (long)incomingDataContent[0] << 16;
    tempZPT8RDdata.serialNo += (long)0b00000000             << 24;

    //Extract pressed button number
    switch (incomingDataContent[3]) {
      case 0: //no button pressed
        tempZPT8RDdata.buttonPressed = 0;
        break;
      case 1:
        tempZPT8RDdata.buttonPressed = 1;
        break;
      case 2:
        tempZPT8RDdata.buttonPressed = 2;
        break;
      case 4:
        tempZPT8RDdata.buttonPressed = 3;
        break;
      case 8:
        tempZPT8RDdata.buttonPressed = 4;
        break;
      default: //unknown value
        tempZPT8RDdata.buttonPressed = 255; 
        break;
    }

    //Extract low battery alert state
    if (incomingDataContent[6] & 0b00000001) {
      tempZPT8RDdata.lowBatteryAlert = true;
    }
    else {
      tempZPT8RDdata.lowBatteryAlert = false;
    }

    //Extract transmitter learnt state
    tempByte = incomingDataContent[6] & 0b00000010;
    if (tempByte >> 1) {
      tempZPT8RDdata.transmitterLearnt = true;
    }
    else {
      tempZPT8RDdata.transmitterLearnt = false;
    }

    //Extract RSSI value and convert to percentage
    tempZPT8RDdata.rssiPercentage = map(incomingDataContent[7], 0, 255, 0, 100);

    //Serial debug
    switch (traceLevel){
      case 0:
        Serial.print("Serial: \t");
        Serial.println(tempZPT8RDdata.serialNo, HEX);
        Serial.print("Button: \t");
        Serial.println(tempZPT8RDdata.buttonPressed);
        Serial.print("LowBatt: \t");
        if (tempZPT8RDdata.lowBatteryAlert) Serial.println("true");
        else (Serial.println("false"));
        Serial.print("Learnt: \t");
        if (tempZPT8RDdata.transmitterLearnt) Serial.println("true");
        else (Serial.println("false"));
        Serial.print("RSSI:   \t"); 
        Serial.print(tempZPT8RDdata.rssiPercentage, DEC);
        Serial.println("%");
        Serial.println();
        for (int i = 1; i <= incomingDataLength; i++) {
          Serial.print("byte "); Serial.print(i); Serial.print("\t: ");
          Serial.print(incomingDataContent[i - 1], BIN);
          Serial.print("\t");
          if(incomingDataContent[i - 1] < 16) Serial.print("\t");
          Serial.println(incomingDataContent[i - 1], DEC);
        }
        Serial.println(); Serial.println();
        break;
      case 1:
        Serial.print("Serial: \t");
        Serial.println(tempZPT8RDdata.serialNo, HEX);
        Serial.print("Button: \t");
        Serial.println(tempZPT8RDdata.buttonPressed);
        Serial.print("LowBatt: \t");
        if (tempZPT8RDdata.lowBatteryAlert) Serial.println("true");
        else (Serial.println("false"));
        Serial.print("Learnt: \t");
        if (tempZPT8RDdata.transmitterLearnt) Serial.println("true");
        else (Serial.println("false"));
        Serial.print("RSSI:   \t");
        Serial.print(tempZPT8RDdata.rssiPercentage, DEC);
        Serial.println("%");
        Serial.println(); Serial.println();
        break;
      case 2:
        Serial.println("RF data received");
        break;
      case 3:
        break;
      default:
        Serial.println("Unknown traceLevel!");
        break;
    }
  }
  return tempZPT8RDdata;
}
