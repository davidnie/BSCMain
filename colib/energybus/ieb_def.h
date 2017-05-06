/*
* ieb_def.h - contains function prototypes
*
* Copyright (c) 2012 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ieb_def.h 8205 2015-01-16 12:07:00Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef IEB_DEF_H
#define IEB_DEF_H 1

#include <co_datatype.h>
#include <eb_data.h>
#include <eb_fct.h>
# ifdef CO_LSS_MASTER_SUPPORTED
#include <co_lss.h>
# endif /* CO_LSS_MASTER_SUPPORTED */



/* datatypes */


/* external data */

/* function prototypes */

EB_RET_T	iebIndInit(void);
RET_T		iebLoadInd(UNSIGNED8 subIndex);
void		iebLoadIndInit(CO_EVENT_STORE_T pLoadFunction);

EB_RET_T	iebStateSetIndication(EB_EVENT_CHECK_STATE_T pCheckFunction, 
				EB_EVENT_STATE_T pFunction);
EB_RET_T	iebStateCheckChange(UNSIGNED16 newState, UNSIGNED8 devIdx);
EB_RET_T	iebStateCheckTransition(UNSIGNED16 ctrlWord, UNSIGNED8 devIdx);

void		iebSdoClientReadInd(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED32	size, UNSIGNED32 result);
void		iebSdoClientWriteInd(UNSIGNED8	sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED32	result);
UNSIGNED32	iebSdoClientGetSizeRead(void);

UNSIGNED16	iebGetPdoMessageOffs(UNSIGNED32	virtualDevice, UNSIGNED8 msgNr,
				UNSIGNED32 *pCobId, BOOL_T mode);
RET_T		iebUpdateStandardTPdos(void);
RET_T		iebUpdateDeviceTPdos(UNSIGNED8	devIdx);

UNSIGNED8	iebGetDevIdxFromSubIndex(UNSIGNED8 subIndex);

EB_RET_T	iebVcCheckState(UNSIGNED16	ctrlWord, UNSIGNED8	devIdx);
RET_T		iebVcSetBatteryNodeId(UNSIGNED16	index, UNSIGNED8 subIndex);

UNSIGNED8	iebSleepInd(CO_SLEEP_MODE_T mode, UNSIGNED8 nodeId);

void		iebResponseTransferData(BOOL_T execFlag);

#endif /* IEB_DEF_H */

