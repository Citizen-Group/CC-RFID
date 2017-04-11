/* ********************************************************************************
 *
 * File: LCD_Utils.ino
 * 
 * Originator: Matthew Caron
 *
 * Description: Handles LCD operations.
 * 
 ******************************************************************************** */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// access to lcd initialization
void displayInit(){
  lcd.init();                      
  lcd.backlight();
}

// displays a string of up to 32 characters on the LCD
void displayText(uint8_t *text, uint8_t len){
  lcd.clear();
  for(uint8_t i = 0; i < smallest(len, 32); i++){
    if(i == 16) // end of row; move to next
      lcd.setCursor(0, 1);
    lcd.write(text[i]);
  }
}

// returns the byte with the lower value
uint8_t smallest(uint8_t a, uint8_t b){
  if(a < b)
    return a;
  return b;
}
