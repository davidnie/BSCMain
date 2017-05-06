/*
* co_sdoclient.c - contains sdo client routines
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_sdoclient.c 15802 2016-10-28 09:12:24Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief sdo client routines
*
* \file co_sdoclient.c
* contains sdo client routines
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_SDO_CLIENT_CNT
#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#include <co_odaccess.h>
#include <co_nmt.h>
#include <co_sdo.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_odaccess.h"
#include "ico_nmt.h"
# ifdef CO_SDO_BLOCK
#  include "ico_event.h"
# endif /* CO_SDO_BLOCK */
# ifdef CO_SDO_NETWORKING
#  include "ico_sdoserver.h"
# endif /* CO_SDO_NETWORKING */
#include "ico_sdo.h"
#include "ico_sdoclient.h"

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_SDO_CLIENT_READ
# ifdef CO_EVENT_PROFILE_SDO_CLIENT_READ
#  define CO_EVENT_SDO_CLIENT_READ_CNT	(CO_EVENT_DYNAMIC_SDO_CLIENT_READ + CO_EVENT_PROFILE_SDO_CLIENT_READ)
# else /* CO_EVENT_PROFILE_SDO_CLIENT_READ */
#  define CO_EVENT_SDO_CLIENT_READ_CNT	(CO_EVENT_DYNAMIC_SDO_CLIENT_READ)
# endif /* CO_EVENT_PROFILE_SDO_CLIENT_READ */
#else /* CO_EVENT_DYNAMIC_SDO_CLIENT_READ */
# ifdef CO_EVENT_PROFILE_SDO_CLIENT_READ
#  define CO_EVENT_SDO_CLIENT_READ_CNT	(CO_EVENT_PROFILE_SDO_CLIENT_READ)
# endif /* CO_EVENT_PROFILE_SDO_CLIENT_READ */
#endif /* CO_EVENT_DYNAMIC_SDO_CLIENT_READ */


#ifdef CO_EVENT_DYNAMIC_SDO_CLIENT_WRITE
# ifdef CO_EVENT_PROFILE_SDO_CLIENT_WRITE
#  define CO_EVENT_SDO_CLIENT_WRITE_CNT	(CO_EVENT_DYNAMIC_SDO_CLIENT_WRITE + CO_EVENT_PROFILE_SDO_CLIENT_WRITE)
# else /* CO_EVENT_PROFILE_SDO_CLIENT_WRITE */
#  define CO_EVENT_SDO_CLIENT_WRITE_CNT	(CO_EVENT_DYNAMIC_SDO_CLIENT_WRITE)
# endif /* CO_EVENT_PROFILE_SDO_CLIENT_WRITE */
#else /* CO_EVENT_DYNAMIC_SDO_CLIENT_WRITE */
# ifdef CO_EVENT_PROFILE_SDO_CLIENT_WRITE
#  define CO_EVENT_SDO_CLIENT_WRITE_CNT	(CO_EVENT_PROFILE_SDO_CLIENT_WRITE)
# endif /* CO_EVENT_PROFILE_SDO_CLIENT_WRITE */
#endif /* CO_EVENT_DYNAMIC_SDO_CLIENT_WRITE */


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_SDO_CLIENT_READ
extern CO_CONST CO_EVENT_SDO_CLIENT_READ_T coEventSdoClientReadInd;
#endif /* CO_EVENT_STATIC_SDO_CLIENT_READ */

#ifdef CO_EVENT_STATIC_SDO_CLIENT_WRITE
extern CO_CONST CO_EVENT_SDO_CLIENT_WRITE_T coEventSdoClientWriteInd;
#endif /* CO_EVENT_STATIC_SDO_CLIENT_WRITE */


/* list of global defined functions
---------------------------------------------------------------------------*/
static CO_INLINE UNSIGNED16 searchSdoClientIndex(UNSIGNED8 sdoNr);
static CO_INLINE void sdoClientCodeMultiplexer(const CO_SDO_CLIENT_T *pSdo,
		UNSIGNED8 pMp[]);
static void sdoClientReadInit(CO_SDO_CLIENT_T *pSdo,
		const CO_CAN_MSG_T *pRecData);
static void sdoClientReadSegmentReq(CO_SDO_CLIENT_T	*pSdo);
static void sdoClientReadSegment(CO_SDO_CLIENT_T *pSdo,
		const CO_CAN_MSG_T *pRecData);
static void sdoClientWriteInit(CO_SDO_CLIENT_T *pSdo,
		const CO_CAN_MSG_T *pRecData);
static void sdoClientWriteSegment( CO_SDO_CLIENT_T	*pSdo);
static void sdoClientWriteSegmentAnswer(CO_SDO_CLIENT_T	*pSdo,
		const CO_CAN_MSG_T *pRecData);
static RET_T sdoClientSendReadInit(CO_SDO_CLIENT_T *pSdo, BOOL_T blockAllowed);
static RET_T sdoClientSendWriteInit(CO_SDO_CLIENT_T *pSdo, BOOL_T blockAllowed);
static RET_T sdoReadReq(UNSIGNED8	sdoNr, UNSIGNED16 index, UNSIGNED8 subIndex, 
		UNSIGNED8 *pData, UNSIGNED32 dataLen, UNSIGNED16 numeric, 
		UNSIGNED32 timeout, BOOL_T blockReq);
static RET_T sdoWriteReq(UNSIGNED8	sdoNr, UNSIGNED16 index, UNSIGNED8 subIndex, 
		UNSIGNED8 *pData, UNSIGNED32 dataLen, UNSIGNED16 numeric, 
		UNSIGNED32 timeout, BOOL_T blockReq);


/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static UNSIGNED8	sdoClientCnt = { 0u };
static CO_SDO_CLIENT_T	sdoClient[CO_SDO_CLIENT_CNT];
# ifdef CO_EVENT_SDO_CLIENT_READ_CNT
static UNSIGNED16	sdoClientReadTableCnt = 0u;
static CO_EVENT_SDO_CLIENT_READ_T sdoClientReadTable[CO_EVENT_SDO_CLIENT_READ_CNT];
# endif /* CO_EVENT_SDO_CLIENT_READ_CNT */
# ifdef CO_EVENT_SDO_CLIENT_WRITE_CNT
static UNSIGNED16	sdoClientWriteTableCnt = 0u;
static CO_EVENT_SDO_CLIENT_WRITE_T sdoClientWriteTable[CO_EVENT_SDO_CLIENT_WRITE_CNT];
# endif /* CO_EVENT_SDO_CLIENT_WRITE_CNT */


