/*
* co_timer.c - contains timer routines
*
* Copyright (c) 2012-2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_timer.c 10353 2015-08-24 14:34:49Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief timer routines
*
* \file co_timer.c
* contains timer routines
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_timer.h>
#include "ico_commtask.h"
#include "ico_timer.h"

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
static RET_T addTimer(CO_TIMER_T *pTimer);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static UNSIGNED32	timerInterVal = 0ul;
static CO_TIMER_T	*pCoTimer = { NULL };
static UNSIGNED32	actTick = { 0u };


/***************************************************************************/
/**
* \brief coTimerStart - start a timer
*
* This function starts a timer with the given timer interval (in µsec).
* If the timer is elapsed,
* the indication function pointed by ptrToFct() with the parameter pData
* is called.
*
* Single-shot or cyclic timer can be defined using the CO_TIMER_ATTR_T attribute.
*
* \return
*	RET_T
*/
RET_T coTimerStart(
		CO_TIMER_T	*pTimer,			/**< pointer to timerstruct */
		UNSIGNED32	timerTime,			/**< timer time in µsec */
		CO_TIMER_FCT_T pFct,			/**< function at timer elapsed */
		void	*pData,					/**< pointer for own data */
		CO_TIMER_ATTR_T	timerAttributes	/**< timer attributes */
	)
{
RET_T	retVal;

	/* check for 0 timer */
	if (timerTime == 0u)  {
		return(RET_INVALID_PARAMETER);
	}

#ifdef xxx
	/* check for to large timer values */
	if (timerTime > (timerInterVal * 0xfffful))  {
		return(RET_INVALID_PARAMETER);
	}
#endif

	/* stop timer and check for validity of pTimer */
    (void) coTimerStop(pTimer);
	
	/* calculate timer ticks */
	pTimer->ticks = timerTime / timerInterVal;
	if ((timerAttributes == CO_TIMER_ATTR_ROUNDUP)
	 || (timerAttributes == CO_TIMER_ATTR_ROUNDUP_CYCLIC)) {
		if ((timerTime % timerInterVal) != 0u)  {
			pTimer->ticks++;
		}
	}
	if (pTimer->ticks == 0u)  {
		pTimer->ticks = 1u;
	}

	pTimer->pFct = pFct;
	pTimer->attr = timerAttributes;
	pTimer->pData = pData;

	retVal = addTimer(pTimer);

	return(retVal);
}


/***************************************************************************/
/**
* \brief coTimerStop - stop a timer
*
* This function stops the given timer.
*
* \return RET_T
* \retval RET_OK
*	timer successful removed
* \retval RET_INVALID_PARAMETER
*	timer not in timer list
*/
RET_T coTimerStop(
		CO_CONST CO_TIMER_T	*pTimer		/**< pointer to timerstruct */
	)
{
CO_TIMER_T	*pT = pCoTimer;
CO_TIMER_T	*pLast = NULL;

	while (pT != NULL)  {
		if (pT == pTimer)  {
			if (pLast == NULL)  {
				pCoTimer = pT->pNext;
			} else {
				pLast->pNext = pT->pNext;
			}
			pT->pNext = NULL;
			return(RET_OK);
		}
		pLast = pT;
		pT = pT->pNext;
	}

	return(RET_INVALID_PARAMETER);
}


/***************************************************************************/
/**
* \brief coTimerIsActive - check if timer is active
*
* With this function can be ckecked,
* if a timer is currently in the timer list.
*
* \return BOOL_T
* \retval CO_TRUE
*	timer is active
* \retval CO_FALSE
*	timer is not active
*
*/
BOOL_T coTimerIsActive(
		CO_CONST CO_TIMER_T	*pTimer		/**< pointer to timer struct */
	)
{
CO_TIMER_T	*pT = pCoTimer;

	while (pT != NULL)  {
		if (pT == pTimer)  {
			return(CO_TRUE);
		}
		pT = pT->pNext;
	}

	return(CO_FALSE);
}


/***************************************************************************/
/**
* \brief coTimerAttrChange - change timer attribute
*
* With this function timer attribute can be change.
*
* \return none
*
*/
void coTimerAttrChange(
		CO_TIMER_T	*pTimer,			/**< pointer to timerstruct */
		CO_TIMER_ATTR_T	timerAttributes	/**< timer attributes */
	)
{
	pTimer->attr = timerAttributes;	

	return;
}


