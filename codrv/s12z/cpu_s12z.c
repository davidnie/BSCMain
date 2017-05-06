/*
* cpu_s12z.c
*
* Copyright (c) 2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: cpu_s12z.c 15789 2016-10-27 14:13:55Z ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \file
* \brief 
*	CPU driver for Freescale/NXP S12Z
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>

#include "cpu_s12z.h"
#include "codrv_mscan.h"

/* hardware header
---------------------------------------------------------------------------*/
#include <mc9s12zvc192.h>

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
inline void codrvTimerISR( void);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/

/***************************************************************************/
/**
* \brief codrvTimerSetup - init Timer
*
* Start a cyclic hardware timer to provide timing interval.
* Alternativly it can be derived from an other system timer
* with the interval given from the DeviceDesigner.
*
* \param
*	none
* \results
*	RET_T
*/
RET_T codrvTimerSetup(
		UNSIGNED32 timerInterval
	)

{  
	TIM0TSCR1_PRNT = 1;	// enable  high precision prescaler
	TIM0PTPSR = 1;	// prescale = 2
			
	TIM0TSCR1_TEN = 1;	// timer enable
	TIM0TSCR2_TOI = 1;	// 

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief codrvTimerISR - Timer interrupt service routine
*
* is normally called from timer interrupt or from an other system timer
*
* \param
*	none
* \results
*	none
*/
inline void codrvTimerISR(
		void
    )
{
	/* inform stack about new timer event */
	coTimerTick();
}



/***************************************************************************/
/**
* initCanHW - customer hardware initialization
*
*/
void initCanHW(void)
{
	//enable physical layer
	CP0CR_CPE = 1;
	CP0CR_SPE = 1;
}



/***************************************************************************/
/**
* initHardware - general customer hardware initalization
*/
void codrvHardwareInit(void)
{
	UNSIGNED8 status;

	CPMUCLKS_PLLSEL = 1;				//FBUS = FPLL/2.   FBUS = 32MHz, 
	CPMUREFDIV_REFFRQ = 1;				//Reference clock between 2MHZ and 6MHZ.	
//	CPMUREFDIV_REFDIV = 0x1;		    //FREF=8/(1+1) = 4MHZ		
	CPMUREFDIV_REFDIV = 0x3;		    //FREF=16/(3+1) = 4MHZ	//TMB todo since people can't follow ref designs	
	CPMUSYNR_VCOFRQ = 0x1;             	//FVCO is between 48MHZ and 80MHZ	
	CPMUSYNR_SYNDIV = 0x7;				//FVCO = 2xFREFx(SYNDIV+1)   =   FVCO = 2x4x(7+1) = 64MHZ
	CPMUPOSTDIV_POSTDIV = 0x0;			//FPLL = FVCO/(POSTDIV+1).  FPLL = 64MHZ/(0+1)    FPLL = 64MHz	
	CPMUOSC_OSCE = 1;					//External oscillator enable. 8MHZ.        FREF=FOSC/(REFDIV+1)		
	while(!CPMUIFLG_LOCK){}				// Wait for LOCK.      	
	CPMUIFLG = 0xFF;					// clear CMPMU int flags - not needed but good practice    

	initCanHW();
}

/***************************************************************************/
/**
* interrupt ivVtim0ovf
*/
__interrupt void Timer_int (void) 
{
	/* reset interrupt bit */
	TIM0TFLG2_TOF = 1;

	//	PTP_PTP6 ^= 1;

	codrvTimerISR();
}


/***************************************************************************/
/**
* interrupt CAN0 Rx
*/
__interrupt void codrvCanReceiveInt0()
{
	codrvCanReceiveInterrupt();
}


/***************************************************************************/
/**
* interrupt CAN0 Tx
*/
__interrupt void codrvCanTransmitInt0()
{
	codrvCanTransmitInterrupt();
}


/***************************************************************************/
/**
* interrupt CAN0 Error
*/
__interrupt void codrvCanErrorInt0()
{
	codrvCanErrorInterrupt();
}

