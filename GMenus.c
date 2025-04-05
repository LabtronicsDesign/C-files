
/*


Copyright 2001 Eumedic Ltd.

*/

#pragma language=extended	/* Enable use of extended keywords */


#include <stdio.h>
#include <inh8.h>	  	/* Intrinsic Funcs */
#include <string.h>

#include "hardware.h" 	
#include "serial.h" 	
#include "time.h" 		
#include "sysinit.h"
#include "graphic.h"
#include "pulse.h"
#include "iic.h"
#include "access.h"
#include "general.h"
#include "debug.h"


char parameters_valid = FALSE; /* TRUE if parameters have been set up - So it's OK to save the to EEPROM at power down. */


/* This enum controls the way menus are navigated. */

typedef enum 
{
	MENU_MAIN,

	MENU_GAP1,			/* A break in the sequence of menus. */

	MENU_FREQ_CYCLE,
	MENU_FREQUENCY,
	MENU_MODULATION,
	MENU_FILTER,
	MENU_INTENSITY,
	MENU_INTERPULSE,
	MENU_OPTIONS,

	MENU_GAP2,			/* A break in the sequence of menus. */

	MENU_CONTRAST,
	MENU_HOURS,

	MENU_GAP3,			/* A break in the sequence of menus. */

	MENU_POWER_OFF		
} menu_label;

/* These initial values set up the initial menus. */
#define MENU_FIRST_IN_LIST (MENU_GAP1 + 1)
#define MENU_LAST_IN_LIST (MENU_GAP2 - 1)

menu_label menu = MENU_MAIN;
menu_label last_menu = MENU_FIRST_IN_LIST;



 
/******************** PARAMETER STORAGE CODE *******************/


/******************************************************************
NAME		: 
PURPOSE		: 
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/



/* Find the checksum of a block of data. */
char calc_checksum(char * data, size_t size)
{
	char sum = 'K'; /* Sum initialiser. */
	for (; size; size--)
	{
		sum += *data++;
	}
	return sum;
}	

void print_params()
{
	PRINT "Ps:C:%i S:%i F:%i I:%i z:%i m:%i f:%i Fc:%i T:%li x:%i\r\n",
		/* not in order of storage... */
		param.contrast, 			
		param.strength, 			
		param.base_freq,			
		param.pulse_out_number,	
		param.z_value,			
		param.modulation,		
		param.filter,			
		param.freq_cycling,		
		param.acc_ther_time,		
		param.checksum			
	 PREND
}	
	

/* Save the instrument parameters in the EEPROM. */
void save_parameters()
{
	//STR("saving params...\r\n");
	param.checksum = calc_checksum( (char *) &param , sizeof(param) - 1);
	write_block_to_eeprom(0 , sizeof(param) , (char *) &param);
}	

/* This doesn't reset the hours log, cutoff time, credit seq no. */
void set_default_params()
{
	param.strength 			 = STRENGTH_MIN;  
	param.base_freq 		 = FREQ_DEF;	   
	param.pulse_out_number 	 = INTENSITY_MIN; 
	param.z_value 			 = Z_DEF;		   
	param.modulation 		 = MODULATION_MIN;
	param.filter 			 = 0;			   
	param.freq_cycling 		 = 0;			   
	param.contrast 		 	 = 9;			   

	/* We should now have some valid parameters, if we need to save any. */
	parameters_valid = TRUE;

}	

/* return true if out of range. */
char range(short value, short lower, short upper)
{
	if (value < lower)	
		return TRUE;
	else if (value > upper)
		return TRUE;
		
	return FALSE;
}

/* Read the parameters from the EEPROM, return TRUE if the defaults needed setting. */
char read_parameters()
{
	char err = FALSE;
	char bad_params = FALSE;

	read_block_from_eeprom(0 , sizeof(param) , (char *) &param);
	
	//STR("Read parameters:\r\n");

	//print_params();

	err |= range(param.strength,			STRENGTH_MIN,		STRENGTH_MAX);
	err |= range(param.base_freq,			FREQ_MIN ,			FREQ_MAX );
	err |= range(param.pulse_out_number,	INTENSITY_MIN, 		INTENSITY_MAX);
	err |= range(param.z_value,				Z_MIN, 				Z_MAX);
	err |= range(param.modulation,			MODULATION_MIN ,	MODULATION_MAX );
	err |= range(param.filter,				0,					1 );
	err |= range(param.freq_cycling,		0, 					1);
	err |= range(param.contrast,			0, 					31);

	if (err)
	{
		STR("parameter out of range.\r\n");
		set_default_params();
		bad_params = TRUE;
	}

	if (param.checksum != calc_checksum( (char *) &param , sizeof(param) - 1))
	{

		//PRINT "%i %i\r\n",param.checksum,calc_checksum( (char *) &param , sizeof(param) - 1) PREND
		STR("bad sum!! set defaults\r\n");
		set_default_params();
		bad_params = TRUE;
	}


	/* We  now have some valid parameters, if we need to save them. */
	parameters_valid = TRUE;

	return(bad_params);
}


/******************** END PARAMETER STORAGE CODE *******************/


