/*
* co_sdoqueue.c - sdo queue
*
* Copyright (c) 2013-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_sdoqueue.c 13648 2016-04-28 13:02:25Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief SDO handling with queuing
*
* \file co_sdoqueue.c
*
*/


/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>

#ifdef CO_SDO_QUEUE
#include <co_commtask.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_sdo.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_event.h"
#include "ico_sdoclient.h"
#ifdef CO_SDO_NETWORKING
# include "co_odaccess.h"
# include "ico_sdoserver.h"
#endif /* CO_SDO_NETWORKING */
#include "ico_sdo.h"

/* constant definitions
---------------------------------------------------------------------------*/
typedef enum {
	CO_SDO_QUEUE_STATE_NEW,
	CO_SDO_QUEUE_STATE_TRANSMITTING,
	CO_SDO_QUEUE_STATE_FINISHED
} CO_SDO_QUEUE_STATE_T;


/* SDO queue entry */
typedef struct {
	BOOL_T		write;				/* read/write */
	UNSIGNED8	sdoNr;				/* sdo number */
	UNSIGNED16	index;				/* index */
	UNSIGNED8	subIndex;			/* subindex */
	UNSIGNED8	data[4];			/* data (saved for write access) */
	UNSIGNED8	*pData;				/* pointer of data */
	UNSIGNED32	dataLen;			/* datalen */
	CO_SDO_QUEUE_IND_T	pFct;		/* pointer to function */
	void		*pFctPara;			/* function parameter */
	CO_SDO_QUEUE_STATE_T state;		/* internal state */
} EB_SDO_QUEUE_T;


/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void startSdoTransfer(void *ptr);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static EB_SDO_QUEUE_T	sdoBuf[CO_SDO_QUEUE_LEN];
static UNSIGNED16	rdIdx = { 0u };
static UNSIGNED16	wrIdx = { 0u };
static const UNSIGNED32	timeOut = { 1000ul };
static CO_EVENT_T	startSdoEvent;




