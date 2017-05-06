/*
* co_time.c - contains TIME services
*
* Copyright (c) 2012-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_time.c 8616 2015-02-27 15:27:50Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief time handling
*
* \file co_time.c
* contains TIME services
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#ifdef CO_TIME_SUPPORTED
#include <co_datatype.h>
#include <co_odaccess.h>
#include <co_time.h>
#include <co_timer.h>
#include <co_drv.h>
#include <co_nmt.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_nmt.h"
#include "ico_time.h"

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_TIME
# ifdef CO_EVENT_PROFILE_TIME
#  define CO_EVENT_TIME_CNT	(CO_EVENT_DYNAMIC_TIME + CO_EVENT_PROFILE_TIME)
# else /* CO_EVENT_PROFILE_TIME */
#  define CO_EVENT_TIME_CNT	(CO_EVENT_DYNAMIC_TIME)
# endif /* CO_EVENT_PROFILE_TIME */
#else /* CO_EVENT_DYNAMIC_TIME */
# ifdef CO_EVENT_PROFILE_TIME
#  define CO_EVENT_TIME_CNT	(CO_EVENT_PROFILE_TIME)
# endif /* CO_EVENT_PROFILE_TIME */
#endif /* CO_EVENT_DYNAMIC_TIME */


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_TIME
extern CO_CONST CO_EVENT_TIME_T coEventTimeInd;
#endif /* CO_EVENT_STATIC_TIME */


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
static COB_REFERENZ_T	timeProdCob;
static COB_REFERENZ_T	timeConsCob;
static UNSIGNED32	timeCobId;
#ifdef CO_EVENT_TIME_CNT
static CO_EVENT_TIME_T	timeEventTable[CO_EVENT_TIME_CNT];
static UNSIGNED8	timeEventTableCnt = 0u;
#endif /* CO_EVENT_TIME_CNT */


/***************************************************************************/
/**
* \internal
*
* \brief icoSynandler - handle TIME
*
*
* \return none
*
*/
void icoTimeHandler(
		const UNSIGNED8	pData[]
	)
{
CO_TIME_T	timeData;
#ifdef CO_EVENT_TIME_CNT
UNSIGNED16	cnt;
#endif /* CO_EVENT_TIME_CNT */

	(void)coNumMemcpyPack(&timeData.msec, &pData[0], 4u, 1u, 0u);
	(void)coNumMemcpyPack(&timeData.days, &pData[4], 2u, 1u, 0u);

#ifdef CO_EVENT_TIME_CNT
	/* call indication to execute */
	cnt = timeEventTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		timeEventTable[cnt](&timeData);
	}
#endif /* CO_EVENT_TIME_CNT */

#ifdef CO_EVENT_STATIC_TIME
	coEventTimeInd(&timeData);
#endif /* CO_EVENT_STATIC_TIME */
}


/***************************************************************************/
/**
*
* \brief coTimeWriteReq - write time request
*
* This function sends a time message to the bus.
*
* \return RET_T
*
*/
RET_T coTimeWriteReq(
		const CO_TIME_T	*pTimeData		/**< time data to transmit */
	)
{
CO_TRANS_DATA_T	trData;
CO_NMT_STATE_T	nmtState;
RET_T			retVal;

	if (timeProdCob == 0xffffu)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* OPERATIONAL ? */
	nmtState = coNmtGetState();
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return(RET_INVALID_NMT_STATE);
	}

	/* time producer enabled ? */
	if ((timeCobId & CO_TIME_PRODUCER_BIT) == 0u)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	coNumMemcpyUnpack(&trData.data[0], &pTimeData->msec, 4u, 1u, 0u);
	coNumMemcpyUnpack(&trData.data[4], &pTimeData->days, 2u, 1u, 0u);

	/* transmit data */
	retVal = icoTransmitMessage(timeProdCob, &trData, MSG_OVERWRITE);

	return(retVal);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief icoTimeGetObjectAddr - get time object address