/******************************************************************
NAME		: power_down
PURPOSE		: Power down the unit.  Must use only this routine to power
				down as it does housekeeping.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void power_down()
{
 	/* Save all the state we may want to keep. */

	if (parameters_valid)
	{
		save_parameters();
		wait_for_eeprom_to_finish(); /* Make sure the EEPROM is not deprived of power before it has done it's stuff. */
		//print_params();
	}
	
	clear_screen();
 	update_display();

	
	POWER_HOLD_OFF;	 /* power off */

	wait(2000);

   	glcd_print_string(4,5,2,"Power off");
	update_display();

	while (1)
	{
		wait (100); /* save power! */
	}
}




/******************************************************************
NAME		: read_button
PURPOSE		: low-level routine to return which button is pressed.
PARAMETERS	: NONE
RETURNS		: char: the button.
SIDE-EFFECTS: none.
CALLED FROM : read_keypad_action & ...
*********************************************************************/

#define BUTTON1_BIT 0x01
#define BUTTON2_BIT 0x02
#define BUTTON3_BIT 0x04
#define BUTTON4_BIT 0x08

#define TOP_BUTTON		4
#define MIDTOP_BUTTON	3   
#define MIDBOT_BUTTON	2
#define BOTTOM_BUTTON	1


char read_button()
{
	register char bits = ~porte;
	register char button = 0;

	KICK_DOG; /* The watchdog. */

	if (! (bits & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT | BUTTON4_BIT)) )
		return 0;
		
	if (bits & BUTTON1_BIT)	
		button = BOTTOM_BUTTON;
	else if (bits & BUTTON2_BIT)	
		button = MIDBOT_BUTTON;
	else if (bits & BUTTON3_BIT)	
		button = MIDTOP_BUTTON;
	else if (bits & BUTTON4_BIT)	
		button = TOP_BUTTON;

	if (!upside_down)
		return button;
		
	return (TOP_BUTTON+1) - button;
}



/* If this is called at a regular rate, returns the button pressed, and takes care of autoincrement. */

#define REPEAT_MAX 1
#define INIT_WAIT_MAX 10

char button_auto_wait[] = 
{
	0, /* Buttons are 1-4 */
	INIT_WAIT_MAX,
	INIT_WAIT_MAX,
	INIT_WAIT_MAX,
	INIT_WAIT_MAX,
};

char auto_repeating = FALSE;


/******************************************************************
NAME		: read_keypad_action
PURPOSE		: reads the keypad and performs auto-repeat function.
PARAMETERS	: the key pressed or 0
RETURNS		: NONE
SIDE-EFFECTS: auto_repeating global is set if we are auto-repeating.
CALLED FROM : all menus etc.
*********************************************************************/


char read_keypad_action()
{
static char last_button = 0;
static char repeat_count = 0;
static char init_wait_count = 0;
	
	char button = read_button();

	if (button)
	{
		if (button == last_button)
		{
			if (init_wait_count < button_auto_wait[button] /*INIT_WAIT_MAX */)
			{
				init_wait_count++;
				return 0;
			}
			else
			{
				repeat_count++;
				if (repeat_count > REPEAT_MAX)
				{
				   repeat_count = 0;
				   beep_for(1);
				   auto_repeating = TRUE;
				   return button;
				   
				}
				return 0;
				
			}
		}
	}

	repeat_count = 0;
	init_wait_count = 0;
	last_button = button;

	auto_repeating = FALSE;

	return button;
	
}	



/******************************************************************
NAME		: print_battery_state_at_col
PURPOSE		: Prints the battery state icon at the given column, on
				bottom two rows.
PARAMETERS	: char col: the column
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void print_battery_state_at_col(char col)
{
#define BATTERY_ICON 126
float read_battery();
signed char i;
//static last_i = 255;	would need to reset it...


	i =  (char)( (read_battery() - 7.0) * 3.5) ;
	if (i < 0) i = 0;
	if (i > 6) i = 6;

	//if (i != last_i)
	{
		plot_sprite(7,col, 2,BATTERY_ICON + i);
		//last_i =  i;
	}
}	



char menu_sprite_col;
char text_lh = 0;
char g_str[100];  /* ! temp print area ! */



/* Icon numbers (in font 2) */
#define ON_OFF_ICON 155
#define PLUS_ICON 137
#define MINUS_ICON 138
#define NEXT_ICON 142
#define DOWN_ICON 143
#define PREV_ICON 148


/******************************************************************
NAME		: menu_icons
PURPOSE		: Prints the soft-key icons down the correct side of the LCD.
PARAMETERS	: the numbers of the four sprites to use, top first.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : All the menus; and startup screens.
*********************************************************************/

void menu_icons(char spr1,char spr2,char spr3,char spr4)
{
	menu_sprite_col = (upside_down)? 0: 91;

	plot_sprite(1,menu_sprite_col,2,spr1);
	plot_sprite(3,menu_sprite_col,2,spr2);
	plot_sprite(5,menu_sprite_col,2,spr3);
	plot_sprite(7,menu_sprite_col,2,spr4);
}

/******************************************************************
NAME		: settings_icons
PURPOSE		: Prints the icon display on the bottom of the LCD
				to indicate the settings.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : Various menus.
*********************************************************************/

