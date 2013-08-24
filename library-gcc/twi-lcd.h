/*
 * Akafugu TWI LCD Library
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

#ifndef TWI_LCD_H
#define TWI_LCD_H

#include <stdbool.h>
#include "twi.h"

void lcd_init(uint8_t addr, uint8_t cols, uint8_t lines);
void lcd_reset(uint8_t addr);

void clcd_hange_address(uint8_t cur_addr, uint8_t new_addr);
void lcd_set_brightness(uint8_t addr, uint8_t brightness);
void lcd_save_brightness(uint8_t addr, uint8_t brightness);
void lcd_set_contrast(uint8_t addr, uint8_t brightness);
void lcd_save_contrast(uint8_t addr, uint8_t brightness);
void lcd_clear(uint8_t addr);
void lcd_home(uint8_t addr);

void lcd_create_char(uint8_t addr, uint8_t location, uint8_t* charmap);

void lcd_display_on(uint8_t addr);
void lcd_display_off(uint8_t addr);
void lcd_cursor_on(uint8_t addr);
void lcd_cursor_off(uint8_t addr);
void lcd_blink_on(uint8_t addr);
void lcd_blink_off(uint8_t addr);
void lcd_scroll_on(uint8_t addr);
void lcd_scroll_off(uint8_t addr);

void lcd_set_position(uint8_t addr, uint8_t col, uint8_t row);

void lcd_write_char(uint8_t addr, char val);
void lcd_write_str(uint8_t addr, char* val);

int lcd_get_firmware_revision(uint8_t addr);

// Low level commands
void lcd_raw_command(uint8_t addr, uint8_t command);
void lcd_raw_data(uint8_t addr, uint8_t data);

#endif // TWI_LCD_H
