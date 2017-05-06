/*
* ico_sdoserver.h - contains internal defines for SDO
*
* Copyright (c) 2013-2016 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_sdoserver.h 12068 2016-01-22 09:57:19Z boe $

*-------------------------------------------------------------------
*
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_USDO_SERVER_H
#define ICO_USDO_SERVER_H 1


/* constants */
#define CO_USDO_CS_ABORT_LOCAL		0x7Fu
#define CO_USDO_CS_ABORT_REMOTE		0xFFu

#define CO_USDO_CCS_DL_EXP_LOCAL	0x01u	/* download expedited request local  */
#define CO_USDO_CCS_DL_EXP_REMOTE	0x81u	/* download expedited request remote */

#define CO_USDO_SCS_DL_EXP_LOCAL	0x21u	/* download expedited response local  */
#define CO_USDO_SCS_DL_EXP_REMOTE	0xA1u	/* download expedited response remote */

#define CO_USDO_CCS_UP_EXP_LOCAL	0x11u	/* upload expedited request local  */
#define CO_USDO_CCS_UP_EXP_REMOTE	0x91u	/* upload expedited request remote */

#define CO_USDO_SCS_UP_EXP_LOCAL	0x31u	/* upload expedited response local  */
#define CO_USDO_SCS_UP_EXP_REMOTE	0xB1u	/* upload expedited response remote */

#define CO_USDO_CCS_DL_SEG_LOCAL_INIT	0x02u	/* download segmented init request local  */
#define CO_USDO_CCS_DL_SEG_LOCAL_CONT	0x03u	/* download segmented cont request local  */
#define CO_USDO_CCS_DL_SEG_LOCAL_LAST	0x04u	/* download segmented last request local  */
#define CO_USDO_CCS_DL_SEG_REMOTE_INIT	0x82u	/* download segmented init request remote */
#define CO_USDO_CCS_DL_SEG_REMOTE_CONT	0x83u	/* download segmented cont request remote */
#define CO_USDO_CCS_DL_SEG_REMOTE_LAST	0x94u	/* download segmented last request remote */

#define CO_USDO_SCS_DL_SEG_LOCAL_INIT	0x22u	/* download segmented init request local  */
#define CO_USDO_SCS_DL_SEG_LOCAL_CONT	0x23u	/* download segmented cont request local  */
#define CO_USDO_SCS_DL_SEG_LOCAL_LAST	0x24u	/* download segmented last request local  */
#define CO_USDO_SCS_DL_SEG_REMOTE_INIT	0xA2u	/* download segmented init request remote */
#define CO_USDO_SCS_DL_SEG_REMOTE_CONT	0xA3u	/* download segmented cont request remote */
#define CO_USDO_SCS_DL_SEG_REMOTE_LAST	0xA4u	/* download segmented last request remote */


#define CO_USDO_SCS_UP_SEG_LOCAL_INIT	0x32u	/* download segmented init request local  */
#define CO_USDO_SCS_UP_SEG_LOCAL_CONT	0x33u	/* download segmented cont request local  */
#define CO_USDO_SCS_UP_SEG_LOCAL_LAST	0x34u	/* download segmented last request local  */

#define CO_USDO_CCS_UP_SEG_LOCAL_CONT	0x13u	/* upload expedited request local  */


#define USDO_MAX_SAVE_DATA	64u


/* datatypes */

typedef enum {
	CO_USDO_STATE_FREE,
	CO_USDO_STATE_UPLOAD,
	CO_USDO_STATE_DOWNLOAD,
	CO_USDO_STATE_UPLOAD_SEGMENT,
	CO_USDO_STATE_DOWNLOAD_SEGMENT,
	CO_USDO_STATE_SPLIT_INDICATION,
	CO_USDO_STATE_REMOTE_UPLOAD
} CO_USDO_STATE_T;

