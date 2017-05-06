/*
* co_pdo.c - contains pdo routines
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_pdo.c 15327 2016-09-23 09:00:10Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief PDO transmission and reception routines
*
* \file
* contains PDO handling routines.
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)

#include <co_datatype.h>
#include <co_timer.h>
#include <co_emcy.h>
#include <co_drv.h>
#include <co_odaccess.h>
#include <co_nmt.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_emcy.h"
#include "ico_odaccess.h"
#include "ico_pdo.h"
#include "ico_nmt.h"


/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_PDO_RECEIVE_CNT

#ifdef CO_EVENT_DYNAMIC_PDO
# ifdef CO_EVENT_PROFILE_PDO
#  define CO_EVENT_PDO_CNT	(CO_EVENT_DYNAMIC_PDO + CO_EVENT_PROFILE_PDO)
# else /* CO_EVENT_PROFILE_PDO */
#  define CO_EVENT_PDO_CNT	(CO_EVENT_DYNAMIC_PDO)
# endif /* CO_EVENT_PROFILE_PDO */
#else /* CO_EVENT_DYNAMIC_PDO */
# ifdef CO_EVENT_PROFILE_PDO
#  define CO_EVENT_PDO_CNT	(CO_EVENT_PROFILE_PDO)
# endif /* CO_EVENT_PROFILE_PDO */
#endif /* CO_EVENT_DYNAMIC_PDO */

#if defined(CO_EVENT_STATIC_PDO) || defined(CO_EVENT_PDO_CNT)
# define CO_EVENT_PDO   1
#endif /* defined(CO_EVENT_STATIC_PDO) || defined(CO_EVENT_PDO_CNT) */

#endif /* CO_PDO_RECEIVE_CNT */


#define CO_COB_RTR_BIT		0x40000000UL


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_PDO_RECEIVE_CNT
# ifdef CO_EVENT_STATIC_PDO
extern CO_CONST CO_EVENT_PDO_T coEventPdoInd;
extern CO_CONST CO_EVENT_PDO_T coEventPdoRecInd;
#  ifdef CO_SYNC_SUPPORTED
extern CO_CONST CO_EVENT_PDO_T coEventPdoSyncInd;
#  endif /* CO_SYNC_SUPPORTED */
# endif /* CO_EVENT_STATIC_PDO */
#endif /* CO_PDO_RECEIVE_CNT */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
#ifdef CO_PDO_RECEIVE_CNT
static CO_INLINE CO_REC_PDO_T *searchRecPdo(UNSIGNED16 pdoNr);
static UNSIGNED8 checkRecPdoMappingTable(
		CO_REC_PDO_T *pPdo, UNSIGNED8 mapCnt);
static RET_T pdoReceiveData(UNSIGNED16 pdoIdx, CO_CONST UNSIGNED8 *pData);
static void stopRecPdoTimer(const CO_REC_PDO_T *pPdo);
static void pdoReceiveEventTimer(void *ptr);
# ifdef CO_EVENT_PDO
static void pdoInd(UNSIGNED16 pdoNr, UNSIGNED8 transType);
static void pdoRecEventInd(UNSIGNED16 pdoNr);
# endif /*  CO_EVENT_PDO */
#endif /* CO_PDO_RECEIVE_CNT */
#ifdef CO_PDO_TRANSMIT_CNT
static UNSIGNED8 checkTrPdoMappingTable(CO_TR_PDO_T *pPdo,
		UNSIGNED8 mapCnt);
static RET_T setupTrPdoTimer(CO_TR_PDO_T *pPdo);
static void pdoEventTimer(void *ptr);
static RET_T pdoTransmitData(CO_CONST CO_TR_PDO_T *pPdo, UNSIGNED8 flags);
#endif /* CO_PDO_TRANSMIT_CNT */
#ifdef CO_TR_PDO_DYN_MAP_ENTRIES
static UNSIGNED16 createPdoTrMapTable(UNSIGNED16 index);
static RET_T setupPdoTrMapTable(UNSIGNED16 mapTableIdx, UNSIGNED16 index);
#endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
#ifdef CO_REC_PDO_DYN_MAP_ENTRIES
static UNSIGNED16 createPdoRecMapTable(UNSIGNED16 index);
static RET_T setupPdoRecMapTable(UNSIGNED16 mapTableIdx, UNSIGNED16 index);
#endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
static RET_T setPdoCob(COB_REFERENZ_T cobRef, UNSIGNED32 cobId, UNSIGNED8 len);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
#ifdef CO_PDO_TRANSMIT_CNT
static CO_TR_PDO_T	pdoTransmit[CO_PDO_TRANSMIT_CNT];
static UNSIGNED16	pdoTrCnt = { 0u };
#endif /* CO_PDO_TRANSMIT_CNT */

#ifdef CO_PDO_RECEIVE_CNT
static CO_REC_PDO_T	pdoReceive[CO_PDO_RECEIVE_CNT];
static UNSIGNED16	pdoRecCnt = { 0u };
# ifdef CO_EVENT_PDO_CNT
static CO_EVENT_PDO_T pdoEventTable[CO_EVENT_PDO_CNT];
static UNSIGNED16	pdoEventTableCnt = 0u;
static CO_EVENT_PDO_T pdoRecEventTable[CO_EVENT_PDO_CNT];
static UNSIGNED16	pdoRecEventTableCnt = 0u;
#  ifdef CO_SYNC_SUPPORTED
static CO_EVENT_PDO_T pdoEventSyncTable[CO_EVENT_PDO_CNT];
static UNSIGNED16	pdoEventSyncTableCnt = 0u;
#  endif /* CO_SYNC_SUPPORTED */
# endif /*  CO_EVENT_PDO_CNT */
#endif /* CO_PDO_RECEIVE_CNT */

/* Mapping tables TPDO */
#ifdef CO_TR_PDO_DYN_MAP_ENTRIES
static PDO_TR_MAP_TABLE_T mapTablesTrPDO[CO_TR_PDO_DYN_MAP_ENTRIES];
static UNSIGNED16	mapTableTrPdoCnt = 0u;
#endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
#ifdef CO_REC_PDO_DYN_MAP_ENTRIES
static PDO_REC_MAP_TABLE_T mapTablesRecPDO[CO_REC_PDO_DYN_MAP_ENTRIES];
static UNSIGNED16	mapTableRecPdoCnt = 0u;
#endif /* CO_REC_PDO_DYN_MAP_ENTRIES */




#ifdef CO_PDO_TRANSMIT_CNT
/***************************************************************************/
/**
* \brief coPdoReqNr - request PDO transmission by PDO number
*
* This function requests the transmission of an PDO given by its number.<br>
* All mapped objects are automatically copied into the CAN message.
* If the inhibit time is not active,
* then the message is transmitted immediately.
*
* If the inhibit time is not ellapsed yet,
* the transmission depends on the parameter flags:
*
* 0 - PDO will be transmitted after inhibit is ellapsed
* 	(if data are not changed, PDO will not be transmitted more than once!)
* MSG_OVERWRITE - if the last PDO is not transmitted yet,
*	overwrite the last data with the new data
* MSG_RET_INHIBIT - return the function with RET_INHIBIT_ACTIVE,
*	if the inhibit is not ellapsed yet
*
* with the same or 
* \return RET_T
* \retval RET_INVALID_NMT_STATE
*	invalid NMT state
* \retval RET_INVALID_PARAMETER
*	unknown PDO number
* \retval RET_COB_DISABLED
*	PDO is disabled
* \retval RET_INHIBIT_ACTIVE
*	inhibit time is not yet ellapsed
* \retval RET_OK
*	all function are ok, but have not to be transmitted yet
*
*/
RET_T coPdoReqNr(
		UNSIGNED16		pdoNr,		/**< PDO number */
		UNSIGNED8		flags		/**< transmit flags */
	)
{
RET_T	retVal = RET_OK;
CO_TR_PDO_T *pPdo;

	/* OPERATIONAL ? */
	if (coNmtGetState() != CO_NMT_STATE_OPERATIONAL)  {
		return(RET_INVALID_NMT_STATE);
	}

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_INVALID_PARAMETER);
	}

	/* check, if PDO is not disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) == CO_COB_INVALID)  {
		return(RET_COB_DISABLED);
	}

	/* synchronous PDO ? */
	if (pPdo->transType <= 240u)  {
		if (pPdo->transType == 0u)  {
			pPdo->state = CO_PDO_STATE_TO_TRANSMIT;
		}
		return(retVal);
	}

	/* only RTR ? */
	if (pPdo->transType == 253u)  {
		return(RET_PARAMETER_INCOMPATIBLE);
	}

	retVal = pdoTransmitData(pPdo, flags);
	return(retVal);
}


