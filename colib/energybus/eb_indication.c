/*
* eb_indication.c - energybus indication handling
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_indication.c 15306 2016-09-22 15:30:40Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief Indication functions
* This file contains indication functions.
*
* \file eb_indication.c
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_nmt.h>
#include <co_sdo.h>
#include <co_pdo.h>
#include <co_sync.h>
#include <co_commtask.h>
#include <co_lss.h>
#include <co_sleep.h>
#include <ico_indication.h>
#include <eb_data.h>
#include <eb_fct.h>
#include <eb_eeprom.h>
#include "ieb_def.h"

#ifdef EB_EBC_CNT
#include "eb_ebc.h"
#include "ieb_ebc.h"
#endif /* EB_EBC_CNT */

#ifdef EB_NETWORK_SECURITY_CNT
#  include <eb_sec.h>
#endif

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
static RET_T ebNmtInd(BOOL_T execute, CO_NMT_STATE_T newState);
static void ebErrCtrlInd(UNSIGNED8 node, CO_ERRCTRL_T event, CO_NMT_STATE_T	nmtState);
#ifdef CO_EVENT_PROFILE_SYNC
static void ebSyncInd(UNSIGNED8	syncCnt);
#endif /* CO_EVENT_PROFILE_SYNC */
#ifdef CO_EVENT_PROFILE_SYNC_FINISHED
static void ebSyncFinishedInd(UNSIGNED8	syncCnt);
#endif /* CO_EVENT_PROFILE_SYNC_FINISHED */
static RET_T ebSdoWriteInd(BOOL_T, UNSIGNED8, UNSIGNED16, UNSIGNED8);
#ifdef CO_PDO_RECEIVE_CNT
static void ebPdoInd(UNSIGNED16);
#endif /* CO_PDO_RECEIVE_CNT */
static RET_T ebSdoReadInd(BOOL_T, UNSIGNED8, UNSIGNED16, UNSIGNED8);
#ifdef EB_NETWORK_SECURITY_CNT
static RET_T ebSdoCheckWriteInd(BOOL_T, UNSIGNED8, UNSIGNED16, UNSIGNED8, const UNSIGNED8 *);
#endif /* EB_NETWORK_SECURITY_CNT */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static CO_EVENT_STORE_T pApplLoadFunction = NULL;
static EB_EVENT_NMT_T	nmtTable = NULL;
static EB_EVENT_EBCMON_T   ebcMonTable = NULL;
#ifdef CO_EVENT_PROFILE_SYNC
static EB_EVENT_SYNC_T	syncTable = NULL;
#endif /* CO_EVENT_PROFILE_SYNC */
#ifdef CO_EVENT_PROFILE_SYNC_FINISHED
static EB_EVENT_SYNC_T	syncFinishedTable = NULL;
#endif /* CO_EVENT_SYNC_PROFILE_FINISHED */
static EB_SDO_WRITE_T	sdoWriteTable = NULL;
static EB_SDO_READ_T	sdoReadTable = NULL;
static EB_PDO_T			pdoTable = NULL;
#ifdef CO_EVENT_SLEEP
static EB_SLEEP_T		sleepTable = NULL;
#endif /* CO_EVENT_SLEEP */


