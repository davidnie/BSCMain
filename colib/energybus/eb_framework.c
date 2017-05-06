/*
* eb_framework.c - energybus framework main part
*
* Copyright (c) 2012 emtas GmbH
*-------------------------------------------------------------------
* $Id: eb_framework.c 8202 2015-01-16 12:04:22Z boe $
*
*
*-------------------------------------------------------------------
*
*
*/

/********************************************************************/
/**
* \brief energyBus Framework main routines
*
* \file eb_framework.c
* contains main routines for EnergyBus Framework
*
*/


/********************************************************************/
/**
* \mainpage EnergyBus Framework Reference Manual
*
* \section i Introduction
* The EnergyBus Framework of emtas is a software library
* that provides communication services and handling of EneryBus behavior
* according to standard CiA 454 ofCiA e.V. and EnergyBus e.V.
* It based on the CANopen stack of emtas,
* so all services of CANopen can also be used.
*
* The main features are:
*	- well-defined interface between application and energyBus Framework
*	- ANSI-C conform
*	- provides all relevant EnergyBus variables
*	- hides communication services for application
*	- supports all kind of energyBus devices
*	- optional use of further CANopen services are possible
*
* This reference manual describes the functions 
* for the API for providing energyBus relevant data
* and the services for events and handling functions.
*
* Configuration and features settings are supported
* by the graphical configuration tool EnergyBus Wizard.
*
* \section g General
* The EnergyBus Framework use strict data hiding,
* so access to internal data are only possible by functions.
* The same is valid for access to the communication data of the 
* object dictionary.
* EnergyBus data are provided as global data,
* so the application has always direct access.
*
* \section u Using EnergyBus Frameworl in an application
* At startup, whole initialization of the framework 
* is done by the functions ebFrameworkInit().
*
* For the main functionality,
* the central function
* coCommTask()
* has to be called in case of
*       - new CAN message was received
*       - timer period has been ellapsed.
*
* Therefore signal handlers should be used
* or a cyclic call of the function coCommTask() is necessary.
* For operating systems (like LINUX) the function
* codrvWaitForEvent()
* can be used to wait for events.
* <br>All EnergyBus functionality is handled inside this function.
*
* \section c Indication functions
* Indication functions inform application about EnergyBus and CANopen service events.
* <br>To receive an indication,
* the application has to register a function
* by the apropriate service register function like ebRegister_xxx().
* <br>Every time the event occures, 
* the registered indication function is called.
*
*
*/

/* header of standard C - libraries
---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

/* header of project specific types
---------------------------------------------------------------------------*/
#include <gen_define.h>
#include <co_canopen.h>
#include <ico_cobhandler.h>

#include <eb_energybus.h>
#include <ieb_def.h>

#ifdef EB_EBC_CNT
# ifdef EB_VOLT_CONV_CNT
#  include <ieb_ebc.h>
# endif /* EB_VOLT_CONV_CNT */
#endif /* EB_EBC_CNT */

/* constant definitions
---------------------------------------------------------------------------*/
typedef struct {
	UNSIGNED32	devType;
	UNSIGNED8	msgNr;
	UNSIGNED8	maxDev;
	UNSIGNED16	pdoOffs;
	UNSIGNED32	cobId;
} PDO_ASSIGN_T;

