/*
* co_event.c - contains event routines
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_event.c 13641 2016-04-28 12:46:42Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief event routines
*
* \file co_event.c
* contains event routines
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>	/*lint !e766 depends on configuration */

#include <co_datatype.h>
#include "ico_commtask.h"
#include "ico_event.h"

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

/* local defined variables
---------------------------------------------------------------------------*/
static CO_EVENT_T	*pCoEvent = { NULL };


/***************************************************************************/
/**
* \brief coEventStart - start a event
*
* This function add an event at end of the event list
*
* \return
*	RET_T
*/
RET_T icoEventStart(
		CO_EVENT_T	*pEvent,			/**< pointer to eventstruct */
		CO_EVENT_FCT_T	ptrToFct,		/**< function for event */
		void		*pData				/**< pointer for own data */
	)
{
CO_EVENT_T	*pT;

	pEvent->pFct = ptrToFct;
	pEvent->pData = pData;

	/* first event ? */
	if (pCoEvent == NULL)  {
		pCoEvent = pEvent;
	} else {
		/* add at end of list */
		pT = pCoEvent;
		while (pT->pNext != NULL) {
			pT = pT->pNext;
		}
		pT->pNext = pEvent;
	}

	pEvent->pNext = NULL;
	coCommTaskSet(CO_COMMTASK_EVENT_NEW_EVENT);

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coEventIsActive - check if event is active
*
* With this function can be ckecked,
* if a event is currently in the event list.
*
* \return BOOL_T
* \retval CO_TRUE
*	event is active
* \retval CO_FALSE
*	event is not active
*
*/
BOOL_T icoEventIsActive(
		CO_CONST CO_EVENT_T	*pEvent		/**< pointer to event struct */
	)
{
CO_EVENT_T	*pT = pCoEvent;

	while (pT != NULL)  {
		if (pT == pEvent)  {
			return(CO_TRUE);
		}
		pT = pT->pNext;
	}

	return(CO_FALSE);
}



/***************************************************************************/
/**
* \internal
*
* \brief icoEventCheck - check next event
*
* \return none
*
*/
void icoEventCheck(
		void	/* no parameter */
	)
{
CO_EVENT_T	*pAct = NULL;
CO_EVENT_T	*pEv = pCoEvent;

	pCoEvent = NULL;

	while (pEv != NULL)  {
		pAct = pEv;

		/* remove event from list */
		pEv = pAct->pNext;
		pAct->pNext = NULL;

		/* call event functions */
		if (pAct->pFct != NULL)  {
			pAct->pFct(pAct->pData);
		}
	}
}


/***************************************************************************/
/**
* \brief icoEventInit - init event interval
*
* This function initializes the internal event handling.
*
* \return none
*
*/
void icoEventInit(
		void
	)
{

	{
		pCoEvent = NULL;
	}
}
