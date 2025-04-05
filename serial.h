

/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/



/* Some control characters. */
#define CTRL_A 1
#define CTRL_C 3
#define CTRL_T 20
#define CTRL_Z 26

extern char control_c_break_allowed;  /* Set when CTRL-C causes a break, rather than being a real char. */



void test_serial(void);

/* initialise the serial port and set up the buffers */
void serial_init();

/* Send char to serial port */
void serial_put(char c);

/* send string to serial port */
void serial_put_string(const char * str);

/* wait for char from serial port */
char serial_get();

/* return the value of the charater if there is one in the queue, else return -1 */
short serial_look();

/* no. of chars waiting in input buffer */
unsigned char serial_input_queue();

/* Wait until all the chars have gone out of the op buffer. */
void serial_wait_for_output();

void serial_wait_for_output_no_swap();

/* remove all chars from the input queue */
void serial_flush_input();

/* set the baud rate for the port */ 
void set_baud_rate(long rate);

 









