/*
* eb_battery.c - energybus battery
*
* Copyright (c) 2012-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_battery.c 8132 2015-01-13 07:11:33Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief battery specific functions
*
* \file eb_battery.c
* contains routines for energyBus battery handling
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#if defined(EB_BATTERY_CNT) || defined(EB_NETWORK_BATTERY_CNT) 

#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#include <co_nmt.h>


#include <ico_cobhandler.h>
#include <ico_queue.h>
#include <ico_nmt.h>

#include "eb_data.h"
#include "eb_fct.h"
#include "eb_battery.h"
#include "ieb_def.h"

/* constant definitions
---------------------------------------------------------------------------*/
#define EB_BAT_STATE_MAX_TRANS	12

const UNSIGNED16 batTransTable[EB_BAT_STATE_MAX_TRANS] = {
		0,
		EB_BAT_STATE_DO_NOT_ATTACH,		/* transition 1 */
		EB_BAT_STATE_READY_TO_ATTACH,	/* transition 2 */
		EB_BAT_STATE_DO_NOT_ATTACH,		/* transition 3 */
		EB_BAT_STATE_NORMAL_OPERATION,	/* transition 4 */
		EB_BAT_STATE_ERROR,				/* transition 5 */
		EB_BAT_STATE_PLEASE_DETACH,		/* transition 6 */
		EB_BAT_STATE_NORMAL_OPERATION,	/* transition 7 */
		EB_BAT_STATE_DO_NOT_ATTACH,		/* transition 8 */
		EB_BAT_STATE_ERROR,				/* transition 9 */
		EB_BAT_STATE_DO_NOT_ATTACH,		/* transition 10 */
		EB_BAT_STATE_ERROR				/* transition 11 */
};

/* MSB: old state || LSB: new state */
#define EB_BAT_STCHG_DONTATT_2_RDYTOATT	\
	((EB_BAT_STATE_DO_NOT_ATTACH << 3) | EB_BAT_STATE_READY_TO_ATTACH)
#define EB_BAT_STCHG_RDYTOATT_2_NORMAL	\
	((EB_BAT_STATE_READY_TO_ATTACH << 3) | EB_BAT_STATE_NORMAL_OPERATION)
#define EB_BAT_STCHG_NORMAL_2_DETACH	\
	((EB_BAT_STATE_NORMAL_OPERATION << 3) | EB_BAT_STATE_PLEASE_DETACH)
#define EB_BAT_STCHG_DETACH_2_NORMAL	\
	((EB_BAT_STATE_PLEASE_DETACH << 3) | EB_BAT_STATE_NORMAL_OPERATION)

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
#ifdef EB_NETWORK_BATTERY_CNT
#else /* EB_NETWORK_BATTERY_CNT */
EB_BATTERY_DATA_T       ebBatData[EB_BATTERY_CNT];
#endif /* EB_NETWORK_BATTERY_CNT */

/* local defined variables
---------------------------------------------------------------------------*/
static EB_EVENT_STATE_T   		batStateTable = NULL;
static EB_EVENT_CHECK_STATE_T   batCheckStateTable = NULL;


/***************************************************************************/
/**
* \internal
*
* \brief ebBatteryStateChangeReq - change Battery state machine
*
* \results
*	0 - ok
*	>0 - error
*/
EB_RET_T ebBatteryStateChange(
		UNSIGNED16	newState,				/* new state */
		UNSIGNED8	devIdx					/* device index */
	)
{
UNSIGNED32	stateChange;
UNSIGNED16	ebState;

	/* ignore same state */
	if (newState == (ebData[devIdx].state & EB_BAT_STATE_MASK))  {
		return(EB_RET_OK);
	}

	/* change to do NOT ATTACHED or ERROR are always allowed */
	if ((newState == EB_BAT_STATE_DO_NOT_ATTACH)
	 || (newState == EB_BAT_STATE_ERROR))  {
	} else {

		/* check for allowed state change */
		stateChange = (newState) | ((ebData[devIdx].state & EB_BAT_STATE_MASK) << 3);

		switch (stateChange)  {
			case EB_BAT_STCHG_DONTATT_2_RDYTOATT:
				break;
			case EB_BAT_STCHG_RDYTOATT_2_NORMAL:
			case EB_BAT_STCHG_NORMAL_2_DETACH:
				ebState = ebData[devIdx].state & EB_STATE_MASK;
				/* only allowed in OPERATING */
				if (ebState != EB_STATE_MASTERLESS)  {
					return(EB_RET_ERROR);
				}
				break;
			case EB_BAT_STCHG_DETACH_2_NORMAL:
				break;
			default:
				return(EB_RET_ERROR);
		}
	}

	/* call BAT user indication */
	if (batCheckStateTable != NULL)  {
		/* ask application for state change */
		if (batCheckStateTable(newState, devIdx + 1) != CO_TRUE)  {
			return(EB_RET_ERROR);
		}
	}

	if (batStateTable != NULL)  {
		batStateTable(newState, devIdx + 1);
	}

	ebData[devIdx].state &= ~EB_BAT_STATE_MASK;
	ebData[devIdx].state |= newState;

	/* write PDO immediately */
	coPdoReqNr(18 + EB_INSTANCE_NUMBER(ebData[devIdx].virtualDevice) - 1, 0);

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebBatteryStateTransition - execute status transition
*
* This functions requests a battery specific state transition.
*
* \return EB_RET_T
*
*/
EB_RET_T ebBatteryStateTransition(
		UNSIGNED16	ctrlWord,			/**< control word for new state */
		UNSIGNED8	devIdx				/**< device index */
	)
{
EB_RET_T	ebRet = EB_RET_OK;
UNSIGNED16	idx;

	/* execute only battery specific changes */
	if ((ctrlWord & EB_CTRL_DEV_SPEC_MASK) != 0)  {
		idx = ctrlWord >> 8;
		if (idx >= EB_BAT_STATE_MAX_TRANS)  {
			return(EB_RET_ERROR);
		}
		ebRet = ebBatteryStateChange(batTransTable[idx], devIdx);
		if (ebRet != EB_RET_OK)  {
			return(ebRet);
		}
	}
	return(ebRet);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \brief ebRegister_BAT_STATE - register Battery STATE function
*
* This function registers the given functions
* for battery specific state changes.
* <br>
* The check function can inhibit the state change by returning
* an error code.<br>
* The state function has to react on the state change.
*
* \return
*	EB_RET_T
*/
EB_RET_T ebRegister_BAT_STATE(
		EB_EVENT_CHECK_STATE_T pCheckFunction,	/**< function for check state change */
		EB_EVENT_STATE_T pFunction				/**< funciton for state change */
	)
{
	/* only one function are allowed */
	if (batStateTable != NULL)  {
		return(EB_RET_ERROR);
	}

	/* save function pointer */
	batStateTable = pFunction;
	batCheckStateTable = pCheckFunction;

	return(EB_RET_OK);
}

#endif /* EB_BATTERY_CNT */