/***************************************************************************/
/**
*
* \brief coSdoQueueAddTransfer - add sdo transfer to sdo queue handler
*
* This function can be used to add sdo transfers to a queue.
* If a tranfer was finished, the next will start automatically.
* After each transfer, the given function with the parameter are called.
*
* Please note: Only allowed for expedited transfers with initialized sdo channel.
* Transmit data are saved internally.
*
* \return
*	RET_T
*/
RET_T coSdoQueueAddTransfer(
		BOOL_T		write,				/**< write/read access */
		UNSIGNED8	sdoNr,				/**< sdo number */
		UNSIGNED16	index,				/**< index */
		UNSIGNED8	subIndex,			/**< subIndex */
		UNSIGNED8	*pData,				/**< pointer to transfer data */
		UNSIGNED32	dataLen,			/**< len of transfer data */
		CO_SDO_QUEUE_IND_T	pFct,		/**< pointer to finish function */
		void		*pFctPara			/**< pointer to data field for finish function */
	)
{
EB_SDO_QUEUE_T	*pSdoBuf;
UNSIGNED16	idx;

	CO_DEBUG4("coSdoAddTransfer: write: %d, sdoNr: %d, idx: %x:%d, ",
		write, sdoNr, index, subIndex);
	CO_DEBUG4("data %x %x %x %x\n",
		pData[0], pData[1], pData[2], pData[3]);
	CO_DEBUG2("coSdoAddTransfer start: rd: %d wr:%d\n", rdIdx, wrIdx);

	if ((sdoNr < 1u) || (sdoNr > 128u) || (dataLen > 4u))  {
		return(RET_INVALID_PARAMETER);
	}

	idx = wrIdx + 1u;
	if (idx == CO_SDO_QUEUE_LEN)  {
		idx = 0u;
	}
	if (idx == rdIdx)  {
		return(RET_OUT_OF_MEMORY);
	}

	pSdoBuf = &sdoBuf[wrIdx];
	pSdoBuf->write = write;
	pSdoBuf->sdoNr = sdoNr;
	pSdoBuf->index = index;
	pSdoBuf->subIndex = subIndex;
	pSdoBuf->pData =  pData;
	memcpy(&pSdoBuf->data[0], (const void *)pData, (size_t)dataLen);
	pSdoBuf->dataLen = dataLen;
	pSdoBuf->pFct = pFct;
	pSdoBuf->pFctPara = pFctPara;
	pSdoBuf->state = CO_SDO_QUEUE_STATE_NEW;

	wrIdx = idx;

	CO_DEBUG2("ebSdoAddTransfer end: rd: %d wr:%d\n", rdIdx, wrIdx);

	/* try to start transfer */
	startSdoTransfer(NULL);

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief startSdoTransfer - try to start next sdo transfer
*
*
* \return
*	void
*/
static void startSdoTransfer(
		void	*ptr
	)
{
RET_T	retVal;
EB_SDO_QUEUE_T	*pSdoBuf;
(void)ptr;

#ifdef __DEBUG__
{
UNSIGNED8	rd = rdIdx, wr = wrIdx;

	while (rd != wr)  {
		EB_DEBUG("buf %d, state %d, idx %x:%d\n", rd,
			sdoBuf[rd].state,
			sdoBuf[rd].index,
			sdoBuf[rd].subIndex);
		rd++;
		if (rd == CO_SDO_QUEUE_LEN)  {
			rd = 0u;
		}
	}
}
#endif

	CO_DEBUG2("startSdoTransfer start: rd: %d wr:%d\n", rdIdx, wrIdx);

	/* data to transmit ? */
	if (rdIdx != wrIdx)  {
		/* buffer waiting ? */
		pSdoBuf = &sdoBuf[rdIdx];
		if (pSdoBuf->state == CO_SDO_QUEUE_STATE_NEW)  {
			if (pSdoBuf->write == CO_TRUE)  {
				/* write */
				CO_DEBUG3("startSdoTransfer: write sdoNr: %d, idx: %x:%d",
					pSdoBuf->sdoNr, pSdoBuf->index,
					pSdoBuf->subIndex);
				CO_DEBUG4(", data %x %x %x %x\n",
					pSdoBuf->pData[0], pSdoBuf->pData[1],
					pSdoBuf->pData[2], pSdoBuf->pData[3]);

				retVal = coSdoWrite(pSdoBuf->sdoNr,
					pSdoBuf->index, pSdoBuf->subIndex,
					&pSdoBuf->data[0], pSdoBuf->dataLen, 0u, timeOut);
			} else {
				/* read */
				CO_DEBUG3("startSdoTransfer: read sdoNr: %d, idx: %x:%d\n",
					pSdoBuf->sdoNr, pSdoBuf->index,
					pSdoBuf->subIndex);
				/* check for internal transfers */
				retVal = coSdoRead(pSdoBuf->sdoNr,
					pSdoBuf->index, pSdoBuf->subIndex,
					pSdoBuf->pData, pSdoBuf->dataLen, 0u, timeOut);
			}
			if (retVal != RET_OK)  {
				/* start function again */
				(void)icoEventStart(&startSdoEvent,
					startSdoTransfer, NULL);
				return;
			}
			pSdoBuf->state = CO_SDO_QUEUE_STATE_TRANSMITTING;
		}
	}

	CO_DEBUG2("startSdoTransfer end: rd: %d wr:%d\n", rdIdx, wrIdx);
}


/***************************************************************************/
/**
* \internal
*
* \brief icoSdoClientQueueInd
*
*
* \return
*	void
*/
void icoSdoClientQueueInd(
		UNSIGNED8		sdoNr,			/* sdo number */
		UNSIGNED16		index,			/* index */
		UNSIGNED8		subIndex,		/* subindex */
		UNSIGNED32		result			/* result of transfer */
	)
{
EB_SDO_QUEUE_T	*pSdoBuf;
(void)subIndex;

	pSdoBuf = &sdoBuf[rdIdx];

	CO_DEBUG2("coSdoClientWriteInd start: rd: %d wr:%d\n", rdIdx, wrIdx);
	CO_DEBUG4("coSdoClientWriteInd: sdoNr: %d, idx: %x:%d, result: %x\n",
		sdoNr, index, subIndex, result);

	CO_DEBUG4("coSdoClientWriteInd: sdoBuf[%d].sdoNr: %d sdoBuf[%d].index: %x\n",
		rdIdx, sdoBuf[rdIdx].sdoNr, rdIdx, sdoBuf[rdIdx].index);

	if ((sdoNr == pSdoBuf->sdoNr) && (index == pSdoBuf->index))  {
		CO_DEBUG1("coSdoClientWriteInd: state: %d\n", pSdoBuf->state);

		if (pSdoBuf->state == CO_SDO_QUEUE_STATE_TRANSMITTING)  {
			pSdoBuf->state = CO_SDO_QUEUE_STATE_FINISHED;

			/* call indication */
			if (pSdoBuf->pFct != NULL)  {
				pSdoBuf->pFct(pSdoBuf->pFctPara, result);
			}

			rdIdx++;
			if (rdIdx == CO_SDO_QUEUE_LEN)  {
				rdIdx = 0u;
			}
		}
	}
	CO_DEBUG2("coSdoClientWriteInd end: rd: %d wr:%d\n", rdIdx, wrIdx);

	/* start next transfer */
	startSdoTransfer(NULL);
}


/***************************************************************************/
/*
* \brief icoSdoClientVarInit - init sdo client variables
*
*/
void icoSdoQueueVarInit(
		void
	)
{

	{
		rdIdx = 0u;
		wrIdx = 0u;
	}
}


#endif /* CO_SDO_QUEUE */
