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

#ifndef MCP_4013__
#define MCP_4013__

#include <avr/io.h>

#define CS_DDR  DDRB
#define CS_PORT PORTB
#define CS_BIT  PB4

#define U_D_DDR  DDRB
#define U_D_PORT PORTB
#define U_D_BIT  PB1

void mcp4013_init(void);
void mcp4013_inc(void);
void mcp4013_dec(void);
void mcp4013_set(uint8_t);

#endif // MCP_4013__