/***************************************************************************/
/**
* \internal
*
* \brief ebIndInit - init indication functions
*
* register indication at CANopen stack
*
* \return
*	EB_RET_T
*/
EB_RET_T iebIndInit(
		void
	)
{
#ifdef EB_EBC_CNT
UNSIGNED8	i;
#endif /* EB_EBC_CNT */

	if (coEventRegister_NMT(ebNmtInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (coEventRegister_ERRCTRL(ebErrCtrlInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}

#ifdef EB_NETWORK_SECURITY_CNT
	if (coEventRegister_SDO_SERVER_CHECK_WRITE(ebSdoCheckWriteInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif

	if (coEventRegister_SDO_SERVER_WRITE(ebSdoWriteInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (coEventRegister_SDO_SERVER_READ(ebSdoReadInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}

#ifdef EB_EBC_CNT
	/* state changes for external devices */
	for (i = 0; i < (EB_NETWORK_VIRTUAL_DEV_CNT - EB_VIRTUAL_DEV_CNT); i++) {
		/* state changes */
		if (coEventRegister_OBJECT_CHANGED(iebcObjectChanged, 0x6002, 128 + i)
				!= RET_OK)  {
			return(EB_RET_ERROR);
		}
	}

# ifdef EB_NETWORK_SECURITY_CNT
	for (i = 0; i < EB_NETWORK_SECURITY_CNT; i++) {
		/* security changes */
		if (coEventRegister_OBJECT_CHANGED(iebcObjectChanged, 0x6311,
				EB_NETWORK_SECURITY_SUBIDX_OFFS + i) != RET_OK)  {
			return(EB_RET_ERROR);
		}
	}
# endif /* EB_NETWORK_SECURITY_CNT */
#endif /* EB_EBC_CNT */

#ifdef CO_PDO_RECEIVE_CNT
	if (coEventRegister_PDO(ebPdoInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_PDO_RECEIVE_CNT */

#ifdef CO_EVENT_PROFILE_SYNC
	if (coEventRegister_SYNC(ebSyncInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_EVENT_PROFILE_SYNC */

#ifdef CO_EVENT_PROFILE_SYNC_FINISHED
	if (coEventRegister_SYNC_FINISHED(ebSyncFinishedInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_EVENT_SYNC_PROFILE_FINISHED */

#ifdef CO_SDO_CLIENT_CNT
	if (coEventRegister_SDO_CLIENT_READ(iebSdoClientReadInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
	if (coEventRegister_SDO_CLIENT_WRITE(iebSdoClientWriteInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_SDO_CLIENT_CNT */

#ifdef CO_LSS_MASTER_SUPPORTED
	if (coEventRegister_LSS_MASTER(iebLssMasterInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_LSS_MASTER_SUPPORTED */

#ifdef CO_EVENT_SLEEP
	if (coEventRegister_SLEEP(iebSleepInd) != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* CO_EVENT_SLEEP */
 
	return(EB_RET_OK);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


/***************************************************************************/
/**
* \brief ebRegister_NMT - register NMT function
*
* With this function an indication function for NMT events
* can be registered.
*
* \return
*	EB_RET_T
*/
EB_RET_T ebRegister_NMT(
		EB_EVENT_NMT_T pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (nmtTable != NULL)  {
		return(EB_RET_ERROR);
	}

	nmtTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_EBCMON - register EBCMON function
*
* With this function an indication function for EBC events
* (EBC available/failed)
* can be registered.
* This function is only valid for EnergyBus Slave Devices.
* 
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_EBCMON(
		EB_EVENT_EBCMON_T	pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (ebcMonTable != NULL)  {
		return(EB_RET_ERROR);
	}

	ebcMonTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_ERRCTRL - register ERRCTRL function
*
* With this function an indication function for NMT error control events
* for EBC can be registered.
*
* Only for EBC!!! - for slave devices, use ebRegister_EBCMON() instead.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_ERRCTRL(
		EB_EVENT_EBCMON_T	pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (ebcMonTable != NULL)  {
		return(EB_RET_ERROR);
	}

	ebcMonTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}

#ifdef CO_EVENT_PROFILE_SYNC
/***************************************************************************/
/**
* \brief ebRegister_SYNC - register SYNC function
*
* With this function an indication function for SYNC events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_SYNC(
		EB_EVENT_SYNC_T	pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (syncTable != NULL)  {
		return(EB_RET_ERROR);
	}

	syncTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}
#endif /* CO_EVENT_PROFILE_SYNC */

#ifdef CO_EVENT_PROFILE_SYNC_FINISHED
/***************************************************************************/
/**
* \brief ebRegister_SYNC_FINISHED - register SYNC_FINISHED function
*
* With this function an indication function for SYNC finish events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_SYNC_FINISHED(
		EB_EVENT_SYNC_T	pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (syncFinishedTable != NULL)  {
		return(EB_RET_ERROR);
	}

	syncFinishedTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}
#endif /* CO_EVENT_PROFILE_SYNC_FINISHED */

/***************************************************************************/
/**
* \brief ebRegister_STATE - register EB STATE function
*
* With this function an indication function for 
* EnergyBus State check and State change events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_STATE(
		EB_EVENT_CHECK_STATE_T	pCheckFunction,	/**< pointer to check function*/
		EB_EVENT_STATE_T		pFunction		/**< pointer to state function*/
	)
{
	if (iebStateSetIndication(pCheckFunction, pFunction) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_SDO_WRITE - register EB sdo write function
*
* With this function an indication function for SDO Write events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_SDO_WRITE(
		EB_SDO_WRITE_T pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (sdoWriteTable != NULL)  {
		return(EB_RET_ERROR);
	}

	sdoWriteTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_SDO_READ - register EB sdo read function
*
* With this function an indication function for SDO Read events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_SDO_READ(
		EB_SDO_READ_T pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (sdoReadTable != NULL)  {
		return(EB_RET_ERROR);
	}

	sdoReadTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_PDO_WRITE - register EB pdo function
*
* With this function an indication function for PDO events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_PDO(
		EB_PDO_T pFunction		/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (pdoTable != NULL)  {
		return(EB_RET_ERROR);
	}

	pdoTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_COMM_EVENT - register communication events
*
* With this function an indication function for communication events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_COMM_EVENT(
		EB_COMM_EVENT_T	pFunction	/**< pointer to indication function */
	)
{
RET_T	retVal;

	retVal = coEventRegister_COMM_EVENT(pFunction);
	if (retVal != RET_OK) {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebRegister_CAN_STATE - register CAN state events
*
* With this function an indication function for CAN status changes
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_CAN_STATE(
		EB_CAN_STATE_T	pFunction	/**< pointer to indication function */
	)
{
RET_T	retVal;

	retVal = coEventRegister_CAN_STATE(pFunction);
	if (retVal != RET_OK) {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


#ifdef CO_EVENT_SLEEP
/***************************************************************************/
/**
* \brief ebRegister_SLEEP - register sleep indication
*
* With this function an indication function for SLEEP events
* can be registered.
*
* \returns
*	EB_RET_T
*/
EB_RET_T ebRegister_SLEEP(
		EB_SLEEP_T	pFunction	/**< pointer to indication function */
	)
{
	/* only one function are allowed */
	if (sleepTable != NULL)  {
		return(EB_RET_ERROR);
	}

	sleepTable = pFunction;      /* save function pointer */

	return(EB_RET_OK);
}
#endif /* CO_EVENT_SLEEP */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief ebNmtInd - NMT indikation
*
*
* \return
*	RET_T
*/
static RET_T ebNmtInd(
		BOOL_T  execute,			/* execute functionality */
		CO_NMT_STATE_T  newState	/* new NMT state */
	)
{
UNSIGNED8	i;

	/* nothing to test */
	if (execute == CO_FALSE)  {
		return(RET_OK);
	}

#ifdef EB_EBC_CNT
	ebcNodeList[0].nmtState = newState;
#endif /* EB_EBC_CNT */

	/* depending on new state call EnergyBus State changes */
	switch (newState)  {
		case CO_NMT_STATE_RESET_NODE:
			/* go to state connected */
			for (i = 0u; i < EB_VIRTUAL_DEV_CNT; i++)  {
				ebStateChange(EB_STATE_DISCONNECTED, i);
			}
			break;

		case CO_NMT_STATE_RESET_COMM:
			for (i = 0u; i < EB_VIRTUAL_DEV_CNT; i++)  {
				/* go to state connected */
				ebStateChange(EB_STATE_CONNECTED, i);

#ifdef EB_EBC_CNT
				iebcUpdateHbConsumer();
#endif /* EB_EBC_CNT */

				/* update standard TPDOs */
				iebUpdateStandardTPdos();
			}
			break;

		case CO_NMT_STATE_UNKNOWN:
		case CO_NMT_STATE_OPERATIONAL:
		case CO_NMT_STATE_PREOP:
		case CO_NMT_STATE_STOPPED:
		default:
			break;
	}

	/* call registered EB user indication */
	if (nmtTable != NULL)  {
		nmtTable((UNSIGNED8)newState);
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief ebErrCtlrInd - error control indication
*
*
* \returns
*	nothing
*/
static void ebErrCtrlInd(
		UNSIGNED8		node,		/* node id */
		CO_ERRCTRL_T	event,		/* nmt event */
		CO_NMT_STATE_T	nmtState	/* actual nmt state */
	)
{
#ifdef EB_EBC_CNT
	EB_PRINT1("hb event node %d ", node);

	switch (event)  {
		case CO_ERRCTRL_BOOTUP:
			EB_PRINT("bootup");
			/* node >= 120 are ignored */
			if (node < FIXED_NODE_ID)  {
				/* node add to node list */
				iebcNodeDetected(node);
			}
			break;
		case CO_ERRCTRL_HB_STARTED:
			EB_PRINT("startet");
			/* node >= 120 as detect event */
			if (node >= FIXED_NODE_ID)  {
				iebcNodeDetected(node);
			}

			iebcNodeState(node, event, nmtState);
			break;
		case CO_ERRCTRL_NEW_STATE:
		case CO_ERRCTRL_HB_FAILED:
			EB_PRINT("new state/failed");
			iebcNodeState(node, event, nmtState);
			break;
		case CO_ERRCTRL_GUARD_FAILED:
		default:
			break;
	}
	EB_PRINT("\n");

	/* call user indication */
	if (ebcMonTable != NULL)  {
		ebcMonTable(node, (UNSIGNED8)nmtState);
	}
#else /* EB_EBC_CNT */
UNSIGNED8	state;
UNSIGNED8	ebc;
UNSIGNED8	i;

	/* only indication for EBC (1) and Charger (127) are allowed */
	if ((node != 1u) && (node != CHARGER_EBC_NODE_ID))  {
		return;
	}

	EB_PRINT2("ebErrctrlInd: node %d event %d\n", node, event);

	/* signal only HB failed and heartbeat started */
	if ((event != CO_ERRCTRL_HB_FAILED) && (event != CO_ERRCTRL_HB_STARTED)) {
		return;
	}

	/* call registered user indication */
	if (ebcMonTable != NULL)  {
		if (node == 1u)  {
			ebc = 1u;
		} else {
			ebc = 0u;
		}
		if (event == CO_ERRCTRL_HB_FAILED)  {
			state = 0u;
		} else {
			state = 1u;
		}
		ebcMonTable(ebc, state);
	}

	/* state change if EBC was lost */
	if ((node == 1u) && (event == CO_ERRCTRL_HB_FAILED))  {
		for (i = 0u; i < EB_VIRTUAL_DEV_CNT; i++)  {
			/* devices should go to disconneted */
			if (EB_DEVICE_CLASS(ebData[i].virtualDevice) != 0u)  {
				/* go to state disconnected */
				ebStateChange(EB_STATE_DISCONNECTED, i);
			} else {
				/* go to state connected */
				ebStateChange(EB_STATE_CONNECTED, i);
			}
		}
	}

# ifdef EB_BATTERY_CNT
	/* go to state disconnected if charger has failed and EBC is not avail */
	if ((node == CHARGER_EBC_NODE_ID) && (event == CO_ERRCTRL_HB_FAILED)
	 && (coNmtGetRemoteNodeState(1) == CO_NMT_STATE_UNKNOWN))  {
		for (i = 0u; i < EB_VIRTUAL_DEV_CNT; i++)  {
			if ((ebData[i].virtualDevice & EB_DEV_TYPE_MASK) == EB_DEV_TYPE_BATTERY)  {
				ebStateChange(EB_STATE_DISCONNECTED, i);
			}
		}
	}
# endif /* EB_BATTERY_CNT */

#endif /* EB_EBC_CNT */
}

#ifdef CO_EVENT_PROFILE_SYNC
/***************************************************************************/
/**
* \brief ebSyncInd - sync indication 
*
* \internal
*
* \returns
*	nothing
*/
static void ebSyncInd(
		UNSIGNED8	syncCnt
	)
{
	/* call registered user indication */
	if (syncTable != NULL)  {
		syncTable();
	}
}
#endif /* CO_EVENT_PROFILE_SYNC */

#ifdef CO_EVENT_PROFILE_SYNC_FINISHED
/***************************************************************************/
/**
* \brief ebSyncFinishedInd - sync finished indication
*
* \internal
*
* \returns
*	nothing
*/
static void ebSyncFinishedInd(
		UNSIGNED8	syncCnt
	)
{
	/* call registered user indication */
	if (syncFinishedTable != NULL)  {
		syncFinishedTable();
	}
}
#endif /* CO_EVENT_PROFILE_SYNC_FINISHED */


/***************************************************************************/
/**
* \brief ebSdoReadInd - sdo read indication
*
* \internal
*
* \returns
*	nothing
*/
static RET_T ebSdoReadInd(
		BOOL_T		execFlag,			/* execute y/n */
		UNSIGNED8	sdoNr,				/* SDO number */
		UNSIGNED16	index,				/* index */
		UNSIGNED8	subIndex			/* subindex */
	)
{
RET_T	retVal = RET_OK;

#ifdef EB_NETWORK_SECURITY_CNT
	/* security device */
	switch (index) {
		case 0x6315u: /* random text - Device */
		case 0x6316u: /* random text - EBC */
		case 0x6321u: /* security device state - encrypted */
			retVal = iebSecSdoReadInd( execFlag, sdoNr, index, subIndex);
			break;
		default:
			break;
	}
#endif

#ifdef EB_EBC_CNT
	/* some datatypes differ from OD and real variables
	 * provide the correct data here */
	/* nmt state ? */
	if ((index & 0xff80u) == 0x5000u) {
		switch (subIndex)  {
			case 6u:			/* NMT state */
				coOdPutObj_u8(index, subIndex,
					ebcNodeList[(index & 0x7fu) - 1u].nmtState);
				break;
			case 7u:			/* valid */
			//	coOdPutObj_u8(index, subIndex,
//					ebcNodeList[(index & 0x7fu) - 1u].valid);
				break;
			default:
				break;
		}
	}
	if ((index & 0xff80u) == 0x5100u) {
		switch (subIndex)  {
			case 1u:			/* dev info state */
				coOdPutObj_u8(index, subIndex,
						ebcDevList[(index & 0x7fu) - 1u].devInfoState);
				break;
			case 4u:			/* dev info writable */
				coOdPutObj_u8(index, subIndex,
						ebcDevList[(index & 0x7fu) - 1u].ginWritable);
				break;
			case 5u:			/* compatible */
				coOdPutObj_u8(index, subIndex,
						ebcDevList[(index & 0x7fu) - 1u].compatible);
				break;
			case 8u:			/* valid */
//				coOdPutObj_u8(index, subIndex,
//						ebcDevList[(index & 0x7fu) - 1u].valid);
				break;
			default:
				break;
		}
	}
#endif /* EB_EBC_CNT */

	/* call registered user function */
	if (execFlag == CO_TRUE)  {
		if (sdoReadTable != NULL)  {
			sdoReadTable(index, subIndex);
		}
	}

	EB_PRINT2("sdoRd %x:%d\n", index, subIndex);
	return retVal;
}


#ifdef EB_NETWORK_SECURITY_CNT
/***************************************************************************/
/**
* \brief ebSdoCheckWriteInd - check SDO write indication
*
* \internal
*
* \returns
*	nothing
*/
static RET_T ebSdoCheckWriteInd(
		BOOL_T		execFlag,			/* execute y/n */
		UNSIGNED8	sdoNr,				/* SDO number */
		UNSIGNED16	index,				/* index */
		UNSIGNED8	subIndex,			/* subindex */
		const UNSIGNED8 * pData				/* pointer to CAN data */
	)
{
RET_T	retVal = RET_OK;
(void)pData;

	/* security device */
	switch(index) {
	case 0x6313: /* Security Key - Device */
	case 0x6314: /* Security Key - EBC */
	case 0x6315: /* random text - Device */
	case 0x6316: /* random text - EBC */
	case 0x6320: /* new command - encrypted */
		retVal = iebSecSdoCheckWriteInd( execFlag, sdoNr, index, subIndex);
		break;
	default:
		break;
	}

	return retVal;

}
#endif


/***************************************************************************/
/**
* \brief ebSdoWriteInd - SDO write indication
*
* \internal
*
* \return
*	nothing
*/
static RET_T ebSdoWriteInd(
		BOOL_T		execFlag,		/* execute y/n */
		UNSIGNED8	sdoNr,			/* SDO number */
		UNSIGNED16	index,			/* index */
		UNSIGNED8	subIndex		/* subindex */
	)
{
RET_T	retVal = RET_OK;
UNSIGNED8	devIdx = 0u;

	/* write to external device should not be allowed */
	if (index >= 0x6000)  {
		devIdx = iebGetDevIdxFromSubIndex(subIndex);
		if (devIdx == 255)  {
			return(RET_INVALID_PARAMETER);
		}
	}

#ifdef EB_NETWORK_SECURITY_CNT
	/* security device */
	switch(index) {
		case 0x6313u: /* Security Key - Device */
		case 0x6314u: /* Security Key - EBC */
		case 0x6315u: /* random text - Device */
		case 0x6316u: /* random text - EBC */
		case 0x6317u: /* security keys */
		case 0x6320u: /* new command - encrypted */
			retVal = iebSecSdoWriteInd(execFlag, sdoNr, index, subIndex);
			break;
		default:
			break;
	}

	if (retVal != RET_OK) {
		/* do not call customer indications */
		return (retVal);
	}
#endif

	/* control word ? */
	if (index == 0x6001u)  {
		if (execFlag == CO_FALSE)  {
			/* check for valid state change */
			if (iebStateCheckTransition(ebData[devIdx].control, devIdx) != EB_RET_OK)  {
				return(RET_SDO_INVALID_VALUE);
			}
			return(RET_OK);
		} else {
			/* execute eb state command */
			ebStateTransition(ebData[devIdx].control, devIdx);
		}
	}

	/* instance offset new */
	if (index == 0x6000u) {
		retVal = iebUpdateDeviceTPdos(subIndex - 1);
	}

#ifdef EB_VOLT_CONV_CNT
	/* node-id of battery for charger */
	if (index == 0x60f1u) {
		if (execFlag == CO_FALSE)  {
			retVal = iebVcSetBatteryNodeId(index, subIndex);
		}
	}
#endif /*  EB_VOLT_CONV_CNT */

	/* data transfers */
	if (index == DATA_TRANSFER_INDEX)  {
		iebResponseTransferData(execFlag);
	}

	/* call registered user function */
	if (execFlag == CO_TRUE)  {
		if (sdoWriteTable != NULL)  {
			sdoWriteTable(index, subIndex);
		}
	}

	return(retVal);
}


#ifdef CO_PDO_RECEIVE_CNT
/***************************************************************************/
/**
* \brief ebPdoInd - PDO indication
*
* \internal
*
* \returns
*	nothing
*/
static void ebPdoInd(
		UNSIGNED16	pdoNr			/* PDO indication */
	)
{
#ifdef EB_EBC_CNT
	/* EBC receives all PDOs */
	iebcPdoReceived(pdoNr);
#endif /* EB_EBC_CNT */

	if (pdoTable != NULL)  {
		pdoTable(pdoNr);
	}
}
#endif /* CO_PDO_RECEIVE_CNT */


/***************************************************************************/
/**
* \internal
*
* \brief ebLoadInd - load indication
*
*
* \return
*	RET_T
*/
void iebLoadIndInit(
		CO_EVENT_STORE_T pLoadFunction	/**< load function called at reset comm */
	)
{
	pApplLoadFunction = pLoadFunction;
}


/***************************************************************************/
/**
* \internal
*
* \brief ebLoadInd - load indication
*
*
* \return
*	RET_T
*/
RET_T iebLoadInd(
		UNSIGNED8 subIndex			/* subindex 1010/1011 */
	)
{
RET_T	retVal = RET_OK;

	/* update standard TPDOs
	 * - required after resetting the communication
	 * - required after resetting of the device profile part
	 */
	iebUpdateStandardTPdos();

	/* call application specific load function */
	if (pApplLoadFunction != NULL)  {
		pApplLoadFunction(subIndex);
	}

	return (retVal);
}


#ifdef CO_EVENT_SLEEP
/***************************************************************************/
/**
* \brief iebSleepInd - sleep indication
*
* \internal
*
* retval 0 - sleep ok
* retval != 0 - sleep not possible
*/
UNSIGNED8 iebSleepInd(
		CO_SLEEP_MODE_T	mode,			/* actual sleep mode */
		UNSIGNED8		nodeId
	)
{
UNSIGNED8	i;
UNSIGNED8	ret = 0u;

	EB_PRINT("iebSleepInd: ");

	switch (mode)  {
		case CO_SLEEP_MODE_CHECK:
#ifdef EB_EBC_CNT
			ebSetEmsStatus(EBC_6080_SLEEP, 1);
#endif /* EB_EBC_CNT */
			EB_PRINT("check sleep mode: ok\n");
			break;
		case CO_SLEEP_MODE_OBJECTION:
			EB_PRINT("slave objection\n");
			break;
		case CO_SLEEP_MODE_PREPARE:
#ifdef EB_EBC_CNT
			ebSetEmsStatus(EBC_6080_SLEEP, 1);
#endif /* EB_EBC_CNT */
			EB_PRINT("prepare sleep mode\n");
			break;
		case CO_SLEEP_MODE_SILENT:
#ifdef EB_EBC_CNT
			ebSetEmsStatus(EBC_6080_SLEEP, 1);
#endif /* EB_EBC_CNT */
			EB_PRINT("start silent mode\n");
			break;
		case CO_SLEEP_MODE_DOZE:
#ifdef EB_EBC_CNT
			ebSetEmsStatus(EBC_6080_SLEEP, 1);
#endif /* EB_EBC_CNT */
			EB_PRINT("doze\n");
			/* go to connected */
			for (i = 0u; i < EB_VIRTUAL_DEV_CNT; i++)  {
				ebStateChange(EB_STATE_CONNECTED, i);
				ebStateChange(EB_STATE_SLEEP, i);
			}
			break;
		default:
			EB_PRINT("unknown\n");
	}

	/* call registered user indication */
	if (sleepTable != NULL)  {
		ret = sleepTable(mode);
		/* return if error */
		if (ret != 0u)  {
			return(0xFEu);
		}
	}

	return(ret);
}


/***************************************************************************/
/**
* \brief ebSleepAwake - awake from sleep
*
* This function have to be called after the device is awaked
* to reinitialize the communication settings.
*
*
*/
void ebSleepAwake(
		void
	)
{
UNSIGNED8	i;

	/* wakeup command */
#ifdef EB_EBC_CNT
	coSleepAwake(CO_TRUE, 0x82, 0xfe, 250);

	/* restore old network state for EBC */
	iebcRestoreSleepState();
#else /* EB_EBC_CNT */
	coSleepAwake(CO_FALSE, 0x81, 0xfe, 250);
#endif /* EB_EBC_CNT */

	/* goto disconnected for non configured LSS slaves */
	for (i = 0; i < EB_VIRTUAL_DEV_CNT; i++)  {
		if (coNmtGetNodeId() == 255)  {
			ebStateChange(EB_STATE_DISCONNECTED, i);
		}
	}
}
#endif /* CO_EVENT_SLEEP */
