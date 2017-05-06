/*
* co_lssmaster.c - contains LSS master services
*
* Copyright (c) 2012-2016 emtas GmbH
*-------------------------------------------------------------------
* $Id: co_lssmaster.c 14540 2016-07-15 08:44:26Z boe $
*
*
*-------------------------------------------------------------------
* Changelog:
*
*
*/

/********************************************************************/
/**
* \brief LSS master handling
*
* \file co_lssmaster.c
* contains LSS master services
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h> /*lint !e766 depends on configuration for CO_LSS_MASTER_SUPPORTED */
#ifdef CO_LSS_MASTER_SUPPORTED
#include <stdlib.h>
#include <string.h>

#include <co_datatype.h>
#include <co_nmt.h>
#include <co_drv.h>
#include <co_timer.h>
#include <co_odaccess.h>
#include <co_lss.h>
#include "ico_indication.h"
#include "ico_cobhandler.h"
#include "ico_queue.h"
#include "ico_lss.h"

/* constant definitions
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_DYNAMIC_LSS
# ifdef CO_EVENT_PROFILE_LSS
#  define CO_EVENT_LSS_CNT      (CO_EVENT_DYNAMIC_LSS + CO_EVENT_PROFILE_LSS)
# else /* CO_EVENT_PROFILE_LSS */
#  define CO_EVENT_LSS_CNT      (CO_EVENT_DYNAMIC_LSS)
# endif /* CO_EVENT_PROFILE_LSS */
#else /* CO_EVENT_DYNAMIC_LSS */
# ifdef CO_EVENT_PROFILE_LSS
#  define CO_EVENT_LSS_CNT      (CO_EVENT_PROFILE_LSS)
# endif /* CO_EVENT_PROFILE_LSS */
#endif /* CO_EVENT_DYNAMIC_LSS */

#if defined(CO_EVENT_STATIC_LSS) || defined(CO_EVENT_LSS_CNT)
# define CO_EVENT_LSS   1u
#endif /* defined(CO_EVENT_STATIC_LSS) || defined(CO_EVENT_LSS_CNT) */



/* local defined data types
---------------------------------------------------------------------------*/
typedef struct  {
	UNSIGNED32	idNr;			/* complete id number */
	UNSIGNED8	bitCheck;		/* number of valid bits */
} FASTSCAN_DATA_T;	


/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
#ifdef CO_EVENT_STATIC_LSS
extern CO_CONST CO_EVENT_LSS_MASTER_T coEventLssInd;
#endif /* CO_EVENT_STATIC_LSS */


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static RET_T lssStartCommand(const CO_TRANS_DATA_T *pData,
		CO_TIMER_FCT_T timeOutFct, UNSIGNED16 timeOutVal);
static void lssMasterTimeout(void *pData);
static void lssMasterFastScanTimeout(void *pData);
static void answerRemoteSlave(void);
static void answerIdentifySlave(void);
static void answerFastScan(UNSIGNED8 bitval);
static void answerNodeId(UNSIGNED8 errorCode, UNSIGNED8 errorSpec);
static void answerBitrate(UNSIGNED8 errorCode, UNSIGNED8 errorSpec);
static RET_T lssNonConfiguredSlaves(UNSIGNED16	timeOutVal);
static void nonConfiguredSlaves(void *pData);
static void answerStore(UNSIGNED8 errorCode, UNSIGNED8 errorSpec);
static void answerInquireNodeId(UNSIGNED8 nodeId);
static void bitrateSwitchFct(void *pData);
static void answerSwitchSelective(void);

#ifdef CO_EVENT_LSS
static void lssEvent(CO_LSS_MASTER_SERVICE_T service,
		UNSIGNED16 errorCode, UNSIGNED8 errorSpec);
#endif /* CO_EVENT_LSS */

#ifdef CO_LSS_INQUIRY_SERVICES
static void answerInquireService(UNSIGNED8 cmd,
		CO_CONST UNSIGNED8 *pData);
#endif /* CO_LSS_INQUIRY_SERVICES */

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
static COB_REFERENZ_T	lssTransCob;
static COB_REFERENZ_T	lssRecCob;
static CO_TIMER_T	lssTimer;
static CO_TIMER_T	lssNonConfigSlavesTimer;
static CO_TIMER_T	lssBitrateTimer;
static UNSIGNED8	lssCmd = { 0u };
static FASTSCAN_DATA_T	vendor;
static FASTSCAN_DATA_T	product;
static FASTSCAN_DATA_T	version;
static FASTSCAN_DATA_T	serial;
static UNSIGNED8	lssSub = { 0u };
static UNSIGNED16	fsTimeOut;
static UNSIGNED8	completeAgain = { 0u };
static UNSIGNED8	identifyBitVal = { 1u };
static CO_LSS_MASTER_SERVICE_T	bitrateSwitch;
/* static BOOL_T		lssMasterEnabled; */
static BOOL_T		lssMasterInit = { CO_FALSE };

