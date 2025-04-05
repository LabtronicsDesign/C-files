



/*

DESCRIPTION:   Pulse IO.
AUTHOR	: Karl Lam
DATE	: 23/1/2001

*/

		


#pragma language=extended	/* Enable use of extended keywords */

#include <stdio.h>
#include <inh8.h>	  	/* Intrinsic Funcs */
#include <string.h>

#include "hardware.h" 	
#include "serial.h" 	
#include "time.h" 		
#include "iic.h"
#include "pulse.h"
		
#include "general.h"
#include "graphic.h"

		
#include "gmenus.h"
#include "debug.h"


/***********************************************************************************/


#pragma memory=near

/* The parameters for the instrument. */
parameter_block param;



volatile char p2ddrc;

short pulse_period;
short inter_pulse_period;

char interrupt_counter = 0;	
char dog_state = 0;	 		
char led_state = 0;	 		
char count = 0;
char pulse_index = 0;
char n_rings = 0;
char initital_rings = 0;


short frequency = FREQ_DEF;


//char pulse_out_count = INTENSITY_MIN;

char ramping = 0;

short modulated_amp = STRENGTH_MIN;

char start_button;

char mod_counter;

#pragma memory=default





void setup_watchdog()
{
	tcnt_w = 0x5A00;					/* Set the counter 0. */
	tcsr_w  = 0xA500 | BIT6 | BIT5 | 7 ; /* Generate ext. reset signal; Counting */ 
	rstcsr_w = 0x5A00 | BIT6;			/* Power-on reset */
}	


void interrupt [NMI] nmi_service()
{
}	


void interrupt [TPU_TGI2A] millisecond_tick_tpu()
{
	/* Clear the interrupt source */
  	tpu_tsr2 &= 0xFF - TGFA;
  		
	INTS_ON; /* Let other interrupts in. */
	
	/* Inc time low */
	time_low += 1;
}



/* Init the system interrupts in the TPU, not 8-bit timer. */
void milli_sec_timer_init()
{
	mstpcrh &= 0xFF - TPU_STOP ;/* enable the TPU */


	/* Set up a PWM channel. */
	tpu_tcr2 =  0x20 |  0x2; 	/* Clear on TGReg A ; [Phi/16->250KHz] on chan 2 */
	tpu_tior2 = 0;				/* Output: NONE  */

	tpu_tgr2a = 250 -1;			/* Overall period (1mS @ 4MHz) */

	tpu_tgr2b = 0; 				/* duty */

	tstr |= BIT2;				/* Start chan 2 */

   	tpu_tier2 |= TGIEA; 		/* Ints on TGReg A compare. */
}	




/***********************************************************************************/

	

/* Set up the H8 I/O */
void io_init()
{
	/* Make sure all ports that are not connected, or are outputs are set up 
	as outputs so they don't drift into the CMOS high-current region. */

	#define PORT_1_OUTPUT_BITS (BIT1 | BIT3 | BIT5 | BIT6 | BIT7) /* Bit 4 now an input too! */
	#define PORT_2_OUTPUT_BITS (BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)
	#define PORT_3_OUTPUT_BITS (BIT2 | BIT3 | BIT4)
	#define PORT_E_OUTPUT_BITS (BIT6 | BIT7)
	#define PORT_F_OUTPUT_BITS (BIT0 | BIT1 | BIT2)
	#define PORT_G_OUTPUT_BITS (BIT0 | BIT1 | BIT2)


	p1ddr = PORT_1_OUTPUT_BITS;

	p2dr  = SDA | SCL;	/* Be careful not to turn on the main transistor! */
	p2ddr = p2ddrc = PORT_2_OUTPUT_BITS;
	p3ddr = PORT_3_OUTPUT_BITS;

	pepcr = BIT0 | BIT1 | BIT2 | BIT3; /* Pull up the buttons */
	pedr = 0xFF; /* Keep the LCD control bits high to start with [else get bad data into LCD?] */
	peddr = PORT_E_OUTPUT_BITS;

	pfddr = PORT_F_OUTPUT_BITS;

	pgddr = PORT_G_OUTPUT_BITS | CS0 | CS1; /* remember to keep the CS\'s that were set up in lowlevel.c! */

    pulse_out_count = INTENSITY_MIN;  /* Needs setting explicitly. */
}



