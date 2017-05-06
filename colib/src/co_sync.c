/*
* co_sync.c - contains SYNC services
*
* Copyright (c) 2012-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_sync.c 10881 2015-10-02 09:06:25Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief sync handling
*
* \file co_sync.c
* contains SYNC services
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_SYNC_SUPPORTED
#include <co_datatype.h>
#include <co_timer.h>
#include <co_odaccess.h>
#include <co_sync.h>
#include <co_drv.h>
#include <co_nmt.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_pdo.h"
#include "ico_nmt.h"
#include "ico_sync.h"

/* constant definitions
---------------------------------------------------------------------------*/
#define CO_SYNC_PRODUCER_BIT	0x40000000ul

#ifdef CO_EVENT_DYNAMIC_SYNC
# ifdef CO_EVENT_PROFILE_SYNC
#  define CO_EVENT_SYNC_CNT	(CO_EVENT_DYNAMIC_SYNC + CO_EVENT_PROFILE_SYNC)
# else /* CO_EVENT_PROFILE_SYNC */
#  define CO_EVENT_SYNC_CNT	(CO_EVENT_DYNAMIC_SYNC)
# endif /* CO_EVENT_PROFILE_SYNC */
#else /* CO_EVENT_DYNAMIC_SYNC */
# ifdef CO_EVENT_PROFILE_SYNC
#  define CO_EVENT_SYNC_CNT	(CO_EVENT_PROFILE_SYNC)
# endif /* CO_EVENT_PROFILE_SYNC */
#endif /* CO_EVENT_DYNAMIC_SYNC */


#if defined(CO_EVENT_STATIC_SYNC) || defined(CO_EVENT_SYNC_CNT)
# define CO_EVENT_SYNC   1
#endif /* defined(CO_EVENT_STATIC_SYNC) || defined(CO_EVENT_SYNC_CNT) */


#ifdef CO_EVENT_DYNAMIC_SYNC_FINISHED
# ifdef CO_EVENT_PROFILE_SYNC_FINISHED
#  define CO_EVENT_SYNC_FINISHED_CNT	(CO_EVENT_DYNAMIC_SYNC_FINISHED + CO_EVENT_PROFILE_SYNC_FINISHED)
# else /* CO_EVENT_PROFILE_SYNC_FINISHED */
#  define CO_EVENT_SYNC_FINISHED_CNT	(CO_EVENT_DYNAMIC_SYNC_FINISHED)
# endif /* CO_EVENT_PROFILE_SYNC_FINISHED */
#else /* CO_EVENT_DYNAMIC_SYNC_FINISHED */
# ifdef CO_EVENT_PROFILE_SYNC_FINISHED
#  define CO_EVENT_SYNC_FINISHED_CNT	(CO_EVENT_PROFILE_SYNC_FINISHED)
# endif /* CO_EVENT_PROFILE_SYNC_FINISHED */
#endif /* CO_EVENT_DYNAMIC_SYNC_FINISHED */

#if defined(CO_EVENT_STATIC_SYNC_FINISHED) || defined(CO_EVENT_SYNC_FINISHED_CNT)
# define CO_EVENT_SYNC_FINISHED   1
#endif /* defined(CO_EVENT_STATIC_SYNC_FINISHED) || defined(CO_EVENT_SYNC_FINISHED_CNT) */


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_SYNC
extern CO_CONST CO_EVENT_SYNC_T coEventSyncInd;
#endif /* CO_EVENT_STATIC_SYNC */

