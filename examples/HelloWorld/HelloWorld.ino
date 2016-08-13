/*****************************************************************************
MIT License

Copyright (c) 2016, CAROBOTIX INC.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

/***
  Basic Hello World display and button input test for RGB 16x2 LCD Shield
  I2C (SCL, SDA) is used so any pins used for that purpose cannot be used
  other than sharing with other I2C device. On Arduino Uno, pin A4 and A5
  is used for I2C.
  Written by Jacky Lau, CAROBOT, August 10, 2016
 ***/

// include the library code:
#include <Wire.h>
#include <CAROBOT_LCDShield.h>

CAROBOT_LCDShield lcd;

void setup() {
  // set up the LCD: 
  lcd.begin();
  // Print a message to the LCD.
  lcd.print("Hello, World!");
  lcd.setBacklight(LCD_BL_ON);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);

  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_A) {
      lcd.print("A ");
      lcd.setBacklight(LCD_BL_PURPLE);
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
      lcd.setBacklight(LCD_BL_BLUE);
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("DOWN ");
      lcd.setBacklight(LCD_BL_YELLOW);
    }
    if (buttons & BUTTON_UP) {
      lcd.print("UP ");
      lcd.setBacklight(LCD_BL_GREEN);
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
      lcd.setBacklight(LCD_BL_RED);
    }
    if (buttons & BUTTON_B) {
      lcd.print("B ");
      lcd.setBacklight(LCD_BL_TEAL);
    }
  }
}