#ifdef CO_EVENT_LSS_CNT
static UNSIGNED16	lssEventTableCnt = 0u;
static CO_EVENT_LSS_MASTER_T lssEventTable[CO_EVENT_LSS_CNT];
#endif /* CO_EVENT_LSS_CNT */

#ifdef CO_LSS_INQUIRY_SERVICES
static UNSIGNED32	inquireData;
#endif /* CO_LSS_INQUIRY_SERVICES */



/***************************************************************************/
/**
* \internal
*
* \brief icoLssMasterHandler - lss master handler
*
*
* \return none
*
*/
void icoLssMasterHandler(
		const UNSIGNED8	pData[]
	)
{
	/* start service depending on lss command */
	switch (pData[0])  {
		case LSS_CS_NON_CONFIG_SLAVE:
			answerRemoteSlave();
			break;

		case LSS_CS_IDENTITY_SLAVE:
			answerIdentifySlave();
			break;

		case LSS_CS_NODE_ID:
			answerNodeId(pData[1], pData[2]);
			break;

		case LSS_CS_BITRATE:
			answerBitrate(pData[1], pData[2]);
			break;

		case LSS_CS_STORE:
			answerStore(pData[1], pData[2]);
			break;

		case LSS_CS_INQUIRE_NODEID:
			answerInquireNodeId(pData[1]);
			break;

#ifdef CO_LSS_INQUIRY_SERVICES
		case LSS_CS_INQUIRE_VENDOR:
		case LSS_CS_INQUIRE_PRODUCT:
		case LSS_CS_INQUIRE_REVISION:
		case LSS_CS_INQUIRE_SERIAL:
			answerInquireService(pData[0], &pData[1]);
			break;
#endif /* CO_LSS_INQUIRY_SERVICES */

		case LSS_CS_SWITCH_SEL_ANSWER:
			answerSwitchSelective();
			break;

		default:
			/* ignore unknown commands */
			break;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief answerRemoteSlave - answer for non configured slaves
*
*
* \return none
*
*/
static void answerRemoteSlave(
		void	/* no parameter */
	)
{
	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

#ifdef CO_EVENT_LSS
	lssEvent(CO_LSS_MASTER_SERVICE_NON_CONFIG_SLAVE, 0, 0);
#endif /* CO_EVENT_LSS */
}


/***************************************************************************/
/**
* \internal
*
* \brief answerIdentifySlave - identify slave received
*
*
* \return none
*
*/
static void answerIdentifySlave(
		void	/* no parameter */
	)
{
	if (lssCmd == LSS_CS_FAST_SCAN)  {
		identifyBitVal = 0;
	}

	if (lssCmd == LSS_CS_IDENTITY_SLAVE)  {
		/* stop timer */
		(void)coTimerStop(&lssTimer);

		/* reset command */
		lssCmd = 0;

		/* call user indication */
		lssEvent(CO_LSS_MASTER_SERVICE_IDENTITY, 0, 0);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief answerNodeId - answer for node id request
*
*
* \return none
*
*/
static void answerNodeId(
		UNSIGNED8	errorCode,
		UNSIGNED8	errorSpec
	)
{
	if (lssCmd != LSS_CS_NODE_ID)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	/* call user indication */
	lssEvent(CO_LSS_MASTER_SERVICE_SET_NODEID, errorCode, errorSpec);
}


/***************************************************************************/
/**
* \internal
*
* \brief answerBitrate - answer for bitrate request
*
*
* \return none
*
*/
static void answerBitrate(
		UNSIGNED8	errorCode,
		UNSIGNED8	errorSpec
	)
{
	if (lssCmd != LSS_CS_BITRATE)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	/* call user indication */
	lssEvent(CO_LSS_MASTER_SERVICE_SET_BITRATE, errorCode, errorSpec);
}


/***************************************************************************/
/**
* \internal
*
* \brief answerStore - answer for store request
*
*
* \return none
*
*/
static void answerStore(
		UNSIGNED8	errorCode,
		UNSIGNED8	errorSpec
	)
{
	if (lssCmd != LSS_CS_STORE)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	/* call user indication */
	lssEvent(CO_LSS_MASTER_SERVICE_STORE, errorCode, errorSpec);
}


/***************************************************************************/
/**
* \internal
*
* \brief answerInqureNodeId
*
*
* \return none
*
*/
static void answerInquireNodeId(
		UNSIGNED8	nodeId
	)
{
	if (lssCmd != LSS_CS_INQUIRE_NODEID)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	/* call user indication */
	lssEvent(CO_LSS_MASTER_SERVICE_INQUIRE_NODEID, 0, nodeId);
}


#ifdef CO_LSS_INQUIRY_SERVICES
/***************************************************************************/
/**
* \internal
*
* \brief answerInqureNodeId
*
*
* \return none
*
*/
static void answerInquireService(
		UNSIGNED8	cmd,
		CO_CONST UNSIGNED8	*pData
	)
{

	if (lssCmd != cmd)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	(void)coNumMemcpyPack(&inquireData, pData, 4u, 1u, CO_ATTR_NUM);

	/* call user indication */
	switch (cmd) {
		case LSS_CS_INQUIRE_VENDOR:
			lssEvent(CO_LSS_MASTER_SERVICE_INQUIRE_VENDOR, 0u, 0u);
			break;
		case LSS_CS_INQUIRE_PRODUCT:
			lssEvent(CO_LSS_MASTER_SERVICE_INQUIRE_PRODUCT, 0u, 0u);
			break;
		case LSS_CS_INQUIRE_REVISION:
			lssEvent(CO_LSS_MASTER_SERVICE_INQUIRE_REVISION, 0u, 0u);
			break;
		case LSS_CS_INQUIRE_SERIAL:
			lssEvent(CO_LSS_MASTER_SERVICE_INQUIRE_SERIAL, 0u, 0u);
			break;
		default:
			break;
	}
}
#endif /* CO_LSS_INQUIRY_SERVICES */


/***************************************************************************/
/**
* \internal
*
* \brief answerInquireNodeId
*
*
* \return none
*
*/
static void answerSwitchSelective(
		void	/* no parameter */
	)
{
	if (lssCmd != LSS_CS_SWITCH_SEL_ANSWER)  {
		return;
	}

	/* stop timer */
	(void)coTimerStop(&lssTimer);

	/* reset command */
	lssCmd = 0;

	/* call user indication */
	lssEvent(CO_LSS_MASTER_SERVICE_SWITCH_SELECTIVE, 0, 0);
}


/***************************************************************************/
/**
* \brief coLssIdentifyNonConfiguredSlaves - identify unconfigured remote slaves
*
* Identify unconfigured remote slaves by sending the LSS command.
* If no slave is available, the indication function is called
* after the time is up, given by the parameter timeoutvalue.
*
* The LSS command is transmitted in a cycle of n seconds,
* given by parameter interval.
* If the parameter is 0, the LSS command is transmitted only once.
*
* \return RET_T
*
*/
RET_T coLssIdentifyNonConfiguredSlaves(
		UNSIGNED16	timeOutVal,			/**< timeout value in msec */
		UNSIGNED16	interval			/**< interval in seconds */
	)
{
RET_T	retVal;
static UNSIGNED16	timeOut;

	retVal = lssNonConfiguredSlaves(timeOutVal);
	if (retVal != RET_OK)   {
		return(retVal);
	}

	if (interval != 0u)  {
		timeOut = timeOutVal;
		retVal = coTimerStart(&lssNonConfigSlavesTimer, interval * 1000000ul,
			nonConfiguredSlaves, &timeOut, CO_TIMER_ATTR_ROUNDUP_CYCLIC);
	} else {
		(void)coTimerStop(&lssNonConfigSlavesTimer);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssIdentifyRemoteSlaves - identify remote slaves
*
* Identify remote slaves by sending the LSS command
* with the given identity parameter.
* If no slave is available, the indication function is called
* after the time is up, given by the parameter timeoutvalue.
*
*
* \return RET_T
*
*/
RET_T coLssIdentifyRemoteSlaves(
		UNSIGNED32	vendorId,			/**< vendor id */
		UNSIGNED32	productCode,		/**< product code */
		UNSIGNED32	revisionLow,		/**< revision low */
		UNSIGNED32	revisionHigh,		/**< revision high */
		UNSIGNED32	serialNumberLow,	/**< serialNumber low */
		UNSIGNED32	serialNumberHigh,	/**< serialNumber high */
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0, 8);

	trData.data[0] = LSS_CS_IDENT_VENDOR;
	memcpy(&trData.data[1], &vendorId, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, NULL, 0);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	trData.data[0] = LSS_CS_IDENT_PRODUCT;
	memcpy(&trData.data[1], &productCode, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, NULL, 0);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	trData.data[0] = LSS_CS_IDENT_REVISION_LOW;
	memcpy(&trData.data[1], &revisionLow, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, NULL, 0);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	trData.data[0] = LSS_CS_IDENT_REVISION_HIGH;
	memcpy(&trData.data[1], &revisionHigh, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, NULL, 0);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	trData.data[0] = LSS_CS_IDENT_SERIAL_LOW;
	memcpy(&trData.data[1], &serialNumberLow, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, NULL, 0);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	trData.data[0] = LSS_CS_IDENT_SERIAL_HIGH;
	memcpy(&trData.data[1], &serialNumberHigh, 4);
	lssCmd = 0;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	lssCmd = LSS_CS_IDENTITY_SLAVE;

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssFastScan - start fastscan 
*
* start fastscan for the given parameter
* if no node was found, the indication will be return 0
*
* \return RET_T
*
*/
RET_T coLssFastScan(
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	/* send lss reset to all nodes */
	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_FAST_SCAN;
	trData.data[5] = 128u;
	retVal = lssStartCommand(&trData, lssMasterFastScanTimeout, timeOutVal);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	identifyBitVal = 1u;

	lssSub = 0u;
	completeAgain = 0u;
	fsTimeOut = timeOutVal;
	vendor.idNr = 0u;
	vendor.bitCheck = 32u;
	product.idNr = 0u;
	product.bitCheck = 32u;
	version.idNr = 0u;
	version.bitCheck = 32u;
	serial.idNr = 0u;
	serial.bitCheck = 32u;

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssFastScanKnownDevice - start fastscan for known device
*
* start fastscan for a well known device
* if no node was found, the indication will be return 0
*
* \return RET_T
*
*/
RET_T coLssFastScanKnownDevice(
		UNSIGNED32	vendorId,			/**< vendor number */
		UNSIGNED32	productCode,		/**< product code */
		UNSIGNED32	versionNr,			/**< version number */
		UNSIGNED32	serNr,				/**< serial number */
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	/* send lss reset to all nodes */
	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_FAST_SCAN;
	trData.data[5] = 128u;
	retVal = lssStartCommand(&trData, lssMasterFastScanTimeout, timeOutVal);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	identifyBitVal = 1u;

	lssSub = 0u;
	completeAgain = 1u;
	fsTimeOut = timeOutVal;
	vendor.idNr = vendorId;
	vendor.bitCheck = 0u;
	product.idNr = productCode;
	product.bitCheck = 0u;
	version.idNr = versionNr;
	version.bitCheck = 0u;
	serial.idNr = serNr;
	serial.bitCheck = 0u;

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssNodeId - set node id for remote node
*
* Send a new node id to an remote slave
* The node has to be set before in configuration mode
*
* \return RET_T
*
*/
RET_T coLssSetNodeId(
		UNSIGNED8	nodeId,				/**< new node id */
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	/* send lss reset to all nodes */
	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_NODE_ID;
	trData.data[1] = nodeId;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssSetBitrate - set bitrate for remote nodes
*
* Send a new bitrate to an remote slaves.
* Allowed bitrates are:
* 1000, 800, 500, 250, 125, 50, 20, 10, 0 (for autobaud)
*
* The nodes have to be set before in configuration mode
*
* \return RET_T
*
*/
RET_T coLssSetBitrate(
        UNSIGNED16	bitRate,			/**< new bitrate */
        UNSIGNED16	timeOutVal			/**< timeout value in msec */
    )
{
RET_T	retVal;
UNSIGNED8   tableIndex = 0xff;

    switch (bitRate)  {
        case 1000:  tableIndex = 0u;
            break;
        case 800:   tableIndex = 1u;
            break;
        case 500:   tableIndex = 2u;
            break;
        case 250:   tableIndex = 3u;
            break;
        case 125:   tableIndex = 4u;
            break;
        case 50:    tableIndex = 6u;
            break;
        case 20:    tableIndex = 7u;
            break;
        case 10:    tableIndex = 8u;
            break;
        case 0:     tableIndex = 9u;
            break;
        default:    tableIndex = 0xffu;
            break;
    }

    if (tableIndex == 0xff)  {
        return(RET_VALUE_NOT_AVAILABLE);
    }

    retVal = coLssSetBitrateTable(0u, tableIndex, timeOutVal);

    return(retVal);
}


/***************************************************************************/
/**
* \brief coLssSetBitrate - set bitrate for remote nodes
*
* Send a new bitrate to an remote slaves.
* Parameter are not checked!
* The nodes have to be set before in configuration mode
*
* \return RET_T
*
*/
RET_T coLssSetBitrateTable(
        UNSIGNED8 tableSelector,        /**< table selector */
        UNSIGNED8 tableIndex,           /**< table index */
        UNSIGNED16	timeOutVal			/**< timeout value in msec */
    )
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_BITRATE;
	trData.data[1] = tableSelector;
	trData.data[2] = tableIndex;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssActivateBitrate - activate bitratenodes
*
* Start service activate bitrate for remote and local node.
* The function transmits the command to the remote slaves
* and start the timer for bitrate switch itself.
* After the time was elapsed, the indication is called.
*
* \return RET_T
*
*/
RET_T coLssActivateBitrate(
        UNSIGNED16 switchDelay     /**< switch delay time */
    )
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_ACTIVATE_BITRATE;
	trData.data[1] = switchDelay & 0xffu;
	trData.data[2] = switchDelay >> 8u;
	retVal = lssStartCommand(&trData, NULL, 0u);
	lssCmd = 0u;
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* start timer for own switching */
	retVal = coTimerStart(&lssBitrateTimer, switchDelay * 1000u,
			bitrateSwitchFct, NULL, CO_TIMER_ATTR_ROUNDUP_CYCLIC);
	bitrateSwitch = CO_LSS_MASTER_SERVICE_BITRATE_OFF;
	lssEvent(CO_LSS_MASTER_SERVICE_BITRATE_OFF, 0u, 0u);

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssStoreConfig - store configuration
*
* Send the LSS store configuration command to a slave.
*
* \return RET_T
*
*/
RET_T coLssStoreConfig(
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	/* send lss reset to all nodes */
	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_STORE;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssSwitchGlobal - send global switch command
*
* Send the global switch command - no answer expected
*
* \return RET_T
*
*/
RET_T coLssSwitchGlobal(
		CO_LSS_STATE_T	mode			/**< mode */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	if ((mode != CO_LSS_STATE_WAITING) && (mode != CO_LSS_STATE_CONFIGURATION)) {
		return(RET_PARAMETER_INCOMPATIBLE);
	}

	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_SWITCH_GLOBAL;
	trData.data[1] = (UNSIGNED8)mode;
	retVal = lssStartCommand(&trData, NULL, 0u);
	lssCmd = 0u;

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssSwitchSelective - send Selective switch command
*
* Send the Selective switch command - the detected node
* should be answer and go into CONFIGURATION mode
*
* \return RET_T
*
*/
RET_T coLssSwitchSelective(
		UNSIGNED32	vendorId,			/**< vendor number */
		UNSIGNED32	productCode,		/**< product code */
		UNSIGNED32	versionNr,			/**< version number */
		UNSIGNED32	serNr,				/**< serial number */
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);

	/* vendor id */
	trData.data[0] = LSS_CS_SWITCH_SEL_VENDOR;
	memcpy(&trData.data[1], &vendorId, 4u);
	retVal = lssStartCommand(&trData, NULL, 0u);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* product code */
	trData.data[0] = LSS_CS_SWITCH_SEL_PRODUCT;
	memcpy(&trData.data[1], &productCode, 4u);
	lssCmd = 0u;
	retVal = lssStartCommand(&trData, NULL, 0u);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* revision */
	trData.data[0] = LSS_CS_SWITCH_SEL_REVISION;
	memcpy(&trData.data[1], &versionNr, 4u);
	lssCmd = 0u;
	retVal = lssStartCommand(&trData, NULL, 0u);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	/* serialNr */
	trData.data[0] = LSS_CS_SWITCH_SEL_SERIAL;
	memcpy(&trData.data[1], &serNr, 4u);
	lssCmd = 0u;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);
	if (retVal != RET_OK)  {
		return(retVal);
	}
	lssCmd = LSS_CS_SWITCH_SEL_ANSWER;

	return(retVal);
}


/***************************************************************************/
/**
* \brief coLssInquireNodeId - inquire actual node ID
*
* Send the inquire node id command
*
* \return RET_T
*
*/
RET_T coLssInquireNodeId(
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_INQUIRE_NODEID;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}


#ifdef CO_LSS_INQUIRY_SERVICES
/***************************************************************************/
/**
* \brief coLssInquireIdentity - inquire identity data
*
* Send the inquire identity command
*
* Please note - the indication function called after the request is finished
* only indicates an erroronous or error free execution of the request.
* It doesn't delivered the requested data.
* To get the requested data the function coLssMasterGetInquireData()
* have to be used.
*
* \return RET_T
*
*/
RET_T coLssInquireIdentity(
		UNSIGNED8	subIndex,			/**< subIndex of requested identity parameter (1..4) */
		UNSIGNED16	timeOutVal			/**< timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);
	switch (subIndex)  {
		case 1u:
			trData.data[0] = LSS_CS_INQUIRE_VENDOR;
			break;
		case 2u:
			trData.data[0] = LSS_CS_INQUIRE_PRODUCT;
			break;
		case 3u:
			trData.data[0] = LSS_CS_INQUIRE_REVISION;
			break;
		case 4u:
			trData.data[0] = LSS_CS_INQUIRE_SERIAL;
			break;
		default:	/* wrong subindex */
			return(RET_INVALID_PARAMETER);
	}
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}
#endif /* CO_LSS_INQUIRY_SERVICES */


/***************************************************************************/
/**
* \internal
*
* \brief lssStartCommand - start command 
*
* send command and start time out function
* If an other service is already running, returns busy
*
*
* \return none
*
*/
static RET_T lssStartCommand(
		const CO_TRANS_DATA_T	*pData,	/* pointer to transmit data */
		CO_TIMER_FCT_T	timeOutFct,		/* time out function */
		UNSIGNED16	timeOutVal			/* timeout value in msec */
	)
{
RET_T	retVal;

	if (lssCmd != 0u)  {
		return(RET_SERVICE_BUSY);
	}

	/* transmit data */
	retVal = icoTransmitMessage(lssTransCob, pData, 0u);
	if (retVal != RET_OK)  {
		return(retVal);
	}

	lssCmd = pData->data[0];

	/* start timer for answer */
	if (timeOutVal != 0u)  {
		retVal = coTimerStart(&lssTimer, timeOutVal * 1000u,
			timeOutFct, NULL, CO_TIMER_ATTR_ROUNDUP);
	}

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief lssMasterFastScanTimeout - timeout for fast scan mode
*
*
* \return none
*
*/
static void lssMasterFastScanTimeout(
		void			*pData
	)
{
(void)pData;

	lssCmd = 0u;

	/* time out are only allowed, in not complete mode */
	if ((completeAgain != 0u) && (identifyBitVal != 0u))  {
		/* complete again test - and no answer received */
#ifdef CO_EVENT_LSS
		lssEvent(CO_LSS_MASTER_SERVICE_FASTSCAN, 0xffffu, 0u);
#endif /* CO_EVENT_LSS */
	} else {
		answerFastScan(identifyBitVal);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief lssMasterTimeout - timeout for lss request
*
*
* \return none
*
*/
static void lssMasterTimeout(
		void			*pData
	)
{
typedef struct {
	UNSIGNED8	cs;
	CO_LSS_MASTER_SERVICE_T	service;
} SERV_CS_T;
static const SERV_CS_T	servCs[] = {
	{ LSS_CS_NODE_ID,			CO_LSS_MASTER_SERVICE_SET_NODEID },
	{ LSS_CS_NON_CONFIG_REMOTE_SLAVE, CO_LSS_MASTER_SERVICE_NON_CONFIG_SLAVE },
	{ LSS_CS_STORE,				CO_LSS_MASTER_SERVICE_STORE },
	{ LSS_CS_INQUIRE_NODEID,	CO_LSS_MASTER_SERVICE_INQUIRE_NODEID },
#ifdef CO_LSS_INQUIRY_SERVICES
	{ LSS_CS_INQUIRE_VENDOR,	CO_LSS_MASTER_SERVICE_INQUIRE_VENDOR },
	{ LSS_CS_INQUIRE_PRODUCT,	CO_LSS_MASTER_SERVICE_INQUIRE_PRODUCT },
	{ LSS_CS_INQUIRE_REVISION,	CO_LSS_MASTER_SERVICE_INQUIRE_REVISION },
	{ LSS_CS_INQUIRE_SERIAL,	CO_LSS_MASTER_SERVICE_INQUIRE_SERIAL },
#endif /* CO_LSS_INQUIRY_SERVICES */
	{ LSS_CS_SWITCH_SEL_ANSWER, CO_LSS_MASTER_SERVICE_SWITCH_SELECTIVE}
};
CO_LSS_MASTER_SERVICE_T	service = CO_LSS_MASTER_SERVICE_INQUIRE_NODEID;
UNSIGNED8	i;

(void)pData;

	for (i = 0u; i < (sizeof(servCs) / sizeof(SERV_CS_T)); i++)  {
		if (servCs[i].cs == lssCmd)  {
			service = servCs[i].service;
		}
	}

	/* reset command */
	lssCmd = 0u;

#ifdef CO_EVENT_LSS
	lssEvent(service, 0xffffu, 0u);
#endif /* CO_EVENT_LSS */
}


/***************************************************************************/
/**
* \internal
*
* \brief bitrateSwitchFct - switch bitrate function
*
*
* \return none
*
*/
static void bitrateSwitchFct(
		void			*pData
	)
{
(void)pData;

	if (bitrateSwitch == CO_LSS_MASTER_SERVICE_BITRATE_OFF)  {
		lssEvent(CO_LSS_MASTER_SERVICE_BITRATE_SET, 0u, 0u);
		bitrateSwitch = CO_LSS_MASTER_SERVICE_BITRATE_SET;
	} else if (bitrateSwitch == CO_LSS_MASTER_SERVICE_BITRATE_SET)  {
		lssEvent(CO_LSS_MASTER_SERVICE_BITRATE_ACTIVE, 0u, 0u);
		(void)coTimerStop(&lssBitrateTimer);
		bitrateSwitch = CO_LSS_MASTER_SERVICE_BITRATE_ACTIVE;
	}
}

/***************************************************************************/
/**
* \internal
*
* \brief nonConfiguredSlaves - call non configured slaves request cyclic
*
*
* \return none
*
*/
static void nonConfiguredSlaves(
		void			*pData
	)
{
(void)pData;

	(void)lssNonConfiguredSlaves(*(UNSIGNED16 *)pData);
}


/***************************************************************************/
/**
* \internal
*
* \brief lssMasterTimeout - timeout for lss request
*
*
* \return none
*
*/
static void answerFastScan(
		UNSIGNED8	bitVal
	)
{
FASTSCAN_DATA_T	*pFastScan;
CO_TRANS_DATA_T	trData;

	/* depending on actual lssPos */
	switch (lssSub)  {
		case 0u:	pFastScan = &vendor;
			break;
		case 1u:	pFastScan = &product;
			break;
		case 2u:	pFastScan = &version;
			break;
		case 3u:	pFastScan = &serial;
			break;
		case 4u:	/* all data are detected - user indication */
#ifdef CO_EVENT_LSS
			lssEvent(CO_LSS_MASTER_SERVICE_FASTSCAN, 0u, 0u);
#endif /* CO_EVENT_LSS */
			return;
		default:
			return;
	}

	/* set new bitval */
	/* except for first call ... */
	if (pFastScan->bitCheck < 32u)  {
		pFastScan->idNr |= ((UNSIGNED32)bitVal << pFastScan->bitCheck);
	}

	trData.data[0] = LSS_CS_FAST_SCAN;
	trData.data[1] = pFastScan->idNr & 0xffu;
	trData.data[2] = (pFastScan->idNr >> 8u) & 0xffu;
	trData.data[3] = (pFastScan->idNr >> 16u) & 0xffu;
	trData.data[4] = (pFastScan->idNr >> 24u) & 0xffu;
	trData.data[6] = lssSub;
	trData.data[7] = lssSub; 

	/* next bit */
	if (pFastScan->bitCheck > 0u)  {
		pFastScan->bitCheck --;

		completeAgain = 0u;
	} else {
		/* bitcheck=0 second call */
		completeAgain ++;

		lssSub ++;
		if (lssSub < 4u)  {
			trData.data[7] = lssSub; 
		} else {
			trData.data[7] = 0u; 
		}
	}
	trData.data[5] = pFastScan->bitCheck;

	(void)lssStartCommand(&trData, lssMasterFastScanTimeout,
		fsTimeOut);
	identifyBitVal = 1u;
}


/***************************************************************************/
/**
* \internal
*
* \brief lssNonConfiguredSlaves - non configured slaves call
*
*
* \return none
*
*/
static RET_T lssNonConfiguredSlaves(
		UNSIGNED16	timeOutVal			/* timeout value in msec */
	)
{
CO_TRANS_DATA_T	trData;
RET_T	retVal;

	memset(&trData.data[0], 0u, 8u);
	trData.data[0] = LSS_CS_NON_CONFIG_REMOTE_SLAVE;
	retVal = lssStartCommand(&trData, lssMasterTimeout, timeOutVal);

	return(retVal);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#ifdef CO_EVENT_LSS_CNT
/***************************************************************************/
/**
* \brief coEventRegister_LSS_MASTER - register LSS master event
*
* This function registers an indication function for LSS Master events.
*
* \return RET_T
*
*/

RET_T coEventRegister_LSS_MASTER(
		CO_EVENT_LSS_MASTER_T pFunction	/**< pointer to function */
    )
{
	if (lssEventTableCnt >= CO_EVENT_LSS_CNT) {
		return(RET_EVENT_NO_RESSOURCE);
	}

	/* set new indication function as first at the list */
	lssEventTable[lssEventTableCnt] = pFunction;
	lssEventTableCnt++;

	return(RET_OK);
}
#endif /* CO_EVENT_LSS_CNT */


#ifdef CO_EVENT_LSS
/***************************************************************************/
/**
* \internal
*
* \brief lssEvent - call user indication
*
*
* \return RET_T
*
*/
static void lssEvent(
		CO_LSS_MASTER_SERVICE_T service,
		UNSIGNED16	errorCode,
		UNSIGNED8	errorSpec
	)
{
# ifdef CO_EVENT_LSS_CNT
UNSIGNED16	cnt;
# endif /* CO_EVENT_LSS_CNT */
static UNSIGNED32	identity[4];

	/* save identity value */
	identity[0] = vendor.idNr;
	identity[1] = product.idNr;
	identity[2] = version.idNr;
	identity[3] = serial.idNr;

	/* inform application */
# ifdef CO_EVENT_LSS_CNT
	cnt = lssEventTableCnt;
	while (cnt > 0u)  {
		cnt--;
		/* call user indication */
		if (service == CO_LSS_MASTER_SERVICE_FASTSCAN)  {
			lssEventTable[cnt](service, errorCode, errorSpec, &identity[0]);
		} else {
			lssEventTable[cnt](service, errorCode, errorSpec, NULL);
		}
	}
# endif /* CO_EVENT_LSS_CNT */

# ifdef CO_EVENT_STATIC_LSS
	if (service == CO_LSS_MASTER_SERVICE_FASTSCAN)  {
		coEventLssInd(service, errorCode, errorSpec, &identity[0]);
	} else {
		coEventLssInd(service, errorCode, errorSpec, NULL);
	}
# endif /* CO_EVENT_STATIC_LSS */
}
#endif /* CO_EVENT_LSS */


#ifdef CO_LSS_INQUIRY_SERVICES
/***************************************************************************/
/**
*
* \brief coLssMasterGetInquireData - get requested inquire data
*
* This function returns the requested data for a inquire request
* started by coLssInquireIdentity() before.
* The data are only valid, if the indication function with the parameter
* given to coLssInquireIdentity() was indicated before without any error.
*
* \return UNSIGNED32 identity value
*
*/
UNSIGNED32 coLssMasterGetInquireData(
		void
	)
{
	return(inquireData);
}
#endif /* CO_LSS_INQUIRY_SERVICES */


/***************************************************************************/
/**
*
* \brief coLssMasterDisable - disable LSS master services
*
* \return none
*
*/
void coLssMasterDisable(
		void	/* no parameter */
	)
{
	/* stop timer */
	(void)coTimerStop(&lssNonConfigSlavesTimer);

	/* disable cobs */
	(void)icoCobDisable(lssTransCob);
	(void)icoCobDisable(lssRecCob);

	/* save state */
	/* lssMasterEnabled = CO_FALSE; */
}


/***************************************************************************/
/**
*
* \brief coLssMasterEnable - enable LSS master services
*
* (Re)enable LSS master services after the was disabled by coLssMasterDisable()
*
* \return none
*
*/
void coLssMasterEnable(
		void	/* no parameter */
	)
{
	icoLssMasterReset();
	/* lssMasterEnabled = CO_TRUE; */
}


/***************************************************************************/
/**
* \internal
*
* \brief icoLssMasterReset - lss master reset
*
* \return none
*
*/
void icoLssMasterReset(
		void	/* no parameter */
	)
{
	if (lssMasterInit == CO_TRUE)  {
		(void)icoCobSet(lssTransCob, LSS_CAN_ID_MASTER, CO_COB_RTR_NONE, 8);
		(void)icoCobSet(lssRecCob, LSS_CAN_ID_SLAVE, CO_COB_RTR_NONE, 8);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief coLssVarInit - LSS master variable init
*
* \return none
*
*/
void icoLssMasterVarInit(
		void
	)
{

	{
		lssCmd = 0u;
		lssSub = 0u;
		completeAgain = 0u;
		identifyBitVal = 1u;
		lssMasterInit = CO_FALSE;
	}

#ifdef CO_EVENT_LSS_CNT
	lssEventTableCnt = 0u;
#endif /* CO_EVENT_LSS_CNT */
}


/***************************************************************************/
/**
* \brief coLssMasterInit - init LSS functionality
*
* This function initializes the LSS functionality,
* depending on the define CO_LSS_SLAVE_SUPPORTED or CO_LSS_MASTER_SUPPORTED
* as slave or master.
*
* \return RET_T
*
*/
RET_T coLssMasterInit(
		void	/* no parameter */
	)
{
	lssTransCob = icoCobCreate(CO_COB_TYPE_TRANSMIT, CO_SERVICE_LSS_M_TRANSMIT, 0u);
	if (lssTransCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	lssRecCob = icoCobCreate(CO_COB_TYPE_RECEIVE, CO_SERVICE_LSS_M_RECEIVE, 0u);
	if (lssRecCob == 0xffffu)  {
		return(RET_NO_COB_AVAILABLE);
	}

	lssMasterInit = CO_TRUE;

	icoLssMasterReset();

	return(RET_OK);
}
#endif /* CO_LSS_MASTER_SUPPORTED */
