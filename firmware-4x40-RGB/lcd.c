/****************************************************************************
 Title	:   HD44780U LCD library
 Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 File:	    $Id: lcd.c,v 1.14.2.1 2006/01/29 12:16:41 peter Exp $
 Software:  AVR-GCC 3.3 
 Target:    any AVR device, memory mapped mode only for AT90S4414/8515/Mega
 
 Modified by: Akafugu Corporation 2011

 DESCRIPTION
       Basic routines for interfacing a HD44780U-based text lcd display

       Originally based on Volker Oth's lcd library,
       changed lcd_init(), added additional constants for lcd_command(),
       added 4-bit I/O mode, improved and optimized code.

       Library can be operated in memory mapped mode (LCD_IO_MODE=0) or in 
       4-bit IO port mode (LCD_IO_MODE=1). 8-bit IO port mode not supported.
       
       Memory mapped mode compatible with Kanda STK200, but supports also
       generation of R/W signal through A8 address line.

 USAGE
       See the C include lcd.h file for a description of each function
       
*****************************************************************************/
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"

/* 
** constants/macros 
*/
#define DDR(x) (*(&x - 1))      /* address of data direction register of port x */
#define PIN(x) (*(&x - 2))    /* address of input register of port x          */


#define lcd_e_delay()   __asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN);
#define lcd_e_toggle()  toggle_e()
#define lcd_e2_high()    LCD_E2_PORT  |=  _BV(LCD_E2_PIN);
#define lcd_e2_low()     LCD_E2_PORT  &= ~_BV(LCD_E2_PIN);
#define lcd_e2_toggle()  toggle_e2()
#define lcd_rw_high()   LCD_RW_PORT |=  _BV(LCD_RW_PIN)
#define lcd_rw_low()    LCD_RW_PORT &= ~_BV(LCD_RW_PIN)
#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN)

uint8_t lcd_lines = 2;              /**< number of visible lines of the display */
uint8_t lcd_disp_length = 16;       /**< visibles characters per line of the display */
uint8_t lcd_controller_ks0073 = 0;  /**< Use 0 for HD44780 controller, 1 for KS0073 controller */
uint8_t lcd_wrap_lines = 0;         /**< 0: no wrap, 1: wrap at end of visibile line */

struct lcd2CtrlMode {
	uint8_t enable:1;
	uint8_t display:1;
} __attribute__((__packed__));

volatile struct lcd2CtrlMode mode;


#define KS0073_EXTENDED_FUNCTION_REGISTER_ON  0x24   /* |0|010|0100 4-bit mode extension-bit RE = 1 */
#define KS0073_EXTENDED_FUNCTION_REGISTER_OFF 0x20   /* |0|000|1001 4 lines mode */
#define KS0073_4LINES_MODE                    0x09   /* |0|001|0000 4-bit mode, extension-bit RE = 0 */


/* 
** function prototypes 
*/
static void toggle_e(void);
static void toggle_e2(void);

/*
** local functions
*/



/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )    
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
    	    "1: sbiw %0,1" "\n\t"                  
    	    "brne 1b"                              // 4 cycles/loop
    	    : "=w" (__count)
    	    : "0" (__count)
    	   );
}


/************************************************************************* 
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/
#define delay(us)  _delayFourCycles( ( ( 1*(F_CPU/4000) )*us)/1000 )

/* toggle Enable Pin to initiate write */
static void toggle_e(void)
{
    lcd_e_high();
    lcd_e_delay();
    lcd_e_low();
}

static void toggle_e2(void)
{
    lcd_e2_high();
    lcd_e_delay();
    lcd_e2_low();
}


/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data    
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data,uint8_t rs) 
{
    unsigned char dataBits ;


    if (rs) {   /* write data        (RS=1, RW=0) */
       lcd_rs_high();
    } else {    /* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }
    lcd_rw_low();

    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        /* configure data pins as output */
        DDR(LCD_DATA0_PORT) |= 0x0F;

        /* output high nibble first */
        dataBits = LCD_DATA0_PORT & 0xF0;
        LCD_DATA0_PORT = dataBits |((data>>4)&0x0F);
        lcd_e_toggle();

        /* output low nibble */
        LCD_DATA0_PORT = dataBits | (data&0x0F);
        lcd_e_toggle();

        /* all data pins high (inactive) */
        LCD_DATA0_PORT = dataBits | 0x0F;
    }
    else
    {
        /* configure data pins as output */
        DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
        
        /* output high nibble first */
        LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
        LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
        LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
        LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    	if(data & 0x80) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    	if(data & 0x40) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    	if(data & 0x20) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    	if(data & 0x10) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);   
        lcd_e_toggle();
        
        /* output low nibble */
        LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
        LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
        LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
        LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    	if(data & 0x08) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    	if(data & 0x04) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    	if(data & 0x02) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    	if(data & 0x01) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
        lcd_e_toggle();        
        
        /* all data pins high (inactive) */
        LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
        LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
        LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
        LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    }
}

