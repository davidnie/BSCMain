/*
* codrv_cpu_generic.c - contains driver for cpu
*
* Copyright (c) 2012-2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: codrv_cpu_generic.c 8086 2015-01-12 13:00:30Z ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief CPU specific routines
*
* \file codrv_cpu_generic.c
* cpu specific routines
*
* This module contains the cpu specific routines for initialization
* and timer handling.
*
* \author emtas GmbH
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_timer.h>

#include <codrv_cpu_generic.h>

/* constant definitions
---------------------------------------------------------------------------*/

/* OS related default definition */
#ifdef CO_OS_SIGNAL_TIMER
#else
#  define CO_OS_SIGNAL_TIMER()
#endif

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/

/***************************************************************************/
/**
* \brief codrvHardwareInit - hardware initialization
*
* This function initializes the hardware, incl. clock and CAN hardware.
*/
void codrvHardwareInit(void)
{
	codrvHardwareCanInit();
}

/***************************************************************************/
/**
* \brief codrvInitCanHW - CAN related hardware initialization
*
* Within this function you find the CAN only hardware part.
* Goal of it is, that you can have your own hardware initialization
* like codrvHardwareInit(), but you can add our tested CAN 
* initialization.
*
*/
void codrvHardwareCanInit(void)
{

}

/***************************************************************************/
/**
* \brief codrvCanEnableInterrupt - enable the CAN interrupt
*
*/
void codrvCanEnableInterrupt(void)
{
	/* enable CAN interrupts */
}

/***************************************************************************/
/**
* \brief codrvCanDisableInterrupt - disable the CAN interrupt
*
*/
void codrvCanDisableInterrupt(void)
{
	/* disable CAN interrupts */
}

/***************************************************************************/
/**
* \brief codrvCanSetTxInterrupt - set pending bit of the Transmit interrupt
*
* This function set the interrupt pending bit. In case of the NVIC
* enable interrupt and the CAN specific enable TX Interrupt mask
* the CAN interrupt handler is calling.
*
*/
void codrvCanSetTxInterrupt(void)
{

}

/***************************************************************************/
/**
* \brief codrvTimerSetup - init and configure the hardware Timer
*
* This function starts a cyclic hardware timer to provide a timing interval
* for the CANopen library.
* Alternativly it can be derived from an other system timer
* with the timer interval given by the function parameter.
*
* \return RET_T
* \retval RET_OK
*	intialization of the timer was ok
*
*/
RET_T codrvTimerSetup(
		UNSIGNED32	timerInterval		/**< timer interval in usec */
	)
{
	/* start hardware timer */


	/* if error */
	// return(RET_INTERNAL_ERROR);

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief codrvTimerISR - Timer interrupt service routine
*
* This function is normally called from timer interrupt
* or from an other system timer.
* It has to call the timer handling function at the library.
*
*
* \return void
*
*/
void codrvTimerISR(
		void
    )
{
	/* inform stack about new timer event */
	coTimerTick();

	/* signal in case of use of an OS */
	CO_OS_SIGNAL_TIMER();
}

