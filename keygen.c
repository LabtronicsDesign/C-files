

/*
Copyright 2001 Eumedic Ltd.

compile with
 gcc -funsigned-char -oEumKey keygen.c


*/


/* Cyphers used in sequence and key generation. */
#define HOURS_LOG_CYPHER  0x7AC43BDE
#define CREDIT_SEQ_CYPHER 0x3E

unsigned long binary_coded_quartal(unsigned short val);
unsigned short key_func(unsigned long w32 , char w8);

#define KEY_FUNC key_func(acc_splat , seq_spat)


/********************* LOOKUP TABLE ********************************/
static const char lookup[] =
{
	  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65, //|   Make no changes!
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220, //|   To this table!
	 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98, //|
	190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255, //|
	 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7, //|
	219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154, //|
	101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36, //|
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185, //|
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205, //|
	 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80, //|
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238, //|
	 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115, //|
	202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139, //|
	 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22, //|
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168, //|
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53, //|
};	
																 
char crypt_acc;	

/******************************************************************
NAME		: 
PURPOSE		: .
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: 
CALLED FROM :
*********************************************************************/
void encrypt(char byte)
{
	crypt_acc = lookup[crypt_acc ^ byte];
}

/* 
	Return a 32-bit result in 'BCQ' given a 16-bit number. 
	each 4-bits represents in a hex digit (range 0-3)
	2 bits of the 16-bit number. 
	When printed as a hex number, gives a 'quartal' representation of val.

*/
unsigned long binary_coded_quartal(unsigned short val)
{
	unsigned long acc = 0;
	char i;
	for (i=8; i; i--)
	{
		acc <<= 4;
		acc	|= ( (val >> 14) & 0x3) ; /* mask off top most 2 bits and put in result */
		val <<= 2; 			 /* shift up next 2 bits */
	}
	return(acc);
}


/* 
	Take the two elements of the code presented to the user and workout 
	what the key for more credit should be.
	At the moment this is just some XORing. 
*/
unsigned short key_func(unsigned long w32 , char w8)
{
	unsigned short val;
	char temp;
	/* combine the 5 bytes of the code to produce one byte. */
	crypt_acc = 0;
	encrypt(w8);
	encrypt(w32);
	w32>>=8;
	encrypt(w32);
	w32>>=8;
	encrypt(w32);
	temp = crypt_acc;
	w32>>=8;
	encrypt(w32);

	/* Produce 2 bytes of key. */
	val = crypt_acc + (temp << 8);

	return val ;
}


//========EVERYTHING ABOVE THIS LINE SHOULD BE IDENTICAL CODE TO THE EMBEDDED CODE IN THE UNIT. =======

void print_time_from_seconds(long seconds)
{
	long hours = seconds/3600;
	char minutes = (seconds/60) % 60;
	char secs = seconds % 60;
	printf("[Logged Therapy time: %i Hours (%i Mins, %i Secs)]\n",hours,minutes,secs);	
}


int main()
{
	char c;

	int code;
	int seq;
	long time;
	int items_scanned;
	int chars_read;
	char * dummy_string;

	printf("Eumedic Credit Key Generator\nVersion 1.00\n\n");


	//while (1)
	{
		printf("**********************\n");
		printf("Enter code:___________\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08");

		items_scanned = scanf("%1X%8LX%2X%n",&code, &time, &seq, &chars_read);
		while (getchar() != 10); /* Eat the rest of the chars, up to the carriage return. */

		//printf("<%i %i>",items_scanned, chars_read);
		//printf("[Entered data: %02X %08LX %02X]\n", code, time, seq);

		switch (code)
		{
			case 0:
			{
				long acc_splat;
				char seq_spat;

				if (items_scanned != 3 || chars_read != 11 )
				{
					printf("Bad code entry!\n");
				}
				else
				{
					acc_splat = time;
					seq_spat = 	seq;
					
					time = acc_splat ^ HOURS_LOG_CYPHER; 
					seq = seq_spat ^ CREDIT_SEQ_CYPHER;

					printf("\n");
					printf("[Credit Request No:%i]\n",seq);
					print_time_from_seconds(time);
					printf("\n");
					printf("     ********\n");
					printf("Key: %08LX\n\n", binary_coded_quartal(KEY_FUNC));
					printf("     ********\n");
					
				}
			}
			break;


			default:
				printf("Unsupported code-type!\n");
				break;
		}

		printf("\n\n[Hit Enter Key to finish]");
		getchar();
	}
}