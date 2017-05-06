/*
* ico_network.h - contains internal defines for multi level networking
*
* Copyright (c) 2014-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_network.h 13646 2016-04-28 12:57:33Z boe $
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

#ifndef ICO_NETWORK_H
#define ICO_NETWORK_H 1

/* datatypes */

/* function prototypes */

UNSIGNED16 icoNetworkLocalId(void);
void *icoNetworkGetObjectAddr(UNSIGNED16 index, UNSIGNED8	subIndex);
void icoNetworkSetDefaultValue(void);
void icoNetworkVarInit(CO_CONST UNSIGNED8 *pList);

#endif /* ICO_NETWORK_H */
