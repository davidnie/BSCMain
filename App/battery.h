/*
* battery.h - contains defines for akku application
*
* Copyright (c) 2012 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: battery.h 1415 2012-08-10 16:02:18Z  $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \file
* \brief datatype header
*/

#ifndef BATTERY_H
#define BATTERY_H 1


/* datatypes */

/* function prototypes */

EB_RET_T batApplInit(void);
void batAppl(void);

/* indication functions */
void indNmt(UNSIGNED8 state);
void indEbcMon(UNSIGNED8 ebc, UNSIGNED8 active);
BOOL_T indEbCheckState(UNSIGNED16 state, UNSIGNED8 vdn);
void indEbState(UNSIGNED16 state, UNSIGNED8 vdn);
void indBatState(UNSIGNED16 state, UNSIGNED8 vdn);
BOOL_T indBatCheckState(UNSIGNED16 state, UNSIGNED8 vdn);
void indPdo(UNSIGNED16 pdoNr);
void indSdoWrite(UNSIGNED16 index, UNSIGNED8 subIndex);
void indCanState(CO_CAN_STATE_T event);
void indErrorHandler(CO_COMM_STATE_EVENT_T event);

#ifdef EB_NETWORK_SECURITY_CNT
BOOL_T indSecurity(EB_SEC_EVENT_T event, UNSIGNED16 devType, UNSIGNED8 vdn);
#endif /* EB_NETWORK_SECURITY_CNT */
UNSIGNED8 sleepInd(CO_SLEEP_MODE_T	mode);
EB_RET_T eepromWriteInd(EB_EEPROM_DATA_T type, UNSIGNED16 offs, UNSIGNED16 len,
	CO_CONST UNSIGNED8 *src); 
EB_RET_T eepromReadInd(EB_EEPROM_DATA_T type, UNSIGNED16 offs, UNSIGNED16 len,
	UNSIGNED8 *dst); 


#endif /* BATTERY_H */

