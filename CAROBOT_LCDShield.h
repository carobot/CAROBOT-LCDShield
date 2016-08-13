#ifndef CAROBOT_LCDShield_H

#define CAROBOT_LCDShield_H

#include <inttypes.h>
#include "Print.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight
#define LCD_BL_OFF 0x00
#define LCD_BL_ON 0x01
#define LCD_BL_RED 0x01
#define LCD_BL_GREEN 0x002
#define LCD_BL_YELLOW 0x03
#define LCD_BL_BLUE 0x04
#define LCD_BL_PURPLE 0x05
#define LCD_BL_TEAL 0x06
#define LCD_BL_WHITE 0x07

#define BUTTON_A 0x01
#define BUTTON_RIGHT 0x02
#define BUTTON_DOWN 0x04
#define BUTTON_UP 0x08
#define BUTTON_LEFT 0x10
#define BUTTON_B 0x20

// for MCP23017
#define MCP23017_ADDRESS 0x20

// registers for MCP23017 port A
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14

// registers for MCP23017 port B
#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

class CAROBOT_LCDShield : public Print {
public:
    void begin();

    void clear();
    void home();

    void noDisplay();
    void display();
    void noBlink();
    void blink();
    void noCursor();
    void cursor();
    void scrollDisplayLeft();
    void scrollDisplayRight();
    void leftToRight();
    void rightToLeft();
    void autoscroll();
    void noAutoscroll();

    void setRowOffsets(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);
    void createChar(uint8_t, uint8_t[]);
    uint8_t readButtons();
    void setBacklight(uint8_t colour);
    void setCursor(uint8_t, uint8_t);
    virtual size_t write(uint8_t);
    void command(uint8_t);

    using Print::write;

private:
    void send(uint8_t, uint8_t);
    void write4bits(uint8_t);
    void pulseEnable();

    uint8_t _rs_pin; // LOW: command.  HIGH: character.
    uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
    uint8_t _enable_pin; // activated by a HIGH pulse.
    uint8_t _data_pins[8];
    uint8_t _button_pins[8];
    uint8_t _backlight_pins[8];

    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;

    uint8_t _initialized;

    uint8_t _numlines;
    uint8_t _row_offsets[4];

    /* MCP23017 */
    void writeRegister(uint8_t regAddress, uint8_t regValue);
    uint8_t readRegister(uint8_t regAddress);
    void writeRegisterBit(uint8_t regAddress, uint8_t bit, uint8_t pinValue);
    void _pinMode(uint8_t pin, uint8_t mode);
    void _digitalWrite(uint8_t pin, uint8_t state);
    uint8_t _digitalRead(uint8_t pin);
};

#endif
