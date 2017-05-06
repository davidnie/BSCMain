/*
* eb_debug.c - debug functions for framework
*
* Copyright (c) 2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_ebc.c 5662 2014-04-16 14:35:11Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief EB debug help routines
*
* \file eb_debug.c
* contains routines for EB debugging
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#include <co_datatype.h>

#include "eb_data.h"
#include <co_commtask.h>
#include <co_nmt.h>
#if defined(EB_BATTERY_CNT) || defined(EB_NETWORK_BATTERY_CNT)
#include "eb_battery.h"
#endif /* EB_BATTERY_CNT */
#if defined(EB_VOLT_CONV_CNT) || defined(EB_NETWORK_VOLT_CONV_CNT)
#include "eb_voltconv.h"
#endif /* EB_VOLT_CONV_CNT */
#ifdef EB_EBC_CNT
# include "eb_ebc.h"
#endif /* EB_EBC_CNT */


/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
	UNSIGNED8	nr;
	char		strg[25];
} VAR_STRG_T;

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
* \brief ebDbgGetCanStateStrg - returns CAN state as formatted string
*
* This function returns the CAN state as formatted string.
*
* \return string
*
*/
const char *ebDbgGetCanStateStrg(
		UNSIGNED8	canState		/**< can state (CO_CAN_STATE...) */
	)
{
static const VAR_STRG_T	canStateStrg[] = {
	{ CO_CAN_STATE_BUS_OFF,		"Bus OFF" },
	{ CO_CAN_STATE_BUS_ON,		"Bus ON" },
	{ CO_CAN_STATE_PASSIVE,		"Error Passive" },
	{ CO_CAN_STATE_UNCHANGED,	"Unchanged"}
};
int i;

	for (i = 0; i < sizeof(canStateStrg) / sizeof(VAR_STRG_T); i++)  {
		if (canState == canStateStrg[i].nr)  {
			return(canStateStrg[i].strg);
		}
	}

	return("Unknown");
}


/***************************************************************************/
/**
* \brief ebDbgGetNmtStrg - returns NMT state as formatted string
*
* This function returns the NMT state as formatted string.
*
* \return string
*
*/
const char *ebDbgGetNmtStrg(
		UNSIGNED8	nmtState		/**< NMT state (CO_NMT_STATE...) */
	)
{
static const VAR_STRG_T	nmtStateStrg[] = {
	{	CO_NMT_STATE_OPERATIONAL,	"Operational" },
	{	CO_NMT_STATE_STOPPED,		"Stopped" },
	{	CO_NMT_STATE_PREOP,			"Pre-Operational" },
	{	CO_NMT_STATE_RESET_NODE,	"Reset Node" },
	{	CO_NMT_STATE_RESET_COMM,	"Reset Communication" },
};
int i;

	for (i = 0; i < sizeof(nmtStateStrg) / sizeof(VAR_STRG_T); i++)  {
		if (nmtState == nmtStateStrg[i].nr)  {
			return(nmtStateStrg[i].strg);
		}
	}

	return("Unknown");
}


/***************************************************************************/
/**
* \brief ebDbgGetCommStateStrg - returns Communication state as formatted string
*
* This function returns the Comminication state as formatted string.
*
* \return string
*
*/
const char *ebDbgGetCommStateStrg(
		UNSIGNED8	nmtState		/**< Comm state (CO_COMM_STATE...) */
	)
{
static const VAR_STRG_T	commStateStrg[] = {
	{ CO_COMM_STATE_EVENT_NONE,					"No Event" },
	{ CO_COMM_STATE_EVENT_CAN_OVERRUN,			"CAN overrung" },
	{ CO_COMM_STATE_EVENT_REC_QUEUE_FULL,		"Receive queue full" },
	{ CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW,	"Receive queue overflow" },
	{ CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY,		"Receive queue empty" },
	{ CO_COMM_STATE_EVENT_TR_QUEUE_FULL,		"Transmit queue full" },
	{ CO_COMM_STATE_EVENT_TR_QUEUE_OVERFLOW,	"Transmit queue overflow" },
	{ CO_COMM_STATE_EVENT_TR_QUEUE_EMPTY,		"Transmit queue empty" },
};
int i;

	for (i = 0; i < sizeof(commStateStrg) / sizeof(VAR_STRG_T); i++)  {
		if (nmtState == commStateStrg[i].nr)  {
			return(commStateStrg[i].strg);
		}
	}

	return("Unknown");
}