const PDO_ASSIGN_T	pdoAssign[] = {
	{ EB_DEV_TYPE_EBC,		1u,	1u,	0x00u,	0x181u },
	{ EB_DEV_TYPE_EBC,		2u,	1u,	0x7fu,	0x281u },
	{ EB_DEV_TYPE_VOLT_CONV,1u,	8u,	0x01u,	0x182u },
	{ EB_DEV_TYPE_VOLT_CONV,2u,	8u,	0x80u,	0x282u },
//	{ EB_DEV_TYPE_VOLT_CONV,7u,	8u,	0xffu,	0x382u },
//	{ EB_DEV_TYPE_VOLT_CONV,8u,	8u,	0x17eu,	0x482u },
	{ EB_DEV_TYPE_BATTERY,	1u,	16u,	0x11u,	0x192u },
	{ EB_DEV_TYPE_BATTERY,	2u,	16u,	0x90u,	0x292u },
	{ EB_DEV_TYPE_BATTERY,	3u,	16u,	0x10fu,	0x392u },
	{ EB_DEV_TYPE_MCU,		1u,	4u,	0x09u,	0x18au },
	{ EB_DEV_TYPE_MCU,		2u,	4u,	0x88u,	0x28au },
	{ EB_DEV_TYPE_MCU,		4u,	4u,	0x107u,	0x38au },
	{ EB_DEV_TYPE_HMI,		1u,	8u,	0x3fu,	0x1c0u },
	{ EB_DEV_TYPE_SECURITY,	10u,	3u,	0x47u,	0x1c8u },
	{ EB_DEV_TYPE_LOAD_MONITORING,
							1u,	4u,	0x0du,	0x18eu },
	{ EB_DEV_TYPE_LOAD_MONITORING,
							2u,	4u,	0x8cu,	0x28eu },
};

typedef struct {
	EB_REQ_NODEID_FCT_T 	pReqNodeidFct;
	EB_REQ_DATA_FCT_T		pReqDataFct;
	EB_REQ_DATA_WRITE_FCT_T	pReqDataWriteFct;
	UNSIGNED8				devIdx;
} EB_REQ_DATA_T;

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/
RET_T coCanOpenStackInit(CO_EVENT_STORE_T pLoadFunction);


/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void responseTransferNodeId(EB_REQ_NODEID_FCT_T	pFct);
static void responseTransferData(EB_REQ_DATA_FCT_T pFct);
static void responseTransferWriteData(EB_REQ_DATA_WRITE_FCT_T pFct);
static RET_T setupRecPdoMSN(UNSIGNED16 pdoOffs, EB_PDO_MAP_ENTRY_T *pMapTable[],
		UNSIGNED8	mapCnt, UNSIGNED32	cobId);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/
#ifdef EB_EBC_CNT
EB_DATA_T		ebData[EB_NETWORK_VIRTUAL_DEV_CNT];
#else
EB_DATA_T		ebData[EB_VIRTUAL_DEV_CNT];
#endif
UNSIGNED32		emtasVendorId = 0x0319ul;
UNSIGNED32		emtasCANopenVersion = CO_STACK_VERSION;
UNSIGNED32		emtasEbFwVersion = EB_FW_VERSION;
OCTET_STRING	dataReqEbc[DATA_REQ_OBJ_LEN];
OCTET_STRING	dataRespEbc[DATA_RESP_OBJ_LEN];

/* local defined variables
---------------------------------------------------------------------------*/
EB_REQ_DATA_T	reqData;