void settings_icons()
{

#define SET_ICN_OFF1 30
#define SET_ICN_OFF2 48

	char mod_or_int;
	char num_dash;

	if (param.modulation)					/* Modulation Icon */
	{
		mod_or_int = 'M';
		num_dash = '0' + param.modulation;	
	}
	else if (param.pulse_out_number > 1)	/* Intensity Icon. */
	{
		mod_or_int = 'I';
		num_dash =  '0' + param.pulse_out_number;	
	}
	else
	{
		mod_or_int = ' ';
		num_dash =  ' ';	
	}

	sprintf(g_str,"%c%c",mod_or_int,num_dash);
	glcd_print_string(7,text_lh+SET_ICN_OFF1,1,g_str);

	if (param.freq_cycling)
	{
		glcd_print_string(7,text_lh+SET_ICN_OFF2,1,"FrCyc");
	}
	else
	{
		sprintf(g_str,"%3iHz",param.base_freq);
		glcd_print_string(7,text_lh+SET_ICN_OFF2,1,g_str);
	}
}


/******************************************************************
NAME		: set_menu_state
PURPOSE		: Sets some of the global state for the menu system
				particularly, the state do to with 
			    whether the display is upside-down.
PARAMETERS	: char upside_down_p: true if we want the display inverted.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void set_menu_state(char upside_down_p)
{
 	set_viewpoint(upside_down_p);
	text_lh =  upside_down? 10 : 0;
}	



/******************************************************************
NAME		: print_big_value
PURPOSE		: Print a number at a place that's useful for many menus.
PARAMETERS	: short value - the value to print.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : Many menus.
*********************************************************************/


void print_big_value(short value)
{
	sprintf(g_str,"%3i",value);		  
	glcd_print_string(3,text_lh+35,2,g_str); 
}

/******************************************************************
NAME		: disp_modulation
PURPOSE		: Display the value of modulation or 'Off'.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void disp_modulation()
{
	if (param.modulation)
	{
		print_big_value(param.modulation);
	}
	else
	{
   		glcd_print_string(3,text_lh+35,2,"Off");
	}
}	



#define PRINT_STRENGTH_BIG	 print_big_value(param.strength);

#define PRINT_STRENGTH_SMALL			  	   \
	sprintf(g_str,"%3i",param.strength);		  	   \
	glcd_print_string(0,text_lh + 68,1,g_str); \

/******************************************************************
NAME		: main_screen
PURPOSE		: Print the static elements of the main screen.
				- used when first setting up the main screen,
				and when coming oout of the skin-screen.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: Timers are set...
CALLED FROM : 
*********************************************************************/

#define NORMAL_POWER_OFF_TIME (30 * 1000L)
void main_screen()
{

	/* Set the power off timer. */
	power_off_timer.period = NORMAL_POWER_OFF_TIME ;
	timer_go(&power_off_timer);

	clear_screen();
   	glcd_print_string(1,text_lh,2,"Strength");
	if (param.pulse_out_number > 1)
	{
		sprintf(g_str,"x%i",param.pulse_out_number);
		glcd_print_string(3,text_lh+65,1,g_str); 		
	}
	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, ON_OFF_ICON);
	button_auto_wait[1] = 30;
	settings_icons();

	/* Print the value */
	PRINT_STRENGTH_BIG
}
	
/******************************************************************
NAME		: skin_screen
PURPOSE		: Print the static elements of the skin screen.
				used when coming out of the main-screen.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: Timers are set...
CALLED FROM : 
*********************************************************************/

void skin_screen()
{

#define SKIN_POWER_OFF_TIME	(10L * 60L * 1000L)

	/* Set the power off timer. */
	power_off_timer.period = param.filter? SKIN_POWER_OFF_TIME : NORMAL_POWER_OFF_TIME;
	timer_go(&power_off_timer);

	clear_screen();
   	glcd_print_string(0,text_lh+50,1,"Str");

	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, ON_OFF_ICON);
	button_auto_wait[1] = 30;


	settings_icons();

	/* Print the value */
	PRINT_STRENGTH_SMALL

	if (param.filter)
	{
		glcd_print_string(3,text_lh,1,"Filter");
		settings_icons();
	}
	else if (param.modulation)
	{	 
		glcd_print_string(3,text_lh,1,"Modulation");
		settings_icons();
	}
	else if (param.freq_cycling)
	{
		glcd_print_string(3,text_lh,1,"Freq Cycle");
		settings_icons();
	}
	else
	{
		glcd_print_string(3,text_lh+70,1,"\x8D"); /* D/T symbol */
	}
}




	

