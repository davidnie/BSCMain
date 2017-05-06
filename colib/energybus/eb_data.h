/*
* eb_data.h - contains defines for eb data
*
* Copyright (c) 2012-2014 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: eb_data.h 7176 2014-10-02 11:25:15Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \brief defines for common energyBus data and structures
*
* \file eb_data.h - common data and structures
*/

#ifndef EB_DATA_H
#define EB_DATA_H 1

#include <co_datatype.h>
/* #include <gen_define.h> */

/**
* Defines for Debug macros
*
* For debugging, this macros can be exchanged by printf or other function
*/
#ifndef EB_PRINT
#define EB_PRINT(f)	
#define EB_PRINT1(f, p1)	
#define EB_PRINT2(f, p1, p2)	
#define EB_PRINT3(f, p1, p2, p3)	
#define EB_PRINT4(f, p1, p2, p3, p4)	
#endif

/* len of security strings */
#define SEC_STRG_LEN	16


/**
* status word masks (index 0x6001)
*/
#define EB_STATE_DEV_STATE_MASK	(0x3fu)						/**< device state mask */
#define EB_STATE_DEV_SPEC_MASK	((UNSIGNED16)0x3fu << 6)	/**< device specific state mask */
#define EB_STATE_DATA_REQ_MASK	((UNSIGNED16)0x01u << 12)	/**< data request mask */
#define EB_STATE_MASK			((UNSIGNED16)0x07u << 13)	/**< energy bus state mask */

#define GET_EB_STATE(devNr)		 (ebData[devNr - 1].state & EB_STATE_MASK)
#define GET_DEVSPEC_STATE(devNr) (ebData[devNr - 1].state & EB_STATE_DEV_SPEC_MASK)

/**
* energyBus state defines (status word index 0x6001)
*/
#define EB_STATE_DISCONNECTED	((UNSIGNED16)0u << 13)	/**< disconnected */
#define EB_STATE_CONNECTED		((UNSIGNED16)1u << 13)	/**< connected */
#define EB_STATE_COMP_CHECK		((UNSIGNED16)2u << 13)	/**< compatibility check */
#define EB_STATE_LIMITING		((UNSIGNED16)3u << 13)	/**< limiting */
#define EB_STATE_OPERATING		((UNSIGNED16)4u << 13)	/**< operating */
#define EB_STATE_MASTERLESS		((UNSIGNED16)5u << 13)	/**< masterless */
#define EB_STATE_SLEEP			((UNSIGNED16)7u << 13)	/**< sleep */

/**
* control word masks (index 0x6002)
*/
#define EB_CTRL_FSA_MASK		(0x00ffu)	/**< energyBus control mask */
#define EB_CTRL_DEV_SPEC_MASK	(0xff00u)	/**< device specific mask */

#define EB_STATE_TRANSITION4_OPERATING	4u	/**< eb state transition to operating */
#define EB_STATE_TRANSITION_LIMITING	5u	/**< eb state transition to limiting */
#define EB_STATE_TRANSITION6_OPERATING	6u	/**< eb state transition to operating */
#define EB_STATE_TRANSITION_CONNECTED	7u	/**< eb state transition to connected */
#define EB_STATE_TRANSITION_DISCONNECT	9u	/**< eb state transition to disconnected */
#define EB_STATE_TRANSITION_MASTERLESS	0x0Au /**< eb state transition to disconnected */
#define EB_STATE_TRANSITION_COMP_CHECK	0x0Bu /**< eb state transition to comp check */
#define EB_STATE_TRANSITION_SLEEP_KEEP	0x7Du /**< eb state transition to sleep with keeping local layer settings */
#define EB_STATE_TRANSITION_SLEEP_NKEEP	0x7Eu /**< eb state transition to sleep and set nodeId to 255 */

/**
* energyBus device types 
*/
#define EB_DEV_TYPE_EBC				0x4u
#define EB_DEV_TYPE_VOLT_CONV		0x5u
#define EB_DEV_TYPE_BATTERY			0x6u
#define EB_DEV_TYPE_MCU				0x7u
#define EB_DEV_TYPE_LOAD_MONITORING	0x8u
#define EB_DEV_TYPE_HMI				0x9u
#define EB_DEV_TYPE_SECURITY		0xau

#define EB_DEV_TYPE_MASK	0x000000fful	/* 0x6000 device type mask */
#define EB_DEV_LOCAL_MASK	0x0000ff00ul	/* 0x6000 device type local instance */
#define EB_DEV_OFFSET_MASK	0x00ff0000ul	/* 0x6000 device type instance offset */
#define EB_DEV_CLASS		0xff000000ul	/* 0x6000 device class */