/***************************************************************************/
/**
* \brief sdoCodeMultiplexer - write multiplexer into message format
*
* \internal
*
* \param
*	pointer to sdo structure
*	pointer to array
* \return none
*
*/
static CO_INLINE void sdoClientCodeMultiplexer(
		const CO_SDO_CLIENT_T	*pSdo,
		UNSIGNED8	pMp[]
	)
{
	pMp[0] = (UNSIGNED8)(pSdo->index & 0xffu);
	pMp[1] = (UNSIGNED8)(pSdo->index >> 8u);
	pMp[2] = pSdo->subIndex;
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientHandler - sdo client handler
*
*
* \return none
*
*/
void icoSdoClientHandler(
		const CO_REC_DATA_T	*pRecData		/* pointer to received data */
	)
{
CO_SDO_CLIENT_T	*pSdo;
CO_NMT_STATE_T nmtState;
UNSIGNED32		result = 0u;
CO_SDO_CLIENT_STATE_T	state;
RET_T			retVal;

	nmtState = coNmtGetState();

	/* OPERATIONAL ? */
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return;
	}

	/* check service index */
	if (pRecData->spec >= CO_SDO_CLIENT_CNT)  {
		return;
	}
	pSdo = &sdoClient[pRecData->spec];

	/* transfer actice ? */
	if (pSdo->state == CO_SDO_CLIENT_STATE_FREE)  {
		return;
	}

	/* check for correct message len */
	if (pRecData->msg.len != 8u)  {
		return;
	}


#ifdef CO_SDO_BLOCK
	/* if block transfer is active don't use CCS */
	if (pSdo->state == CO_SDO_CLIENT_STATE_BLK_UL_BLK)  {
		icoSdoClientReadBlock(pSdo, &pRecData->msg);

		return;
	}
#endif /* CO_SDO_BLOCK */

	switch (pRecData->msg.data[0] & CO_SDO_CCS_MASK)  {
		case CO_SDO_CS_ABORT:				/* abort message ? */
			(void)coTimerStop(&pSdo->timer);

			/* if we started block transfer and slave doesn't support it
			   try again with segmented */
			if (pSdo->state == CO_SDO_CLIENT_STATE_BLK_UL_INIT)  {
				retVal = sdoClientSendReadInit(pSdo, CO_FALSE);
				if (retVal == RET_OK)  {
					return;
				}
			}
			if (pSdo->state == CO_SDO_CLIENT_STATE_BLK_DL_INIT)  {
				retVal = sdoClientSendWriteInit(pSdo, CO_FALSE);
				if (retVal == RET_OK)  {
					return;
				}
			}

			/* user indicaton */
			(void)coNumMemcpyPack(&result, &pRecData->msg.data[4], 4u, CO_ATTR_NUM, 0u);
			state = pSdo->state;
			pSdo->state = CO_SDO_CLIENT_STATE_FREE;
			icoSdoClientUserInd(pSdo, state, result);
			break;

		case CO_SDO_SCS_UPLOAD_INIT:		/* init upload */
			sdoClientReadInit(pSdo, &pRecData->msg);
			break;

		case CO_SDO_SCS_UPLOAD_SEGMENT:		/* segment upload */
			sdoClientReadSegment(pSdo, &pRecData->msg);
			break;

		case CO_SDO_SCS_DOWNLOAD_INIT:		/* init download */
			sdoClientWriteInit(pSdo, &pRecData->msg);
			break;

		case CO_SDO_SCS_DOWNLOAD_SEGMENT:	/* segment download */
			sdoClientWriteSegmentAnswer(pSdo, &pRecData->msg);
			break;

#ifdef CO_SDO_BLOCK
		case CO_SDO_SCS_BLOCK_UPLOAD:		/* block upload */
			if ((pRecData->msg.data[0] & CO_SDO_SCS_BLOCK_SS_UL_END) != 0u)  {
				icoSdoClientReadBlockEnd(pSdo, &pRecData->msg);
			} else {
				icoSdoClientReadBlockInit(pSdo, &pRecData->msg);
			}
			break;

		case CO_SDO_SCS_BLOCK_DOWNLOAD:		/* block download */
			switch (pRecData->msg.data[0] & CO_SDO_SCS_BLOCK_SS_DL_MASK)  {
				case CO_SDO_SCS_BLOCK_SS_DL_INIT:
					icoSdoClientWriteBlockInit(pSdo, &pRecData->msg);
					break;
				case CO_SDO_SCS_BLOCK_SS_DL_ACQ:
					icoSdoClientWriteBlockAcq(pSdo, &pRecData->msg);
					break;
				case CO_SDO_SCS_BLOCK_SS_DL_END:
					(void)coTimerStop(&pSdo->timer);
					pSdo->state = CO_SDO_CLIENT_STATE_FREE;
					icoSdoClientUserInd(pSdo,
						CO_SDO_CLIENT_STATE_BLK_DL_END, 0u);
					break;
				default:
					break;
			}
			break;
#endif /* CO_SDO_BLOCK */

#ifdef CO_SDO_NETWORKING
		case CO_SDO_CCS_NET_IND:		/* networking */
			if ((pSdo->state == CO_SDO_CLIENT_STATE_NETWORK_READ_REQ)
			 || (pSdo->state == CO_SDO_CLIENT_STATE_NETWORK_WRITE_REQ))  {
				icoSdoClientNetworkCon(pSdo, &pRecData->msg); 
				return;
			}
			break;
#endif /* CO_SDO_NETWORKING */

		default:
			break;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief  sdoClientReadInit
*
*
* \return none
*
*/
static void sdoClientReadInit(
		CO_SDO_CLIENT_T	*pSdo,				/* pointer to SDO */
		const CO_CAN_MSG_T	*pRecData		/* pointer to receive data */
	)
{
UNSIGNED32	len = 0u;
UNSIGNED8	u8;
UNSIGNED8	dataOffs = 4u;
UNSIGNED16	index;

	if (pSdo->state != CO_SDO_CLIENT_STATE_UPLOAD_INIT) {
		/* ignore message */
		return;
	}

	/* delete timer */
	(void)coTimerStop(&pSdo->timer);

	/* check for correct index/subindex */
	index = (UNSIGNED16)((UNSIGNED16)pRecData->data[2] << 8u);
	index |= pRecData->data[1];
	if ((pSdo->index != index) || (pSdo->subIndex != pRecData->data[3]))  {
		/* send abort */
		icoSdoClientAbort(pSdo, RET_SDO_INVALID_VALUE);
		return;
	}

	/* get data len */
	switch (pRecData->data[0] & (CO_SDO_SCS_EXPEDITED | CO_SDO_SCS_CONT_FLAG)) {
		case 0u:			/* reserved */
			len = 0xffffffffu;
			break;
		case 1u:			/* segmented transfer, len in byte 4..7 */
			(void)coNumMemcpyPack(&len, &pRecData->data[4], 4u, CO_ATTR_NUM, 0u);
			pSdo->state = CO_SDO_CLIENT_STATE_UPLOAD_SEGMENT;
			break;
		case 2u:			/* unspecified data length */
			len = pSdo->size;
			break;
		case 3u:			/* date len in bit 2..3 */
			u8 = (pRecData->data[0] & 0xcu) >> 2;
			len = 4ul - u8;
			break;
		default:
			len = 0ul;
			break;
	}

	/* received len > as internal buffer size */
	if (len > pSdo->size)  {
		/* send abort */
		icoSdoClientAbort(pSdo, RET_DATA_TYPE_MISMATCH);
		return;
	}

	/* segmented */
	if (pSdo->state == CO_SDO_CLIENT_STATE_UPLOAD_SEGMENT)   {
		pSdo->size = 0u;
		pSdo->restSize = len;
		pSdo->toggle = 0u;
		sdoClientReadSegmentReq(pSdo);
		return;
	}

	/* lock OD */
	CO_OS_LOCK_OD();

	(void)coNumMemcpyPack(pSdo->pData, &pRecData->data[dataOffs], len,
			pSdo->numeric, 0u);

	/* unlock OD */
	CO_OS_UNLOCK_OD();

	/* correct received size len */
	pSdo->size = len;
	pSdo->state = CO_SDO_CLIENT_STATE_FREE;

	icoSdoClientUserInd(pSdo, CO_SDO_CLIENT_STATE_UPLOAD_INIT, 0u);
}


/***************************************************************************/
/**
* \internal
+
* \brief sdoClientReadSegmentReq - request next segment
*
*
* \return none
*
*/
static void sdoClientReadSegmentReq(
		CO_SDO_CLIENT_T	*pSdo			/* pointer to sdo */
	)
{
CO_TRANS_DATA_T	trData;

	/* SCS - number of bytes specified */
	trData.data[0] = CO_SDO_CCS_UPLOAD_SEGMENT | pSdo->toggle;

	memset(&trData.data[1], 0, 7u);

	/* transmit request */
	(void)icoTransmitMessage(pSdo->trCob, &trData, 0u);

	/* start timer */
	(void)coTimerStart(&pSdo->timer, pSdo->timeOutVal, icoSdoClientTimeOut, pSdo, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */

}


/***************************************************************************/
/**
* \internal
*
* \brief sdoClientReadSegment - sdo client read segment
*
*
* \return none
*
*/
static void sdoClientReadSegment(
		CO_SDO_CLIENT_T	*pSdo,				/* pointer to sdo */
		const CO_CAN_MSG_T	*pRecData		/* pointer to receive data */
	)
{
UNSIGNED32	len = 0u;
UNSIGNED8	packOffset = 0u;
UNSIGNED8	u8;

	if (pSdo->state != CO_SDO_CLIENT_STATE_UPLOAD_SEGMENT)  {
		/* ignore message */
		return;
	}

	/* delete timer */
	(void)coTimerStop(&pSdo->timer);

	/* check toggle bit */
	if (((pRecData->data[0] & CO_SDO_CCS_TOGGLE_BIT)) != pSdo->toggle)  {
		icoSdoClientAbort(pSdo, RET_TOGGLE_MISMATCH);
		return;
	}
	if (pSdo->toggle != 0u)  {
		pSdo->toggle = 0u;
	} else {
		pSdo->toggle = CO_SDO_CCS_TOGGLE_BIT;
	}

	/* more data ? */
	if ((pRecData->data[0] & CO_SDO_SCS_CONT_FLAG) != 0u)  {
		/* last segment */
		u8 = (pRecData->data[0] & CO_SDO_SCS_LEN_MASK) >> 1u;
		len = 7ul - u8;
		pSdo->state = CO_SDO_CLIENT_STATE_FREE;
	} else {
		len = 7u;
	}

	/* received len > as internal buffer size */
	if (len > pSdo->restSize)  {
		/* send abort */
		icoSdoClientAbort(pSdo, RET_DATA_TYPE_MISMATCH);
		return;
	}

#ifdef CO_CPU_DSP
	if (pSdo->domain == CO_TRUE)  {
		packOffset = pSdo->size % CO_CPU_DSP_BYTESIZE;
	}
#endif /* CO_DSP32 */

	/* lock OD */
	CO_OS_LOCK_OD();

	(void)coNumMemcpyPack(pSdo->pData, &pRecData->data[1], len, pSdo->numeric,
		packOffset);

	/* lock OD */
	CO_OS_UNLOCK_OD();

	pSdo->pData += len;	/*lint !e960 Note pointer arithmetic other than array indexing used */
	pSdo->restSize -= len;
	pSdo->size += len;

	if (pSdo->state == CO_SDO_CLIENT_STATE_FREE)  {
		icoSdoClientUserInd(pSdo, CO_SDO_CLIENT_STATE_UPLOAD_SEGMENT, 0u);
	} else {
		sdoClientReadSegmentReq(pSdo);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief  sdoClientWriteInit
*
*
* \return none
*
*/
static void sdoClientWriteInit(
		CO_SDO_CLIENT_T	*pSdo,			/* pointer to sdo */
		const CO_CAN_MSG_T *pRecData
	)
{
UNSIGNED16	index;

	if (pSdo->state != CO_SDO_CLIENT_STATE_DOWNLOAD_INIT) {
		/* ignore message */
		return;
	}

	/* delete timer */
	(void)coTimerStop(&pSdo->timer);

	/* check for correct index/subindex */
	index = ((UNSIGNED16)pRecData->data[2] << 8);
	index |= pRecData->data[1];
	if ((pSdo->index != index) || (pSdo->subIndex != pRecData->data[3]))  {
		/* send abort */
		icoSdoClientAbort(pSdo, RET_SDO_INVALID_VALUE);
		return;
	}

	/* expetided transfer ? */
	if (pSdo->size == 0u)  {
		pSdo->state = CO_SDO_CLIENT_STATE_FREE;
		icoSdoClientUserInd(pSdo, CO_SDO_CLIENT_STATE_DOWNLOAD_INIT, 0u);
		return;
	}

	/* next segment */
	sdoClientWriteSegment(pSdo);

	pSdo->state = CO_SDO_CLIENT_STATE_DOWNLOAD_SEGMENT;
}


/***************************************************************************/
/**
* \internal
*
* \brief sdoClientWriteSegment
*
*
* \return none
*
*/
static void sdoClientWriteSegment(
		CO_SDO_CLIENT_T	*pSdo			/*  pointer to sdo */
	)
{
CO_TRANS_DATA_T	trData;
UNSIGNED8	len;
UNSIGNED8	u8;
UNSIGNED8	packOffset = 0u;
UNSIGNED8	dataStartIdx = 1u;

	trData.data[0] = CO_SDO_CCS_DOWNLOAD_SEGMENT;
	trData.data[0] |= pSdo->toggle;
	if (pSdo->size > 7u)  {
		len = 7u;
	} else {
		len = (UNSIGNED8)pSdo->size;
		u8 = (((7u - len) & 0x7u) << 1);
		trData.data[0] |= (u8 | CO_SDO_CCS_CONT_BIT);
		memset(&trData.data[1], 0, 7u);
	}

# ifdef CO_CPU_DSP
	if (pSdo->domain == CO_TRUE)  {
		packOffset = pSdo->restSize % CO_CPU_DSP_BYTESIZE;
	}
# endif /* CO_CPU_DSP */

	coNumMemcpyUnpack(&trData.data[dataStartIdx], pSdo->pData, (UNSIGNED32)len,
		pSdo->numeric, packOffset);
	pSdo->size -= len;
	pSdo->restSize += len;
	pSdo->pData += len;	/*lint !e960 Note pointer arithmetic other than array indexing used */

#ifdef CO_EVENT_CSDO_DOMAIN_WRITE
	/* splitted domain transfer ? */
	if (pSdo->split == CO_TRUE)  {
		/* size reached or last segment */
		if (((pSdo->restSize % (7 * pSdo->msgCnt)) == 0)
			/*|| ((pRecData->data[0] & CO_SDO_CCS_CONT_BIT) != 0u)*/)  {
			/* domain indication */
			if (pSdo->pFunction != NULL)  {
			RET_T retVal;
				retVal = pSdo->pFunction(pSdo->sdoNr, pSdo->index, 
					pSdo->subIndex, pSdo->restSize, pSdo->pFctData);
				/* application decided to abort */
				if (retVal != RET_OK)  {
					icoSdoClientAbort(pSdo, retVal);
					return;
				}
				pSdo->pData -= (7 * pSdo->msgCnt);
			}
		}
	}
#endif /* CO_EVENT_CSDO_DOMAIN_WRITE */

	/* transmit request */
	(void)icoTransmitMessage(pSdo->trCob, &trData, 0u);

	/* start timer */
	(void)coTimerStart(&pSdo->timer, pSdo->timeOutVal,
			icoSdoClientTimeOut, pSdo, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
}


/***************************************************************************/
/**
* \internal
*
* \brief sdoClientWriteSegmentAnswer
*
*
* \return none
*
*/
static void sdoClientWriteSegmentAnswer(
		CO_SDO_CLIENT_T		*pSdo,			/* pointer to sdo */
		const CO_CAN_MSG_T	*pRecData		/* pointer to receive data */
	)
{
	if (pSdo->state != CO_SDO_CLIENT_STATE_DOWNLOAD_SEGMENT)  {
		/* ignore message */
		return;
	}

	/* delete timer */
	(void)coTimerStop(&pSdo->timer);

	/* check toggle bit */
	if (((pRecData->data[0] & CO_SDO_CCS_TOGGLE_BIT)) != pSdo->toggle)  {
		icoSdoClientAbort(pSdo, RET_TOGGLE_MISMATCH);
		return;
	}
	if (pSdo->toggle != 0u)  {
		pSdo->toggle = 0u;
	} else {
		pSdo->toggle = CO_SDO_CCS_TOGGLE_BIT;
	}

	/* transfer finished ? */
	if (pSdo->size == 0u)  {
		pSdo->state = CO_SDO_CLIENT_STATE_FREE;
		icoSdoClientUserInd(pSdo, CO_SDO_CLIENT_STATE_DOWNLOAD_SEGMENT, 0u);
		return;
	}

	/* next segment */
	sdoClientWriteSegment(pSdo);
}


/***************************************************************************/
/**
*
* \brief coSdoRead - read value by SDO
*
* This function starts a sdo transfer with the given parameters.
* The data are stored at the given pointer pData
* with a maximal length of dataLen.
* <br>The timeout value given in msec is started with each message transmission.
* <br>The numeric flag is only valid for big-endian transfers.
* If this parameter is set, the data are changed to little endian format.
*
* Before an SDO can be started, it has to be initialized.
* Initialization is done by setup the COB-Ids of this SDO at index 0x128x:1 and 0x128x:2
*
* If SDO block transfer is enabled,
* it will be used automatically if dataLen is larger
* than CO_SDO_BLOCK_MIN_SIZE.
* If the server doesn't support block transfer,
* segmented transfer will be used instead.
*
* \return RET_T
*
*/
RET_T coSdoRead(
		UNSIGNED8		sdoNr,		/**< sdo number */
		UNSIGNED16		index,		/**< index at server OD */
		UNSIGNED8		subIndex,	/**< index at server OD */
		UNSIGNED8		*pData,		/**< pointer to transfer data */
		UNSIGNED32		dataLen,	/**< data len for transfer */
		UNSIGNED16		numeric,	/**< numeric flag (only for big endian) */
		UNSIGNED32		timeout		/**< timeout in msec */
	)
{
RET_T		retVal;

	retVal = sdoReadReq(sdoNr, index, subIndex, pData, dataLen, numeric, timeout, CO_TRUE);

	return(retVal);
}

#ifdef CO_SDO_BLOCK
/***************************************************************************/
/**
*
* \brief coSdoReadSeg - read value by segmented SDO
*
* This function starts a sdo transfer with the given parameters.
* The data are stored at the given pointer pData
* with a maximal length of dataLen.
* <br>The timeout value given in msec is started with each message transmission.
* <br>The numeric flag is only valid for big-endian transfers.
* If this parameter is set, the data are changed to little endian format.
*
* Before an SDO can be started, it has to be initialized.
* Initialization is done by setup the COB-Ids of this SDO at index 0x128x:1 and 0x128x:2
*
* The segmented transfer will be used.
*
* \return RET_T
*
*/
RET_T coSdoReadSeg(
		UNSIGNED8		sdoNr,		/**< sdo number */
		UNSIGNED16		index,		/**< index at server OD */
		UNSIGNED8		subIndex,	/**< index at server OD */
		UNSIGNED8		*pData,		/**< pointer to transfer data */
		UNSIGNED32		dataLen,	/**< data len for transfer */
		UNSIGNED16		numeric,	/**< numeric flag (only for big endian) */
		UNSIGNED32		timeout		/**< timeout in msec */
	)
{
RET_T		retVal;

	retVal = sdoReadReq(sdoNr, index, subIndex, pData, dataLen, numeric, timeout, CO_FALSE);

	return(retVal);
}
#endif /* CO_SDO_BLOCK */

/***************************************************************************/
/**
* \internal
*
* \brief sdoReadReq 
*
* \return RET_T
*
*/
static RET_T sdoReadReq(
		UNSIGNED8		sdoNr,		/* sdo number */
		UNSIGNED16		index,		/* index at server OD */
		UNSIGNED8		subIndex,	/* index at server OD */
		UNSIGNED8		*pData,		/* pointer to transfer data */
		UNSIGNED32		dataLen,	/* data len for transfer */
		UNSIGNED16		numeric,	/* numeric flag (only for big endian) */
		UNSIGNED32		timeout,	/* timeout in msec */
		BOOL_T			blockReq
	)
{
CO_NMT_STATE_T nmtState;
CO_SDO_CLIENT_T	*pSdo;
UNSIGNED16	idx;
RET_T		retVal;

	nmtState = coNmtGetState();

	/* PRE_OPERATIONAL ? */
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return(RET_INVALID_NMT_STATE);
	}

	idx = searchSdoClientIndex(sdoNr);
	if (idx == 0xffffu) {
		return(RET_SDO_INVALID_VALUE);
	}

	pSdo = &sdoClient[idx];
	if (pSdo->state != CO_SDO_CLIENT_STATE_FREE)  {
		return(RET_SERVICE_BUSY);
	}

	pSdo->index = index;
	pSdo->subIndex = subIndex;
	pSdo->pData = pData;
	pSdo->size = dataLen;
	pSdo->numeric = numeric;
	pSdo->timeOutVal = timeout * 1000u;
	pSdo->domain = CO_FALSE;

	retVal = sdoClientSendReadInit(pSdo, blockReq);

	return(retVal);	
	
}

/***************************************************************************/
/**
* \internal
*
* \brief sdoClientSendReadInit 
*
* \return RET_T
*
*/
static RET_T sdoClientSendReadInit(
		CO_SDO_CLIENT_T	*pSdo,
		BOOL_T			blockAllowed
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	/* SCS - number of bytes specified */
	trData.data[0] = CO_SDO_CCS_UPLOAD_INIT;
	trData.data[1] = (UNSIGNED8)(pSdo->index & 0xffu);
	trData.data[2] = (UNSIGNED8)(pSdo->index >> 8u);
	trData.data[3] = pSdo->subIndex;

	memset(&trData.data[4], 0, 4u);
	pSdo->state = CO_SDO_CLIENT_STATE_UPLOAD_INIT;

#ifdef CO_SDO_BLOCK
	/* block transfer allowed ? */
	if (blockAllowed == CO_TRUE)  {
		/* min datalen for block reached ? */
		if (pSdo->size >= CO_SDO_BLOCK_MIN_SIZE)  {
			/* use block transfer */
			/* block init and CRC supported */
			trData.data[0] = CO_SDO_CCS_BLOCK_UPLOAD;
			/* | CO_SDO_CCS_BLOCK_SC_UL_INIT; */
# ifdef CO_SDO_BLOCK_CRC
			trData.data[0] |= CO_SDO_CCS_BLOCK_UL_CRC;
# endif /* CO_SDO_BLOCK_CRC */

			pSdo->blockSize = CO_SDO_BLOCK_SIZE;
			trData.data[4] = CO_SDO_BLOCK_SIZE;
			trData.data[5] = 0u;		/* don't allow to change sdo protocol */

			pSdo->state = CO_SDO_CLIENT_STATE_BLK_UL_INIT;
		}
	}
#else  /* CO_SDO_BLOCK */
	(void)blockAllowed;
#endif /* CO_SDO_BLOCK */

	/* transmit request */
	retVal = icoTransmitMessage(pSdo->trCob, &trData, 0u);
	if (retVal != RET_OK)  {
		pSdo->state = CO_SDO_CLIENT_STATE_FREE;
		return(retVal);
	}

	/* start timer */
	(void)coTimerStart(&pSdo->timer, pSdo->timeOutVal,
			icoSdoClientTimeOut, pSdo, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */

	return(retVal);
}


/***************************************************************************/
/**
* \brief coSdoWrite - Write value by SDO
*
* This function starts a sdo write transfer with the given parameter.
* The data are read from the given pointer pData
* and with a length of dataLen.
* <br>
* The timeout value given in msec is started with each message transmission.
* <br>
* The numeric flag is only valid for big-endian transfers.
* If this parameter is set, the data are changed to little endian format.
*
* Before an SDO can be started, it has to be initialized.
* This is done by setup the COB-Ids of this SDO at index 0x128x:1 and 0x128x:2
*
* If SDO block transfer is enabled,
* it will be used automatically if dataLen is larger
* than CO_SDO_BLOCK_MIN_SIZE.
* If the server doesn't support block transfer,
* segmented transfer will be used instead.
*
* \return RET_T
*
*/
RET_T coSdoWrite(
		UNSIGNED8		sdoNr,		/**< sdo number */
		UNSIGNED16		index,		/**< index at server OD */
		UNSIGNED8		subIndex,	/**< index at server OD */
		UNSIGNED8		*pData,		/**< pointer to transfer data */
		UNSIGNED32		dataLen,	/**< data len for transfer */
		UNSIGNED16		numeric,	/**< numeric flag (only for big endian) */
		UNSIGNED32		timeout		/**< timeout in msec */
	)
{
RET_T		retVal;

	retVal = sdoWriteReq(sdoNr, index, subIndex, pData, dataLen, numeric, timeout, CO_TRUE);
	
	return(retVal);
}

#ifdef CO_SDO_BLOCK
/***************************************************************************/
/**
* \brief coSdoWriteSeg - Write value by segmented SDO
*
* This function starts a sdo write transfer with the given parameter.
* The data are read from the given pointer pData
* and with a length of dataLen.
* <br>
* The timeout value given in msec is started with each message transmission.
* <br>
* The numeric flag is only valid for big-endian transfers.
* If this parameter is set, the data are changed to little endian format.
*
* Before an SDO can be started, it has to be initialized.
* This is done by setup the COB-Ids of this SDO at index 0x128x:1 and 0x128x:2
*
* The segmented transfer will be used.
*
* \return RET_T
*
*/
RET_T coSdoWriteSeg(
		UNSIGNED8		sdoNr,		/**< sdo number */
		UNSIGNED16		index,		/**< index at server OD */
		UNSIGNED8		subIndex,	/**< index at server OD */
		UNSIGNED8		*pData,		/**< pointer to transfer data */
		UNSIGNED32		dataLen,	/**< data len for transfer */
		UNSIGNED16		numeric,	/**< numeric flag (only for big endian) */
		UNSIGNED32		timeout		/**< timeout in msec */
	)
{
RET_T		retVal;

	retVal = sdoWriteReq(sdoNr, index, subIndex, pData, dataLen, numeric, timeout, CO_FALSE);
	
	return(retVal);
}
#endif /* CO_SDO_BLOCK */

#ifdef CO_EVENT_CSDO_DOMAIN_WRITE
RET_T coSdoDomainWrite(
		UNSIGNED8 sdoNr,
		UNSIGNED16 index,
		UNSIGNED8 subIndex,
		UNSIGNED8 *pData,
		UNSIGNED32 dataLen,
		UNSIGNED32 timeout,
		UNSIGNED32 nbrMsg,
		CO_EVENT_SDO_CLIENT_DOMAIN_WRITE_T pFunction,
		void*pFctPara
	)
{
CO_NMT_STATE_T nmtState;
CO_SDO_CLIENT_T	*pSdo;
UNSIGNED16	idx;
RET_T		retVal;

	nmtState = coNmtGetState();

	/* PRE_OPERATIONAL ? */
	if ((nmtState != CO_NMT_STATE_PREOP)
		&& (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return(RET_INVALID_NMT_STATE);
	}

	idx = searchSdoClientIndex(sdoNr);
	if (idx == 0xffffu) {
		return(RET_SDO_INVALID_VALUE);
	}

	pSdo = &sdoClient[idx];
	if (pSdo->state != CO_SDO_CLIENT_STATE_FREE)  {
		return(RET_SERVICE_BUSY);
	}

	pSdo->split = CO_TRUE;
	pSdo->msgCnt = nbrMsg;
	pSdo->pFunction = pFunction;
	pSdo->pFctData = pFctPara;

	pSdo->index = index;
	pSdo->subIndex = subIndex;
	pSdo->pData = pData;
	pSdo->numeric = CO_FALSE;
	pSdo->timeOutVal = timeout * 1000u;
	pSdo->domain = CO_FALSE;
	pSdo->restSize = 0u;
	pSdo->size = dataLen;

	retVal = sdoClientSendWriteInit(pSdo, CO_TRUE);

	return(retVal);
}

#endif /* CO_EVENT_CSDO_DOMAIN_WRITE */

/***************************************************************************/
/**
* \internal
*
* \brief sdoWriteReq
*
* \return RET_T
*
*/
static RET_T sdoWriteReq(
		UNSIGNED8		sdoNr,		/* sdo number */
		UNSIGNED16		index,		/* index at server OD */
		UNSIGNED8		subIndex,	/* index at server OD */
		UNSIGNED8		*pData,		/* pointer to transfer data */
		UNSIGNED32		dataLen,	/* data len for transfer */
		UNSIGNED16		numeric,	/* numeric flag (only for big endian) */
		UNSIGNED32		timeout,	/* timeout in msec */
		BOOL_T			blockReq
	)
{
CO_NMT_STATE_T nmtState;
CO_SDO_CLIENT_T	*pSdo;
UNSIGNED16	idx;
RET_T		retVal;

	nmtState = coNmtGetState();

	/* PRE_OPERATIONAL ? */
	if ((nmtState != CO_NMT_STATE_PREOP)
	 && (nmtState != CO_NMT_STATE_OPERATIONAL))  {
		return(RET_INVALID_NMT_STATE);
	}

	idx = searchSdoClientIndex(sdoNr);
	if (idx == 0xffffu) {
		return(RET_SDO_INVALID_VALUE);
	}

	pSdo = &sdoClient[idx];
	if (pSdo->state != CO_SDO_CLIENT_STATE_FREE)  {
		return(RET_SERVICE_BUSY);
	}

	pSdo->index = index;
	pSdo->subIndex = subIndex;
	pSdo->pData = pData;
	pSdo->numeric = numeric;
	pSdo->timeOutVal = timeout * 1000u;
	pSdo->domain = CO_FALSE;
	pSdo->restSize = 0u;
	pSdo->size = dataLen;
#ifdef CO_EVENT_CSDO_DOMAIN_WRITE
	pSdo->split = CO_FALSE;
	pSdo->pFunction = NULL;
#endif /* CO_EVENT_CSDO_DOMAIN_WRITE */


	retVal = sdoClientSendWriteInit(pSdo, blockReq);
	
	return(retVal);	
}

/***************************************************************************/
/**
* \internal
*
* \brief sdoClientSendWriteInit
*
* \return RET_T
*
*/
static RET_T sdoClientSendWriteInit(
		CO_SDO_CLIENT_T	*pSdo,
		BOOL_T			blockAllowed
	)
{
CO_TRANS_DATA_T	trData;
UNSIGNED8	u8;
RET_T		retVal;
CO_SDO_CLIENT_STATE_T sdoState = CO_SDO_CLIENT_STATE_DOWNLOAD_INIT;

	/* expedited or segmented ? */
	if (pSdo->size < 5u)  {
		/* expedited transfer */
		u8 = ((4u - (UNSIGNED8)pSdo->size) & 0x7u) << 2;
		trData.data[0] = CO_SDO_CCS_DOWNLOAD_INIT | u8 | 3u;
		memset(&trData.data[4], 0, 4u);
		coNumMemcpyUnpack(&trData.data[4], pSdo->pData, pSdo->size,
			pSdo->numeric, 0u);
		pSdo->size = 0u;

	} else {

		memset(&trData.data[4], 0, CO_CAN_MAX_DATA_LEN - 4u);

		/* segmented transfer */
		trData.data[0] = CO_SDO_CCS_DOWNLOAD_INIT | 1u;

# ifdef CO_SDO_BLOCK
		if (blockAllowed == CO_TRUE)  {
			/* determine block or segmented transfer */
			if (pSdo->size >= CO_SDO_BLOCK_MIN_SIZE)  {
				/* try block transfer */
				trData.data[0] = CO_SDO_CCS_BLOCK_DOWNLOAD
					| CO_SDO_CCS_BLOCK_DL_SIZE /*| CO_SDO_CCS_BLOCK_CS_DL_INIT*/;
#  ifdef CO_SDO_BLOCK_CRC
				trData.data[0] |= CO_SDO_CCS_BLOCK_DL_CRC;
#  endif /* CO_SDO_BLOCK_CRC */
				sdoState = CO_SDO_CLIENT_STATE_BLK_DL_INIT;
			}
		}
# else /* CO_SDO_BLOCK */
		(void)blockAllowed;
# endif /*  CO_SDO_BLOCK */

		/* size information */	
		coNumMemcpyUnpack(&trData.data[4], &pSdo->size, 4u, CO_ATTR_NUM, 0u);

		pSdo->toggle = 0u;
	}

	/* SCS - number of bytes specified */
	trData.data[1] = (UNSIGNED8)(pSdo->index & 0xffu);
	trData.data[2] = (UNSIGNED8)(pSdo->index >> 8);
	trData.data[3] = pSdo->subIndex;

	/* transmit request */
	retVal = icoTransmitMessage(pSdo->trCob, &trData, 0u);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* start timer */
	(void)coTimerStart(&pSdo->timer, pSdo->timeOutVal,
			icoSdoClientTimeOut, pSdo, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */

	pSdo->state = sdoState;

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientTimeOut
*
*
* \return none
*
*/
void icoSdoClientTimeOut(
		void			*pData		/* pointer to sdo client data */
	)
{
CO_SDO_CLIENT_T	*pSdo = (CO_SDO_CLIENT_T *)pData;

	icoSdoClientAbort(pSdo, RET_SDO_TIMEOUT);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientAbort - abort sdo transfer
*
*	
* \return none
*
*/
void icoSdoClientAbort(
		CO_SDO_CLIENT_T	*pSdo,		/* pointer to sdo client */
		RET_T	errorReason			/* error reason */
    )
{
typedef struct {
	RET_T	reason;			/* error reason */
	UNSIGNED32	abortCode;	/* sdo abort code */
} SDO_ABORT_CODE_TABLE_T;
static const SDO_ABORT_CODE_TABLE_T	abortCodeTable[] = {
	{ RET_TOGGLE_MISMATCH,				0x05030000ul },
	{ RET_DATA_TYPE_MISMATCH,			0x06070010ul },
	{ RET_SDO_TIMEOUT,					0x05040000ul },
	{ RET_SDO_WRONG_BLOCKSIZE,			0x05040002ul },
	{ RET_SDO_WRONG_SEQ_NR,				0x05040003ul },
	{ RET_SDO_CRC_ERROR,				0x05040004ul },
	{ RET_SDO_INVALID_VALUE,			0x06090030ul },
};
CO_TRANS_DATA_T	trData;
UNSIGNED32	abortCode = 0x08000000ul;
UNSIGNED16	i;
CO_SDO_CLIENT_STATE_T	state;

	for (i = 0u; i < (sizeof(abortCodeTable) / sizeof(SDO_ABORT_CODE_TABLE_T)); i++) {
		if (abortCodeTable[i].reason == errorReason)  {
			abortCode = abortCodeTable[i].abortCode;
			break;
		}
	}

	trData.data[0] = CO_SDO_CS_ABORT;
#ifdef CO_SDO_NETWORKING
	if ((pSdo->state == CO_SDO_CLIENT_STATE_NETWORK_READ_REQ)
	 || (pSdo->state == CO_SDO_CLIENT_STATE_NETWORK_WRITE_REQ))  {
		trData.data[1] = (UNSIGNED8)(pSdo->networkNr & 0xffu);
		trData.data[2] = (UNSIGNED8)(pSdo->networkNr >> 8u);
		trData.data[3] = pSdo->networkNode;
	} else
#endif /* CO_SDO_NETWORKING */
	{
		sdoClientCodeMultiplexer(pSdo, &trData.data[1]);
	}
	coNumMemcpyUnpack(&trData.data[4], &abortCode, 4u, CO_ATTR_NUM, 0u);
	(void)icoTransmitMessage(pSdo->trCob, &trData, 0u);

	state = pSdo->state;
	pSdo->state = CO_SDO_CLIENT_STATE_FREE;

	icoSdoClientUserInd(pSdo, state, abortCode);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#ifdef CO_EVENT_SDO_CLIENT_READ_CNT
/***************************************************************************/
/**
* \brief coEventRegister_SdoClientRead - register SDO client read event
*
* This function registers the sdo read indication function.
* It is called after a SDO read,
* started by coSdoRead() was finished.
*
* \return RET_T
*
*/
RET_T coEventRegister_SDO_CLIENT_READ(
		CO_EVENT_SDO_CLIENT_READ_T pFunction	/**< pointer to function */
    )
{
	if (sdoClientReadTableCnt >= CO_EVENT_SDO_CLIENT_READ_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	sdoClientReadTable[sdoClientReadTableCnt] = pFunction;
	sdoClientReadTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_SDO_CLIENT_READ_CNT */


#ifdef CO_EVENT_SDO_CLIENT_READ_CNT
/***************************************************************************/
/**
* \brief coEventUnregister_SDO_CLIENT_READ - unregister SDO client read event
*
* This function unregisters the sdo read indication function.
*
* \return RET_T
*
*/
RET_T coEventUnregister_SDO_CLIENT_READ(
	CO_EVENT_SDO_CLIENT_READ_T pFunction	/**< pointer to function */
	)
{
RET_T retVal = RET_OK;
UNSIGNED16 idx = 0xffffu;
UNSIGNED16 u16;

	for (u16 = 0u; u16 < sdoClientReadTableCnt; u16++)  {
		if (sdoClientReadTable[u16] == pFunction)  {
			idx = u16;
			break;
		}
	}

	if (idx != 0xffffu)  {
		for (u16 = idx; u16 < (sdoClientReadTableCnt - 1u); u16++)  {
			sdoClientReadTable[u16] = sdoClientReadTable[u16 + 1u]; /*lint !e796 Conceivable access of out-of-bounds pointer */
		}
		sdoClientReadTableCnt--;
	} else {
		retVal = RET_NOT_INITIALIZED;
	}

	return(retVal);
}
#endif /* CO_EVENT_SDO_CLIENT_READ_CNT */


#ifdef CO_EVENT_SDO_CLIENT_WRITE_CNT
/***************************************************************************/
/**
* \brief coEventRegister_SdoClientWrite - register SDO client write event
*
* This function registers the sdo write indication function.
* It is called after a SDO write,
* started by coSdoWrite() was finished.
*
* \return RET_T
*
*/
RET_T coEventRegister_SDO_CLIENT_WRITE(
		CO_EVENT_SDO_CLIENT_WRITE_T pFunction
    )
{
	if (sdoClientWriteTableCnt >= CO_EVENT_SDO_CLIENT_WRITE_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	sdoClientWriteTable[sdoClientWriteTableCnt] = pFunction;
	sdoClientWriteTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_SDO_CLIENT_WRITE_CNT */


#ifdef CO_EVENT_SDO_CLIENT_WRITE_CNT
/***************************************************************************/
/**
* \brief coEventUnregister_SDO_CLIENT_WRITE - unregister SDO client write event
*
* This function unregisters the sdo write indication function.
*
* \return RET_T
*
*/RET_T coEventUnregister_SDO_CLIENT_WRITE(
	CO_EVENT_SDO_CLIENT_WRITE_T pFunction	/**< pointer to function */
	)
{
RET_T retVal = RET_OK;
UNSIGNED16 idx = 0xffffu;
UNSIGNED16 u16;

	for (u16 = 0u; u16 < sdoClientWriteTableCnt; u16++)  {
		if (sdoClientWriteTable[u16] == pFunction)  {
			idx = u16;
			break;
		}
	}

	if (idx != 0xffffu)  {
		for (u16 = idx; u16 < (sdoClientWriteTableCnt - 1u); u16++)  {
			sdoClientWriteTable[u16] = sdoClientWriteTable[u16 + 1u]; /*lint !e796 Conceivable access of out-of-bounds pointer */
		}
		sdoClientWriteTableCnt--;
	}
	else {
		retVal = RET_NOT_INITIALIZED;
	}

	return(retVal);
}
#endif /* CO_EVENT_SDO_CLIENT_WRITE_CNT */


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientUserInd - call sdo client user indication
*
* \return none
*
*/
CO_INLINE void icoSdoClientUserInd(
		const CO_SDO_CLIENT_T	*pSdo,		/* pointer to client sdo */
		CO_SDO_CLIENT_STATE_T	state,		/* sdo client state */
		UNSIGNED32		result				/* result of the transfer */
	)
{
#if defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_SDO_CLIENT_WRITE_CNT) || defined(CO_SDO_QUEUE) || defined(CO_EVENT_STATIC_SDO_CLIENT_READ)
UNSIGNED16	index;
UNSIGNED8	subIndex;
# if defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_SDO_CLIENT_WRITE_CNT)
UNSIGNED16	cnt;
# endif /* defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_SDO_CLIENT_WRITE_CNT) */
# if defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_STATIC_SDO_CLIENT_READ)
UNSIGNED32	size;
# endif /* CO_EVENT_SDO_CLIENT_READ_CNT */


	/* may be somewhere starts a new SDO transfer at indication,
	 * then the index/subindex are changed before next indication fct
	 * is called, and get wrong index/subindex
	 * Therefore save the data before get to indication
	 */
	index = pSdo->index;
	subIndex = pSdo->subIndex;
# if defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_STATIC_SDO_CLIENT_READ)
	size = pSdo->size;
# endif /* CO_EVENT_SDO_CLIENT_READ_CNT */
#endif /* defined(CO_EVENT_SDO_CLIENT_READ_CNT) || defined(CO_EVENT_SDO_CLIENT_WRITE_CNT) */

	switch (state)  {
		case CO_SDO_CLIENT_STATE_UPLOAD_INIT:
		case CO_SDO_CLIENT_STATE_UPLOAD_SEGMENT:
#ifdef CO_SDO_BLOCK
		case CO_SDO_CLIENT_STATE_BLK_UL_INIT:
		case CO_SDO_CLIENT_STATE_BLK_UL_END:
		case CO_SDO_CLIENT_STATE_BLK_UL_BLK:
#endif /* CO_SDO_BLOCK */
#ifdef CO_SDO_NETWORKING
		case CO_SDO_CLIENT_STATE_NETWORK_READ_REQ:
#endif /* CO_SDO_NETWORKING */

#ifdef CO_SDO_QUEUE
			icoSdoClientQueueInd(pSdo->sdoNr, index, subIndex, result);
#endif /* CO_SDO_QUEUE */

			/* user indication */
#ifdef CO_EVENT_SDO_CLIENT_READ_CNT
			cnt = sdoClientReadTableCnt;
			while (cnt > 0u)  {
				cnt--;
				/* call user indication */
				sdoClientReadTable[cnt](pSdo->sdoNr, index, subIndex,
					size, result);
			}
#endif /* CO_EVENT_SDO_CLIENT_READ_CNT */

#ifdef CO_EVENT_STATIC_SDO_CLIENT_READ
			coEventSdoClientReadInd(pSdo->sdoNr, index, subIndex,
					size, result);
#endif /* CO_EVENT_STATIC_SDO_CLIENT_READ */
			break;

		case CO_SDO_CLIENT_STATE_DOWNLOAD_INIT:
		case CO_SDO_CLIENT_STATE_DOWNLOAD_SEGMENT:
#ifdef CO_SDO_BLOCK
		case CO_SDO_CLIENT_STATE_BLK_DL_INIT:
		case CO_SDO_CLIENT_STATE_BLK_DL:
		case CO_SDO_CLIENT_STATE_BLK_DL_ACQ:
		case CO_SDO_CLIENT_STATE_BLK_DL_END:
#endif /* CO_SDO_BLOCK */
#ifdef CO_SDO_NETWORKING
		case CO_SDO_CLIENT_STATE_NETWORK_WRITE_REQ:
#endif /* CO_SDO_NETWORKING */

#ifdef CO_SDO_QUEUE
			icoSdoClientQueueInd(pSdo->sdoNr, index, subIndex, result);
#endif /* CO_SDO_QUEUE */

#ifdef CO_EVENT_SDO_CLIENT_WRITE_CNT
			/* user indication */
			cnt = sdoClientWriteTableCnt;
			while (cnt > 0u)  {
				cnt--;
				/* call user indication */
				sdoClientWriteTable[cnt](pSdo->sdoNr, index, subIndex,
					result);
			}
#endif /* CO_EVENT_SDO_CLIENT_WRITE_CNT */

#ifdef CO_EVENT_STATIC_SDO_CLIENT_WRITE
			coEventSdoClientWriteInd(pSdo->sdoNr, index, subIndex,
					result);
#endif /* CO_EVENT_STATIC_SDO_CLIENT_WRITE */
			break;

		case CO_SDO_CLIENT_STATE_FREE:
		default:
			break;
	}
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientGetObjectAddr - get sdo client object address
*
* \return pointer to address
*
*/
void *icoSdoClientGetObjectAddr(
		UNSIGNED8		sdoNr,		/* sdo client number */
		UNSIGNED8		subIndex	/* subindex */
	)
{
UNSIGNED16	idx;
void	*pAddr = NULL;

    idx = searchSdoClientIndex(sdoNr);
	if (idx == 0xffffu)  {
		return(NULL);
	}

	switch (subIndex)  {
		case 1u:
			/* copy from cob handler */
			pAddr = (void *)&sdoClient[idx].trCobId;
			break;
		case 2u:
			/* copy from cob handler */
			pAddr = (void *)&sdoClient[idx].recCobId;
			break;
		case 3u:
			pAddr = (void *)&sdoClient[idx].node;
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
* \brief icoSdoCheckObjLimitNode - check object limits node entry
*
*
* \return RET_T
*
*/
RET_T icoSdoClientCheckObjLimitNode(
		UNSIGNED16		sdoNr		/* sdo number */
	)
{
RET_T	retVal = RET_OK;
UNSIGNED16	idx;

    idx = searchSdoClientIndex((UNSIGNED8)sdoNr);
	if (idx == 0xffffu)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientCheckObjLimitCobId - check cob-id
*
*
* \return RET_T
*
*/
RET_T icoSdoClientCheckObjLimitCobId(
		UNSIGNED16		sdoNr,		/* sdo number */
		UNSIGNED8		subIndex,	/* sub index */
		UNSIGNED32		canId		/* pointer to receive data */
	)
{
RET_T	retVal = RET_OK;
UNSIGNED16	idx;
UNSIGNED32	cobId;

    idx = searchSdoClientIndex((UNSIGNED8)sdoNr);
	if (idx == 0xffffu)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	if (subIndex == 2u)  {
		if ((canId & CO_COB_VALID_MASK) == 0u)  {
			/* new cobid is valid, only allowed if cob was disabled before*/
			cobId = icoCobGet(sdoClient[idx].recCob);
			if ((cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
				return(RET_SDO_INVALID_VALUE);
			}
			if (icoCheckRestrictedCobs(canId, 0x581u, 0x5ffu) == CO_TRUE)  {
				return(RET_SDO_INVALID_VALUE);
			}
		}
	} else 
	if (subIndex == 1u)  {
		if ((canId & CO_COB_VALID_MASK) == 0u)  {
			/* new cobid is valid, only allowed if cob was disabled before*/
			cobId = icoCobGet(sdoClient[idx].trCob);
			if ((cobId & CO_COB_VALID_MASK) != CO_COB_INVALID)  {
				return(RET_SDO_INVALID_VALUE);
			}
			if (icoCheckRestrictedCobs(canId, 0x601u, 0x6ffu) == CO_TRUE)  {
				return(RET_SDO_INVALID_VALUE);
			}
		}
	} else {
		retVal = RET_SUBIDX_NOT_FOUND;
	}

	/* value 0 isnt allowed */
	if (canId == 0u)  {
		return(RET_SDO_INVALID_VALUE);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientObjChanged - sdo object was changed
*
*
* \return RET_T
*
*/
RET_T icoSdoClientObjChanged(
		UNSIGNED16		sdoNr,			/* sdo number */
		UNSIGNED8		subIndex		/* subindex */
	)
{
UNSIGNED16	idx;

/*printf("sdo object changed indication %x:%d\n", 0x1200 + sdoNr- 1, subIndex); */
    idx = searchSdoClientIndex((UNSIGNED8)sdoNr);
	if (idx == 0xffffu)  {
		return(RET_SDO_TRANSFER_NOT_SUPPORTED);
	}

	switch (subIndex)  {
		case 2u:
			(void)icoCobSet(sdoClient[idx].recCob, sdoClient[idx].recCobId,
					CO_COB_RTR_NONE, 8u);
			break;
		case 1u:
			(void)icoCobSet(sdoClient[idx].trCob, sdoClient[idx].trCobId,
					CO_COB_RTR_NONE, 8u);
			break;

		default:
			break;
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief
*
*
* \return if sdo client is in use
*
*/
CO_SDO_CLIENT_T	*icoSdoClientPtr(
		UNSIGNED8		sdoNr
	)
{
UNSIGNED16	idx;

	idx = searchSdoClientIndex(sdoNr);
	if (idx == 0xffffu) {
		return(NULL);
	}

	return(&sdoClient[idx]);

}


/***************************************************************************/
/**
* \internal
*
* \brief searchSdoClientIndex - search for sdo server index
*
*
* \return client index
*
*/
static CO_INLINE UNSIGNED16 searchSdoClientIndex(
		UNSIGNED8	sdoNr
	)
{
UNSIGNED16	cnt;

	for (cnt = 0u; cnt < sdoClientCnt; cnt++)  {
		if (sdoNr == sdoClient[cnt].sdoNr)  {
			return(cnt);
		}
	}

	return(0xffffu);
}


/***************************************************************************/
/**
* \internal
*
* \brief coResetSdoClient - reset comm for sdo client
*
* \return none
*
*/
void icoSdoClientReset(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
UNSIGNED16	idx;

	for (cnt = 0u; cnt < sdoClientCnt; cnt++)  {
		idx = cnt;

		/* if sdo transfer is active, send aborted to the application */
		if (sdoClient[idx].state != CO_SDO_CLIENT_STATE_FREE)  {
			icoSdoClientUserInd(&sdoClient[idx], sdoClient[idx].state,
				0xffffffffu);

			sdoClient[idx].state = CO_SDO_CLIENT_STATE_FREE;
		}

		/* ignore temporary cob-ids */
		if ((sdoClient[idx].recCobId & CO_SDO_DYN_BIT) != 0u)  {
			sdoClient[idx].recCobId = CO_COB_INVALID;
			sdoClient[idx].trCobId = CO_COB_INVALID;
		}

		(void)icoCobSet(sdoClient[idx].trCob, sdoClient[idx].trCobId,
				CO_COB_RTR_NONE, 8u);
		(void)icoCobSet(sdoClient[idx].recCob, sdoClient[idx].recCobId,
				CO_COB_RTR_NONE, 8u);
	}

	return;
}


/***************************************************************************/
/**
* \internal
*
* \brief coResetSdoClient - reset comm for sdo client
*
* \return none
*
*/
void icoSdoClientSetDefaultValue(
		void	/* no parameter */
	)
{
UNSIGNED16	cnt;
UNSIGNED16	idx;

	for (cnt = 0u; cnt < sdoClientCnt; cnt++)  {
		idx = cnt;

		/* disable sdo */
		sdoClient[idx].trCobId = CO_COB_INVALID;
		sdoClient[idx].recCobId = CO_COB_INVALID;
		(void)coOdGetDefaultVal_u8(0x1280u + cnt, 3u, &sdoClient[idx].node);
	}
}


/***************************************************************************/
/*
* \brief icoSdoClientVarInit - init sdo client variables
*
*/
void icoSdoClientVarInit(
		CO_CONST UNSIGNED8	*pList		/* line counts */
	)
{
(void)pList;

	{
		sdoClientCnt = 0u;
	}

	memset(&sdoClient[0], 0, sizeof(sdoClient));

# ifdef CO_EVENT_SDO_CLIENT_READ_CNT
	sdoClientReadTableCnt = 0u;
# endif /* CO_EVENT_SDO_CLIENT_READ_CNT */

# ifdef CO_EVENT_SDO_CLIENT_WRITE_CNT
	sdoClientWriteTableCnt = 0u;
# endif /* CO_EVENT_SDO_CLIENT_WRITE_CNT */
}


/***************************************************************************/
/**
* \brief coInitSdoClient - init SDO client functionality
*
* This function initializes the SDO client with the given number.
*
* \return RET_T
*
*/
RET_T coSdoClientInit(
		UNSIGNED8		clientNr	/**< sdo client number */
	)
{
UNSIGNED16      idx;

	if (sdoClientCnt >= CO_SDO_CLIENT_CNT)  {
		return(RET_INVALID_PARAMETER);
	}

	idx = sdoClientCnt;

	sdoClient[idx].sdoNr = clientNr;
	sdoClient[idx].trCob =
			icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_SDO_CLIENT,
					(UNSIGNED16)sdoClientCnt);
	if (sdoClient[idx].trCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}
	sdoClient[idx].recCob =
			icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_SDO_CLIENT,
					(UNSIGNED16)sdoClientCnt);
	if (sdoClient[idx].recCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	sdoClientCnt++;

	return(RET_OK);
}

#endif /* CO_SDO_CLIENT_CNT */
