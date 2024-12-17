/* 
 * twi_lcd.c
 *
 * Created: 2024-12-14
 *  Author: Dong-ha Kim
*/

#include "twi_master_lcd_HW-061.h"

#include <avr/io.h>
#include <avr/iom128.h>
#include <util/delay.h>

//Initialize TWI data.
//If you called lcd_init, you don't need to call it again because lcd_init calls it.
void twi_init()
{
  PORTD |= 0x03;

  // SCL frequency = 16 000 000 / (16 + 2 * freq * 4 ^ (TWSR) )
  TWBR = 0x48;
  TWSR = 0x00;
}

//Start TWI connection.
void twi_start()
{
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));

  TWDR = ADDR_SLAVE << 1;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));

}

//Send data through TWI
void twi_write(uint8_t data)
{
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);

  while (!(TWCR & (1 << TWINT)));
}

//Send multibyte(multiple call of twi_write) through TWI
void twi_write_multibyte(uint8_t* data, uint8_t length)
{
  twi_start();

  for(uint8_t i = 0; i < length; i++)
  {
    twi_write(data[i]);
    _delay_us(100);
  }

  twi_stop();
}

//Send TWSTO signal to stop data exchange.
void twi_stop()
{
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

//Initialize both TWI and LCD.
//frequency = TWI_FREQ_100K or TWI_FREQ_400K
//line = LCD_LINE_2LINE or LCD_LINE_1LINE
//size = LCD_FONT_5x10 or LCD_FONT_5x8
void lcd_init(uint8_t frequency, uint8_t line, uint8_t size)
{
  _delay_ms(200);
  twi_init();
  _delay_ms(200);

  lcd_cmd(0x30);
  _delay_ms(10);
  lcd_cmd(0x30);
  _delay_us(200);
  lcd_cmd(0x30);
  _delay_ms(200);

  lcd_cmd(0x20);

  lcd_function_set(line, size);

  lcd_cmd(LCD_CMD_CLEAR);
  lcd_cmd(LCD_CMD_DISPLAY_OFF);
  lcd_cmd(LCD_CMD_ENTRY_INC_NOSHIFT);
  lcd_cmd(LCD_CMD_CURSOR_UNDERLINE);
  lcd_cmd(LCD_CMD_RETURN_HOME);
}

//LCD command call
void lcd_cmd(uint8_t data)
{
  uint8_t cmd[4];

  cmd[0] = (data & 0xF0) | LCD_BLA | LCD_E;
  cmd[1] = (data & 0xF0) | LCD_BLA;
  data <<= 4;
  cmd[2] = (data & 0xF0) | LCD_BLA | LCD_E;
  cmd[3] = (data & 0xF0) | LCD_BLA;

  twi_write_multibyte(cmd, 4);

  _delay_ms(3);
}

//'LCD_LINE_2 AND LCD_FONT_5x10' is not working because HD44780 does not support it.
//Every other combination works.
void lcd_function_set(uint8_t line, uint8_t size)
{
  uint8_t data = 0x20;

  data |= line ? 0x8 : 0;
  data |= size ? 0x4 : 0;

  lcd_cmd(data);
}

//Set position of cursor
void lcd_set_position(uint8_t x, uint8_t y)
{
  uint8_t data = 0x80;
  data |= (x % 2) << 6;
  data |= (y % 16);

  lcd_cmd(data);
}

//Send 1 character to LCD
void lcd_char(uint8_t character)
{
  uint8_t cmd[4];

  cmd[0] = (character & 0xF0) | LCD_BLA | LCD_E | LCD_RS;
  cmd[1] = (character & 0xF0) | LCD_BLA | LCD_RS;
  character <<= 4;
  cmd[2] = (character & 0xF0) | LCD_BLA | LCD_E | LCD_RS;
  cmd[3] = (character & 0xF0) | LCD_BLA | LCD_RS;

  twi_write_multibyte(cmd, 4);
}

//Send string(multiple call of lcd_char) to LCD
void lcd_str(char* str)
{
  while(*str != '\0')
  {
    lcd_char(*str++);
  }
}