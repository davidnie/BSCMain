/*
* ico_store.h - contains internal defines for STORE
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_store.h 8127 2015-01-13 06:53:49Z boe $

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

#ifndef ICO_STORE_H
#define ICO_STORE_H 1


/* datatypes */


/* function prototypes */

RET_T	icoStoreLoadReq(UNSIGNED8 subIndex);
void	*icoStoreGetObjectAddr(UNSIGNED16 idx, UNSIGNED8 subIndex);
RET_T	icoStoreCheckObjLimit_u32(UNSIGNED16 index, UNSIGNED32 signature);
RET_T	icoStoreObjChanged(UNSIGNED16 index, UNSIGNED8 subIndex);
void	icoStoreVarInit(void);

#endif /* ICO_STORE_H */
