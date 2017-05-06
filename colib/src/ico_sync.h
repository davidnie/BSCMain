/*
* ico_sync.h - contains internal defines for SYNC
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_sync.h 8127 2015-01-13 06:53:49Z boe $

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

#ifndef ICO_SYNC_H
#define ICO_SYNC_H 1


/* datatypes */


/* function prototypes */

void	*icoSyncGetObjectAddr(UNSIGNED16 index);
RET_T	icoSyncObjChanged(UNSIGNED16 valIdx);
void	icoSyncReset(void);
void	icoSyncSetDefaultValue(void);
void	icoSyncHandler(UNSIGNED8 syncCounterVal);
RET_T	icoSyncCheckObjLimit_u8(UNSIGNED16 index, UNSIGNED8 value);
RET_T	icoSyncCheckObjLimit_u32(UNSIGNED16 index, UNSIGNED32 value);
void	icoSyncVarInit(void);

#endif /* ICO_SYNC_H */
