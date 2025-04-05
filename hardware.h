

/*

AUTHOR	: Karl Lam
DATE	: 4/10/2000

History:																						
--------------
*/





#pragma language=extended

#include <ioh82345.h>



#pragma memory=near
extern volatile char p2ddrc;
#pragma memory=default


#define INTS_OFF set_imask_ccr(1)
#define INTS_ON  set_imask_ccr(0)


/* The RAM memory map */
#define RAM_START_512K 	(0x200000) /* Start of 512K RAM */
#define RAM_START_128K 	(0x260000) /* Start of 128K RAM */
#define RAM_END 	   	(0x280000) /* The top of RAM for both the 128K and 512K memory maps. */


#pragma warnings=off
#define ICC_UCHAR  /* need to do this for the moment!!! */
#define ICC_USHORT   /* need to do this for the moment!!! */
#pragma warnings=default




/* The definitions of the I/O pins etc. */
sfr astcr =	ASTCR;	/* Access state control reg. */
sfr	wcrh  =	WCRH;	/* Wait control register high */
sfr	wcrl  =	WCRL;	/* Wait control register low */



//sfr syscr = SYSCR;

//sfr wcrh = WCRH;       /* wait control reg */
//sfr wcrl = WCRL;       /* wait control reg */



/* PORTS */

/* Port 1 */
sfr  p1ddr = P1DDR; 	/* Port 1 data dir      */	 
sfr  p1dr  = P1DR ; 	/* Port 1 data reg      */	 
sfr  port1 = PORT1; 	/* Port 1 reg           */

#define POWER_HOLD_ON p1dr |= BIT7	// Hold on the power - use this macro only for power hold!
#define POWER_HOLD_OFF p1dr &= 0xFF - BIT7	
	 
		   		  
/* Port 2 */
sfr  p2ddr = P2DDR; 	/* Port 2 data dir      */	 
sfr  p2dr  = P2DR ; 	/* Port 2 data reg      */	 
sfr  port2 = PORT2; 	/* Port 2 reg           */	 
	

#define SDA BIT7
#define SCL BIT5
bit sda_bit = p2dr.7;
bit scl_bit = p2dr.5;
	

bit filter_bit = p2dr.2;
#define FILTER 0x40

	
	
	
		   		  
/* Port 3 */
sfr  p3ddr = P3DDR; 	/* Port 3 data dir      */	 
sfr  p3dr  = P3DR ; 	/* Port 3 data reg      */	 
sfr  port3 = PORT3; 	/* Port 3 reg           */	 
sfr  p3odr = P3ODR; 	/* Port 3 open-drain    */	 



/* Port 4 */
sfr  port4 = PORT4; 	/* Port 4 reg           */	 
		   
sfr pulse_out_count = PCDR; /* use this a fast access variable! */

/* Port E */
/* The expansion databus [the low half of the data bus] */
sfr  peddr = PEDDR;   /* Port E data dir      */		   
sfr  pedr  = PEDR ;   /* Port E data          */		   
sfr  porte = PORTE;   /* Port E               */		   
sfr  pepcr = PEPCR;   /* Port E pull-up ctrl  */		   

/* Port F */
sfr  pfddr = PFDDR;   /* Port F data dir      */		   
sfr  pfdr  = PFDR ;   /* Port F data          */		   
sfr  portf = PORTF;   /* Port F               */		   
		   		  
/* Port G */
sfr  pgddr = PGDDR;	  /* Port G data dir      */	   
sfr  pgdr  = PGDR ;	  /* Port G data          */	   
sfr  portg = PORTG;	  /* Port G               */	   

/* The bits to set in the DDR for  port G to enable the chip selects. */
#define CS0 0x10
#define CS1 0x8



/* The serial port defs */


sfr  sci0_smr0	 =	SCI0_SMR0;       /* Serial mode     ch 0 */
sfr  sci0_brr0	 =	SCI0_BRR0;       /* Bit rate        ch 0 */
sfr  sci0_scr0	 =	SCI0_SCR0;       /* Serial control  ch 0 */
sfr  sci0_tdr0	 =	SCI0_TDR0;       /* Transmit data   ch 0 */
sfr  sci0_ssr0	 =	SCI0_SSR0;       /* Serial status   ch 0 */
sfr  sci0_rdr0 	 =	SCI0_RDR0;       /* Receive data    ch 0 */
sfr  sci0_scmr0	 =	SCI0_SCMR0;      /* Smart card mode ch 0 */