static void lcd_write2(uint8_t data,uint8_t rs) 
{
    unsigned char dataBits ;


    if (rs) {   /* write data        (RS=1, RW=0) */
       lcd_rs_high();
    } else {    /* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }
    lcd_rw_low();

    /* configure data pins as output */
    DDR(LCD_DATA0_PORT) |= 0x0F;

    /* output high nibble first */
    dataBits = LCD_DATA0_PORT & 0xF0;
    LCD_DATA0_PORT = dataBits |((data>>4)&0x0F);
    lcd_e2_toggle();

    /* output low nibble */
    LCD_DATA0_PORT = dataBits | (data&0x0F);
    lcd_e2_toggle();

    /* all data pins high (inactive) */
    LCD_DATA0_PORT = dataBits | 0x0F;
}

/*************************************************************************
Low-level function to read byte from LCD controller
Input:    rs     1: read data    
                 0: read busy flag / address counter
Returns:  byte read from LCD controller
*************************************************************************/
static uint8_t lcd_read(uint8_t rs) 
{
    uint8_t data;
    
    
    if (rs)
        lcd_rs_high();                       /* RS=1: read data      */
    else
        lcd_rs_low();                        /* RS=0: read busy flag */
    lcd_rw_high();                           /* RW=1  read mode      */
    
    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && ( LCD_DATA0_PIN == 0 )&& (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        DDR(LCD_DATA0_PORT) &= 0xF0;         /* configure data pins as input */
        
        lcd_e_high();
        lcd_e_delay();        
        data = PIN(LCD_DATA0_PORT) << 4;     /* read high nibble first */
        lcd_e_low();
        
        lcd_e_delay();                       /* Enable 500ns low       */
        
        lcd_e_high();
        lcd_e_delay();
        data |= PIN(LCD_DATA0_PORT)&0x0F;    /* read low nibble        */
        lcd_e_low();
    }
    else
    {
        /* configure data pins as input */
        DDR(LCD_DATA0_PORT) &= ~_BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) &= ~_BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) &= ~_BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) &= ~_BV(LCD_DATA3_PIN);
                
        /* read high nibble first */
        lcd_e_high();
        lcd_e_delay();        
        data = 0;
        if ( PIN(LCD_DATA0_PORT) & _BV(LCD_DATA0_PIN) ) data |= 0x10;
        if ( PIN(LCD_DATA1_PORT) & _BV(LCD_DATA1_PIN) ) data |= 0x20;
        if ( PIN(LCD_DATA2_PORT) & _BV(LCD_DATA2_PIN) ) data |= 0x40;
        if ( PIN(LCD_DATA3_PORT) & _BV(LCD_DATA3_PIN) ) data |= 0x80;
        lcd_e_low();

        lcd_e_delay();                       /* Enable 500ns low       */
    
        /* read low nibble */    
        lcd_e_high();
        lcd_e_delay();
        if ( PIN(LCD_DATA0_PORT) & _BV(LCD_DATA0_PIN) ) data |= 0x01;
        if ( PIN(LCD_DATA1_PORT) & _BV(LCD_DATA1_PIN) ) data |= 0x02;
        if ( PIN(LCD_DATA2_PORT) & _BV(LCD_DATA2_PIN) ) data |= 0x04;
        if ( PIN(LCD_DATA3_PORT) & _BV(LCD_DATA3_PIN) ) data |= 0x08;        
        lcd_e_low();
    }
    return data;
}

static uint8_t lcd_read2(uint8_t rs) 
{
    uint8_t data;
    
    if (rs)
        lcd_rs_high();                       /* RS=1: read data      */
    else
        lcd_rs_low();                        /* RS=0: read busy flag */
    lcd_rw_high();                           /* RW=1  read mode      */
    

	DDR(LCD_DATA0_PORT) &= 0xF0;         /* configure data pins as input */
	
	lcd_e2_high();
	lcd_e_delay();        
	data = PIN(LCD_DATA0_PORT) << 4;     /* read high nibble first */
	lcd_e2_low();
	
	lcd_e_delay();                       /* Enable 500ns low       */
	
	lcd_e2_high();
	lcd_e_delay();
	data |= PIN(LCD_DATA0_PORT)&0x0F;    /* read low nibble        */
	lcd_e2_low();
    
    return data;
}

