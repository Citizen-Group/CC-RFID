/* ********************************************************************************
 *
 * File: RFID_POC.ino
 * 
 * Originator: Matthew Caron
 *
 * Description: 
 * RFID project proof of concept demo.
 * Attempts to recognize scanned RFID cards.
 *  
 ******************************************************************************** */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SSPIN 10
#define BUZZER_PIN 2

MFRC522 mfrc522(SSPIN, RST_PIN);
uint8_t defaultMessage[] = "Please present  an RFID device. ";
uint8_t *tmp;

void setup() {
  
  // setup buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();  
  displayInit();
  mfrc522.PCD_Init();

  displayText(defaultMessage, 32);
}

void loop() { // TODO: attach interrupt to RFID reader
  
  // Look for new cards
  if(!mfrc522.PICC_IsNewCardPresent())
    return;
  
  // Select one of the cards
  if(!mfrc522.PICC_ReadCardSerial())
    return;

  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);

  tmp = retrieveRecord(mfrc522.uid.uidByte, mfrc522.uid.size);

  mfrc522.PICC_HaltA(); // Stop reading

  displayText(tmp, 20);

  delay(4000);

  displayText(defaultMessage, 32);
}
