/*
 * TWI LCD Character Display
 * (C) 2011 Akafugu Corporation
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */

#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include <stdlib.h>

#include "lcd.h"
#include "usiTwiSlave.h"

#include "max5160.h"
#include "mcp4013.h"

#define FIRMWARE_REVISION 2
#define SLAVE_ADDRESS 0x32

#ifndef DEFAULT_BRIGHTNESS
#define DEFAULT_BRIGHTNESS 100
#endif // DEFAULT_BRIGHTNESS

#ifndef DEFAULT_CONTRAST
#define DEFAULT_CONTRAST 40
#endif // DEFAULT_CONTRAST

uint8_t EEMEM b_slave_address = SLAVE_ADDRESS;
uint8_t EEMEM b_brightness = DEFAULT_BRIGHTNESS;
uint8_t EEMEM b_contrast = DEFAULT_CONTRAST;

uint8_t displaycontrol;
uint8_t displaymode;

/*
** constant definitions
*/
static const PROGMEM unsigned char copyRightChar[] =
{
	0x07, 0x08, 0x13, 0x14, 0x14, 0x13, 0x08, 0x07,
	0x00, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x00
};

// PB2 for PWM backlight

void backlight_init(void)
{
	// Set pin to output
	DDRB |= _BV(PB2);

	// Get stored brightness
	uint8_t stored_brightness = eeprom_read_byte(&b_brightness);
	OCR0A = stored_brightness;

	// fast PWM, set OC0A (boost output pin) on match
	TCCR0A = _BV(WGM00) | _BV(WGM01);  

	// Use the fastest clock
	TCCR0B = _BV(CS00);

	TCCR0A |= _BV(COM0A1);
}

void init(void)
{
	cli();	// disable interrupts

	uint8_t stored_address = eeprom_read_byte(&b_slave_address);
	// Check that stored_address is sane
	if (stored_address >= 128)
		stored_address = SLAVE_ADDRESS;
	
	usiTwiSlaveInit(stored_address);
	//set_brightness(eeprom_read_byte(&b_brightness));

	backlight_init();
	
#ifdef MAX5160
	max5160_init();
#endif
	
	mcp4013_init();
	
	sei(); // enable interrupts 
}

