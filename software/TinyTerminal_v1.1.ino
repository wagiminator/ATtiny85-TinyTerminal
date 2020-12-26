// TinyTerminal - OLED display with USB serial interface based on ATtiny45/85
//
// This code implements a simple USB controlled 128x64 pixels I2C OLED display.
// It uses V-USB to build a USB communication device class (CDC) for serial
// communication via USB with the ATtiny.
//
// The I2C OLED routine is based on tinyOLEDdemo
// https://github.com/wagiminator/attiny13-tinyoleddemo
//
// The display routine is based on the Tiny Terminal by David Johnson-Davies
// http://www.technoblogy.com/show?TV4
//
//                           +-\/-+
//             A0 (D5) PB5  1|    |8  Vcc
// VUSB D- --- A3 (D3) PB3  2|    |7  PB2 (D2) A1 --- OLED (SCK)
// VUSB D+ --- A2 (D4) PB4  3|    |6  PB1 (D1) ------ BUZZER
//                     GND  4|    |5  PB0 (D0) ------ OLED (SDA)
//                           +----+
//
// Controller:  ATtiny45/85
// Core:        VUSB-AVR (https://github.com/wagiminator/VUSB-AVR)
// Clockspeed:  16.5 MHz internal
//
// 2020 by Stefan Wagner 
// Project Files (EasyEDA): https://easyeda.com/wagiminator
// Project Files (Github):  https://github.com/wagiminator
// License: http://creativecommons.org/licenses/by-sa/3.0/

// Libraries
#include <VUSB_CDC.h>           // part of USB-AVR core
#include <avr/pgmspace.h>       // for using data in program space

// Pin definition
#define BUZZER_PIN      PB1     // buzzer
#define I2C_SCL         PB2     // serial clock pin
#define I2C_SDA         PB0     // serial data pin

// -----------------------------------------------------------------------------
// Main Function
// -----------------------------------------------------------------------------

// main setup
void setup() {
  // prepare and start V-USB CDC
  Serial_VUSB.begin();

  // prepare and start OLED
  OLED_init();
  OLED_clearScreen();

  // prepare buzzer
  DDRB |= (1<<BUZZER_PIN);
  beep();
}

// main loop
void loop() {
  if (Serial_VUSB.available()) OLED_print(Serial_VUSB.read());
}

// -----------------------------------------------------------------------------
// I2C Implementation
// -----------------------------------------------------------------------------

// I2C macros
#define I2C_SDA_HIGH()  DDRB &= ~(1<<I2C_SDA) // release SDA   -> pulled HIGH by resistor
#define I2C_SDA_LOW()   DDRB |=  (1<<I2C_SDA) // SDA as output -> pulled LOW  by MCU
#define I2C_SCL_HIGH()  DDRB &= ~(1<<I2C_SCL) // release SCL   -> pulled HIGH by resistor
#define I2C_SCL_LOW()   DDRB |=  (1<<I2C_SCL) // SCL as output -> pulled LOW  by MCU
#define I2C_DELAY()     asm("lpm");asm("lpm") // delay 2*3 clock cycles

// I2C init function
void I2C_init(void) {
  DDRB  &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // pins as input (HIGH-Z) -> lines released
  PORTB &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // should be LOW when as ouput
}

// I2C transmit one data byte to the slave, ignore ACK bit, no clock stretching allowed
void I2C_write(uint8_t data) {
  I2C_DELAY();                            // delay 6 clock cycles
  for(uint8_t i = 8; i; i--, data<<=1) {  // transmit 8 bits, MSB first
    (data & 0x80) ? (I2C_SDA_HIGH()) : (I2C_SDA_LOW());  // SDA HIGH if bit is 1
    I2C_DELAY();                          // delay 6 clock cycles
    I2C_SCL_HIGH();                       // clock HIGH -> slave reads the bit
    I2C_DELAY();                          // delay 6 clock cycles
    I2C_SCL_LOW();                        // clock LOW again
  }
  I2C_DELAY();                            // delay 6 clock cycles
  I2C_DELAY();                            // delay 6 clock cycles
  I2C_SDA_HIGH();                         // release SDA for ACK bit of slave
  I2C_SCL_HIGH();                         // 9th clock pulse is for the ACK bit
  I2C_DELAY();                            // delay 6 clock cycles
  I2C_SCL_LOW();                          // but ACK bit is ignored
}

