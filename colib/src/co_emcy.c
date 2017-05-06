/*
* co_emcy.c - contains emcy routines
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_emcy.c 15802 2016-10-28 09:12:24Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/


/********************************************************************/
/**
* \brief Emergency handling
*
* \file co_emcy.c
* contains emcy routines
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>	/*lint !e766 depends on configuration */

#include <co_datatype.h>
#include <co_emcy.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_odaccess.h>
#include <co_nmt.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_emcy.h"
#ifdef CO_EMCY_NETWORKING
# include "ico_network.h"
#endif /* CO_EMCY_NETWORKING */

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_EMCY
# ifdef CO_EVENT_PROFILE_EMCY
#  define CO_EVENT_EMCY_CNT	(CO_EVENT_DYNAMIC_EMCY + CO_EVENT_PROFILE_EMCY)
# else /* CO_EVENT_PROFILE_EMCY */
#  define CO_EVENT_EMCY_CNT	(CO_EVENT_DYNAMIC_EMCY)
# endif /* CO_EVENT_PROFILE_EMCY */
#else /* CO_EVENT_DYNAMIC_EMCY */
# ifdef CO_EVENT_PROFILE_EMCY
#  define CO_EVENT_EMCY_CNT	(CO_EVENT_PROFILE_EMCY)
# endif /* CO_EVENT_PROFILE_EMCY */
#endif /* CO_EVENT_DYNAMIC_EMCY */


#ifdef CO_EVENT_DYNAMIC_EMCY_CONSUMER
# ifdef CO_EVENT_PROFILE_EMCY_CONSUMER
#  define CO_EVENT_EMCY_CONSUMER_CNT	(CO_EVENT_DYNAMIC_EMCY_CONSUMER + CO_EVENT_PROFILE_EMCY_CONSUMER)
# else /* CO_EVENT_PROFILE_EMCY_CONSUMER */
#  define CO_EVENT_EMCY_CONSUMER_CNT	(CO_EVENT_DYNAMIC_EMCY_CONSUMER)
# endif /* CO_EVENT_PROFILE_EMCY_CONSUMER */
#else /* CO_EVENT_DYNAMIC_EMCY_CONSUMER */
# ifdef CO_EVENT_PROFILE_EMCY_CONSUMER
#  define CO_EVENT_EMCY_CONSUMER_CNT	(CO_EVENT_PROFILE_EMCY_CONSUMER)
# endif /* CO_EVENT_PROFILE_EMCY_CONSUMER */
#endif /* CO_EVENT_DYNAMIC_EMCY_CONSUMER */


/* local defined data types
---------------------------------------------------------------------------*/
#ifdef CO_EMCY_CONSUMER_CNT
typedef struct {
	UNSIGNED32 cobId;
	COB_REFERENZ_T	cob;
# ifdef CO_EMCY_NETWORKING
	UNSIGNED32	routingList;
# endif /* CO_EMCY_NETWORKING */
} CO_EMCY_CONSUMER_T;
#endif /* CO_EMCY_CONSUMER_CNT */


/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_EMCY
extern CO_CONST CO_EVENT_EMCY_T coEventEmcyInd;
#endif /* CO_EVENT_STATIC_EMCY */

#ifdef CO_EVENT_STATIC_EMCY_CONSUMER
extern CO_CONST CO_EVENT_EMCY_CONS_T coEventEmcyConsumerInd;
#endif /* CO_EVENT_STATIC_EMCY_CONSUMER */


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
#ifdef CO_EMCY_PRODUCER
static UNSIGNED32 emcyCobId = { 0ul };
static COB_REFERENZ_T	emcyCob;
# ifdef CO_EMCY_ERROR_HISTORY
static UNSIGNED32	errorHistory[CO_EMCY_ERROR_HISTORY];
static UNSIGNED8	errorHistoryCnt = { 0u };
# endif /* CO_EMCY_ERROR_HISTORY */
static UNSIGNED16	inhibit = { 0u };
# ifdef CO_EVENT_EMCY_CNT
static CO_EVENT_EMCY_T	emcyTable[CO_EVENT_EMCY_CNT];
static UNSIGNED8		emcyTableCnt = 0u;
# endif /* CO_EVENT_EMCY_CNT */
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
static UNSIGNED8	emcyConsumerCnt = { 0u };
static CO_EMCY_CONSUMER_T	emcyCons[CO_EMCY_CONSUMER_CNT];
# ifdef CO_EVENT_EMCY_CONSUMER_CNT
static CO_EVENT_EMCY_CONS_T	emcyConsTable[CO_EVENT_EMCY_CONSUMER_CNT];
static UNSIGNED8		emcyConsTableCnt = 0u;
# endif /* CO_EVENT_EMCY_CONSUMER_CNT */
#endif /* CO_EMCY_CONSUMER_CNT */



