


/*


Copyright 2001 Eumedic Ltd.

*/








/*

DESCRIPTION:   The C language entry point.
AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------

Released first version:
 V1.00

2001 04 04
 Added Frequency Modulation (or freq cycling)
 7 second period, from 30 to 120Hz.

2001 04 04
 Using 10S FIFO on the delta-dynamic display - don't know if that's the best way to do it.
 Serial port only turned on if CTS is detected.

 Release: 
 V1.01


2001 05 01
 Put in sound when reach d/t <= 0 for 2 seconds. Used double speed warble.
 Release: 
 V1.02


2001 06 21
 Added short warble on skin contact.
  V1.03

2001 06 21
 Using DTC to meaure pulse widths:
	Setting up DTC in the pulse in int.
	Setting up pulse in int in the pulse out int.
 	Tested Running CPU at 1/4 clock to test if we can use 4Mhz xtal. -> looks good.
 	[Could run even slower!]


2001 07 03
 Visual sign that 10s Calc bell has rung.
 Increased moving average D/T to run over 15S.
 Put in 3S screen freeze for each of the bells: dose bell and flat bell.
 [Moved warble so that it happens after the screen has updated to give better feel]
 Put back the display of n_rings, and added display of the initial rings: taken after 0.5 Secs.

 V1.04


2001 08 16
Put in graphics LCD drivers and GLCD based menu system.

2001 08 21
Took time_low out of time calcs.

*/
		
#include <stdio.h>
#include <inh8.h>	  	/* Intrinsic Funcs */
#include <string.h>

#include "hardware.h"					  
#include "debug.h"					  
#include "time.h"					  
#include "pulse.h"					  
#include "iic.h"					  
#include "graphic.h"					  





		 





/* This is where we come in after the C system init that sets a stack in 
internal RAM, and fills the initialised data.
Don't put any more code in here, as we dont have much of a stack at this point,
and the stack may overwrite any date put in internal RAM.
*/


void set_stack(long pos);

int main()
{  
void setup_buzzer();

void start_screen(); 

	io_init();			 		/* Configure most of the I/O ports. */

	milli_sec_timer_init();	  	/* Initialise the time keeping systems, based on interrupts. */
	
	INTS_ON;					/* Let interrupts in from here. */

	iic_init();					/* Init the IIC Bus */

	serial_init();	 			/* Configure the serial port. */

	sci1_init();  				/* Init the LCD synch serial */

	glcd_init();				/* Init the Graphics LCD */

	setup_watchdog();			/* Setup software watchdog. */

	setup_buzzer();

STR("Eumedic\r\n");

	start_screen();

	/*********************/
	/* We should never get here. */
	STR("BackStop!\a\r\n");
	while (1)
	{	
		power_down();
	}
}

