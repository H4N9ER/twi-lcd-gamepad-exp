/* 
    HID Gamepad using ATmega128
    Author: Dong-ha Kim

    LCD:
      PDO: Y
      PD1: O
      VCC: B
      GND: G

    PCF8574:
      LCD is working on 4-bit mode due to PCF8574
        P7: D7
        P6: D6
        P5: D5
        P4: D4
        P3: BT
        P2: E
        P1: RW
        P0: RS

    F_CPU = 16 000 000


    -HOW TO TWI(I2C)

    

    TWBR: TWI Bit Rate Register
      Used in calculating SCL frequency
      frequency = F_CPU / ( 16 + 2 * TWBR * ( 4 ^ Prescaler(=TWBR[1:0]) ) )
        TWBRn[n = 7:0]: nth bit of TWBR

    TWCR: TWI Control Register
      Used to control the operation of TWI
        TWINT(TWCR[7]): Interrupt Flag
        TWEA(TWCR[6]) : Enable ACK
        TWSTA(TWCR[5]): START Condition
        TWSTO(TWCR[4]): STOP Condition
        TWWC(TWCR[3]) : Write Collision Flag - readonly
        TWEN(TWCR[2]) : Enable TWI
        RESERVED(TWCR[1])
        TWIE(TWCR[0]) : Interrupt Enable

    TWSR: TWI Status Register
      Check table 26-3 in ATmega128A datasheet
        TWSn[n = 7:3] : current status data - readonly
        RESERVED(TWSR[2])
        TWPSn[n = 1:0]: Bit rate prescaler

    TWDR: TWI Data Register
      Contains the next byte to be transmitted
        TWDn[n = 7:0] : nth bit of TWDR

    TWAR - TWI Address Register
      Contains slave address and general call recognition flag
        TWAn[n = 7:1] : 7-bit TWI address
        TWGCE(TWAR[0]): enables the recognition of a general call(TWAn = 0b000 0000) given over TWI bus

  
  PORTA:  1   3   5   7     
          B1  B3  B5  B7    BTN 0   1   2   3     
          B0  B2  B4  B6        4   5   6   7
          0   2   4   6


  PORTB:  1   3   5   7     
          LR  LT    
          LU  LL      
          0   2   4   6


  PORTD:  1   3   5   7     
          SDA B10 B9  B8    BTN S0  S1  S2    
          SCL S0  S1  S2    BTN 8   9   10 
          0   2   4   6
    
*/
#include <avr/io.h>
#include <avr/iom128.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "twi_master_lcd_HW-061.h"

#define BTN0          0
#define BTN1          1
#define BTN2          2
#define BTN3          3
#define BTN4          4
#define BTN5          5
#define BTN6          6
#define BTN7          7
#define BTN8          8
#define BTN9          9
#define BTN10         10   
#define LEVER_DOWN    11
#define LEVER_RIGHT   12
#define LEVER_LEFT    13
#define LEVER_UP      14


typedef struct profile
{
  char profile_name[13];
  uint16_t profile_map[15];
} profile;


volatile uint8_t buffer0;
volatile uint8_t buffer1;

//buffer[0] = BTN7:0
//buffer[1] = 0 LEVER (TLRU) BTN10:8

const profile normal = 
{
  "DEFAULT",
  {
    BTN0, BTN1, BTN2, BTN3,
    BTN4, BTN5, BTN6, BTN7,
    BTN8, BTN9, BTN10, LEVER_DOWN,
    LEVER_RIGHT, LEVER_LEFT, LEVER_UP
  }
};

const profile lever_inverted = 
{
  "INV LEVER",
  {
    BTN0, BTN1, BTN2, BTN3,
    BTN4, BTN5, BTN6, BTN7,
    BTN8, BTN9, BTN10, LEVER_UP,
    LEVER_LEFT, LEVER_RIGHT, LEVER_DOWN
  }
};