/******************************************************************
NAME		: main_menu
PURPOSE		: The main menu for the unit.  Allows the strength to be
				set.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void write_analogue(char c);

void main_menu()
{
	char button;
	char loop_count = 0;
	char flip_count = 0;
	char skin_screen_on = FALSE;
	short last_mins;

	/* Reset Some timers. */
	menu_timer.period = 2000;
	display_timer.period = 3000;

	main_screen();
	update_display();

	while (read_button()) ;


	while (1)
	{
		wait (30);

		button = read_keypad_action();
		
		switch (button)
		{

			case TOP_BUTTON:
				if (param.strength < STRENGTH_MAX)
				{
					param.strength++;
				   	set_pulse_width(param.strength); 
				   	set_pulse_period(); 
				}
				else
					end_beep();

				break;

			case MIDTOP_BUTTON:
				if (param.strength > STRENGTH_MIN)
				{
				   	param.strength--;
				   	set_pulse_width(param.strength); 
				   	set_pulse_period(); 
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				/* Make sure we are not partway through a modulation when we go to the menus. */
				reset_modulation();
				menu = last_menu;
				return ;

			case BOTTOM_BUTTON:

				if (auto_repeating)	/* Request for power off. */
				{
					power_down();
				}

				flip_count++;
				plot_sprite(7,menu_sprite_col,2,ON_OFF_ICON + flip_count);
				update_display();

				if (flip_count > 2)
				{
					beep_for(5);
					set_menu_state(!upside_down);
					return;
				}
				break;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{
			if (button >= 3 && flip_count)
			{
				flip_count = 0;
				menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, ON_OFF_ICON);
				update_display();
			}

	        /* Hold off auto power-down. */
            timer_go(&power_off_timer);

			/* Update the value */
			if (skin_screen_on)
			{
				PRINT_STRENGTH_SMALL
			}
			else
			{
				PRINT_STRENGTH_BIG
			}

			/* Dont allow modulation whist adjusting strength. */
			reset_modulation();
		}
		

		/* Control the therapy. */
		therapy_control();
		modulate();


		/* Display the information. */
		if (  ((loop_count++ & 0x7) == 0) )
		{
		 char freeze_time = !timer_done(&display_timer);

			if (skin_display || freeze_time) /* If diagnostics frozen, then keep the display there. */
			{
				char mins; 
				char secs; 
			
			  	mins = seconds / 60;
			  	secs = seconds % 60;

				if (!skin_screen_on)
				{
					skin_screen();
					skin_screen_on = TRUE;
				}

				if (!freeze_time)	 /* Don't update diags if we have frozen display. */
				{
					sprintf(g_str,"%01i:%02i", mins,secs);		  
					glcd_print_string(0,text_lh,1,g_str);

					/* [No diagnostics in these cases:] */
					if ( !(param.filter || param.modulation || param.freq_cycling) )
					{
						/* Diagnostics */
						sprintf(g_str,"%3i %3i",initial_width,delta_width);	
						glcd_print_string(3,text_lh,2,g_str);

						sprintf(g_str,"%3i%c",this_width, dose? PLUS_ICON : ' ');		  
						glcd_print_string(5,text_lh,2,g_str);


						if (seconds >= RESPONSE_WAIT) /* Only show response if it's been evaluated. */
						{
							sprintf(g_str,"%2i %2i",n_rings>>1 , response);		  
							glcd_print_string(5,text_lh+46 ,1,g_str );
						}
						else
						{
							sprintf(g_str,"%2i  -",n_rings>>1);		  
							glcd_print_string(5,text_lh+46 ,1,g_str );
						}

						if (no_reading_count >= NO_READING_COUNT)
						{
							glcd_print_string(6,text_lh,1,"* No Info *");
						}

					}
					else
					{void bar_graph(char line, char col , char size , short max , short variable);
						if (param.modulation)
						{
							bar_graph( 4, text_lh , 60 ,param.strength , modulated_amp);
						}
						else if (param.freq_cycling)
						{
							bar_graph( 4, text_lh , 60 ,MOD_FREQ_MAX , frequency);
							/* Beep once for each minute in FreCyc mode. */
							if ( mins != last_mins)
							{
								last_mins = mins;
								if (mins)
								{
									pip_n_times(mins);
								}
							}
						}
						else if (param.filter)
						{
							//PRINT "TS%li\r\n",timer_seconds(&power_off_timer) PREND
			                /* Power off warning control - has to be taken care of separately when in filter mode... */
			                if ( timer_done(&power_off_timer) )
			                {
			                    menu = MENU_POWER_OFF;
			                    return ;
			                }
						}
					}
				}

				if (freeze_display)	/* Got a signal from the therapy code to freeze and warble. */
				{
					update_display();

					timer_go(&display_timer);
					warble(freeze_display);
					freeze_display = FALSE;
				}

	            /* Hold off auto power-down. */
				if (!param.filter)
	            	timer_go(&power_off_timer);
			}
			else  /* Off-Skin display. */
			{
				if (skin_screen_on)
				{
					main_screen();
					skin_screen_on = FALSE;
				}

				print_battery_state_at_col( text_lh+5);
				#if 0  // Some useful debug.
				sprintf(g_str,"%3i %3i",this_width,n_rings>>1 );		  
				glcd_print_string(5,text_lh+18,1,g_str);
				#endif
                /* Power off warning control */
                if ( timer_done(&power_off_timer) )
                {
					power_down();
                }
			}
		}

		update_display();

	}
}	




