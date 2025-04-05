

/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/

extern char PTR_ATTRIBUTE * const _heap_of_memory;
extern char PTR_ATTRIBUTE * /*not now const!*/ _top_of_heap;


void *malloc (size_t n);


/*********************************************************
	Functions added to the IAR heap module by Karl Lam:
**********************************************************/


/******************************************************************
NAME		: reset_heap
PURPOSE		: Discard all the contents of the heap
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE.
CALLED FROM : 
*********************************************************************/
void reset_heap();


/******************************************************************
NAME		: realloc_from_free_list
PURPOSE		: Does as realloc, but tries to take a block
			  from the free list.  If there is none in the free list
			  then it takes one as normal. This can be used judiciously to
			  prevent heap fragmentation.
COMMENT		:
PARAMETERS	: void *ptr : the existing block.
			  size_t n: The size of heap block required.
RETURNS		: A block, or NULL.
SIDE-EFFECTS: NONE.
CALLED FROM : 
*********************************************************************/
void * realloc_from_free_list ( void *ptr, size_t n );

void * realloc ( void *ptr, size_t n );

/******************************************************************
NAME		: print_heap
PURPOSE		: Dump a print-out of the heap to the serial.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: Prints.
CALLED FROM : 
*********************************************************************/
void print_heap();

/******************************************************************
NAME		: analyse_heap
PURPOSE		: Print an analysis of the heap.
PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: NONE.
CALLED FROM : 
*********************************************************************/
void analyse_heap();


/******************************************************************
NAME		: heap_mem_free
PURPOSE		: Return the amount of free memory in the heap.
PARAMETERS	: NONE
RETURNS		: The amount of heap left, total.
SIDE-EFFECTS: NONE.
CALLED FROM : 
*********************************************************************/
long heap_mem_free();


/******************************************************************
NAME		: clean_out_heap
PURPOSE		: Removes all claimed heap blocks apart from those that
			  have pointers from global variables of type procedure
			  or type array. 

!!! - need to make sure this detects circular refs - just make sure each 
!!! Also we could do with checking whether the proc is in the heap - it could be in ROM.  
 That would not really matter at the moment, as we would be marking the ROM, which 
 is read only, and not damaged - but it would be well to know the implications.


PARAMETERS	: NONE
RETURNS		: NONE
SIDE-EFFECTS: Alters heap.
CALLED FROM : startup.c
*********************************************************************/
void clean_out_heap();

