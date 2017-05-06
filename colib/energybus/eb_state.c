/*
* eb_state.c - energybus state machine
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_state.c 8884 2015-03-24 13:11:56Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief state handling for slaves
*
* \file eb_state.c
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_pdo.h>
#include <co_sleep.h>
#include <co_nmt.h>
#include <ico_indication.h>
#include <ico_cobhandler.h>
#include <ico_queue.h>
#include <ico_nmt.h>
#include <ico_nmt.h>
#include <ico_event.h>
#ifdef CO_LSS_SUPPORTED
# include <co_lss.h>
#endif /* CO_LSS_SUPPORTED */
#include "eb_data.h"
#include "eb_fct.h"
#include "ieb_def.h"
#ifdef EB_BATTERY_CNT
# include "eb_battery.h"
#endif /* EB_BATTERY_CNT */
#ifdef EB_VOLT_CONV_CNT
# include "eb_voltconv.h"
#endif /* EB_VOLT_CONV_CNT */
#ifdef EB_EBC_CNT
# include "eb_ebc.h"
#endif /* EB_EBC_CNT */
#if defined(EB_SECURITY_CNT) || defined(EB_NETWORK_SECURITY_CNT)
# include "eb_sec.h"
#endif

/* constant definitions
---------------------------------------------------------------------------*/
#define EB_STATE_MAX_TRANS	12u

/* wait time in sec, before enter sleep mode (no node id was received) */
#define EB_NO_NODE_ID_SLEEP_TIME	30ul

const UNSIGNED16 transTable[EB_STATE_MAX_TRANS] = {
		0u,
		EB_STATE_DISCONNECTED,	/* transition 1 */
		EB_STATE_CONNECTED,		/* transition 2 */
		EB_STATE_COMP_CHECK,	/* transition 3 */
		EB_STATE_OPERATING,		/* transition 4 */
		EB_STATE_LIMITING,		/* transition 5 */
		EB_STATE_OPERATING,		/* transition 6 */
		EB_STATE_CONNECTED,		/* transition 7 */
		EB_STATE_DISCONNECTED,	/* transition 8 */
		EB_STATE_DISCONNECTED,	/* transition 9 */
		EB_STATE_MASTERLESS,	/* transition 10 */
		EB_STATE_COMP_CHECK		/* transition 11 */
};

/*
* internal representation
* external the old state is in bit 13..15
*/

/* stateChange = (newState >> 8) | (ebData.state & EB_STATE_MASK) */
#define EB_STCHG_DISCON_2_CON	\
		(EB_STATE_DISCONNECTED | (EB_STATE_CONNECTED >> 8))
#define EB_STCHG_CON_2_COMPCHK	\
		(EB_STATE_CONNECTED | (EB_STATE_COMP_CHECK >> 8))
#define EB_STCHG_COMPCHK_2_OPER	\
		(EB_STATE_COMP_CHECK | (EB_STATE_OPERATING >> 8))
#define EB_STCHG_LIMIT_2_OPER	\
		(EB_STATE_LIMITING | (EB_STATE_OPERATING >> 8))
#define EB_STCHG_COMPCHK_2_LIMIT	\
		(EB_STATE_COMP_CHECK | (EB_STATE_LIMITING >> 8))
#define EB_STCHG_OPER_2_MSTRLESS	\
		(EB_STATE_OPERATING | (EB_STATE_MASTERLESS >> 8))
#define EB_STCHG_MSTRLESS_2_COMPCHK	\
		(EB_STATE_MASTERLESS | (EB_STATE_COMP_CHECK >> 8))
#define EB_STCHG_CON_2_SLEEP	\
		(EB_STATE_CONNECTED | (EB_STATE_SLEEP >> 8))

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void nextStateChange(void *pData);
#ifdef CO_LSS_SUPPORTED
static void lssGoSleep( void *ptr);
#endif /* CO_LSS_SUPPORTED */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static EB_EVENT_STATE_T   stateTable = NULL;
static EB_EVENT_CHECK_STATE_T   checkStateTable = NULL;
static CO_EVENT_T	nextStateEvent[EB_VIRTUAL_DEV_CNT];
#ifdef CO_LSS_SUPPORTED
static CO_TIMER_T	lssSleepTimer;
#endif /* CO_LSS_SUPPORTED */