/***************************************************************************/
/**
* \brief ebDbgGetDeviceStrg - returns device type as formatted string
*
* This function returns the energyBus device name as formatted string.
*
* \return string
*
*/
const char *ebDbgGetDeviceStrg(
		UNSIGNED8	devType		/**< device type (EB_DEV_TYPE_xxx */
	)
{
static const VAR_STRG_T	devTypeStrg[] = {
	{ EB_DEV_TYPE_EBC,			"EBC" },
	{ EB_DEV_TYPE_VOLT_CONV,	"Voltage Converter" },
	{ EB_DEV_TYPE_BATTERY,		"Battery" },
	{ EB_DEV_TYPE_MCU,			"MCU" },
	{ EB_DEV_TYPE_HMI,			"HMI" },
	{ EB_DEV_TYPE_SECURITY,		"Security" },
	{ EB_DEV_TYPE_LOAD_MONITORING, "LMD" },
};
int i;

	for (i = 0; i < sizeof(devTypeStrg) / sizeof(VAR_STRG_T); i++)  {
		if (devType == devTypeStrg[i].nr)  {
			return(devTypeStrg[i].strg);
		}
	}

	return("Unknown");
}


/***************************************************************************/
/**
* \brief ebDbgGetStateStrg - returns state as formatted string
*
* This function returns the energyBus and device specific state
* as formatted string.
*
* \return string
*
*/
const char *ebDbgGetStateStrg(
		UNSIGNED16	state,		/**< energybus/device state */
		UNSIGNED8	devType		/**< device type (EB_DEV_TYPE_xxx */
	)
{
typedef struct {
	UNSIGNED16	state;
	char		strg[20];
} STATE_TAB_T;
static const STATE_TAB_T	stateTab[] = {
	{ EB_STATE_DISCONNECTED,	"Disconnect/Not set"	},
	{ EB_STATE_CONNECTED,		"Connected"	},
	{ EB_STATE_COMP_CHECK,		"Compatibily check"	},
	{ EB_STATE_LIMITING,		"Limiting"	},
	{ EB_STATE_OPERATING,		"Operating"	},
	{ EB_STATE_MASTERLESS,		"Masterless" },
	{ EB_STATE_SLEEP,			"Sleeping" },
};
#if defined(EB_BATTERY_CNT) || defined(EB_NETWORK_BATTERY_CNT)
static const STATE_TAB_T	batTab[] = {
	{ EB_BAT_STATE_DO_NOT_ATTACH,	"Do not attach" },
	{ EB_BAT_STATE_READY_TO_ATTACH, "Rdy to attach" },
	{ EB_BAT_STATE_ERROR,			"Error" },
	{ EB_BAT_STATE_PLEASE_DETACH,	"Please Detach" },
	{ EB_BAT_STATE_NORMAL_OPERATION, "Normal operation" },
};
#endif /* EB_BATTERY_CNT */
#if defined(EB_VOLT_CONV_CNT) || defined(EB_NETWORK_VOLT_CONV_CNT)
static const STATE_TAB_T	vcTab[] = {
	{ EB_VC_STATE_IDLE,				"Idle" },
	{ EB_VC_STATE_APPLY_PROCESSING,	"Apply Processing" },
	{ EB_VC_STATE_PROCESSING_IDLE,	"Processing idle" },
	{ EB_VC_STATE_STOP_PROCESSING,	"Stop Processing" },
	{ EB_VC_STATE_ERROR,		"Error" },
	{ EB_VC_STATE_UA_CONST,		"Ua const" },
	{ EB_VC_STATE_UB_CONST,		"Ub const" },
	{ EB_VC_STATE_IA_CONST,		"Ia const" },
	{ EB_VC_STATE_IB_CONST,		"Ib const" },
	{ EB_VC_STATE_NIA_CONST,	"-Ia const" },
	{ EB_VC_STATE_NIB_CONST,	"-Ib const" },
	{ EB_VC_STATE_PA_CONST,		"Pa const" },
	{ EB_VC_STATE_PB_CONST,		"Pb const" },
	{ EB_VC_STATE_NPA_CONST,	"-Pa const" },
	{ EB_VC_STATE_NPB_CONST,	"-Pb const" },
	{ EB_VC_STATE_MPPT,			"MPPT" },
	{ EB_VC_STATE_RA_CONST,		"Ra const" },
	{ EB_VC_STATE_RB_CONST,		"Rb const" },
};
#endif /* EB_VOLT_CONV_CNT */
static char stateStrg[50] = "";
int i;

	sprintf(stateStrg, "Unknown %x", state);
	for (i = 0; i < sizeof(stateTab) / sizeof(STATE_TAB_T); i++)  {
		if ((state & EB_STATE_MASK) == stateTab[i].state)  {
			strcpy(stateStrg, stateTab[i].strg);
		}
	}

#if defined(EB_BATTERY_CNT) || defined(EB_NETWORK_BATTERY_CNT)
	if (devType == EB_DEV_TYPE_BATTERY)  {
		for (i = 0; i < sizeof(batTab) / sizeof(STATE_TAB_T); i++)  {
			if ((state & EB_BAT_STATE_MASK) == batTab[i].state)  {
				strcat(stateStrg, " - ");
				strcat(stateStrg, batTab[i].strg);
			}
		}
	}
#endif /* EB_BATTERY_CNT */

#if defined(EB_VOLT_CONV_CNT) || defined(EB_NETWORK_VOLT_CONV_CNT)
	if (devType == EB_DEV_TYPE_VOLT_CONV)  {
		for (i = 0; i < sizeof(vcTab) / sizeof(STATE_TAB_T); i++)  {
			if ((state & EB_STATE_DEV_SPEC_MASK) == vcTab[i].state)  {
				strcat(stateStrg, " - ");
				strcat(stateStrg, vcTab[i].strg);
			}
		}
	}
#endif /* EB_VOLT_CONV_CNT */

	return(stateStrg);
}


