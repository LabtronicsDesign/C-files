




/* Display buffer */
#define COLS_IN_PAGE 128  
#define COLS_IN_LCD_PAGE 100  
#define PAGES_IN_DISPLAY 8
#define PIXEL_LINES_IN_DISPLAY 64  /* KL: would it be faster to make this 128?  !!! */

#define DISP_BUFF_SIZE (COLS_IN_PAGE * PAGES_IN_DISPLAY)

extern char disp_data[DISP_BUFF_SIZE];
extern char changed_lines[PAGES_IN_DISPLAY];
extern char upside_down;

#define DEFAULT_CONTRAST 10

/******************************************************************
NAME		: sci1_init
PURPOSE		: Initialise the second serial port to drive the LCD.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void sci1_init ();

/******************************************************************
NAME		: sci1_send
PURPOSE		: Send a character from the synch serial port.
PARAMETERS	: unsigned char senddata: the char to send.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void sci1_send (unsigned char senddata);

/******************************************************************
NAME		: glcd_control_write
PURPOSE		: Write a byte to the control reg of the LCD.
PARAMETERS	: char byte: the byte to write.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void glcd_control_write(char byte);


/******************************************************************
NAME		: set_contrast
PURPOSE		: set the contrast on the display according to the global value.
PARAMETERS	: char contr - the contrast setting.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void set_contrast(char contr);


/******************************************************************
NAME		: set_viewpoint
PURPOSE		: Set whether the display is upsidedown or not.
PARAMETERS	: char upside_down_p: TRUE if we need the display upsidedown.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void set_viewpoint(char upside_down_p);

/******************************************************************
NAME		: update_display 
PURPOSE		: Write the contents of the ram buffer of display 
				data to the LCD.  Doesn't write to lines that
				have not been recorded as having changed.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void update_display();

#define CLEAR_DISP_RAM memset(disp_data,0,DISP_BUFF_SIZE)




/******************************************************************
NAME		: fill_rectangle
PURPOSE		: Blackout a rectangle in pixel coordinates.  Y is downwards(?), X is leftwards.
PARAMETERS	: char x1, char y1, char x2, char y2: coordinates of the rectangle.
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void fill_rectangle(char x1, char y1, char x2, char y2);

/******************************************************************
NAME		: plot_small_char
PURPOSE		: Plot a small font char, on one of the lines of the LCD.
			  Note: This can only write to particular 8-pixel high lines
			  of the LCD. It also writes over anything underneath. 
PARAMETERS	: char line: The LCD line to write to.
			  char col: The LCD column to write to.
			  char c: The character to write.
			  char clear_space_after: Clear the space after the char?
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void plot_small_char(char line, char col, char c, char clear_space_after);


/******************************************************************
NAME		: bar_graph
PURPOSE		: Draws a 8-pixel high bar across the LCD, scaled 
				according to the variable and it's max value.
PARAMETERS	: ...
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void bar_graph(char line, char col , char size , short max , short variable);

/******************************************************************
NAME		: plot_big_char
PURPOSE		: Plot a big font char, on two of the lines of the LCD.
			  Note: This can only write to particular 8-pixel high lines
			  of the LCD. It also writes over anything underneath. 
PARAMETERS	: char line: The LCD line to write to.
			  char col: The LCD column to write to.
			  char c: The character to write.
			  char clear_space_after: Clear the space after the char?
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/


void plot_big_char(char line, char col, char c, char clear_space_after);

/******************************************************************
NAME		: glcd_print_string
PURPOSE		: Print a string to the LCD in either font at the
				line and collumn given.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void glcd_print_string(char line, char col, char font, char * str);


/******************************************************************
NAME		: plot_sprite
PURPOSE		: Plot a sprite on the LCD in either font at the
				line and collumn given.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void plot_sprite(char line, char col, char font, char c);
	


/******************************************************************
NAME		: clear_screen
PURPOSE		: Clears the display RAM and sets the screen to all changed.
				Does not update the LCD.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/
void clear_screen();

/******************************************************************
NAME		: glcd_init
PURPOSE		: Initialises the LCD controller.
			  Needs the synch serial port initialised first.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE
CALLED FROM : 
*********************************************************************/

void glcd_init();							   