#ifdef CO_EMCY_PRODUCER
/***************************************************************************/
/**
* \brief coEmcyWriteReq - write an emergency message
*
* With this function, an emergency message can be send.<br>
* The message is automatically composed and transmitted by the given parameter 
* and the error register value (index 0x1001:0).
* After that, the error history (index 0x1003:n) is updated
* with the new data.<br>
* The parameter pData can be NULL,
* if no application specific data should be sent.<br>
*
* The error register (index 0x1001:0) has to be updated by the application.
*
* \return RET_T
*
*/
RET_T coEmcyWriteReq(
		UNSIGNED16	emcyErrCode,		/**< emergency error code */
		CO_CONST UNSIGNED8	pData[]		/**< pointer to additional 5 data bytes */
	)
{
RET_T	retVal;
CO_TRANS_DATA_T	trData;
CO_NMT_STATE_T	nmtState;
UNSIGNED8	i;
#ifdef CO_EMCY_ERROR_HISTORY
UNSIGNED8	cnt;
UNSIGNED32	u32;
#endif /* CO_EMCY_ERROR_HISTORY */
UNSIGNED8	*pErrorRegister;

	pErrorRegister = coOdGetObjAddr(0x1001u, 0u);
	if (pErrorRegister != NULL)  {

		/* handling of generic error bit */
		/* set it if one of the other bits are set */
		if ((*pErrorRegister & 0xfeu) == 0u)  {
			*pErrorRegister = 0u;
		} else {
			*pErrorRegister |= 1u;
		}
	}

	/* OPERATIONAL ? */
	nmtState = coNmtGetState();
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return(RET_INVALID_NMT_STATE);
	}

	trData.data[0] = (UNSIGNED8)(emcyErrCode & 0xffu);
	trData.data[1] = (UNSIGNED8)(emcyErrCode >> 8u);
	if (pErrorRegister != NULL)  {
		trData.data[2] = *pErrorRegister;
	} else {
		trData.data[2] = 0u;
	}

	/* set generic error bit temporyry if error code != 0x00xx */
	if ((emcyErrCode & 0xff00u) != 0u)  {
		trData.data[2] |= 1u;
	}

	/* allow to get NULL as pointer of manufacturer data */
	if (pData == NULL)  {
		for (i = 0u; i < 5u; i++)  {
			trData.data[3u + i] = 0u;
		}
	} else {
		for (i = 0u; i < 5u; i++)  {
			trData.data[3u + i] = pData[i];
		}
	}

	/* transmit data */
	retVal = icoTransmitMessage(emcyCob, &trData, 0u);

#ifdef CO_EMCY_ERROR_HISTORY
	if (emcyErrCode != 0x0000u)  {
		/* save at error history */
		cnt = errorHistoryCnt;
		if (errorHistoryCnt >= CO_EMCY_ERROR_HISTORY)  {
			cnt--;
		}
		while (cnt > 0u)  {
			/* access to out-of-bounds not possible while cnt > 0 */
			if (cnt < CO_EMCY_ERROR_HISTORY)  {
				errorHistory[cnt] =
					errorHistory[cnt - 1u]; /*lint !e661 out-of-bound ensured */
			}
			cnt--;
		} 
		if (pData == NULL)  {
			u32 = (UNSIGNED32)emcyErrCode;
		} else {
			u32 = (UNSIGNED32)emcyErrCode | ((UNSIGNED32)pData[1u] << 16u) | ((UNSIGNED32)pData[0u] << 24u);
		}
		errorHistory[0u] = u32;
		if (errorHistoryCnt < CO_EMCY_ERROR_HISTORY)  {
			errorHistoryCnt++;
		}
	}