sfr  sci1_smr1	 =	SCI1_SMR1;       /* Serial mode     ch 1 */
sfr  sci1_brr1	 =	SCI1_BRR1;       /* Bit rate        ch 1 */
sfr  sci1_scr1	 =	SCI1_SCR1;       /* Serial control  ch 1 */
sfr  sci1_tdr1	 =	SCI1_TDR1;       /* Transmit data   ch 1 */
sfr  sci1_ssr1	 =	SCI1_SSR1;       /* Serial status   ch 1 */
sfr  sci1_rdr1 	 =	SCI1_RDR1;       /* Receive data    ch 1 */
sfr  sci1_scmr1	 =	SCI1_SCMR1;      /* Smart card mode ch 1 */




#define SCR_TIE 0x80	 /* Transmit interrupt enable flag in Serial Control reg. */
#define SCR_RIE 0x40	 /* Rx interrupt enable flag in Serial Control reg. */
#define SCR_TE 0x20	 /* Transmit enable flag in Serial Control reg. */
#define SCR_RE 0x10	 /* Rx enable flag in Serial Control reg. */

 
 




#define TDRE 0x80	 /* Transmit data empty flag in Serial Status Register */
#define RDRF 0x40	 /* Receive Data Register Full in Serial Status Register */
#define ORER 0x20	 /* Overrun error bit */
#define FER	 0x10	 /* Framing error bit */
#define PER	 0x8	 /* Parity error bit */




/* Module stop control register pair, and as individual 8-bit registers */

sfrp mstpcr = MSTPCRH; /* Module stop mode full 16 bit reg */
sfr mstpcrh = MSTPCRH; /* Module stop mode reg, upper byte */
sfr mstpcrl = MSTPCRL; /* Module stop mode reg, lower byte */

/* Bits for use with mstpcrl */
#define SCI0_MODULE_STOP 	0x20
#define SCI1_MODULE_STOP 	0x40

/* Bits for use with mstpcrh as an 8-bit reg. */
#define TMR8_MODULE_STOP 	0x10
#define ADC_MODULE_STOP 	0x2
#define DAC_MODULE_STOP		0x4



/* 8-bit timers */
sfr tmr8_tcr0 = TCR0; 	 	/* Timer 0 control register 0 */
#define CMIEA 0x40	  			/* Counter match A interrupt enable */
#define CCLR1 0x10	  			/* Counter clear source */
#define	CCLR0 0x8	  			/* -"- */
#define	TMR8_PHI_OVER_64 0x2 	/* Clock at Phi / 64 => 250KHz */

sfr tmr8_tcsr0 = TCSR0;	/* Timer control status register 0 */
#define	CMFA 0x40	  			/* Compare match flag A */


sfr tmr8_tcora0 = TCORA0;	/* Time costant register A0 */
sfr tmr8_tcnt0 = TCNT0;	/* Timer counter register 0 */


/* Analogue to Digital Convertor */

sfrp addra  =  ADDRAH;  
sfr  addrah	=  ADDRAH;	
sfr  addral	=  ADDRAL;	

sfrp addrb	=  ADDRBH;	
sfr  addrbh	=  ADDRBH;	
sfr  addrbl	=  ADDRBL;	

sfrp addrc	=  ADDRCH;	
sfr  addrch	=  ADDRCH;	
sfr  addrcl	=  ADDRCL;	
 
sfrp addrd	=  ADDRDH;	
sfr  addrdh	=  ADDRDH;	
sfr  addrdl	=  ADDRDL;	

sfr  adcsr	=  ADCSR;	
sfr  adcr	=  ADCR;	
			

#define ADST		0x20
#define AD_CKS		0x8
#define AD_SCAN 	0x10
#define ADF			0x80

/* Digital to Analogue Converter */

sfr  dadr0  =  DADR0;
sfr  dadr1  =  DADR1;  
sfr  dacr	=  DACR;


#define DAE			0x20
#define DAOE0		0x40
#define DAOE1		0x80	
	

/* TPU defs */

#define TPU_STOP 0x20

sfr tstr = TSTR;



#define TGFA 0x1
#define TGFB 0x2
#define TGIEA 0x1
#define TGIEB 0x2