typedef struct co_usdo_server_con_t {
	struct co_usdo_server_con_t* pNext; /* pointer to next active connection */
	struct co_usdo_server_con_t* pPrev; /* pointer to prev active connection */
	
	CO_USDO_STATE_T	state;			/* sdo state */
	UNSIGNED8		sessionId;		/* session Id of the client */
	BOOL_T			changed;		/* object was changed */
	
	UNSIGNED16		index;			/* index */
	UNSIGNED8		subIndex;		/* sub index */
	UNSIGNED8		clientNode;		/* node id of client */
	
	CO_CONST CO_OBJECT_DESC_T *pObjDesc;		/* object description pointer */

	UNSIGNED32		objSize;		/* object size */
	UNSIGNED32		transferedSize;	/* transfered size */

	UNSIGNED16		connRef;
	UNSIGNED8		saveData[USDO_MAX_SAVE_DATA];
	UNSIGNED8		savedSize;
	UNSIGNED8		segCnt;
# ifdef CO_EVENT_SSDO_DOMAIN_WRITE
# endif /* CO_EVENT_SSDO_DOMAIN_WRITE */
# ifdef CO_SDO_NETWORKING
# endif /* CO_SDO_NETWORKING */
} CO_USDO_SERVER_CON_T;



typedef enum {
	USDO_ABORT_OK = 0x0,					/**<no error */
	USDO_ABORT_UNKNOWN =  0x10u, 			/**<Unknown error */
	USDO_ABORT_PROTOCOL = 0x11u,			/**<USDO protocol error detected */
	USDO_ABORT_TIME_OUT = 0x12u,			/**<USDO time out error */
	USDO_ABORT_COMMAND = 0x13u,				/**<Unknown command specifier */
	USDO_ABORT_COUNTER_BYTE = 0x14u,		/**<Invalid value in counter byte(segmented protocol) */
	USDO_ABORT_CRC = 0x15u,					/**<CRC error */
	USDO_ABORT_MEMORY = 0x16u,				/**<Out of memory */
	USDO_ABORT_INCOMPATIBLE_PARA = 0x17u,	/**<General parameter incompatibility reason */
	USDO_ABORT_INTERNAL_DEVICE = 0x18u,		/**<General internal incompatibility in the device */
	USDO_ABORT_ROUTING = 0x19u,				/**<USDO general routing error */
	USDO_ABORT_DESTINATION_NET = 0x1au,		/**<USDO destination network unknown */
	USDO_ABORT_DESTINATION_NODE = 0x1bu,	/**<USDO destination node unknown */
	USDO_ABORT_SOURCE_NET = 0x1cu,			/**<USDO source network unknown */
	USDO_ABORT_SOURCE_NODE = 0x1du,			/**<USDO source node unknown */
	USDO_ABORT_DLC = 0x1eu,					/**<Unexpected CAN DLC */
	USDO_ABORT_MULTIPLEXER = 0x1fu,			/**<Unexpected multiplexer */
	USDO_ABORT_OBJECT_ACCESS = 0x30u,		/**<Unsupported access to an object */
	USDO_ABORT_WRITE_ONLY = 0x31u,			/**<Attempt to read a write only object */
	USDO_ABORT_READ_ONLY = 0x32u,			/**<Attempt to write a read only object */
	USDO_ABORT_OBJECT_DICTIONARY = 0x33u,	/**<Object does not exist in the object dictionary */
	USDO_ABORT_SUBINDEX = 0x34u,			/**<Sub-index does not exist */
	USDO_ABORT_HARDWARE = 0x35u,			/**<Access failed due to a hardware error */
	USDO_ABORT_DATA_NOT_MATCH = 0x36u,		/**<Data type does not match, length of service parameter does not match */
	USDO_ABORT_DATA_HIGH = 0x37u,			/**<Data type does not match, length of service parameter too high */
	USDO_ABORT_DATA_LOW = 0x38u,			/**<Data type does not match, length of service parameter too low */
	USDO_ABORT_INVALID_PARA = 0x40u,		/**<Invalid value for parameter (download only) */
	USDO_ABORT_PARA_HIGH = 0x41u,			/**<Value of parameter written too high (download only) */
	USDO_ABORT_PARA_LOW = 0x42u,			/**<Value of parameter written too low (download only) */
	USDO_ABORT_WRONG_MAX = 0x43u,			/**<Maximum value is less than minimum value */
	USDO_ABORT_WRONG_MIN = 0x44u,			/**<Minimum value is higher than maximum value */
	USDO_ABORT_CAN_ID = 0x45u,				/**<CAN-ID and CAN frame format are of access type read-only */
	USDO_ABORT_CANNOT_MAPPED = 0x50u,		/**<Object cannot be mapped to the PDO */
	USDO_ABORT_EXCEED_PDO = 0x51u,			/**<The number and length of the objects to be mapped would exceed PDO length */
	USDO_ABORT_PARA_PDO = 0x52u,			/**<Attempt to map a non-mappable parameter to a PDO */
	USDO_ABORT_PARA_PDO_RPDO_TPDO = 0x53u,	/**<Attempt to map a parameter of PDO access type RPDO to a TPDO */
	USDO_ABORT_PARA_PDO_TPDO_RPDO = 0x54u,	/**<Attempt to map a parameter of PDO access type TPDO to a RPDO */
	USDO_ABORT_PDO_VALID = 0x55u,			/**<Attempt to write to PDO parameters, while PDO is valid */
	USDO_ABORT_POD_MAPPING_VALID = 0x56u,	/**<Attempt to write to PDO mapping parameters, while PDO mapping is valid */
	USDO_ABORT_APPLICATION = 0x60u,			/**<Data cannot be transferred or stored to the application */
	USDO_ABORT_APPLICATION_LOCAL = 0x61u,	/**<Data cannot be transferred or stored to the application because of local control */
	USDO_ABORT_APPLICATION_DEVICE = 0x62u,	/**<Data cannot be transferred or stored to the application because of the present device state */
	USDO_ABORT_GENERATION_FAILS = 0x63u,	/**<Object dictionary dynamic generation fails or no object dictionary is present */
	USDO_ABORT_NO_DATA = 0x64u,				/**<No data available */
	USDO_ABORT_SERVER_BUSY = 0x70u,			/**<USDO server busy */
} USDO_ABORT_T;


