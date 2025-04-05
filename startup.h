
/*

 Micro-Robotics Ltd.


THIS FILE IS THE PROPERTY OF MICRO-ROBOTICS LTD.
IT MAY NOT BE USED, COPIED OR EDITTED WITHOUT THE 
EXPRESS PRIOR WRITTEN PERMISSION OF MICRO-ROBOTICS LTD.

Copyright 2000 Micro-Robotics Ltd.

*/

/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/

		


/******************************************************************
NAME		: startup_sequence
PURPOSE		: Determine the sequence of actions to take when the 
		      unit is switched on.
			  If Prog mode then gives the Clear Memory message.
			  If there is a Rommed application then run that
			  unless the Skip option used.
			  
PARAMETERS	: NONE
RETURNS		: NONE (Does not return!)
SIDE-EFFECTS: Lots
CALLED FROM : Main.c
*********************************************************************/
void startup_sequence();



/******************************************************************
NAME		: find_startup_procedure
PURPOSE		: find_startup_procedure! (to run an application.)
PARAMETERS	: NONE
RETURNS		: Global Number of the startup proc if any, or -1.
SIDE-EFFECTS: NONE
CALLED FROM : Startup sequence.
*********************************************************************/
short find_startup_procedure();



/******************************************************************
NAME		: create_default_startup_procedure
PURPOSE		: Create the default startup if one does not exist.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: creates a proc.
CALLED FROM : the startup sequence.
*********************************************************************/

void create_default_startup_procedure();
