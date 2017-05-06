/*
* co_manager.h - contains defines for nmt services
*
* Copyright (c) 2014-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_manager.h 10282 2015-08-18 15:04:56Z boe $
*-------------------------------------------------------------------
*
*
*/

/**
* \brief defines for bootup manager services
*
* \file co_manager.h - contains defines for bootup manager services
*
*/

#ifndef CO_MANAGER_H
#define CO_MANAGER_H 1

#include <co_datatype.h>


/* datatypes */

/**
* manager bootup events
*/
typedef enum {
	CO_MANAGER_EVENT_BOOT,			/**< node x start boot */
	CO_MANAGER_EVENT_ERROR_B,		/**< node x read 0x1000 failed */
	CO_MANAGER_EVENT_ERROR_C,		/**< node x check device type failed */
	CO_MANAGER_EVENT_ERROR_D,		/**< node x check vendor id type failed */
	CO_MANAGER_EVENT_ERROR_J,		/**< node x check configuration failed */
	CO_MANAGER_EVENT_ERROR_G,		/**< node x update configuration failed */
	CO_MANAGER_EVENT_ERROR_K,		/**< node x start errctl failed */
	CO_MANAGER_EVENT_ERROR_M,		/**< node x check product code failed */
	CO_MANAGER_EVENT_ERROR_N,		/**< node x check version nr failed */
	CO_MANAGER_EVENT_ERROR_O,		/**< node x check serial nr failed */
	CO_MANAGER_EVENT_UPDATE_SW,		/**< node x software update necessary */
	CO_MANAGER_EVENT_UPDATE_CONFIG,	/**< node x update config necessary */
	CO_MANAGER_EVENT_BOOTED,		/**< node x boot successfully */
	CO_MANAGER_EVENT_ERROR_NODE,	/**< node x failure heartbeat */
	CO_MANAGER_EVENT_RDY_OPERATIONAL,/**< manager ready for OPERATIONAL */
	CO_MANAGER_EVENT_FAILURE,		/**< bootup failure about mandatory slave*/
	CO_MANAGER_EVENT_FINISHED		/**< bootup finished without errors */
} CO_MANAGER_EVENT_T;


/** \brief function pointer to NMT event function 
 * \param node - node id for event (0 = manager event)
 * \param event - event type from CO_MANAGER_EVENT_T
 * 
 * \return void
 *
 */
typedef void (* CO_EVENT_MANAGER_BOOTUP_T)(UNSIGNED8, CO_MANAGER_EVENT_T); /*lint !e960 customer specific parameter names */



/* function prototypes */
EXTERN_DECL RET_T	coEventRegister_MANAGER_BOOTUP(
					CO_EVENT_MANAGER_BOOTUP_T pFunction);
EXTERN_DECL RET_T	coManagerStart(void);
EXTERN_DECL RET_T	coManagerContinueSwUpdate(UNSIGNED8 slave,
					RET_T result);
EXTERN_DECL RET_T	coManagerContinueConfigUpdate(UNSIGNED8 slave,
					RET_T result);
EXTERN_DECL RET_T	coManagerContinueOperational(void);

#endif /* CO_MANAGER_H */
