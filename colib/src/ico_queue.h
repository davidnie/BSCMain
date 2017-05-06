/*
* ico_queue.h - contains defines for internal queue handling
*
* Copyright (c) 2012-2014 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_queue.h 9669 2015-06-29 13:49:46Z boe $
*
*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_QUEUE_H
#define ICO_QUEUE_H 1


/* datatypes */

typedef struct {
	CO_CAN_MSG_T	msg;		/* can message */
	CO_SERVICE_T 	service;	/* canopen service */
	UNSIGNED16		spec;		/* service specification */
} CO_REC_DATA_T;

typedef struct {
	UNSIGNED8	data[CO_CAN_MAX_DATA_LEN];	/* data */
} CO_TRANS_DATA_T;


/* function prototypes */

BOOL_T	icoQueueGetReceiveMessage(CO_REC_DATA_T *pRecData);
RET_T	icoTransmitMessage(COB_REFERENZ_T cobRef,
			CO_CONST CO_TRANS_DATA_T *pData, UNSIGNED8 flags);
void	icoQueueHandler(void);
void	icoQueueDisable(BOOL_T on);
void	icoQueueDeleteInhibit(COB_REFERENZ_T cobRef);
void	icoQueueVarInit(void);
BOOL_T icoQueueInhibitActive(COB_REFERENZ_T cobRef);

#endif /* ICO_QUEUE_H */

