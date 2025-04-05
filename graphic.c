/*


Copyright 2001 Eumedic Ltd.

*/

#pragma language=extended	/* Enable use of extended keywords */

#include <stdio.h>
#include <inh8.h>	  	/* Intrinsic Funcs */
#include <string.h>

#include "hardware.h" 	
#include "serial.h" 	
#include "time.h" 				
#include "sysinit.h"
#include "graphic.h"
#include "fonts.h"
#include "general.h"
#include "debug.h"

	






#define SCI1_SCR_INIT   0x01  /* no interrupts, Tx & Rx enabled, internal sync clock */
#define SCI1_SMR_INIT   0x80  /* synchronous, 8-bits, no parity, clock divide 1 */
#define SCI1_BRR_INIT   1    /* 500KHz @4M clock. [See H8S2345 Manual Page 430] */


/* scr bits */
#define TIE  0x80
#define RIE  0x40
#define TE   0x20
#define RE   0x10
#define MPIE 0x08
#define TEIE 0x04
#define CKE1 0x02
#define CKE0 0x01

#define TEND BIT2

#define MSB_FIRST sci1_scmr1 |= BIT3
#define LSB_FIRST sci1_scmr1 &= 0xFF - BIT3


#if 0
/* service the serial output - Debug */
void interrupt [SCI_TXI1] serial_transmit_service1()
{

	STR("SER2 Interrupt!\r\n");

	sci1_scr1 &= 0xFF - SCR_TIE; /* Turn off the interrupt. */ 
}
#endif

/******************************************************************
NAME		: sci1_init
PURPOSE		: Initialise the second serial port to drive the LCD.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void sci1_init ()
{
	mstpcrl &= 0xFF - SCI1_MODULE_STOP; /* SCI are Go! */

	/* Initialise SCR1 */
	sci1_scr1 = SCI1_SCR_INIT;  /* init scr1 leaving TE and RE zero */
	sci1_smr1 = SCI1_SMR_INIT;
	sci1_brr1 = SCI1_BRR_INIT; 
	 
	MSB_FIRST;

	wait(2);    /* wait for one bit period - 1mS is overkill but OK */

	sci1_scr1 = SCI1_SCR_INIT | TE;	 /* Enable Tx. */
}

/******************************************************************
NAME		: sci1_send
PURPOSE		: Send a character from the synch serial port.
PARAMETERS	: unsigned char senddata: the char to send.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void sci1_send (unsigned char senddata)
{
	while ((sci1_ssr1 & TDRE) == 0); 	/* wait for transmitter ready */
	sci1_tdr1 = senddata;            	/* transmit control with start bit set */
	sci1_ssr1 &= 0xFF - TDRE; 	   	 	/* clear TDRE to start transmission */
}


#define GLCD_A0  BIT7
#define GLCD_CS1 BIT6
//#define GLCD_BITS (GLCD_A0 | GLCD_CS1)

/******************************************************************
NAME		: glcd_control_write
PURPOSE		: Write a byte to the control reg of the LCD.
PARAMETERS	: char byte: the byte to write.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void glcd_control_write(char byte)
{
	/* Make sure ALL bits out done before changing A0 and bit direction! */
	/* As the A0 is clocked in at the end of the data... */
	while ((sci1_ssr1 & TEND) == 0);

	pedr &= 0xFF - GLCD_A0;	
	sci1_send(byte);		
}




char disp_data[DISP_BUFF_SIZE];

char changed_lines[PAGES_IN_DISPLAY];


/******************************************************************
NAME		: set_contrast
PURPOSE		: set the contrast on the display according to the global value.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void set_contrast(char contr)
{
	glcd_control_write(0x80 | ( contr & 0x1F));  //SET THE CONTRAST	
}	


char upside_down = 0;

/******************************************************************
NAME		: set_viewpoint
PURPOSE		: Set whether the display is upsidedown or not.
PARAMETERS	: char upside_down_p: TRUE if we need the display upsidedown.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void set_viewpoint(char upside_down_p)
{
	upside_down = upside_down_p;
	/* Set all lines changed. */
	memset(changed_lines , 1 , PAGES_IN_DISPLAY);
}	


