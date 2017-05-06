/*
* co_cob.h - contains defines for cob types
*
* Copyright (c) 2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_cob.h 8189 2015-01-16 11:51:35Z boe $
*
*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \brief cob defines
*
* \file co_cob.h - contains defines for cobs
*
*/

#ifndef CO_COB_H
#define CO_COB_H 1


/**
* COB invalid
* if this bit is set, the COB-ID (and the service) is invalid
*/
#define CO_COB_INVALID		0x80000000UL

/**
* COB 29bit flag
* if this bit is set, the COB-ID is a 29-bit identifier
*/
#define CO_COB_29BIT		0x20000000UL

/**
* cob valid mask
* With this mask, cobs can be checked for valid
*/
#define CO_COB_VALID_MASK	0x80000000UL

/**
* COB 29bit mask
* With this mask, cobs can be checked for 29bit identifier
*/
#define CO_COB_29BIT_MASK	0x20000000UL

/**
* COB ID mask
* With this mask, only identifier bits are masked
*/
#define CO_COB_ID_MASK		0x1FFFFFFFUL

#endif /* CO_COB_H */