/***************************************************************************/
/**
* \brief ebFrameworkInit - init energybus Framework
*
* This function initializes the whole framework,
* included the CANopen stack, the CAN driver and the timer initialization.
* It has to be called before any energyBus functionality is used.
*
* The parameter pLoadFunction can give a pointer to a function
* to initialize objects at reset communication.
*
* \return EB_RET
*
*/
EB_RET_T ebFrameworkInit(
		UNSIGNED16	bitRate,			/**< CAN bitRate - typical 250 kBit */
		UNSIGNED32	timerInterval,		/**< timer interval */
		CO_EVENT_STORE_T pLoadFunction	/**< load function called at reset comm */
	)
{
RET_T	retVal;
UNSIGNED8	devIdx;

	/* init CAN driver */
	if (codrvCanInit(bitRate) != RET_OK)  {
		return(EB_RET_ERROR);
	}

	/* save load indication internally */
	iebLoadIndInit(pLoadFunction);

	/* init canopen stack */
    retVal = coCanOpenStackInit(iebLoadInd);
	if (retVal != RET_OK)  {
		return(EB_RET_ERROR);
	}

	/* register internal indication functions */
	if (iebIndInit() != EB_RET_OK)  {
		return(EB_RET_ERROR);
	}

#ifdef EB_NETWORK_SECURITY_CNT
	/* init security functionality */
	retVal = iebSecInit();
	if (retVal != RET_OK)  {
		return(EB_RET_ERROR);
	}
#endif

	/* init timer */
	codrvTimerSetup(timerInterval);	

	/* enable can */
#ifdef EB_CUSTOMER_CAN_ENABLE
	/* customer application has to enable CAN */
#else /* EB_CUSTOMER_CAN_ENABLE */
	codrvCanEnable();
#endif /* EB_CUSTOMER_CAN_ENABLE */

#ifdef CO_LSS_SUPPORTED
	/* send an unconfigured slave command to LSS master */
//	coLssNonConfigSlave();
#endif /* CO_LSS_SUPPORTED */

	/* set statemachine for all internal devices to disconnected */
	for (devIdx = 0u; devIdx < EB_VIRTUAL_DEV_CNT; devIdx++)  {
		/* set unknown state to force change to disconnected */
		ebData[devIdx].state = EB_STATE_MASK;
		ebStateChange(EB_STATE_DISCONNECTED, devIdx);

		/* set default state for device specific state machine */
#ifdef EB_BATTERY_CNT
		if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_BATTERY)  {
			ebData[devIdx].state |= EB_BAT_STATE_DO_NOT_ATTACH;
		}
#endif /* EB_BATTERY_CNT */
#ifdef EB_VOLT_CONV_CNT
		if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == EB_DEV_TYPE_VOLT_CONV)  {
			ebData[devIdx].state |= EB_VC_STATE_IDLE;
		}