/******************************************************************
NAME		: update_display 
PURPOSE		: Write the contents of the ram buffer of display 
				data to the LCD.  Doesn't write to lines that
				have not been recorded as having changed.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void update_display()
{
 char i,j;
 register char * data_pointer;

	data_pointer = disp_data;	 /* Start at the begining of the display RAM. */

	/* Make sure the clock can get back in to synch if it gets corrupted. */
    pedr |= GLCD_CS1;  /* Turn off CS\  */
    pedr &= 0xFF - GLCD_CS1;  /* Turn on CS\  */

	if (!upside_down)
	{
		glcd_control_write(0xA0);  /* Normal left & right */	

		/* 8 Pages...each page starts at col 33 due to 'output configuration'. */
		for (j=0; j<8; j++ )
		{
			if (changed_lines[j])
			{
				changed_lines[j] = 0;

				glcd_control_write(0xB0 | j);  	//page j
				glcd_control_write(0x12);  		//col msNyb=2 	
				glcd_control_write(0x01);  		//col lsNyb=1 	


				/* Make sure ALL serial data bits sent before changing A0 and bit direction! */
				while ((sci1_ssr1 & TEND) == 0);

		   		pedr |= GLCD_A0;		  /* Raise A0 to send to data register. */

				/* COLS_IN_LCD_PAGE bytes per page... */
				for (i=COLS_IN_LCD_PAGE; i; i--)
				{	
					sci1_send(*data_pointer++);
				}
				data_pointer += COLS_IN_PAGE - COLS_IN_LCD_PAGE;
			}
			else
			{
			   data_pointer += COLS_IN_PAGE;	
			}
		}
	}
	else
	{

		glcd_control_write(0xA1);  /* Reversed left & right */	


		/* 8 Pages...each page starts at col 33 due to 'output configuration'. */
		for (j=0; j<8; j++ )
		{
			if (changed_lines[j])
			{
			   	changed_lines[j] = 0;

				glcd_control_write(0xB0 | (7-j));  	//page j
				glcd_control_write(0x12);  		//col msNyb=2 	
				glcd_control_write(0x01);  		//col lsNyb=1 	


				/* Make sure ALL serial data bits sent before changing A0 and bit direction! */
				while ((sci1_ssr1 & TEND) == 0);

		   		pedr |= GLCD_A0;		  /* Raise A0 to send to data register. */

				LSB_FIRST;

				/* COLS_IN_LCD_PAGE bytes per page... */
				for (i=COLS_IN_LCD_PAGE; i; i--)
				{		      
					sci1_send(*data_pointer++);
				}
				MSB_FIRST;

				data_pointer += COLS_IN_PAGE - COLS_IN_LCD_PAGE;
			}
			else
			{
			   data_pointer += COLS_IN_PAGE;	
			}
		}
	}
}	


#define CLEAR_DISP_RAM memset(disp_data,0,DISP_BUFF_SIZE)


/* Masks for vertical line and rectangle drawing. */
const char start_masks[] =
{
	0xFF,
	0xFE,
	0xFC,
	0xF8,
	0xF0,
	0xE0,
	0xC0,
	0x80
};

const char end_masks[] =
{
	0x01,
	0x03,
	0x07,
	0x0F,
	0x1F,
	0x3F,
	0x7F,
	0xFF
};


