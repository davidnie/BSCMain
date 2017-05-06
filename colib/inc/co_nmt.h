/*
* co_nmt.h - contains defines for nmt services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_nmt.h 15305 2016-09-22 15:29:15Z boe $
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for nmt services
*
* \file co_nmt.h - contains defines for nmt services
*
*/

#ifndef CO_NMT_H
#define CO_NMT_H 1

#include <co_datatype.h>


/* datatypes */

/**
* error control states
*/
typedef enum {
	CO_ERRCTRL_BOOTUP,			/**< bootup */
	CO_ERRCTRL_NEW_STATE,		/**< NMT state changed */
	CO_ERRCTRL_HB_STARTED,		/**< heartbeat started */
	CO_ERRCTRL_HB_FAILED,		/**< heartbeat failed */
	CO_ERRCTRL_GUARD_FAILED,	/**< Lifetime failure from master detected */
	CO_ERRCTRL_MGUARD_TOGGLE,	/**< Master guarding toggle failure detected */
	CO_ERRCTRL_MGUARD_FAILED,	/**< Master guarding failure detected */
	CO_ERRCTRL_BOOTUP_FAILURE	/**< error transmit bootup */
} CO_ERRCTRL_T;


/**
* NMT states 
*/
typedef enum {
	CO_NMT_STATE_UNKNOWN = 0,		/**< unknown */
	CO_NMT_STATE_OPERATIONAL = 5,	/**< OPERATIONAL */
	CO_NMT_STATE_STOPPED = 4,		/**< STOPPED */
	CO_NMT_STATE_PREOP = 127,		/**< PRE-OPERATIONAL */
	CO_NMT_STATE_RESET_NODE = 128,	/**< Reset NODE */
	CO_NMT_STATE_RESET_COMM = 129	/**< Reset Communication */
} CO_NMT_STATE_T;


/**
* NMT REQ states 
*/
typedef enum {
	CO_NMT_REQ_STATE_STOPPED = 4,		/**< STOPPED */
	CO_NMT_REQ_STATE_OPERATIONAL = 5,	/**< OPERATIONAL */
	CO_NMT_REQ_STATE_RESET_NODE = 6,	/**< Reset NODE */
	CO_NMT_REQ_STATE_RESET_COMM = 7,	/**< Reset Communication */
	CO_NMT_REQ_STATE_PREOP = 127		/**< PRE-OPERATIONAL */
} CO_NMT_REQ_STATE_T;


/** \brief function pointer to get node id function
 * This function shall get the node id for the device
 *
 * \return node id
 */
typedef UNSIGNED8 (* CO_NODE_ID_T)(void); /*lint !e960 customer specific parameter names */


/** \brief function pointer to error control event function 
 * \param nodeId - node Id
 * \param errCtrlState - error control state
 * \param nmtState - actual NMT state
 * 
 * \return void
 */
typedef void (* CO_EVENT_ERRCTRL_T)(UNSIGNED8, CO_ERRCTRL_T, CO_NMT_STATE_T); /*lint !e960 customer specific parameter names */


/** \brief function pointer to NMT event function 
 * \param execute - execute status change y/n
 * \param nmtState - new NMT state
 * 
 * \return RET_T
 * \retval RET_OK - state change allowed (only valid for OPERATIONAL)
 * \retval RET_ - state change not allowed (only valid for OPERATIONAL)
 */
typedef RET_T (* CO_EVENT_NMT_T)(BOOL_T, CO_NMT_STATE_T); /*lint !e960 customer specific parameter names */



/* function prototypes */
EXTERN_DECL RET_T coNmtInit(UNSIGNED8);
EXTERN_DECL UNSIGNED8 coNmtGetNodeId(void);
EXTERN_DECL BOOL_T	coNmtInhibitActive(void);
EXTERN_DECL RET_T coErrorCtrlInit(UNSIGNED16, UNSIGNED8);

EXTERN_DECL RET_T coEventRegister_ERRCTRL(CO_EVENT_ERRCTRL_T pFunction);
EXTERN_DECL RET_T coEventRegister_NMT(CO_EVENT_NMT_T pFunction);

EXTERN_DECL CO_NMT_STATE_T coNmtGetState(void);
EXTERN_DECL CO_NMT_STATE_T coNmtGetRemoteNodeState(
					UNSIGNED8 nodeId);
EXTERN_DECL RET_T coNmtStateReq(UNSIGNED8 node,
					CO_NMT_STATE_T reqState, BOOL_T master);
EXTERN_DECL RET_T coNmtLocalStateReq(CO_NMT_STATE_T reqState);
EXTERN_DECL BOOL_T coNmtNodeIsMaster(void);

EXTERN_DECL RET_T coHbConsumerSet(UNSIGNED8 node,
					UNSIGNED16 hbTime);
EXTERN_DECL RET_T coHbConsumerStart(UNSIGNED8 node);
EXTERN_DECL RET_T coGuardingMasterStart(UNSIGNED8	node);
EXTERN_DECL RET_T coGuardingMasterStop(UNSIGNED8	node);


#endif /* CO_NMT_H */