#endif /* CO_EMCY_ERROR_HISTORY */

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoEmcyWriteReq - internal write of an emergency message
*
* This function writes an emergency message.
*
* \return RET_T
*
*/
RET_T icoEmcyWriteReq(
		UNSIGNED16	emcyErrCode,	/* emergency error code */
		const UNSIGNED8	*pAddErrorBytes	/* pointer to additional 5 data bytes */
	)
{
RET_T	retVal;

#ifdef CO_EVENT_EMCY_CNT
UNSIGNED8	cnt;

	/* call all user indications */
	cnt = emcyTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		retVal = emcyTable[cnt](emcyErrCode, pAddErrorBytes);
		if (retVal != RET_OK)  {
			return(RET_INTERNAL_ERROR);
		}
	}
#endif /* CO_EVENT_EMCY_CNT */

#ifdef CO_EVENT_STATIC_EMCY
	coEventEmcyInd(emcyErrCode, pAddErrorBytes);
#endif /* CO_EVENT_STATIC_EMCY */

	retVal = coEmcyWriteReq(emcyErrCode, pAddErrorBytes);

	return(retVal);
}


# ifdef CO_EVENT_EMCY_CNT
/***************************************************************************/
/**
* \brief coEventRegister_EMCY - register emergency event function
*
* This function registers an emergency indication function.
*
* \return RET_T
*
*/
RET_T coEventRegister_EMCY(
		CO_EVENT_EMCY_T pFunction
	)
{
	if (emcyTableCnt >= CO_EVENT_EMCY_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	emcyTable[emcyTableCnt] = pFunction;	/* save function pointer */
	emcyTableCnt++;

	return(RET_OK);
}
# endif /* CO_EVENT_EMCY_CNT */
#endif /* CO_EMCY_PRODUCER */


#ifdef CO_EMCY_CONSUMER_CNT
/***************************************************************************/
/*
* \brief icoEmcyConsumerHandler
*
* This function registers an emergency indication function.
*
* \return RET_T
*
*/
void icoEmcyConsumerHandler(
		const CO_REC_DATA_T	*pRecData
	)
{
# ifdef CO_EVENT_EMCY_CONSUMER_CNT
UNSIGNED8	cnt;
# endif /* CO_EVENT_EMCY_CONSUMER_CNT */
# ifdef CO_EMCY_NETWORKING
UNSIGNED8	emcyBuf[5];
UNSIGNED32	mask;
UNSIGNED8	i;
# endif /* CO_EMCY_NETWORKING */

	/* check for correct message len */
	if (pRecData->msg.len != 8u)  {
		return;
	}

	if (pRecData->spec > emcyConsumerCnt)  {
		return;
	}

# ifdef CO_EVENT_EMCY_CONSUMER_CNT
	/* call all user indications */
	cnt = emcyConsTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		emcyConsTable[cnt]((UNSIGNED8)(pRecData->msg.canCob.canId & 0x7fu),
			pRecData->msg.data[0]
				| (UNSIGNED16)((UNSIGNED16)pRecData->msg.data[1] << 8u),
			pRecData->msg.data[2],
			&pRecData->msg.data[3]);
	}
# endif /* CO_EVENT_EMCY_CONSUMER_CNT */

# ifdef CO_EVENT_STATIC_EMCY_CONSUMER
		coEventEmcyConsumerInd((UNSIGNED8)(pRecData->msg.canCob.canId & 0x7fu),
			pRecData->msg.data[0]
				| (UNSIGNED16)((UNSIGNED16)pRecData->msg.data[1] << 8u),
			pRecData->msg.data[2],
			&pRecData->msg.data[3]);
# endif /* CO_EVENT_STATIC_EMCY_CONSUMER */

# ifdef CO_EMCY_NETWORKING
	mask = emcyCons[pRecData->spec].routingList;
	if (mask != 0u) {
		for (i = 0u; i < CO_MULTI_LINE_CNT; i++)  {
			if ((mask & (1ul << i)) != 0u)  {
				/* forward emcy */
				UNSIGNED16	nwId = icoNetworkLocalId();
				emcyBuf[0] = (UNSIGNED8)(nwId & 0xffu);
				emcyBuf[1] = (UNSIGNED8)(nwId >> 8u);
				emcyBuf[2] = (UNSIGNED8)(pRecData->msg.canCob.canId & 0x7fu);
				emcyBuf[3] = pRecData->msg.data[0];
				emcyBuf[4] = pRecData->msg.data[1];
				(void)icoEmcyWriteReq(0xf000u, &emcyBuf[0]);
			}
		}
	}
# endif /* CO_EMCY_NETWORKING */
}


