/*
* co_user.h - contains defines for user transmit/receive services
*
* Copyright (c) 2015-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_user.h 10878 2015-10-02 09:03:47Z boe $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for time services
*
* \file co_user.h - contains defines for time services
*
*/

#ifndef CO_USER_H
#define CO_USER_H 1

#include <co_datatype.h>


/* constant */

/** \brief function pointer to user function 
 *
 * \param msgNr - message number
 * \param dataLen - received data len
 * \param precData - received data
 * 
 * \return void
 */
typedef void (* CO_EVENT_USER_T)(CO_CONST UNSIGNED16 msgNr, CO_CONST UNSIGNED8 dataLen, CO_CONST UNSIGNED8 *pRecData);


/* function prototypes */
EXTERN_DECL RET_T coUserInitTransmitMessage(UNSIGNED16 msgNr,
				UNSIGNED32 canId, UNSIGNED8 dataLen);
EXTERN_DECL RET_T coUserInitReceiveMessage(UNSIGNED16	msgNr,	
				UNSIGNED32 canId, UNSIGNED8 dataLen, CO_EVENT_USER_T pRecFct);
EXTERN_DECL RET_T coUserTransmitMessage(UNSIGNED16 msgNr,
				const UNSIGNED8	*pData);

#endif /* CO_USER_H */
