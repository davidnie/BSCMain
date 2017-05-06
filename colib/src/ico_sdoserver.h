/*
* ico_sdoserver.h - contains internal defines for SDO
*
* Copyright (c) 2013-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_sdoserver.h 12068 2016-01-22 09:57:19Z boe $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_SDO_SERVER_H
#define ICO_SDO_SERVER_H 1


/* constants */
#define MAX_SAVE_DATA	4u


/* datatypes */

typedef enum {
	CO_SDO_STATE_FREE,
/*	CO_SDO_STATE_UPLOAD_INIT, */
	CO_SDO_STATE_UPLOAD_SEGMENT,
/*	CO_SDO_STATE_DOWNLOAD_INIT, */
	CO_SDO_STATE_DOWNLOAD_SEGMENT,
	CO_SDO_STATE_BLOCK_UPLOAD_INIT,
	CO_SDO_STATE_BLOCK_UPLOAD,
	CO_SDO_STATE_BLOCK_UPLOAD_RESP,
	CO_SDO_STATE_BLOCK_UPLOAD_LAST,
	CO_SDO_STATE_BLOCK_UPLOAD_END,
	CO_SDO_STATE_BLOCK_DOWNLOAD,
	CO_SDO_STATE_BLOCK_DOWNLOAD_END,
	CO_SDO_STATE_FD_UPLOAD_SEGMENT,
	CO_SDO_STATE_FD_DOWNLOAD_SEGMENT,
	CO_SDO_STATE_NETWORK_IND,
	CO_SDO_STATE_RD_SPLIT_INDICATION,
	CO_SDO_STATE_WR_SPLIT_INDICATION,
	CO_SDO_STATE_WR_SEG_SPLIT_INDICATION
} CO_SDO_STATE_T;

typedef struct co_sdo_server_t {
	CO_CONST CO_OBJECT_DESC_T *pObjDesc;		/* object description pointer */
	UNSIGNED32		trCobId;		/* cob id */
	UNSIGNED32		recCobId;		/* cob id */
	UNSIGNED32		objSize;		/* object size */
	UNSIGNED32		transferedSize;	/* transfered size */
	UNSIGNED16		index;			/* index */
	UNSIGNED8		saveData[MAX_SAVE_DATA];
	UNSIGNED8		sdoNr;			/* sdo number */
	UNSIGNED8		subIndex;		/* sub index */
	UNSIGNED8		node;			/* subindex 3 */
	UNSIGNED8		toggle;
	COB_REFERENZ_T	trCob;
	COB_REFERENZ_T	recCob;
	CO_SDO_STATE_T	state;			/* sdo state */
	BOOL_T			changed;		/* object was changed */
# ifdef CO_SDO_BLOCK
	UNSIGNED8		seqNr;			/* sequence number */
	UNSIGNED8		blockSize;		/* max number of blocks for one transfer */
	BOOL_T			blockCrcUsed;	/* use CRC */
	UNSIGNED16		blockCrc;		/* CRC */
	UNSIGNED32		blockCrcSize;	/* size of calculated crc sum */
	CO_EVENT_T		blockEvent;		/* event structure */
	UNSIGNED8		blockSaveData[7]; /* save data for last block */
# endif /* CO_SDO_BLOCK */
#ifdef CO_EVENT_SSDO_DOMAIN_WRITE
	BOOL_T			domainTransfer;	/* object with domain transfer */
	UNSIGNED32		domainTransferedSize;	/* overall transfered size */
#endif /* CO_EVENT_SSDO_DOMAIN_WRITE */
# ifdef CO_SDO_NETWORKING
	UNSIGNED8		networkCanLine;
	UNSIGNED8		clientSdoNr;
	UNSIGNED8		localRequest;
	struct co_sdo_server_t *pLocalReqSdo;
# endif /* CO_SDO_NETWORKING */
} CO_SDO_SERVER_T;


/* function prototypes */

void	icoSdoServerVarInit(CO_CONST UNSIGNED8 *pList);
void	icoSdoDeCodeMultiplexer(CO_CONST UNSIGNED8 pMp[],
			CO_SDO_SERVER_T *pSdo);
RET_T	icoSdoCheckUserWriteInd(const CO_SDO_SERVER_T *pSdo);
RET_T	icoSdoCheckUserReadInd(UNSIGNED8 sdoNr, UNSIGNED16 index,
			UNSIGNED8 subIndex);
RET_T	icoSdoCheckUserCheckWriteInd(UNSIGNED8 sdoNr,
			UNSIGNED16 index, UNSIGNED8 subIndex, const UNSIGNED8 *pData);
void	icoSdoDomainUserWriteInd(const CO_SDO_SERVER_T *pSdo);
void	icoSdoServerAbort(CO_SDO_SERVER_T *pSdo, RET_T	errorReason,
			BOOL_T fromClient);

# ifdef CO_SDO_BLOCK
RET_T	icoSdoServerBlockReadInit(CO_SDO_SERVER_T	*pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockRead(CO_SDO_SERVER_T	*pSdo);
RET_T	icoSdoServerBlockReadCon(CO_SDO_SERVER_T	*pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockWriteInit(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockWrite(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T *pRecData);
RET_T	icoSdoServerBlockWriteEnd(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T	*pRecData);
# endif /* CO_SDO_BLOCK */

#ifdef CO_SDO_NETWORKING
RET_T	icoSdoServerNetworkReq(CO_SDO_SERVER_T *pSdo, const CO_CAN_MSG_T *pRecData);
void	icoSdoServerNetwork(CO_SDO_SERVER_T *pSdo, const CO_CAN_MSG_T *pRecData);
RET_T	icoSdoServerLocalResp(	CO_SDO_SERVER_T	*pSdo,
			const CO_TRANS_DATA_T	*pTrData);
#endif /* CO_SDO_NETWORKING */

CO_SDO_SERVER_T	*icoSdoServerPtr(UNSIGNED16 sdoNr);

#endif /* ICO_SDO_SERVER_H */
