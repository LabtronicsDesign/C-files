

/*

DESCRIPTION:   Serial port code + object.
AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
Addapted from VM-1 code.
*/


/*
NOTES:

*/


#pragma language=extended	/* Enable use of extended keywords */

#include <stdio.h>
#include <inh8.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hardware.h"
#include "serial.h"

#include "time.h"


#include "general.h"

#include "debug.h" 

    
		  

char print_temp[PRINT_TEMP_BUFFER_SIZE]; /* Workspace to print to for use by PRINT macro. */


/*
Note: if handshake is disabled then can comment out housekeeping code for handshake
 in the task swap interrupt to increase efficiency.
*/
#define USING_CTS 0		/* Enable the handshake input. See above note. */
#define ALLOW_SWAPS_WHEN_WAITING 0 /* swap out is called when processing awaiting a serial buffer to empty. */



#if ALLOW_SWAPS_WHEN_WAITING
 #define SWAP_OUT check_break_and_swap()
#else
 #define SWAP_OUT
#endif



char control_c_break_allowed = 0; /* Set when CTRL-C causes a break, rather than being a real char. */

static  char input_buffer [256];
static  char output_buffer [256];


static volatile unsigned char input_head  = 0;
static          unsigned char input_tail  = 0;
static          unsigned char output_head = 0;
static volatile unsigned char output_tail = 0;

char transmit_hs_status = 1; /* gives the status of the transmiter handshake input. 0==Tx has been turned off by HSin */ 



#define HS_ON_POINT  (256/4)   /* The input buffer size below which the HS output is turned on */
#define HS_OFF_POINT  (256/4*3) /* The input buffer size above which the HS output is turned off */

/* service the serial output */
void interrupt [SCI_TXI0] serial_transmit_service()
{
    char c;

#if USING_CTS
    if (cts0_bit) /* handshake input says 'off' */
	{
	   sci0_scr0 &= 0xFF - SCR_TIE; /* [Turn off the interrupt] */
	   transmit_hs_status = 0; 
	} 
#endif

    c = output_buffer[output_tail++];	/* get the char from the buffer and update the pointer */

	sci0_tdr0 = c;  /* stuff the byte in the TDR */

    sci0_ssr0 &= 0xFF - TDRE; /* clear the empty flag */

	//INTS_ON;	/* Let other ints in. */

    if (output_head == output_tail) /* if there are no more chars then disable the transmit interrupt  */
	   sci0_scr0 &= 0xFF - SCR_TIE;  

}


#if 0
/* service the serial input */
void interrupt [SCI_RXI0] serial_receive_service()
{
	char c;

	//sci0_ssr0;	/* must read this before resetting the bit! (this should not get optimised away as it is an sfr.) */
	sci0_ssr0 &= 0xFF - RDRF; /* clear the received char flag */

	//INTS_ON;	/* Let other ints in. */


	/* get the char from the receive reg and put it in the buffer */
	c = sci0_rdr0;
	
	input_buffer[input_head++] = c;


	if ( (unsigned char)(input_head - input_tail) >= HS_OFF_POINT)	/* The cast to 'char' avoids promotion of the calc above 8-bit. */
       rts0_bit = 1;	 /* HS output off, please */
}
#endif



/* service the serial input if there is an error */
void interrupt [SCI_ERI0] serial_receive_error_service()
{

	sci0_ssr0 &= 0xFF - (ORER|FER|PER); /* clear the received char interrupt flag */
}




/* put a character in the serial buffer */
/* wait if the buffer is full */
/* turn on the interrupt if not already on */


void serial_put(char c)
{
	while ( (unsigned char) (output_head - output_tail)  == 0xFF)
		SWAP_OUT;

    INTS_OFF;
    output_buffer[output_head++] = c;  /* put the char in the buffer and update the pointer */

			#if USING_CTS
				if (cts0_bit == 0)  /* handshake in says 'on' */
				{
				   sci0_scr0 |= SCR_TIE; /* turn on the transmit interrupt */
				   transmit_hs_status = 1;
				}
				else
				   transmit_hs_status = 0;
			#else
				sci0_scr0 |= SCR_TIE; /* turn on the transmit interrupt */
			#endif
	INTS_ON;
}


#if 0
/* get a character from the serial input buffer */
char serial_get()
{
    char c;

	while (input_head == input_tail) /* wait if empty */
	  SWAP_OUT;
	  
    INTS_OFF;

    c = input_buffer[input_tail++];	/* get the char from the buffer and update the pointer */
	if ( (unsigned char) (input_head - input_tail) < HS_ON_POINT)
	   rts0_bit = 0;	 /* HS output on, please */
	INTS_ON;

	return c;
}

/* return the value of the charater if there is one in the queue, else return -1 */
short serial_look()
{
	if (serial_input_queue())
	{
		return serial_get();
	}
	return -1;
}
#endif



/* return the number of characters waiting in the input buffer */
unsigned char serial_input_queue()
{
   return (unsigned char)(input_head - input_tail);
}

/* return the number of characters waiting in the output buffer */
unsigned char serial_output_queue()
{
   return output_head - output_tail;
}

/* Wait until all the chars have gone out of the op buffer. */
void serial_wait_for_output()
{
   while ((unsigned char)(output_head - output_tail))
     SWAP_OUT;
}

/* Wait until all the chars have gone out of the op buffer. */
void serial_wait_for_output_no_swap()
{
   while ((unsigned char)(output_head - output_tail))
		;
}



/* Send the null-terminated string to the serial */
void serial_put_string(const char * str)
{
   while(*str)
   {
      serial_put(*str++);
   }
}


/* initialise the serial port and set up the buffers */
void serial_init()
{
   /* init port 0 */
	mstpcrl &= 0xFF - SCI0_MODULE_STOP; /* SCI are Go! */

/* Serial port initialisation. */



	//p3ddr = HS_OUT0; /* Turn on the HS out pin. */
	//p3dr = 0; /* Turn on the HS out . */


	sci0_scr0 = 0; /* (0=default value anyway), select clock source */
	sci0_smr0 = 0;  /* 8-None-1, processor clock with no division. */

   	sci0_brr0 = 12; /* 9600 baud */
		    
	/* Should wait one bit time here */
	wait (2);

	sci0_scr0 |= SCR_TE; /* Enable the transmitter. */


	/* We prob wont use the Rx - but if we do - make sure it is not allowed to float!! */
   	// sci0_scr0 |= SCR_RE; /* Enable the rx. */
   	// sci0_scr0 |= SCR_RIE; /* Enable the rx. interrupt */

/* END Serial port initialisation. */

}


