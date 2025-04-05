



/*
 Turns IIC bus on 
SDA: HI OUT -> LO OUT
SCL: HI     -> HI
*/
void iic_on(void);

/*
 Turns IIC bus off
SDA: LO OUT -> HI OUT
SCL: HI     -> HI
*/
void iic_off(void);

/*
 Write a byte to the bus and detect the acknowledge
Initially:
SDA: LO OUT
SCL: LO
Ends with the same.
Returns true if acknowledges.
*/
char iic_put(char byte);


/*
Read a byte from the bus and acknowledge it 
Initially:
SDA: LO OUT
SCL: LO
Ends with the same.
Returns true if acknowledges.
*/
char iic_get(char last);


/* returns true if there is a device at 'address' */
char iic_poll(char address);


/* Initialise the IIC net */
void iic_init(void);

void wait_for_eeprom_to_finish();

void write_block_to_eeprom(char address, char count , char * data);


void read_block_from_eeprom(char address, char count , char * data);

