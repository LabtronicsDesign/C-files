

/*

DESCRIPTION:   IIC bus driver
AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
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
#include "iic.h"
#include "time.h"
#include "general.h"
#include "debug.h" 											    



#define iic_wait() no_operation()//;no_operation();no_operation();no_operation()







/* Turns IIC bus on 
SDA: HI OUT -> LO OUT
SCL: HI     -> HI
*/
void iic_on()
{
   p2dr &= 0xFF - SDA; /* lower the data line */
   iic_wait();
   p2dr &= 0xFF - SCL; /* lower the clock line */
   iic_wait();
}

/* Turns IIC bus off
SDA: LO OUT -> HI OUT
SCL: HI     -> HI
*/
void iic_off()
{
   p2dr |= SCL; /* raise the clock line */
   iic_wait();
   p2dr |= SDA; /* raise the data line */
   iic_wait();
}

/* Write a byte to the bus and detect the acknowledge
Initially:
SDA: LO OUT
SCL: LO
Ends with the same.
Returns true if acknowledges.
*/
char iic_put(char byte)
{char i;

   /* set the data bits, MSB first */
   for(i=0; i<8; i++)
   {
      if (byte & 0x80)
        p2dr |= SDA;       	/* raise the data line */
      else    
        p2dr &= 0xFF - SDA;	/* lower the data line */
   
      iic_wait();
      p2dr |=  SCL;        	/* raise the clock line */
      iic_wait();
      p2dr &= 0xFF - SCL;  	/* lower the clock line */
      iic_wait();
      byte <<= 1;          	/* shift the next bit in */
   }
   /* read the ack */
   p2ddrc &= (0xFF - SDA); 	/* make SDA an input */
   p2ddr = p2ddrc;

   iic_wait();
   p2dr |=  SCL;           	/* raise the clock line - after making SDA input, else get stop cond! */
   iic_wait();
   byte = port2;           	/* read SDA for the ack */
   p2dr &= 0xFF - SCL;     	/* lower the clock line, ending the ack period */
   iic_wait();

   //INTS_OFF;             	/* this has to be here else an interrupt (or task) could get in and change SDA */
   p2dr &= 0xFF - SDA; 	   	/* lower the data line, for end state */
   p2ddrc |= SDA;   	   	/* make SDA an output */
   p2ddr = p2ddrc;
   //INTS_ON;

   iic_wait();
   
   if (byte & SDA)
      return 0;
   return 1;  /* return true if SDA pulled low */
}


/* ! watch out for doing &= etc. on a port as it can change the state of o/p levels when
they are currently inputs */

/*
Read a byte from the bus and acknowledge it 
Initially:
SDA: LO OUT
SCL: LO
Ends with the same.
Returns true if acknowledges.
*/
char iic_get(char last)
{
   char i=0, byte = 0;
   
   p2ddrc &= (0xFF - SDA); /* make SDA an input */
   p2ddr = p2ddrc;
   iic_wait();
   
   while (1)
   {
      p2dr |=  SCL;    /* raise SCL */
      iic_wait();
      if (port2 & SDA)  /* if SDA is high put a 1 in the byte */
         byte |= 0x01;
         
      p2dr &= 0xFF - SCL - SDA;   /* lower the clock */
      iic_wait();
      
      if (++i == 8)     /* exit when 8 bits put in */
         break;

      byte <<=1;       /* shift the bits along ready for the next one */
   }
   /* Ack */
   
   if (last) /* don't ack */
   {
      p2dr |=  SCL;    /* raise SCL */
      iic_wait();
      p2dr &= 0xFF - SCL - SDA;   /* lower the clock */
      iic_wait();

      //INTS_OFF;            /* this has to be here else an interrupt (or task) could get in and change SDA */
      p2dr &= 0xFF - SDA; /* lower the data line */
      p2ddrc |= SDA;   /* make SDA an output */
      p2ddr = p2ddrc;
      //INTS_ON;
   }
   else /* do ack */
   {
      //INTS_OFF;            /* this has to be here else an interrupt (or task) could get in and change SDA */
      p2dr &= 0xFF - SDA; /* lower the data line */
      p2ddrc |= SDA;   /* make SDA an output */
      p2ddr = p2ddrc;
      //INTS_ON;

      iic_wait();
      p2dr |=  SCL;    /* raise SCL */
      iic_wait();
      p2dr &= 0xFF - SCL - SDA;   /* lower the clock */
   }
   iic_wait();
   return byte;
}
 
/* returns true if there is a device at 'address' */
char iic_poll(char address)
{  char x;
   iic_on();
   x = iic_put(address);
   iic_off();
   return x;
}


void iic_init()
{
   //INTS_OFF;
   p2dr = p2dr | (SDA + SCL);             /* make SDA and SCL go high when they become outputs */
   p2ddrc |= (SDA | SCL);                 /* make SDA and SCL outputs */
   p2ddr = p2ddrc;
   //INTS_ON;
   iic_on();
   iic_off();
}


#define EEPROM_ADDRESS 160 /* All address lines low */

void wait_for_eeprom_to_finish()
{
	char i = 0;

	while (!iic_poll(EEPROM_ADDRESS))  /* Wait while EEPROM busy */
	{
   		wait (5);
		if (i++ > 200)
		{
			STR("EEPROM not acknowledging!\r\n");
			break;	
		}
	}
}	

void write_block_to_eeprom(char address, char count , char * data)
{
	/* Send 1 byte at a time till we find out how many the EEPROM can take.!!! */
	for ( ; count; count--)
	{
		wait_for_eeprom_to_finish();

		iic_on();
		iic_put(EEPROM_ADDRESS);
		iic_put(address++);
		iic_put(*data++);
		iic_off();
	}

}	


void read_block_from_eeprom(char address, char count , char * data)
{
	wait_for_eeprom_to_finish();

	iic_on();
	iic_put(EEPROM_ADDRESS);
	iic_put(address);
	iic_off();

	iic_on();
	iic_put(EEPROM_ADDRESS+1);


	for ( ; count; count--)
	{
		*data++ = iic_get(0);
	}

	iic_get(1);	 /* Terminate the transaction and discard dummy byte.[easier to code but less efficient!] */

	iic_off();
}	