/*************************************************************************
loops while lcd is busy, returns address counter
*************************************************************************/
static uint8_t lcd_waitbusy(void)
{
    register uint8_t c;
    
    /* wait until busy flag is cleared */
    while ( (c=lcd_read(0)) & (1<<LCD_BUSY)) {}
    
    /* the address counter is updated 4us after the busy flag is cleared */
    delay(2);

    /* now read the address counter */
    return (lcd_read(0));  // return address counter
    
}

static uint8_t lcd_waitbusy2(void)
{
    register uint8_t c;
    
    /* wait until busy flag is cleared */
    while ( (c=lcd_read2(0)) & (1<<LCD_BUSY)) {}
    
    /* the address counter is updated 4us after the busy flag is cleared */
    delay(2);

    /* now read the address counter */
    return (lcd_read2(0));  // return address counter
    
}

/* lcd_waitbusy */

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(uint8_t cmd)
{
    lcd_waitbusy();
    lcd_write(cmd,0);
}

void lcd_command2(uint8_t cmd)
{
    lcd_waitbusy2();
    lcd_write2(cmd,0);
}

/*************************************************************************
Send data byte to LCD controller 
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(uint8_t data)
{
    lcd_waitbusy();
    lcd_write(data,1);
}

void lcd_data2(uint8_t data)
{
    lcd_waitbusy2();
    lcd_write2(data,1);
}


/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor 
is already on the last line.
*************************************************************************/
static inline void lcd_newline(uint8_t pos)
{
    register uint8_t addressCounter;

    if (mode.enable)
    {
    	if( mode.display == 0)
    	{
    		if ( pos < (LCD_START_LINE2) )
    			addressCounter = LCD_START_LINE2;
    		else
    		{
    			addressCounter = LCD_START_LINE1;
    			mode.display = 1;
    		}
    	}
    	else
    	{
    		if ( pos < (LCD_START_LINE2) )
    			addressCounter = LCD_START_LINE2;
    		else
    		{
    			addressCounter = LCD_START_LINE1;
    			mode.display = 1;
    		}
    	}
    	if (mode.display == 0)
    		lcd_command((1<<LCD_DDRAM)+addressCounter);
    	else
    		lcd_command2((1<<LCD_DDRAM)+addressCounter);
    }
    else 
    {
    	if (lcd_lines == 1)
    		addressCounter = 0;
		else if (lcd_lines == 4)
		{
			if (lcd_controller_ks0073)
			{
				if ( pos < LCD_START_LINE2 )
					addressCounter = LCD_START_LINE2;
				else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE3) )
					addressCounter = LCD_START_LINE3;
				else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE4) )
					addressCounter = LCD_START_LINE4;
				else 
					addressCounter = LCD_START_LINE1;
			} else {
				if ( pos < LCD_START_LINE3 )
					addressCounter = LCD_START_LINE2;
				else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
					addressCounter = LCD_START_LINE3;
				else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
					addressCounter = LCD_START_LINE4;
				else 
					addressCounter = LCD_START_LINE1;
			}
		}
		else // lcd_lines == 2
		{
			if ( pos < (LCD_START_LINE2) )
				addressCounter = LCD_START_LINE2;
			else
				addressCounter = LCD_START_LINE1;
		}
		lcd_command((1<<LCD_DDRAM)+addressCounter);
    }
}/* lcd_newline */


/*
** PUBLIC FUNCTIONS 
*/

/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(uint8_t x, uint8_t y)
{
	if (mode.enable == 1)
	{
		if ( y==0 )
		{
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
			mode.display = 0;
		}
		else if ( y==1)
		{
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
			mode.display = 0;
		}
		else if ( y==2)
		{
			lcd_command2((1<<LCD_DDRAM)+LCD_START_LINE1+x);
			mode.display = 1;
		}
		else /* y==3 */
		{
			lcd_command2((1<<LCD_DDRAM)+LCD_START_LINE2+x);
			mode.display = 1;
		}
	}
	else if (lcd_lines == 1)
		lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
	else if (lcd_lines == 4)
	{
		if ( y==0 )
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
		else if ( y==1)
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
		else if ( y==2)
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
		else /* y==3 */
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
	}
	else // lcd_lines == 2
	{
		if ( y==0 ) 
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
		else
			lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
	}
}/* lcd_gotoxy */