/******************************************************************
NAME		: menu_timeout
PURPOSE		: Determing if a menu has timed out and should return 
			  to the main menu.
PARAMETERS	: char remember: TRUE if system should remember the current 
			  menu to come back to.
RETURNS		: If menu has timed out.
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


char menu_timeout(char remember) 
{
	if (timer_done(&menu_timer))
	{
		if (remember)
			last_menu = menu;
		menu= 0;
		beep_for(1);
		return TRUE;
	}
	return FALSE;
}	


void invert_char(char row, char col);

/******************************************************************
NAME		: menu_map
PURPOSE		: Display a little map of the menus to give some indication
				of where the user is in the menu system.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void menu_map()
{
   	glcd_print_string(6,text_lh+30,1,"cfMFIg-");
	plot_sprite(5 ,text_lh+30+((menu-MENU_FIRST_IN_LIST)*6) , 1, 134);
	

   	//glcd_print_string(6,text_lh+30+((menu-MENU_MODULATION)*6),1,"*");
}	

/******************************************************************
NAME		: gmenu_modulation
PURPOSE		: Menu allowing setting of modulation.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: sets modulation.
CALLED FROM : 
*********************************************************************/

	
void menu_modulation()
{
#define PRINT_MOD  disp_modulation();
	char button;


	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Modulate");

	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;

	settings_icons();
	PRINT_MOD

	menu_map();
	
	update_display();

	timer_go(&menu_timer);

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
			return ;
			
		switch (button)
		{

			case TOP_BUTTON:
				if (param.modulation < MODULATION_MAX)
				{
					++param.modulation;
					intensity_off();
					freq_cycling_off();

				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.modulation > MODULATION_MIN)
				{
					--param.modulation;
					if (!param.modulation)
						modulation_off();
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu++;
				return;

			case BOTTOM_BUTTON:
				menu--;
				return;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			PRINT_MOD
			settings_icons();
			update_display();

			/* Audible signal */
	 		//pip_n_times(param.modulation);

			reset_modulation();

			timer_go(&menu_timer);	/* Keep the menu alive */
		}

		/* Do the therapy stuff while in this menu */
		therapy_control();
		modulate();


	}


}

/******************************************************************
NAME		: gmenu_freq_cycle
PURPOSE		: Menu to turn frequency cycling on and off.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: ...
CALLED FROM : 
*********************************************************************/

#define PRINT_ON_OFF(A) glcd_print_string(3,text_lh+35,2,(A)? "On ":"Off");
	
void menu_freq_cycle()
{
	char button;


	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Freq Cyc");
	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;
	settings_icons();
	PRINT_ON_OFF(param.freq_cycling)

	menu_map();

	update_display();

	timer_go(&menu_timer);
	while (1)
	{
		wait (30);
		button = read_keypad_action();  
		if (menu_timeout(TRUE)) 
			return ;
		switch (button)
		{
			case TOP_BUTTON:
				if (!param.freq_cycling)
				{
					param.freq_cycling = 1;
					modulation_off();
				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.freq_cycling)
				{	/* we want to stick at the freq we got to... */
					//freq_cycling_off();  ... so we don't call this.
					param.freq_cycling = FALSE; 	/* Turn off cycling */
					param.base_freq = frequency;	/* and stick here. */
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu++;
				return;
			case BOTTOM_BUTTON:
				menu--;
				return;
		}
		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			PRINT_ON_OFF(param.freq_cycling)
			settings_icons();
			update_display();
			timer_go(&menu_timer);	/* Keep the menu alive */
		}
	}
	
}

/******************************************************************
NAME		: gmenu_filter
PURPOSE		: Menu to turn the filter setting on and off.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: filter.
CALLED FROM : 
*********************************************************************/
		
void menu_filter()
{

	char button;


	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Filter");
	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;
	settings_icons();
	PRINT_ON_OFF(param.filter)

	menu_map();

	update_display();

	timer_go(&menu_timer);
	while (1)
	{
		wait (30);
		button = read_keypad_action();  
		if (menu_timeout(TRUE)) 
			return ;
		switch (button)
		{
			case TOP_BUTTON:
				if (!param.filter)
					filter_bit = param.filter = 1;
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.filter)
					filter_bit = param.filter = 0;
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu++;
				return;
			case BOTTOM_BUTTON:
				menu--;
				return;
		}
		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			PRINT_ON_OFF(param.filter)
			settings_icons();
			update_display();
			timer_go(&menu_timer);	/* Keep the menu alive */
		}
	}
}

/******************************************************************
NAME		: gmenu_intensity
PURPOSE		: Menu to set the intensity.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: ...
CALLED FROM : 
*********************************************************************/
	
void menu_intensity()
{
#define PRINT_INT print_big_value(param.pulse_out_number);


	char button;

	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Intensity");
	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;
	settings_icons();
	PRINT_INT
	menu_map();
	update_display();

	timer_go(&menu_timer);



	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
			return ;
			
		switch (button)
		{

			case TOP_BUTTON:
				if (param.pulse_out_number < INTENSITY_MAX)
				{
					param.pulse_out_number++;
					set_pulse_period(); 

					modulation_off();	/* Turn off mod if we turn on intensity. */

				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.pulse_out_number > INTENSITY_MIN)
				{
					param.pulse_out_number--;
					set_pulse_period(); 
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu++;
				return;

			case BOTTOM_BUTTON:
				menu--;
				return;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			PRINT_INT
			settings_icons();
			update_display();

			/* Audible signal */
	 		//pip_n_times(param.pulse_out_number);

			timer_go(&menu_timer);	/* Keep the menu alive */
		}
	}
}
	

/******************************************************************
NAME		: gmenu_interpulse
PURPOSE		: Menu to set the inter-pulse time for intensity.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: ...
CALLED FROM : 
*********************************************************************/

void menu_interpulse()
{
	char button;


	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Int Gap");
	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;
	settings_icons();
	print_big_value(param.z_value);
	menu_map();
	update_display();

	timer_go(&menu_timer);

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
			return ;
			
		switch (button)
		{

			case TOP_BUTTON:
				if (param.z_value < Z_MAX)
				{
					param.z_value++;
					set_pulse_period(); 
				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.z_value > Z_MIN)
				{
					param.z_value--;
					set_pulse_period(); 
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu++;
				return;

			case BOTTOM_BUTTON:
				menu--;
				return;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			print_big_value(param.z_value);
			update_display();
			timer_go(&menu_timer);	/* Keep the menu alive */
		}
	}
}