/***************************************************************************/
/**
* \brief coPdoReqObj - request PDO transmission by object
*
* This function requests the transmission of the PDO,
* which the given object is mapped into.<br>
* All mapped objects are automatically copied into the CAN message.
* If the inhibit time is not active,
* then the message is transmitted immediately.
*
* If the inhibit time is not ellapsed yet,
* the transmission depends on the parameter flags:
*
* 0 - PDO will be transmitted after inhibit is ellapsed
* MSG_OVERWRITE - if the last PDO is not transmitted yet,
*	overwrite the last data with the new data
* MSG_RET_INHIBIT - return the function with RET_INHIBIT_ACTIVE,
*	if the inhibit is not ellapsed yet
*
* \return RET_T
* \retval RET_INVALID_NMT_STATE
*	invalid NMT state
* \retval RET_INVALID_PARAMETER
*	unknown PDO number
* \retval RET_COB_DISABLED
*	PDO is disabled
* \retval RET_INHIBIT_ACTIVE
*	inhibit time is not yet ellapsed
* \retval RET_OK
*	all function are ok, but have not to be transmitted yet
*
*/
RET_T coPdoReqObj(
		UNSIGNED16		index,		/**< index of mapped object */
		UNSIGNED8		subIndex,	/**< subindex of mapped object */
		UNSIGNED8		flags		/**< transmit flags */
	)
{
RET_T	retVal;
UNSIGNED16	i, m, idx;
CO_CONST CO_OBJECT_DESC_T	*pDesc;
UNSIGNED32		mapEntry;
UNSIGNED32		size;
UNSIGNED8		mapCnt;

	/* get object description and size */
	retVal = coOdGetObjDescPtr(index, subIndex, &pDesc);
	if (retVal != RET_OK)  {
		return(retVal);
	}

#ifdef CO_PDO_BIT_MAPPING
	if ((pDesc->dType == CO_DTYPE_BOOL_CONST)
	 || (pDesc->dType == CO_DTYPE_BOOL_VAR)
	 || (pDesc->dType == CO_DTYPE_BOOL_PTR))  {
		size = 1u;
	} else
#endif /* CO_PDO_BIT_MAPPING */
	{
		size = coOdGetObjSize(pDesc) << 3;
	}

	mapEntry = ((UNSIGNED32)index << 16) | ((UNSIGNED32)subIndex << 8) | size;

	/* for all PDOs */
	for (i = 0u; i < pdoTrCnt; i++)  {
		idx = i;
		mapCnt = pdoTransmit[idx].mapTableConst->mapCnt;
#ifdef CO_MPDO_PRODUCER
		/* MPDO ? */
		if (mapCnt == CO_MPDO_DAM_MODE)  {
			mapCnt = 0u;
		} else
		if (mapCnt == CO_MPDO_SAM_MODE)  {
			mapCnt = 0u;
		} else 
#endif /* CO_MPDO_PRODUCER */
		{
			/* for all map entries */
			for (m = 0u; m < mapCnt; m++)  {
				if (pdoTransmit[idx].mapTableConst->mapEntry[m].val == mapEntry)  {
					retVal = coPdoReqNr(pdoTransmit[idx].pdoNr, flags);
				}
			}
		}
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief pdoTransmitData - transmit data
*
* \return RET_T
*
*/
static RET_T pdoTransmitData(
		CO_CONST CO_TR_PDO_T		*pPdo,		/* pointer to pdo */
		UNSIGNED8		flags		/* flags */
	)
{
RET_T	retVal;
CO_TRANS_DATA_T	trData;
UNSIGNED8	cnt;
UNSIGNED8	offs;

#ifdef CO_MPDO_PRODUCER
	/* MPDO are not allowed to send with this function */
	if (pPdo->pdoType != CO_PDO_TYPE_STD)  {
		return(RET_PARAMETER_INCOMPATIBLE);
	}
#endif /* CO_MPDO_PRODUCER */

	memset(&trData, 0, sizeof(trData));

	offs = 0u;

	/* map pdo data */
	for (cnt = 0u; cnt < pPdo->mapTableConst->mapCnt; cnt++)  {
#ifdef CO_PDO_BIT_MAPPING
		/* byte order? */
		if (((offs % 8u) == 0u)
		 && ((pPdo->mapTableConst->mapEntry[cnt].len % 8u) == 0u))  {
			/* normal byte mapping */
			/* numeric value ? */
			coNumMemcpyUnpack(&trData.data[offs >> 3],
				pPdo->mapTableConst->mapEntry[cnt].pVar,
				(UNSIGNED32)pPdo->mapTableConst->mapEntry[cnt].len >> 3,
				(UNSIGNED16)pPdo->mapTableConst->mapEntry[cnt].numeric, 0u);

		} else {
			UNSIGNED8 byteLen, i;
			UNSIGNED8 tmpBuf[8];

			byteLen = (pPdo->mapTableConst->mapEntry[cnt].len + 7u) >> 3;
			/* copy object to tmpvar */
			coNumMemcpyUnpack(&tmpBuf[0],
				pPdo->mapTableConst->mapEntry[cnt].pVar,
				(UNSIGNED32)byteLen,
				(UNSIGNED16)pPdo->mapTableConst->mapEntry[cnt].numeric, 0u);

			/* bitvar ? */
			if (pPdo->mapTableConst->mapEntry[cnt].len == 1u)  {
				/* set bit at can buffer if variable is true */
				trData.data[(offs >> 3)] |= (tmpBuf[0] & 0x1u) << (offs & 7u);
			} else {
				/* move bytevar to can buffer */
				for (i = 0u; i < byteLen; i++)  {
					/* low part */
					trData.data[(offs >> 3) + i] |= (UNSIGNED8)(tmpBuf[i] << (offs & 7u));
					/* high part */
					trData.data[(offs >> 3) + i + 1u] =
						tmpBuf[i] >> (8u - (offs & 7u));
				}
			}
		}
#else /* CO_PDO_BIT_MAPPING */
		/* numeric value ? */
		coNumMemcpyUnpack(&trData.data[offs],
			pPdo->mapTableConst->mapEntry[cnt].pVar,
			(UNSIGNED32)pPdo->mapTableConst->mapEntry[cnt].len,
			(UNSIGNED16)pPdo->mapTableConst->mapEntry[cnt].numeric, 0u);
#endif /* CO_PDO_BIT_MAPPING */

		offs += pPdo->mapTableConst->mapEntry[cnt].len;
	}

	/* transmit data */
	retVal = icoTransmitMessage(pPdo->cob, &trData, flags);

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoRtrHandler - PDO RTR receive handler
*
*
* \return none
*
*/
void icoPdoRtrHandler(
		CO_CONST CO_REC_DATA_T	*pRecData		/* pointer to received data */
	)
{
CO_TR_PDO_T	*pPdo;

	/* OPERATIONAL ? */
	if (coNmtGetState() != CO_NMT_STATE_OPERATIONAL)  {
		return;
	}

	if (pRecData->spec > pdoTrCnt)  {
		return;
	}

	pPdo = &pdoTransmit[pRecData->spec];

	/* synchronous PDO ? */
	if (pPdo->transType <= 240u)  {
		if (pPdo->transType == 0u)  {
			pPdo->state = CO_PDO_STATE_TO_TRANSMIT;
		}
	} else {
		(void) pdoTransmitData(pPdo, 0u);
	}

	return;
}
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
/***************************************************************************/
/**
* \brief coPdoObjIsMapped - check, if object mapped to given PDO
*
* This function checks the PDO,
* if the given object is actual mapped to this PDO.
* (Only valid for receive PDOs)
*
* \return BOOL_T
* \retval CO_TRUE
*	object is mapped
* \retval CO_FALSE
*	object is not mapped
*
*/
BOOL_T coPdoObjIsMapped(
		UNSIGNED16		pdoNr,		/**< PDO number */
		UNSIGNED16		index,		/**< index of mapped object */
		UNSIGNED8		subIndex	/**< subindex of mapped object */
	)
{
RET_T	retVal;
UNSIGNED16	m;
CO_CONST CO_OBJECT_DESC_T	*pDesc;
UNSIGNED32	mapEntry;
UNSIGNED32	size;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(CO_FALSE);
	}

	/* get object description and size */
	retVal = coOdGetObjDescPtr(index, subIndex, &pDesc);
	if (retVal != RET_OK)  {
		return(CO_FALSE);
	}
#ifdef CO_PDO_BIT_MAPPING
	if ((pDesc->dType == CO_DTYPE_BOOL_CONST)
	 || (pDesc->dType == CO_DTYPE_BOOL_VAR)
	 || (pDesc->dType == CO_DTYPE_BOOL_PTR))  {
		size = 1u;
	} else
#endif /* CO_PDO_BIT_MAPPING */
	{
		size = coOdGetObjSize(pDesc) << 3;
	}
	mapEntry = ((UNSIGNED32)index << 16) | ((UNSIGNED32)subIndex << 8) | size;

	/* for all map entries */
	for (m = 0u; m < pPdo->mapTableConst->mapCnt; m++)  {
		if (pPdo->mapTableConst->mapEntry[m].val == mapEntry)  {
			return(CO_TRUE);
		}
	}

	return(CO_FALSE);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoReceiveHandler - PDO receive handler
*
*
* \return none
*
*/
void icoPdoReceiveHandler(
		CO_CONST CO_REC_DATA_T	*pRecData	/* pointer to received data */
	)
{
CO_REC_PDO_T	*pPdo;

	/* OPERATIONAL ? */
	if (coNmtGetState() != CO_NMT_STATE_OPERATIONAL)  {
		return;
	}

	if (pRecData->spec > pdoRecCnt)  {
		return;
	}

	pPdo = &pdoReceive[pRecData->spec];

	/* check for correct received data length */
	if (pRecData->msg.len < pPdo->mapLen)  {
#ifdef CO_EMCY_PRODUCER
		UNSIGNED8 strg[5] = { 0u, 0u, 0u, 0u, 0u };
		strg[0] = (UNSIGNED8)(pPdo->pdoNr & 0xffu);
		strg[1] = (UNSIGNED8)((pPdo->pdoNr >> 8u) & 0xffu);
/*
		strg[2] = pRecData->len;
		strg[3] = pdoReceive[idx].mapTable->mapCnt;
*/
		(void)icoEmcyWriteReq(CO_EMCY_ERRCODE_PDO_LEN, strg);
#endif /* CO_EMCY_PRODUCER */

		/* ignore message */
		return;
	}

	/* start event timer */
	if (pPdo->eventTime != 0u)  {
		(void) coTimerStart(&pPdo->pdoTimer,
				(UNSIGNED32)pPdo->eventTime * 1000u,
				pdoReceiveEventTimer, pPdo, CO_TIMER_ATTR_ROUNDDOWN);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
	}

#ifdef CO_MPDO_CONSUMER
	if (pPdo->pdoType != CO_PDO_TYPE_STD)  {
		icoMPdoReceive(pPdo, pRecData);
		return;
	}
#endif /* CO_MPDO_CONSUMER */

	/* synchronous PDO ? */
	if (pPdo->transType <= 240u)  {
		/* save data at shadow buffer */
		memcpy(pPdo->shadowData, pRecData->msg.data, CO_PDO_MAX_DATA_LEN);

		/* sign data as received for next sync */
		pPdo->state = CO_PDO_STATE_DATA_RECEIVED;
		return;
	}

	/* async data - unmap data */
	(void)pdoReceiveData(pRecData->spec, pRecData->msg.data);

	return;
}


/***************************************************************************/
/**
* \internal
*
* \brief pdoReceiveData - pdo data received 
*
* \return RET_T
*
*/
static RET_T pdoReceiveData(
		UNSIGNED16			pdoIdx,		/* PDO index */
		CO_CONST UNSIGNED8	pData[]		/* pointer to received data */
	)
{
RET_T	retVal = RET_OK;
UNSIGNED8	cnt;
UNSIGNED8	offs;
CO_REC_PDO_T	*pRecPdo;
BOOL_T		changed;

	pRecPdo = &pdoReceive[pdoIdx];

	/* map pdo data */
	offs = 0u;
	for (cnt = 0u; cnt < pRecPdo->mapTableConst->mapCnt; cnt++)  {
		/* not for dummy mapping */
		if (pRecPdo->mapTableConst->mapEntry[cnt].pVar != NULL)  {

#ifdef CO_PDO_BIT_MAPPING
			/* byte order? */
			if (((offs % 8u) == 0u)
			 && ((pRecPdo->mapTableConst->mapEntry[cnt].len % 8u) == 0u))  {
				/* normal byte mapping */
				/* numeric value ? */
				changed = coNumMemcpyPack(
					pRecPdo->mapTableConst->mapEntry[cnt].pVar,
					&pData[offs],
					(UNSIGNED32)pRecPdo->mapTableConst->mapEntry[cnt].len >> 3,
					(UNSIGNED16)pRecPdo->mapTableConst->mapEntry[cnt].numeric, 0u);
			} else {
				UNSIGNED8 byteLen, i, shiftCnt;
				UNSIGNED8 tmpBuf[8];

				byteLen = (pRecPdo->mapTableConst->mapEntry[cnt].len + 7u) >> 3;
				/* copy object to tmpvar */
				memset(&tmpBuf[0], 0, 8u);

				/* bitvar ? */
				if (pRecPdo->mapTableConst->mapEntry[cnt].len == 1u)  {
					/* set bit at can buffer if variable is true */
					tmpBuf[0] = (pData[offs >> 3] >> (offs & 7u)) & 0x1u;
				} else {
					/* move bytevar to can buffer */
					for (i = 0u; i < byteLen; i++)  {
						/* low part */
						shiftCnt = offs & 7u;
						tmpBuf[i] |= (UNSIGNED8)(pData[(offs >> 3) + i] >> shiftCnt);
						tmpBuf[i] &= 0xffu >> (offs & 7u);
						/* high part */
						tmpBuf[i] |= (UNSIGNED8)(pData[(offs >> 3) + i + 1u] << (8u - shiftCnt));
					}
				}
				changed = coNumMemcpyPack(
					pRecPdo->mapTableConst->mapEntry[cnt].pVar,
					&tmpBuf[0],
					(pRecPdo->mapTableConst->mapEntry[cnt].len + 7ul) >> 3u,
					(UNSIGNED16)pRecPdo->mapTableConst->mapEntry[cnt].numeric, 0u);
			}
#else /* CO_PDO_BIT_MAPPING */
			changed = coNumMemcpyPack(
				pRecPdo->mapTableConst->mapEntry[cnt].pVar,
				&pData[offs],
				(UNSIGNED32)pRecPdo->mapTableConst->mapEntry[cnt].len,
				(UNSIGNED16)pRecPdo->mapTableConst->mapEntry[cnt].numeric, 0u);
#endif /* CO_PDO_BIT_MAPPING */

#ifdef CO_EVENT_OBJECT_CHANGED
			if (changed == CO_TRUE)  {
				UNSIGNED16	index = (UNSIGNED16)(pRecPdo->mapTableConst->mapEntry[cnt].val >> 16u);
				UNSIGNED8	subIndex = (UNSIGNED8)((pRecPdo->mapTableConst->mapEntry[cnt].val >> 8u) & 0xffu);
				CO_CONST CO_OBJECT_DESC_T *pDesc;

				(void)coOdGetObjDescPtr(index, subIndex, &pDesc);
				(void)icoEventObjectChanged(pDesc,
					(UNSIGNED16)(pRecPdo->mapTableConst->mapEntry[cnt].val >> 16u),
					changed);
			}
#else
			(void)changed;
#endif /* CO_EVENT_OBJECT_CHANGED */


		}
		offs += pRecPdo->mapTableConst->mapEntry[cnt].len;
	}

# ifdef CO_EVENT_PDO
	/* user indication */
	pdoInd(pRecPdo->pdoNr, pRecPdo->transType);
# endif /*  CO_EVENT_PDO */

	return(retVal);
}


/***************************************************************************/
/**
* \internal stopRecPdoTimer - stop receive timer
*
* \brief 
*
* \return none
*
*/
static void stopRecPdoTimer(
		const CO_REC_PDO_T *pPdo	/* pointer to pdo */
	)
{
	(void) coTimerStop(&pPdo->pdoTimer);
}


/***************************************************************************/
/**
* \internal pdoReceiceEventTimer - pdo receive timer ellapsed
* wird vom timer aufgerufen
*
* \brief 
*
* \return none
*
*/
static void pdoReceiveEventTimer(
		void			*ptr
	)
{
# ifdef CO_EVENT_PDO
CO_REC_PDO_T	*pPdo = (CO_REC_PDO_T *)ptr;

	pdoRecEventInd(pPdo->pdoNr);
# endif /*  CO_EVENT_PDO */
	(void)ptr;
}
#endif /* CO_PDO_RECEIVE_CNT */


#ifdef CO_SYNC_SUPPORTED
/***************************************************************************/
/**
* \internal 
*
* \brief icoPdoSyncHandling - handle sync pdos
*
* \return none
*
*/
void icoPdoSyncHandling(
		UNSIGNED8		syncCounterVal	/* sync counter val */
	)
{
UNSIGNED16	cnt;
# ifdef CO_PDO_TRANSMIT_CNT
CO_TR_PDO_T	*pTrPdo;
# endif /* CO_PDO_TRANSMIT_CNT */
# ifdef CO_PDO_RECEIVE_CNT
CO_REC_PDO_T	*pRecPdo;
# endif /* CO_PDO_RECEIVE_CNT */

	/* return if not OPER */
	if (coNmtGetState() != CO_NMT_STATE_OPERATIONAL)  {
		return;
	}

# ifdef CO_PDO_TRANSMIT_CNT
	/* handle TPDOs */
	for (cnt = 0u; cnt < pdoTrCnt; cnt++)  {
		pTrPdo = &pdoTransmit[cnt];
		/* check, if PDO is not disabled */
		if ((pTrPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
			/* sync pdo ? */
			if (pTrPdo->transType <= 240u)  {
				/* sync pdo found */
				/* acyclic PDO ? */
				if (pTrPdo->transType == 0u)  {
					/* to transmit ? */
					if (pTrPdo->state == CO_PDO_STATE_TO_TRANSMIT)  {
						/* transmit data */
						(void)pdoTransmitData(pTrPdo, 0u);

						pTrPdo->state = CO_PDO_STATE_READY;
					}
				} else {
					/* cyclic PDO */
					if (pTrPdo->syncStartVal >= syncCounterVal)  {
						pTrPdo->actSyncCnt--;
						if ((syncCounterVal != 0u)
						 && (pTrPdo->syncStartVal == syncCounterVal))  {
							pTrPdo->actSyncCnt = 0u;
						}
						if (pTrPdo->actSyncCnt == 0u)  {
							pTrPdo->actSyncCnt = pTrPdo->transType;
							/* transmit */
							(void)pdoTransmitData(pTrPdo, 0u);
						}
					}
				}
			}
		}
	}
# endif /* CO_PDO_TRANSMIT_CNT */

# ifdef CO_PDO_RECEIVE_CNT
	/* handle RPDOs */
	for (cnt = 0u; cnt < pdoRecCnt; cnt++)  {
		pRecPdo = &pdoReceive[cnt];
		if (pRecPdo->transType <= 240u)  {
			/* sync pdo found */
			if (pRecPdo->state == CO_PDO_STATE_DATA_RECEIVED)  {
				/* unmap data from shadow buffer */
				(void)pdoReceiveData(cnt, pRecPdo->shadowData);

				pRecPdo->state = CO_PDO_STATE_READY;
			}
		}
	}
# endif /* CO_PDO_RECEIVE_CNT */
}
#endif /* CO_SYNC_SUPPORTED */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


#ifdef CO_PDO_TRANSMIT_CNT
/***************************************************************************/
/**
* \internal
*
* \brief icoTrPdoGetObjectAddr - get TPDO object address
*
* \return none
*
*/
void *icoTrPdoGetObjectAddr(
		UNSIGNED16		pdoNr,			/* PDO number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
void	*pAddr = NULL;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 1:
			/* copy from cob handler */
			pAddr = (void *)&pPdo->cobId;
			break;
		case 2:
			pAddr = (void *)&pPdo->transType;
			break;
		case 3:
			pAddr = (void *)&pPdo->inhibit;
			break;
		case 5:
			pAddr = (void *)&pPdo->eventTime;
			break;
		case 6:
			pAddr = (void *)&pPdo->syncStartVal;
			break;
		default:
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrMapGetObjectAddr - get address of mapping data
*
* \return none
*
*/
void *icoTrMapGetObjectAddr(
		UNSIGNED16		pdoNr,			/* PDO number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
void *pAddr = NULL;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 0u:
			pAddr = &pPdo->mapTable->mapCnt;
			break;
		default:
			pAddr = &pPdo->mapTable->mapEntry[subIndex - 1u].val;
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrMapGetObjectAddr_R - get address of mapping data for reading
*
* \return none
*
*/
const void *icoTrMapGetObjectAddr_R(
		UNSIGNED16		pdoNr,			/* PDO number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
const void *pAddr = NULL;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 0u:
			pAddr = &pPdo->mapTableConst->mapCnt;
			break;
		default:
			pAddr = &pPdo->mapTableConst->mapEntry[subIndex - 1u].val;
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrPdoCheckObjLimit_u8 - check object limits u8 value
*
*
* \return RET_T
*
*/
RET_T icoTrPdoCheckObjLimit_u8(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		subIndex,	/* subindex */
		UNSIGNED8		u8			/* pointer to receive data */
	)
{
RET_T	retVal = RET_OK;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

#ifdef CIA301_V3
#else /* CIA301_V3 */
	/* only allowed if pdo is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_SDO_INVALID_VALUE);
	}
#endif /* CIA301_V3 */

	/* transmission type */
	if (subIndex == 2u)  {
#ifdef CO_SYNC_SUPPORTED
		if ((u8 > 240u) && (u8 < 252u))  {
#else /* CO_SYNC_SUPPORTED */
		if (u8 < 253u)  {
#endif /* CO_SYNC_SUPPORTED */

			retVal = RET_SDO_INVALID_VALUE;

#ifdef CO_RTR_NOT_SUPPORTED
		} else {
			if ((u8 == 252u) || (u8 == 253u))  {
				retVal = RET_SDO_INVALID_VALUE;
			}
#endif /* CO_RTR_NOT_SUPPORTED */
		}

#ifdef CO_MPDO_PRODUCER
		/* for MPDO only 254 or 255 allowed */
		if ((pPdo->pdoType != CO_PDO_TYPE_STD)
		 && (u8 < 254u)) {
			retVal = RET_SDO_INVALID_VALUE;
		}
#endif /* CO_MPDO_PRODUCER */

	} else

	/* sync start value */
	if (subIndex == 6u)  {
	} else
	{
		retVal = RET_SUBIDX_NOT_FOUND;
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoCheckObjLimit_u16 - check object limits u16 value
*
*
* \return RET_T
*
*/
RET_T icoTrPdoCheckObjLimit_u16(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		subIndex	/* subindex */
	)
{
RET_T	retVal = RET_OK;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if (subIndex == 3u)  {
		/* only allowed if pdo is disabled */
		if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
			return(RET_SDO_INVALID_VALUE);
		}
	} else
	if (subIndex == 5u)  {
	} else
	{
		retVal = RET_SUBIDX_NOT_FOUND;
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrPdoCheckObjLimitCobId - check cob-id
*
*
* \return RET_T
*
*/
RET_T icoTrPdoCheckObjLimitCobid(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED32		canId		/* pointer to receive data */
	)
{
RET_T	retVal = RET_OK;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if ((canId & CO_COB_VALID_MASK) == 0u)  {
		/* new cobid is valid, only allowed if cob was disabled before*/
		if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
			/* cct checks for the same cob-od - allow it */
			if (pPdo->cobId != canId) {
				return(RET_SDO_INVALID_VALUE);
			}
		}
#ifdef CO_RTR_NOT_SUPPORTED
		if ((canId & CO_COB_RTR_BIT) == 0u)  {
			return(RET_SDO_INVALID_VALUE);
		}
#endif

		/* check for reserved cobs */
		if (icoCheckRestrictedCobs(canId, 0x181u, 0x181u) == CO_TRUE)  {
			return(RET_SDO_INVALID_VALUE);
		}
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrMapCheckObjLimit_u8 - check object limits mapping value
*
*
* \return RET_T
*
*/
RET_T icoTrMapCheckObjLimitMapTable(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		subIndex,
		UNSIGNED8		value
	)
{
RET_T	retVal = RET_OK;
UNSIGNED8	mapLen;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

#ifdef CIA301_V3
#else /* CIA301_V3 */
	/* change mapping is only allowed if PDO is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_MAP_ERROR);
	}
#endif /* CIA301_V3 */

	/* changing sub > 0 is only allowed if subindex 0 = 0 */
	if (subIndex == 0u)  {
		if (value != 0u)  {
			/* check mapping table */
			mapLen = checkTrPdoMappingTable(pPdo, value);
			if (mapLen > CO_PDO_MAX_DATA_LEN)  {
				return(RET_MAP_LEN_ERROR);
			}
		}
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrMapCheckObjLimitMapEntry - check object limits mapping entry
*
*
* \return RET_T
*
*/
RET_T icoTrMapCheckObjLimitMapEntry(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED32		value
	)
{
RET_T	retVal;
CO_CONST void		*pVar;
UNSIGNED8	len;
BOOL_T		numeric;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

#ifdef CIA301_V3
#else /* CIA301_V3 */
	/* change mapping is only allowed if PDO is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_MAP_ERROR);
	}
#endif /* CIA301_V3 */

	/* changing sub > 0 is only allowed if subindex 0 = 0 */
	if (pPdo->mapTableConst->mapCnt != 0u)  {
		return(RET_MAP_ERROR);
	}

	/* check mapping entry */
	retVal = icoOdGetObjTrMapData(
			(UNSIGNED16)(value >> 16u),
			(UNSIGNED8)((value >> 8u) & 0xffu),
			&pVar,
			&len,
			&numeric);

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrPdoObjChanged - TPdo object was changed
*
*
* \return RET_T
*
*/
RET_T icoTrPdoObjChanged(
		UNSIGNED16		pdoNr,		/* PDO number */
		UNSIGNED8		subIndex	/* subindex */
	)
{
UNSIGNED8	mapLen;
RET_T		retVal = RET_OK;
CO_TR_PDO_T *pPdo;

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	switch (subIndex)  {
		case 1u:
			/* get number of mapped bytes */
			mapLen = checkTrPdoMappingTable(pPdo, 0u);
			retVal = setPdoCob(pPdo->cob, pPdo->cobId, mapLen);
			if (retVal == RET_OK)  {
				/* setup timer */
				retVal = setupTrPdoTimer(pPdo);
			}
			if (retVal == RET_OK)  {
				/* set inhibit - only for transtype fe and ff (otherwise 0) */
				if (pPdo->transType >= 254u)  {
					retVal = icoCobSetInhibit(pPdo->cob,
								pPdo->inhibit);
				} else {
					retVal = icoCobSetInhibit(pPdo->cob, 0u);
				}
			}
			break;
		case 2u:
			pPdo->actSyncCnt = pPdo->transType;
			retVal = setupTrPdoTimer(pPdo);
			break;
		case 3u:
			break;
		case 5u:
			retVal = setupTrPdoTimer(pPdo);
			break;
		default:
			break;
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTrPdoMapChanged - TPdo mapping was changed
*
*
* \return RET_T
*
*/
RET_T icoTrPdoMapChanged(
		UNSIGNED16		pdoNr			/* PDO number */
	)
{
CO_TR_PDO_T *pPdo;
RET_T		retVal = RET_OK;
# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
PDO_TR_MAP_TABLE_T	*pMap;
UNSIGNED8	cnt;
UNSIGNED32	entry;
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */

	pPdo = icoPdoSearchTrPdoIndex(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
	if (pPdo->dynMapping == CO_TRUE)  {
		pMap = &mapTablesTrPDO[pPdo->mapTableIdx];

		/* for all mapping entries */
		for (cnt = 0u; cnt < pMap->mapCnt; cnt++)  {
			entry = pMap->mapEntry[cnt].val;

			retVal = icoOdGetObjTrMapData(
				(UNSIGNED16)(entry >> 16u),
				(UNSIGNED8)((entry >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
			if (retVal != RET_OK)  {
				return(retVal);
			}
		}
	}
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */

	return(retVal);
}
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoGetObjectAddr - get RPDO object address
*
* \return RET_T
*
*/
void *icoRecPdoGetObjectAddr(
		UNSIGNED16		pdoNr,			/* pdo number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
void	*pAddr = NULL;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 1u:
			/* copy from cob handler */
			pAddr = (void *)&pPdo->cobId;
			break;
		case 2u:
			pAddr = (void *)&pPdo->transType;
			break;
		case 3u:
			pAddr = (void *)&pPdo->inhibit;
			break;
		case 5u:
			pAddr = (void *)&pPdo->eventTime;
			break;
		default:
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecMapGetObjectAddr - get address of mapping data
*
* \return none
*
*/
void *icoRecMapGetObjectAddr(
		UNSIGNED16		pdoNr,			/* pdo number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
void *pAddr = NULL;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 0u:
			pAddr = &pPdo->mapTable->mapCnt;
			break;
		default:
			pAddr = &pPdo->mapTable->mapEntry[subIndex - 1u].val;
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecMapGetObjectAddr_R - get address of mapping data for reading
*
* \return none
*
*/
const void *icoRecMapGetObjectAddr_R(
		UNSIGNED16		pdoNr,			/* pdo number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
const void *pAddr = NULL;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 0u:
			pAddr = &pPdo->mapTableConst->mapCnt;
			break;
		default:
			pAddr = &pPdo->mapTableConst->mapEntry[subIndex - 1u].val;
			break;
	}

	return(pAddr);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoCheckObjLimit_u8 - check object limits u8 value for RPDO
*
*
* \return RET_T
*
*/
RET_T icoRecPdoCheckObjLimit_u8(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		subIndex,	/* subindex */
		UNSIGNED8		u8			/* pointer to receive data */
	)
{
RET_T	retVal = RET_OK;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	/* only allowed if pdo is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_SDO_INVALID_VALUE);
	}

	/* transmission type */
	if (subIndex == 2u)  {
#ifdef CO_SYNC_SUPPORTED
		if ((u8 > 240u) && (u8 < 254u)) {
#else /* CO_SYNC_SUPPORTED */
		if (u8 < 254u) {
#endif /* CO_SYNC_SUPPORTED */
			return(RET_SDO_INVALID_VALUE);
		}

#ifdef CO_MPDO_CONSUMER
		/* for MPDO only 254 or 255 allowed */
		if ((pPdo->pdoType != CO_PDO_TYPE_STD)
		 && (u8 < 254u)) {
			retVal = RET_SDO_INVALID_VALUE;
		}
#endif /* CO_MPDO_CONSUMER */

	} else

	/* sync start value */
	if (subIndex == 6u)  {
	} else
	{
		retVal = RET_SUBIDX_NOT_FOUND;
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoCheckObjLimit_u16 - check object limits u16 value
*
*
* \return RET_T
*
*/
RET_T icoRecPdoCheckObjLimit_u16(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		subIndex	/* subindex */
	)
{
RET_T	retVal = RET_OK;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if (subIndex == 3u)  {
		/* only allowed if pdo is disabled */
		if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
			return(RET_SDO_INVALID_VALUE);
		}
	} else
	if (subIndex == 5u)  {
	} else
	{
		retVal = RET_SUBIDX_NOT_FOUND;
	}
	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoCheckObjLimitCobId - check cob-id
*
*
* \return RET_T
*
*/
RET_T icoRecPdoCheckObjLimitCobid(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED32		canId		/* pointer to receive data */
	)
{
RET_T	retVal = RET_OK;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if ((canId & CO_COB_VALID_MASK) == 0u)  {
		/* new cobid is valid, only allowed if cob was disabled before*/
		if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
			/* cct checks for the same cob-od - allow it */
			if (pPdo->cobId != canId) {
				return(RET_SDO_INVALID_VALUE);
			}
		}
		/* check for reserved cobs */
		if (icoCheckRestrictedCobs(canId, 0x181u, 0x181u) == CO_TRUE)  {
			return(RET_SDO_INVALID_VALUE);
		}
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecMapCheckObjLimitMapTable - check object limits mapping table
*
*
* \return RET_T
*
*/
RET_T icoRecMapCheckObjLimitMapTable(
		UNSIGNED16		pdoNr,			/* pdo number */
		UNSIGNED8		subIndex,
		UNSIGNED8		value
	)
{
RET_T	retVal = RET_OK;
UNSIGNED8	mapLen;
PDO_REC_MAP_ENTRY_T	CO_CONST *pEntry;
void		*pVar;
UNSIGNED8	len;
BOOL_T		numeric;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	/* change mapping is only allowed if PDO is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_MAP_ERROR);
	}

	/* changing sub > 0 is only allowed if subindex 0 = 0 */
	if (subIndex != 0u)  {
		if (pPdo->mapTableConst->mapCnt != 0u)  {
			return(RET_MAP_ERROR);
		}
		/* check mapping entry */
		pEntry = &pPdo->mapTableConst->mapEntry[subIndex - 1u];
		retVal = icoOdGetObjRecMapData(
				(UNSIGNED16)(pEntry->val >> 16u),
				(UNSIGNED8)((pEntry->val >> 8u) & 0xffu),
				&pVar,
				&len,
				&numeric);
	} else {
		if (value != 0u)  {
			/* check mapping table */
			mapLen = checkRecPdoMappingTable(pPdo, value);
			if (mapLen > CO_PDO_MAX_DATA_LEN)  {
				return(RET_MAP_LEN_ERROR);
			}
		}
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecMapCheckObjLimitMapEntry - check object limits mapping value
*
*
* \return RET_T
*
*/
RET_T icoRecMapCheckObjLimitMapEntry(
		UNSIGNED16		pdoNr,			/* pdo number */
		UNSIGNED32		mapEntry		/* mapping entry */
	)
{
RET_T	retVal;
UNSIGNED16	mapIdx;
UNSIGNED8	mapSubIdx;
void 		*pVar;
UNSIGNED8	len;
BOOL_T		numeric;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	/* change mapping is only allowed if PDO is disabled */
	if ((pPdo->cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
		return(RET_MAP_ERROR);
	}

	/* changing sub > 0 is only allowed if subindex 0 = 0 */
	if (pPdo->mapTableConst->mapCnt != 0u)  {
		return(RET_MAP_ERROR);
	}

	mapIdx = (UNSIGNED16)(mapEntry >> 16);
	mapSubIdx = (UNSIGNED8)((mapEntry >> 8) & 0xffu);

	/* check mapping entry */
	retVal = icoOdGetObjRecMapData(mapIdx, mapSubIdx, &pVar, &len, &numeric);
	if (retVal != RET_OK)  {
		return(RET_MAP_ERROR);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoObjChanged - RecPdo object was changed
*
*
* \return RET_T
*
*/
RET_T icoRecPdoObjChanged(
		UNSIGNED16		pdoNr,		/*  pdo number */
		UNSIGNED8		subIndex	/* subindex */
	)
{
UNSIGNED8	mapLen;
RET_T		retVal = RET_OK;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	switch (subIndex)  {
		case 1u:
			/* get number of mapped bytes */
			mapLen = checkRecPdoMappingTable(pPdo, 0u);
			retVal = setPdoCob(pPdo->cob, pPdo->cobId, mapLen);
			stopRecPdoTimer(pPdo);
			if (retVal == RET_OK)  {
				pPdo->mapLen = mapLen;
			}
			break;
		case 5u:
			stopRecPdoTimer(pPdo);
			break;
		default:
			break;
	}

	return(retVal);
}


# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
/***************************************************************************/
/**
* \internal
*
* \brief icoRecPdoMapChanged - RPdo mapping was changed
*
*
* \return RET_T
*
*/
RET_T icoRecPdoMapChanged(
		UNSIGNED16		pdoNr		/* PDO number */
	)
{
RET_T		retVal = RET_OK;
PDO_REC_MAP_TABLE_T	*pMap;
UNSIGNED8	cnt;
UNSIGNED32	entry;
CO_REC_PDO_T *pPdo;

	/* search PDO */
    pPdo = searchRecPdo(pdoNr);
	if (pPdo == NULL)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if (pPdo->dynMapping == CO_TRUE)  {
		pMap = &mapTablesRecPDO[pPdo->mapTableIdx];

		/* for all mapping entries */
		for (cnt = 0u; cnt < pMap->mapCnt; cnt++)  {
			entry = pMap->mapEntry[cnt].val;

			retVal = icoOdGetObjRecMapData(
				(UNSIGNED16)(entry >> 16u),
				(UNSIGNED8)((entry >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
			if (retVal != RET_OK)  {
				return(retVal);
			}
		}
	}

	return(retVal);
}
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
#endif /* CO_PDO_RECEIVE_CNT */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


#ifdef CO_PDO_RECEIVE_CNT
# ifdef CO_EVENT_PDO_CNT
/***************************************************************************/
/**
* \brief coEventRegister_PDO - register asynchronous PDO event
*
* Register an indication function for asynchrounous PDOs.
*
* After a PDO has been received, the data are stored in the object dictionary,
* and then the given indication function is called.
* This function is only valid for asynchronous PDOs.
*
* \return RET_T
*
*/

RET_T coEventRegister_PDO(
		CO_EVENT_PDO_T pFunction	/**< pointer to function */
    )
{
	if (pdoEventTableCnt >= CO_EVENT_PDO_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	pdoEventTable[pdoEventTableCnt] = pFunction;
	pdoEventTableCnt++;

	return(RET_OK);
}


/***************************************************************************/
/**
* \brief coEventRegister_PDO_REC_EVENT - register receive PDO event
*
* Register an indication function for receive PDO event.
*
* For monitoring of receive PDOs the event timer can be used.
* If the event timer value is unequal 0 then after the reception of a PDO
* the monitoring is started automatically.
* if no further PDO in the given time was received,
* the indication function given to this function is called.
*
* \return RET_T
*
*/

RET_T coEventRegister_PDO_REC_EVENT(
		CO_EVENT_PDO_T pFunction	/**< pointer to function */
    )
{
	if (pdoRecEventTableCnt >= CO_EVENT_PDO_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	pdoRecEventTable[pdoRecEventTableCnt] = pFunction;
	pdoRecEventTableCnt++;

	return(RET_OK);
}


#  ifdef CO_SYNC_SUPPORTED
/***************************************************************************/
/**
* \brief coEventRegister_PDO_SYNC - register PDO SYNC event
*
* Register an indication function for received synchronous PDOs.
*
* After the SYNC was received,
* the received data are stored in the object dictionary,
* and then this given indication function is called.
*
* \return RET_T
*
*/

RET_T coEventRegister_PDO_SYNC(
		CO_EVENT_PDO_T pFunction	/**< pointer to function */
    )
{
	if (pdoEventSyncTableCnt >= CO_EVENT_PDO_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	pdoEventSyncTable[pdoEventSyncTableCnt] = pFunction;
	pdoEventSyncTableCnt++;

	return(RET_OK);
}
#  endif /* CO_SYNC_SUPPORTED */
# endif /*  CO_EVENT_PDO_CNT */


# ifdef CO_EVENT_PDO
/***************************************************************************/
/**
* \internal
*
* \brief pdoInd - check user indication
*
* \return RET_T
*
*/
static void pdoInd(
		UNSIGNED16		pdoNr,		/* pdo number */
		UNSIGNED8		transType	/* transmission type */
	)
{
#  ifdef CO_EVENT_PDO_CNT
UNSIGNED16	cnt;
#  endif /* CO_EVENT_PDO_CNT */

	if (transType > 240u)  {
		/* asynchronous */
#  ifdef CO_EVENT_PDO_CNT
		/* call indication to execute */
		cnt = pdoEventTableCnt;
		while (cnt > 0u)  {
			cnt--;
			/* call user indication */
			pdoEventTable[cnt](pdoNr);
		}
#  endif /*  CO_EVENT_PDO_CNT */

#  ifdef CO_EVENT_STATIC_PDO
		coEventPdoInd(pdoNr);
#  endif /* CO_EVENT_STATIC_PDO */

#  ifdef CO_SYNC_SUPPORTED
	} else {
		/* synchronous */
#   ifdef CO_EVENT_PDO_CNT
		/* call indication to execute */
		cnt = pdoEventSyncTableCnt;
		while (cnt > 0u)  {
			cnt--;
			/* call user indication */
			pdoEventSyncTable[cnt](pdoNr);
		}
#   endif /*  CO_EVENT_PDO_CNT */
#   ifdef CO_EVENT_STATIC_PDO
		coEventPdoSyncInd(pdoNr);
#   endif /* CO_EVENT_STATIC_PDO */
#  endif /* CO_SYNC_SUPPORTED */
	}

	return;
}


/***************************************************************************/
/**
* \internal
*
* \brief pdoRecEventInd - receive event occured
*
* This function is called, if receive event timer was timed out
*
*
*/
static void pdoRecEventInd(
		UNSIGNED16		pdoNr		/* pdo number */
	)
{
#  ifdef CO_EVENT_PDO_CNT
UNSIGNED16	cnt;
#  endif /*  CO_EVENT_PDO_CNT */

#  ifdef CO_EVENT_PDO_CNT
	/* call indication to execute */
	cnt = pdoRecEventTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		pdoRecEventTable[cnt](pdoNr);
	}
#  endif /*  CO_EVENT_PDO_CNT */

#  ifdef CO_EVENT_STATIC_PDO
	coEventPdoRecInd(pdoNr);
#  endif /* CO_EVENT_STATIC_PDO */

	return;
}
# endif /*  CO_EVENT_PDO */
#endif /* CO_PDO_RECEIVE_CNT */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


#ifdef CO_PDO_TRANSMIT_CNT
/***************************************************************************/
/**
* \internal
*
* \brief setupTrPdoTimer - setup transmit PDO timer
*
* \return RET_T
*
*/
static RET_T setupTrPdoTimer(
		CO_TR_PDO_T		*pPdo		/* pointer to Transmit PDO */
	)
{
RET_T	retVal = RET_OK;
CO_NMT_STATE_T nmtState;

	nmtState = coNmtGetState();

/* start event timer */
/* start sync start value */

	if (((pPdo->cobId & CO_COB_VALID_MASK) == CO_COB_INVALID) 
	  || (pPdo->eventTime == 0u)
	  || (pPdo->transType < 254u)
	  || (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		(void)coTimerStop(&pPdo->pdoTimer);
	} else {
		if (pPdo->eventTime != 0u)  {
			retVal = coTimerStart(&pPdo->pdoTimer,
				(UNSIGNED32)pPdo->eventTime * 1000u,
				pdoEventTimer, pPdo, CO_TIMER_ATTR_ROUNDDOWN_CYCLIC);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
		}
	}

	return(retVal);
}


/* wird vom timer aufgerufen */
static void pdoEventTimer(
		void			*ptr
	)
{
CO_TR_PDO_T	*pPdo = (CO_TR_PDO_T *)ptr;

	(void)coPdoReqNr(pPdo->pdoNr, 0u);
}


/***************************************************************************/
/**
* \internal
*
* \brief checkTrPdoMappingTable - check PDO transmit mapping table
*
* if mapCnt == 0, use value from pdoTransmit[idx].mapTable->mapCnt
*
* \return mapcount
*/
static UNSIGNED8 checkTrPdoMappingTable(
		CO_TR_PDO_T		*pPdo,	/* pointer to pdo */
		UNSIGNED8		mapCnt
	)
{
UNSIGNED8	offs;
UNSIGNED8	cnt;
PDO_TR_MAP_ENTRY_T	CO_CONST *pEntry;
RET_T		retVal;
PDO_TR_MAP_TABLE_T *pMap;
PDO_TR_MAP_TABLE_T map;

	if (mapCnt == 0u) {
		mapCnt = pPdo->mapTableConst->mapCnt;
	}

#ifdef CO_MPDO_PRODUCER
	/* MPDO ? */
	if (mapCnt == CO_MPDO_DAM_MODE)  {
		pPdo->pdoType = CO_PDO_TYPE_DAM;
		mapCnt = 1u;
	} else
	if (mapCnt == CO_MPDO_SAM_MODE)  {
		pPdo->pdoType = CO_PDO_TYPE_SAM;
		return(8u);
	} else {
		pPdo->pdoType = CO_PDO_TYPE_STD;
	}
#endif /* CO_MPDO_PRODUCER */

	/* mapCnt > max sub index */
	if (mapCnt > icoOdGetNumberOfSubs(0x1A00u + (pPdo->pdoNr - 1u)))  {
		return(255u);
	}

	offs = 0u;
	for (cnt = 0u; cnt < mapCnt; cnt++)  {
		/* check mapentry */
		pEntry = &pPdo->mapTableConst->mapEntry[cnt];
# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
		if (pPdo->mapTableIdx == 0xffffu)  {
			pMap = &map;
		} else {
			pMap = &mapTablesTrPDO[pPdo->mapTableIdx];
		}
# else /* CO_TR_PDO_DYN_MAP_ENTRIES */
		pMap = &map;
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */

		retVal = icoOdGetObjTrMapData(
				(UNSIGNED16)(pEntry->val >> 16u),
				(UNSIGNED8)((pEntry->val >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
		if (retVal != RET_OK)  {
			return(255u);
		}

		offs += pMap->mapEntry[cnt].len;
	}

#ifdef CO_PDO_BIT_MAPPING
	/* convert len from bit to bytes */
	offs = (offs + 7u) >> 3;
#endif /* CO_PDO_BIT_MAPPING */

#ifdef CO_MPDO_PRODUCER
	/* MPDO ? */
	if (pPdo->pdoType == CO_PDO_TYPE_DAM)  {
		/* max. 4 Bytes allowed */
		if (offs > 4u)  {
			return(255u);
		}

		/* but can frame has 8 bytes */
		offs = 8u;
	}
#endif /* CO_MPDO_PRODUCER */

#ifdef CO_PDO_RECEIVE_CNT
# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
#endif /* CO_PDO_RECEIVE_CNT */


	return(offs);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoSearchTrPdoIndex - search for pdo transmit index
*
*
* \return index
*
*/
CO_INLINE CO_TR_PDO_T *icoPdoSearchTrPdoIndex(
		UNSIGNED16		pdoNr		/* pdo number */
	)
{
UNSIGNED16	cnt;

	for (cnt = 0u; cnt < CO_PDO_TRANSMIT_CNT; cnt++)  {
		if (pdoNr == pdoTransmit[cnt].pdoNr)  {
			return(&pdoTransmit[cnt]);
		}
	}

	return(NULL);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoOperational - new OPERATIONAL state
*
* transmit all async pdos and start event timers
*
* \return none
*
*/
void icoPdoOperational(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
CO_TR_PDO_T	*pPdo;

	for (cnt = 0u; cnt < pdoTrCnt; cnt++)  {
		pPdo = &pdoTransmit[cnt];

#ifdef CO_PDO_NO_TRANSMIT_OPERATIONAL
#else /* CO_PDO_NO_TRANSMIT_OPERATIONAL */
		/*  if async or sync non-cyclic PDO ? */
		(void)coPdoReqNr(pPdo->pdoNr, 0u);

# ifdef CO_PDO_SYNC_TRANSMIT_OPERATIONAL
		/* send all SYNC PDOs immediately at next sync */
		pPdo->actSyncCnt = 1;
# endif /* CO_PDO_SYNC_TRANSMIT_OPERATIONAL */
#endif /* CO_PDO_NO_TRANSMIT_OPERATIONAL */

		(void)setupTrPdoTimer(pPdo);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoPreOperational - new PRE_OPERATIONAL state
*
* stop event timers
*
* \return none
*
*/
void icoPdoPreOperational(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
CO_TR_PDO_T	*pPdo;

	for (cnt = 0u; cnt < pdoTrCnt; cnt++)  {
		pPdo = &pdoTransmit[cnt];

		(void)setupTrPdoTimer(pPdo);
	}
}


# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
/***************************************************************************/
/**
* \internal
*
* \brief createPdoMapTable - create new dynamic mapping table
*
* \return none
*
*/
static UNSIGNED16 createPdoTrMapTable(
		UNSIGNED16		index
	)
{
RET_T		retVal;
UNSIGNED16	mapTableIdx;

	/* mapping table available ? */
	if (mapTableTrPdoCnt == CO_TR_PDO_DYN_MAP_ENTRIES)  {
		/* no, return */
		return(0xffffu);
	}
	mapTableIdx = mapTableTrPdoCnt;

	retVal = setupPdoTrMapTable(mapTableIdx, index);
	if (retVal != RET_OK)  {
		return(0xffffu);
	}

	mapTableTrPdoCnt++;

	return(mapTableIdx);
}


/***************************************************************************/
/**
* \internal
*
* \brief setupPdoMapTable - create new dynamic mapping table
*
* \return RET_T
*
*/
static RET_T setupPdoTrMapTable(
		UNSIGNED16		mapTableIdx,
		UNSIGNED16		index
	)
{
UNSIGNED8	cnt;
RET_T		retVal;
UNSIGNED32	mapEntry;
UNSIGNED8	numberOfSubs;
PDO_TR_MAP_TABLE_T *pMap;

	pMap = &mapTablesTrPDO[mapTableIdx];

	/* get number of mappings */
	retVal = coOdGetDefaultVal_u8(index, 0u, &pMap->mapCnt);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* get number of subindex for this mapping table */
	numberOfSubs = icoOdGetNumberOfSubs(index) - 1u;

	/* for all mapping entries */
	for (cnt = 0u; cnt < numberOfSubs; cnt++)  {
		retVal = coOdGetDefaultVal_u32(index, cnt + 1u, &mapEntry);
		if (retVal != RET_OK)  {
			return(retVal);
		}
		pMap->mapEntry[cnt].val = mapEntry;

		/* check mapping entry only for valid mappings */
		if (cnt < pMap->mapCnt)  {
			retVal = icoOdGetObjTrMapData(
				(UNSIGNED16)(mapEntry >> 16u),
				(UNSIGNED8)((mapEntry >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
			if (retVal != RET_OK)  {
				return(retVal);
			}
		}
	}

	return(retVal);
}
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
/***************************************************************************/
/**
* \internal
*
* \brief checkRecPdoMappingTable - check PDO receive mapping table
*
* if mapCnt == 0, use value from pdoTransmit[idx].mapTable->mapCnt
*
* \return mapping count
*
*/
static UNSIGNED8 checkRecPdoMappingTable(
		CO_REC_PDO_T	*pPdo,
		UNSIGNED8		mapCnt
	)
{
UNSIGNED8	offs;
UNSIGNED8	cnt;
PDO_REC_MAP_ENTRY_T	CO_CONST *pEntry;
RET_T		retVal;
PDO_REC_MAP_TABLE_T *pMap;
PDO_REC_MAP_TABLE_T map;

	if (mapCnt == 0u) {
		mapCnt = pPdo->mapTableConst->mapCnt;
	}

#ifdef CO_MPDO_CONSUMER
	/* MPDO ? */
	if (mapCnt == CO_MPDO_DAM_MODE)  {
		pPdo->pdoType = CO_PDO_TYPE_DAM;
		return(8u);
	} else
	if (mapCnt == CO_MPDO_SAM_MODE)  {
		pPdo->pdoType = CO_PDO_TYPE_SAM;
		return(8u);
	} else {
		pPdo->pdoType = CO_PDO_TYPE_STD;
	}
#endif /* CO_MPDO_CONSUMER */


	/* mapCnt > max sub index */
	if (mapCnt > icoOdGetNumberOfSubs(0x1600u + (pPdo->pdoNr - 1u)))  {
		return(255u);
	}
	offs = 0u;
	for (cnt = 0u; cnt < mapCnt; cnt++)  {
		/* check mapentry */
		pEntry = &pPdo->mapTableConst->mapEntry[cnt];
# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
		if (pPdo->mapTableIdx == 0xffffu)  {
			pMap = &map;
		} else {
			pMap = &mapTablesRecPDO[pPdo->mapTableIdx];

		}
# else /* CO_REC_PDO_DYN_MAP_ENTRIES */
		pMap = &map;
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */

		retVal = icoOdGetObjRecMapData(
				(UNSIGNED16)(pEntry->val >> 16u),
				(UNSIGNED8)((pEntry->val >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
		if (retVal != RET_OK)  {
			return(255u);
		}

		offs += pPdo->mapTableConst->mapEntry[cnt].len;
	}

# ifdef CO_PDO_BIT_MAPPING
	/* convert len from bit to bytes */
	offs = (offs + 7u) >> 3;
# endif /* CO_PDO_BIT_MAPPING */

# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */

	return(offs);
}


/***************************************************************************/
/**
* \internal
*
* \brief searchRecPdo - search for pdo receive and return pointer to PDO
*
*
* \return PDO pointer
*
*/
static CO_INLINE CO_REC_PDO_T *searchRecPdo(
		UNSIGNED16		pdoNr
	)
{
UNSIGNED16	cnt;

	for (cnt = 0u; cnt < CO_PDO_RECEIVE_CNT; cnt++)  {
		if (pdoNr == pdoReceive[cnt].pdoNr)  {
			return(&pdoReceive[cnt]);
		}
	}

	return(NULL);
}


# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
/***************************************************************************/
/**
* \internal
*
* \brief createPdoMapTable - create new dynamic mapping table
*
* \return map table index
*
*/
static UNSIGNED16 createPdoRecMapTable(
		UNSIGNED16		index
	)
{
RET_T		retVal;
UNSIGNED16	mapTableIdx;

	/* mapping table available ? */
	if (mapTableRecPdoCnt == CO_REC_PDO_DYN_MAP_ENTRIES)  {
		/* no, return */
		return(0xffffu);
	}
	mapTableIdx = mapTableRecPdoCnt;

	retVal = setupPdoRecMapTable(mapTableIdx, index);
	if (retVal != RET_OK)  {
		return(0xffffu);
	}

	mapTableRecPdoCnt++;

	return(mapTableIdx);
}


/***************************************************************************/
/**
* \internal
*
* \brief setupPdoMapTable - create new dynamic mapping table
*
* \return none
*
*/
static RET_T setupPdoRecMapTable(
		UNSIGNED16		mapTableIdx,
		UNSIGNED16		index
	)
{
UNSIGNED8	cnt;
RET_T		retVal;
UNSIGNED32	mapEntry;
UNSIGNED8	numberOfSubs;
PDO_REC_MAP_TABLE_T *pMap;

	pMap = &mapTablesRecPDO[mapTableIdx];

	/* get number of mappings */
	retVal = coOdGetDefaultVal_u8(index, 0u, &pMap->mapCnt);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* get number of subindex for this mapping table */
	numberOfSubs = icoOdGetNumberOfSubs(index) - 1u;

	/* for all mapping entries */
	for (cnt = 0u; cnt < numberOfSubs; cnt++)  {
		retVal = coOdGetDefaultVal_u32(index, cnt + 1u, &mapEntry);
		if (retVal != RET_OK)  {
			return(retVal);
		}
		pMap->mapEntry[cnt].val = mapEntry;

		/* check mapping entry only for valid mappings */
		if (cnt < pMap->mapCnt)  {
			retVal = icoOdGetObjRecMapData(
				(UNSIGNED16)(mapEntry >> 16u),
				(UNSIGNED8)((mapEntry >> 8u) & 0xffu),
				&pMap->mapEntry[cnt].pVar,
				&pMap->mapEntry[cnt].len,
				&pMap->mapEntry[cnt].numeric);
			if (retVal != RET_OK)  {
				return(retVal);
			}
		}
	}

	return(retVal);
}


# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
#endif /* CO_PDO_RECEIVE_CNT */




/***************************************************************************/
/**
* \internal
*
* \brief setPdoCob - set PDO cob id
*
* \return none
*	RET_T
*/
static RET_T setPdoCob(
		COB_REFERENZ_T	cobRef,			/* cob reference */
		UNSIGNED32		cobId,			/* cob-id */
		UNSIGNED8		len				/* data len */
	)
{
RET_T	retVal;

	if ((cobId & CO_COB_RTR_BIT) != 0ul)  {
		retVal = icoCobSet(cobRef, cobId, CO_COB_RTR_NONE, len);
	} else {
		retVal = icoCobSet(cobRef, cobId, CO_COB_RTR, len);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief coPdoReset - reset comm for Pdo
*
* \return none
*	RET_T
*/
void icoPdoReset(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
#ifdef CO_PDO_RECEIVE_CNT
UNSIGNED8	mapLen;
CO_REC_PDO_T	*pRecPdo;
#endif /* CO_PDO_RECEIVE_CNT */
#ifdef CO_PDO_TRANSMIT_CNT
CO_TR_PDO_T	*pTrPdo;
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_TRANSMIT_CNT
	for (cnt = 0u; cnt < pdoTrCnt; cnt++)  {
		pTrPdo = &pdoTransmit[cnt];

		/* reset mapping */
		if ((pTrPdo->cobId & CO_COB_INVALID) != 0u)  {
			(void)icoCobDisable(pTrPdo->cob);
		} else {
			/* get number of mapped bytes */
			(void)icoTrPdoObjChanged(pTrPdo->pdoNr, 1u);
		}

		pTrPdo->actSyncCnt = pTrPdo->transType;
		pTrPdo->state = CO_PDO_STATE_READY;

		(void) setupTrPdoTimer(pTrPdo);
	}
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
	/* receive PDO */
	for (cnt = 0u; cnt < pdoRecCnt; cnt++)  {
		pRecPdo = &pdoReceive[cnt];

		/* reset mapping */
		if ((pRecPdo->cobId & CO_COB_INVALID) != 0u)  {
			/* disable sdo */
			(void) icoCobDisable(pRecPdo->cob);
		} else {
			/* get number of mapped bytes */
			mapLen = checkRecPdoMappingTable(pRecPdo, 0u);
			pRecPdo->mapLen = mapLen;
			(void) setPdoCob(pRecPdo->cob, pRecPdo->cobId, mapLen);
		}

		pRecPdo->state = CO_PDO_STATE_READY;

		stopRecPdoTimer(pRecPdo);
	}
#endif /* CO_PDO_RECEIVE_CNT */

	return;
}


/***************************************************************************/
/**
* \internal
*
* \brief icoPdoSetDefaultValue
*
* \return none
*	
*/
void icoPdoSetDefaultValue(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
RET_T		retVal;
UNSIGNED32	u32;
UNSIGNED16	u16;
UNSIGNED8	u8;
#ifdef CO_PDO_RECEIVE_CNT
CO_REC_PDO_T	*pRecPdo;
#endif /* CO_PDO_RECEIVE_CNT */
#ifdef CO_PDO_TRANSMIT_CNT
CO_TR_PDO_T	*pTrPdo;
#endif /* CO_PDO_TRANSMIT_CNT */
UNSIGNED16	pdoIdx;


#ifdef CO_PDO_TRANSMIT_CNT
	for (cnt = 0u; cnt < pdoTrCnt; cnt++)  {
		pTrPdo = &pdoTransmit[cnt];

		/* get disable and RTR bit from OD */
		retVal = coOdGetDefaultVal_u32((0x1800u + pTrPdo->pdoNr) - 1u, 1u, &u32);
		if (retVal == RET_OK)  {
			u32 &= (CO_COB_INVALID | CO_COB_RTR_BIT);
		} else {
			u32 = 0u;
		}

		if ((pTrPdo->pdoNr >= 1u) && (pTrPdo->pdoNr <= 4u))  {
			/* calculate cobs for predefined connection set */
			pdoIdx = pTrPdo->pdoNr - 1u;
			pTrPdo->cobId = u32
				| ((0x180ul + (0x100ul * pdoIdx)) + coNmtGetNodeId());
		} else {
			/* disable sdo */
			pTrPdo->cobId = u32 | CO_COB_INVALID;
		}

		/* transmission type */
		retVal = coOdGetDefaultVal_u8((0x1800u + pTrPdo->pdoNr) - 1u, 2u, &u8);
		if (retVal == RET_OK)  {
			pTrPdo->transType = u8;
		}
		retVal = coOdGetDefaultVal_u16((0x1800u + pTrPdo->pdoNr) - 1u, 3u, &u16);
		if (retVal == RET_OK)  {
			pTrPdo->inhibit = u16;
		}
		retVal = coOdGetDefaultVal_u16((0x1800u + pTrPdo->pdoNr) - 1u, 5u, &u16);
		if (retVal == RET_OK)  {
			pTrPdo->eventTime = u16;
		}
		retVal = coOdGetDefaultVal_u8((0x1800u + pTrPdo->pdoNr) - 1u, 6u, &u8);
		if (retVal == RET_OK)  {
			pTrPdo->syncStartVal = u8;
		}

# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
		if (pTrPdo->dynMapping == CO_TRUE)  {
			(void)setupPdoTrMapTable(pTrPdo->mapTableIdx,
				(0x1a00u + pTrPdo->pdoNr) - 1u);
		}
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
	}
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
	/* receive PDO */
	for (cnt = 0u; cnt < pdoRecCnt; cnt++)  {
		pRecPdo = &pdoReceive[cnt];

		/* get disable and RTR bit from OD */
		retVal = coOdGetDefaultVal_u32((0x1400u + pRecPdo->pdoNr) - 1u, 1u, &u32);
		if (retVal == RET_OK)  {
			u32 &= (CO_COB_INVALID | CO_COB_RTR_BIT);
		} else {
			u32 = 0u;
		}

		if ((pRecPdo->pdoNr >= 1u)  && (pRecPdo->pdoNr <= 4u))  {
			/* calculate cobs for predefined connection set */
			pdoIdx = pRecPdo->pdoNr - 1u;
			pRecPdo->cobId = u32
				| ((0x200ul + (0x100ul * pdoIdx)) + coNmtGetNodeId());
		} else {
			/* disable sdo */
			pRecPdo->cobId = u32 | CO_COB_INVALID;
		}

		/* transmission type */
		retVal = coOdGetDefaultVal_u8((0x1400u + pRecPdo->pdoNr) - 1u, 2u, &u8);
		if (retVal == RET_OK)  {
			pRecPdo->transType = u8;
		}
		retVal = coOdGetDefaultVal_u16((0x1400u + pRecPdo->pdoNr) - 1u, 3u, &u16);
		if (retVal == RET_OK)  {
			pRecPdo->inhibit = u16;
		}
		retVal = coOdGetDefaultVal_u16((0x1400u + pRecPdo->pdoNr) - 1u, 5u, &u16);
		if (retVal == RET_OK)  {
			pRecPdo->eventTime = u16;
		}

# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
		if (pRecPdo->dynMapping == CO_TRUE)  {
			(void)setupPdoRecMapTable(pRecPdo->mapTableIdx,
					(0x1600u + pRecPdo->pdoNr) - 1u);
		}
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */

	}
#endif /* CO_PDO_RECEIVE_CNT */

	return;
}


/***************************************************************************/
/**
* \brief icoPdoVarInit - init pdo variables
*
*
*/
void icoPdoVarInit(
		UNSIGNED16	*pTrCnt,
		UNSIGNED16	*pRecCnt
	)
{
(void)pTrCnt;
(void)pRecCnt;

#ifdef CO_PDO_TRANSMIT_CNT
	{
		pdoTrCnt = 0u;
	}

# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
	mapTableTrPdoCnt = 0u;
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT

	{
		pdoRecCnt = 0u;
	}

# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
	mapTableRecPdoCnt = 0u;
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */

# ifdef CO_EVENT_PDO_CNT
	pdoEventTableCnt = 0u;
	pdoRecEventTableCnt = 0u;
#  ifdef CO_SYNC_SUPPORTED
	pdoEventSyncTableCnt = 0u;
#  endif /* CO_SYNC_SUPPORTED */
# endif /* CO_EVENT_PDO */
#endif /* CO_PDO_RECEIVE_CNT */
}


#ifdef CO_PDO_TRANSMIT_CNT
/***************************************************************************/
/**
* \brief coPdoTransmitInit - init transmit pdo functionality
*
* This function initializes a transmit PDO.
* The COB-ID is set at reset communication or at load parameter.
*
* Note: All parameters are reset to their default values at reset communication.
*
* \return RET_T
*
*/
RET_T coPdoTransmitInit(
		UNSIGNED16		pdoNr,				/**< PDO number */
		UNSIGNED8		transType,			/**< transmission type */
		UNSIGNED16		inhibit,			/**< inhibit time 100 usec */
		UNSIGNED16		eventTime,			/**< event timer in msec */
		UNSIGNED8		syncStartVal,		/**< sync start value */
		CO_CONST PDO_TR_MAP_TABLE_T	*mapTable	/**< pointer to mapping table */
	)
{
UNSIGNED8	mapLen;
RET_T		retVal = RET_OK;
# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
UNSIGNED16	mapTableIdx;
# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
CO_TR_PDO_T	*pPdo;
UNSIGNED16	idx;

	if (pdoTrCnt >= CO_PDO_TRANSMIT_CNT)  {
		return(RET_INVALID_PARAMETER);
	}

	idx = pdoTrCnt;
	pPdo = &pdoTransmit[idx];

	pPdo->pdoNr = pdoNr;
	pPdo->cob = icoCobCreate(CO_COB_TYPE_TRANSMIT,
			CO_SERVICE_PDO_TRANSMIT, pdoTrCnt);
	if (pPdo->cob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}
	pPdo->transType = transType;
	pPdo->actSyncCnt = transType;
	pPdo->inhibit = inhibit;
	pPdo->eventTime = eventTime;
	pPdo->syncStartVal = syncStartVal;
	pPdo->state = CO_PDO_STATE_READY;

	if (mapTable == NULL)  {
# ifdef CO_TR_PDO_DYN_MAP_ENTRIES
		/* dynamic mapping */
		mapTableIdx = createPdoTrMapTable((0x1a00u + pdoNr) - 1u);
		if (mapTableIdx == 0xffffu)  {
			return(RET_MAP_LEN_ERROR);
		}
		pPdo->mapTable = &mapTablesTrPDO[mapTableIdx];
		pPdo->mapTableConst = pdoTransmit[idx].mapTable;
		pPdo->mapTableIdx = mapTableIdx;
		pPdo->dynMapping = CO_TRUE;

# endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
	} else {
		/* static mapping */
		pPdo->mapTableConst = mapTable;
		pPdo->mapTable = NULL;
		pPdo->dynMapping = CO_FALSE;
#ifdef CO_TR_PDO_DYN_MAP_ENTRIES
		pPdo->mapTableIdx = 0xffffu;
#endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
	}

	/* check mapping table */
	mapLen = checkTrPdoMappingTable(pPdo, 0u);
	if (mapLen <= CO_PDO_MAX_DATA_LEN)  {
		retVal = setPdoCob(pPdo->cob, CO_COB_INVALID, mapLen);
	} else {
		(void)setPdoCob(pPdo->cob, CO_COB_INVALID, 0u);
		retVal = RET_MAP_LEN_ERROR;
	}

	/* only for 254 255 */
	if (transType >= 254u)  {
		(void)icoCobSetInhibit(pPdo->cob, pPdo->inhibit);
	} else {
		(void)icoCobSetInhibit(pPdo->cob, 0u);
	}

	pdoTrCnt++;

	return(retVal);
}
#endif /* CO_PDO_TRANSMIT_CNT */


#ifdef CO_PDO_RECEIVE_CNT
/***************************************************************************/
/**
* \brief coPdoReceiveInit - init receive pdo functionality
*
* This function initializes a receive PDO.
* The COB-ID is set at reset communication or at load parameter.
*
* Note: All parameter are reset by their default values at reset communication.
*
* \return RET_T
*
*/
RET_T coPdoReceiveInit(
		UNSIGNED16	pdoNr,					/**< PDO number */
		UNSIGNED8	transType,				/**< transmission type */
		UNSIGNED16	inhibit,				/**< inhibit time 100 usec */
		UNSIGNED16	eventTime,				/**< event timer in msec */
		CO_CONST PDO_REC_MAP_TABLE_T *mapTable /**< pointer to mapping table */
	)
{
UNSIGNED8	mapLen;
RET_T		retVal = RET_OK;
#ifdef CO_REC_PDO_DYN_MAP_ENTRIES
UNSIGNED16	mapTableIdx;
#endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
UNSIGNED16	idx;
CO_REC_PDO_T	*pPdo;

	if (pdoRecCnt >= CO_PDO_RECEIVE_CNT)  {
		return(RET_INVALID_PARAMETER);
	}

	idx = pdoRecCnt;
	pPdo = &pdoReceive[idx];
	pPdo->pdoNr = pdoNr;
	pPdo->cob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_PDO_RECEIVE, pdoRecCnt);
	if (pPdo->cob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}
	pPdo->transType = transType;
	pPdo->inhibit = inhibit;
	pPdo->eventTime = eventTime;
	pPdo->state = CO_PDO_STATE_READY;

	if (mapTable == NULL)  {
# ifdef CO_REC_PDO_DYN_MAP_ENTRIES
		/* dynamic mapping */
		mapTableIdx = createPdoRecMapTable((0x1600u + pdoNr) - 1u);
		if (mapTableIdx == 0xffffu)  {
			return(RET_MAP_LEN_ERROR);
		}
		pPdo->mapTable = &mapTablesRecPDO[mapTableIdx];
		pPdo->mapTableConst = pdoReceive[idx].mapTable;
		pPdo->mapTableIdx = mapTableIdx;
		pPdo->dynMapping = CO_TRUE;
# endif /* CO_REC_PDO_DYN_MAP_ENTRIES */

	} else {
		/* static mapping */
		pPdo->mapTable = NULL;
		pPdo->mapTableConst = mapTable;
		pPdo->dynMapping = CO_FALSE;
#ifdef CO_REC_PDO_DYN_MAP_ENTRIES
		pPdo->mapTableIdx = 0xffffu;
#endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
	}

	/* check mapping table */
	mapLen = checkRecPdoMappingTable(pPdo, 0u);
	if (mapLen <= CO_PDO_MAX_DATA_LEN)  {
		retVal = setPdoCob(pPdo->cob, CO_COB_INVALID, mapLen);
	} else {
		(void)setPdoCob(pPdo->cob, CO_COB_INVALID, 0u);
		retVal = RET_MAP_LEN_ERROR;
	}

	pdoRecCnt++;

	return(retVal);
}
#endif /* CO_PDO_RECEIVE_CNT */

#endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */
