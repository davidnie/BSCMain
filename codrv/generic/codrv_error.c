/*
* error state handler
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* $Id: codrv_error.c 8086 2015-01-12 13:00:30Z ro $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \file
* \brief   error state handling
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdio.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_commtask.h>
#include <co_drv.h>

#include <codrv_error.h>


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

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/
static CAN_ERROR_FLAGS_T canErrorFlags;

/* local defined variables
---------------------------------------------------------------------------*/

/***************************************************************************/
/**
* \brief codrvCanErrorgetFlags - Reference to the error flags
*
* \retval
*	pointer to error flags
*
*/

CAN_ERROR_FLAGS_T * codrvCanErrorGetFlags(void)
{
	return &canErrorFlags;
}


/***************************************************************************/
/**
* \brief codrvCanErrorInit - init Error variables
*
*/

void codrvCanErrorInit(void)
{
CAN_ERROR_FLAGS_T * pError;

	pError = codrvCanErrorGetFlags();

	pError->canErrorRxOverrun = CO_FALSE; 

	pError->canErrorPassive = CO_FALSE;
	pError->canErrorActive = CO_FALSE;
	pError->canErrorBusoff = CO_FALSE;

	pError->canOldState = Error_Offline; /* last signaled state */

}

/***********************************************************************/
/**
* codrvCanErrorInformStack - inform the stack about changes
*
* Call outside of interrupts!
* Typical call in codrvCanDriverHandler().
*
*/

RET_T codrvCanErrorInformStack(void) 
{
CAN_ERROR_FLAGS_T * pError;

	pError = codrvCanErrorGetFlags();

	if (pError->canErrorRxOverrun == CO_TRUE) {		
		pError->canErrorRxOverrun = CO_FALSE;
		/* printf("RX Overrun\n");*/
		coCommStateEvent(CO_COMM_STATE_EVENT_CAN_OVERRUN);
	}

	/* signal CAN Error chanhed events 
	* 	(Active <-> Passive -> Busoff -> Active)
	*/

	if ((pError->canNewState == Error_Busoff) 
			|| (pError->canErrorBusoff == CO_TRUE)) 
	{
		if (pError->canOldState == Error_Active) {
			/* melde Passive */
			/* printf("Passive\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_PASSIVE);
		}
		if (pError->canOldState != Error_Busoff) {
			/* melde Busoff */
			/* printf("Busoff\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_BUS_OFF);
		}
	}

	if ((pError->canOldState == Error_Busoff) 
		|| (pError->canOldState == Error_Offline) 
		|| (pError->canErrorBusoff == CO_TRUE))
	{
		if (pError->canNewState == Error_Passive) {
			/* melde Active */
			/* printf("Active\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_ACTIVE);
			/* melde Passive */
			/* printf("Passive\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_PASSIVE);
		}

	}


	if ((pError->canOldState == Error_Active) 
			&& (pError->canNewState == Error_Passive)
			&& (pError->canErrorBusoff == CO_FALSE))
	{
		/* melde Passive */
		/* printf("Passive\n"); */
		coCommStateEvent(CO_COMM_STATE_EVENT_PASSIVE);
	}


	if ((pError->canOldState == Error_Active) 
		&& (pError->canNewState == Error_Active))
	{
		if (pError->canErrorBusoff == CO_TRUE) {
			/* melde Active */
			/* printf("Active\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_ACTIVE);
		} else
		if (pError->canErrorPassive == CO_TRUE) {
			/* melde Passive */
			/* printf("Passive\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_PASSIVE);
			/* melde Active */
			/* printf("Active\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_ACTIVE);
		}

	}

	/* if ((pError->canOldState == Error_Passive) 
		|| (pError->canOldState == Error_Busoff) 
		|| (pError->canOldState == Error_Offline)) */
	if (pError->canOldState != Error_Active) 
	{
		if (pError->canNewState == Error_Active) {
			/* melde Active */
			/* printf("Active\n"); */
			coCommStateEvent(CO_COMM_STATE_EVENT_ACTIVE);
		}

	}


	pError->canErrorPassive = CO_FALSE;
	pError->canErrorActive = CO_FALSE;
	pError->canErrorBusoff = CO_FALSE;
	pError->canOldState = pError->canNewState;

	return RET_OK;
}