/******************************************************************
NAME		: gmenu_freq
PURPOSE		: menu to set the frequency.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: ...
CALLED FROM : 
*********************************************************************/

	
void menu_freq()
{


	char button;
	char increment=0;


	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Frequency");
	if (param.freq_cycling)
   		glcd_print_string(3,text_lh+70,1,"Cyc");

	menu_icons(PLUS_ICON, MINUS_ICON, NEXT_ICON, PREV_ICON);
	button_auto_wait[1] = INIT_WAIT_MAX;
	settings_icons();
	print_big_value(param.base_freq);
	menu_map();
	update_display();

	timer_go(&menu_timer);

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
			return ;
			
		switch (button)
		{

			case TOP_BUTTON:
				if (frequency < FREQ_MAX  && !param.freq_cycling)
				{
                    /* Go faster at higher values - and take care to hit the same values coming down. */
					increment = (frequency) / 50;
					if (!increment)
						increment = 1;


					param.base_freq = frequency +=increment;
					set_pulse_period(); 
				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (frequency > FREQ_MIN  && !param.freq_cycling)
				{
                    /* Go faster at higher values - and take care to hit the same values coming down. */
					increment = (frequency-increment) / 50;
					if (!increment)
						increment = 1;


					param.base_freq = frequency-=increment;
					set_pulse_period(); 
				}
				else
					end_beep();
				break;


			case MIDBOT_BUTTON:
				menu++;
				return;

			case BOTTOM_BUTTON:
				menu--;
				return;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{
			/* Update the value */
			print_big_value(param.base_freq);
			settings_icons();
			update_display();
			timer_go(&menu_timer);	/* Keep the menu alive */

	        /* Pip at certain points in the freq scale. */
		    switch(frequency)
		    {
		        case 30:
                    pip_n_times(1);
                    break;
		        case 60:
                    pip_n_times(2);
                    break;
		        case 90:
                    pip_n_times(3);
                    break;
		        case 120:
                    pip_n_times(4);
                    break;
		        case 180:
                    pip_n_times(5);
                    break;
		    }
		}

		therapy_control();
		modulate();
	}
}



/******************************************************************
NAME		: options_menu
PURPOSE		: Menu to allow less often used parameters to be accessed.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void options_menu()
{
	char button;

	menu_timer.period = 2000;
	timer_go(&menu_timer);

	/* Draw the screen. */
	clear_screen();

   	glcd_print_string(0,text_lh,1,"Contrast");
   	glcd_print_string(2,text_lh,1,"Usage");   


	menu_icons(DOWN_ICON, DOWN_ICON, NEXT_ICON, PREV_ICON);
	//print_big_value();
	menu_map();
	update_display();

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
			return ;
			
		switch (button)
		{
			case TOP_BUTTON:
				menu = MENU_CONTRAST;
				return;

			case MIDTOP_BUTTON:
				menu = MENU_HOURS;
				return;

			case MIDBOT_BUTTON:
				menu++;
				return;

			case BOTTOM_BUTTON:
				menu--;
				return;
		}
	}
}


/******************************************************************
NAME		: gmenu_contrast
PURPOSE		: Menu to allow LCD contrast to be set.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void menu_contrast()
{
	char button;

	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Contrast");
	menu_icons(PLUS_ICON, MINUS_ICON, 0, 0);

	print_big_value(param.contrast);
	update_display();

	timer_go(&menu_timer);

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE))
		{
			last_menu = MENU_OPTIONS;
			return ;
		}
			
		switch (button)
		{

			case TOP_BUTTON:
				if (param.contrast < 0x1F)
				{
					param.contrast++;
				}
				else
					end_beep();
				break;

			case MIDTOP_BUTTON:
				if (param.contrast > 0)
				{
					param.contrast--;
				}
				else
					end_beep();
				break;

			case MIDBOT_BUTTON:
				menu = MENU_OPTIONS;
				return;

			case BOTTOM_BUTTON:
				menu = MENU_OPTIONS;
				return;
		}

		/* Things to do if a button is pressed... */
		if (button)
		{void set_contrast(char contr);
			set_contrast(param.contrast);
			/* Update the value */
			print_big_value(param.contrast);
			update_display();
			timer_go(&menu_timer);	/* Keep the menu alive */
		}
	}
}


/******************************************************************
NAME		: gmenu_hours
PURPOSE		: Menu to view the therapy time accumulator.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void menu_hours()
{
	char button,clear_count=0;
	short hours,mins,secs;


	menu_timer.period = 20000; /* Give us some time here. */
	timer_go(&menu_timer);

	/* Draw the screen. */
	clear_screen();
   	glcd_print_string(1,text_lh,2,"Usage");
	menu_icons(0, 0, 0, PREV_ICON);

	//test:
	// param.acc_ther_time = (3600 * 2000L) + (60 * 56L) + 24L; 

	hours = param.acc_ther_time / 3600;
	mins = (param.acc_ther_time / 60) % 60; 
	secs = 	param.acc_ther_time % 60;

	/* Display full details of time on the serial. */
	PRINT "%4iHrs %2iMin %2iSec\r\n",hours, mins, secs PREND

	sprintf(g_str,"%4iHrs %2iMin",hours, mins );
	glcd_print_string(4,0,1,g_str); 

	update_display();

	while (1)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
		{
			last_menu = MENU_OPTIONS;
			return ;
		}
			
		if (button)
		{
			switch (button)
			{
				case 4:
					if (clear_count == 13) 
					{
						param.acc_ther_time = 0;
						param.cutoff_time = 10; 
						beep_for(1000);
					}
					else
						clear_count = 0;

					break;

				case 3:
					clear_count++;
					break;
					
				case 2:
					clear_count = 0;
					break;

				case 1:
					menu = MENU_OPTIONS;
					return;
					//break;
			}
		}
	}
}

