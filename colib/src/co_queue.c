/*
* co_queue.c - contains functions for queue handling
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_queue.c 14542 2016-07-15 08:48:49Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief Queue handling
*
* \file co_queue.c
* contains functions for queue handling
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_timer.h>
#include <co_drv.h>
#include <co_commtask.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"

/* constant definitions
---------------------------------------------------------------------------*/
/*#define BUF_NUM(pTrBuf)	((UNSIGNED32)(pTrBuf - &trDataBuffer[0]) )*/

/* local defined data types
---------------------------------------------------------------------------*/
typedef enum {
	CO_TR_STATE_FREE = 0,		/* buffer is free */
	CO_TR_STATE_WAITING,		/* buffer waits for inhibit elapsed */
	CO_TR_STATE_TO_TRANSMIT,	/* buffer should be transmitted */
	CO_TR_STATE_ACTIVE,			/* buffer is transmitting */
	CO_TR_STATE_TRANSMITTED		/* buffer was transmitted */
} CO_TR_STATE_T;

struct CO_TRANS_QUEUE {
	CO_CAN_MSG_T	msg;		/* can message */
	COB_REFERENZ_T	cobRef;		/* cob reference */
	CO_TR_STATE_T	state;		/* status transmit message (written from ISR */
	struct CO_TRANS_QUEUE *pNext;	/* index to next buffer */
};
typedef struct CO_TRANS_QUEUE CO_TRANS_QUEUE_T;



/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static CO_TRANS_QUEUE_T	*searchLastMessage(UNSIGNED16 cobRef, BOOL_T all);
static CO_TRANS_QUEUE_T	*getNextTransBuf(void);
static void inhibitTimer(void *pData);
static void addToInhibitList(CO_TRANS_QUEUE_T *pTrBuf);
static void addToTransmitList(CO_TRANS_QUEUE_T *pTrBuf);
static CO_TRANS_QUEUE_T *moveInhibitToTransmitList(CO_CONST CO_COB_T *pCob);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/
#ifdef CO_EVAL_MODE
UNSIGNED32 demoTimerEnable = 0u; 
#endif /* CO_EVAL_MODE */

/* local defined variables
---------------------------------------------------------------------------*/
static CO_REC_DATA_T	recDataBuffer[CO_CONFIG_REC_BUFFER_CNT];
static UNSIGNED16		recBufferWrCnt = { 0u };
static UNSIGNED16		recBufferRdCnt = { 0u };
static CO_TRANS_QUEUE_T	trDataBuffer[CO_CONFIG_TRANS_BUFFER_CNT];
static BOOL_T			drvBufAccess = { CO_TRUE };
static CO_TRANS_QUEUE_T	*pInhibitList = { NULL };
static CO_TRANS_QUEUE_T	*pTransmitList = { NULL };
static BOOL_T			recBufFull = { CO_FALSE };
#ifdef CO_EVENT_SLEEP
static BOOL_T			queueDisabled = { CO_FALSE };
#endif /* CO_EVENT_SLEEP */
#ifdef CO_GATEWAY_BUFFER
static CO_REC_DATA_T	gwRecDataBuffer[CO_CONFIG_REC_BUFFER_CNT];
static UNSIGNED16		gwRecBufferWrCnt = 0u;
static UNSIGNED16		gwRecBufferRdCnt = 0u;
#endif /* CO_GATEWAY_BUFFER */

#ifdef CO_EVAL_MODE
#warning "Compiling for EVAL mode"
static CO_TIMER_T	demoTimer;
static UNSIGNED32	demoTrCnt = 0;
#define CO_DEMO_TIMER_VALUE	320100	/* in µsec */
static void demoTimerFct(void * pData);
#endif /* CO_EVAL_MODE */



