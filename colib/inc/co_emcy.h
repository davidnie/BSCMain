/*
* co_emcy.h - contains defines for emcy services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_emcy.h 10342 2015-08-24 13:53:52Z boe $
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for emcy services
*
* \file co_emcy.h - contains defines for emcy services
*
*/

#ifndef CO_EMCY_H
#define CO_EMCY_H 1

#include <co_datatype.h>


/* constant */

/**
* define for Emergency Error Code wrong PDO length 
*/
#define CO_EMCY_ERRCODE_PDO_LEN		0x8210u

/**
* define for Emergency Error Code communication error
*/
#define CO_EMCY_ERRCODE_COMM_ERROR	0x8130u


/* datatypes */

/** \brief function pointer to emergency function 
 *
 * \param errCode - emergency error code
 * \param addErrorCode - pointer to 5 bytes error code
 * 
 * \return RET_T
 * \retval RET_OK
 *	send emergency
 * \retval RET_xx
 *	don't send emergency
 */
typedef RET_T (* CO_EVENT_EMCY_T)(UNSIGNED16 errCode, const UNSIGNED8 *addErrorCode);


/** \brief function pointer to emergency consumer function 
 *
 * \param node - node id of received emergency
 * \param errCode - emergency error code
 * \param errorRegister - emergency error register
 * \param addErrorCode - pointer to 5 bytes error code
 * 
 * \return none
 */
typedef void (* CO_EVENT_EMCY_CONS_T)(UNSIGNED8 node, UNSIGNED16 errCode, UNSIGNED8 errorRegister, UNSIGNED8 const *addErrorCode);

/* function prototypes */
EXTERN_DECL RET_T coEmcyProducerInit(void);
EXTERN_DECL RET_T coEmcyConsumerInit(UNSIGNED8 emcyCnt);
EXTERN_DECL RET_T coEmcyWriteReq(UNSIGNED16 emcyErrCode,
			CO_CONST UNSIGNED8 *pData);
EXTERN_DECL RET_T coEventRegister_EMCY(CO_EVENT_EMCY_T pFunction);
EXTERN_DECL RET_T coEventRegister_EMCY_CONSUMER(CO_EVENT_EMCY_CONS_T pFunction);

#endif /* CO_EMCY_H */
