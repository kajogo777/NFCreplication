#include <SPI.h>
#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"
#include "emulatetag.h"
#include "NdefMessage.h"
#include <EEPROM.h>

PN532_SPI interface(SPI, 10);
PN532 nfc_adapter(interface);
EmulateTag nfc_emulator(interface);

NdefMessage message;
uint8_t ndefBuf[126];
uint8_t messageSize;

bool captured = false;

uint8_t success;
uint8_t uid[7];
uint8_t uidLength;

//interface io
uint8_t a = 3;  //For displaying segment "a"
uint8_t b = 4;  //For displaying segment "b"
uint8_t c = 5;  //For displaying segment "c"
uint8_t d = 8;  //For displaying segment "d"
uint8_t e = 9;  //For displaying segment "e"
uint8_t f = 7;  //For displaying segment "f"
uint8_t g = 6;  //For displaying segment "g"
uint8_t buttonNext = 2;

uint8_t digit = 0;
uint8_t buttonState[1];

long debouncing_time = 300; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;

void setup() 
{
  Serial.begin(115200);
  Serial.println("Starting NFC replication");
  pinMode(a, OUTPUT);  //A
  pinMode(b, OUTPUT);  //B
  pinMode(c, OUTPUT);  //C
  pinMode(d, OUTPUT);  //D
  pinMode(e, OUTPUT);  //E
  pinMode(f, OUTPUT);  //F
  pinMode(g, OUTPUT);  //G
  pinMode(buttonNext, INPUT);
  digitalWrite(buttonNext, HIGH);
  attachInterrupt(0, clickedNext, FALLING);
  uidLength = 7;
  message = NdefMessage();
  message.addUriRecord("https://www.google.com");
  messageSize = message.getEncodedSize();
  message.encode(ndefBuf);
  nfc_emulator.setNdefFile(ndefBuf, messageSize);
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  nfc_adapter.begin();
}

void loop() 
{ 
  turnOff();
  displayDigit(digit);
  tryToRead();
  readCard(digit);
  nfc_emulator.setUid(uid);
  nfc_emulator.init();
  Serial.println("\nEmulating\n");
  nfc_emulator.emulate(5000);
}

void tryToRead()
{
    Serial.println("\nWaiting for a tag\n");
    success = nfc_adapter.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if(success){
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");
      Serial.print(uidLength, DEC);
      Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc_adapter.PrintHex(uid, uidLength);
      Serial.println("");
      writeCard(digit);
    }
}

void clickedNext()
{
   if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    clickedHelper();
    last_micros = micros();
  } 
}

void clickedHelper()
{
  digit = (digit+1)%10;
}

bool listenToButton(int button)
{
  uint8_t currentState = digitalRead(button);
  bool clicked = buttonState[button] != currentState && buttonState[button] == LOW;
  buttonState[button] = currentState;
  return clicked;
}

void writeCard(uint8_t card){
  for (uint8_t j = 0 ; j < uidLength ; j++)
      EEPROM.write(card*uidLength+j, uid[j]);
}

void readCard(uint8_t card){
  for (uint8_t j = 0 ; j < uidLength ; j++)
     uid[j] = EEPROM.read(card*uidLength+j);
}

void displayDigit(uint8_t digit)
{
 if(digit!=1 && digit != 4)
 digitalWrite(a,LOW);
 if(digit != 5 && digit != 6)
 digitalWrite(b,LOW);
 if(digit !=2)
 digitalWrite(c,LOW);
 if(digit != 1 && digit !=4 && digit !=7)
 digitalWrite(d,LOW);
 if(digit == 2 || digit ==6 || digit == 8 || digit==0)
 digitalWrite(e,LOW);
 if(digit != 1 && digit !=2 && digit!=3 && digit !=7)
 digitalWrite(f,LOW);
 if (digit!=0 && digit!=1 && digit !=7)
 digitalWrite(g,LOW);
}

void turnOff()
{
  digitalWrite(a,HIGH);
  digitalWrite(b,HIGH);
  digitalWrite(c,HIGH);
  digitalWrite(d,HIGH);
  digitalWrite(e,HIGH);
  digitalWrite(f,HIGH);
  digitalWrite(g,HIGH);
}
