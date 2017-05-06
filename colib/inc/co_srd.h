/*
* co_srd.h - contains defines for srd services
*
* Copyright (c) 2014-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_srd.h 10342 2015-08-24 13:53:52Z boe $
*-------------------------------------------------------------------
*
*
*/

/**
* \brief defines for srd services
*
* \file co_srd.h - contains defines for srd services
*
*/

#ifndef CO_SRD_H
#define CO_SRD_H 1

#include <co_datatype.h>


/* datatypes */

/**
* request type for SDO register
*/
typedef enum {
	CO_SRD_REQ_TYPE_ALL_SDOS = 3,	/**< request all default Server SDOs */
	CO_SRD_REQ_TYPE_NORMAL = 1		/**< request one SDO connection */
} CO_SRD_REQ_TYPE_T;


/**
* result values for indication function
*/
typedef enum {
	CO_SRD_RESULT_SUCCESS,			/**< requested service ok */
	CO_SRD_RESULT_TIMEOUT,			/**< time out occured, fct aborted */
	CO_SRD_RESULT_ERROR,			/**< error */
	CO_SRD_RESULT_ALL_REQUEST_SUCCESS,	/**< request all sdos ok */
	CO_SRD_RESULT_NODE_REQUEST_SUCCESS	/**< request connection ok */
} CO_SRD_RESULT_T;


/** \brief function pointer to srd result function
 * \param result - result status of action
 * \param errorcode - errorcode if 
 * 
 * \return void
 */
typedef void (* CO_EVENT_SRD_T)(CO_SRD_RESULT_T result, UNSIGNED8 errorCode); /*lint !e960 customer specific parameter names */



/* function prototypes */

EXTERN_DECL RET_T coSrdRequestRegister(CO_SRD_REQ_TYPE_T reqType,
				UNSIGNED8 sdoClientChannel, UNSIGNED32 timeOut);
EXTERN_DECL RET_T coSrdReleaseRegister(UNSIGNED32 timeOut);
EXTERN_DECL RET_T coSrdRequestConnection(
				UNSIGNED8 sdoClientChannel, UNSIGNED8 remoteNodeId,
				UNSIGNED32 timeOut);
EXTERN_DECL RET_T coSrdReleaseConnection(
				UNSIGNED8 sdoClientChannel, UNSIGNED8 remoteNodeId,
				UNSIGNED32 timeOut);
EXTERN_DECL RET_T coEventRegister_SRD(CO_EVENT_SRD_T pFunction);
EXTERN_DECL RET_T coSrdInit(void);

#endif /* CO_SRD_H */

