/*
 * TWI 4-digit 7-segment display
 * (C) 2011 Akafugu
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

#include "mcp4013.h"

#include <util/delay.h>

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

void mcp4013_init(void)
{
	// outpit
	sbi(CS_DDR,  CS_BIT);
	sbi(U_D_DDR, U_D_BIT);

	// disable chip
	sbi(CS_PORT, CS_BIT);
	sbi(U_D_PORT, U_D_BIT);
	_delay_ms(2);

	for (uint8_t i = 0; i < 64; i++)
		mcp4013_inc();
}

void mcp4013_inc(void)
{
	sbi(U_D_PORT, U_D_BIT);
	_delay_us(10);
	cbi(CS_PORT, CS_BIT);
	_delay_us(2);

	cbi(U_D_PORT, U_D_BIT);
	_delay_us(2);
	sbi(U_D_PORT, U_D_BIT); // Change value on raising-edge
	_delay_us(2);

	// disable chip
	sbi(INC_PORT, INC_BIT);
	sbi(U_D_PORT, U_D_BIT);
	sbi(CS_PORT, CS_BIT);
	//_delay_ms(50);
}

void mcp4013_dec(void)
{
	cbi(U_D_PORT, U_D_BIT);
	_delay_us(5);
	cbi(CS_PORT, CS_BIT);
	_delay_us(2);

	sbi(U_D_PORT, U_D_BIT); // Change value on raising-edge
	_delay_us(2);

	// disable chip
	sbi(INC_PORT, INC_BIT);
	sbi(U_D_PORT, U_D_BIT);
	sbi(CS_PORT, CS_BIT);
	//_delay_ms(50);
}

void mcp4013_set(uint8_t val)
{
	for (uint8_t i = 0; i < 64; i++)
			mcp4013_dec();
	
	val = val >> 2; // 0 - 63
	for (uint8_t i = 0; i < val; i++)
				mcp4013_inc();
}