/*************************************************************************
*************************************************************************/
int lcd_getxy(void)
{
    return lcd_waitbusy();
}


/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
    if(mode.enable)
        lcd_write2(1<<LCD_CLR,0);
    mode.display = 0;
}


/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
    mode.display = 0;
}


/*************************************************************************
Display character at current cursor position 
Input:    character to be displayed                                       
Returns:  none
*************************************************************************/
void lcd_putc(char c)
{
    uint8_t pos;

    if(mode.enable && mode.display == 1)
    	pos = lcd_waitbusy2();
    else
    	pos = lcd_waitbusy();   // read busy-flag and address counter
    if (c=='\n')
    {
        lcd_newline(pos);
    }
    else if(c=='\r')
    {
    	; // Just ignore linefeed
    }
    else
    {
    	if (lcd_wrap_lines == 1)
    	{
    		if (mode.enable)
    		{
    			if( mode.display == 0)
    			{
        			if ( pos == LCD_START_LINE1+lcd_disp_length ) {
        				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        			}else if ( pos == LCD_START_LINE2+lcd_disp_length ){
        				lcd_write2((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        				mode.display = 1;
        			}
    			}
    			else
    			{
        			if ( pos == LCD_START_LINE1+lcd_disp_length ) {
        				lcd_write2((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        			}else if ( pos == LCD_START_LINE2+lcd_disp_length ){
        				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        				mode.display = 0;
        			}
    			}
    		}
    		else if (lcd_lines == 1)
    		{
    			if ( pos == LCD_START_LINE1+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
    			}
    		}
    		else if (lcd_lines == 4)
    		{
    			if ( pos == LCD_START_LINE1+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
    			}else if ( pos == LCD_START_LINE2+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE3,0);
    			}else if ( pos == LCD_START_LINE3+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE4,0);
    			}else if ( pos == LCD_START_LINE4+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
    			}
    		}
    		else // lcd_lines == 2
    		{
    			if ( pos == LCD_START_LINE1+lcd_disp_length ) {
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
    			}else if ( pos == LCD_START_LINE2+lcd_disp_length ){
    				lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
    			}
    		}
    		if(mode.enable && mode.display == 1)
    			lcd_waitbusy2();
    		else
    			lcd_waitbusy();
    	}
    	if(mode.enable && mode.display == 1)
    		lcd_write2(c, 1);
    	else
    		lcd_write(c, 1);
    }

}/* lcd_putc */


/*************************************************************************
Display string without auto linefeed 
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }

}/* lcd_puts */


/*************************************************************************
Display string from program memory without auto linefeed 
Input:     string from program memory be be displayed                                        
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_putc(c);
    }

}/* lcd_puts_p */


/*************************************************************************
Initialize display and select type of cursor 
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init(uint8_t dispAttr)
{
	mode.enable = 0;
	mode.display = 0;
    /*
     *  Initialize LCD to 4 bit I/O mode
     */
     
    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && ( &LCD_RS_PORT == &LCD_DATA0_PORT) && ( &LCD_RW_PORT == &LCD_DATA0_PORT) && (&LCD_E_PORT == &LCD_DATA0_PORT)
      && (LCD_DATA0_PIN == 0 ) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) 
      && (LCD_RS_PIN == 4 ) && (LCD_RW_PIN == 5) && (LCD_E_PIN == 6 ) )
    {
        /* configure all port bits as output (all LCD lines on same port) */
        DDR(LCD_DATA0_PORT) |= 0x7F;
        DDR(LCD_E2_PORT)    |= _BV(LCD_E2_PIN);
    }
    else if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
           && (LCD_DATA0_PIN == 0 ) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        /* configure all port bits as output (all LCD data lines on same port, but control lines on different ports) */
        DDR(LCD_DATA0_PORT) |= 0x0F;
        DDR(LCD_RS_PORT)    |= _BV(LCD_RS_PIN);
        DDR(LCD_RW_PORT)    |= _BV(LCD_RW_PIN);
        DDR(LCD_E_PORT)     |= _BV(LCD_E_PIN);
        DDR(LCD_E2_PORT)    |= _BV(LCD_E2_PIN);
    }
    else
    {
        /* configure all port bits as output (LCD data and control lines on different ports */
        DDR(LCD_RS_PORT)    |= _BV(LCD_RS_PIN);
        DDR(LCD_RW_PORT)    |= _BV(LCD_RW_PIN);
        DDR(LCD_E_PORT)     |= _BV(LCD_E_PIN);
        DDR(LCD_E2_PORT)    |= _BV(LCD_E2_PIN);
        DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
    }
    delay(16000);        /* wait 16ms or more after power-on       */
    
    /* initial write to lcd is 8bit */
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);  // _BV(LCD_FUNCTION)>>4;
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);  // _BV(LCD_FUNCTION_8BIT)>>4;
    lcd_e_toggle();
    delay(4992);         /* delay, busy flag can't be checked here */
   
    /* repeat last command */ 
    lcd_e_toggle();      
    delay(64);           /* delay, busy flag can't be checked here */
    
    /* repeat last command a third time */
    lcd_e_toggle();      
    delay(64);           /* delay, busy flag can't be checked here */

    /* now configure for 4bit mode */
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);   // LCD_FUNCTION_4BIT_1LINE>>4
    lcd_e_toggle();
    delay(64);           /* some displays need this additional delay */
    
    /* from now the LCD only accepts 4 bit I/O, we can use lcd_command() */    

	if (lcd_lines == 4 && lcd_controller_ks0073)
	{
		/* Display with KS0073 controller requires special commands for enabling 4 line mode */
		lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_ON);
		lcd_command(KS0073_4LINES_MODE);
		lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_OFF);
	}
	else if (lcd_lines == 1)
		lcd_command(LCD_FUNCTION_4BIT_1LINE);      /* function set: display lines  */
	else //lcd_lines == 2
		lcd_command(LCD_FUNCTION_4BIT_2LINES);

    lcd_command(LCD_DISP_OFF);              /* display off                  */
    lcd_clrscr();                           /* display clear                */ 
    lcd_command(LCD_MODE_DEFAULT);          /* set entry mode               */
    lcd_command(dispAttr);                  /* display/cursor control       */
    
    // Clear out the second display.
    // As we don't know if we have a second display we can not use lcd_command2
    delay(64);
    lcd_write2(LCD_FUNCTION_4BIT_2LINES,0);
    delay(64);
    lcd_write2(LCD_DISP_OFF,0);		// Turn off dispaly2 in case we have one.
    delay(64);

}/* lcd_init */