void processTWI( void )
{
	uint8_t b,c,d;

	b = usiTwiReceiveByte();
	
	switch (b) {
		case 0x80: // set brightness
			c = usiTwiReceiveByte();
			OCR0A = c;
			eeprom_write_byte(&b_brightness, c);
			break;
#ifdef FEATURE_CHANGE_TWI_ADDRESS
		case 0x81: // set slave address
			c = usiTwiReceiveByte();
			if(c < 128) // Address is 7 bit
			{
				eeprom_update_byte(&b_slave_address, c);
				usiTwiSlaveInit(eeprom_read_byte(&b_slave_address));
			}
			break;
#endif
		case 0x82: // clear display
			lcd_clrscr();
			break;
		case 0x83: // set scroll mode
			break;
		case 0x84: // receive segment data
			c = usiTwiReceiveByte(); // segment data
			break;
		case 0x85: // set dots (the four bits of the second byte controls dots individually)
			c = usiTwiReceiveByte();
			break;
/*#ifdef FEATURE_SET_TIME
		case 0x87: // display time (hh:mm with seconds controlling middle dot)
			//set_time(usiTwiReceiveByte(), usiTwiReceiveByte(), usiTwiReceiveByte());
			break;
#endif // FEATURE_SET_TIME*/
		case 0x88: // display integer
			{
				/*
				uint8_t i1 = usiTwiReceiveByte();
				uint8_t i2 = usiTwiReceiveByte();

				uint16_t i = (i2 << 8) + i1;
				set_number(i);
				*/
			}
			break;
		case 0x89: // set position (only valid for ROTATE mode)
			c = usiTwiReceiveByte();
			lcd_gotoxy(c,0);
			break;
		case 0x8a: // get firmware revision
			usiTwiTransmitByte(FIRMWARE_REVISION);
			break;
		case 0x8b: // get number of digits
			usiTwiTransmitByte(16);
			break;
		case 0x90: // Show address
			break;
		case 0x91:
		case 0xa0: // home
			lcd_home();
			break;
		case 0x92:
		case 0xa1: // gotoxy
			c = usiTwiReceiveByte();
			d = usiTwiReceiveByte();
			lcd_gotoxy(c,d);
			break;
		/* Low level commands */
		case 0x93: // Display off
			displaycontrol &= ~LCD_DISPLAYON;
			lcd_command(displaycontrol);
			break;
		case 0x94: // Display on
			displaycontrol |= LCD_DISPLAYON;
			lcd_command(displaycontrol);
			break;
		case 0x95: // Cursor off
			displaycontrol &= ~LCD_CURSORON;
			lcd_command(displaycontrol);
			break;
		case 0x96: // Cursor on
			displaycontrol |= LCD_CURSORON;
			lcd_command(displaycontrol);
			break;
		case 0x97: // Blink off
			displaycontrol &= ~LCD_BLINKON;
			lcd_command(displaycontrol);
			break;
		case 0x98: // Blink on
			displaycontrol |= LCD_BLINKON;
			lcd_command(displaycontrol);
			break;
		case 0x99: // scroll display left
			lcd_command(LCD_MOVE_DISP_LEFT);
			break;
		case 0x9a: //scroll display right
			lcd_command(LCD_MOVE_DISP_RIGHT);
			break;
		case 0x9b: // left to right mode
			displaymode |= LCD_ENTRYLEFT;
			lcd_command(displaymode);
			break;
		case 0x9c: // right to left mode
			displaymode &= ~LCD_ENTRYLEFT;
			lcd_command(displaymode);
			break;
		case 0x9d: // autoscroll on
			displaymode |= LCD_ENTRYSHIFTINCREMENT;
			lcd_command(displaymode);
			break;
		case 0x9e: // autoscroll off
			displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
			lcd_command(displaymode);
			break;
		case 0x9f: // create custom character
			c = usiTwiReceiveByte() & 0x7; // locations are from 0~7
			lcd_command(_BV(LCD_CGRAM) | (c<<3)); // set CG RAM start address

			for(uint8_t i = 0; i < 8; i++) {
				lcd_data(usiTwiReceiveByte());
			}
			break;
			

		/* LCD commands */
		
		case 0xa2: // putc
			c = usiTwiReceiveByte();
			lcd_putc(c);
			break;
		case 0xa3: // command
			c = usiTwiReceiveByte();
			lcd_command(c);
			break;
		case 0xa4: // Character
			c = usiTwiReceiveByte();
			lcd_putc(c);
			break;
		case 0xa5: // Send raw data
			c = usiTwiReceiveByte();
			lcd_data(c);
			break;
		case 0xd0: // Set new contrast
			c = usiTwiReceiveByte();
			mcp4013_set(c);
			eeprom_write_byte(&b_contrast, c);
			break;
		case 0xd1: // Test contrast
			c = usiTwiReceiveByte();		
			mcp4013_set(c);
			break;
		case 0xfb: // Set line wrap
			lcd_linewrap(usiTwiReceiveByte());
			break;
		case 0xfc: // Set KS0073 controller
			lcd_ks0073(usiTwiReceiveByte());
			break;
		case 0xfd: // Set row/col
			lcd_setup(usiTwiReceiveByte(),usiTwiReceiveByte());
			break;
		case 0xfe: // reset to known state
			flushTwiBuffers();
			lcd_clrscr();
			backlight_init();
			mcp4013_set(eeprom_read_byte(&b_contrast));
			break;
		case 0xff: // flush the bus
			break;
		default:
			if (b >= 0x80) break; // anything above 0x80 is considered a reserved command and is ignored
			else if (b == (uint8_t)'\r') break; // Don't really care about carrier returns
			else if (b >= 0 && b <= 9) lcd_putc(b + '0');
			else lcd_putc(b);
			break;
	}
}

void main(void) __attribute__ ((noreturn));

void main(void)
{
    char buffer[7];
    //int  num=134;
    //unsigned char i;

	// initialize display control: display on, cursor off, blink off
	displaycontrol = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	// initialize display mode: Left to right, right justify
	displaymode = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    
    /* initialize display, cursor off */
    lcd_init(displaycontrol);
    
    init();
    lcd_clrscr();
    
#ifdef MAX5160
    for (i = 0; i < 29; i++) {
	    max5160_dec();
    }
#endif

    mcp4013_set(eeprom_read_byte(&b_contrast));
    
#ifdef FEATURE_SHOW_ADDRESS_ON_STARTUP
	uint8_t counter = 0;
	
#define MAX_COUNTER	200

	while(!usiTwiDataInReceiveBuffer() && counter <= MAX_COUNTER)
	{
		counter++;
		_delay_ms(10);

		if(counter == MAX_COUNTER)
		{
			// fixme: What if someone sets brighness to 0 and then changes the address
			// should reset brightness here if it is too dim to see.

			uint8_t address = eeprom_read_byte(&b_slave_address);
			itoa(address , buffer, 10);
			lcd_clrscr();
			lcd_puts("TWI/I2C Adr: ");
			lcd_puts(buffer);
		}
	}
#endif //FEATURE_SHOW_ADDRESS_ON_STARTUP
	
	while (1) {
		while (usiTwiDataInReceiveBuffer())	{ // process I2C command
			processTWI();
		}
	}


    /*
    while (1) {
	    // go to min
	    for (i = 0; i < 32; i++) {
		    itoa( i , buffer, 10);
		    lcd_clrscr();
		    lcd_puts("Decrement: ");
		    lcd_puts(buffer);

		    max5160_dec();
		    _delay_ms(500);
	    }

	    // go to max
	    for (i = 0; i < 32; i++) {
		    itoa( i , buffer, 10);
		    lcd_clrscr();
		    lcd_puts("Increment: ");
		    lcd_puts(buffer);

		    max5160_inc();
		    _delay_ms(500);
	    }
    }
    */
}