/*
The OUTPUT COMPARE interrupt - every time the output pulse goes high.
See diagrams in Karl's notebook 19/9/01 for schematic of timing.

Important:
 The capture should be enabled as soon as possible after we get into the interrupt.
 We only enable input capture for the last pulse in a train, as earlier
  pulses will be cut short by later ones.
 The pulse counting is done in hardware, so we don't have to do that very fast.
 We only read the pulse count at the start of the first pulse in a train, 
  as that is the result of the last pulse's ringing.

 To get into the fast bits quickly, we make sure that the test is for zero,
 and that we test a fast access location.

*/
void interrupt [TPU_TGI3A] pulse_out_svc()
{
	if (! pulse_out_count )	/* Is this the _last_ pulse in an 'intensity group'? */
	{
		/* Only measure the effects of the last pulse in the group. */
		/* Set up the input capture via the TPU and DTC. */
		tpu_tsr0 &= 0xFF - TGFA; 	/* [Clear pulse in interrupts to avoid any input from prev 'intensity' pulses.] */
		tpu_tier0 |= TGIEA; 	 	/* Ints ON TGReg A capture. Allow DTC/interrupts for next set of captures. */

	    tpu_tgr3a = pulse_period;   /* Set up the normal gap till the next output pulse. */


        if (param.pulse_out_number == 1)  /* first pulse? */
        {
            n_rings = tpu_tgr1a;            
        }


		pulse_out_count = param.pulse_out_number; /* Set up the pulse number. */
	}
	else
	{
		tpu_tgr3a = inter_pulse_period;


        if (param.pulse_out_number-1 == pulse_out_count) /* first pulse? */
        {
            n_rings = tpu_tgr1a;            
        }
	}

	/* Clear the interrupt source */
  	tpu_tsr3 &= 0xFF - TGFA;


    pulse_out_count--;     /* count down pulses. */


}					 


/***********************************************************************************/

/******
DTC data is stored in the internal RAM: F800 - FBFF (p173 of hardware manual 18-010B)
*****/


typedef struct
{
	union
	{
		char mode;
		void * address; /* Must set mode AFTER address! */
	}chana;		  /* Mode A and Source. */

	union
	{
		char mode;
		void * address; /* Must set mode AFTER address! */
	}chanb;		  /* Mode A and Dest. */

	short counta;

	short countb;

}dtc_frame;	

#pragma memory=dataseg(DTCRAM)
	dtc_frame transfer1;
#pragma memory=default

#define ARRAY_SIZE 2
#pragma memory=near
unsigned short data_dest[ARRAY_SIZE+2]; /* +2: a bit of extra room. */
#pragma memory=default


/******

The INPUT CAPTURE interrupt.

 We only get this int after the DTC has done it's job. 

*****/

void interrupt [TPU_TGI0A] pulse_in_svc()
{
					//p2dr &= 0xFF - SCL;
					//p2dr |= SCL;

	/* Turn OFF input capture ints. */
   	tpu_tier0 &= 0xFF - TGIEA; /* TGReg A capture. */

  	tpu_tsr0 &= 0xFF - TGFA; /* [clear any possible pending interrupt cause.] */


	/***** setup DTC for the next set of data ***********************/
	transfer1.chanb.address	= data_dest; 	/* DEST */
	transfer1.chanb.mode	= 0;
	transfer1.counta =  ARRAY_SIZE;			/* No. of data transfers. */
	dtcerb |= BIT5; 						/* DTC activation for pulse in [p185] */
	/***************************************/
}	