#define EB_INSTANCE_OFFS_SHIFT	16
#define EB_INSTANCE_OFFS_MASK	(0xffUL << EB_INSTANCE_OFFS_SHIFT)
#define EB_INSTANCE_NR_SHIFT	8
#define EB_INSTANCE_NR_MASK		(0xffUL << EB_INSTANCE_NR_SHIFT)

#define EB_DEVICE_TYPE(vdInfo)		(vdInfo & 0xffu)
#define EB_INSTANCE_OFFSET(vdInfo)	((vdInfo >> EB_INSTANCE_OFFS_SHIFT) & 0xffu)
#define EB_INSTANCE_NUMBER(vdInfo)	((vdInfo >> EB_INSTANCE_NR_SHIFT) & 0xffu)
#define EB_GLOBAL_INSTANCE_NUMBER(vdInfo)	(EB_INSTANCE_OFFSET(vdInfo) + EB_INSTANCE_NUMBER(vdInfo))
#define EB_DEVICE_CLASS(vdInfo)		(vdInfo >> 24)

/* datatypes */

/**
* Defines for RET_T
*/
typedef enum {
	EB_RET_OK,						/**< all ok */
	EB_RET_ERROR,					/**< unspecified error */
	EB_RET_BUSY,					/**< functionality busy */
	EB_RET_UNWRITABLE_GIN,			/**< unwritable global instance number */
	EB_RET_STATE_CHANGE_BUSY		/**< state change already active */
} EB_RET_T;

#define EB_ACTIVE_DEVICE 1

/**
* Defines for EB_DATA_T
*
* accessible by ebData[];
*/
typedef struct {
	UNSIGNED32	virtualDevice;				/**< object 0x6000 */
	UNSIGNED16	control;					/**< object 0x6001 */
	UNSIGNED16	state;						/**< object 0x6002 */
	UNSIGNED16	capability;					/**< object 0x6003 */
#ifdef EB_ACTIVE_DEVICE
	INTEGER32	nominalVoltage;				/**< object 0x6008 */
#endif
	UNSIGNED16	reqDynVoltageLimitation;	/**< object 0x6020 */
	UNSIGNED16	reqDynCurrInpLimitation;	/**< object 0x6022 */
	UNSIGNED16	reqDynCurrOutLimitation;	/**< object 0x6023 */

#ifdef EB_ACTIVE_DEVICE
	INTEGER32	maxContinousInpCurrent;		/**< object 0x6024 */
	INTEGER32	maxContinousOutCurrent;		/**< object 0x6025 */
	INTEGER32	maxVoltage;					/**< object 0x6026 */
	INTEGER32	minVoltage;					/**< object 0x6027 */
#endif

	INTEGER32	actualCurrent;				/**< object 0x603e */
	INTEGER32	actualVoltage;				/**< object 0x6040 */

	INTEGER16	electronicTemperature;		/**< object 0x6042 */

#ifdef EB_ACTIVE_DEVICE
	INTEGER32	setMaxVoltage;				/**< object 0x6046 */
	INTEGER32	setMinVoltage;				/**< object 0x6047 */
	INTEGER32	setMaxContinousInpCurrent;	/**< object 0x604A */
	INTEGER32	setMaxContinousOutCurrent;	/**< object 0x604B */
#endif

#if defined(EB_SECURITY_CNT) || defined(EB_NETWORK_SECURITY_CNT)
	UNSIGNED32	secProperties;				/**< object 0x6310 */
	UNSIGNED16	secState;					/**< object 0x6311 */
	UNSIGNED8	secAlgorithm;				/**< object 0x6312 */
	OCTET_STRING  secDevStrg[SEC_STRG_LEN];	/**< object 0x6315 */
	OCTET_STRING  secEbcStrg[SEC_STRG_LEN];	/**< object 0x6316 */

	OCTET_STRING secCommandEnc[SEC_STRG_LEN];/**< object 0x6320 */
	OCTET_STRING secStateEnc[SEC_STRG_LEN];	/**< object 0x6321 */

	UNSIGNED16	secDeviceCommand;			/**< internal dev cmd */
	BOOL_T		secKeysReady;				/**< internal - Keys valid */
	OCTET_STRING secRandomDevice[SEC_STRG_LEN];	/**< random device string */
	OCTET_STRING secRandomEbc[SEC_STRG_LEN];/**< random ebc string */

	OCTET_STRING *keyDevice;				/**< pointer to device key */
	OCTET_STRING *keyEBC;					/**< pointer to ebc key */

#endif /* EB_SECURITY_CNT */

} EB_DATA_T;


