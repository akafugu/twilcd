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

#ifdef MAX5160

#include "max5160.h"

#include <util/delay.h>

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

void max5160_init(void)
{
	// outpit
	sbi(CS_DDR,  CS_BIT);
	sbi(INC_DDR, INC_BIT);
	sbi(U_D_DDR, U_D_BIT);

	// disable chip
	sbi(CS_PORT, CS_BIT);
	sbi(INC_PORT, INC_BIT);
	sbi(U_D_PORT, U_D_BIT);
	_delay_ms(50);

	for (uint8_t i = 0; i < 32; i++)
		max5160_inc();
}

void max5160_inc(void)
{
	cbi(CS_PORT, CS_BIT);
	sbi(U_D_PORT, U_D_BIT);
	_delay_us(50);

	cbi(INC_PORT, INC_BIT);
	_delay_ms(1);

	// disable chip
	sbi(INC_PORT, INC_BIT);
	sbi(U_D_PORT, U_D_BIT);
	sbi(CS_PORT, CS_BIT);
	//_delay_ms(50);


	/*
	sbi(INC_PORT, INC_BIT); // set increment bit high
	sbi(U_D_PORT, U_D_BIT); // U/D bit high to increment
	_delay_us(1);

	cbi(CS_PORT, CS_BIT);   // enable chip
	_delay_us(1);
	cbi(INC_PORT, INC_BIT); // flip increment bit to increment
	_delay_us(1);
	sbi(CS_PORT, CS_BIT);   // disable chip
	*/
}

void max5160_dec(void)
{
	cbi(CS_PORT, CS_BIT);
	cbi(U_D_PORT, U_D_BIT);
	_delay_us(50);

	cbi(INC_PORT, INC_BIT);
	_delay_ms(1);

	// disable chip
	sbi(INC_PORT, INC_BIT);
	sbi(U_D_PORT, U_D_BIT);
	sbi(CS_PORT, CS_BIT);
	//_delay_ms(50);

	/*
	sbi(INC_PORT, INC_BIT); // set increment bit high
	cbi(U_D_PORT, U_D_BIT); // U/D bit low to decrement

	cbi(CS_PORT, CS_BIT);   // enable chip
	_delay_us(1);
	cbi(INC_PORT, INC_BIT); // flip increment bit to decrement
	_delay_us(1);
	sbi(CS_PORT, CS_BIT);   // disable chip
	*/
}

#endif // MAX5160