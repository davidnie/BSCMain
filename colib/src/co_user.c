/*
* co_user.c - contains user functionality to send/receive other CAN messages
*
* Copyright (c) 2015-2015 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_user.c 10352 2015-08-24 14:34:14Z boe $
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief User CAN functionality
*
* \file co_user.c
* Contain functions to send other data over CAN
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_USER_EXTENSION_SUPPORTED
#include <co_datatype.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_user.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_user.h"

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
	COB_REFERENZ_T	cobData;
	UNSIGNED8		dataLen;
} CO_USR_TR_DATA_T;

typedef struct {
	COB_REFERENZ_T	cobData;
	CO_EVENT_USER_T	userInd;
} CO_USR_REC_DATA_T;

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
static CO_USR_TR_DATA_T	 usrTrData[USER_TRANSMIT_MSG_CNT];
static CO_USR_REC_DATA_T usrRecData[USER_RECEIVE_MSG_CNT];


/***************************************************************************/
/**
*
* \brief coUserInitTransmitMessage - init transmit message
*
* Initilize user transmit message
* Parameter Message number have to be between 1..USER_TRANSMIT_MSG_CNT.
* canId must no be a restricted can identifier.
*
* Transmission can be done by calling coUserTransmitMessage()
*
* \return RET_T
*
*/
RET_T coUserInitTransmitMessage(
		UNSIGNED16		msgNr,			/**< message number 1..n */
		UNSIGNED32		canId,			/**< transmit CAN identifier */
		UNSIGNED8		dataLen			/**< data len */
	)
{
RET_T	retVal;
CO_USR_TR_DATA_T	*pUsr;

	/* check message number */
	if ((msgNr < 1) || (msgNr > USER_TRANSMIT_MSG_CNT)) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* check restricted cobs */
	if (icoCheckRestrictedCobs(canId, 0u, 0u) == CO_TRUE)  {
		return(RET_SDO_INVALID_VALUE);
	}

	pUsr = &usrTrData[msgNr - 1];

	/* define COB */
	if (pUsr->cobData == 0xffffu)  {
		pUsr->cobData = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_USER, 0u);
		if (pUsr->cobData == 0xffffu)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}

	pUsr->dataLen = dataLen;

	/* setup cobid */
	retVal = icoCobSet(pUsr->cobData, canId, CO_COB_RTR_NONE, dataLen);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	return(RET_OK);
}


/***************************************************************************/
/**
*
* \brief coUserTransmitMessage - transmit message
*
*
* \return RET_T
*
*/
RET_T coUserTransmitMessage(
		UNSIGNED16		msgNr,		/**< message number 1..n */
		const UNSIGNED8	*pData		/**< pointer to data */
	)
{
CO_TRANS_DATA_T	trData;
RET_T			retVal;
CO_USR_TR_DATA_T	*pUsr;

	/* check message number */
	if ((msgNr < 1u) || (msgNr > USER_TRANSMIT_MSG_CNT)) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	pUsr = &usrTrData[msgNr - 1];
	if (pUsr->cobData == 0xffffu)  {
		return(RET_NOT_INITIALIZED);
	}

	/* copy data */
	memcpy(&trData.data[0], pData, (size_t)pUsr->dataLen);

	/* transmit data */
	retVal = icoTransmitMessage(pUsr->cobData, &trData, 0u);

	return(retVal);
}


/***************************************************************************/
/**
*
* \brief coUserInitReceiveMessage - init receive message
*
* Initilize user receive messages
* Parameter Message number have to be between 1..USER_RECEIVE_MSG_CNT.
* canId must no be a restricted can identifier.
*
* Received messages are indicated by calling given pRecFct()
*
* \return RET_T
*
*/
RET_T coUserInitReceiveMessage(
		UNSIGNED16		msgNr,			/**< message number 1..n */
		UNSIGNED32		canId,			/**< transmit CAN identifier */
		UNSIGNED8		dataLen,		/**< data len */
		CO_EVENT_USER_T	pRecFct			/**< pointer to receive function */
	)
{
RET_T	retVal;
CO_USR_REC_DATA_T	*pUsr;

	/* check message number */
	if ((msgNr < 1) || (msgNr > USER_RECEIVE_MSG_CNT)) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* check restricted cobs */
	if (icoCheckRestrictedCobs(canId, 0u, 0u) == CO_TRUE)  {
		return(RET_SDO_INVALID_VALUE);
	}

	pUsr = &usrRecData[msgNr - 1];

	/* define COB */
	if (pUsr->cobData == 0xffffu)  {
		pUsr->cobData = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_USER, msgNr);
		if (pUsr->cobData == 0xffffu)  {
			return(RET_EVENT_NO_RESSOURCE);
		}
	}

	/* setup cobid */
	retVal = icoCobSet(pUsr->cobData, canId, CO_COB_RTR_NONE, dataLen);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	pUsr->userInd = pRecFct;

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoUserHandler
*
* \return none
*
*/
void icoUserHandler(
		CO_CONST CO_REC_DATA_T	*pRec	/* pointer to receive data */
	)
{
CO_USR_REC_DATA_T	*pUsr;

	pUsr = &usrRecData[pRec->spec - 1];
	if (pUsr->userInd != NULL)  {
		pUsr->userInd(pRec->spec, pRec->msg.len, &pRec->msg.data[0]);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief
*
* \return none
*
*/
void icoUserVarInit(
		void
	)
{
UNSIGNED16	i;

	{
		for (i = 0u; i < USER_TRANSMIT_MSG_CNT; i++)  {
			usrTrData[i].cobData = 0xffff;
		}
		for (i = 0u; i < USER_RECEIVE_MSG_CNT; i++)  {
			usrRecData[i].cobData = 0xffff;
		}
	}
}
#endif /* CO_USER_EXTENSION_SUPPORTED */
