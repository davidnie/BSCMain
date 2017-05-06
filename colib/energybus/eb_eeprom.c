/*
* eb_eeprom.c - load/save from/to eeprom
*
* Copyright (c) 2013 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_eeprom.c 3373 2013-05-31 06:23:20Z  $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief eeprom handling
* contains routines for eeprom handling
*
* \file eb_eeprom.c
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <eb_energybus.h>

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

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
static EB_EEPROM_LOAD_ACCESS_T	loadTable = NULL;
static EB_EEPROM_SAVE_ACCESS_T  saveTable = NULL;



/***************************************************************************/
/**
* \brief ebRegister_LOAD - register LOAD function
*
* Register a function for load and save EEPROM data.
*
* \return
*	EB_RET_T
*/
EB_RET_T ebRegister_EEPROM(
		EB_EEPROM_SAVE_ACCESS_T pSave,		/**< pointer for eeprom save routine */
		EB_EEPROM_LOAD_ACCESS_T pLoad		/**< pointer for eeprom load routine */
	)
{
	/* only one function are allowed */
	if ((loadTable != NULL) || (saveTable != NULL)) {
		return(EB_RET_ERROR);
	}

	loadTable = pLoad;      /* save function pointer */
	saveTable = pSave;      /* save function pointer */

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebEepromLoad - load parameter 
*
*
* \return
*	RET_OK .. data loaded
*   RET_HARDWARE_ERROR .. data not loaded
*/
EB_RET_T iebEepromLoad(
		EB_EEPROM_DATA_T eepromData,	/* type of data */
		UNSIGNED16 number,				/* number depend at the type of data */
		UNSIGNED16 length,				/* number of bytes to load */
		UNSIGNED8 * pData 				/* pointer for loaded data of length Byte */
	)
{
EB_RET_T retVal = EB_RET_ERROR;

	if (loadTable != NULL)  {
		retVal = loadTable(eepromData, number, length, pData);
	} 
	return retVal;
}


/***************************************************************************/
/**
* \internal
*
* \brief iebEepromSave - save parameter 
*
*
* \return
*	RET_OK .. data saved or no callback
*   RET_HARDWARE_ERROR .. data not saved
*/
EB_RET_T iebEepromSave(
		EB_EEPROM_DATA_T eepromData,	/* type of data */
		UNSIGNED16 number,				/* number depend at the type of data */
		UNSIGNED16 length,				/* number of bytes to load */
		CO_CONST UNSIGNED8 * pData 		/* pointer of data to save */
	)
{
EB_RET_T retVal = EB_RET_OK;

	if (saveTable != NULL)  {
		retVal = saveTable(eepromData, number, length, pData);
	} 
	return retVal;
}