/******************************************************************
NAME		: gmenu_power_off
PURPOSE		: Menu to warn that power will be turned off.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void menu_power_off()
{
    char button,i;

	clear_screen();
   	glcd_print_string(2,0,2, "Power off?");
   	glcd_print_string(4,0,2, "any button");
   	glcd_print_string(6,0,2, "to stay ON");
 	update_display();

	while (1)
	{

		wait (50);

		button = read_keypad_action();  
        if (button)
        {
            menu = 0;
            return ;
        }

		
		if (i++ % 60 == 0 )
		{
			beep_for(20);
			beep_for(-20);
		}

        if (i > 250)
        {
        	power_down();
        }
    }
}

/******************************************************************
NAME		: menu_tree
PURPOSE		: Governs the navigation among menus.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void menu_tree()
{
	while (1)
	{
		switch(menu)
		{
			case MENU_MAIN:
				main_menu();
				break;

			case MENU_GAP1:
				menu = MENU_LAST_IN_LIST;
				break;

			case MENU_MODULATION:
				menu_modulation();
				break;

			case MENU_FILTER:
				menu_filter();
				break;

			case MENU_INTENSITY:
				menu_intensity();
				break;

			case MENU_INTERPULSE:
				menu_interpulse();
				break;

			case MENU_FREQUENCY:
				menu_freq();
				break;

			case MENU_FREQ_CYCLE:
				menu_freq_cycle();
				break;


			case MENU_OPTIONS:
				options_menu();
				break;
				

			case MENU_GAP2:
				menu = MENU_FIRST_IN_LIST;
				break;



			case MENU_CONTRAST:
				menu_contrast();
				break;

			case MENU_HOURS:
				menu_hours();
				break;

			case MENU_GAP3:
				menu = MENU_CONTRAST;
				break;

            case MENU_POWER_OFF:
                menu_power_off();
                break;

			default:
				menu = MENU_MAIN;
				break;
		}
	}
}	


void plot_seq_chars_at(char line, char col, char start_c, char n)
{
	char i;

	changed_lines[line-1] = 1;
	changed_lines[line] = 1;

	for (i=0; i< n; i++)
	{
		plot_big_char( line,  col, start_c, 1);
		col += 9;
		start_c ++;
	}
}



/* do the things that set up the unit with the parameters loaded. */
void realise_parameters()
{
	filter_bit = param.filter;
	frequency = param.base_freq;
   	set_pulse_width(param.strength); 
   	set_pulse_period(); 
}


/******************** CREDIT CONTROL **********************/
/******************** CREDIT CONTROL **********************/
/******************** CREDIT CONTROL **********************/


/* The Credit levels in seconds. */
#define CREDIT_WARNING_LEVEL (3 * 3600L)
#define CREDIT_INCREMENT (50 * 3600L)    /* Number of Hours to credit each time. */

#define KEY_FUNC key_func(acc_splat , seq_spat)


/* 
	Menu to cover the generation of the sequence code, and the entry of the credit key.
*/
char get_credit()
{
	char ind;
    char button = 0;
	unsigned short key_bits = 0;

 	unsigned long acc_splat;
	char seq_spat;

	acc_splat = param.acc_ther_time ^ HOURS_LOG_CYPHER;
	seq_spat = 	param.credit_seq ^ CREDIT_SEQ_CYPHER;

	/* Draw the screen. */
	clear_screen();
	menu_icons( '3' ,'2', '1' , '0');

   	glcd_print_string(1,text_lh,2,"Credit");
	glcd_print_string(2,text_lh,1,"Quote code:");
	
	sprintf(g_str,"0%08LX%02X" ,acc_splat  , seq_spat );

	glcd_print_string(3,text_lh,1,g_str);
	glcd_print_string(5,text_lh,1,"Enter key:");
	glcd_print_string(6,text_lh,1,"________");
	update_display();

	/* send answer out on serial ! */
	//PRINT "%08LX\r\n",binary_coded_quartal(KEY_FUNC)  PREND
	
	/* wait for no key - don't know why we have to do this here and not elsewhere! */
	do {wait (30);}	while(read_keypad_action());


	menu_timer.period = 5 * 60 * 1000L; /* Give us some time here. */
	timer_go(&menu_timer);

	g_str[0] = '\0';
	ind = 0;

	while (ind < 8)
	{
		wait (30);

		button = read_keypad_action();  

		if (menu_timeout(TRUE)) 
		{
			 power_down();
		}
			
		if (button)
		{

			g_str[ind++] = button-1 + '0'; // 0123
			g_str[ind] = '\0';

			key_bits <<= 2;
			key_bits |= button-1;

			glcd_print_string(6,text_lh ,1,g_str);
			update_display();

			timer_go(&menu_timer);
		}
	}

	if (key_bits == KEY_FUNC )
	{
		param.credit_seq++;	  /* Go to next in seq. */

		if (param.cutoff_time < param.acc_ther_time) /* ignore -ve credit run up. */
		   param.cutoff_time = param.acc_ther_time;    

		param.cutoff_time += CREDIT_INCREMENT ; /* Increase credit. */
		save_parameters();
		beep_for(100);
		return(TRUE);
	}
	else
	{
		glcd_print_string(5,text_lh,1,"Bad key!!!:");
		update_display();
		beep_for(1000);
		wait (1000);
		return(FALSE);
	}
}
	