void setup_dtc()
{	char i;

	/* TPU_TGI0A - the interrupt that activates the DTC.
	    corresponds to vector #32 - address 0x0440.
	   Page 185 of hardware manual. */

	for (i=0; i < ARRAY_SIZE; i++ )
	{
	   data_dest[i] = 0;
	}

	/*  DTC vector has been set up to point to the frame in 'context.s__' */
//PRINT "$%x $%p \r\n", *(short *)0x440 , &transfer1 PREND

	/* Set up the DTC frame. */
	transfer1.chana.address	= (void*)TPU_TGR0A; /* source */

	transfer1.chana.mode	= MRA_SRC_FIX | MRA_DST_INC	| MRA_NORM | MRA_SZ_WORD;

	transfer1.chanb.address	= data_dest; /* DEST */

	transfer1.chanb.mode	= 0;

	transfer1.counta =  ARRAY_SIZE;	/* No. of data transfers. */

	transfer1.countb = 0;	/* only used in block mode. */

}



	

#define TRANSISTOR_TURNOFF_uS 11

/* Give the value to put in the compare reg from the 'strength' value. */
void set_pulse_width(char strength)
{
	 tpu_tgr3d = strength * 2 - TRANSISTOR_TURNOFF_uS -1 ;	
}

#define MAIN_TPU_FREQ 1000000

/* Give the value to put in the compare reg from the 'frequency' value. */
void set_pulse_period()
{
	long period = (MAIN_TPU_FREQ / frequency) - 1;

	/* Take account of multiple pulses going out. */
	period = period - ( (param.pulse_out_number-1) * (inter_pulse_period ) ); 


	/* limit the freq. */
	if (period < MAIN_TPU_FREQ / FREQ_MAX)
		period = MAIN_TPU_FREQ / FREQ_MAX;

	if (period > 0xFFFF)
		period = 0xFFFF;


	pulse_period = period;


	inter_pulse_period = (param.z_value * 20) + (tpu_tgr3b + 1 + 20) ;
}

	

void set_up_pulse_output()
{

	/* Set up a PWM channel. */
	tpu_tcr3 =  0x20 |  0x1; 	/* Clear on TGReg A ; Phi/4 on chan 3 */
	tpu_tior3h = 0x20 | 0x1;	/* Output: 1 on TGReg B; 0 on TGReg A. */

	set_pulse_width(STRENGTH_MIN); 		/* duty */
	tpu_tgr3b =	tpu_tgr3d; /* Set up initial pulse width. */

	set_pulse_period();		/* 60Hz */



	tpu_tmdr3 = BIT1 | BIT5 ; 			/* PWM Mode 1 & buffer mode for B/D */

	tstr |= BIT3;				/* Start chan 3 */

   	tpu_tier3 |= TGIEA; /* Ints on TGReg A compare. */
}	


void set_up_input_capture()
{
	tpu_tcr0 =  0x20 |  0x1; 	/* Clear on TGReg A ; Phi/4 */
	tpu_tior0h = 0x8 | 0x1; 	/* Capture; TIOCA0/TGR0A/FALL edge. */

	tstr |= BIT0;				/* Start chan 0 */

   	tpu_tier0 |= TGIEA; 		/* Ints on TGReg A capture. */
}

/* Count clock pulses on channel 1. */
void set_up_hardware_count()
{
	tpu_tcr1 = BIT5 | BIT4 | BIT2; 	/* Both edges; TCLKA; CLEAR ON TGRA */
	tpu_tior1 = BIT3 | BIT0; 	    /* Capture; TIOCA1/TGR1A/FALL edge. */
	tstr |= BIT1;				    /* Start chan 1 */
}




/* Buzzer is on channel 4 */
#define BEEP_NORM (250) /* 4KHz is 250 */

void setup_buzzer()
{
	mstpcrh &= 0xFF - TPU_STOP ;/* enable the TPU */

	/* Set up a PWM channel. */
	tpu_tcr4 =  0x20 | 0x1; 	/* Clear on TGReg A ; Phi/4 */

	tpu_tior4 = 0x20 | 0x1;		/* Output: 1 on TGReg B; 0 on TGReg A. */

	tpu_tgr4a = BEEP_NORM;		/* Period */

	tpu_tgr4b = tpu_tgr4a; 		/* Duty - off to start with. */

	tpu_tmdr4 = 2; 				/* PWM Mode 1 */

	tstr |= BIT4;				/* Start chan 4 */
}		



void setup_tpu()
{
	mstpcrh &= 0xFF - TPU_STOP ;/* enable the TPU */

	set_up_pulse_output();

	set_up_input_capture();

	set_up_hardware_count();
}


