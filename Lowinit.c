
/*
 * low_level_init makes it possible to customize 
 * initialization before main function is executed.
 *
 *
 * Copyright:      1996-1998 IAR Systems. All rights reserved. 
 *
 * Archived:       $Revision: 1.4 $ 
 *
 * Created: 10/Sep/96  ITHA
 *
 */

#pragma language=extended

#pragma codeseg(RCODE)


#include "hardware.h"


#if ((__TID__ & 0x0F) == 0x00)

/* small memory model           */
unsigned char near_func low_level_init (void)

#else

/* large and huge memory models */
unsigned char far_func low_level_init (void)

#endif

  {
    /*==================================*/
    /*  Check your watch dog if needed	*/
    /*==================================*/
    
    /*==================================*/
    /*  Init I/O which must be enabled	*/
    /*  before segment initializations	*/
    /*  (as address lines).		*/
    /*==================================*/

    /* set Bus Controller for no program wait states */
	wcrh = 0;
	wcrl = 0;
	astcr = 0; /* All areas are two state access. */



    pgddr = CS0 | CS1 ; /* turn on the [CS0], CS1 signals - for our external RAM (and leave CS0 on) */



    		 
    /*==================================*/
    /* Choose if segment initialization	*/
    /* should be done or not.		*/
    /* Return: 0 to omit seg_init	*/
    /*	       1 to run seg_init	*/
    /*==================================*/
    return (1);
  }