#ifdef EB_EBC_CNT
/***************************************************************************/
/**
* \brief ebDbgGetEbcHandlingStrg - returns EBC handling state as formatted string
*
* This function returns the energyBus EBC hanlding state
* as formatted string.
*
* \return string
*
*/
const char *ebDbgGetEbcHandlingStrg(
		EBC_HANDLING_STATE_T	event		/**< ebc handling event */
	)
{
typedef struct {
	UNSIGNED8	nr;
	char		strg[50];
} DEV_TYPE_STRG_T;
static const DEV_TYPE_STRG_T	devTypeStrg[] = {
    { EBC_EVENT_STATE_WRITE_ERROR,		"Write error state change"},
    { EBC_EVENT_STATE_CHANGE_ERROR,		"State change error"},
    { EBC_EVENT_STATE_CHANGE_TOUT_ERROR, "Timeout status change"},
    { EBC_EVENT_STATE_CHANGE_SDO_TIMEOUT, "State change SDO timeout"},
    { EBC_EVENT_COMP_CHECK_WRITE_ERROR, "Go comp check write error"},
	{ EBC_EVENT_LIMITING_WRITE_ERROR,	"Go limiting write error"},
	{ EBC_EVENT_OPERATING_WRITE_ERROR,	"Go operating write error"},
	{ EBC_EVENT_OPERATING,				"OPERATING"},
	{ EBC_EVENT_NETWORK_OPERATING,		"NETWORK OPERATING"},
    { EBC_EVENT_STATUS_CHANGED,			"State changed ok"},
    { EBC_EVENT_AUTO_STATUS_CHANGED,	"State changed from device"},
	{ EBC_EVENT_DEVICE_FAILED,			"Device Failed"},
	{ EBC_EVENT_MODE_EBC,				"EBC Mode"},
	{ EBC_EVENT_MODE_CHARGER,			"Silent-EBC Mode"}
};
int i;
	
	for (i = 0; i < sizeof(devTypeStrg) / sizeof(DEV_TYPE_STRG_T); i++)  {
		if (event == devTypeStrg[i].nr)  {
			return(devTypeStrg[i].strg);
		}
	}

	return("Unknown");
}


/***************************************************************************/
/**
* \brief ebDbgGetEbcChargeStrg - returns EBC charge state as formatted string
*
* This function returns the energyBus EBC hanlding state
* as formatted string.
*
* \return string
*
*/
const char *ebDbgGetEbcChargeStrg(
		EBC_CHARGE_EVENT_STATE_T	event		/**< ebc charge event */
	)
{
typedef struct {
	UNSIGNED8	nr;
	char		strg[60];
} DEV_TYPE_STRG_T;
static const DEV_TYPE_STRG_T	chargeTypeStrg[] = {
	{ EBC_CHARGE_STATE_SDO_ABORT,	"SDO Abort - charging aborted" },
	{ EBC_CHARGE_STATE_COPY_OK,		"all data are copied - go to limiting and write limit values" },
	{ EBC_CHARGE_STATE_CHARGING,	"charging started" },
	{ EBC_CHARGE_STATE_PREPARE_ERROR, "prepare error - charging aborted" },
	{ EBC_CHARGE_STATE_LIMITING, "state limiting reached - start charging now"},
	{ EBC_CHARGE_STATE_START_ERROR,	"error start charing - abort" },
	{ EBC_CHARGE_STATE_CHARGE_ERROR,"charging error - abort" },
	{ EBC_CHARGE_STATE_STOPPED,		"charging finished" },
	{ EBC_CHARGE_STATE_NEW_BAT_CMD,	"new battery command" },
};
int i;
	
	for (i = 0; i < sizeof(chargeTypeStrg) / sizeof(DEV_TYPE_STRG_T); i++)  {
		if (event == chargeTypeStrg[i].nr)  {
			return(chargeTypeStrg[i].strg);
		}
	}

	return("Unknown");
}
#endif /* EB_EBC_CNT */
