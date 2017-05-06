/*
* co_lss.h - contains defines for LSS services
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_lss.h 15305 2016-09-22 15:29:15Z boe $
*
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for lss services
*
* \file co_lss.h - contains defines for lss services
*/

#ifndef CO_LSS_H
#define CO_LSS_H 1

#include <co_datatype.h>


/* datatypes */

/**
 * \brief LSS slave states
 */
typedef enum {
	CO_LSS_STATE_WAITING = 0u,			/**< LSS state waiting */
	CO_LSS_STATE_CONFIGURATION = 1u		/**< LSS state configuration */
} CO_LSS_STATE_T;

/**
 * \brief LSS slave services for indication functions 
 */
typedef enum {
	CO_LSS_SERVICE_STORE,		/**< LSS service indication store node id */
	CO_LSS_SERVICE_NEW_BITRATE,	/**< LSS service indication new bitrate */
	CO_LSS_SERVICE_BITRATE_OFF, /**< LSS service indication bitrate off */
	CO_LSS_SERVICE_BITRATE_SET, /**< LSS service indication set new bitrate*/
	CO_LSS_SERVICE_BITRATE_ACTIVE,	/**< LSS service indication bitrate active */
	CO_LSS_SERVICE_NEW_NODE_ID		/**< LSS service indication new node-id */
} CO_LSS_SERVICE_T;

/**
 * \brief LSS master services for indication functions 
 */
typedef enum {
	CO_LSS_MASTER_SERVICE_NON_CONFIG_SLAVE,	/**< LSS Master service non-config slave */
	CO_LSS_MASTER_SERVICE_SET_NODEID,	/**< LSS Master service set node id*/
	CO_LSS_MASTER_SERVICE_SET_BITRATE,	/**< LSS Master service set bitrate*/
	CO_LSS_MASTER_SERVICE_FASTSCAN,		/**< LSS Master service fastscan */
	CO_LSS_MASTER_SERVICE_STORE,		/**< LSS Master service store */
	CO_LSS_MASTER_SERVICE_INQUIRE_NODEID,/**< LSS Master service inquire node*/
	CO_LSS_MASTER_SERVICE_INQUIRE_VENDOR,/**< LSS Master service inquire vendor */
	CO_LSS_MASTER_SERVICE_INQUIRE_PRODUCT,/**< LSS Master service inquire product */
	CO_LSS_MASTER_SERVICE_INQUIRE_REVISION,/**< LSS Master service inquire revision */
	CO_LSS_MASTER_SERVICE_INQUIRE_SERIAL,/**< LSS Master service inquire serial */
	CO_LSS_MASTER_SERVICE_BITRATE_OFF,	/**< LSS Master service indication bitrate off */
	CO_LSS_MASTER_SERVICE_BITRATE_SET,	/**< LSS Master service indication set new bitrate*/
	CO_LSS_MASTER_SERVICE_BITRATE_ACTIVE,/**< LSS Master service indication bitrate active */
	CO_LSS_MASTER_SERVICE_SWITCH_SELECTIVE,/**< LSS Master service switch selektive */
	CO_LSS_MASTER_SERVICE_IDENTITY,		/**< LSS Master service indentity */
	CO_LSS_MASTER_SERVICE_SWITCH_GLOBAL    /**< LSS Master switch global */
} CO_LSS_MASTER_SERVICE_T;


/**
 * \brief function pointer to LSS indication
 * \param service - answer for service LSS_SERVICE_xxx
 * \param bitrate - new bitrate / pending node id (only for CO_LSS_SERVICE_STORE)
 * 		1000, 500, ... 10	standard bitrates
 * 		0					autobaud
 * 		0					table specific, values in pErrCode and pErrSpec)
 * \param pErrCode - pointer to error code
 * \param pErrSpec - pointer to error spec
 * 
 * \return UNSIGNED8
 * \retval 0 - success
 * \retval 1 - store not supported
 * \retval 2 - media access error
 * \retval 255 - implementation specific (value in parameter pErr)
 */
typedef void (* CO_EVENT_LSS_T)(CO_LSS_SERVICE_T service,
		UNSIGNED16 bitrate, UNSIGNED8 *pErrCode, UNSIGNED8 *pErrSpec);	 /*lint !e960 customer specific parameter names */


/**
 * \brief function pointer to LSS master indication
 * \param service - answer for service LSS_MASTER_SERVICE_xxx
 * \param errorCode	== 65535 - timeout
 * \param errorCode == 1..255 - error code
 * \param errorCode == 0 - ok
 * \param errorSpec - error spec (if errorCode == 65365)
 * \param pIdentity == NULL - no data available
 * \param pIdentity =! NULL - pIdentity valid
 * 
 * \return void
 */
typedef void (* CO_EVENT_LSS_MASTER_T)(CO_LSS_MASTER_SERVICE_T,
		UNSIGNED16 errorCode, UNSIGNED8 errorSpec, UNSIGNED32 *pIdentity);	/*lint !e960 customer specific parameter names */


EXTERN_DECL RET_T coLssInit(void);
EXTERN_DECL RET_T coLssMasterInit(void);
EXTERN_DECL RET_T coEventRegister_LSS(CO_EVENT_LSS_T pFunction); 
EXTERN_DECL RET_T coEventRegister_LSS_MASTER(CO_EVENT_LSS_MASTER_T pFunction);
EXTERN_DECL RET_T coLssIdentifyNonConfiguredSlaves(
			UNSIGNED16 timeOutVal, UNSIGNED16	interval);
EXTERN_DECL void	coLssNonConfigSlave(void);
EXTERN_DECL RET_T coLssFastScan(UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssFastScanKnownDevice(UNSIGNED32 vendorId,
			UNSIGNED32 productCode,
			UNSIGNED32 versionNr, UNSIGNED32 serNr,	UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssSetNodeId(UNSIGNED8 nodeId,
			UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssSetBitrate(UNSIGNED16 bitRate,
			UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssSetBitrateTable(UNSIGNED8 tableSelector,
			UNSIGNED8 tableIndex, UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssActivateBitrate(UNSIGNED16 switchDelay);
EXTERN_DECL RET_T coLssSwitchGlobal(CO_LSS_STATE_T mode);
EXTERN_DECL RET_T coLssSwitchSelective(UNSIGNED32 vendorId,
			UNSIGNED32 productCode,
			UNSIGNED32 versionNr, UNSIGNED32 serNr, UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssStoreConfig(UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssInquireNodeId(UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssInquireIdentity(UNSIGNED8 subIndex,
			UNSIGNED16 timeOutVal);
EXTERN_DECL RET_T coLssIdentifyRemoteSlaves(UNSIGNED32 vendor,
			UNSIGNED32 productCode,
			UNSIGNED32 revisionLow, UNSIGNED32 revisionHigh,
			UNSIGNED32 serialNumberLow, UNSIGNED32 serialNumberHigh,
			UNSIGNED16 timeOutVal);
EXTERN_DECL void	coLssMasterDisable(void);
EXTERN_DECL void	coLssMasterEnable(void);
EXTERN_DECL UNSIGNED32 coLssMasterGetInquireData(void);

#endif /* CO_LSS_H */