/***************************************************************************/
/**
*
* \brief ebStateChange - change EB state machine
*
* Changes the energybus state machine to the given state.
* If the status change is not possible returns an error.
*
* \return
*	EB_RET_T
*/
EB_RET_T ebStateChange(
		UNSIGNED16	newState,	/* new state */
		UNSIGNED8	devIdx		/* device index */
	)
{
	/* ignore same state */
	if (newState == (ebData[devIdx].state & EB_STATE_MASK))  {
		return(EB_RET_OK);
	}

	if (iebStateCheckChange(newState, devIdx) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	ebData[devIdx].state &= ~EB_STATE_MASK;
	ebData[devIdx].state |= newState;

	/* start specific actions on new state */
	switch (newState)  {
		case EB_STATE_DISCONNECTED:	/* called at reset application */

#ifdef EB_BATTERY_CNT
			if ((ebData[devIdx].virtualDevice & EB_DEV_TYPE_MASK) == EB_DEV_TYPE_BATTERY)  {
				/* 8,10 */
				ebBatteryStateChange(EB_BAT_STATE_DO_NOT_ATTACH, devIdx);
			/* forgot node id */
			}
#endif /* EB_BATTERY_CNT */

#ifdef EB_DONT_FORGET_NODEID
#else /* EB_DONT_FORGET_NODEID */
# ifdef CO_LSS_SUPPORTED
			/* forgot node id */
			icoNmtResetNodeId();

			/* start timer for 10sec
			 * if no nodeid was received in time, go sleep mode */
			coTimerStart(&lssSleepTimer, EB_NO_NODE_ID_SLEEP_TIME * 1000000ul,
				lssGoSleep, NULL, CO_TIMER_ATTR_ROUNDUP);
# endif /* CO_LSS_SUPPORTED */
#endif /* EB_DONT_FORGET_NODEID */

			/* start timer to change to next state */
			icoEventStart(&nextStateEvent[devIdx], nextStateChange, &ebData[devIdx]);
			break;

		case EB_STATE_CONNECTED:	/* called at reset comm */
#ifdef EB_BATTERY_CNT
			if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_BATTERY)  {
				/* 8 */
				ebBatteryStateChange(EB_BAT_STATE_DO_NOT_ATTACH, devIdx);
			}
#endif /* EB_BATTERY_CNT */
#ifdef EB_VOLT_CONV_CNT
			if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_VOLT_CONV)  {
				/* 6 */
				ebVcStateTransition(EB_VC_CTRL_STOP_PROCESSING, devIdx);
			}
#endif /* EB_VOLT_CONV_CNT */

#ifdef CO_EVENT_SLEEP
			if (coSleepModeActive() == CO_FALSE)
#endif /* CO_EVENT_SLEEP */
			{
				/* start timer to change to next state */
				icoEventStart(&nextStateEvent[devIdx], nextStateChange,
						&ebData[devIdx]);
#ifdef EB_EBC_CNT
				ebSetEmsStatus(EBC_6080_SLEEP, 0);
#endif /* EB_EBC_CNT */
			}
			break;

		case EB_STATE_COMP_CHECK:
#if defined(EB_SECURITY_CNT) || defined(EB_NETWORK_SECURITY_CNT)
#  ifdef EB_EBC_CNT
#  else  /* EB_EBC_CNT */
			iebSecKeyInit(SEC_GET_KEY_CLASS(ebData[devIdx].secProperties));
#  endif   /* EB_EBC_CNT */
#endif 

			/* init PDO settings */
			iebUpdateStandardTPdos();

#ifdef EB_EBC_CNT
			ebSetEmsStatus(EBC_6080_CAN, 1);
#endif /* EB_EBC_CNT */
			break;

		case EB_STATE_LIMITING:
		case EB_STATE_OPERATING:
		case EB_STATE_MASTERLESS:
		default:
			break;
	}

	/* write PDO */
	coPdoReqObj(0x6002u, devIdx + 1u, 0u);

	EB_PRINT1("new state %x\n", newState);
	/* call EB user indication */
	if (stateTable != NULL)  {
		stateTable(newState, devIdx + 1);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief nextStateChange - call next state change
*
* start automatic status changes
*
* \return
*	void
*/
static void nextStateChange(
		void	*pData
	)
{
EB_DATA_T	*pEbData = (EB_DATA_T *)pData;
UNSIGNED8	devIdx;

	devIdx = (UNSIGNED8)(((char *)pEbData - (char *)&ebData[0]) / sizeof(EB_DATA_T));
	switch (pEbData->state & EB_STATE_MASK)  {
		case EB_STATE_DISCONNECTED:
			ebStateChange(EB_STATE_CONNECTED, devIdx);

			icoEventStart(&nextStateEvent[devIdx], nextStateChange, pEbData);
			break;

		case EB_STATE_CONNECTED:
			/* valid node id available ? */
			if (coNmtGetNodeId() != 255u)  {
				ebStateChange(EB_STATE_COMP_CHECK, devIdx);
			} else {
				icoEventStart(&nextStateEvent[devIdx], nextStateChange, pEbData);
			}
			break;

		default:
			break;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief iebStateChangeReq - echek change states
*
*
* \return
*	EB_RET_T
*/
EB_RET_T iebStateCheckChange(
		UNSIGNED16	newState,
		UNSIGNED8	devIdx	/* device index at ebData */
	)
{
UNSIGNED32	stateChange;

	/* ignore same state */
	if (newState == (ebData[devIdx].state & EB_STATE_MASK))  {
		return(EB_RET_OK);
	}

	/* change to disconnected or connected are always allowed */
	if ((newState == EB_STATE_DISCONNECTED)
	 || (newState == EB_STATE_CONNECTED))  {
	} else {

		/* check for allowed state change - code internal representation */
		stateChange = (newState >> 8) | (ebData[devIdx].state & EB_STATE_MASK);

		EB_PRINT1("stateChange %x", stateChange);
		switch (stateChange)  {
			case EB_STCHG_DISCON_2_CON:
				break;
			case EB_STCHG_CON_2_COMPCHK:
				/* only allowed if we have a valid node id */
				if (coNmtGetNodeId() == 255u)  {
					return(EB_RET_ERROR);
				}
				break;
			case EB_STCHG_COMPCHK_2_OPER:
				/* not allowed for active devices */
				if (EB_DEVICE_CLASS(ebData[devIdx].virtualDevice) != 0u)  {
					return(EB_RET_ERROR);
				}
				break;
			case EB_STCHG_LIMIT_2_OPER:
				break;
			case EB_STCHG_COMPCHK_2_LIMIT:
				break;
			case EB_STCHG_OPER_2_MSTRLESS:
				break;
			case EB_STCHG_MSTRLESS_2_COMPCHK:
				break;
			case EB_STCHG_CON_2_SLEEP:
				break;
			default:
				EB_PRINT(" error\n");
				return(EB_RET_ERROR);
		}
	}

	EB_PRINT("ok\n");
	/* call EB user indication */
	if (checkStateTable != NULL)  {
		if (checkStateTable(newState, devIdx + 1u) != CO_TRUE)  {
			return(EB_RET_ERROR);
		}
	}
	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebStateTransition - execute transition
*
* Change the energybus state machine by executing the given controlword.
* if the state change is not allowed returns an error.
*
* \return
*	EB_RET_T
*/
EB_RET_T ebStateTransition(
		UNSIGNED16	ctrlWord,			/**< control word */
		UNSIGNED8	devIdx				/**< device index */
	)
{
/* emergency shutdown 0x0080 */

EB_RET_T	ebRet = EB_RET_OK;
UNSIGNED16	idx;

	/* eb state machine ? */
	if ((ctrlWord & EB_CTRL_FSA_MASK) != 0u)  {
		idx = ctrlWord & EB_CTRL_FSA_MASK;
		if (idx >= EB_STATE_MAX_TRANS)  {
			return(EB_RET_ERROR);
		}

#ifdef CO_LSS_SUPPORTED
		if ((ctrlWord & EB_CTRL_FSA_MASK) == EB_STATE_TRANSITION_DISCONNECT)  {
			/* forgot node id */
			icoNmtResetNodeId();
		}
#endif /* CO_LSS_SUPPORTED */

		ebRet = ebStateChange(transTable[idx], devIdx);
		if (ebRet != EB_RET_OK)  {
			return(ebRet);
		}
	}

	/* device specific state change */
	if ((ctrlWord & EB_CTRL_DEV_SPEC_MASK) != 0u)  {
#ifdef EB_BATTERY_CNT
		if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_BATTERY)  {
			ebRet = ebBatteryStateTransition(ctrlWord, devIdx);
		}
#endif /* EB_BATTERY_CNT */
#ifdef EB_VOLT_CONV_CNT
		if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_VOLT_CONV)  {
			ebRet = ebVcStateTransition(ctrlWord, devIdx);
		}
#endif /* EB_VOLT_CONV_CNT */
	}

	return(ebRet);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebStateCheckTransition - check transition
*
* \return
*	EB_RET_T
*/
EB_RET_T iebStateCheckTransition(
		UNSIGNED16	ctrlWord,
		UNSIGNED8	devIdx
	)
{
EB_RET_T	ebRet = EB_RET_OK;
UNSIGNED16	idx;

	/* eb state machine ? */
	if ((ctrlWord & EB_CTRL_FSA_MASK) != 0u)  {
		idx = ctrlWord & EB_CTRL_FSA_MASK;
		if (idx >= EB_STATE_MAX_TRANS)  {
			/* later: check for sleep mode */
			return(EB_RET_ERROR);
		}
		ebRet = iebStateCheckChange(transTable[idx], devIdx);
		if (ebRet != EB_RET_OK)  {
			EB_PRINT1(" error %x\n", ebRet);
			return(ebRet);
		}
		EB_PRINT(" ok\n");
	}

	/* device specific state change */
#ifdef EB_BATTERY_CNT
#endif /* EB_BATTERY_CNT */

#ifdef EB_VOLT_CONV_CNT
	/* eb state machine ? */
	if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_VOLT_CONV)  {
		ebRet = iebVcCheckState(ctrlWord, devIdx);
	}
#endif /* EB_VOLT_CONV_CNT */

	return(ebRet);
}


#ifdef CO_LSS_SUPPORTED
static void lssGoSleep(
		void *ptr
	)
{
#ifdef CO_SLEEP_454
UNSIGNED8	ret = 0u;

	if (coNmtGetNodeId() == 255)  {
		/* no node id received, go into sleep mode */

#ifdef CO_EVENT_SLEEP
		/* check before user indication */
		ret = iebSleepInd(CO_SLEEP_MODE_CHECK, 0);
#endif /* CO_EVENT_SLEEP */

		if (ret == 0)  {
			coSleepModeStart(CO_SLEEP_WAITTIME);
		} else {
			/* start timer again */
			coTimerStart(&lssSleepTimer, EB_NO_NODE_ID_SLEEP_TIME * 1000000ul,
				lssGoSleep, NULL, CO_TIMER_ATTR_ROUNDUP);
		}
	}
#endif /* CO_SLEEP_454 */
}
#endif /* CO_LSS_SUPPORTED */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief ebStateSetIndication - s
*
*
* \return
*	EB_RET_T
*/
EB_RET_T iebStateSetIndication(
		EB_EVENT_CHECK_STATE_T	pCheckFunction,
		EB_EVENT_STATE_T		pFunction
	)
{
	/* only one function are allowed */
	if (stateTable != NULL)  {
		return(EB_RET_ERROR);
	}

	/* save function pointer */
	stateTable = pFunction;
	checkStateTable = pCheckFunction;

	return(EB_RET_OK);
}