#endif /* EB_VOLT_CONV_CNT */
	}

	/* call canopen main to switch the state machine */
	coCommTask();
	coCommTask();

	return(EB_RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebUpdateStandardTPdo - update standard PDOs
*
* Disable all PDOs, because we don't know which is actual used
* and may be moved to other GIN (=PDO)
*
* return RET_T
*/
RET_T  iebUpdateStandardTPdos(
		void
	)
{
#define MAX_MSG_CNT	10u
UNSIGNED8	devIdx;		/* device number - 1 */
UNSIGNED8	i, j;
RET_T	retVal = RET_OK;

	/* disable all PDOs */
	for (devIdx = 0u; devIdx < EB_VIRTUAL_DEV_CNT; devIdx++)  {

		/* search device type in pdoAssign table */
		for (i = 0u; i < (sizeof(pdoAssign) / sizeof(PDO_ASSIGN_T)); i++)  {

			/* find device type */
			if (EB_DEVICE_TYPE(ebData[devIdx].virtualDevice) == pdoAssign[i].devType)  {
				/* for all max devices */
				for (j = 0u; j < pdoAssign[i].maxDev; j++)  {
					/* disable PDO */
					coOdSetCobid(0x1800u + pdoAssign[i].pdoOffs + j, 1u, 0xc0000000ul);
				}
			}
		}
	}

	/* set actual used PDOs */
	for (devIdx = 0u; devIdx < EB_VIRTUAL_DEV_CNT; devIdx++)  {
		/* enable PDOs for this device */
		retVal = iebUpdateDeviceTPdos(devIdx);
	}

#ifdef EB_EBC_CNT
# ifdef EB_VOLT_CONV_CNT
	iebcUpdateEbcPdos();
# endif /* EB_VOLT_CONV_CNT */
#endif /* EB_EBC_CNT */

	return(retVal);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebUpdatedeviceTPdo - update PDOs for one device
*
* return RET_T
*/
RET_T iebUpdateDeviceTPdos(
		UNSIGNED8	devIdx
	)
{
UNSIGNED16	pdoOffs;
UNSIGNED32	cobId;
RET_T	retVal = RET_OK;
UNSIGNED8	i;

	for (i = 1u; i <= MAX_MSG_CNT; i++)  {
		pdoOffs = iebGetPdoMessageOffs(ebData[devIdx].virtualDevice, i,
				&cobId, CO_FALSE);
		if (pdoOffs != 0xffffu)  {
			/* At the moment, we don't use the correct pdoOffs now,
			* instead this the internal device nr */
//			pdoOffs -= (EB_GLOBAL_INSTANCE_NUMBER(ebData[devIdx].virtualDevice) - 1);
			pdoOffs += (EB_INSTANCE_NUMBER(ebData[devIdx].virtualDevice) - 1u);
			retVal = coOdSetCobid(0x1800u + pdoOffs, 1u, 0x80000000ul);
			retVal = coOdSetCobid(0x1800u + pdoOffs, 1u, 0x40000000ul | cobId);
			if (retVal != RET_OK)  {
				return(retVal);
			}
		}
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebGetPdoMessageOffs - get PDO offset
*
* This function returns the PDO offset for the given device type
* and message number
*
* If no message type is available return 0xffff
*
* \return
*	pdo offset
*/
UNSIGNED16 iebGetPdoMessageOffs(
		UNSIGNED32	virtualDevice,	/* virtual device data */
		UNSIGNED8	msgNr,			/* message number 1..n */
		UNSIGNED32	*pCobId,		/* pointer to cob-id */
		BOOL_T		mode			/* return with gin offset */
	)
{
UNSIGNED8	i;
UNSIGNED8	gin;

	/* search device type in pdoAssign table */
	for (i = 0; i < (sizeof(pdoAssign) / sizeof(PDO_ASSIGN_T)); i++)  {
		if (EB_DEVICE_TYPE(virtualDevice) == pdoAssign[i].devType)  {
			/* look for correct message number */
			if (msgNr == pdoAssign[i].msgNr)  {
				/* found, add global instance number */
				gin = EB_INSTANCE_NUMBER(virtualDevice);
				if ((gin < 1u) || (gin > 0x7Fu)) {
					return(0xffffu);
				}
				gin--;

				if (EB_INSTANCE_OFFSET(virtualDevice) > 0x7fu)  {
					return(0xffffu);
				}
				gin += EB_INSTANCE_OFFSET(virtualDevice);

				/* check for max device count */
				if (gin >= pdoAssign[i].maxDev)  {
					return(0xffffu);
				}

				*pCobId = pdoAssign[i].cobId + gin;

				/* At the moment, we don't use the correct pdoOffs now,
				* instead this the internal device nr */
				//return(pdoAssign[i].pdoOffs + gin);
				if (mode == CO_TRUE)  {
					return(pdoAssign[i].pdoOffs + gin);
				} else {
					return(pdoAssign[i].pdoOffs);
				}
			}
		}
	}
	return(0xffffu);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebGetDevIdxFromSubIndex - get device index from given subindex
*
* if not found, return 255
*
* \return
*/
UNSIGNED8	iebGetDevIdxFromSubIndex(
		UNSIGNED8 subIndex
	)
{
//UNSIGNED8	devIdx;

	/* only valid for local devices */
	if (subIndex < 0x80)  {
//		for (devIdx = 0; devIdx < EB_VIRTUAL_DEV_CNT; devIdx++)  {
//			if (ebData[devIdx].subIndex == subIndex)  {
//				return(devIdx);
//			}
//		}
		return(subIndex - 1);
	}

	return(255);
}


/***************************************************************************/
/**
*
* \brief ebGetEbDataPtr - return pointer to ebData
*
* This function returns a pointer to ebdata structure
* for the requested device type.
*
*
* \return
*	pointer to ebData
*/
EB_DATA_T *ebGetEbDataPtr(
		UNSIGNED16 devType,			/**< device type (EB_DEV_TYPE_BATTERY) */
		UNSIGNED8 devIdx			/**< device index (starts with 0 */
	)
{
#ifdef EB_EBC_CNT
EBC_DEVICE_T *pDev;

	pDev = ebGetDevicePtr(devType, devIdx + 1);
	if (pDev != NULL)  {
		return(pDev->pEbData);
	}
#else /* EB_EBC_CNT */
UNSIGNED8	i;

	for (i = 0; i < EB_VIRTUAL_DEV_CNT; i++)  {
		if ((EB_DEVICE_TYPE(ebData[i].virtualDevice) == devType)
		 && (EB_INSTANCE_NUMBER(ebData[i].virtualDevice) == (devIdx + 1)))  {
			return(&ebData[i]);
		}
	}
#endif /* EB_EBC_CNT */

	return(NULL);

}


/***************************************************************************/
/**
* \internal
*
* \brief ebRequestData
*
* \return
*/
static void ebRequestData(
		UNSIGNED8	devIdx,
		UNSIGNED8	ein
	)
{
	if (ein != 0)  {
		/* set request bit */
		ebData[devIdx].state |= EB_STATE_DATA_REQ_MASK;
	} else {
		/* reset request bit */
		ebData[devIdx].state &= ~EB_STATE_DATA_REQ_MASK;
	}

	/* write PDO */
	coPdoReqObj(0x6002u, devIdx + 1u, 0u);
}


/***************************************************************************/
/**
*
* \brief ebRequestEbDataRead - request EB data
*
* This function requests EnergyBus data from another node
* over the EBC.
* As parameter device type and GIN is used.
* If the function is finished, 
* the given indication function is called.
*
* Note: Only one data request is possible at one time.
*
* \return
*/
EB_RET_T ebRequestEbDataRead(
		UNSIGNED8	devIdx,				/**< local device index */
		UNSIGNED8	deviceType,			/**< device type */
		UNSIGNED8	gin,				/**< global instance number */
		UNSIGNED16	index,				/**< index at device */
		EB_REQ_DATA_FCT_T pFct			/**< function after result is available */
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];

	if (pReq->s.cmd != 0)  {
		return(EB_RET_BUSY);
	}
	pReq->s.gin_nodeid = gin;
	pReq->s.devType = deviceType;
	pReq->s.indexL = index & 0xff;
	pReq->s.indexH = index >> 8;
    pReq->s.subIndex = 255;

	pReq->s.cmd = DATA_REQCMD_EB_SINGLE_READ;

	reqData.pReqDataFct = pFct;
	reqData.devIdx = devIdx;

	ebRequestData(devIdx, 1);

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebRequestEbDataWrite - write EB data
*
* This function write EnergyBus data at another node
* over the EBC.
* As parameter device type and GIN is used.
* If the function is finished, 
* the given indication function is called.
*
* Note: Only one data request is possible at one time.
*
* \return
*/
EB_RET_T ebRequestEbDataWrite(
		UNSIGNED8	devIdx,				/**< local device index */
		UNSIGNED8	deviceType,			/**< device type */
		UNSIGNED8	gin,				/**< global instance number */
		UNSIGNED16	index,				/**< index at device */
		UNSIGNED8	*pData,				/**< data to write */
		UNSIGNED8	dataLen,			/**< len of data */
		EB_REQ_DATA_WRITE_FCT_T pFct	/**< function after result is available */
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];

	if (pReq->s.cmd != 0)  {
		return(EB_RET_BUSY);
	}
	/* check max data len */
	if (dataLen > EB_DATA_REQUEST_MAX_LEN)  {
		return(EB_RET_ERROR);
	}
	pReq->s.gin_nodeid = gin;
	pReq->s.devType = deviceType;
	pReq->s.indexL = index & 0xff;
	pReq->s.indexH = index >> 8;
    pReq->s.subIndex = 255;
    pReq->s.dataLen = dataLen;
    memcpy(&pReq->s.data[0], pData, dataLen);

	pReq->s.cmd = DATA_REQCMD_EB_SINGLE_WRITE;

	reqData.pReqDataWriteFct = pFct;
	reqData.devIdx = devIdx;

	ebRequestData(devIdx, 1);

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebRequestObjectRead
*
* This function requests an read of an object from another node
* over the EBC.
* If the function is finished, 
* the given indication function is called.
*
* Note: Only one request is possible at one time.
*
* \return
*/
EB_RET_T ebRequestObjectRead(
		UNSIGNED8	devIdx,				/**< local device index */
		UNSIGNED8	nodeId,				/**< node id */
		UNSIGNED16	index,				/**< index at device */
		UNSIGNED8	subIndex,			/**< subindex at device */
		EB_REQ_DATA_FCT_T pFct			/**< function after result is available */
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];

	if (pReq->s.cmd != 0)  {
		return(EB_RET_BUSY);
	}
	pReq->s.gin_nodeid = nodeId;
	pReq->s.devType = 0;
	pReq->s.indexL = index & 0xff;
	pReq->s.indexH = index >> 8;
	pReq->s.subIndex = subIndex;
	pReq->s.cmd = DATA_REQCMD_SINGLE_READ;

	reqData.pReqDataFct = pFct;
	reqData.devIdx = devIdx;

	ebRequestData(devIdx, 1);


	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebRequestObjectWrite - write object data
*
* This function write object data at another node
* over the EBC.
* If the function is finished, 
* the given indication function is called.
*
* Note: Only one data request is possible at one time.
*
* \return
*/
EB_RET_T ebRequestObjectWrite(
		UNSIGNED8	devIdx,				/**< local device index */
		UNSIGNED8	nodeId,				/**< node id */
		UNSIGNED16	index,				/**< index at device */
		UNSIGNED8	subIndex,			/**< subindex at device */
		UNSIGNED8	*pData,				/**< data to write */
		UNSIGNED8	dataLen,			/**< len of data */
		EB_REQ_DATA_WRITE_FCT_T pFct	/**< function after result is available */
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];

	if (pReq->s.cmd != 0)  {
		return(EB_RET_BUSY);
	}
	/* check max data len */
	if (dataLen > EB_DATA_REQUEST_MAX_LEN)  {
		return(EB_RET_ERROR);
	}
	pReq->s.gin_nodeid = nodeId;
	pReq->s.devType = 0;
	pReq->s.indexL = index & 0xff;
	pReq->s.indexH = index >> 8;
	pReq->s.subIndex = subIndex;
	pReq->s.cmd = DATA_REQCMD_SINGLE_READ;
    pReq->s.dataLen = dataLen;
    memcpy(&pReq->s.data[0], pData, dataLen);

	pReq->s.cmd = DATA_REQCMD_SINGLE_WRITE;

	reqData.pReqDataWriteFct = pFct;
	reqData.devIdx = devIdx;

	ebRequestData(devIdx, 1);

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebRequestNodeId
*
* This function requests the node id from a node over the EBC.
* If the function is finished, 
* the given indication function is called.
*
* Note: Only one request is possible at one time.
*
* \return
*/
EB_RET_T ebRequestNodeId(
		UNSIGNED8	devIdx,				/**< local device index */
		UNSIGNED8	deviceType,			/**< device type for request node id */
		UNSIGNED8	gin,				/**< global instance number */
		EB_REQ_NODEID_FCT_T pFct		/**< function after result is available */
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];

	if (devIdx >= EB_VIRTUAL_DEV_CNT)  {
		return(EB_RET_ERROR);
	}

	if (pReq->s.cmd != 0)  {
		return(EB_RET_BUSY);
	}
	pReq->s.gin_nodeid = gin;
	pReq->s.devType = deviceType;
	pReq->s.cmd = DATA_REQCMD_NODEID;

	reqData.pReqNodeidFct = pFct;
	reqData.devIdx = devIdx;

	ebRequestData(devIdx, 1);

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebSetupRecPdoMSN_1 - setup a receive PDO for MSN1
*
* This function init a receive PDO for the MSN1
* with the given values.
* If the index == 0, dummy mapping is used with the given bitcount
*
* \return
*/
EB_RET_T ebSetupRecPdoMSN_1(
		UNSIGNED16			pdoOffs,			/**< PDO Offset */
		EB_PDO_MAP_ENTRY_T	*pDevStatus,		/**< device status */
		EB_PDO_MAP_ENTRY_T	*pDynCurrInpLimit,	/**< curr inp limit */
		EB_PDO_MAP_ENTRY_T	*pDynCurrOutpLimit,	/**< curr outp limit */
		EB_PDO_MAP_ENTRY_T	*pDynVoltLimit		/**< voltage limit */
	)
{
RET_T	retVal;
EB_PDO_MAP_ENTRY_T	*mapTable[4];

	mapTable[0] = pDevStatus;
	mapTable[1] = pDynCurrInpLimit;
	mapTable[2] = pDynCurrOutpLimit;
	mapTable[3] = pDynVoltLimit;

	retVal = setupRecPdoMSN(pdoOffs, mapTable, 4, 0x181 + pdoOffs);
	if (retVal != RET_OK)  {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebSetupRecPdoMSN_2 - setup a receive PDO for MSN2
*
* This function init a receive PDO for the MSN2
* with the given values.
* If the index == 0, dummy mapping is used with the given bitcount
*
* \return
*/
EB_RET_T ebSetupRecPdoMSN_2(
		UNSIGNED16			pdoOffs,			/**< PDO Offset */
		EB_PDO_MAP_ENTRY_T	*pActualCurrent,	/**< device actual current */
		EB_PDO_MAP_ENTRY_T	*pActualVoltage		/**< device actual voltage */
	)
{
RET_T	retVal;
EB_PDO_MAP_ENTRY_T	*mapTable[4];

	mapTable[0] = pActualCurrent;
	mapTable[1] = pActualVoltage;

	retVal = setupRecPdoMSN(pdoOffs, mapTable, 2, 0x202 + pdoOffs);
	if (retVal != RET_OK)  {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \brief ebSetupRecPdoMSN_3 - setup a receive PDO for MSN3
*
* This function init a receive PDO for the MSN3
* with the given values.
* If the index == 0, dummy mapping is used with the given bitcount
*
* \return
*/
EB_RET_T ebSetupRecPdoMSN_3(
		UNSIGNED16			pdoOffs,			/**< PDO Offset */
		EB_PDO_MAP_ENTRY_T	*pBatCapacity,		/**< battery capacity */
		EB_PDO_MAP_ENTRY_T	*pBatTemp,			/**< battery temperure */
		EB_PDO_MAP_ENTRY_T	*pElectronicTemp	/**< electronic tempereture */
	)
{
RET_T	retVal;
EB_PDO_MAP_ENTRY_T	*mapTable[4];

	mapTable[0] = pBatCapacity;
	mapTable[1] = pBatTemp;
	mapTable[2] = pElectronicTemp;

	retVal = setupRecPdoMSN(pdoOffs, mapTable, 3, 0x283 + pdoOffs);
	if (retVal != RET_OK)  {
		return(EB_RET_ERROR);
	}

	return(EB_RET_OK);
}


/***************************************************************************/
/**
*
* \internal
*
* \brief setupRecPdoMSN - setup a receive PDO
*
*
* \return
*/
static RET_T setupRecPdoMSN(
		UNSIGNED16			pdoOffs,		/* PDO Offset */
		EB_PDO_MAP_ENTRY_T	*pMapTable[],	/* map table */
		UNSIGNED8			mapCnt,			/* map count */
		UNSIGNED32			cobId			/* cobid */
	)
{
RET_T	retVal;
UNSIGNED8	i;
UNSIGNED32	mapVal = 0x00060010;

	/* disable PDO */
	retVal = coOdSetCobid(0x1400 + pdoOffs, 1, 0x80000000);
	if (retVal != RET_OK)  {
		return retVal;
	}

	/* disable mapping */
	retVal = coOdPutObj_u8(0x1600 + pdoOffs, 0, 0);
	if (retVal != RET_OK)  {
		return retVal;
	}

	for (i = 0; i < mapCnt; i++)  {

		if (pMapTable[i]->index != 0)  {
			mapVal =  ((UNSIGNED32)pMapTable[i]->index << 16)
					| ((UNSIGNED32)pMapTable[i]->subIndex << 8)
					| pMapTable[i]->len;
		} else {
			mapVal = 0x00060010;
		}
		retVal = coOdPutObj_u32(0x1600 + pdoOffs, i + 1, mapVal);
		if (retVal != RET_OK)  {
			return retVal;
		}
	}

	/* enable mapping */
	retVal = coOdPutObj_u8(0x1600 + pdoOffs, 0, mapCnt);
	if (retVal != RET_OK)  {
		return retVal;
	}

	/* enable PDO */
	retVal = coOdSetCobid(0x1400 + pdoOffs, 1, cobId);
	if (retVal != RET_OK)  {
		return retVal;
	}

	return(RET_OK);
}


/***************************************************************************/
/**
* \internal
*
* \brief iebResponsetransferData - response for transfer data
*
*
* \return
*/
void iebResponseTransferData(
		BOOL_T execFlag
	)
{
DATA_REQUEST_T	*pReq = (DATA_REQUEST_T *)&dataReqEbc[0];
DATA_RESPONSE_T	*pResp = (DATA_RESPONSE_T *)&dataRespEbc[0];

	if (execFlag == CO_FALSE)  {
		return;
	}

	pReq->s.cmd = 0;

	ebRequestData(reqData.devIdx, 0);

	switch (pResp->s.cmd)  {
		case DATA_REQCMD_EB_SINGLE_READ:
		case DATA_REQCMD_SINGLE_READ:
			responseTransferData(reqData.pReqDataFct);
			break;
		case DATA_REQCMD_EB_SINGLE_WRITE:
		case DATA_REQCMD_SINGLE_WRITE:
			responseTransferWriteData(reqData.pReqDataWriteFct);
			break;
		case DATA_REQCMD_NODEID:
			responseTransferNodeId(reqData.pReqNodeidFct);
			break;
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief
*
*
* \return
*/
static void responseTransferData(
		EB_REQ_DATA_FCT_T	pFct
	)
{
DATA_RESPONSE_T	*pResp = (DATA_RESPONSE_T *)&dataRespEbc[0];
UNSIGNED8	i;
UNSIGNED16	idx;

	idx = pResp->s.indexL | pResp->s.indexH << 8;

	EB_PRINT4("iebResponseObject: answer %lx for idx %x:%d, len: %d, data ",
		pResp->s.errCode, pResp->s.indexL | pResp->s.indexH << 8, pResp->s.subIndex,
		pResp->s.dataLen);
	for (i = 0; i < pResp->s.dataLen; i++)  {
		EB_PRINT1("0x%x ", pResp->s.data[i]);
	}
	EB_PRINT("\n");

	if (pFct != NULL)  {
		pFct(pResp->s.errCode, pResp->s.devType, pResp->s.gin_nodeid, idx,
			pResp->s.subIndex, pResp->s.data, pResp->s.dataLen);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief
*
*
* \return
*/
static void responseTransferWriteData(
		EB_REQ_DATA_WRITE_FCT_T	pFct
	)
{
DATA_RESPONSE_T	*pResp = (DATA_RESPONSE_T *)&dataRespEbc[0];
UNSIGNED16	idx;

	idx = pResp->s.indexL | pResp->s.indexH << 8;

	EB_PRINT3("iebResponseWrite: answer %lx for idx %x:%d",
		pResp->s.errCode, pResp->s.indexL | pResp->s.indexH << 8, pResp->s.subIndex);
	EB_PRINT("\n");

	if (pFct != NULL)  {
		pFct(pResp->s.errCode, pResp->s.devType, pResp->s.gin_nodeid, idx,
			pResp->s.subIndex);
	}
}


/***************************************************************************/
/**
* \internal
*
* \brief iebResponseNodeId - response for node id
*
*
* \return
*/
static void responseTransferNodeId(
		EB_REQ_NODEID_FCT_T	pFct
	)
{
DATA_RESPONSE_T	*pResp = (DATA_RESPONSE_T *)&dataRespEbc[0];

	EB_PRINT2("iebResponseNodeId: answer %lx, node id %d\n",
		pResp->s.errCode, pResp->s.indexL);

	if (pFct != NULL)  {
		pFct(pResp->s.errCode, pResp->s.devType, pResp->s.gin_nodeid, pResp->s.indexL);
	}
}