//ro: pragmas sehr Compilerspezifisch
#pragma pack(1u)
#define EB_DATA_REQUEST_MAX_LEN	(52 - 7)
typedef union  {
	struct rd_single {
		UNSIGNED8	cmd;		/**< command code */
		UNSIGNED8	gin_nodeid;	/**< GIN or node id */
		UNSIGNED8	devType;	/**< device type */
		UNSIGNED8	indexL;		/**< index low */
		UNSIGNED8	indexH;		/**< index high */
		UNSIGNED8	subIndex;	/**< subindex */
		UNSIGNED8	dataLen;	/**< data len */
		UNSIGNED8	data[EB_DATA_REQUEST_MAX_LEN];	/**< data */
	} s;
	struct mEbRead {
		UNSIGNED8	cmd;		/**< command code */
		UNSIGNED8	gin_nodeid;	/**< GIN or node id */
		UNSIGNED8	devType;	/**< device type */
		UNSIGNED8	nrOfIdx;	/**< number of index */
		UNSIGNED8	index1L;	/**< index 1 low */
		UNSIGNED8	index1H;	/**< index 1 high */
		UNSIGNED8	index2L;	/**< index 2 low */
		UNSIGNED8	index2H;	/**< index 2 high */
		UNSIGNED8	index3L;	/**< index 3 low */
		UNSIGNED8	index3H;	/**< index 3 high */
		UNSIGNED8	index4L;	/**< index 4 low */
		UNSIGNED8	index4H;	/**< index 4 high */
	} mEbRead;
	struct mEbWrite {
		UNSIGNED8	cmd;		/**< command code */
		UNSIGNED8	gin_nodeid;	/**< GIN or node id */
		UNSIGNED8	devType;	/**< device type */
		UNSIGNED8	nrOfIdx;	/**< number of index */
		UNSIGNED8	index1L;	/**< index 1 low */
		UNSIGNED8	index1H;	/**< index 1 high */
		UNSIGNED8	dataLen1;	/**< data len 1 */
		UNSIGNED8	data1[4];	/**< data 1 */
		UNSIGNED8	index2L;	/**< index 2 low */
		UNSIGNED8	index2H;	/**< index 2 high */
		UNSIGNED8	dataLen2;	/**< data len 2 */
		UNSIGNED8	data2[4];	/**< data 2 */
		UNSIGNED8	index3L;	/**< index 3 low */
		UNSIGNED8	index3H;	/**< index 3 high */
		UNSIGNED8	dataLen3;	/**< data len 3 */
		UNSIGNED8	data3[4];	/**< data 3 */
		UNSIGNED8	index4L;	/**< index 4 low */
		UNSIGNED8	index4H;	/**< index 4 high */
		UNSIGNED8	dataLen4;	/**< data len 4 */
		UNSIGNED8	data4[4];	/**< data 4 */
	} mEbWrite;
	struct mRead {
		UNSIGNED8	cmd;		/**< command code */
		UNSIGNED8	gin_nodeid;	/**< GIN or node id */
		UNSIGNED8	devType;	/**< device type */
		UNSIGNED8	nrOfIdx;	/**< number of index */
		UNSIGNED8	index1L;	/**< index 1 low */
		UNSIGNED8	index1H;	/**< index 1 high */
		UNSIGNED8	subIndex1;	/**< subIndex 1 */
		UNSIGNED8	index2L;	/**< index 2 low */
		UNSIGNED8	index2H;	/**< index 2 high */
		UNSIGNED8	subIndex2;	/**< subIndex 2 */
		UNSIGNED8	index3L;	/**< index 3 low */
		UNSIGNED8	index3H;	/**< index 3 high */
		UNSIGNED8	subIndex3;	/**< subIndex 3 */
		UNSIGNED8	index4L;	/**< index 4 low */
		UNSIGNED8	index4H;	/**< index 4 high */
		UNSIGNED8	subIndex4;	/**< subIndex 4 */
	} mRead;
	struct mWrite {
		UNSIGNED8	cmd;		/**< command code */
		UNSIGNED8	gin_nodeid;	/**< GIN or node id */
		UNSIGNED8	devType;	/**< device type */
		UNSIGNED8	nrOfIdx;	/**< number of index */
		UNSIGNED8	index1L;	/**< index 1 low */
		UNSIGNED8	index1H;	/**< index 1 high */
		UNSIGNED8	subIndex1;	/**< subIndex 1 */
		UNSIGNED8	dataLen1;	/**< data len 1 */
		UNSIGNED8	data1[4];	/**< data 1 */
		UNSIGNED8	index2L;	/**< index 2 low */
		UNSIGNED8	index2H;	/**< index 2 high */
		UNSIGNED8	subIndex2;	/**< subIndex 2 */
		UNSIGNED8	dataLen2;	/**< data len 2 */
		UNSIGNED8	data2[4];	/**< data 2 */
		UNSIGNED8	index3L;	/**< index 3 low */
		UNSIGNED8	index3H;	/**< index 3 high */
		UNSIGNED8	subIndex3;	/**< subIndex 3 */
		UNSIGNED8	dataLen3;	/**< data len 3 */
		UNSIGNED8	data3[4];	/**< data 3 */
		UNSIGNED8	index4L;	/**< index 4 low */
		UNSIGNED8	index4H;	/**< index 4 high */
		UNSIGNED8	subIndex4;	/**< subIndex 4 */
		UNSIGNED8	dataLen4;	/**< data len 4 */
		UNSIGNED8	data4[4];	/**< data 4 */
	} mWrite;
} DATA_REQUEST_T;


