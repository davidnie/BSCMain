/*
* co_sdo.h - contains defines for sdo services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_sdo.h 15305 2016-09-22 15:29:15Z boe $
*
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for sdo service
*
* \file co_sdo.h - contains defines for sdo service
*/

#ifndef CO_SDO_H
#define CO_SDO_H 1

#include <co_datatype.h>

#define CO_SDO_DYN_BIT   0x40000000ul


/* datatypes */

/** \brief function pointer to SDO server event
 * \param execute - execute or test only
 * \param sdoNr - sdo number
 * \param index - object index
 * \param subindex - object subindex
 * 
 * \return RET_T
 */
typedef	RET_T (* CO_EVENT_SDO_SERVER_T)(BOOL_T, UNSIGNED8, UNSIGNED16, UNSIGNED8); /*lint !e960 customer specific parameter names */


/** \brief function pointer to SDO server write check event
 * \param execute - execute or test only
 * \param sdoNr - sdo number
 * \param index - object index
 * \param subindex - object subindex
 * \param pData - pointer to receive buffer
 * 
 * \return RET_T
 */
typedef RET_T (* CO_EVENT_SDO_SERVER_CHECK_WRITE_T)(BOOL_T, UNSIGNED8, UNSIGNED16, UNSIGNED8, const UNSIGNED8 *); /*lint !e960 customer specific parameter names */

/** \brief function pointer to SDO server write domain event
 * \param index - object index
 * \param subindex - object subindex
 * \param domainBufSize - actual size at domain buffer
 * \param transferSize - actual transfered size
 * 
 * \return RET_T
 */
typedef void (* CO_EVENT_SDO_SERVER_DOMAIN_WRITE_T)(UNSIGNED16, UNSIGNED8, UNSIGNED32, UNSIGNED32);

/** \brief function pointer to SDO client read event
 * \param sdoNr - sdo number
 * \param index - object index
 * \param subindex - object subindex
 * \param size - size of received data
 * \param result - result of transfer
 * 
 * \return void
 */
typedef	void (* CO_EVENT_SDO_CLIENT_READ_T)(UNSIGNED8, UNSIGNED16, UNSIGNED8, UNSIGNED32, UNSIGNED32); /*lint !e960 customer specific parameter names */


/** \brief function pointer to SDO client write event
 * \param sdoNr - sdo number
 * \param index - object index
 * \param subindex - object subindex
 * \param result - result of transfer
 * 
 * \return void
 */
typedef	void (* CO_EVENT_SDO_CLIENT_WRITE_T)(UNSIGNED8, UNSIGNED16, UNSIGNED8, UNSIGNED32); /*lint !e960 customer specific parameter names */


/** \brief function pointer to SDO client domain write event
* \param sdoNr - sdo number
* \param index - object index
* \param subindex - object subindex
* \param transfered - bytes transfered
* \param pointer - pointer to application data
*
* \return RET_T
*/
typedef RET_T (*CO_EVENT_SDO_CLIENT_DOMAIN_WRITE_T)(UNSIGNED8, UNSIGNED16, UNSIGNED8, UNSIGNED32, void*); /*lint !e960 customer specific parameter names */


/* \brief function pointer to SDO queue indication function
 * \param pData - pointer to data
 * \param result - result of sdo transfer
 */
typedef void (* CO_SDO_QUEUE_IND_T)(void *pData, UNSIGNED32 result);

/* function prototypes */
EXTERN_DECL RET_T coSdoServerInit(UNSIGNED8);
EXTERN_DECL RET_T coEventRegister_SDO_SERVER_READ(CO_EVENT_SDO_SERVER_T pFunction);
EXTERN_DECL RET_T coEventRegister_SDO_SERVER_WRITE(CO_EVENT_SDO_SERVER_T pFunction);
EXTERN_DECL RET_T coEventRegister_SDO_SERVER_CHECK_WRITE(CO_EVENT_SDO_SERVER_CHECK_WRITE_T pFunction);
EXTERN_DECL RET_T coEventRegister_SDO_SERVER_DOMAIN_WRITE(CO_EVENT_SDO_SERVER_DOMAIN_WRITE_T pFunction);

