

/*

DESCRIPTION:  Millisecond timing.
AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/






#pragma language=extended	/* Enable use of extended keywords */

#include <inh8.h>    /* intrisics */

#include "hardware.h"
#include "time.h"
#include "general.h"

//#include "debug.h" /* */



/* These ones are in the near area: 16 bit addressable. */
#pragma memory=near

/* These two are the system time.  [2^48 Ms == 9000 YEARS!] */
//volatile unsigned short time_high=0; 
volatile unsigned long time_low=0;

#pragma memory=default




/******************************************************************
NAME		: wait
PURPOSE		: Waits for a number of milliseconds, swapping while it waits.
			  It will always swap out before testing
PARAMETERS	: long ms: The No. of milli-seconds to wait.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void wait (long ms)
{
   big_time then;


   /* read the time now */
   then . time_low = time_low;
      
   /* add the milliseconds value to it */
   then.time_low += ms;
      
   /* do the wait with a comparison of the time */
   
	while(1)
	{
		sleep();   /* Only need to wake when there's something to do... */

		if ( ( time_low >= then.time_low ) /*&& ( time_high == then.time_high )*/ )
			break;
	}
}



/*-------------------**
**  Timer 'object' **
**-------------------*/



/******************************************************************
NAME		: timer_init
PURPOSE		: Set up a timer with a period and set it's base time to now.
PARAMETERS	: timer* t : Pointer to the timer.
			  unsigned long period : The period to initialise it with.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void timer_init(timer* t, unsigned long period)
{
	/* Set the period for the timer. */
	t->period  = period;

	/* Set the basetime for the timer */
	//INTS_OFF;
	t->base . time_low  = time_low;
	//t->base . time_high = time_high;
	//INTS_ON;
}	

/******************************************************************
NAME		: timer_go
PURPOSE		: Set the timer timing.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void timer_go(timer* t)
{
   big_time now;

   /* read the time NOW */
   //INTS_OFF;

   now . time_low = time_low;
   //now . time_high = time_high;

   /* add the period  to it */

   now.time_low += t->period;
   //if (read_ccr() & 0x1)     /* if the carry bit set then increment the high word */
   //   now.time_high++;

   /* set the timer */
   t->base = now;
   //INTS_ON;  /* in the wrong place? */

}


/******************************************************************
NAME		: timer_done
PURPOSE		: Return whether the time has passed.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: TRUE if the timer has timed out.
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


char timer_done(timer* t)
{
	register char done=0;

	//INTS_OFF;
	if ( ( time_low >= t->base.time_low ) /*&& ( time_high == t-> base.time_high )*/ )
		done = TRUE;
	//else if ( time_high > t-> base.time_high )
	//	done = TRUE;
	//INTS_ON;
	return done; 
}
 

/******************************************************************
NAME		: timer_seconds
PURPOSE		: Return the number of seconds left to run on the timer.
COMMENT		: Only returns meaningful numbers up to 25 day periods!!
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: TRUE if the timer has timed out.
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


long timer_seconds(timer* t)
{
   long time_left;
   time_left = t->base.time_low - time_low;
   return time_left / 1000;
}


/******************************************************************
NAME		: timer_reset
PURPOSE		: Reset the timer.
COMMENT		: Only returns meaningful numbers up to 25 day periods!!
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void timer_reset(timer* t)
{
  //INTS_OFF;
  t->base.time_low = 0;
  //t->base.time_high = 0;
  //INTS_ON;
}


/******************************************************************
NAME		: timer_advance
PURPOSE		: Advance the timer by one tick period with no waiting.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

static void timer_advance(timer *t)
{
   //INTS_OFF;
   t->base.time_low += t->period;
   //if (read_ccr() & 0x1)     /* if the carry bit set then increment the high word */
   //   t->base.time_high++;
   //INTS_ON;
}



/******** STOP WATCH ADDITIONS TO THE TIMER OBJECT **********/

/* Start the stopwatch timing. */
void stop_watch_start(timer *t)
{

	/* Indicate the stopwatch is going. */
	t->period  = 1;

	/* Set the basetime for the timer */
	//INTS_OFF;
	t->base . time_low  = time_low;
	//t->base . time_high = time_high;
	//INTS_ON;
}	



/* [Only works for 50 days from turn on!] */
long stop_watch_seconds(timer* t)
{
	if (t->period)
   		return (time_low - t->base.time_low) / 1000;
	return 0;
}

/* [Only works for 50 days from turn on!] */
long stop_watch_time(timer* t)
{
	if (t->period)
   		return (time_low - t->base.time_low);
	return 0;
}

