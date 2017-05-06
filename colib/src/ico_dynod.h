/*
* ico_dynod.h - contains internal defines for DYNAMIC created Objects
*
* Copyright (c) 2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_dynod.h 9827 2015-07-09 09:23:36Z boe $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_DYNOD_H
#define ICO_DYNOD_H 1


/* datatypes */


/* function prototypes */

UNSIGNED8	*icoDynOdGetObjAddr(CO_CONST CO_OBJECT_DESC_T *pDesc);
RET_T		icoDynOdGetObjDescPtr(UNSIGNED16 index,
				UNSIGNED8 subIndex, CO_CONST CO_OBJECT_DESC_T **pDescPtr);
UNSIGNED32	icoDynOdGetObjSize(CO_CONST CO_OBJECT_DESC_T *pDesc);

#endif /* ICO_DYNOD_H */