void beep_for(short t)
{
	if (t < 0)	 /* Low freq beep when t negative. */
	{
		t = -t;
		tpu_tgr4a = BEEP_NORM << 2;
	}
	else
		tpu_tgr4a = BEEP_NORM;

	tpu_tgr4b = tpu_tgr4a >> 1;
	tpu_tcnt4 = 0;
	wait (t);
	tpu_tgr4b = tpu_tgr4a; /* Off */
}

												 

void beep_period_and_time(short period, short time)
{
	tpu_tgr4a = period;
	tpu_tgr4b = tpu_tgr4a >> 1;
	tpu_tcnt4 = 0;
	wait (time);
	tpu_tgr4b = tpu_tgr4a; /* Off */
}

void end_beep()
{
    beep_for(500);
}    	

void warble(char type)
{
	char i;

	switch (type)
	{
		case 1:
			for (i=0; i<5; i++)
			{
				beep_period_and_time(BEEP_NORM,50);
				beep_period_and_time(BEEP_NORM << 1,50);
			}

			break;

		case 2:
			for (i=0; i<10; i++)
			{
				beep_period_and_time(BEEP_NORM,25);
				beep_period_and_time(BEEP_NORM << 1,25);
			}
			break;

		case 3:
			for (i=0; i<1; i++)
			{
				beep_period_and_time(BEEP_NORM << 1,125);
				beep_period_and_time(BEEP_NORM,125);
			}
			break;

		default:
			break;
	}

}

void pip_n_times(char n)
{
	while (n--)
	{
		beep_for(100);
		wait (50);
	}
}							   


#define RES_LOW	33000.0	/* The lower arm of the divider. */
#define RES_HI 100000.0	/* The upper arm of the divider. */
#define LOW_VOLTAGE	7.0 /* When the battery symbol comes on */
		
/* returns the battery voltage in volts. */
float read_battery()
{
	float volts;

	//STR("Test analogues\r\n");

 	mstpcrh &= 0xFF - ADC_MODULE_STOP; /* Turn on the analogue module */

	adcsr = 0;                         /* clear the flags, stop conversions */
	adcsr = ADST + AD_CKS + 0;		   /* start a scanned conversion of first chan group 0-3 */
	while ((adcsr & ADF) == 0)
		;                              /* wait for the end */
	adcsr = 0;                         /* clear the flags, stop conversions */

	volts =  addra * (RES_LOW + RES_HI)/ RES_LOW * 5.0 / (1024L*64) ;

 	mstpcrh |= ADC_MODULE_STOP; 	   /* Turn off the analogue module */

	return volts;
}

/* writes threshold volts to the analogue output channel. */
void write_analogue(char value)
{
 	mstpcrh &= 0xFF - DAC_MODULE_STOP; 	/* Turn on the analogue module */

	/* write conversion data to appropriate data register and */
	/* enable D/A conversion on appropriate channel; DA1 or DA0 pin becomes an output */
	dadr1 = value;					/* write value to data register 1 */
	dacr |= DAOE1;					/* set DA1 pin to output in control register */

 	mstpcrh |= DAC_MODULE_STOP; 	   	/* Turn off the analogue module */
}


/* Find the level at which the CMOS gate changes state. */
char find_thresh()
{
	char i;

	write_analogue(0);

	do /* wait for an value to drop. */ /* KL: could wait forever here  !!! */
	{
		wait (20);
	} while (port1 & BIT0);
	

	for (i=120; i < 220; i++)
	{
		write_analogue(i);
		wait (2);
		if (port1 & BIT0)
			break;
	}
	return i;
	
}	


/* Find the level at which the CMOS gate changes state:
	do it several times and get the average, then subtract
	some to get out of the noisy region.
*/
void setup_threshold()
{	char i;
	short sum = 0;

//long t1; 
//STR("find thr\r\n");
//t1 = time_low;

	#define THRESHOLD_SAMPLES 3	  /* No. of times we try to get the samples. */
	#define THRESHOLD_HEADROOM 20 /* How far below the CMOS gate threshold we set the level. */
	for (i=0; i < THRESHOLD_SAMPLES ; i++ )
	{
		sum += find_thresh();
	}
	sum  /= THRESHOLD_SAMPLES;

//PRINT "threshold took ms to measure %li\r\n",time_low - t1 PREND

	/* Set the threshold we want. */
	write_analogue(sum - THRESHOLD_HEADROOM);

//PRINT "set thr at %i\r\n",sum - THRESHOLD_HEADROOM PREND
}	
	