void lcd_setup(uint8_t col, uint8_t row)
{
	if (lcd_lines != row)
	{
		lcd_lines = row;
		if (lcd_lines == 4 && lcd_controller_ks0073)
		{
		    /* Display with KS0073 controller requires special commands for enabling 4 line mode */
			lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_ON);
			lcd_command(KS0073_4LINES_MODE);
			lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_OFF);
		}
		else if (lcd_lines == 1)
		    lcd_command(LCD_FUNCTION_4BIT_1LINE);      /* function set: display lines  */
		else // lcd_lines == 2
			lcd_command(LCD_FUNCTION_4BIT_2LINES);
	}
	if (lcd_disp_length != col)
		lcd_disp_length = col;

	if (lcd_lines == 4 && lcd_disp_length == 40)
	{
		mode.enable = 1;
		mode.display = 0;
		lcd_command2(LCD_FUNCTION_4BIT_2LINES);
		lcd_command2(LCD_DISP_OFF);
		lcd_clrscr(); 
		lcd_command2(LCD_MODE_DEFAULT);
		lcd_command2(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	}
	else
		mode.enable = 0;
}

void lcd_linewrap(uint8_t on)
{
	lcd_wrap_lines = on;
}

void lcd_ks0073(uint8_t on)
{
	lcd_controller_ks0073 = on;
	if(on)
	{
		/* Display with KS0073 controller requires special commands for enabling 4 line mode */
		lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_ON);
		lcd_command(KS0073_4LINES_MODE);
		lcd_command(KS0073_EXTENDED_FUNCTION_REGISTER_OFF);
	}
}

void lcd_createCharacter(uint8_t pos, uint8_t *data)
{
	lcd_command(_BV(LCD_CGRAM) | (pos<<3)); // set CG RAM start address
	lcd_write2(_BV(LCD_CGRAM) | (pos<<3), 0);

	for(uint8_t i = 0; i < 8; i++) {
		lcd_data(data[i]);
		delay(10);
		lcd_write2(data[i],1);
	}
}