EXTERN_DECL RET_T coSdoClientInit(UNSIGNED8);
EXTERN_DECL RET_T coSdoRead(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED8 *pData, UNSIGNED32 dataLen,
				UNSIGNED16 numeric, UNSIGNED32 timeout);
EXTERN_DECL RET_T coSdoWrite(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED8 *pData, UNSIGNED32 dataLen,
				UNSIGNED16 numeric, UNSIGNED32 timeout);
EXTERN_DECL RET_T coSdoDomainWrite(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED8 *pData, UNSIGNED32 dataLen,
				UNSIGNED32 timeout, UNSIGNED32 nbrMsg, CO_EVENT_SDO_CLIENT_DOMAIN_WRITE_T pFunction, void* pFctPara);
EXTERN_DECL RET_T coSdoQueueAddTransfer(BOOL_T write,
				UNSIGNED8 sdoNr, UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED8 *pData, UNSIGNED32 dataLen,
				CO_SDO_QUEUE_IND_T	pFct, void *pFctPara);
#ifdef CO_SDO_BLOCK
EXTERN_DECL RET_T coSdoReadSeg(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED8 *pData, UNSIGNED32 dataLen,
				UNSIGNED16 numeric, UNSIGNED32 timeout);
EXTERN_DECL RET_T coSdoWriteSeg(UNSIGNED8 sdoNr, UNSIGNED16 index,
				UNSIGNED8 subIndex, UNSIGNED8 *pData, UNSIGNED32 dataLen,
				UNSIGNED16 numeric, UNSIGNED32 timeout);
#endif /* CO_SDO_BLOCK */
				
				
EXTERN_DECL RET_T coEventRegister_SDO_CLIENT_READ(CO_EVENT_SDO_CLIENT_READ_T pFunction);
EXTERN_DECL RET_T coEventRegister_SDO_CLIENT_WRITE(CO_EVENT_SDO_CLIENT_WRITE_T pFunction);

EXTERN_DECL RET_T coEventUnregister_SDO_CLIENT_READ(CO_EVENT_SDO_CLIENT_READ_T pFunction);
EXTERN_DECL RET_T coEventUnregister_SDO_CLIENT_WRITE(CO_EVENT_SDO_CLIENT_WRITE_T pFunction);


EXTERN_DECL RET_T coSdoServerReadIndCont(UNSIGNED8 sdoNr,
				RET_T result);
EXTERN_DECL RET_T coSdoServerWriteIndCont(UNSIGNED8 sdoNr,
				RET_T result);

EXTERN_DECL RET_T coSdoNetworkRead(UNSIGNED8 sdoNr,
				UNSIGNED16 network, UNSIGNED8 node,
				UNSIGNED16 index, UNSIGNED8 subIndex, UNSIGNED8 *pData,
				UNSIGNED32 dataLen, UNSIGNED16 numeric, UNSIGNED32 timeout);
EXTERN_DECL RET_T coSdoNetworkWrite(UNSIGNED8	sdoNr,
				UNSIGNED16 network, UNSIGNED8 node,
				UNSIGNED16 index, UNSIGNED8	subIndex, UNSIGNED8	*pData,
				UNSIGNED32 dataLen, UNSIGNED16 numeric, UNSIGNED32 timeout);

# ifdef CO_APPL_SDO_HANDLER
EXTERN_DECL void applSdoHandler(UNSIGNED16 sdoSpec,
				const CO_CAN_MSG_T *pCanMsg);
EXTERN_DECL RET_T coSdoServerApplAnswer(UNSIGNED16 sdoSpec,
				CO_CAN_MSG_T *pCanMsg);
# endif /* CO_APPL_SDO_HANDLER */

#endif /* CO_SDO_H */

