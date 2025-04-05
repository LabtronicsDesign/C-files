



#define BUTTON1 0x10
#define BUTTON2 0x20
#define BUTTON3 0x40
#define BUTTON4 0x80

char read_button();


/* If this is called at a regular rate, returns the button pressed, and takes care of autoincrement. */

#define REPEAT_MAX 1
#define INIT_WAIT_MAX 10

char read_keypad_action();


void gstart_screen();