



/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
2001 08 21
Removed time_high for this project as it will never need it, and it simplifies things.
It is ~50 days before it will overflow - much longer than the battery life.

*/






/* These ones are in the near area: 16 bit addressable. */
#pragma memory=near
/* These two are the system time.  [2^48 Ms == 9000 YEARS!] */
//extern volatile unsigned short time_high; 
extern volatile unsigned long time_low;

#pragma memory=default




/* big_time: This is used in timer.  The size should be kept constant unless dependancies on
the format of variable and timer are taken into account.
*/
typedef struct
{
	//unsigned short time_high;
	unsigned long time_low;
}
big_time;



/******************************************************************
NAME		: wait
PURPOSE		: Waits for a number of milliseconds, swapping while it waits.
			  It will always swap out before testing
PARAMETERS	: long ms: The No. of milli-seconds to wait.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void wait (long ms);



/*-------------------**
**  A timer 'object' **
**-------------------*/


/* If the size of this is changed, EVERY will need checking! */
typedef struct
{
  big_time base;  /* This takes 6 bytes */
  short padding;	/* to make sure timers on the BCI stack don't overwrite val_type of 
					    the 'variables' they pretend to be on the stack. */
  unsigned long period;
}timer;

/******************************************************************
NAME		: timer_init
PURPOSE		: Set up a timer with a period and set it's base time to now.
PARAMETERS	: timer* t : Pointer to the timer.
			  unsigned long period : The period to initialise it with.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void timer_init( timer* t , unsigned long period);


/******************************************************************
NAME		: timer_go
PURPOSE		: Set the timer timing.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void timer_go(timer* t);

/******************************************************************
NAME		: timer_done
PURPOSE		: Return whether the time has passed.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: TRUE if the timer has timed out.
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
char timer_done(timer* t);



/******************************************************************
NAME		: timer_seconds
PURPOSE		: Return the number of seconds left to run on the timer.
COMMENT		: Only returns meaningful numbers up to 25 day periods!!
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: TRUE if the timer has timed out.
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
long timer_seconds(timer* t);

/******************************************************************
NAME		: timer_reset
PURPOSE		: Reset the timer.
COMMENT		: Only returns meaningful numbers up to 25 day periods!!
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void timer_reset(timer* t);


/******************************************************************
NAME		: every_tick
PURPOSE		: advance timer one period exactly from the last tick,
			  and then wait for timeout.
PARAMETERS	: timer* t : Pointer to the timer.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void every_tick(timer *t);



/******** STOP WATCH ADDITIONS TO THE TIMER OBJECT **********/

/* Start the stopwatch timing. */
void stop_watch_start(timer *t);


/* Stop the stopwatch timing. */
#define stop_watch_stop(T) (T)->period=0
#define stop_watch_going(T)	((T)->period)


/* [Only works for 50 days from turn on!] */
long stop_watch_seconds(timer* t);

/* [Only works for 50 days from turn on!] */
long stop_watch_time(timer* t);

