/*
* co_nmt.c - contains NMT services
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_nmt.c 14541 2016-07-15 08:45:00Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief Network Managment(NMT) handler
*
* \file co_nmt.c
* contains routines for NMT handling
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdlib.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_timer.h>
#include <co_led.h>
#include <co_odaccess.h>
#include <co_drv.h>
#include <co_store.h>
#include <co_commtask.h>
#include <co_nmt.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_event.h"
#ifdef CO_SDO_NETWORKING
# include "ico_sdoserver.h"
#endif /* CO_SDO_NETWORKING */
#include "ico_sdo.h"
#include "ico_pdo.h"
#include "ico_nmt.h"
#include "ico_sync.h"
#include "ico_emcy.h"
#include "ico_odaccess.h"
#include "ico_time.h"
#include "ico_store.h"
#ifdef CO_LSS_SUPPORTED
# include "ico_lss.h"
#endif /* CO_LSS_SUPPORTED */
#ifdef CO_LSS_MASTER_SUPPORTED
# include "ico_lss.h"
#endif /* CO_LSS_MASTER_SUPPORTED */
#ifdef CO_EVENT_SLEEP
# include "ico_sleep.h"
#endif /* CO_EVENT_SLEEP */
#ifdef CO_SRD_SUPPORTED
# include "ico_srd.h"
#endif /* CO_SRD_SUPPORTED */
#ifdef CO_NETWORK_ROUTING_CNT
# include "ico_network.h"
#endif /* CO_NETWORK_ROUTING_CNT */
#ifdef CO_GFC_SUPPORTED
# include "ico_gfc.h"
#endif /* CO_GFC_SUPPORTED */
#ifdef CO_SRDO_SUPPORTED
# include "ico_srdo.h"
#endif /* CO_SRDO_SUPPORTED */
#ifdef CO_USDO_SUPPORTED
# include "ico_usdoserver.h"
#endif /* CO_USDO_SUPPORTED */


/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_NMT
# ifdef CO_EVENT_PROFILE_NMT
#  define CO_EVENT_NMT_CNT	(CO_EVENT_DYNAMIC_NMT + CO_EVENT_PROFILE_NMT)
# else /* CO_EVENT_PROFILE_NMT */
#  define CO_EVENT_NMT_CNT	(CO_EVENT_DYNAMIC_NMT)
# endif /* CO_EVENT_PROFILE_NMT */
#else /* CO_EVENT_DYNAMIC_NMT */
# ifdef CO_EVENT_PROFILE_NMT
#  define CO_EVENT_NMT_CNT	(CO_EVENT_PROFILE_NMT)
# endif /* CO_EVENT_PROFILE_NMT */
#endif /* CO_EVENT_DYNAMIC_NMT */

#if defined(CO_EVENT_STATIC_NMT) || defined(CO_EVENT_NMT_CNT)
# define CO_EVENT_NMT   1u
#endif /* defined(CO_EVENT_STATIC_NMT) || defined(CO_EVENT_NMT_CNT) */


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_NMT
extern CO_CONST CO_EVENT_NMT_T coEventNmtInd;
#endif /* CO_EVENT_STATIC_NMT */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void nmtReset(void);
static RET_T nmtResetComm(void);
static void nmtResetAppl(void);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static UNSIGNED8		nodeId = { 127u };
static CO_NMT_STATE_T	nmtState = {CO_NMT_STATE_RESET_NODE};
#ifdef CO_EVENT_NMT_CNT
static CO_EVENT_NMT_T	nmtTable[CO_EVENT_NMT_CNT];
static UNSIGNED8		nmtTableCnt = 0u;
#endif /* CO_EVENT_NMT_CNT */
static COB_REFERENZ_T	nmtCob;
static UNSIGNED8		persistantNodeId;
static CO_NODE_ID_T		nodeIdFunction;



