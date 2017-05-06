/*
* eb_eeprom.h - contains defines to load/save parameter
*
* Copyright (c) 2013 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: eb_eeprom.h 4731 2013-10-11 15:38:24Z  $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for eeprom usage
*
* \file eb_eeprom.h - contains defines for eeprom usage
*/

#ifndef EB_EEPROM_H
#define EB_EEPROM_H 1

#include <co_datatype.h>

/**
* number of known nodes structures at eeprom
*/
#ifndef EB_LSS_KNOWN_NODES_CNT 
# define EB_LSS_KNOWN_NODES_CNT 31
#endif

/**
* EEPROM parts
*/
typedef enum {
	EB_EEPROM_KNOWN_DEVICES,	/**< LSS address and node id assign */
	EB_EEPROM_SECURITY_KEYS	/**< security keys */
} EB_EEPROM_DATA_T;


/** \brief function pointer EEPROM load function
 * \param type - eeprom data type (see EB_EEPROM_DATA_T)
 * \param nr   - data record number - depends at the type of data
 * \param len  - number of bytes to load 
 * \param pData - pointer to load data
 * 
 * \return EB_RET_T
 */
typedef EB_RET_T (* EB_EEPROM_LOAD_ACCESS_T)(EB_EEPROM_DATA_T type, UNSIGNED16 nr, UNSIGNED16 len, UNSIGNED8 *pData);


/** \brief function pointer EEPROM save function
 * \param type - eeprom data type (see EB_EEPROM_DATA_T)
 * \param nr   - data record number - depends at the type of data
 * \param len  - number of bytes to save
 * \param pData - pointer to data
 * 
 * \return EB_RET_T
 */
typedef EB_RET_T (* EB_EEPROM_SAVE_ACCESS_T)(EB_EEPROM_DATA_T type, UNSIGNED16 nr, UNSIGNED16 len, CO_CONST UNSIGNED8 *pData);

/* external data */

/* function prototypes */
EB_RET_T	iebEepromLoad(EB_EEPROM_DATA_T eepromData, UNSIGNED16 number,
				UNSIGNED16 length, UNSIGNED8 * pData);
EB_RET_T	iebEepromSave(EB_EEPROM_DATA_T eepromData, UNSIGNED16 number,
				UNSIGNED16 length, CO_CONST UNSIGNED8 * pData);

EB_RET_T	ebRegister_EEPROM(EB_EEPROM_SAVE_ACCESS_T pSaveFct,
				EB_EEPROM_LOAD_ACCESS_T pLoadFct);


#endif /* EB_EEPROM_H */

