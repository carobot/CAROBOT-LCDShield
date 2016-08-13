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
  Library for CAROBOT LCD Shield
  Written by Jacky Lau, CAROBOT, August 9, 2016
 ***/

#if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
#else
    #include <WProgram.h>
#endif

#include "CAROBOT_LCDShield.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Wire.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// CAROBOT_LCDShield constructor is called).


/***
 * Initialize the LCD Shield
 ***/
void CAROBOT_LCDShield::begin() {

    uint8_t cols = 16;
    uint8_t lines = 2;

    _displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;

    /* MCP23017 pinout*/
    _rs_pin = 15; // GPB7
    _rw_pin = 14; // GPB6
    _enable_pin = 13; // GPB5

    _data_pins[0] = 12; // GPB4 (D4), 4-bit mode
    _data_pins[1] = 11; // GPB3 (D5)
    _data_pins[2] = 10; // GPB2 (D6)
    _data_pins[3] = 9; // GPB1 (D7)

    _button_pins[0] = 0; // GPA0 (Button A)
    _button_pins[1] = 1; // GPA1 (Button Right)
    _button_pins[2] = 2; // GPA2 (Button Down)
    _button_pins[3] = 3; // GPA3 (Button Up)
    _button_pins[4] = 4; // GPA4 (Button Left)
    _button_pins[5] = 5; // GPA5 (Button B)

    _backlight_pins[0] = 6; // GPA6 (Primary/Red)
    _backlight_pins[1] = 7; // GPA7 (Green)
    _backlight_pins[2] = 8; // GPA6 (Blue)

    /* Start I2C communication with MCP23017 */
    Wire.begin();

    // set all I/O to input
    writeRegister(MCP23017_IODIRA, 0xFF);
    writeRegister(MCP23017_IODIRB, 0xFF);

    // set all I/O to default value
    writeRegister(MCP23017_GPIOA, 0x00);
    writeRegister(MCP23017_GPIOB, 0x00);

    // set all pull-up to disable
    writeRegister(MCP23017_GPPUA, 0x00);
    writeRegister(MCP23017_GPPUB, 0x00);

    _numlines = lines;

    setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

    _pinMode(_rs_pin, OUTPUT);
    _pinMode(_rw_pin, OUTPUT);
    _pinMode(_enable_pin, OUTPUT);

    for (uint8_t i = 0; i < 4; ++i) {
        _pinMode(_data_pins[i], OUTPUT);
    }

    for (uint8_t i = 0; i < 6; ++i) {
        _pinMode(_button_pins[i], INPUT_PULLUP);
    }

    for (uint8_t i = 0; i < 3; ++i) {
        _pinMode(_backlight_pins[i], OUTPUT);
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
    delayMicroseconds(50000);
    // Now we pull both RS and R/W low to begin commands
    _digitalWrite(_rs_pin, LOW);
    _digitalWrite(_enable_pin, LOW);
    _digitalWrite(_rw_pin, LOW);

    // put the LCD into 4 bit mode
    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02);

    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);

}