/******************************************************************
NAME		: fill_rectangle
PURPOSE		: Blackout a rectangle in pixel coordinates.  Y is downwards(?), X is leftwards.
PARAMETERS	: char x1, char y1, char x2, char y2: coordinates of the rectangle.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void fill_rectangle(char x1, char y1, char x2, char y2)
{
	register char start_row = y1>>3;
	register char * ram_ptr = disp_data + start_row * COLS_IN_PAGE + x1;
	char i,j,cl = start_row;
	char bytes_across;
	signed char full_rows;

	/* Work out the masks */
	char start_mask = start_masks[y1 & 7];
   	char end_mask = end_masks[y2 & 7];
	full_rows = (y2>>3) - start_row -1;



	if (full_rows < 0)
	{
		start_mask &= end_mask;	
	}

	bytes_across = x2-x1;

	/* Do the start bytes */
	for (i=0; i<bytes_across; i++)
	{
		ram_ptr[i] |= start_mask;
	}
	changed_lines[cl++] = 1;

	/* Do the middle bytes. */
	for (j=0; j<full_rows ; j++ )
	{
		ram_ptr+= COLS_IN_PAGE;
		for (i=0; i<bytes_across; i++)
		{
			ram_ptr[i] = 0xFF;
		}
		changed_lines[cl++] = 1;
	}

	if (full_rows >= 0)
	{
		ram_ptr+= COLS_IN_PAGE;

		/* Do the end bytes. */
		for (i=0; i<bytes_across; i++)
		{
			ram_ptr[i] |= end_mask;
		}
		changed_lines[cl++] = 1;
	}
}


/******************************************************************
NAME		: plot_small_char
PURPOSE		: Plot a small font char, on one of the lines of the LCD.
			  Note: This can only write to particular 8-pixel high lines
			  of the LCD. It also writes over anything underneath. 
PARAMETERS	: char line: The LCD line to write to.
			  char col: The LCD column to write to.
			  char c: The character to write.
			  char clear_space_after: Clear the space after the char?
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void plot_small_char(char line, char col, char c, char clear_space_after)
{
/* Fixed width font. */
#define FONT_FIRST_CHAR '!'
#define FONT_FIXED_WIDTH 5
#define FONT_FIXED_INCREMENT 8
	register char * ram_ptr = disp_data + line * COLS_IN_PAGE + col; /* The pointer into the disp. RAM. */
	register char * font_ptr =  (char *)font_data1 + (c-FONT_FIRST_CHAR) * FONT_FIXED_INCREMENT;
	register char i;

	/* Space char etc. */
	if (c < FONT_FIRST_CHAR)
	{
		for (i=0; i<FONT_FIXED_WIDTH; i++)
		{
			*ram_ptr++ = 0;
		}
	}
	else /* Normal chars. */
	{
		for (i=0; i<FONT_FIXED_WIDTH; i++)
		{
			*ram_ptr++ = *font_ptr++;	  //Write the data: use '=' to wipe what's underneath.
		}
	}

	if (clear_space_after)
	{
		*ram_ptr = 0;
	}

}

#if 0
void invert_char(char line, char col)
{	char i;

	register char * ram_ptr = disp_data + line * COLS_IN_PAGE + col-1; /* The pointer into the disp. RAM. */
	for (i = FONT_FIXED_WIDTH+2; i-- ;)
	{
	   * ram_ptr = ~  (* ram_ptr);
	   ram_ptr++;
	}
}	
#endif

/******************************************************************
NAME		: bar_graph
PURPOSE		: Draws a 8-pixel high bar across the LCD, scaled 
				according to the variable and it's max value.
PARAMETERS	: ...
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void bar_graph(char line, char col , char size , short max , short variable)
{
	register char * ram_ptr = disp_data + line * COLS_IN_PAGE + col; /* The pointer into the disp. RAM. */
	char i,black_bytes;

	black_bytes = variable * size / max	;


//PRINT "%i %i %i\r\n",black_bytes,max,variable PREND

	for (i = 0; i < black_bytes ; i++)
	{
		*ram_ptr++ = 0xFF;
	}

	for (; i < size ; i++)
	{
		*ram_ptr++ = 0;
	}
	changed_lines[line] = 1;
}	