// I2C start transmission
void I2C_start(uint8_t addr) {
  I2C_SDA_LOW();                          // start condition: SDA goes LOW first
  I2C_SCL_LOW();                          // start condition: SCL goes LOW second
  I2C_write(addr);                        // send slave address
}

// I2C stop transmission
void I2C_stop(void) {
  I2C_SDA_LOW();                          // prepare SDA for LOW to HIGH transition
  I2C_SCL_HIGH();                         // stop condition: SCL goes HIGH first
  I2C_SDA_HIGH();                         // stop condition: SDA goes HIGH second
}

// -----------------------------------------------------------------------------
// OLED Implementation
// -----------------------------------------------------------------------------

// OLED definitions
#define OLED_ADDR       0x78    // OLED write address
#define OLED_CMD_MODE   0x00    // set command mode
#define OLED_DAT_MODE   0x40    // set data mode
#define OLED_INIT_LEN   24      // length of OLED init command array

// OLED character set stored in program memory
const uint8_t OLED_FONT[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 
  0x14, 0x7F, 0x14, 0x7F, 0x14, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62, 
  0x36, 0x49, 0x56, 0x20, 0x50, 0x00, 0x08, 0x07, 0x03, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00, 
  0x00, 0x41, 0x22, 0x1C, 0x00, 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x08, 0x08, 0x3E, 0x08, 0x08, 
  0x00, 0x80, 0x70, 0x30, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x60, 0x60, 0x00, 
  0x20, 0x10, 0x08, 0x04, 0x02, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00, 
  0x72, 0x49, 0x49, 0x49, 0x46, 0x21, 0x41, 0x49, 0x4D, 0x33, 0x18, 0x14, 0x12, 0x7F, 0x10, 
  0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x41, 0x21, 0x11, 0x09, 0x07, 
  0x36, 0x49, 0x49, 0x49, 0x36, 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x14, 0x00, 0x00, 
  0x00, 0x40, 0x34, 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x41, 0x14, 0x14, 0x14, 0x14, 0x14, 
  0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x59, 0x09, 0x06, 0x3E, 0x41, 0x5D, 0x59, 0x4E, 
  0x7C, 0x12, 0x11, 0x12, 0x7C, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E, 0x41, 0x41, 0x41, 0x22, 
  0x7F, 0x41, 0x41, 0x41, 0x3E, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x7F, 0x09, 0x09, 0x09, 0x01, 
  0x3E, 0x41, 0x41, 0x51, 0x73, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00, 
  0x20, 0x40, 0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x7F, 0x40, 0x40, 0x40, 0x40, 
  0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E, 0x41, 0x41, 0x41, 0x3E, 
  0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x7F, 0x09, 0x19, 0x29, 0x46, 
  0x26, 0x49, 0x49, 0x49, 0x32, 0x03, 0x01, 0x7F, 0x01, 0x03, 0x3F, 0x40, 0x40, 0x40, 0x3F, 
  0x1F, 0x20, 0x40, 0x20, 0x1F, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x63, 0x14, 0x08, 0x14, 0x63, 
  0x03, 0x04, 0x78, 0x04, 0x03, 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00, 0x7F, 0x41, 0x41, 0x41, 
  0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x41, 0x41, 0x41, 0x7F, 0x04, 0x02, 0x01, 0x02, 0x04, 
  0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x03, 0x07, 0x08, 0x00, 0x20, 0x54, 0x54, 0x78, 0x40, 
  0x7F, 0x28, 0x44, 0x44, 0x38, 0x38, 0x44, 0x44, 0x44, 0x28, 0x38, 0x44, 0x44, 0x28, 0x7F, 
  0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x08, 0x7E, 0x09, 0x02, 0x18, 0xA4, 0xA4, 0x9C, 0x78, 
  0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x20, 0x40, 0x40, 0x3D, 0x00, 
  0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x7C, 0x04, 0x78, 0x04, 0x78, 
  0x7C, 0x08, 0x04, 0x04, 0x78, 0x38, 0x44, 0x44, 0x44, 0x38, 0xFC, 0x18, 0x24, 0x24, 0x18, 
  0x18, 0x24, 0x24, 0x18, 0xFC, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x48, 0x54, 0x54, 0x54, 0x24, 
  0x04, 0x04, 0x3F, 0x44, 0x24, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x1C, 0x20, 0x40, 0x20, 0x1C, 
  0x3C, 0x40, 0x30, 0x40, 0x3C, 0x44, 0x28, 0x10, 0x28, 0x44, 0x4C, 0x90, 0x90, 0x90, 0x7C, 
  0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 
  0x00, 0x41, 0x36, 0x08, 0x00, 0x02, 0x01, 0x02, 0x04, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// OLED initialisation sequence
const uint8_t OLED_INIT_CMD[] PROGMEM = {
  0xAE,             // display off
  0xD5, 0x80,       // set display clock
  0xA8, 0x3F,       // set multiplex ratio  
  0xD3, 0x00,       // set display offset  
  0x40,             // set start line to zero
  0x8D, 0x14,       // set DC-DC enable  
  0x20, 0x02,       // set page addressing mode
  0xC8, 0xA1,       // flip screen
  0xDA, 0x12,       // set com pins
  0x81, 0x7F,       // set contrast  
  0xD9, 0xF1,       // set pre charge 
  0xDB, 0x40,       // set vcom detect  
  0xA6,             // set normal (0xA7=Inverse)
  0xAF              // display on
};

// OLED global variables
uint8_t Line, Column, Scroll;

// OLED init function
void OLED_init(void) {
  I2C_init();                             // initialize I2C first
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  for (uint8_t i = 0; i < OLED_INIT_LEN; i++)
    I2C_write(pgm_read_byte(&OLED_INIT_CMD[i])); // send the command bytes
  I2C_stop();                             // stop transmission
}

// OLED set cursor to line start
void OLED_setLine(uint8_t line) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(0xB0 + line);                 // set line
  I2C_write(0x00); I2C_write(0x10);       // set column to "0"
  I2C_stop();                             // stop transmission
}