timer menu_timer;
timer therapy_timer;
timer power_off_timer;
timer display_timer;


#pragma memory=near
short seconds,last_seconds;

short this_width = 0; 
short last_width = 0; 
short initial_width = 0; 
short delta_width;
//short filtered_width;
signed char response = 0;

unsigned short init_plus_percent;
unsigned short init_plus_50;		     


char dose = 0;

char delta_flattened_count = 0;


short rate1 = 0;
short initial_rate = 0;
long milliseconds;


char skin_display = 0;
char freeze_display = 0; /* Flag to freeze the display, also indicates the warble to use. */

char no_reading_count = 0;

#pragma memory=default


#define FIFO_LENGTH 15
short fifo[FIFO_LENGTH];   

 


#define PULSE_SKIN_THRESHOLD (15)

/* Called every (30?) milliseconds round the main loop in each menu to control
 and monitor the pulse therapy. */
void therapy_control()
{
	char i;


	/* calcuate pulse readings */

	if (data_dest[1] > 999 || n_rings <= 2) /* Detect unreadable signals */
    {
		if (no_reading_count < NO_READING_COUNT)
		{
			no_reading_count++;
			if (no_reading_count == NO_READING_COUNT) /* [Occurs once only] */
			{
        		freeze_display = 3;
			}
		}
    }
    else  /* Readable. */
	{
        this_width = data_dest[1] ;	    	
	}

	if (data_dest[1] > PULSE_SKIN_THRESHOLD)
	{
		skin_display = TRUE;

		if (!stop_watch_going(&therapy_timer))
		{
			stop_watch_start(&therapy_timer);
			beep_for(5);
		}

	}
	else /* Off skin... */
	{
		/* Therapy time record. */
		if (skin_display)
			param.acc_ther_time += seconds;

		skin_display = FALSE;

		no_reading_count = 0;


		stop_watch_stop(&therapy_timer);
		last_width = this_width;
		last_seconds = 0;
		initial_width = 0;
        dose = 0;
		delta_flattened_count = 0;
		delta_width = 0;
			
		rate1 = 0;
		initial_rate = 0;
		initital_rings = 0;

		response = 0;

	}


	seconds = stop_watch_seconds(&therapy_timer);


	/* Dont do any of the calculations/alarms if we have these things set... */
	if (param.filter || param.modulation || param.freq_cycling)
		return;
		
    if (no_reading_count >= NO_READING_COUNT)
    {
        return ;
    }

        
    /* This code is executed once a second, every time a second rolls over. */
	if (seconds != last_seconds)
	{
		for (i = FIFO_LENGTH-1; i  ; i--)
		{
			 fifo[i] = fifo[i-1];
		}
		fifo[0] = this_width;


		/* Average the start and end of the fifo to filter d/t */
		delta_width = ((   fifo[0]
					    + fifo[1]
					    + fifo[2]
					    + fifo[3]
					    + fifo[4]
					  )
					  - 
					  (   fifo[FIFO_LENGTH-1] 
					    + fifo[FIFO_LENGTH-2] 
					    + fifo[FIFO_LENGTH-3]
					    + fifo[FIFO_LENGTH-4]
					    + fifo[FIFO_LENGTH-5]
					  )) / 5;

		/* Ring if delta is zero or less for 2 seconds */
		if (delta_width <= 0 /* && !delta_flattened */)
		{
			if (delta_flattened_count++ >= 2)
			{
        		freeze_display = 2;
		   		delta_flattened_count = 0;	
			}
		}
		else
		{
		   delta_flattened_count = 0;	
		}

        /* Calcuate the response. */
		if (seconds == RESPONSE_WAIT)
		{
			response = this_width - initial_width ;
		}

		last_seconds = seconds;
	}




	milliseconds = stop_watch_time(&therapy_timer);	 /* Could be read above! */

    /* We haven't found the initial width yet: */
    if (initial_width == 0)
    {

        /* Init width is taken after fixed no. of milliseconds: */
		if (milliseconds > 500)
		{
			initial_width = this_width; /* Set the init width. */
			initital_rings = n_rings >> 1;	/* Set the init rings */

			/* INIT fifo */
			for (i = 0; i<FIFO_LENGTH  ; i++ )
			{
				 fifo[i] = initial_width;
			}
 
            /* Calculate some Dose figures: */
			init_plus_50 = 	initial_width * 3 / 2;

            if (initial_width <= 40)
            {
               init_plus_percent = initial_width * 35; 
            }
            else if (initial_width <= 60)
            {
               init_plus_percent = initial_width * 25; 
            }
            else if (initial_width <= 80)
            {
               init_plus_percent = initial_width * 10; 
            }
            else 
            {
               init_plus_percent = initial_width * 5; 
            }
            init_plus_percent = initial_width + init_plus_percent / 100;
		   
		}
    }

    if (initial_width && !dose)
    {
        if (this_width >= init_plus_50 )
        {
            dose = 1;
            freeze_display = 1;
        }
        else if (seconds >= 10)
        {
            if (this_width >= init_plus_percent)
            {
	            dose = 1;
            	freeze_display = 1;
            }
        }
    }
}