/* function prototypes */

void	icoUsdoServerVarInit(void);

void icoUsdoServerAbort(
	CO_USDO_SERVER_CON_T	**ppConn,			/* pointer to sdo */
	RET_T					errorReason,	/* error reason */
	BOOL_T					fromClient		/* called from client */
	);
			
#if 0
RET_T	icoSdoCheckUserWriteInd(const CO_SDO_SERVER_T *pSdo);
RET_T	icoSdoCheckUserReadInd(UNSIGNED8 sdoNr, UNSIGNED16 index,
			UNSIGNED8 subIndex);
RET_T	icoSdoCheckUserCheckWriteInd(UNSIGNED8 sdoNr,
			UNSIGNED16 index, UNSIGNED8 subIndex, const UNSIGNED8 *pData);
void	icoSdoDomainUserWriteInd(const CO_SDO_SERVER_T *pSdo);
void	icoSdoServerAbort(CO_SDO_SERVER_T *pSdo, RET_T	errorReason,
			BOOL_T fromClient);

# ifdef CO_SDO_BLOCK
RET_T	icoSdoServerBlockReadInit(CO_SDO_SERVER_T	*pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockRead(CO_SDO_SERVER_T	*pSdo);
RET_T	icoSdoServerBlockReadCon(CO_SDO_SERVER_T	*pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockWriteInit(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T	*pRecData);
RET_T	icoSdoServerBlockWrite(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T *pRecData);
RET_T	icoSdoServerBlockWriteEnd(CO_SDO_SERVER_T *pSdo,
			const CO_CAN_MSG_T	*pRecData);
# endif /* CO_SDO_BLOCK */

#ifdef CO_SDO_NETWORKING
RET_T	icoSdoServerNetworkReq(CO_SDO_SERVER_T *pSdo, const CO_CAN_MSG_T *pRecData);
void	icoSdoServerNetwork(CO_SDO_SERVER_T *pSdo, const CO_CAN_MSG_T *pRecData);
RET_T	icoSdoServerLocalResp(	CO_SDO_SERVER_T	*pSdo,
			const CO_TRANS_DATA_T	*pTrData);
#endif /* CO_SDO_NETWORKING */

CO_SDO_SERVER_T	*icoSdoServerPtr(UNSIGNED16 sdoNr);
#endif /* 0 */

#endif /* ICO_USDO_SERVER_H */