// OLED clear line
void OLED_clearLine(uint8_t line) {
  OLED_setLine(line);                     // set cursor to line start
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  for(uint8_t i=128; i; i--) I2C_write(0x00); // clear the line
  I2C_stop();                             // stop transmission
}

// OLED clear screen
void OLED_clearScreen(void) {
  for (uint8_t p=0; p<8; p++) OLED_clearLine(p);
  Line = Scroll;
  Column = 0;
  OLED_setLine((Line + Scroll) & 0x07);
}

// OLED clear the top line, then scroll the display up by one line
void OLED_scrollDisplay(void) {
  OLED_clearLine(Scroll);                 // clear line
  Scroll = (Scroll + 1) & 0x07;           // set next line
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(0xD3); I2C_write(Scroll << 3);// scroll up
  I2C_stop();                             // stop transmission
}

// OLED plot a single character
void OLED_plotChar(char c) {
  uint16_t ptr = c - 32;                  // character pointer
  ptr += ptr << 2;                        // -> ptr = (ch - 32) * 5;
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  for (uint8_t i=5 ; i; i--) I2C_write(pgm_read_byte(&OLED_FONT[ptr++]));
  I2C_write(0x00);                        // write space between characters
  I2C_stop();                             // stop transmission
}

// OLED print a character or handle control characters
void OLED_print(char c) {
  c = c & 0x7F; // Ignore top bit
  if (c >= 32) {
    OLED_plotChar(c);
    if (++Column > 20) {
      Column = 0;
      if (Line == 7) OLED_scrollDisplay(); else Line++;
      OLED_setLine((Line + Scroll) & 0x07);
    }
  }
  // new line
  else if (c == 10) {
    Column = 0;
    if (Line == 7) OLED_scrollDisplay(); else Line++;
    OLED_setLine((Line + Scroll) & 0x07);
    beep();
  }
  // carriage return
  else if (c == 13) {
    Column = 0;
    OLED_setLine((Line + Scroll) & 0x07);
  }
  // ring the bell
  else if (c == 7) beep();
}

// -----------------------------------------------------------------------------
// Buzzer Implementation
// -----------------------------------------------------------------------------

// create a short beep on the buzzer
void beep(void) {
  for (uint8_t i=255; i; i--) {
    PORTB |=  (1<<BUZZER_PIN);
    delayMicroseconds(125);
    PORTB &= ~(1<<BUZZER_PIN);
    delayMicroseconds(125);
  }
}