typedef union resp_ms {
	struct resp_single {
		UNSIGNED8	cmd;
		UNSIGNED32	errCode;
		UNSIGNED8	gin_nodeid;
		UNSIGNED8	devType;
		UNSIGNED8	indexL;
		UNSIGNED8	indexH;
		UNSIGNED8	subIndex;
		UNSIGNED8	dataLen;
		UNSIGNED8	data[4];
	} s;
	struct resp_mEbRead {
		UNSIGNED8	cmd;
		UNSIGNED8	gin;
		UNSIGNED8	nrOfIdx;	/**< number of index */
	} mEbRead;
	struct resp_mEbWrite {
		UNSIGNED8	cmd;
		UNSIGNED8	gin;
		UNSIGNED8	nrOfIdx;	/**< number of index */
	} mEbWrite;
	struct resp_mRead {
		UNSIGNED8	cmd;
		UNSIGNED8	dummy;
		UNSIGNED8	nrOfIdx;	/**< number of index */
	} mRead;
	struct resp_mWrite {
		UNSIGNED8	cmd;
		UNSIGNED8	dummy;
		UNSIGNED8	nrOfIdx;	/**< number of index */
	} mWrite;
} DATA_RESPONSE_T;
#pragma pack()


/* PDO mapping entry for MSNs */
typedef struct {
	UNSIGNED16	index;			/* mapping index */
	UNSIGNED8	subIndex;		/* mapping subIndex */
	UNSIGNED8	len;			/* len of object in bits */
} EB_PDO_MAP_ENTRY_T;

#define DATA_REQCMD_EB_SINGLE_READ	0x01	/* EB single read */
#define DATA_REQCMD_EB_SINGLE_WRITE	0x02	/* EB single write */
#define DATA_REQCMD_EB_MULTI_READ	0x03	/* EB multiple read */
#define DATA_REQCMD_EB_MULTI_WRITE	0x04	/* EB multiple write */
#define DATA_REQCMD_SINGLE_READ		0x11	/* EB single read */
#define DATA_REQCMD_SINGLE_WRITE	0x12	/* EB single write */
#define DATA_REQCMD_MULTI_READ		0x13	/* EB multiple read */
#define DATA_REQCMD_MULTI_WRITE		0x14	/* EB multiple write */
#define DATA_REQCMD_NODEID			0x21	/* request node id */

#define DATA_TRANSFER_INDEX		0x5ffe

#define FIXED_NODE_ID			120u		/* first fixed node id */
#define LOCKSLAVE_NODE_ID		124u		/* charge & lock cable connector */
#define PCTOOL_NODE_ID			125u		/* PC Tool */
#define BOOTLOADER_NODE_ID		126u		/* Bootloader Node Id */
#define CHARGER_EBC_NODE_ID		127u		/* EBC of the charger */

/* external data */
#ifdef EB_EBC_CNT
extern EB_DATA_T		ebData[EB_NETWORK_VIRTUAL_DEV_CNT];
#else
extern EB_DATA_T		ebData[EB_VIRTUAL_DEV_CNT];
#endif

#define DATA_REQ_OBJ_LEN	12
#define DATA_RESP_OBJ_LEN	16
extern OCTET_STRING	dataReqEbc[DATA_REQ_OBJ_LEN];
extern OCTET_STRING	dataRespEbc[DATA_RESP_OBJ_LEN];


/* function prototypes */

#endif /* EB_DATA_H */

