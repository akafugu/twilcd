/*
 * Akafugu TWI LCD Driver
 * (C) 2013 Akafugu Corporation
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
#include <string.h>
#include <util/delay.h>

#include "twi.h"
#include "twi-lcd.h"

void lcd_init(uint8_t addr, uint8_t cols, uint8_t lines)
{
	lcd_reset(addr);
	twi_begin_transmission(addr);
	twi_send_byte(0xfd); // Setup the display
	twi_send_byte(cols); // number of cols
	twi_send_byte(lines); // number of lines
	twi_end_transmission();
	_delay_ms(5);
}

void lcd_reset(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xFF); // sending some NOP to clear input buffer on display
	twi_send_byte(0xFF); // sending some NOP to clear input buffer on display
	twi_send_byte(0xFF); // sending some NOP to clear input buffer on display
	twi_send_byte(0xFF); // sending some NOP to clear input buffer on display
	twi_send_byte(0xFE); // clear
	twi_end_transmission();	
}

void lcd_change_address(uint8_t cur_addr, uint8_t new_addr)
{
	twi_begin_transmission(cur_addr);
	twi_send_byte(0x81); // change address
	twi_send_byte(new_addr);
	twi_end_transmission();
	_delay_ms(5);
}

void lcd_set_brightness(uint8_t addr, uint8_t brightness)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xd3); // set brightness
	twi_send_byte(brightness);
	twi_end_transmission();
}

void lcd_save_brightness(uint8_t addr, uint8_t brightness)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x80); // save brightness
	twi_send_byte(brightness);
	twi_end_transmission();
	_delay_ms(5);
}

void lcd_set_contrast(uint8_t addr, uint8_t contrast)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xd1); // set contrast
	twi_send_byte(contrast);
	twi_end_transmission();
}

void lcd_save_contrast(uint8_t addr, uint8_t contrast)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xd0); // save contrast
	twi_send_byte(contrast);
	twi_end_transmission();
	_delay_ms(5);
}

void lcd_clear(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x82); // clear
	twi_end_transmission();	
}

void lcd_home(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x91); // home
	twi_end_transmission();	
}

void lcd_create_char(uint8_t addr, uint8_t location, uint8_t* charmap)
{
	_delay_ms(5);
	twi_begin_transmission(addr);
	twi_send_byte(0x9f); // create custom character
	twi_send_byte(location&0x7); // we only have 8 locations 0-7
	
	for (int i=0; i<8; i++)
		twi_send_byte(charmap[i]);
	
	twi_end_transmission();	
	_delay_ms(25);
}


void lcd_display_on(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x94);
	twi_end_transmission();	
}

void lcd_display_off(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x93);
	twi_end_transmission();	
}

void lcd_cursor_on(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x96);
	twi_end_transmission();	
}

void lcd_cursor_off(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x95);
	twi_end_transmission();	
}

void lcd_blink_on(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x98);
	twi_end_transmission();	
}

void lcd_blink_off(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x97);
	twi_end_transmission();	
}

void lcd_scroll_on(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x9d);
	twi_end_transmission();	
}

void lcd_scroll_off(uint8_t addr)
{
	twi_begin_transmission(addr);
	twi_send_byte(0x9e);
	twi_end_transmission();	
}


void lcd_set_position(uint8_t addr, uint8_t col, uint8_t row)
{	
	twi_begin_transmission(addr);
	twi_send_byte(0x92); // set position
	twi_send_byte(col);
	twi_send_byte(row);
	twi_end_transmission();
}

void lcd_write_char(uint8_t addr, char val)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xa4);
	twi_send_byte(val);
	twi_end_transmission();
}

void lcd_write_str(uint8_t addr, char* val)
{	
	for (uint8_t i = 0; i < strlen(val); i++) {
		lcd_write_char(addr, val[i]);
	}
}

int lcd_get_firmware_revision(uint8_t addr)
{
  twi_begin_transmission(addr);
  twi_send_byte(0x8a); // get firmware revision
  twi_end_transmission();

  twi_request_from(addr, 1);
  return twi_receive();
}

// Low level commands

void lcd_raw_command(uint8_t addr, uint8_t command)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xa3);
	twi_send_byte(command);
	twi_end_transmission();
}

void lcd_raw_data(uint8_t addr, uint8_t data)
{
	twi_begin_transmission(addr);
	twi_send_byte(0xa5);
	twi_send_byte(data);
	twi_end_transmission();
}