#ifdef CO_EVENT_STATIC_SYNC_FINISHED
extern CO_CONST CO_EVENT_SYNC_FINISHED_T coEventSyncFinishedInd;
#endif /* CO_EVENT_STATIC_SYNC_FINISHED */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void syncEventTimer(void *ptr);
# ifdef CO_EVENT_SYNC
static void syncInd(UNSIGNED8	syncCounterVal);
# endif /* CO_EVENT_SYNC */
# ifdef CO_EVENT_SYNC_FINISHED
static void syncFinishedInd(UNSIGNED8	syncCounterVal);
# endif /* CO_EVENT_SYNC_FINISHED */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static COB_REFERENZ_T	syncCob;
static UNSIGNED32 syncCobId = { 0x80ul };
static UNSIGNED32 cyclePeriod = { 0ul };
static UNSIGNED8 counterValue = { 0u };
static UNSIGNED8 actCounterValue = { 1u };
static UNSIGNED32 windowLength = { 0ul };
static CO_TIMER_T	syncTimer;
# ifdef CO_EVENT_SYNC_CNT
static UNSIGNED16	syncEventTableCnt = 0u;
static CO_EVENT_SYNC_T syncEventTable[CO_EVENT_SYNC_CNT];
# endif /* CO_EVENT_SYNC_CNT */
# ifdef CO_EVENT_SYNC_FINISHED_CNT
static CO_EVENT_SYNC_FINISHED_T syncFinishedEventTable[CO_EVENT_SYNC_FINISHED_CNT];
static UNSIGNED16	syncFinishedEventTableCnt = 0u;
# endif /* CO_EVENT_SYNC_FINISHED_CNT */


/***************************************************************************/
/**
* \internal
*
* \brief icoSynandler - handle SYNC
*
*
* \return none
*
*/
void icoSyncHandler(
		UNSIGNED8	syncCounterVal		/* sync counter value */
	)
{
# ifdef CO_EVENT_SYNC
	/* start user indication */
	syncInd(syncCounterVal);
# endif /* CO_EVENT_SYNC */

# if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)
	/* work PDO handling */
	icoPdoSyncHandling(syncCounterVal);
# endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */

# ifdef CO_EVENT_SYNC_FINISHED
	/* end user indication */
	syncFinishedInd(syncCounterVal);
# endif /* CO_EVENT_SYNC_FINISHED */
}


