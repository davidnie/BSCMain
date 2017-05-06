/*
* ico_nmt.h - contains internal defines for NMT
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: ico_nmt.h 12546 2016-02-29 10:26:29Z boe $
*
*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief dataty type header
*/

#ifndef ICO_NMT_H
#define ICO_NMT_H 1

/* datatypes */

typedef enum {
	CO_NMT_MASTER_STATE_POWERON,
	CO_NMT_MASTER_STATE_DETECT,
	CO_NMT_MASTER_STATE_SLAVE,
	CO_NMT_MASTER_STATE_MASTER
} CO_NMT_MASTER_STATE_T;


/* object 0x1f80 */
#define CO_NMT_STARTUP_BIT_MASTER		0x00000001ul
#define CO_NMT_STARTUP_BIT_STARTNMT0	0x00000002ul
#define CO_NMT_STARTUP_BIT_STARTITSELF	0x00000004ul
#define CO_NMT_STARTUP_BIT_STARTNODE	0x00000008ul
#define CO_NMT_STARTUP_BIT_RESETNODES	0x00000010ul
#define CO_NMT_STARTUP_BIT_FLYMA		0x00000020ul
#define CO_NMT_STARTUP_BIT_STOPNODES	0x00000040ul

/* object 0x1f81 */
#define CO_NMT_SLAVE_ASSIGN_BIT_SLAVE		0x00000001ul
#define CO_NMT_SLAVE_ASSIGN_BIT_BOOT		0x00000004ul
#define CO_NMT_SLAVE_ASSIGN_BIT_MANDATORY	0x00000008ul
#define CO_NMT_SLAVE_ASSIGN_BIT_RESETCOMM	0x00000010ul
#define CO_NMT_SLAVE_ASSIGN_BIT_SWVERSION	0x00000020ul


#ifndef CO_NODE_ID
# define CO_NODE_ID		icoNmtPersistantNodeIdGet()
#endif /* CO_NODE_ID */


/* function prototypes */

void	icoNmtMsgHandler(CO_CONST CO_REC_DATA_T *pNmtRec);
void	icoNmtErrorCtrlHandler(CO_CONST CO_REC_DATA_T *pRecData);
void	icoErrorCtrlReset(void);
void	icoErrorCtrlSetDefaultValue(void);
RET_T	icoHbProdStart(void);
void	*icoErrCtrlGetObjectAddr(UNSIGNED16 index, UNSIGNED8	subIndex);
RET_T	icoErrCtrlObjChanged(UNSIGNED16 index, UNSIGNED8	subIndex);
RET_T	icoErrCtrlCheckObjLimit(UNSIGNED8	subIndex,
			UNSIGNED32 newValue);
COB_REFERENZ_T icoNmtGetCob(void);
void	icoNmtStateChange(CO_NMT_STATE_T reqState, BOOL_T	cmdFromMaster);
RET_T	icoNmtStateReq(UNSIGNED8 node, CO_NMT_STATE_T	reqState,
			BOOL_T master, UNSIGNED8 flags);
void	icoErrorBehavior(void);
void	icoNmtResetNodeId(void);
void	icoNmtVarInit(const UNSIGNED8 *pNodeId, const CO_NODE_ID_T *nodeIdFct);
void	icoErrCtrlInd(UNSIGNED8 node, CO_ERRCTRL_T state,
			CO_NMT_STATE_T nmtState);

UNSIGNED8	icoNmtPersistantNodeIdGet(void);
void	icoErrorCtrlVarInit(UNSIGNED8 *pList);
void	icoNmtSetDefaultValue(void);
void	icoNmtApplyObj(void);

void	icoNmtMasterVarInit(UNSIGNED8 *pSlaveAssign);
void	icoGuardingVarInit(UNSIGNED8 *pSlaveAssign);
void	icoManagerVarInit(UNSIGNED8 *pSlaveAssign);

void	icoFlymaVarInit(void);
void	icoNmtMasterReset(void);
void	*icoNmtGetObjectAddr(UNSIGNED16 index, UNSIGNED8 subIndex);
RET_T	icoNmtObjChanged(UNSIGNED16 index, UNSIGNED8 subIndex);

void	icoNmtFlymaReset(void);
UNSIGNED32	icoNmtMasterGetStartupObj(void);
RET_T	icoNmtFlymaInit(void);
void	icoNmtMasterSetState(CO_NMT_MASTER_STATE_T mState);
CO_NMT_MASTER_STATE_T icoNmtMasterGetState(void);
void	icoNmtFlymaHandler(CO_CONST CO_REC_DATA_T *pRecData);
void	icoNmtFlymaErrCtrlFailure(UNSIGNED8 node);
void	*icoFlymaGetObjectAddr(UNSIGNED16 index, UNSIGNED8 subIndex);

CO_NMT_STATE_T icoGuardGetRemoteNodeState(UNSIGNED8 nodeId);
void	icoGuardingHandler(CO_CONST CO_REC_DATA_T	*pRecData);

UNSIGNED32	*icoNmtMasterGetSlaveAssignObj(void);
#endif /* ICO_NMT_H */




