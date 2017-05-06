/*
* battery.c - contains battery functions
*
* Copyright (c) 2012-2014 emtas GmbH
*-------------------------------------------------------------------
* $Id: battery.c 1415 2012-08-10 16:02:18Z  $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \file
* \brief energybus battery functions
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include "gen_define.h"
#include <co_canopen.h>
#include <eb_energybus.h>

#include "battery.h"

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
--------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void batVariableInit(UNSIGNED8 vdnNr);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/

/***************************************************************************/
/**
* \brief batVariableInit - init energybus variables
*
* Can be also set by the DeviceWizard
*
* \param
*	nothing
* \results
*	nothing
*/
#if 0
static void batVariableInit( 
		void
	)
{
}
#endif
static void batVariableInit( 
		UNSIGNED8 vdnNr
)
{
//printf("Variable init Battery\n");

UNSIGNED8 vdnIdx = vdnNr - 1;
UNSIGNED8 batIdx = 0;

	/* generic energybus data */
	ebData[vdnIdx].virtualDevice = 0x01000106ul;
	ebData[vdnIdx].control = 0;
	ebData[vdnIdx].state = 0; /* State machine interaction required ?? */
	ebData[vdnIdx].capability = 0x1A; /* ?? */
	ebData[vdnIdx].nominalVoltage = 48000l; /* ??? */
	ebData[vdnIdx].reqDynVoltageLimitation = 10000u;
	ebData[vdnIdx].reqDynCurrInpLimitation = 10000u;
	ebData[vdnIdx].reqDynCurrOutLimitation = 10000u;
	ebData[vdnIdx].maxContinousInpCurrent = 0; /* ??? */
	ebData[vdnIdx].maxContinousOutCurrent = 0; /* ??? */
	ebData[vdnIdx].maxVoltage = 50000l; /* ??? */
	ebData[vdnIdx].minVoltage = 20000l; /* ??? */
	ebData[vdnIdx].actualCurrent = 0;
	ebData[vdnIdx].actualVoltage = 0;
	ebData[vdnIdx].electronicTemperature = 0; /* ??? */

	/* battery specific data */
	ebBatData[batIdx].type = 0; /* ?? */
	ebBatData[batIdx].packRatedWhCapacity = 0; /* ?? */
	ebBatData[batIdx].temperature[0] = 0; /* ?? */
	ebBatData[batIdx].packMaximumCellVoltage = 50000l; /* ?? */
	ebBatData[batIdx].packMinimumCellVoltage = 20000l; /* ??? */
	ebBatData[batIdx].packActualWhCapacity = 0; /* ??? */
	ebBatData[batIdx].fullOutputWhCapacity = 0; /* ??? */
	ebBatData[batIdx].relativeWhCapacity = 0; /* ??? */
	ebBatData[batIdx].packSOH = 10000ul; /* ??? */
	ebBatData[batIdx].Cell1 = 3.111; /* ??? */
	ebBatData[batIdx].Cell2 = 3.122; /* ??? */
	ebBatData[batIdx].Cell3 = 3.133; /* ??? */
	ebBatData[batIdx].Cell4 = 3.144; /* ??? */
	ebBatData[batIdx].Cell5 = 3.155; /* ??? */
	ebBatData[batIdx].Cell6 = 3.200; /* ??? */
	ebBatData[batIdx].Cell7 = 3.200; /* ??? */
	ebBatData[batIdx].Cell8 = 3.200; /* ??? */
	ebBatData[batIdx].Cell9 = 3.200; /* ??? */
	ebBatData[batIdx].Cell10 = 3.200; /* ??? */
	ebBatData[batIdx].Cell11 = 3.211; /* ??? */
	ebBatData[batIdx].Cell12 = 3.222; /* ??? */
	ebBatData[batIdx].Cell13 = 3.233; /* ??? */
	ebBatData[batIdx].Cell14 = 3.244; /* ??? */
	ebBatData[batIdx].Cell15 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell1 = 1; /* ??? */
	ebBatData[batIdx].BalCell2 = 2; /* ??? */
	ebBatData[batIdx].BalCell3 = 3; /* ??? */
	ebBatData[batIdx].BalCell4 = 4; /* ??? */
	ebBatData[batIdx].BalCell5 = 5; /* ??? */
	ebBatData[batIdx].BalCell6 = 6; /* ??? */
	ebBatData[batIdx].BalCell7 = 7; /* ??? */
	ebBatData[batIdx].BalCell8 = 8; /* ??? */
	ebBatData[batIdx].BalCell9 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell10 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell11 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell12 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell13 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell14 = 0000ul; /* ??? */
	ebBatData[batIdx].BalCell15 = 0000ul; /* ??? */
	ebBatData[batIdx].packVoltage = 45678; /* 0x2004:1 */
	ebBatData[batIdx].fuseVoltage = 0; /* 0x2004:2 */
	ebBatData[batIdx].motVoltage = 222; /* 0x2004:3 */
	ebBatData[batIdx].chargeVoltage = 0; /* 0x2004:4 */
	ebBatData[batIdx].HVIPL0Voltage = 0; /* 0x2004:5 */
	ebBatData[batIdx].HVIPL1Voltage = 0; /* 0x2004:6 */
 	ebBatData[batIdx].Temp1 = -23; /* 0x2003:1 */
	ebBatData[batIdx].Temp2 = -11; /* 0x2003:2 */
	ebBatData[batIdx].Temp3 = 21; /* 0x2003:3 */
	ebBatData[batIdx].Temp4 = 123; /* 0x2003:4 */
	ebBatData[batIdx].packCurrent = 12; /* 0x2005:1 */
	ebBatData[batIdx].Current_12V = 12345; /* 0x2005:2 */
	ebBatData[batIdx].preChargeDone=0; /* 0x2006:1 */
	ebBatData[batIdx].isChargerConnected=0; /* 0x2006:2 */
}


