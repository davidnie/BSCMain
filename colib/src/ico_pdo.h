/*
* ico_pdo.h - contains internal defines for PDO
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_pdo.h 10885 2015-10-02 10:20:17Z boe $

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

#ifndef ICO_PDO_H
#define ICO_PDO_H 1

#include <co_pdo.h>


#define CO_MPDO_DAM_MODE	255u
#define CO_MPDO_SAM_MODE	254u

#define CO_PDO_MAX_DATA_LEN	CO_CAN_MAX_DATA_LEN


# if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)

/* datatypes */

typedef enum {
	CO_PDO_STATE_READY,
	CO_PDO_STATE_TO_TRANSMIT
#ifdef CO_PDO_RECEIVE_CNT
	,CO_PDO_STATE_DATA_RECEIVED
#endif /* CO_PDO_RECEIVE_CNT */
} CO_PDO_STATE_T;


#if defined(CO_MPDO_PRODUCER) || defined(CO_MPDO_CONSUMER)
typedef enum {
	CO_PDO_TYPE_STD,		/* standard PDO */
	CO_PDO_TYPE_SAM,		/* Source adress mode PDO */
	CO_PDO_TYPE_DAM			/* Destination adress mode PDO */
} CO_PDO_TYPE_T;
#endif /* defined(CO_MPDO_PRODUCER) | defined(CO_MPDO_CONSUMER) */


typedef struct {
	UNSIGNED32		cobId;
	CO_CONST PDO_TR_MAP_TABLE_T	*mapTableConst;
	PDO_TR_MAP_TABLE_T	*mapTable;
	CO_TIMER_T		pdoTimer;
	CO_PDO_STATE_T	state;			/* pdo state */
	COB_REFERENZ_T	cob;
	BOOL_T			dynMapping;		/* dynamic mapped PDO */
#ifdef CO_MPDO_PRODUCER
	CO_PDO_TYPE_T	pdoType;
#endif /* CO_MPDO_PRODUCER */
	UNSIGNED16		pdoNr;			/* pdo number */
	UNSIGNED16		inhibit;
	UNSIGNED16		eventTime;
#ifdef CO_TR_PDO_DYN_MAP_ENTRIES
	UNSIGNED16		mapTableIdx;	/* index at mapping table */
#endif /* CO_TR_PDO_DYN_MAP_ENTRIES */
	UNSIGNED8		transType;
	UNSIGNED8		syncStartVal;
	UNSIGNED8		actSyncCnt;
} CO_TR_PDO_T;


#ifdef CO_PDO_RECEIVE_CNT
typedef struct {
	UNSIGNED32		cobId;
	CO_CONST PDO_REC_MAP_TABLE_T	*mapTableConst;
	PDO_REC_MAP_TABLE_T	*mapTable;
	CO_TIMER_T		pdoTimer;
	CO_PDO_STATE_T	state;			/* pdo state */
	COB_REFERENZ_T	cob;
#ifdef CO_MPDO_CONSUMER
	CO_PDO_TYPE_T	pdoType;
#endif /* CO_MPDO_CONSUMER */
	BOOL_T			dynMapping;		/* dynamic mapped PDO */
	UNSIGNED16		pdoNr;			/* pdo number */
	UNSIGNED16		inhibit;
	UNSIGNED16		eventTime;
#ifdef CO_REC_PDO_DYN_MAP_ENTRIES
	UNSIGNED16		mapTableIdx;	/* index at mapping table */
#endif /* CO_REC_PDO_DYN_MAP_ENTRIES */
	UNSIGNED8		shadowData[CO_PDO_MAX_DATA_LEN];
	UNSIGNED8		transType;
	UNSIGNED8		mapLen;
} CO_REC_PDO_T;
#endif /* CO_PDO_RECEIVE_CNT */



/* function prototypes */

CO_INLINE CO_TR_PDO_T *icoPdoSearchTrPdoIndex(UNSIGNED16 pdoNr);
void	*icoTrPdoGetObjectAddr(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
void	*icoRecPdoGetObjectAddr(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
void	*icoTrMapGetObjectAddr(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
const void	*icoTrMapGetObjectAddr_R(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
void	*icoRecMapGetObjectAddr(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
const void	*icoRecMapGetObjectAddr_R(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
RET_T	icoTrPdoCheckObjLimit_u8(UNSIGNED16 pdoNr, UNSIGNED8 subIndex,
			UNSIGNED8 u8);
RET_T	icoRecPdoCheckObjLimit_u8(UNSIGNED16 pdoNr, UNSIGNED8 subIndex,
			UNSIGNED8 u8);
RET_T	icoTrPdoCheckObjLimit_u16(UNSIGNED16 pdoNr, UNSIGNED8	subIndex);
RET_T	icoRecPdoCheckObjLimit_u16(UNSIGNED16 pdoNr, UNSIGNED8	subIndex);
RET_T	icoTrPdoCheckObjLimitCobid(UNSIGNED16 pdoNr, UNSIGNED32 canId);
RET_T	icoRecPdoCheckObjLimitCobid(UNSIGNED16 pdoNr, UNSIGNED32 canId);
RET_T	icoTrPdoObjChanged(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
RET_T	icoRecPdoObjChanged(UNSIGNED16 pdoNr, UNSIGNED8 subIndex);
RET_T	icoTrMapCheckObjLimitMapTable(UNSIGNED16 pdoNr, UNSIGNED8 subIndex,
			UNSIGNED8 value);
RET_T	icoTrMapCheckObjLimitMapEntry(UNSIGNED16 pdoNr, UNSIGNED32 value);
RET_T	icoRecMapCheckObjLimitMapTable(UNSIGNED16 pdoNr, UNSIGNED8 subIndex,
			UNSIGNED8 value);
RET_T	icoRecMapCheckObjLimitMapEntry(UNSIGNED16 pdoNr, UNSIGNED32 mapEntry);
RET_T	icoTrPdoMapChanged(UNSIGNED16 pdoNr);
RET_T	icoRecPdoMapChanged(UNSIGNED16 pdoNr);
void	icoPdoReset(void);
void	icoPdoSetDefaultValue(void);
void	icoPdoOperational(void);
void	icoPdoPreOperational(void);
void	icoPdoSyncHandling(UNSIGNED8  syncCounterVal);
void	icoPdoReceiveHandler(CO_CONST CO_REC_DATA_T *pRecData);
void	icoPdoRtrHandler(CO_CONST CO_REC_DATA_T	*pRecData);
void	icoPdoVarInit(UNSIGNED16 *pTrCnt, UNSIGNED16 *pRecCnt);

# ifdef CO_PDO_RECEIVE_CNT
void	icoMPdoReceive(CO_CONST CO_REC_PDO_T *pPdo,
			CO_CONST CO_REC_DATA_T	*pRecData);
# endif /* CO_PDO_RECEIVE_CNT */

# endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */
#endif /* ICO_PDO_H */