timer modulation_timer;


/* Does both strength and frequency modulation. */
void modulate()
{

#define RAMP_UP	   	0
#define RAMPED_UP  	1
#define RAMP_DOWN  	2
#define RAMPED_DOWN	3
	
	if (param.modulation)
	{
		if (ramping == RAMP_UP)
		{
			if (modulated_amp < param.strength)
			{
				modulated_amp++;
				set_pulse_width(modulated_amp);
			}
			else
			{
				ramping = RAMPED_UP;
			   	modulation_timer.period = param.modulation * 1000;
				timer_go(&modulation_timer);
			}
		}
		else if (ramping == RAMP_DOWN)
		{
			if (modulated_amp > STRENGTH_MIN)
			{
				modulated_amp--;
				set_pulse_width(modulated_amp);
			}
			else
			{
				ramping = RAMPED_DOWN;
			   	modulation_timer.period = 1000;
				timer_go(&modulation_timer);
			}
		}
		/* Not ramping - waiting... */
		else 	
		{
            if (timer_done(&modulation_timer))
            {
		 	    ramping = (ramping == RAMPED_UP)? RAMP_DOWN : RAMP_UP;
            }
		}
	}
	else if (param.freq_cycling)
	{
		if ( (mod_counter++ & 1) == 0)  /* Every other time... */
		{
			if (ramping == RAMP_UP)	
			{
				frequency++;

				if (frequency >= MOD_FREQ_MAX)
					ramping = RAMP_DOWN;

			}
			else
			{
				frequency--;

				if (frequency <= MOD_FREQ_MIN)
					ramping = RAMP_UP;
			}
			set_pulse_period();
		}
	}
}	


/* Set the modulation state machine to a start state. */
void reset_modulation()
{
	modulation_timer.period = 500; /* How long modulation takes to get started. */
	ramping = RAMPED_UP;	
	modulated_amp = param.strength;
	timer_go(&modulation_timer);
}

void freq_cycling_off()
{
	param.freq_cycling = FALSE; 
	frequency = param.base_freq;
	set_pulse_period();
}	


/* turn off modulation. */
void modulation_off()
{
	param.modulation = 0;
 	set_pulse_width(param.strength);  /* Make sure we are not partway through a modulation. */
}

/* Turn off the intensity function. */
void intensity_off()
{
	param.pulse_out_number = INTENSITY_MIN;
	set_pulse_period(); 	
}	


   

/******************************************************************
NAME		: startup_sequence
PURPOSE		: Initialise the underlying operating system before
			  starting the language
PARAMETERS	: NONE
RETURNS		: NONE 
SIDE-EFFECTS: Lots
CALLED FROM : Main.c
*********************************************************************/






		   