/***************************************************************************/
/**
* \internal
*
* \brief coNmtMsgHandler - handle received NMT mesages
*
*
* \return none
*
*/
void icoNmtMsgHandler(
		CO_CONST CO_REC_DATA_T *pNmtRec		/* pointer to receive data */
	)
{
RET_T	retVal;

	/* switch depending on cob-id - start with NMT command */
	if (pNmtRec->msg.canCob.canId == 0u)  {

		/* check for correct message len */
		if (pNmtRec->msg.len != 2u)  {
			return;
		}

		/* node id = own node id or = 0 ? */
		if ((pNmtRec->msg.data[1] == nodeId) || (pNmtRec->msg.data[1] == 0u)) {
			/* new state command */
			switch (pNmtRec->msg.data[0])  {
				case 1u:
					icoNmtStateChange(CO_NMT_STATE_OPERATIONAL, CO_TRUE);
					break;
				case 2u:
					icoNmtStateChange(CO_NMT_STATE_STOPPED, CO_TRUE);
					break;
				case 128u:
					icoNmtStateChange(CO_NMT_STATE_PREOP, CO_TRUE);
					break;
				case 129u:
					icoNmtStateChange(CO_NMT_STATE_RESET_NODE, CO_TRUE);
					nmtResetAppl();
					retVal = nmtResetComm();
					if (retVal == RET_OK)  {
						icoNmtStateChange(CO_NMT_STATE_PREOP, CO_TRUE);
					}
					break;
				case 130u:
					retVal = nmtResetComm();
					if (retVal == RET_OK)  {
						icoNmtStateChange(CO_NMT_STATE_PREOP, CO_TRUE);
					}
					break;
				default:
					break;
			}
		}
	}

	/* NMT error control message ? */
	if (pNmtRec->msg.canCob.canId > 0x700u)  {
		/* check for correct message len */
		if (pNmtRec->msg.len != 1u)  {
			return;
		}

		icoNmtErrorCtrlHandler(pNmtRec);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief icoNmtStateChange - setup new state change
*
*
* \return none
*
*/
void icoNmtStateChange(
		CO_NMT_STATE_T	reqState,		/* new requested state */
		BOOL_T			cmdFromMaster	/* command comes from master */
	)
{
RET_T	retVal;
#ifdef CO_EVENT_NMT_CNT
UNSIGNED8			cnt;
#endif /* CO_EVENT_NMT_CNT */

	/* ignore same command */
	if (nmtState == reqState)  {
		return;
	}

	/* no command from master ? */
	if (cmdFromMaster != CO_TRUE)  {
		/* only in state OPER allowed */
		if (nmtState != CO_NMT_STATE_OPERATIONAL)  {
			return;
		}
	}

#ifdef CO_SRDO_SUPPORTED
	if (reqState == CO_NMT_STATE_OPERATIONAL)  {
		retVal = icoSrdoCheckOperational();
		if (retVal != RET_OK)  {
			return;
		}
	} else
#endif /* CO_SRDO_SUPPORTED */
	{
		/* normally user indication is ok */
		retVal = RET_OK;
	}


#ifdef CO_EVENT_NMT
	/* only in state OPER the change state can be aborted by user */
	if (reqState == CO_NMT_STATE_OPERATIONAL)  {
		/* check all indications for correct return value */

# ifdef CO_EVENT_NMT_CNT
		cnt = nmtTableCnt;
		while (cnt > 0u)  {
			cnt--;

			/* check user indication */
			retVal = nmtTable[cnt](CO_FALSE, reqState);
			if (retVal != RET_OK)  {
				break;
			}
		}
# endif /* CO_EVENT_NMT_CNT */

# ifdef CO_EVENT_STATIC_NMT
		if (retVal == RET_OK)  {
			retVal = coEventNmtInd(CO_FALSE, reqState);
		}
# endif /* CO_EVENT_STATIC_NMT */
	}
#endif /* CO_EVENT_NMT */

	/* execute only if retval was ok */
    if (retVal == RET_OK)  {
		/* set new nmt state */
		nmtState = reqState;

#ifdef CO_EVENT_LED
		switch (nmtState)  {
			case CO_NMT_STATE_PREOP:
				coLedSetGreen(CO_LED_STATE_BLINKING);
				break;
			case CO_NMT_STATE_OPERATIONAL:
				coLedSetGreen(CO_LED_STATE_ON);
				break;
			case CO_NMT_STATE_STOPPED:
				coLedSetGreen(CO_LED_STATE_FLASH_1);
				break;
			default:
				break;
		}
#endif /* CO_EVENT_LED */

#ifdef CO_EVENT_NMT
# ifdef CO_EVENT_NMT_CNT
		/* call all user indications */
        cnt = nmtTableCnt;
		while (cnt > 0u)  {
			cnt--;
			/* call user indication */
			(void)nmtTable[cnt](CO_TRUE, reqState);
		}
# endif /* CO_EVENT_NMT_CNT */

# ifdef CO_EVENT_STATIC_NMT
		(void)coEventNmtInd(CO_TRUE, reqState);
# endif /* CO_EVENT_STATIC_NMT */
#endif /* CO_EVENT_NMT */


		/* new state == OPER ? */
		if (reqState == CO_NMT_STATE_OPERATIONAL)  {

#ifdef CO_PDO_TRANSMIT_CNT
			/* transmit async PDOs and start pdo event timer */
			icoPdoOperational();
#endif /* CO_PDO_TRANSMIT_CNT */

#ifdef CO_SRDO_SUPPORTED
			icoSrdoOperational();
#endif /* CO_SRDO_SUPPORTED */

		} else {

#ifdef CO_PDO_TRANSMIT_CNT
			/* stop event timers */
			icoPdoPreOperational();
#endif /* CO_PDO_TRANSMIT_CNT */

#ifdef CO_SRDO_SUPPORTED
			icoSrdoPreOperational();
#endif /* CO_SRDO_SUPPORTED */

		}
	}
}


#ifdef CO_EVENT_NMT_CNT
/***************************************************************************/
/**
* \brief coEventRegister_NMT - register NMT event
*
* register indication function for NMT events
*
* \return RET_T
*
*/

RET_T coEventRegister_NMT(
		CO_EVENT_NMT_T pFunction	/**< pointer to function */
    )
{
	/* set new indication function as first at the list */
	if (nmtTableCnt >= CO_EVENT_NMT_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	nmtTable[nmtTableCnt] = pFunction;	/* save function pointer */
	nmtTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_NMT_CNT */


/***************************************************************************/
/**
* \internal
*
* \brief get persistant node id
*
*
* \return node id
*
*/
UNSIGNED8	icoNmtPersistantNodeIdGet(
		void	/* no parameter */
	)
{
UNSIGNED8	nId;

	if (nodeIdFunction != NULL)  {
		nId = nodeIdFunction();
	} else {
		nId = persistantNodeId;
	}

	return(nId);
}


/***************************************************************************/
/**
* \internal
*
* \brief reset application services
*
*
* \return none
*
*/
static void nmtResetAppl(
		void	/* no parameter */
	)
{
	/* init event handler queue */
	icoEventInit();

#ifdef CO_LSS_SUPPORTED
	icoLssResetAppl();
#endif /* CO_LSS_SUPPORTED */

	icoOdReset(0x2000u, 0x9fffu);
	(void) icoStoreLoadReq(CO_STORE_AREA_APPL);

	coQueueInit();
}


/***************************************************************************/
/**
* \internal
*
* \brief reset communication parameter and services
*
*
* \return none
*
*/
static RET_T nmtResetComm(
		void	/* no parameter */
	)
{
#ifdef CO_EVENT_SLEEP
# ifdef CO_NMT_MASTER
CO_NMT_MASTER_STATE_T mState = CO_NMT_MASTER_STATE_SLAVE;
# endif /* CO_NMT_MASTER */
#endif /* CO_EVENT_SLEEP */

	nodeId = icoNmtPersistantNodeIdGet();

	/* disable all cobs */
	icoCobDisableAll();

	nmtReset();

#ifdef CO_LSS_SUPPORTED
	icoLssReset(&nodeId);
	if (nodeId == 255u)  {
# ifdef CO_NO_NODE_ID_LED
		coLedSetGreen(CO_LED_STATE_FLICKERING);
# endif /* CO_NO_NODE_ID_LED */ 
		return(RET_INVALID_NODEID);
	}
#endif /* CO_LSS_SUPPORTED */

#ifdef CO_EVENT_SLEEP
# ifdef CO_NMT_MASTER
	mState = icoNmtMasterGetState();

	if (mState == CO_NMT_MASTER_STATE_MASTER)  {
		icoSleepReset(1u, nodeId);
	} else
# endif /* CO_NMT_MASTER */
	{
		icoSleepReset(0u, nodeId);
	}
#endif /* CO_EVENT_SLEEP */

	if ((nodeId < 1u) || (nodeId > 127u))  {
		return(RET_INVALID_NODEID);
	}

	/* reset objcect dictionary */
	icoOdReset(0x1000u, 0x1fffu);

	/* setup predefined connection set */
	icoSdoServerSetDefaultValue();
	
#ifdef CO_USDO_SUPPORTED
	icoUsdoServerSetDefaultValue();
#endif

#ifdef CO_SDO_CLIENT_CNT
	icoSdoClientSetDefaultValue();
#endif /*  CO_SDO_CLIENT_CNT */

#if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)
	icoPdoSetDefaultValue();
#endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */

	icoErrorCtrlSetDefaultValue();

#ifdef CO_SYNC_SUPPORTED
	icoSyncSetDefaultValue();
#endif /* CO_SYNC_SUPPORTED */

#ifdef CO_EMCY_PRODUCER
	icoEmcyProducerSetDefaultValue();
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
	icoEmcyConsumerSetDefaultValue();
#endif /* CO_EMCY_CONSUMER_CNT */

#ifdef CO_TIME_SUPPORTED
	icoTimeSetDefaultValue();
#endif /* CO_TIME_SUPPORTED */

#if defined(CO_NMT_MASTER) || defined(CO_SELF_STARTING_DEVICE)
	icoNmtSetDefaultValue();
#endif /* CO_NMT_MASTER || CO_SELF_STARTING_DEVICE */

#ifdef CO_NETWORK_ROUTING_CNT
	icoNetworkSetDefaultValue();
#endif /* CO_NETWORK_ROUTING_CNT */

#ifdef CO_SRDO_SUPPORTED
	icoSrdoSetDefaultValue();
#endif /* CO_SRDO_SUPPORTED */

	/* load stored parameter */
	(void) icoStoreLoadReq(CO_STORE_AREA_COMM);


	/* call reset services */
	icoSdoServerReset();
	
#ifdef CO_USDO_SUPPORTED
	icoUsdoServerReset();
#endif /* CO_USDO_SUPPORTED */

#ifdef CO_SDO_CLIENT_CNT
	icoSdoClientReset();
#endif /*  CO_SDO_CLIENT_CNT */

#if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)
	icoPdoReset();
#endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */

#ifdef CO_SRDO_SUPPORTED
	icoSrdoReset();
#endif /* CO_SRDO_SUPPORTED */

	icoNmtStateChange(CO_NMT_STATE_RESET_COMM, CO_TRUE);
	icoNmtStateChange(CO_NMT_STATE_PREOP, CO_TRUE);
	icoErrorCtrlReset();

#ifdef CO_SYNC_SUPPORTED
	icoSyncReset();
#endif /* CO_SYNC_SUPPORTED */

#ifdef CO_EMCY_PRODUCER
	icoEmcyReset();
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
	icoEmcyConsumerReset();
#endif /* CO_EMCY_CONSUMER_CNT */

#ifdef CO_TIME_SUPPORTED
	icoTimeReset();
#endif /* CO_TIME_SUPPORTED */

#ifdef CO_LSS_MASTER_SUPPORTED
	icoLssMasterReset();
#endif /* CO_LSS_MASTER_SUPPORTED */

#ifdef CO_NMT_MASTER
	icoNmtMasterReset();

# ifdef CO_FLYING_MASTER_SUPPORTED
	icoNmtFlymaReset();
# endif /* CO_FLYING_MASTER_SUPPORTED */
#endif /* CO_NMT_MASTER */

#ifdef CO_SRD_SUPPORTED
	icoSrdReset();
#endif /* CO_SRD_SUPPORTED */

#ifdef CO_GFC_SUPPORTED
	icoGfcReset();
#endif /* CO_GFC_SUPPORTED */

#ifndef CO_NMT_MASTER
# ifdef CO_SELF_STARTING_DEVICE
	icoNmtApplyObj();
# endif /* CO_SELF_STARTING_DEVICE */
#endif /* !CO_NMT_MASTER */

	(void)icoHbProdStart();

	return(RET_OK);
}


/***************************************************************************/
/**
*
* \brief coNmtGetNodeId - returns actual node id 
*
*
* \return node-id
*
*/
UNSIGNED8 coNmtGetNodeId(
		void	/* no parameter */
	)
{
	return(nodeId);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoNmtGetCob - returns nmt cob ident
*
*
* \return COB_REFERENCE_T
*
*/
COB_REFERENZ_T icoNmtGetCob(
		void	/* no parameter */
	)
{
	return(nmtCob);
}


#ifdef CO_LSS_SUPPORTED
/***************************************************************************/
/**
* \internal
*
* \brief icoNmtResetNodeId - reset node id to invalid
*
* Set the internal node-id to 255.
* Communication is not more possible until a reset comm was called
*
* \return none
*
*/
void icoNmtResetNodeId(
		void	/* no parameter */
	)
{
UNSIGNED8	node;

	/* only possible for fixed node id == 255 */
	node = icoNmtPersistantNodeIdGet();
	if (node != 255u)  { /*lint !e774 depends on LSS configuration */
		return;
	}

	nodeId = 255u;

	/* disable all cobs */
	icoCobDisableAll();

	/* enable NMT COB */
	nmtReset();

	icoLssResetNodeId();
}
#endif /* CO_LSS_SUPPORTED */


/***************************************************************************/
/**
* \brief coGetNmtState - returns current NMT state
*
* This function returns the current NMT state of the local node.
*
* \return NMT state
*
*/
CO_NMT_STATE_T coNmtGetState(
		void	/* no parameter */
	)
{
	return(nmtState);
}


/***************************************************************************/
/**
* \internal
*
* \brief coResetNmt - reset comm for nmt
*
*
* \return none
*
*/
static void nmtReset(
		void	/* no parameter */
	)
{
#ifdef CO_EVENT_SLEEP
# ifdef CO_NMT_MASTER
CO_NMT_MASTER_STATE_T mState = CO_NMT_MASTER_STATE_SLAVE;
# endif /* CO_NMT_MASTER */
#endif /* CO_EVENT_SLEEP */

	nmtState = CO_NMT_STATE_RESET_NODE;

	(void)icoCobSet(nmtCob, 0u, CO_COB_RTR_NONE, 2u);

#ifdef CO_EVENT_SLEEP
# ifdef CO_NMT_MASTER
	mState = icoNmtMasterGetState();

	if (mState == CO_NMT_MASTER_STATE_MASTER)  {
		icoSleepReset(1u, 0u);
	} else
# endif /* CO_NMT_MASTER */
	{
		icoSleepReset(0u, 0u);
	}
#endif /* CO_EVENT_SLEEP */
}


/***************************************************************************/
/**
*
* \brief coNmtLocalStateReq - request local NMT state change
*
* Be carfule - NMT commands should be generated only by the master
*
* \return RET_T
*
*/
RET_T coNmtLocalStateReq(
        CO_NMT_STATE_T	reqState		/**< new requested state */
	)
{
CO_REC_DATA_T	nmtRec;
UNSIGNED8		cmd;

	switch (reqState)  {
		case CO_NMT_STATE_OPERATIONAL:
			cmd = 1u;
			break;
		case CO_NMT_STATE_STOPPED:
			cmd = 2u;
			break;
		case CO_NMT_STATE_PREOP:
			cmd = 128u;
			break;
		case CO_NMT_STATE_RESET_NODE:
			cmd = 129u;
			break;
		case CO_NMT_STATE_RESET_COMM:
			cmd = 130u;
			break;
		case CO_NMT_STATE_UNKNOWN:
		default:
			return(RET_INVALID_PARAMETER);
	}

	/* simulate command for own node */
	nmtRec.msg.canCob.canId = 0u;
	nmtRec.msg.len = 2u;
	nmtRec.msg.data[0] = cmd;
	nmtRec.msg.data[1] = coNmtGetNodeId();
	icoNmtMsgHandler(&nmtRec);

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoNmtVarInit - init nmt variables
*
* If node id-function is null, fixed node id from pNodeId is used
*
* \return none
*
*/
void icoNmtVarInit(
		const UNSIGNED8		*pNodeId,
		const CO_NODE_ID_T	*nodeIdFct
	)
{

	{
		nmtCob = 0xffffu;
		persistantNodeId = pNodeId[0];/*lint !e960 Note pointer arithmetic other than array indexing used */
		nodeIdFunction = nodeIdFct[0];/*lint !e960 Note pointer arithmetic other than array indexing used */
		nodeId = 127u;
		nmtState = CO_NMT_STATE_RESET_NODE;
	}

#ifdef CO_EVENT_NMT_CNT
	nmtTableCnt = 0u;
#endif /* CO_EVENT_NMT */
}


/***************************************************************************/
/**
* \brief coInitNmt - init NMT functionality
*
* This function initializes the NMT functionality
* and calls an internal reset communication.
*
* If parameter master is unequal 0 the node will be initialized as NMT master.
* If flying master is enabled, the decision for master/slave or flying master
* is done by check object 0x1f80. (parameter master is not used!)
* In this case, the node starts as slave and wait for the master negotiation.
*
* \return RET_T
*
*/
RET_T coNmtInit(
		UNSIGNED8		master			/**< master mode */
	)
{
CO_NMT_MASTER_STATE_T mState;
#ifdef CO_FLYING_MASTER_SUPPORTED
RET_T	retVal;

	retVal = icoNmtFlymaInit();
	if (retVal != RET_OK)  {
		return(retVal);
	}
	mState = CO_NMT_MASTER_STATE_POWERON;
#else /* CO_FLYING_MASTER_SUPPORTED */
	if (master != 0u)  {
		mState = CO_NMT_MASTER_STATE_MASTER;
	} else {
		mState = CO_NMT_MASTER_STATE_SLAVE;
	}
#endif /* CO_FLYING_MASTER_SUPPORTED */


	if (mState == CO_NMT_MASTER_STATE_MASTER)  {
		nmtCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_NMT, 0u);
	} else {
		nmtCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_NMT, 0u);
	}
	if (nmtCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

#ifdef CO_NMT_MASTER
	icoNmtMasterSetState(mState);
#endif /* CO_NMT_MASTER */

#ifdef CO_EVENT_SLEEP
	if (icoSleepInit(master) != RET_OK)  {
		return(RET_NO_COB_AVAILABLE);
	}
#endif /* CO_EVENT_SLEEP */

	nmtResetAppl();
	(void)nmtResetComm();

	return(RET_OK);
}
