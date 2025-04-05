



/*

DESCRIPTION:   Pulse IO.
AUTHOR	: Karl Lam
DATE	: 23/1/2001

*/

		

#define N_WIDTHS 5

#define Z_MIN 10
#define Z_DEF 20
#define Z_MAX 80
#define INTENSITY_MIN 1
#define INTENSITY_MAX 8
#define STRENGTH_MAX 250
#define STRENGTH_MIN 10
#define FREQ_MIN 15
#define FREQ_MAX 350
#define FREQ_DEF 60

#define MODULATION_MIN 0
#define MODULATION_MAX 5

#define MOD_FREQ_MAX 120 /* The limits of frequency cycling. */
#define MOD_FREQ_MIN 30

typedef struct
{	/* I think we need to keep chars at the end to avoid holes in the struct. */
 	unsigned long acc_ther_time;			/* 1  4 */
 	unsigned long cutoff_time;				/* 2  4 */
 	short strength; 						/* 3  2 */
 	short base_freq;						/* 4  2 */
 	char pulse_out_number;					/* 5  1 */
 	char z_value;							/* 6  1 */
 	char modulation;						/* 7  1 */
 	char filter;							/* 8  1 */
 	char freq_cycling;						/* 9  1 */
 	char contrast;							/* 10 1 */
 	char credit_seq;			        	/* 11 1 */
	//char dummy;							/* 		   [must be even size for sizeof to be correct! - use a dummy if nec.] */
 	char checksum;							/* 12 1    [This is the checksum] */
} parameter_block;

			   
#pragma memory=near
extern parameter_block param;	


extern char interrupt_counter;	
extern char dog_state;	 		
extern char led_state;	 		
extern char count;
extern char pulse_index;
extern char n_rings;
extern char initital_rings;
extern short pulse_period;
extern short inter_pulse_period;
 


 


extern short frequency;
 
 
//extern char pulse_out_count;
 
extern char ramping;
 
extern short modulated_amp;
 
extern char start_button;
 
extern char mod_counter;

#pragma memory=default



void power_down();


void setup_watchdog();	





/* Init the system interrupts in the TPU, not 8-bit timer. */
void milli_sec_timer_init();	




/***********************************************************************************/

	

/* Set up the H8 I/O */
void io_init();




void setup_dtc();



void set_pulse_width(char strength);


/* Give the value to put in the compare reg from the 'frequency' value. */
void set_pulse_period();	

void set_up_pulse_output();

void set_up_input_capture();


/* Count clock pulses on channel 1. */
void set_up_hardware_count();




/* Buzzer is on channel 4 */
#define BEEP_NORM (250) /* 4KHz is 250 */

void setup_buzzer();		



void setup_tpu();


void beep_for(short t);												 

void beep_period_and_time(short period, short time);

void end_beep();

void warble(char type);

void pip_n_times(char n);
		
/* returns the battery voltage in volts. */
float read_battery();

/* writes threshold volts to the analogue output channel. */
void write_analogue(char value);



/* Find the level at which the CMOS gate changes state:
	do it several times and get the average, then subtract
	some to get out of the noisy region.
*/
void setup_threshold();
	


extern timer menu_timer;
extern timer therapy_timer;
extern timer power_off_timer;
extern timer display_timer;
extern timer modulation_timer;


#pragma memory=near
extern short seconds,last_seconds;
 
extern short this_width; 
extern short last_width; 
extern short initial_width; 
extern short delta_width;
//extern short filtered_width;
extern signed char response;


#define RESPONSE_WAIT 5

extern unsigned short init_plus_percent;
extern unsigned short init_plus_50;		     
 
 
extern char dose;
 
extern char delta_flattened_count;
 
 
extern short rate1;
extern short initial_rate;
extern long milliseconds;
 
 
extern char skin_display;
extern char freeze_display; /* Flag to freeze the display, also indicates the warble to use. */
extern char no_reading_count;
#define NO_READING_COUNT 20


#pragma memory=default

	
extern no_init long accumulated_therapy_time;


void therapy_control();


/* Does both strength and frequency modulation. */
void modulate();


/* Set the modulation state machine to a start state. */
void reset_modulation();
void freq_cycling_off();

/* turn off modulation. */
void modulation_off();

/* Turn off the intensity function. */
void intensity_off();




		   