/******************************************************************
NAME		: plot_big_char
PURPOSE		: Plot a big font char, on two of the lines of the LCD.
			  Note: This can only write to particular 8-pixel high lines
			  of the LCD. It also writes over anything underneath. 
PARAMETERS	: char line: The LCD line to write to.
			  char col: The LCD column to write to.
			  char c: The character to write.
			  char clear_space_after: Clear the space after the char?
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void plot_big_char(char line, char col, char c, char clear_space_after)
{
/* Fixed width font. */
#define FONT_FIRST_CHAR2 '!'
#define FONT_FIXED_WIDTH2 9
#define FONT_FIXED_INCREMENT2 18
	register char * ram_ptr = disp_data + line * COLS_IN_PAGE + col; /* The pointer into the disp. RAM. */
	register char * font_ptr =  (char *)font_data2 + (c-FONT_FIRST_CHAR2) * FONT_FIXED_INCREMENT2;
	register char i;

	/* Space char etc. */
	if (c < FONT_FIRST_CHAR2)
	{
		for (i=0; i<FONT_FIXED_WIDTH2; i++)
		{
			*ram_ptr = 0;
			*(ram_ptr - COLS_IN_PAGE) = 0;	
			ram_ptr++;
		}
	}
	else /* Normal chars. */
	{
		for (i=0; i<FONT_FIXED_WIDTH2; i++)
		{
			*ram_ptr = *font_ptr;	  //Write the data: use '=' to wipe what's underneath.
			*(ram_ptr - COLS_IN_PAGE) = *(font_ptr + (FONT_FIXED_INCREMENT2/2));

			font_ptr++;
			ram_ptr++;
		}
	}

	if (clear_space_after)
	{
		*ram_ptr = 0;
		*(ram_ptr - COLS_IN_PAGE) = 0;	
	}
}	

/******************************************************************
NAME		: glcd_print_string
PURPOSE		: Print a string to the LCD in either font at the
				line and collumn given.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void glcd_print_string(char line, char col, char font, char * str)
{
	char c;

	switch (font)
	{
		case 1:
			while (c = *str++)
			{
				plot_small_char(line,col,c,*str);
				col += FONT_FIXED_WIDTH + 1;
			}
			changed_lines[line] = 1;
			break;

		case 2:
			while (c = *str++)
			{
				plot_big_char(line,col,c,*str);
				col += FONT_FIXED_WIDTH2 + 1;
			}
			changed_lines[line - 1] = changed_lines[line] = 1;
			break;

		default:
			break;
	}
}


/******************************************************************
NAME		: plot_sprite
PURPOSE		: Plot a sprite on the LCD in either font at the
				line and collumn given.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void plot_sprite(char line, char col, char font, char c)
{
	switch (font)
	{
		case 1:
			plot_small_char(line,col,c,0);
			changed_lines[line] = 1;
			break;

		case 2:
			plot_big_char(line,col,c,0);
			changed_lines[line - 1] = changed_lines[line] = 1;
			break;

		default:
			break;
	}
}	
	


/******************************************************************
NAME		: clear_screen
PURPOSE		: Clears the display RAM and sets the screen to all changed.
				Does not update the LCD.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void clear_screen()
{
	CLEAR_DISP_RAM;
	memset(changed_lines , 1 , PAGES_IN_DISPLAY);
}	


/******************************************************************
NAME		: glcd_init
PURPOSE		: Initialises the LCD controller.
			  Needs the synch serial port initialised first.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/



void glcd_init()
{ 
    pedr &= 0xFF - GLCD_CS1;  /* Turn on CS\  */


	/* Software reset. */
	glcd_control_write(0xE2);  //RESET DISPLAY	


	glcd_control_write(0xC4);  //SETS OUTPUT STATE (configuration of segment drivers)	
	glcd_control_write(0xA9);  //SETS DUTY TO 1/64	
	glcd_control_write(0x80 | ( DEFAULT_CONTRAST & 0x1F));  //SET THE CONTRAST	
	glcd_control_write(0x25);  //Onboard POWER ON
							   
	wait (50);				   //PAUSE FOR THE TIMING CONSTRAINTS

							   
	glcd_control_write(0xED);  //SETS POWER MODE	
	glcd_control_write(0xAF);  //TURNS DISPLAY ON	
	glcd_control_write(0x40);  //SET LINE ONE TO TOP LINE OF DISPLAY	

	clear_screen();
}							   
							   