/***************************************************************************/
/**
* \internal
*
* \brief addTimer - add timer to timer list
*
* \return RET_T
*
*/
static RET_T addTimer(
		CO_TIMER_T		*pTimer		/* pointer to timer structure */
	)
{
CO_TIMER_T	*pT;
CO_TIMER_T	*pLast;

	pTimer->actTicks = pTimer->ticks + actTick;

	/* first timer ? */
	if (pCoTimer == NULL)  {
		pCoTimer = pTimer;
		pTimer->pNext = NULL;
		return(RET_OK);
	}

	/* look for correct index */
	pT = pCoTimer;
	pLast = NULL;
	while ((pTimer->actTicks > pT->actTicks) && (pT->pNext != NULL)) {
		pLast = pT;
		pT = pT->pNext;
	}

	/* save it before */
	if (pTimer->actTicks < pT->actTicks)  {
		if (pLast == NULL)  {
			pCoTimer = pTimer;
		} else {
			pLast->pNext = pTimer;
		}
		pTimer->pNext = pT;
	} else {
		if (pT->pNext == NULL)  {
			pT->pNext = pTimer;
			pTimer->pNext = NULL;
		} else {
			pTimer->pNext = pT->pNext;
			pT->pNext = pTimer;
		}
	}

#ifdef xxx
{
CO_TIMER_T	*pT = pCoTimer;

printf("timerAdd\n");
	while (pT != NULL)  {
		printf("timer: %d\n", pT->actTicks);
		pT = pT->pNext;
	}
	printf("\n");
}
#endif

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coTimerTick - timer tick elapsed
*
* This function should be called,
* if the CANopen timer has been elapsed
* to signal a new timer interval to the stack.
*
* It can be called at interrupt level.
*
* \return none
*
*/
void coTimerTick(
		void
	)
{

	{
		coCommTaskSet(CO_COMMTASK_EVENT_TIMER);
		actTick++;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTimerCheck - check next timer
*
* \return none
*
*/
void icoTimerCheck(
		void	/* no parameter */
	)
{
CO_TIMER_T	*pT;
CO_TIMER_T	*pAct = NULL;
UNSIGNED32	ticks;

	/* if no timer enabled ? */
	if (pCoTimer == NULL)  {
		actTick = 0u;
		return;
	}

	/* save act ticks */
	pT = pCoTimer;
	ticks = actTick;

	/* first timer reached ? */
	if (ticks < pT->actTicks)  {
		return;
	}

	actTick -= ticks;

	if (ticks > pT->actTicks)  {
		ticks = pT->actTicks;
	}

	/* sub ticks from all timers */
	while (pT != NULL)  {
		pT->actTicks -= ticks;
		pT = pT->pNext;
	}

	/* call timer functions */
	pT = pCoTimer;
	while (pT != NULL)  {
		if (pT->actTicks == 0u)  {
			pAct = pT;
			pT = pAct->pNext;

			/* remove timer from list */
			(void) coTimerStop(pAct);

			/* call function */
			if (pAct->pFct != NULL)  {
				pAct->pFct(pAct->pData);
			}

			/* cyclic timer ? */
			if ((pAct->attr == CO_TIMER_ATTR_ROUNDUP_CYCLIC) 
			 || (pAct->attr == CO_TIMER_ATTR_ROUNDDOWN_CYCLIC))  {
				/* add timer again */
				(void) addTimer(pAct);
			}
		} else {
			pT = pT->pNext;
		}
	}
}


/***************************************************************************/
/**
* \brief coTimerInit - init timer interval
*
* This function initializes the internal timer handling.
* It does nothing with the hardware timer
* and initializes only internal variables.
* <br>The given timer interval is used to calculate
* the timer period for timer depending functions
* started by coTimerStart().
*
* \return none
*
*/
void coTimerInit(
		UNSIGNED32	timerVal		/**< timer interval in µsec */
	)
{

	timerInterVal = timerVal;

	{
		pCoTimer = NULL;
		actTick = 0u;
	}
}


#ifdef xxx
int coTimeDiff()
{
#include <sys/time.h>
static struct timeval savetv;
struct timeval tv;
struct timezone tz;

	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;

	gettimeofday(&tv, &tz);
	if ((tv.tv_usec - savetv.tv_usec) > 20000)  {
		//printf("time %d:%06d\n", tv.tv_sec - savetv.tv_sec, tv.tv_usec - savetv.tv_usec);
	}
	savetv.tv_sec = tv.tv_sec;
	savetv.tv_usec = tv.tv_usec;
}
#endif