*
* \return none
*
*/
void *icoTimeGetObjectAddr(
		void	/* no parameter */
	)
{
	return(&timeCobId);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTimeCheckObjLimit_u32 - check UNSIGNED32 object limits
*
*
* \return RET_T
*
*/
RET_T	icoTimeCheckObjLimit_u32(
		UNSIGNED32	value			/* new value */
	)
{
RET_T	retVal = RET_OK;

	if ((value & CO_TIME_PRODUCER_BIT) != 0u)  {
		if (timeProdCob == 0xffffu)  {
			retVal = RET_SDO_INVALID_VALUE;
		}
	}

	if ((timeCobId & CO_TIME_CONSUMER_BIT) != 0u)  {
		if (timeConsCob == 0xffffu)  {
			retVal = RET_SDO_INVALID_VALUE;
		}
	}

	/* no change of ID if the object exist */
	if ((timeCobId & (CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT)) != 0u)  {
		/* change can-id ? */
		if ((((timeCobId ^ value)
			& ~(CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT)) != 0u)
		 && ((value & (CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT)) != 0u))  {
			return(RET_SDO_INVALID_VALUE);
		}
	}
	if (icoCheckRestrictedCobs(
				value & ~(CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT),
				0x100u, 0x100u
			) == CO_TRUE)  {
		return(RET_SDO_INVALID_VALUE);
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTimeObjChanged - time object changed
*
*
* \return RET_T
*
*/
RET_T icoTimeObjChanged(
		void	/* no parameter */
	)
{
RET_T	retVal = RET_OK;

	if (timeProdCob != 0xffffu)  {
		if ((timeCobId & CO_TIME_PRODUCER_BIT) != 0u)  {
			retVal = icoCobSet(timeProdCob,
				timeCobId & ~(CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT),
				CO_COB_RTR_NONE, 6u);
		} else {
			retVal = icoCobSet(timeProdCob,
				timeCobId | 0x80000000ul,
				CO_COB_RTR_NONE, 6u);
		}
	}

	if (timeConsCob != 0xffffu)  {
		if ((timeCobId & CO_TIME_CONSUMER_BIT) != 0u)  {
			retVal = icoCobSet(timeConsCob,
				timeCobId & ~(CO_TIME_PRODUCER_BIT | CO_TIME_CONSUMER_BIT),
				CO_COB_RTR_NONE, 6u);
		} else {
			retVal = icoCobSet(timeConsCob,
				timeCobId | 0x80000000ul,
				CO_COB_RTR_NONE, 6u);
		}
	}

	return(retVal);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
# ifdef CO_EVENT_TIME_CNT
/***************************************************************************/
/**
* \brief coEventRegister_TIME - register TIME event
*
* This function registers an indication function for TIME events.
*
* \return RET_T
*
*/

RET_T coEventRegister_TIME(
		CO_EVENT_TIME_T pFunction	/**< pointer to function */
    )
{
	if (timeEventTableCnt >= CO_EVENT_TIME_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	timeEventTable[timeEventTableCnt] = pFunction;
	timeEventTableCnt++;

	return(RET_OK);
}
# endif /* CO_EVENT_TIME_CNT */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief coResetTime - reset comm for Time
*
* \return none
*
*/
void icoTimeReset(
		void	/* no parameter */
	)
{
	(void) icoTimeObjChanged();
}


/***************************************************************************/
/**
* \internal
*
* \brief coTimeSetDefaultValue
*
* \return none
*
*/
void icoTimeSetDefaultValue(
		void	/* no parameter */
	)
{
RET_T	retVal;
UNSIGNED32	u32;

	timeCobId = 0x100ul;

	/* setup reset values */
	retVal = coOdGetDefaultVal_u32(0x1012u, 0u, &u32);
	if (retVal == RET_OK)  {
		timeCobId |= u32 & 0xc0000000ul;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTimeVarInit
*
* \return none
*
*/
void icoTimeVarInit(
		void
	)
{

	{
		timeProdCob = 0xffffu;
		timeConsCob = 0xffffu;
	}

#ifdef CO_EVENT_TIME_CNT
	timeEventTableCnt = 0u;
#endif /* CO_EVENT_TIME_CNT */
}


/***************************************************************************/
/**
* \brief coTimeInit - init time functionality
*
* This function initializes the TIME functionality.
* The parameters give the possibilities to be producer and/or consumer,
* independ on the current value of the entry in the object dictionary.
*
* \return RET_T
*
*/
RET_T coTimeInit(
		BOOL_T	producer,		/**< node can be time producer */
		BOOL_T	consumer		/**< node can be time consumer */
	)
{
	/* time producer ? */
	if (producer == CO_TRUE)  {
		timeProdCob = icoCobCreate(CO_COB_TYPE_TRANSMIT,
				CO_SERVICE_TIME_TRANSMIT, 0u);
		if (timeProdCob == 0xffffu)  {
			return(RET_NO_COB_AVAILABLE);
		}
	} else {
		timeProdCob = 0xffffu;
	}

	/* time consumer ? */
	if (consumer == CO_TRUE)  {
		timeConsCob = icoCobCreate(CO_COB_TYPE_RECEIVE,
				CO_SERVICE_TIME_RECEIVE, 0u);
		if (timeConsCob == 0xffffu)  {
			return(RET_NO_COB_AVAILABLE);
		}
	} else {
		timeConsCob = 0xffffu;
	}

	return(RET_OK);
}
#endif /* CO_TIME_SUPPORTED */
