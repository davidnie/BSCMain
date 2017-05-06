/*
* co_candebug.c - contains debug functionality over CAN
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_candebug.c 13636 2016-04-28 12:42:24Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief CAN debug functionality
*
* \file co_candebug.c
* Contain functions to send any data over CAN
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_CAN_DEBUG_SUPPORTED
#include <co_datatype.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_candebug.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_candebug.h"

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
static COB_REFERENZ_T	debugCob = { 0xffffu };


/***************************************************************************/
/**
*
* \brief coCandebugPrint - send debug message over CAN
*
*
* \return RET_T
*
*/
RET_T coCanDebugPrint(
		UNSIGNED32	canId,			/**< CAN identifier */
		UNSIGNED8	dataLen,		/**< CAN date len */
		const UNSIGNED8	*pData		/**< pointer to data */
	)
{
CO_TRANS_DATA_T	trData;
RET_T			retVal;

	if (debugCob == 0xffffu)  {
		debugCob = icoCobCreate(CO_COB_TYPE_TRANSMIT,
			CO_SERVICE_CAN_DEBUG, 0u);
		if (debugCob == 0xffffu)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}

	retVal = icoCobSet(debugCob, canId, CO_COB_RTR_NONE, dataLen);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	memcpy(&trData.data[0], pData, (size_t)dataLen);

	/* transmit data */
	retVal = icoTransmitMessage(debugCob, &trData, 0u);

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoDebugVarInit
*
* \return none
*
*/
void icoCanDebugVarInit(
		void
	)
{

	{
		debugCob = 0xffffu;
	}
}
#endif /* CO_CAN_DEBUG_SUPPORTED */