const profile button_vflip =
{
  "VFLIP BUTTON",
  {
    BTN4, BTN5, BTN6, BTN7,
    BTN0, BTN1, BTN2, BTN3,
    BTN8, BTN9, BTN10, LEVER_DOWN,
    LEVER_RIGHT, LEVER_LEFT, LEVER_UP
  }

};

const profile* profiles[3] = {&normal, &lever_inverted, &button_vflip};
uint8_t current_profile = 0;
uint8_t profile_size = 3;

void init()
{
  DDRA = 0x00;
  DDRB = 0x00;
  DDRD = 0x00;
  PORTA = 0xFF;
  PORTB = 0x0F;
  PORTD = 0xFF;

  EICRA = (1 << ISC21);
  EIMSK = (1 << INT2);
  
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 624;
  TIMSK |= (1 << OCIE1A);

  UBRR0H = 0;
  UBRR0L = 3; //Baud Rate 250KHz
  UCSR0B = (1 << TXEN);
  UCSR0C = (1 << UPM01) | (1 << UCSZ01) | (1 << UCSZ00);

  _delay_ms(1000);

  lcd_init(TWI_FREQ_100K ,LCD_LINE_2, LCD_FONT_5X8);
  lcd_cmd(LCD_CMD_CLEAR);
  lcd_str("INITIALIZING");
  _delay_ms(1000);

  lcd_cmd(LCD_CMD_CLEAR);
  lcd_set_position(0, 0);
  lcd_str("P: ");
  lcd_str(profiles[current_profile]->profile_name);
  lcd_set_position(1, 0);
  lcd_str("READY");

  sei();
}

void changeprofile(uint8_t n)
{
  if(n > 2) current_profile = 2;
  else current_profile = n;
}

void putchar0(uint8_t c)
{
  while(!(UCSR0A & 0x20));
  UDR0 = c;
}

void puts0()
{
  putchar0(buffer1);
  putchar0(buffer0);
  return;
}

ISR(INT2_vect)
{
  lcd_cmd(LCD_CMD_CLEAR);
  lcd_set_position(1, 0);
  lcd_str("CHANGING");
  uint8_t cursor = current_profile;
  while(true)
  {
    lcd_set_position(0, 0);
    lcd_str("                ");

    lcd_set_position(0, 0);
    lcd_str("< ");
    lcd_str(profiles[cursor]->profile_name);
    lcd_set_position(0, 14);
    lcd_str(" >");
    
    _delay_ms(200);
    while((PIND & 0x54) == 0x54);

    if(!(PIND & 0x04))
    {
      current_profile = cursor;
      lcd_cmd(LCD_CMD_CLEAR);
      lcd_set_position(0, 0);
      lcd_str("P: ");
      lcd_str(profiles[current_profile]->profile_name);
      lcd_set_position(1, 0);
      lcd_str("READY");
      break;
    }
    else if(!(PIND & 0x10))
      cursor = (cursor + 1) % 3;
    else if(!(PIND & 0x40))
      cursor = (cursor - 1 < 0) ? 2 : (cursor - 1);
  }
  for(int k = 0; k < 1024; k++);
}

ISR(TIMER1_COMPA_vect)
{
  buffer0 = 0;
  buffer1 = 0;

  for(int i = 0; i < 8; i++)
    if(PINA & (1 << i))
      buffer0 |= (1 << (profiles[current_profile]->profile_map[i]));

  for(int i = 0; i < 3; i++)
    if(!(PIND & (1 << (i * 2 + 3) )))
      buffer1 |= (1 << (profiles[current_profile]->profile_map[8 + i] - 8));
  
  for(int i = 3; i < 7; i++)
    if(!(PINB & (1 << (i - 3))))
      buffer1 |= (1 << (profiles[current_profile]->profile_map[8 + i] - 8));

  puts0();
}

int main()
{
  init();

  while(true)
  {  }
}