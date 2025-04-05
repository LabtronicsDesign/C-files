
/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/


/*
	Has all the non-volatile memory definitions.
*/

/* The heap */
#include "memstruc.h"		/* Describes memory space objects */
#define HEAP_SIZE 0x1000   	/* This is how big our heap is */

extern no_init PTR_ATTRIBUTE char bulk_storage [HEAP_SIZE]; 	/* Preserve the heap when power off. */

extern no_init char PTR_ATTRIBUTE *_last_heap_object/* = bulk_storage*/;	 /* Preserve the heap when power off. */



/* The global variables. */
