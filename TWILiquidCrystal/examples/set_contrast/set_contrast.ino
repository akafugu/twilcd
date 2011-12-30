
#include <Wire.h>
#include "TWILiquidCrystal.h"

uint8_t m_addr = 0x32;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(m_addr);

void setup() {                
  Wire.begin();
  Serial.begin(9600);
  
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(13, OUTPUT);
  
  delay(100);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.clear();
  lcd.println("Contrast test");
}

uint8_t contrast = 0;
bool raise = true;

void loop() {
  lcd.setCursor(0, 1);
  if(raise)
    lcd.print("Increment: ");
  else
    lcd.print("Decrement: ");
  
  lcd.print("   ");
  lcd.setCursor(11,1);
  lcd.print(contrast, DEC);
  lcd.testContrast(contrast);
  Serial.print("Contrast: ");
  Serial.println(contrast,DEC);
  delay(500);
  contrast += (raise ? 4 : -4);
  if(contrast == 0 || contrast >= 252)
    raise = (raise ? false : true);
    
  // Clear
  /*Wire.beginTransmission(m_addr);
  Wire.write(0x82); // clear
  Wire.endTransmission();

  

  char foo[] = "hello there¥nbye bye there";
  
  for (uint8_t i = 0; i < strlen(foo); i++) {
     Wire.beginTransmission(m_addr);
     Wire.write(foo[i]);
     Wire.endTransmission();
     delay(250); 
  }
  
  delay(2000);*/
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  
}
