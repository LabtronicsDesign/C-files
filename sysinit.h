
/*


Copyright 2001 Eumedic Ltd.

*/

/*

DESCRIPTION:   Hardware-specific initialisation.
AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/

char in_program_mode();

		
/******************************************************************
NAME		: find_ram_size
PURPOSE		: Called at startup to determine what size of RAM we have.
PARAMETERS	: NONE
RETURNS		: unsigned short: no. of K bytes found
SIDE-EFFECTS: write to 4 bytes at the start of 512K mem. map
CALLED FROM : Main.c
*********************************************************************/

unsigned short find_ram_size();					  

/******************************************************************
NAME		: system_init
PURPOSE		: Initialise the underlying operating system before
			  starting the language
PARAMETERS	: NONE
RETURNS		: NONE 
SIDE-EFFECTS: Lots
CALLED FROM : Main.c
*********************************************************************/

void system_init();