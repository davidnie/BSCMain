/*
* co_stackinit.c - contains functions for stack initialisation
*
* Copyright (c) 2014-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_stackinit.c 13747 2016-05-04 08:10:13Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief Functions for stack intialization handling
*
* \file co_stackinit.c contains functions for initialization handling
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_datatype.h>
#include <co_drv.h>
#include <co_timer.h>
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_pdo.h"
#include "ico_commtask.h"
#include <co_canopen.h>
#include "ico_emcy.h"
#include "ico_event.h"
#include "ico_indication.h"
#include "ico_led.h"
#include "ico_lss.h"
#include "ico_nmt.h"
#include "ico_odaccess.h"
#include "ico_sdoserver.h"
#include "ico_sdoclient.h"
#include "ico_store.h"
#include "ico_sync.h"
#include "ico_srd.h"
#include "ico_sleep.h"
#include "ico_time.h"
#include "ico_candebug.h"
#ifdef CO_NETWORK_ROUTING_CNT
# include "ico_network.h"
#endif /* CO_NETWORK_ROUTING_CNT */
#ifdef CO_USER_EXTENSION_SUPPORTED
# include "ico_user.h"
#endif /* CO_USER_EXTENSION_SUPPORTED */
#ifdef CO_SRDO_SUPPORTED
# include "ico_srdo.h"
#endif /* CO_SRDO_SUPPORTED */
#ifdef CO_GFC_SUPPORTED
#include "ico_gfc.h"
#endif /* CO_GFC_SUPPORTED */

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/



/***************************************************************************/
/**
* \brief coCanOpenStackVarInit - init of variables of the stack
*
* This function initializes all global and local variables of the stack.
* 
* It can also be used to reinitialize the stack.
*
* \return nothing
*
*/
void coCanOpenStackVarInit(
		CO_SERVICE_INIT_VAL_T *pServiceInitVals	/**< pointer to init vals */
	)
{
	icoCobHandlerVarInit();
	icoCommTaskVarInit();

	icoEmcyVarInit(&pServiceInitVals->emcyErrHistCnt[0],
		&pServiceInitVals->emcyConsCnt[0]);

	icoErrorCtrlVarInit(&pServiceInitVals->errorCtrlCnt[0]);
	icoEventInit();

#ifdef CO_EVENT_LED
	icoLedVarInit();
#endif /* CO_EVENT_LED */

#ifdef CO_LSS_SUPPORTED
	icoLssVarInit();
#endif /* CO_LSS_SUPPORTED */

#ifdef CO_LSS_MASTER_SUPPORTED
	icoLssMasterVarInit();
#endif /* CO_LSS_MASTER_SUPPORTED */

	icoNmtVarInit(&pServiceInitVals->nodeId[0], &pServiceInitVals->nodeIdFct[0]);

#ifdef CO_NMT_MASTER
	icoNmtMasterVarInit(&pServiceInitVals->slaveAssign[0]);

# ifdef CO_BOOTUP_MANAGER
	icoManagerVarInit(&pServiceInitVals->slaveAssign[0]);
# endif /* CO_BOOTUP_MANAGER */

# ifdef CO_FLYING_MASTER_SUPPORTED
	icoFlymaVarInit();
# endif /* CO_FLYING_MASTER_SUPPORTED */
#endif /* CO_NMT_MASTER */

	icoOdAccessVarInit();

#if defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT)
	icoPdoVarInit(&pServiceInitVals->pdoTrCnt[0],
		&pServiceInitVals->pdoRecCnt[0]);
#endif /* defined(CO_PDO_TRANSMIT_CNT) || defined(CO_PDO_RECEIVE_CNT) */

	icoQueueVarInit();

#ifdef CO_SDO_CLIENT_CNT
	icoSdoClientVarInit(&pServiceInitVals->sdoClientCnt[0]);
#endif /* CO_SDO_CLIENT_CNT */

#ifdef CO_SDO_QUEUE
	icoSdoQueueVarInit();
#endif /* CO_SDO_QUEUE */

	icoSdoServerVarInit(&pServiceInitVals->sdoServerCnt[0]);

#ifdef CO_EVENT_SLEEP
	icoSleepVarInit();
#endif /* CO_EVENT_SLEEP */

#ifdef CO_SRD_SUPPORTED
	icoSrdVarInit();
#endif

#ifdef CO_STORE_SUPPORTED
	icoStoreVarInit();
#endif /* CO_STORE_SUPPORTED */

#ifdef CO_SYNC_SUPPORTED
	icoSyncVarInit();
#endif /* CO_SYNC_SUPPORTED */

#ifdef CO_TIME_SUPPORTED
	icoTimeVarInit();
#endif /* CO_IME_SUPPORTED */

#ifdef CO_SRDO_SUPPORTED
	icoSrdoVarInit();
#endif /* CO_SRDO_SUPPORTED */

#ifdef CO_GUARDING_CNT
	icoGuardingVarInit(&pServiceInitVals->guardingCnt[0]);
#endif /* CO_GUARDING_CNT */

#ifdef CO_NETWORK_ROUTING_CNT
	icoNetworkVarInit(&pServiceInitVals->networkCnt[0]);
#endif /* CO_NETWORK_ROUTING_CNT */

#ifdef CO_GFC_SUPPORTED
	icoGfcVarInit();
#endif /* CO_GFC_SUPPORTED */

#ifdef CO_CAN_DEBUG_SUPPORTED
	icoCanDebugVarInit();
#endif /* CO_CAN_DEBUG_SUPPORTED */

#ifdef CO_USER_EXTENSION_SUPPORTED
	icoUserVarInit();
#endif /* CO_USER_EXTENSION_SUPPORTED */
}