/***************************************************************************/
/**
* \internal
*
* \brief icoGetReceiveMessage - get next receives message
*
*
* \retval CO_FALSE
*	no data available
* \retval CO_FALSE
*	data available
*/
BOOL_T icoQueueGetReceiveMessage(
		CO_REC_DATA_T	*pRecData		/* pointer to receive data */
	)
{
CO_REC_DATA_T	*pData = NULL;
CO_COB_T		*pCob;
BOOL_T			cobFound = CO_FALSE;
BOOL_T			msgFound = CO_FALSE;
#ifdef CO_GATEWAY_BUFFER
UNSIGNED8		getGW = 1;
#endif /* CO_GATEWAY_BUFFER */


	/* check queue for message and get it */
#ifdef CO_GATEWAY_BUFFER
	while ((recBufferWrCnt != recBufferRdCnt)
	    || (gwRecBufferWrCnt != gwRecBufferRdCnt)) {

		/* get message alternate from gw */
		if (getGW == 0)  {
			getGW++;
		} else {
			getGW = 0;
		}
		if (getGW != 0)  {
			/* data from gw buffer */
			if (gwRecBufferWrCnt != gwRecBufferRdCnt)  {
				msgFound = CO_TRUE;
		
				gwRecBufferRdCnt++;
				if (gwRecBufferRdCnt >= CO_CONFIG_REC_BUFFER_CNT)  {
					gwRecBufferRdCnt = 0u;
				}
				pData = &gwRecDataBuffer[gwRecBufferRdCnt];
			}
		} else
#else /* CO_GATEWAY_BUFFER */
	while (recBufferWrCnt != recBufferRdCnt) {
#endif /* CO_GATEWAY_BUFFER */
		{

			/* get data form CAN buffer */
			if (recBufferWrCnt != recBufferRdCnt)  {
				msgFound = CO_TRUE;

				recBufferRdCnt++;
				if (recBufferRdCnt >= CO_CONFIG_REC_BUFFER_CNT)  {
					recBufferRdCnt = 0u;
				}
				pData = &recDataBuffer[recBufferRdCnt];

				/* if buffer is empty, inform application */
				if (recBufferRdCnt == recBufferWrCnt)  {
					coCommStateEvent(CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY);
				}

#ifdef CO_EVENT_SLEEP
				if (queueDisabled == CO_TRUE)  {
					return(CO_FALSE);
				}
#endif /* CO_EVENT_SLEEP */

#ifdef CUSTOMER_RECEIVE_MESSAGES_CALLBACK
		/* sometime the customer want to see all received messages,
		 * e.g. for USB Gateway.
		 *
		 * The customer require a function
		 * 
		 * void fct( const CO_CAN_MSG_T	*pRecData)
		 * Set CUSTOMER_RECEIVE_MESSAGES_CALLBACK to fct.
		 * (include co_drv.h for CO_CAN_MSG_T)
		 */
		CUSTOMER_RECEIVE_MESSAGES_CALLBACK((const void *)&pData->msg);
#endif /* CUSTOMER_RECEIVE_MESSAGES_CALLBACK */

#ifdef CO_GATEWAY_BUFFER
				/* write data to gateway */
				coGatewayTransmitMessage(&pData->msg);
#endif /* CO_GATEWAY_BUFFER */
			}
		}

		/* msg available */
		if ((msgFound == CO_TRUE) && (pData != NULL))  {

			cobFound = CO_FALSE;
			pCob = icoCobCheck(&pData->msg.canCob);
			if (pCob == NULL)  {
				/* check for bootup message */ 
				if ((pData->msg.canCob.canId & 0xFFFFFF80ul) == 0x700ul)  {
					/* ok, bootup found */
					pData->service = CO_SERVICE_NMT;
					pData->spec = 0xffffu;
					cobFound = CO_TRUE;
				}
			} else {
				/* sync message ? */
				pData->service = pCob->service;
				pData->spec = pCob->serviceNr;
				cobFound = CO_TRUE;
			}
		}

		if ((cobFound == CO_TRUE) && (pData != NULL))  {
			memcpy((void *)pRecData, (void *)pData, sizeof(CO_REC_DATA_T));
			return(CO_TRUE);
		}
	}

	return(CO_FALSE);
}


/***************************************************************************/
/**
*
* \brief coQueueReceiveMessageAvailable - receive messages available
*
* This functions checks the receive queue for new messages.
* Are new messages available, return CO_TRUE.
* Otherwise CO_FALSE
*
* \retval CO_FALSE
*	no data available
* \retval CO_FALSE
*	data available
*/
BOOL_T coQueueReceiveMessageAvailable(
		void	/* no parameter */
	)
{
	/* check queue for message */
	if ((recBufferWrCnt != recBufferRdCnt)
#ifdef CO_GATEWAY_BUFFER
	 || (gwRecBufferWrCnt != gwRecBufferRdCnt)
#endif /* CO_GATEWAY_BUFFER */
		) {
		return(CO_TRUE);
	} else {
		return(CO_FALSE);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief icoQueueGetReceiveBuffer - get pointer to receive buffer
*
*
* can be called at interrupt level
*
* \return pointer to receice data
*
*/
CO_CAN_MSG_T *coQueueGetReceiveBuffer(
		void	/* no parameter */
	)
{
UNSIGNED16	tmpCnt;

	/* get next receive buffer */
	tmpCnt = recBufferWrCnt + 1u;
	if (tmpCnt >= CO_CONFIG_REC_BUFFER_CNT)  {
		tmpCnt = 0u;
	}
	if (tmpCnt == recBufferRdCnt)  {
		/* save event, and signal it later outsite of the interrupt */
		recBufFull = CO_TRUE;

		return(NULL);
	}

	return(&recDataBuffer[tmpCnt].msg);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief coQueueReceiveBufferIsFilled - given queue buffer is filled
*
* This function is called from the driver,
* if the given receive buffer was filled by a received CAN message.<br>
*
* It can be called at interrupt level.
*
* \return none
*/
void coQueueReceiveBufferIsFilled(
		void	/* no parameter */
	)
{
UNSIGNED16	tmpCnt;

#ifdef CO_EVENT_SLEEP
	if (queueDisabled == CO_TRUE)  {
		return;
	}
#endif /* CO_EVENT_SLEEP */

	tmpCnt = recBufferWrCnt + 1u;
	if (tmpCnt >= CO_CONFIG_REC_BUFFER_CNT)  {
		tmpCnt = 0u;
	}
	if (tmpCnt == recBufferRdCnt)  {
		coCommStateEvent(CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW);
		return;
	}

	recBufferWrCnt = tmpCnt;

	/* last message buffer used ? */
/* 	coCommStateEvent(CO_COMM_STATE_EVENT_REC_QUEUE_FULL); */
}


/***************************************************************************/
/**
* \internal
*
* \brief icoTransmitMessage - save message in transmit queue
*
* flags:
* MSG_OVERWRITE - if the last message is not transmitted yet,
*	overwrite the last data with the new data
* MSG_RET_INHIBIT - add message to transmit queue
*	and send it after the inhibit time has been ellapsed
* Otherwise the function returns with RET_COB_INHIBIT
*
* \return RET_T
*
*/
RET_T icoTransmitMessage(
		COB_REFERENZ_T	cobRef,				/* cob reference */
		CO_CONST CO_TRANS_DATA_T *pData,	/* pointer to transmit data */
		UNSIGNED8		flags				/* data handle flags */
	)
{
CO_COB_T	*pCob;
CO_TRANS_QUEUE_T *pTrBuf = NULL;
CO_TRANS_QUEUE_T *pLastBuf = NULL;
UNSIGNED16	i;
BOOL_T		inhibitActive;
BOOL_T		msgDoubled = CO_FALSE;
BOOL_T		msgOverwrite = CO_FALSE;

#ifdef CO_EVENT_SLEEP
	if (queueDisabled == CO_TRUE)  {
		return(RET_OK);
	}
#endif /* CO_EVENT_SLEEP */

	pCob = icoCobGetPointer(cobRef);
	if (pCob == NULL)  {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* if cob is disabled, return */
	if (pCob->canCob.enabled != CO_TRUE)  {
		return(RET_COB_DISABLED);
	}

	/* if inhibit active ? */
	inhibitActive = coTimerIsActive(&pCob->inhibitTimer);
	/* inhibit is active, return RET_INHIBIT_ACTIVE */
	if (((flags & MSG_RET_INHIBIT) != 0u)
	 &&	(inhibitActive == CO_TRUE))  {
		return(RET_INHIBIT_ACTIVE);
	}

	/* disable driver buffer access */
	drvBufAccess = CO_FALSE;

	/* printf("*** icoTransmitMessage: id: %lx, %x %x %x %x\n", pCob->canCob.canId,
		pData->data[0], pData->data[1],
		pData->data[2], pData->data[3]
		);
	*/

	/* should data at queue always overwritten ? */
	if ((flags & MSG_OVERWRITE) != 0u)  {
		/* search last message */
		pTrBuf = searchLastMessage(cobRef, CO_FALSE);
	} else {
		/* overwrite data only, if there has been changed
		 * Only valid for for PDO, EMCY and HB */
		if ((pCob->service == CO_SERVICE_ERRCTRL)
		 || (pCob->service == CO_SERVICE_GUARDING)
		 || (pCob->service == CO_SERVICE_EMCY_TRANSMIT)
		 || (pCob->service == CO_SERVICE_PDO_TRANSMIT))  {
			/* search last message */
			pTrBuf = searchLastMessage(cobRef, CO_FALSE);
			if (pTrBuf != NULL)  {
				/* message is in buffer, check for changed data */
				for (i = 0u; i < pTrBuf->msg.len; i++)  {
					if (pTrBuf->msg.data[i] != pData->data[i])  {
						/* data are changed... */
						pTrBuf = NULL;
						break;
					}
				}
				if (pTrBuf != NULL) {
					msgDoubled = CO_TRUE;
				}
			}
		}
	}

	/* no old buffer available */
	if (pTrBuf != NULL)  {
		msgOverwrite = CO_TRUE;
	} else {
		pTrBuf = getNextTransBuf();
		/* printf("No Overwrite, "); */
		if (pTrBuf == NULL)  {
			/* allow driver buffer access */
			drvBufAccess = CO_TRUE;

			/* start can transmission again */
			(void) codrvCanStartTransmission();

			/* inform application */
			coCommStateEvent(CO_COMM_STATE_EVENT_TR_QUEUE_OVERFLOW);
			return(RET_DRV_TRANS_BUFFER_FULL);
		}
	}
	/* printf("use buffer %ld\n", BUF_NUM(pTrBuf));*/

	/* if not the same message */
	if (msgDoubled == CO_FALSE) {

		/* save data at transmit buffer */
		pTrBuf->cobRef = cobRef;
		pTrBuf->msg.len = pCob->len;
		pTrBuf->msg.canCob.canId = pCob->canCob.canId;
		pTrBuf->msg.canCob.extended = pCob->canCob.extended;
		if (pCob->type == CO_COB_TYPE_TRANSMIT)  {
			pTrBuf->msg.canCob.rtr = CO_FALSE;
		} else {
			pTrBuf->msg.canCob.rtr = CO_TRUE;
		}
		pTrBuf->msg.canCob.canChan = pCob->canCob.canChan;

		if (pData != NULL)  {
			for (i = 0u; i < pCob->len; i++)  {
				pTrBuf->msg.data[i] = pData->data[i];
			}
		}

		/* set state only for new messages */
		if (msgOverwrite == CO_FALSE)  {
			/* inhibit active */ 
			if (inhibitActive == CO_TRUE)  {
				/* the same message is already at buffer */
				/* printf("inhibit active\n"); */

				/* add to inhibit list */
				addToInhibitList(pTrBuf);

			} else {
				/* searchLastMessage incl tranmitted messages */
				pLastBuf = searchLastMessage(cobRef, CO_TRUE);
				/* if the same message already on the buffer
				 * and inhibit time != 0*/
				if ((pLastBuf != NULL) && (pCob->inhibit != 0u)) {
					/* yes */

					/* add to inhibit list */
					addToInhibitList(pTrBuf);
				} else {

					/* add to transmit list */
					addToTransmitList(pTrBuf);

#ifdef CO_GATEWAY_BUFFER
					/* send it by gateway */
					coGatewayTransmitMessage(&pTrBuf->msg);
#endif /* CO_GATEWAY_BUFFER */
				}
			}
			/* 
			for (i = 0; i < 10; i++)  {
				printf("buf %d: %d\n", i, trDataBuffer[i].state);
			}
			*/
		}
	}

	/* allow driver buffer access */
	drvBufAccess = CO_TRUE;

	/* start can transmitting */
	(void) codrvCanStartTransmission();

#ifdef CO_EVAL_MODE
	if (demoTimerEnable != 0u) {
		/* demo timer started ? */
		if (coTimerIsActive(&demoTimer) == CO_TRUE)  {
			/* check, if timer functionality isn't called */
			demoTrCnt++;
			/* more than 10 per msec ? */
			if (demoTrCnt > (CO_DEMO_TIMER_VALUE / 100))  {
				/* then the timer will not work... */
				demoTimerFct((void *)0x12735);
			}
		} else {
			/* Start timer */
			coTimerStart(&demoTimer, CO_DEMO_TIMER_VALUE, demoTimerFct,
				NULL, CO_TIMER_ATTR_ROUNDUP_CYCLIC);
		}
	}
#endif /* CO_EVAL_MODE */

	return(RET_OK);
}


BOOL_T icoQueueInhibitActive(
		COB_REFERENZ_T	cobRef				/* cob reference */
	)
{
CO_COB_T	*pCob;
CO_TRANS_QUEUE_T *pLastBuf;
BOOL_T	inhibitActive;

	pCob = icoCobGetPointer(cobRef);
	if (pCob == NULL)  {
		return(CO_FALSE);
	}
	inhibitActive = coTimerIsActive(&pCob->inhibitTimer);
	pLastBuf = searchLastMessage(cobRef, CO_TRUE);

	if ((inhibitActive == CO_TRUE)
	 || ((pLastBuf != NULL) && (pCob->inhibit != 0u))) {
		return(CO_TRUE);
	} else {
		return(CO_FALSE);
	}
}


#ifdef CO_EVAL_MODE
/* fct is called every CO_DEMO_TIMER_VALUE msec
* wait about 1 h and then stop the program
*/
#if !defined(CO_EVAL_TIMEOUT)
/* ca 1 h       1.037 * 60 * 60 * 1000000ul */
# define CO_EVAL_TIMEOUT	(1.037 * 60 * 60 *1000000ul)
#endif

static void demoTimerFct(
		void * pData
	)
{
static UNSIGNED32 demoFctCnt = 0;

	if (pData == NULL)  {
		demoTrCnt = 0;
	}

	demoFctCnt ++;

	if (demoFctCnt > (CO_EVAL_TIMEOUT / CO_DEMO_TIMER_VALUE))  {
/*		printf("**** Zeit ist um\n"); */
		pInhibitList = (void *)0x17835;
		pTransmitList = (void *)0x84653;
	}
}
#endif /* CO_EVAL_MODE */


/***************************************************************************/
/**
* \internal
*
* \brief addToInhibitList - add buffer to inhibit list
*
* Add an buffer entry to the inhibit list as the first element
*
* \return none
*
*/
static void addToInhibitList(
		CO_TRANS_QUEUE_T	*pTrBuf
	)
{
CO_TRANS_QUEUE_T	*pList;

	/*
	printf("addToInhibitList: buf %ld - ", BUF_NUM(pTrBuf));
	pList = pInhibitList;
	while (pList != NULL)  {
		printf("%ld ", BUF_NUM(pList));
		pList = pList->pNext;
	}
	printf("\n");
	*/

	/* add it as first entry */
	/* as first save actual pointer */
	pList = pInhibitList;
	pInhibitList = pTrBuf;
	pTrBuf->pNext = pList;
	/* save buffer state */
	pTrBuf->state = CO_TR_STATE_WAITING;
	/* printf(" WAITING\n"); */
}


/***************************************************************************/
/**
* \internal
*
* \brief addToTransmitList - add an buffer to transmit list
*
* Add an buffer entry to the transmit list at the end of the list
*
* \return none
*
*/
static void addToTransmitList(
		CO_TRANS_QUEUE_T	*pTrBuf
	)
{
CO_TRANS_QUEUE_T	*pList;

	/* add it at last position */
	pList = pTransmitList;

	/* list empty ? */
	if (pList == NULL)  {
		/* save at first */
		pTransmitList = pTrBuf;
	} else {
		/* look for end of the list */
		while (pList->pNext != NULL)  {
			pList = pList->pNext;
		}
		pList->pNext = pTrBuf;
	}

	/* set next element to 0 */
	pTrBuf->pNext = NULL;
	/* set buffer state */
	pTrBuf->state = CO_TR_STATE_TO_TRANSMIT;
	/* printf(" TO TRANSMIT\n"); */
}


/***************************************************************************/
/**
* \internal
*
* \brief moveInhibitToTransmitList - move last entry to transmit list
*
* \return pointer to moved entry
*
*/
static CO_TRANS_QUEUE_T *moveInhibitToTransmitList(
		CO_CONST CO_COB_T	*pCob
	)
{
CO_TRANS_QUEUE_T	*pBuf;
CO_TRANS_QUEUE_T	*pTrData = NULL;
CO_COB_T	*cobPtr;

	pBuf = pInhibitList;

	/* look for last message at inhibit queue with this cobRef */
	while (pBuf != NULL)  {
		cobPtr = icoCobGetPointer(pBuf->cobRef);
		if (cobPtr == pCob)  {
			pTrData = pBuf;
		}

		/* search whole list - oldest message is at the end */
		pBuf = pBuf->pNext;
	}

	/* buffer found ? */
	if (pTrData != NULL)  {
		/* transmit data found - copy it to transmit list */

		/* disable driver buffer access */
		drvBufAccess = CO_FALSE;

		/* delete it from inhibit list */
		if (pInhibitList == pTrData)  {
			pInhibitList = pTrData->pNext;
		} else {
			pBuf = pInhibitList;
			while (pBuf->pNext != pTrData)  {
				pBuf = pBuf->pNext;
			}
			pBuf->pNext = pTrData->pNext;
		}

		/* and save it at transmit list */
		addToTransmitList(pTrData);
		/* printf("inhibitTimer: buf %ld TO TRANSMIT\n", BUF_NUM(pTrData)); */

#ifdef CO_GATEWAY_BUFFER
		/* send it by gateway */
		coGatewayTransmitMessage(&pTrData->msg);
#endif /* CO_GATEWAY_BUFFER */

		/* allow driver buffer access */
		drvBufAccess = CO_TRUE;

		/* start can transmission again */
		(void) codrvCanStartTransmission();
	}

	return(pTrData);
}


/***************************************************************************/
/**
* \internal
*
* \brief searchLastMessage - look for last message from same type
*
* look for message with same typ and status waiting
* Parameter all gibt an, ob auch aktuell schon versendete Nachrichten
* mit einbezogen werden sollen
*
* \return buffer index
*
*/
static CO_TRANS_QUEUE_T	*searchLastMessage(
		UNSIGNED16		cobRef,		/* cob reference */
		BOOL_T			all			/* use all messages incl. transmitted messages*/
	)
{
CO_TRANS_QUEUE_T	*pList;
CO_TRANS_QUEUE_T	*pLast = NULL;

	/* as first, look on inhibit list */
	pList = pInhibitList;
	while (pList != NULL)  {
		/* correct cob reference ? */
		if (pList->cobRef == cobRef)  {
			/* yes, save pointer */
			pLast = pList;
		}
		/* try until end of list */
		pList = pList->pNext;
	}

	/* entry found - then it is the oldest in our list */
	if (pLast != NULL)  {
		return(pLast);
	}

	/* no entry at inhibit list found - try the same at transmit list */
	pList = pTransmitList;
	while (pList != NULL)  {
		/* cobref found ? */
		if (pList->cobRef == cobRef)  {
			/* depending on parameter all */
			if (all == CO_TRUE)  {
				/* check state of the message */
				if (pList->state != CO_TR_STATE_FREE)  {
					pLast = pList;
				}
			} else {
				if (pList->state == CO_TR_STATE_TO_TRANSMIT)  {
					pLast = pList;
				}
			}

			/* if we have found an entry, return it */
		}
		pList = pList->pNext;
	}

	return(pLast);
}


/***************************************************************************/
/**
* \internal
*
* \brief getNextTransBuf - get next transmit buffer
*
* get next transmit buffer depending on the last transmission time
*
* \return buffer index
*/
static CO_TRANS_QUEUE_T	*getNextTransBuf(
		void	/* no parameter */
	)
{
UNSIGNED16	i;

	/* look for next free buffer */
	for (i = 0u; i < CO_CONFIG_TRANS_BUFFER_CNT; i++)  {
		if (trDataBuffer[i].state == CO_TR_STATE_FREE)  {
			/* check for buffer full -
			 * it comes over only if we use the last buffer */
			if (i == (CO_CONFIG_TRANS_BUFFER_CNT - 1u))  {
				coCommStateEvent(CO_COMM_STATE_EVENT_TR_QUEUE_FULL);
			}
			return(&trDataBuffer[i]);
		}
	}

	return(NULL);
}


/***************************************************************************/
/**
* \brief coQueueGetNextTransmitMessage - get next message to transmit
*
* This function returns the next available transmit message
* from the transmit queue.
* It increments also trBufferRdCnt.
*
* \return CO_CAN_MSG_T* pointer to next tx message
* \retval !NULL
*	pointer to transmit queue entry
* \retval NULL
*	no message available
*
*/
CO_CAN_MSG_T *coQueueGetNextTransmitMessage(
		void	/* no parameter */
	)
{
CO_TRANS_QUEUE_T *pBuf;

	/* access to buffer allowed ? */
	if (drvBufAccess != CO_TRUE)  {
		return(NULL);
	}

	/* start with first entry */
	pBuf = pTransmitList;
	while (pBuf != NULL)  {
		/* correct state ? */
		if (pBuf->state == CO_TR_STATE_TO_TRANSMIT)  {
			/* set state to active */
			pBuf->state = CO_TR_STATE_ACTIVE;
			/* save handle */
			pBuf->msg.handle = pBuf;
			/* printf("coQueueGetNextTransmitMessage: buffer %ld\n", BUF_NUM(pBuf)); */
			return(&pBuf->msg);
		}

		pBuf = pBuf->pNext;
	}

	return(NULL);
}


/***************************************************************************/
/**
* \brief coQueueMsgTransmitted - message was transmitted
*
* This function is called after a message was succesfull transmitted.
*
* \return none
*
*/
void coQueueMsgTransmitted(
		const CO_CAN_MSG_T *pBuf		/**< pointer to transmitted message */
	)
{
CO_TRANS_QUEUE_T	*pQueue;

	pQueue = (CO_TRANS_QUEUE_T	*)pBuf->handle;
	pQueue->state = CO_TR_STATE_TRANSMITTED;
}


/***************************************************************************/
/**
* \internal
*
* \brief icoQueueHandler
*
* \return none
*
*/
void icoQueueHandler(
		void	/* no parameter */
	)
{
CO_COB_T	*pCob;

/*
* trBufferRdCnt wird im Interrupt verändert, daher ist hier evtl. zusätzlicher Schutz notwendig?
*/

	/* recBuffer was full ? */
	if (recBufFull == CO_TRUE)  {
		recBufFull = CO_FALSE;
		/* inform application, if buffer is full */
		coCommStateEvent(CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW);
	}

	/* no entries at transmit list, return */
	if (pTransmitList == NULL)  {
		return;
	}

	/* check all buffer with state transmitted */
	while (pTransmitList->state == CO_TR_STATE_TRANSMITTED)  {
		/* delete transmitted messages ...... and save inhibit */
		/* printf("icoQueueHandler: buffer %ld to free\n", BUF_NUM(pTransmitList));*/

		/* get cob reference */
		pCob = icoCobGetPointer(pTransmitList->cobRef);
		if (pCob != NULL)  {
			/* if inhibit is != 0 */
			if (pCob->inhibit != 0u)  {
				/* start inhibit timer */
				(void)coTimerStart(&pCob->inhibitTimer,
						pCob->inhibit * 100ul,
						inhibitTimer, pCob, CO_TIMER_ATTR_ROUNDUP);	/*lint !e960 function identifier used without '&' or parenthesized parameter list */
				/* printf("icoQueueHandler: start inhibit buffer %ld\n", BUF_NUM(pTransmitList)); */
			}
		}
        /* set buffer state */
		pTransmitList->state = CO_TR_STATE_FREE;

		/* set transmit list to next buffer */
		pTransmitList = pTransmitList->pNext;
		if (pTransmitList == NULL)  {
			break;
		}
	}

	/* transmit list is empty */
	if (pTransmitList == NULL)  {
		/* inhibit list is also empty, signal it to eventHandler */
		if (pInhibitList == NULL)  {
			coCommStateEvent(CO_COMM_STATE_EVENT_TR_QUEUE_EMPTY);
		}
	} else {
		/* should buffer be transmitted? */
		if (pTransmitList->state == CO_TR_STATE_TO_TRANSMIT)  {
			(void) codrvCanStartTransmission();
		}
	}

}


/***************************************************************************/
/**
* \internal
*
* \brief icoQueueDeleteInhibit - delete inhibit entries of this cobRef
*
* \return none
*
*/
void icoQueueDeleteInhibit(
		COB_REFERENZ_T	cobRef			/* cob reference */
	)
{
CO_COB_T	*pCob;
CO_TRANS_QUEUE_T	*pData = NULL;

	pCob = icoCobGetPointer(cobRef);
	if (pCob == NULL)  {
		return;
	}

	/* delete inhibit until list is empty */
	do {
		pData = moveInhibitToTransmitList(pCob);
	} while (pData != NULL);
}


/***************************************************************************/
/**
* \internal
*
* \brief inhibitTimer - inhibit timer has been ellapsed
*
* \return none
*
*/
static void inhibitTimer(
		void			*pData
	)
{
CO_COB_T	*pCob;

	pCob = (CO_COB_T *)pData;

	(void)moveInhibitToTransmitList(pCob);
}


#ifdef CO_EVENT_SLEEP
/***************************************************************************/
/**
* \internal
*
* \brief icoQueueDisable - enable/disable message queues
*
* \return none
*
*/
void icoQueueDisable(
		BOOL_T	on
	)
{
	queueDisabled = on;

	/* delete all entries at the inhibit queue */
	pInhibitList = NULL;
}
#endif /* CO_EVENT_SLEEP */



/***************************************************************************/
/**
*
* \brief coQueueInit - (re)init queues
*
* This function clears the transmit and the receive queue
*
* \return none
*
*/
void coQueueInit(
		void	/* no parameter */
	)
{
UNSIGNED16	i;

	/* receive queue */
	recBufferWrCnt = 0u;
	recBufferRdCnt = 0u;

	/* transmit queue */
	for (i = 0u; i < CO_CONFIG_TRANS_BUFFER_CNT; i++)  {
		trDataBuffer[i].state = CO_TR_STATE_FREE;
	}

	drvBufAccess = CO_TRUE;
	pInhibitList = NULL;
	pTransmitList = NULL;

	recBufferWrCnt = 0u;
	recBufferRdCnt = 0u;
	pInhibitList = NULL;
	pTransmitList = NULL;
	recBufFull = CO_FALSE;

#ifdef CO_EVENT_SLEEP
	queueDisabled = CO_FALSE;
#endif /* CO_EVENT_SLEEP */
}


#ifdef CO_GATEWAY_BUFFER
/***************************************************************************/
/**
*
* \brief coQueueRecMsgFromGw - receive message from gateway
*
* This function is called, if a new message is available on the gw
* The message is saved at the receive buffer and the transmit buffer.
* O it can be used from the CANopen stack and send out to the CAN.
*
* \return none
*
*/
void coQueueRecMsgFromGw(
		CO_CAN_MSG_T	*pMsg		/* can message */
	)
{
UNSIGNED16	tmpCnt;
CO_TRANS_QUEUE_T	*pTrBuf;

	/* try to save at receive buffer */
	tmpCnt = gwRecBufferWrCnt + 1u;
	if (tmpCnt >= CO_CONFIG_REC_BUFFER_CNT)  {
		tmpCnt = 0u;
	}
	if (tmpCnt != gwRecBufferRdCnt)  {
		/* fill out standard values */
		pMsg->canCob.enabled = CO_TRUE;

		/* emty entry found */
		memcpy(&gwRecDataBuffer[tmpCnt].msg, pMsg, sizeof(CO_CAN_MSG_T));

		gwRecBufferWrCnt = tmpCnt;

		coCommTaskSet(CO_COMMTASK_EVENT_MSG_AVAIL);
	} else {
		coCommStateEvent(CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW);
	}


	/* transmit buffer */

	drvBufAccess = CO_FALSE;
	pTrBuf = getNextTransBuf();
	if (pTrBuf == NULL)  {
		/* allow driver buffer access */
		drvBufAccess = CO_TRUE;

		/* start can transmission again */
		(void) codrvCanStartTransmission();

		/* inform application */
		coCommStateEvent(CO_COMM_STATE_EVENT_TR_QUEUE_OVERFLOW);
	
		return;
	}

	memcpy(&pTrBuf->msg, pMsg, sizeof(CO_CAN_MSG_T));
	pTrBuf->cobRef = 0xffff;

	addToTransmitList(pTrBuf);

	/* allow driver buffer access */
	drvBufAccess = CO_TRUE;

	/* start can transmission again */
	(void) codrvCanStartTransmission();
}
#endif /* CO_GATEWAY_BUFFER */


/***************************************************************************/
/**
* \internal
*
* \brief icoQueuVarInit - init lokal variables
*
* \return none
*
*/
void icoQueueVarInit(
		void
	)
{

	{
		recBufferWrCnt = 0u;
		recBufferRdCnt = 0u;
		drvBufAccess = CO_TRUE;
		pInhibitList = NULL;
		pTransmitList = NULL;
		recBufFull = CO_FALSE;

#ifdef CO_EVENT_SLEEP
		queueDisabled = CO_FALSE;
#endif /* CO_EVENT_SLEEP */
	}

#ifdef CO_GATEWAY_BUFFER
	gwRecBufferWrCnt = 0u;
	gwRecBufferRdCnt = 0u;
#endif /* CO_GATEWAY_BUFFER */

#ifdef CO_EVAL_MODE
	demoTrCnt = 0;
# ifdef CO_EVAL_MODE_ENABLED
	demoTimerEnable = 1u; 
# else /* CO_EVAL_MODE_ENABLED */
	demoTimerEnable = 0u; 
# endif /* CO_EVAL_MODE_ENABLED */
#endif /* CO_EVAL_MODE */
}


#ifdef xxx
void printBufState(
	char 	*info
	)
{
UNSIGNED8	i;

	printf("%s: wr: %d, rd %d\n", info, trBufferWrCnt, trBufferRdCnt);

	for (i = 0; i < CO_CONFIG_TRANS_BUFFER_CNT; i++)  {
//CO_TRANS_QUEUE_T	trDataBuffer[CO_CONFIG_TRANS_BUFFER_CNT];
		printf("%d: state %d, id %x\n", i,
			trDataBuffer[i].state,
			trDataBuffer[i].cobRef);
	}
	printf("-----------------------------\n");
}


int coPrintTime()
{
#include <sys/time.h>
struct timeval tv;
struct timezone tz;

	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;

	gettimeofday(&tv, &tz);
	printf("time %d:%06d", tv.tv_sec, tv.tv_usec);
}
#endif
