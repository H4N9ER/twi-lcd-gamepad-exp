/* 
 * twi_lcd.h
 *
 * Created: 2024-12-14
 *  Author: Dong-ha Kim
*/
#ifndef TWI_LCD_H
#define TWI_LCD_H

//Check your I2C slave address before proceed.
#define ADDR_SLAVE  0x27

#define LCD_BLA 0x08
#define LCD_E   0x04
#define LCD_RW  0x02
#define LCD_RS  0x01

#define LCD_LINE_2 1
#define LCD_LINE_1 0

#define LCD_FONT_5X10   1
#define LCD_FONT_5X8    0

#define LCD_CMD_CLEAR               0x01
#define LCD_CMD_RETURN_HOME         0x02
#define LCD_CMD_ENTRY_DEC_NOSHIFT   0x04
#define LCD_CMD_ENTRY_DEC_SHIFT     0x05
#define LCD_CMD_ENTRY_INC_NOSHIFT   0x06
#define LCD_CMD_ENTRY_INC_SHIFT     0x07
#define LCD_CMD_DISPLAY_OFF         0x08
#define LCD_CMD_DISPLAY_ON          0x0C
#define LCD_CMD_CURSOR_OFF          0x0C
#define LCD_CMD_CURSOR_UNDERLINE    0x0E
#define LCD_CMD_CURSOR_BLINKING     0x0F
#define LCD_CMD_SHIFT_CURSOR_LEFT   0x10
#define LCD_CMD_SHIFT_CURSOR_RIGHT  0x14
#define LCD_CMD_SCROLL_LEFT         0x18
#define LCD_CMD_SCROLL_RIGHT        0x1E

#define TWI_FREQ_100K   0x48
#define TWI_FREQ_400K   0x03

#include <avr/io.h>
#include <avr/iom128.h>
#include <util/delay.h>

void twi_init();
void twi_start();
void twi_stop();
void twi_write(uint8_t data);
void twi_write_multibyte(uint8_t* data, uint8_t length);

void lcd_init(uint8_t frequency, uint8_t line, uint8_t size);
void lcd_cmd(uint8_t command);

void lcd_function_set(uint8_t line, uint8_t size);
void lcd_set_position(uint8_t x, uint8_t y);
void lcd_char(uint8_t character);
void lcd_str(char* str);

#endif /* TWI_LCD_H */