void CAROBOT_LCDShield::setRowOffsets(uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3) {
    _row_offsets[0] = row0;
    _row_offsets[1] = row1;
    _row_offsets[2] = row2;
    _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void CAROBOT_LCDShield::clear() {
    command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    delayMicroseconds(2000);  // this command takes a long time!
}

void CAROBOT_LCDShield::home() {
    command(LCD_RETURNHOME);  // set cursor position to zero
    delayMicroseconds(2000);  // this command takes a long time!
}

void CAROBOT_LCDShield::setCursor(uint8_t col, uint8_t row) {
    const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
    if ( row >= max_lines ) {
        row = max_lines - 1;    // we count rows starting w/0
    }
    if ( row >= _numlines ) {
        row = _numlines - 1;    // we count rows starting w/0
    }

    command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

/***
 * Turn the display on/off (quickly)
 ***/
void CAROBOT_LCDShield::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CAROBOT_LCDShield::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/***
 * Turns the underline cursor on/off
 ***/
void CAROBOT_LCDShield::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CAROBOT_LCDShield::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/***
 * Turn on and off the blinking cursor
 ***/
void CAROBOT_LCDShield::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CAROBOT_LCDShield::blink() {
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/***
 * These commands scroll the display without changing the RAM
 ***/
void CAROBOT_LCDShield::scrollDisplayLeft(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void CAROBOT_LCDShield::scrollDisplayRight(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

/***
 * This is for text that flows Left to Right
 ***/
void CAROBOT_LCDShield::leftToRight(void) {
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

/***
 * This is for text that flows Right to Left
 ***/
void CAROBOT_LCDShield::rightToLeft(void) {
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

/***
 * This will 'right justify' text from the cursor
 ***/
void CAROBOT_LCDShield::autoscroll(void) {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

/***
 * This will 'left justify' text from the cursor
 ***/
void CAROBOT_LCDShield::noAutoscroll(void) {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

/***
 * Allows us to fill the first 8 CGRAM locations
 * with custom characters
 ***/
void CAROBOT_LCDShield::createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    for (uint8_t i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

/***
 * Allows us to read inputs from the buttons
 ***/
uint8_t CAROBOT_LCDShield::readButtons() {
    uint8_t reply = 0x3F;

    for (uint8_t i = 0; i < 6; i++) {
        reply &= ~((_digitalRead(_button_pins[i])) << i);
    }
    return reply;
}

/***
 * Allows us to set the backlight colour
 ***/
void CAROBOT_LCDShield::setBacklight(uint8_t colour) {
    _digitalWrite(_backlight_pins[0], ~colour & 0x1); // RED
    _digitalWrite(_backlight_pins[1], ~colour >> 1 & 0x1); // GREEN
    _digitalWrite(_backlight_pins[2], ~colour >> 2 & 0x1); // BLUE
}

/*********** mid level commands, for sending data/cmds */

inline void CAROBOT_LCDShield::command(uint8_t value) {
    send(value, LOW);
}

#if ARDUINO >= 100
inline size_t CAROBOT_LCDShield::write(uint8_t value) {
    send(value, HIGH);
    return 1; // assume sucess
}
#else
inline size_t CAROBOT_LCDShield::write(uint8_t value) {
    send(value, HIGH);
    // no return for order arduion version
}
#endif

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void CAROBOT_LCDShield::send(uint8_t value, uint8_t mode) {
    _digitalWrite(_rs_pin, mode);
    _digitalWrite(_rw_pin, LOW);

    write4bits(value>>4);
    write4bits(value);
}

void CAROBOT_LCDShield::pulseEnable(void) {
    _digitalWrite(_enable_pin, LOW);
    delayMicroseconds(1);
    _digitalWrite(_enable_pin, HIGH);
    delayMicroseconds(1);    // enable pulse must be >450ns
    _digitalWrite(_enable_pin, LOW);
    delayMicroseconds(100);   // commands need > 37us to settle
}

void CAROBOT_LCDShield::write4bits(uint8_t value) {
    for (uint8_t i = 0; i < 4; i++) {
        _digitalWrite(_data_pins[i], (value >> i) & 0x01);
    }

    pulseEnable();
}

/*****************************************************************************
 * MCP23017 Functions
 ***/

/***
 * Helper functions to keep backward compatibility
 ***/
static inline void wire_write(uint8_t x) {
#if ARDUINO >= 100
    Wire.write((uint8_t) x);
#else
    Wire.send(x);
#endif
}

static inline uint8_t wire_read(void) {
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}

/***
 * Write to a given register
 ***/
void CAROBOT_LCDShield::writeRegister(uint8_t regAddress, uint8_t regValue) {
	Wire.beginTransmission(MCP23017_ADDRESS);
	wire_write(regAddress);
	wire_write(regValue);
	Wire.endTransmission();
}

/***
 * Read from a given register
 ***/
uint8_t CAROBOT_LCDShield::readRegister(uint8_t regAddress) {
	Wire.beginTransmission(MCP23017_ADDRESS);
	wire_write(regAddress);
	Wire.endTransmission();

	Wire.requestFrom(MCP23017_ADDRESS, 1);

	return wire_read();
}

/***
 * Write to a given bit in a register
 ***/
void CAROBOT_LCDShield::writeRegisterBit(uint8_t regAddress, uint8_t bit, uint8_t pinValue) {
	uint8_t regValue = readRegister(regAddress);

	regValue = bitWrite(regValue, bit, pinValue);

	writeRegister(regAddress, regValue);
}

/***
 * Set the pinmode of one GPIO
 ***/
void CAROBOT_LCDShield::_pinMode(uint8_t pin, uint8_t mode) {

    uint8_t regIODIR = MCP23017_IODIRA;
    uint8_t regGPPU = MCP23017_GPPUA;
    if (pin > 7) {
        regIODIR = MCP23017_IODIRB;
        regGPPU = MCP23017_GPPUB;
    }

    // For MCP23017, output is 0, input is 1
    // For weak 100k pull-up, enable is 1, disable is 0
    if (mode == OUTPUT) {
        writeRegisterBit(regGPPU, pin % 8, 0);
        writeRegisterBit(regIODIR, pin % 8, 0);
    } else if (mode == INPUT) {
        writeRegisterBit(regIODIR, pin % 8, 1);
        writeRegisterBit(regGPPU, pin % 8, 0);
    } else if (mode == INPUT_PULLUP) {
        writeRegisterBit(regIODIR, pin % 8, 1);
        writeRegisterBit(regGPPU, pin % 8, 1);
    }
}

/***
 * Write to a GPIO
 ***/
void CAROBOT_LCDShield::_digitalWrite(uint8_t pin, uint8_t state) {

    uint8_t regIODIR = MCP23017_IODIRA;
    uint8_t regGPPU = MCP23017_GPPUA;
    uint8_t regGPIO = MCP23017_GPIOA;
    if (pin > 7) {
        regIODIR = MCP23017_IODIRB;
        regGPPU = MCP23017_GPPUB;
        regGPIO = MCP23017_GPIOB;
    }

    writeRegisterBit(regGPPU, pin % 8, 0);
    writeRegisterBit(regIODIR, pin % 8, 0);
    writeRegisterBit(regGPIO, pin % 8, state);
}

/***
 * Read from a GPIO
 ***/
uint8_t CAROBOT_LCDShield::_digitalRead(uint8_t pin) {
    // return 1 if HIGH
    if (pin > 7) {
        return ((readRegister(MCP23017_GPIOB) >> (pin % 8)) & 0x01);
    } else {
        return ((readRegister(MCP23017_GPIOA) >> (pin % 8)) & 0x01);
    }
}