/***************************************************************************/
/**
* \brief batApplInit - init application
*
* \param
*	nothing
* \results
*	EB_RET_T
*/
EB_RET_T batApplInit(
		void
	)
{
	/* init indication functions */
	if (ebRegister_NMT(indNmt) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (ebRegister_EBCMON(indEbcMon) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (ebRegister_STATE(indEbCheckState, indEbState) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (ebRegister_BAT_STATE(indBatCheckState, indBatState) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (ebRegister_SDO_WRITE(indSdoWrite) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

	if (ebRegister_COMM_EVENT(indErrorHandler) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}
	
	if (ebRegister_CAN_STATE(indCanState) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

#ifdef EB_NETWORK_SECURITY_CNT
	if (ebRegister_SECURITY_EVENT(indSecurity) != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif /* EB_SECURITY_CNT */

#ifdef CO_SLEEP_454
	if (ebRegister_SLEEP(sleepInd) != EB_RET_OK)  {
		return(EB_RET_ERROR);
   	}
#endif /* CO_SLEEP_454 */
   	
	if (ebRegister_EEPROM(eepromWriteInd, eepromReadInd) != EB_RET_OK)  {
		return(EB_RET_ERROR);
   	}

	/* set variables to the default value */
//	batVariableInit();
	batVariableInit(VDN_BATTERY);
	
	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \brief ebAppl - application
*
* \param
*	nothing
* \results
*	nothing
*/
//Battery Status variabled from BQ769x0.c
extern int g_Can_BatteryPackMilliVolts ;
extern int g_Can_BatteryPackTemperature ;
extern int g_Can_PackCurrent;
extern int g_Can_BatteryPackTemperature;
void batAppl(
		void
	)
{
	/* set state ready to attach automatically */
	if ((ebData[0].state & EB_BAT_STATE_MASK) == EB_BAT_STATE_DO_NOT_ATTACH)  {
		ebBatteryStateChange(EB_BAT_STATE_READY_TO_ATTACH, 0);
	}
	/* here can follow code to measure and calculate battery specific
	 * values and set the object dictionary accordingly.
	 * The battery project can be used to be visited
	 * with the CANopen Device Designer
	 * or the battery.csv file, to look up variable names
	 * related to on object dictionary index.
	 * Check the current value range and resolution with CiA 454
	 */
	/* 0x6008 */
#if 1
	ebData[0].nominalVoltage          = 48000; /* INTEGER32 in mV */
	/* 0x6040 */
	//ebData[0].actualVoltage           = 41234;
	//ebData[0].actualCurrent           = 1000; /* INTEGER32 in mA */
	ebBatData[0].packActualWhCapacity = 40000; /* UNSIGNED32 mWh */
#else
	ebData[0].nominalVoltage          = g_Can_BatteryPackMilliVolts; /* INTEGER32 in mV */
	/* 0x6040 */
	ebData[0].actualVoltage           = g_Can_BatteryPackMilliVolts;
	ebData[0].actualCurrent           = g_Can_PackCurrent; /* INTEGER32 in mA */
	ebBatData[0].packActualWhCapacity = 40000; /* UNSIGNED32 mWh */
#endif
}