/***************************************************************************/
/**
* \internal
*
* \brief syncEventTimer - called from timer to transmit sync message
*
*
* \return none
*
*/
static void syncEventTimer(
		void			*ptr		/* compatibility entry */
	)
{
CO_TRANS_DATA_T	trData;
CO_NMT_STATE_T	nmtState;
(void)ptr;

	/* OPERATIONAL ? */
	nmtState = coNmtGetState();
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return;
	}

	trData.data[0] = actCounterValue;
	actCounterValue ++;
	if (actCounterValue > counterValue)  {
		actCounterValue = 1u;
	}

	/* transmit data */
	(void) icoTransmitMessage(syncCob, &trData, MSG_OVERWRITE);

	if (counterValue != 0u)  {
		icoSyncHandler(trData.data[0]);
	} else {
		icoSyncHandler(0u);
	}

	return;
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief icoSyncGetObjectAddr - get sync object address
*
* \return none
*
*/
void *icoSyncGetObjectAddr(
		UNSIGNED16	index				/* index */
	)
{
void	*pAddr = NULL;

	switch (index)  {
		case 0x1005u:
			pAddr = (void *)&syncCobId;
			break;
		case 0x1006u:
			pAddr = (void *)&cyclePeriod;
			break;
		case 0x1007u:
			pAddr = (void *)&windowLength;
			break;
		case 0x1019u:
			pAddr = (void *)&counterValue;
			break;
		default:
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSyncCheckObjLimit_u8 - check UNSIGNED8 object limits
*
*
* \return RET_T
*
*/
RET_T	icoSyncCheckObjLimit_u8(
		UNSIGNED16	index,			/* index */
		UNSIGNED8	value			/* new value */
	)
{
	if (index == 0x1019u)  {
		if ((value == 1u) || (value > 240u))  {
			return(RET_SDO_INVALID_VALUE);
		}
		if (cyclePeriod != 0u)  {
			return(RET_ERROR_PRESENT_DEVICE_STATE);
		}
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSyncCheckObjLimit_u32 - check UNSIGNED32 object limits
*
*
* \return RET_T
*
*/
RET_T	icoSyncCheckObjLimit_u32(
		UNSIGNED16	index,			/* index */
		UNSIGNED32	value			/* new value */
	)
{
CO_CONST CO_OBJECT_DESC_T *pDescPtr;

	if (index == 0x1005u)  {
		/* sync producer only allowed if 1006 is available */
		if ((value & CO_SYNC_PRODUCER_BIT) != 0u)  {
			if (coOdGetObjDescPtr(0x1006u, 0u, &pDescPtr) != RET_OK)  {
				return(RET_SDO_INVALID_VALUE);
			}
		}
		/* check maximum value */
		if (value > 0x4ffffffful)  {
			return(RET_SDO_INVALID_VALUE);
		}
		/* check restricted CAN ids */
		if (icoCheckRestrictedCobs(value, 0x80u, 0x80u) == CO_TRUE)  {
			return(RET_SDO_INVALID_VALUE);
		}
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSyncObjChanged - sync object changed
*
*
* \return RET_T
*
*/
RET_T icoSyncObjChanged(
		UNSIGNED16	valIdx				/* index */
	)
{
RET_T	retVal = RET_OK;

	switch (valIdx)  {
		case 0x1005u:	/* cob-id or cycle period */
		case 0x1006u:
			if ((cyclePeriod != 0u)
			 && ((syncCobId & CO_SYNC_PRODUCER_BIT) != 0u))  {
				/* sync producer */
				retVal = icoCobChangeType(syncCob, CO_COB_TYPE_TRANSMIT);
				if (retVal != RET_OK)  {
					return(retVal);
				}
				if (counterValue != 0u)  {
					retVal = icoCobSet(syncCob, syncCobId, CO_COB_RTR_NONE, 1u);
				} else {
					retVal = icoCobSet(syncCob, syncCobId, CO_COB_RTR_NONE, 0u);
				}
				if (retVal != RET_OK)  {
					return(retVal);
				}
				/* start timer */
				retVal = coTimerStart(&syncTimer,
					cyclePeriod,
					syncEventTimer, NULL, CO_TIMER_ATTR_ROUNDDOWN_CYCLIC);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
			} else {
				retVal = icoCobChangeType(syncCob, CO_COB_TYPE_RECEIVE);
				if (retVal != RET_OK)  {
					return(retVal);
				}
				if (counterValue != 0u)  {
					actCounterValue = 1u;
					retVal = icoCobSet(syncCob, syncCobId, CO_COB_RTR_NONE, 1u);
				} else {
					retVal = icoCobSet(syncCob, syncCobId, CO_COB_RTR_NONE, 0u);
				}
				(void) coTimerStop(&syncTimer);
			}
			break;
		default:
			break;
	}
	return(retVal);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
# ifdef CO_EVENT_SYNC_CNT
/***************************************************************************/
/**
* \brief coEventRegister_SYNC - register SYNC event
*
* This function registers an indication function for SYNC events.
*
* It is called every time a sync message was received or generated
* before PDOs are handled.
*
* \return RET_T
*
*/

RET_T coEventRegister_SYNC(
		CO_EVENT_SYNC_T pFunction	/**< pointer to function */
    )
{
	if (syncEventTableCnt >= CO_EVENT_SYNC_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}
	/* insanity check */
	if (pFunction == NULL) {
		return(RET_INVALID_PARAMETER);
	}
	
	/* set new indication function as first at the list */
	syncEventTable[syncEventTableCnt] = pFunction;
	syncEventTableCnt++;

	return(RET_OK);
}
# endif /* CO_EVENT_SYNC_CNT */


# ifdef CO_EVENT_SYNC
/***************************************************************************/
/**
* \internal
*
* \brief syncInd - call sync indications
*
* \return
*	RET_T
*/
static void syncInd(
		UNSIGNED8	syncCounterVal		/* sync counter value */
	)
{
UNSIGNED16	cnt;

# ifdef CO_EVENT_SYNC_CNT
	/* call indication to execute */
	cnt = syncEventTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		syncEventTable[cnt](syncCounterVal);
	}
# endif /* CO_EVENT_SYNC_CNT */

# ifdef CO_EVENT_STATIC_SYNC
	coEventSyncInd(syncCounterVal);
# endif /* CO_EVENT_STATIC_SYNC */

	return;
}
# endif /* CO_EVENT_SYNC */


# ifdef CO_EVENT_SYNC_FINISHED_CNT
/***************************************************************************/
/**
* \brief coEventRegister_SYNC_FINISHED - register SYNC finished event
*
* This function registers an indication function for finished SYNC handling.
*
* It is called every time a sync message was received or generated
* and PDO handling is completed.
*
* \return RET_T
*
*/

RET_T coEventRegister_SYNC_FINISHED(
		CO_EVENT_SYNC_FINISHED_T pFunction	/**< pointer to function */
    )
{
	if (syncFinishedEventTableCnt >= CO_EVENT_SYNC_FINISHED_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	syncFinishedEventTable[syncFinishedEventTableCnt] = pFunction;
	syncFinishedEventTableCnt++;

	return(RET_OK);
}
# endif /* CO_EVENT_SYNC_FINISHED_CNT */


# ifdef CO_EVENT_SYNC_FINISHED
/***************************************************************************/
/**
* \brief syncFinishedInd - call sync finish indication
*
* \return
*	RET_T
*/
static void syncFinishedInd(
		UNSIGNED8	syncCounterVal		/* sync counter value */
	)
{
UNSIGNED16	cnt;

# ifdef CO_EVENT_SYNC_FINISHED_CNT
	/* call indication to execute */
	cnt = syncFinishedEventTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		syncFinishedEventTable[cnt](syncCounterVal);
	}
# endif /* CO_EVENT_SYNC_FINISHED_CNT */

# ifdef CO_EVENT_STATIC_SYNC_FINISHED
	coEventSyncFinishedInd(syncCounterVal);
# endif /* CO_EVENT_STATIC_SYNC_FINISHED */

	return;
}
# endif /* CO_EVENT_SYNC_FINISHED */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief coResetNmt - reset comm for nmt
*
* \return none
*
*/
void icoSyncReset(
		void	/* no parameter */
	)
{
	(void) icoSyncObjChanged(0x1005u);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSyncVarInit
*
* \return none
*
*/
void icoSyncVarInit(
		void
	)
{

	{
		syncCobId = 0x80ul;
		cyclePeriod = 0ul;
		counterValue = 0u;
		actCounterValue = 1u;
		windowLength = 0ul;
		syncCob = 0xffffu;
	}

#ifdef CO_EVENT_SYNC
	syncEventTableCnt = 0u;
#endif /* CO_EVENT_SYNC */

#ifdef CO_EVENT_SYNC_FINISHED
	syncFinishedEventTableCnt = 0u;
#endif /* CO_EVENT_SYNC_FINISHED */
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSyncSetDefaultValue - set predefined connection set
*
* \return none
*
*/
void icoSyncSetDefaultValue(
		void	/* no parameter */
	)
{
RET_T	retVal;
UNSIGNED32	u32;
UNSIGNED8	u8;


	/* setup reset values */
	retVal = coOdGetDefaultVal_u32(0x1005u, 0u, &u32);
	if (retVal == RET_OK)  {
		syncCobId = u32;
	} else {
		syncCobId = 0x80ul;
	}
	retVal = coOdGetDefaultVal_u32(0x1006u, 0u, &u32);
	if (retVal == RET_OK)  {
		cyclePeriod = u32;
	}
	retVal = coOdGetDefaultVal_u32(0x1007u, 0u, &u32);
	if (retVal == RET_OK)  {
		windowLength = u32;
	}
	retVal = coOdGetDefaultVal_u8(0x1019u, 0u, &u8);
	if (retVal == RET_OK)  {
		counterValue = u8;
	}
}


/***************************************************************************/
/**
* \brief coSyncInit - init sync functionality
*
* This function initializes the SYNC functionality.
*
* If the node is a sync producer or a sync consumer
* depends on the value of the object dictionary index 0x1005.
* <br>Sync counter value can also be set/reset by the value
* at the object dictionary at index 0x1019
*
* \return RET_T
*
*/
RET_T coSyncInit(
		UNSIGNED32	cobId				/**< sync cob-id */
	)
{
	/* if cyclePeriod != 0 than sync producer */
	if ((cobId & CO_SYNC_PRODUCER_BIT) != 0u)  {
		syncCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SYNC_TRANSMIT, 0u);
	} else {
		syncCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SYNC_RECEIVE, 0u);
	}
	if (syncCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	return(RET_OK);
}
#endif /* CO_SYNC_SUPPORTED */