# ifdef CO_EVENT_EMCY_CONSUMER_CNT
/***************************************************************************/
/**
* \brief coEventRegister_EMCY_CONSUMER - register emergency consumer event function
*
* This function registers an emergency consumer indication function.
*
* \return RET_T
*
*/
RET_T coEventRegister_EMCY_CONSUMER(
		CO_EVENT_EMCY_CONS_T pFunction
	)
{
	if (emcyConsTableCnt >= CO_EVENT_EMCY_CONSUMER_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	emcyConsTable[emcyConsTableCnt] = pFunction;	/* save function pointer */
	emcyConsTableCnt++;

	return(RET_OK);
}
# endif /* CO_EVENT_EMCY_CONSUMER_CNT */
#endif /* CO_EMCY_CONSUMER_CNT */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief icoEmcyGetObjectAddr - get Emcy object address
*
*
* \return RET_T
*
*/
void *icoEmcyGetObjectAddr(
		UNSIGNED16	idx,				/* index */
		UNSIGNED8	subIndex			/* subindex */
	)
{
void	*pAddr = NULL;
(void)subIndex;

	switch (idx) {

#ifdef CO_EMCY_PRODUCER
# ifdef CO_EMCY_ERROR_HISTORY
		case 0x1003u:		/* error history */
			if (subIndex == 0u)  {
				pAddr = &errorHistoryCnt;
			} else {
				if (subIndex > errorHistoryCnt)  {
					pAddr = NULL;
				} else {
					pAddr = &errorHistory[subIndex - 1u];
				}
			}
			break;
# endif /* CO_EMCY_ERROR_HISTORY */
		case 0x1014u:	/* cob-id emcy producer */
			pAddr = &emcyCobId;
			break;
		case 0x1015u:	/* inhibit */
			pAddr = &inhibit;
			break;
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
		case 0x1028u:	/* emcy consumer */
			pAddr = (void *)&emcyCons[subIndex - 1u].cobId;
			break;
#endif /* CO_EMCY_CONSUMER_CNT */

#ifdef CO_EMCY_NETWORKING
		case 0x1f2fu:	/* emcy routing list */
			pAddr = (void *)&emcyCons[subIndex - 1u].routingList;
			break;
# endif /* CO_EMCY_NETWORKING */

		default:
			break;
	}

	return(pAddr);
}


#if defined(CO_EMCY_PRODUCER) || defined (CO_EMCY_CONSUMER_CNT)
/***************************************************************************/
/**
* \internal
*
* \brief icoEmcyCheckObjLimitCobId - check cob-id
*
*
* \return RET_T
*
*/
RET_T icoEmcyCheckObjLimitCobid(
		UNSIGNED16	index,		/* index */
		UNSIGNED8	subIndex,	/* subIndex */
		UNSIGNED32	canId		/* new cob ID*/
	)
{
RET_T	retVal = RET_OK;
(void)subIndex;

#ifdef CO_EMCY_PRODUCER
	if (index == 0x1014u)  {
		/* write the same value again is allowed */
		if (canId != emcyCobId)  {
			if ((canId & CO_COB_VALID_MASK) == 0u)  {
				/* new cobid is valid, only allowed if cob was disabled before*/
				if ((emcyCobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
					return(RET_SDO_INVALID_VALUE);
				}
			}
			if (icoCheckRestrictedCobs(canId, 0x81u, 0x81u) == CO_TRUE)  {
				return(RET_SDO_INVALID_VALUE);
			}
		}
	}
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
	if (index == 0x1028u)  {
		if ((canId & CO_COB_VALID_MASK) == 0u)  {
			/* new cobid is valid, only allowed if cob was disabled before*/
			if ((emcyCons[subIndex - 1u].cobId & CO_COB_VALID_MASK)
					!= CO_COB_INVALID)  {
				return(RET_SDO_INVALID_VALUE);
			}
			if (icoCheckRestrictedCobs(canId, 0x81u, 0x81u) == CO_TRUE)  {
				return(RET_SDO_INVALID_VALUE);
			}
		}
	}
#endif /* CO_EMCY_CONSUMER_CNT */

	return(retVal);
}



/***************************************************************************/
/**
* \internal
*
* \brief icoEmcyObjChanged - EMCY object was changed
*
*
* \return RET_T
*
*/
RET_T icoEmcyObjChanged(
		UNSIGNED16	index,			/* object index */
		UNSIGNED8	subIndex		/* object subIndex */
	)
{
RET_T	retVal = RET_OK;
#ifdef CO_EMCY_CONSUMER_CNT
UNSIGNED8	idx;
#endif /* CO_EMCY_CONSUMER_CNT */
(void)subIndex;

	switch (index)  {
#ifdef CO_EMCY_PRODUCER
# ifdef CO_EMCY_ERROR_HISTORY
		case 0x1003u:
			errorHistoryCnt = 0u;
			break;
# endif /* CO_EMCY_ERROR_HISTORY */
		case 0x1014u:
			retVal = icoCobSet(emcyCob, emcyCobId, CO_COB_RTR_NONE, 8u);
			break;
		case 0x1015u:	/* inhibit */
			retVal = icoCobSetInhibit(emcyCob, inhibit);
			break;
#endif /* CO_EMCY_PRODUCER */

#ifdef CO_EMCY_CONSUMER_CNT
		case 0x1028u:	/* emcy consumer */
			idx = subIndex - 1u;
			(void)icoCobSet(emcyCons[idx].cob,
					emcyCons[idx].cobId, CO_COB_RTR_NONE, 8u);
			break;
#endif /* CO_EMCY_CONSUMER_CNT */
		default:
			break;
	}

	return(retVal);
}
#endif /* defined(CO_EMCY_PRODUCER) || defined (CO_EMCY_CONSUMER_CNT) */


#ifdef CO_EMCY_PRODUCER
/***************************************************************************/
/**
* \internal
*
* \brief icoEmcyCheckObjLimitHist - check object limits u8 value
*
*
* \return RET_T
*
*/
RET_T icoEmcyCheckObjLimitHist(
		UNSIGNED16	index,			/* object index */
		UNSIGNED8	subIndex,	/* subindex */
		UNSIGNED8	u8			/* received data */
	)
{
RET_T	retVal = RET_OK;

	/* 0x1001 */
	if (index == 0x1001u)  {
		/* all values allowed */
	} else {
		/* index 1003 */
		if ((subIndex != 0u) || (u8 != 0u))  {
			retVal = RET_SDO_INVALID_VALUE;
		}
	}
	return(retVal);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief coEmcyReset - reset comm for Emcy
*
* \return none
*/
void icoEmcyReset(
		void	/* no parameter */
	)
{
	(void)icoCobSet(emcyCob, emcyCobId, CO_COB_RTR_NONE, 8u);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief coEmcySetDefaultValue - set default (predefined connection set)
*
* \return none
*/
void icoEmcyProducerSetDefaultValue(
		void	/* no parameter */
	)
{
RET_T	retVal;
UNSIGNED16	u16;
UNSIGNED32	u32;

	emcyCobId = 0x80ul + coNmtGetNodeId();

	retVal = coOdGetDefaultVal_u32(0x1014u, 0u, &u32);
	if (retVal == RET_OK)  {
		if ((u32 & CO_COB_VALID_MASK) != 0u)  {
			/* cob invalid */
			emcyCobId |= CO_COB_INVALID;
		}
	}
	retVal = coOdGetDefaultVal_u16(0x1015u, 0u, &u16);
	if (retVal == RET_OK)  {
		inhibit = u16;
		(void)icoCobSetInhibit(emcyCob, inhibit);
	}

	return;
}


/***************************************************************************/
/**
* \brief coEmcyProducerInit - initialization for emergency producer
*
* This function initializes the emergency producer functionality.
*
* \return RET_T
*
*/
RET_T coEmcyProducerInit(
		void	/* no parameter */
	)
{
	emcyCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_EMCY_TRANSMIT, 0u);
	if (emcyCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	return(RET_OK);
}

#endif /* CO_EMCY_PRODUCER */


/***************************************************************************/
/*
* \brief icoEmcyVarInit - init emcy variables
*
*/
void icoEmcyVarInit(
		UNSIGNED8	*pEmcyErrHistList,	/* line counts */
		UNSIGNED8	*pEmcyConsList		/* line counts */
	)
{
#ifdef CO_EMCY_CONSUMER_CNT
UNSIGNED16		i;
#endif /* CO_EMCY_CONSUMER_CNT */
#if defined(CO_EMCY_CONSUMER_CNT) || defined(CO_EMCY_PRODUCER)
#endif /* defined(CO_EMCY_CONSUMER_CNT) || (defined(CO_EMCY_PRODUCER) */
#if defined(CO_EMCY_ERROR_HISTORY) || defined(CO_EMCY_CONSUMER_CNT)
#endif /* defined(CO_EMCY_ERROR_HISTORY) || defined(CO_EMCY_CONSUMER_CNT) */
(void)pEmcyErrHistList;
(void)pEmcyConsList;

#ifdef CO_EMCY_PRODUCER
	{
		emcyCobId = 0ul;
		emcyCob = 0xffffu;
		inhibit = 0u;
# ifdef CO_EMCY_ERROR_HISTORY
		errorHistoryCnt = 0u;
# endif /* CO_EMCY_ERROR_HISTORY */
	}

# ifdef CO_EVENT_EMCY_CNT
	emcyTableCnt = 0u;
# endif /* CO_EVENT_EMCY_CNT */
#endif /* CO_EMCY_PRODUCER */


#ifdef CO_EMCY_CONSUMER_CNT

	{
		emcyConsumerCnt = 0u;
	}

	/* reset cobs */
	for (i = 0u; i < CO_EMCY_CONSUMER_CNT; i++)  {
		emcyCons[i].cob = 0xffffu;
	}

# ifdef CO_EVENT_EMCY_CONSUMER_CNT
	emcyConsTableCnt = 0u;
# endif /* CO_EVENT_EMCY_CONSUMER_CNT */
#endif /* CO_EMCY_CONSUMER_CNT */
}


#ifdef CO_EMCY_CONSUMER_CNT
/***************************************************************************/
/**
* \internal
*
* \brief coEmcyConsumerReset - reset comm for Emcy
*
* \return none
*/
void icoEmcyConsumerReset(
		void	/* no parameter */
	)
{
UNSIGNED8	i;
UNSIGNED16	idx;

	for (i = 0u; i < emcyConsumerCnt; i++)  {
		idx = i;
		(void)icoCobSet(emcyCons[idx].cob, emcyCons[idx].cobId, CO_COB_RTR_NONE, 8u);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief coEmcyConsumerSetDefaultValue - reset comm for Emcy
*
* \return none
*/
void icoEmcyConsumerSetDefaultValue(
		void	/* no parameter */
	)
{
RET_T	retVal;
UNSIGNED8	i;
UNSIGNED16	idx;

	for (i = 0u; i < emcyConsumerCnt; i++)  {
		idx = i;
		retVal = coOdGetDefaultVal_u32(0x1028u, i + 1u, &emcyCons[idx].cobId);
		if (retVal != RET_OK)  {
			emcyCons[idx].cobId |= CO_COB_INVALID;
		}

#ifdef CO_EMCY_NETWORKING
		(void)coOdGetDefaultVal_u32(0x1f2fu, i + 1u,
			&emcyCons[idx].routingList);
#endif /* CO_EMCY_NETWORKING */
	}
}


/***************************************************************************/
/**
* \brief coEmcyConsumerInit - initialization for emergency consumer
*
* This function initializes the emergency consumers.
*
* \return RET_T
*
*/
RET_T coEmcyConsumerInit(
		UNSIGNED8		emcyCnt		/**< number of emergency consumers */
	)
{
UNSIGNED8	cnt;
UNSIGNED8	idx;

	if (emcyCnt > CO_EMCY_CONSUMER_CNT)  {
		return(RET_EVENT_NO_RESSOURCE);
	}
 
	for (cnt = 0u; cnt < emcyCnt; cnt++)  {
		idx = cnt;
		emcyCons[idx].cob = icoCobCreate(CO_COB_TYPE_RECEIVE,
				CO_SERVICE_EMCY_RECEIVE, (UNSIGNED16)cnt);
		if (emcyCons[idx].cob == 0xffffu)  {
			return(RET_NO_COB_AVAILABLE);
		}
	}

	emcyConsumerCnt = emcyCnt;

	return(RET_OK);
}
#endif /* CO_EMCY_CONSUMER_CNT */
