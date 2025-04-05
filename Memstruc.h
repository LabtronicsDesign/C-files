

/*			- MEMSTRUC.I -

   Include file used by MALLOC.C, FREE.C etc.

$Name: V3_34C V3_34B V3_34A $	   

*/



/* Do we really need this lot? */
//#ifndef MEM_ATTRIBUTE
//#define MEM_ATTRIBUTE
//#endif
//
//#ifndef PTR_ATTRIBUTE
//#define PTR_ATTRIBUTE MEM_ATTRIBUTE
//#endif
#define PTR_ATTRIBUTE 

typedef struct
{
	char PTR_ATTRIBUTE *next;
	char busy;
	char heap_dummy;
} _m_header;