/* Timer 0 */
sfr tpu_tcr0 = TPU_TCR0;
sfr tpu_tmdr0 = TPU_TMDR0;
sfr tpu_tior0h = TPU_TIOR0H;
sfr tpu_tier0 = TPU_TIER0;
sfr tpu_tsr0 = TPU_TSR0;


sfrp tpu_tcnt0 = TPU_TCNT0;
sfrp tpu_tgr0a = TPU_TGR0A;
sfrp tpu_tgr0b = TPU_TGR0B;
sfrp tpu_tgr0c = TPU_TGR0C;
sfrp tpu_tgr0d = TPU_TGR0D;

/* Timer 1 */
sfr tpu_tcr1 = TPU_TCR1;
sfr tpu_tmdr1 = TPU_TMDR1;
sfr tpu_tior1 = TPU_TIOR1;
sfr tpu_tier1 = TPU_TIER1;
sfr tpu_tsr1 = TPU_TSR1;


sfrp tpu_tcnt1 = TPU_TCNT1;
sfrp tpu_tgr1a = TPU_TGR1A;
sfrp tpu_tgr1b = TPU_TGR1B;


/* Timer 2 */
sfr tpu_tcr2 = TPU_TCR2;
sfr tpu_tmdr2 = TPU_TMDR2;
sfr tpu_tior2 = TPU_TIOR2;
sfr tpu_tier2 = TPU_TIER2;
sfr tpu_tsr2 = TPU_TSR2;


sfrp tpu_tcnt2 = TPU_TCNT2;
sfrp tpu_tgr2a = TPU_TGR2A;
sfrp tpu_tgr2b = TPU_TGR2B;




/* Timer 3 */

sfr tpu_tcr3 = TPU_TCR3;
sfr tpu_tmdr3 = TPU_TMDR3;
sfr tpu_tior3h = TPU_TIOR3H;
sfr tpu_tier3 = TPU_TIER3;
sfr tpu_tsr3 = TPU_TSR3;


sfrp tpu_tcnt3 = TPU_TCNT3;
sfrp tpu_tgr3a = TPU_TGR3A;
sfrp tpu_tgr3b = TPU_TGR3B;
sfrp tpu_tgr3c = TPU_TGR3C;
sfrp tpu_tgr3d = TPU_TGR3D;

/* Timer 4 */

sfr tpu_tcr4 = TPU_TCR4;
sfr tpu_tmdr4 = TPU_TMDR4;
sfr tpu_tior4 = TPU_TIOR4;
sfr tpu_tier4 = TPU_TIER4;
sfr tpu_tsr4 = TPU_TSR4;


sfrp tpu_tcnt4 = TPU_TCNT4;
sfrp tpu_tgr4a = TPU_TGR4A;
sfrp tpu_tgr4b = TPU_TGR4B;


/* System clock control register. */
sfr sckcr = SCKCR;

/* DTC  - Data transfer controller */
sfr  dtcera = DTCERA;
sfr  dtcerb = DTCERB;
sfr  dtcerc = DTCERC;
sfr  dtcerd = DTCERD;
sfr  dtcere = DTCERE;
sfr  dtvecr = DTVECR;

/* Bits in the DTC mode A and mode B 'registers'. */
#define MRA_SRC_FIX    0
#define MRA_SRC_DEC	   0xC0
#define MRA_SRC_INC	   0x80

#define MRA_DST_FIX    0
#define MRA_DST_DEC	   0x30
#define MRA_DST_INC	   0x20

#define MRA_NORM	     0
#define MRA_BLOCK	   0x8
#define MRA_REPEAT	   0x4

#define MRA_DST_IS_RPT_BLK 0	  
#define MRA_SRC_IS_RPT_BLK 0x2	  

#define MRA_SZ_BYTE	     0
#define MRA_SZ_WORD	   0x1

#define MRB_CHNE  0x80
#define MRB_DISEL 0x40

#define SWDTE 0x80


/* Watchdog timer */

/* Watchdog timer defs. */
sfr tcsr_r	 =  TCSR_R;
sfr tcnt_r	 =  TCNT_R;		
sfr rstcsr_r =  RSTCSR_R;		

sfrp tcsr_w	 =  TCSR_W;	
sfrp tcnt_w	 =  TCNT_W;
sfrp rstcsr_w=  RSTCSR_W;	


#define KICK_DOG tcnt_w = 0x5A00 /* Set the counter 0. */