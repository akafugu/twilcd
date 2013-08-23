/*
 * Akafugu TWI LCD: Test
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
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>

#include "twi-lcd.h"

#define SLAVE_ADDR 50

void main(void) __attribute__ ((noreturn));


void test_time(void)
{
	static uint8_t hour = 0, min = 0, sec = 0;
	//rtc_get_time(&hour, &min, &sec);
	
	sec++;
	if (sec >= 60) { sec = 0; min++; }
	if (min >= 60) { min = 0; hour++; }
	if (hour >= 24) { hour = 0; min = 0; sec = 0; }
	
	lcd_set_position(SLAVE_ADDR, 8, 0);
	lcd_write_char(SLAVE_ADDR, ((hour/10) + '0'));
	lcd_write_char(SLAVE_ADDR, ((hour%10) + '0'));
	lcd_write_char(SLAVE_ADDR, ':');
	lcd_write_char(SLAVE_ADDR, ((min/10) + '0'));
	lcd_write_char(SLAVE_ADDR, ((min%10) + '0'));
	lcd_write_char(SLAVE_ADDR, ':');
	lcd_write_char(SLAVE_ADDR, ((sec/10) + '0'));
	lcd_write_char(SLAVE_ADDR, ((sec%10) + '0'));
}

void main(void)
{
	twi_init_master();
	sei();

	_delay_ms(500); // Make the module stabilize

#if 0
	// Code for testing 20x4 display
	lcd_init(SLAVE_ADDR, 20, 4);
	lcd_set_brightness(SLAVE_ADDR, 255);

	lcd_write_str(SLAVE_ADDR, "1234");
	lcd_set_position(SLAVE_ADDR, 1, 1);
	lcd_write_str(SLAVE_ADDR, "5678");
	lcd_set_position(SLAVE_ADDR, 2, 2);
	lcd_write_str(SLAVE_ADDR, "abcd");
	lcd_set_position(SLAVE_ADDR, 3, 3);
	lcd_write_str(SLAVE_ADDR, "ABCD ");

#else
	lcd_init(SLAVE_ADDR, 16, 2);
	lcd_set_brightness(SLAVE_ADDR, 255);

	// Write firmware
	char ver[] = "Firmware: ";
	lcd_write_str(SLAVE_ADDR, ver);
	int v = lcd_get_firmware_revision(SLAVE_ADDR);
	if((v/10) != 0)
		lcd_write_char(SLAVE_ADDR, (v/10) + '0');
	lcd_write_char(SLAVE_ADDR, (v%10) + '0');
	_delay_ms(1000);
	lcd_clear(SLAVE_ADDR);
	lcd_home(SLAVE_ADDR);

	// test write string
	lcd_write_str(SLAVE_ADDR, "1234");
	_delay_ms(500);
	lcd_write_str(SLAVE_ADDR, "ABCD");
	_delay_ms(500);


	// test write string
	char buf[] = "this is a long string";
	lcd_set_position(SLAVE_ADDR, 0, 1);
	lcd_write_str(SLAVE_ADDR, buf);
	_delay_ms(500);
	
	// test set position
	lcd_home(SLAVE_ADDR);
	lcd_clear(SLAVE_ADDR);
	lcd_write_char(SLAVE_ADDR, 'a');
	lcd_set_position(SLAVE_ADDR, 3, 1);
	lcd_write_char(SLAVE_ADDR, 'b');
	_delay_ms(2000);

	lcd_clear(SLAVE_ADDR);
#endif

	// Show clock
	lcd_home(SLAVE_ADDR);
	while(1) {
		test_time();
		_delay_ms(1000);
	}

}