/* 
	Print the remaining credit to the LCD and 
	return +ve if we have therapy time in credit 
*/
long therapy_credit()
{
	long secs = param.cutoff_time - param.acc_ther_time;	
	char hours = secs / 3600;
	char mins =  (secs / 60) % 60;

	sprintf(g_str,"%2iH%02i Credit",hours,mins);
	glcd_print_string(7,11,1,g_str);

PRINT "Credit: %liS\r\n",secs PREND

	return(secs);
}

/******************** END CREDIT CONTROL **********************/
/******************** END CREDIT CONTROL **********************/
/******************** END CREDIT CONTROL **********************/
	

/******************************************************************
NAME		: start_screen
PURPOSE		: The first screens that appear at startup.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/



void start_screen()
{
 char start_button;
 char b;
 char i = 0;
 long in_credit;


	/******* The power on sequence *******/
    start_button = read_button(); /* Read any button pressed at startup. */

	if (start_button == TOP_BUTTON)
	{
		upside_down = TRUE;
	} 

	set_menu_state(upside_down);

	/* To stop the controller waking on a false button press. */
	while (!read_button()) wait(1);			  /* wait for a button. */



	/******* Saved Parameter Management *******/
	/* We read the params from the EEPROM, then display them 
	   on the LCD, but don't actually turn on pulses.
	   This allows the user to choose whether to have them 
	   or not.
	*/
	read_parameters(); /* Read the parameters from the EEPROM */

	set_contrast(param.contrast);	/* Set the stored contrast. */


   	glcd_print_string(3,10,2,"Eumedic\xA9");
   	glcd_print_string(5,11,1,"(C)2003 V2.05");	/* Version and copyright */


	in_credit = therapy_credit();	 /* Write credit time to screen. */


	update_display();

	POWER_HOLD_ON; /* hold power on now */

	beep_for(50);


	setup_threshold();			/* Setup the CMOS gate bias for pulse measurement. */


	
	for (i = 0; read_button(); i++ , wait (100))		  /* wait for no buttons. */
	{
		if (i == 150 && ( (~porte & 0xF) == (BUTTON1_BIT | BUTTON3_BIT) )  ) /* allow single use access if buttons 1 & 3 held for 15S. */
		{
			in_credit = 1;
			beep_for(100);
			i = 0;
		}
	}

	main_screen();

	#define TICK_ICON 154
	#define XCROSS_ICON 153
	#define ALLOW_CREDIT (in_credit < CREDIT_WARNING_LEVEL)

	menu_icons(TICK_ICON, XCROSS_ICON ,(in_credit < CREDIT_WARNING_LEVEL)? '$' : 0, ON_OFF_ICON);
	
	update_display();


	/* Get the selection. */
	menu_timer.period = 20 * 1000L;
	timer_go(&menu_timer);
	while ( !(b = read_button()) )
	{
		wait (30);
		if ( timer_done(&menu_timer) )
			power_down();
	}
		
			
	beep_for(2);
  	 
	switch (b)
	{
		case TOP_BUTTON:	   	  	/* Use saved settings */
			/* Buzz a warning according to the strength! */
			{
				char i;

				#define STRENGTH_WARNING_LEVEL 15
				if (param.strength > STRENGTH_WARNING_LEVEL )
				{
					menu_icons(TICK_ICON, 0 ,0, 0);
					glcd_print_string(5,text_lh + 6,1,"HIGH VOLTAGE!");
					update_display();

					for (i=0; i < 20; i++)
					{
						beep_for(3);
						wait (10);
					}

					wait (700);
				}
			}
			break;

		case MIDTOP_BUTTON:	   	  	/* Reset settings */
			set_default_params(); 	/* Set default params */
			break;

		case MIDBOT_BUTTON:
			if (ALLOW_CREDIT)
			{
				if (get_credit())/* use a key-code to get more credit. */	
				 in_credit = TRUE;
			}
			else
				power_down();

			break;
			
		case BOTTOM_BUTTON:
			power_down();
			break;
	}

	/* No credit? - don't go any further. */
	if (in_credit <= 0)
	{
		clear_screen();
   		glcd_print_string(3,5,2,"No Credit");
		update_display();
		beep_for(1000);
		wait (2000);
		power_down();	
	}


	/* Now allow the pulses to start... */
   	setup_tpu();				/* Setup the TPU for pulse output and input. */ 
   	setup_dtc();				/* Setup the DTC for pulse measurement */


	realise_parameters(); 		/* do the things that set up the unit with the parameters loaded. */

	menu_tree();
	
}	

