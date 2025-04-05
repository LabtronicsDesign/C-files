


/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/





/*
History:																						
--------------
*/




/***************************/
/*** Debug output macros ***/
/***************************/

#include "serial.h"

#define PRINT sprintf(print_temp,                  /* Start print debug macro */
#define PREND ); serial_put_string(print_temp);    /* End print debug macro */


#define PRINT_TEMP_BUFFER_SIZE 260
extern char print_temp[PRINT_TEMP_BUFFER_SIZE];    /* Workspace to print to - don't exceed this size in a print */

#define STR(a)  serial_put_string(a) 			   /* Send a string to the debug output  */


#define SERIAL_WAIT serial_wait_for_output()	   /* Wait for debug output to finnish going out before continuing. */
#define SERIAL_WAIT_NO_SWAP serial_wait_for_output_no_swap()  /* ...same without swapping. */


