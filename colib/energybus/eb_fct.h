/*
* eb_fct.h - contains function prototypes
*
* Copyright (c) 2012-2014 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: eb_fct.h 9832 2015-07-09 09:27:20Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \file
* \brief datatype type header
*/

#ifndef EB_FCT_H
#define EB_FCT_H 1

#include <co_canopen.h>
#include <eb_data.h>



/* datatypes */

/** \brief function pointer to CANopen NMT events
 * \param state - new NMT state
 * 
 * This function is called, if the CANopen NMT state has been changed.
 * <br>
 * Valid states are: <br>
 * 4 - STOPPED,<br>
 * 5 - OPERATIONAL,<br>
 * 127 - PRE_OPERATIONAL
 *
 * \return void
 */
typedef void (* EB_EVENT_NMT_T)(UNSIGNED8 state);


/** \brief function pointer to EBC monitoring events
 * \param node	- node number (1 - EBC, 0 - other node)
 * \param nmtState - new NMT state
 * 
 * This function reports a change of the EBC monitoring state.
 * If the heartbeat of the EBC failed or starts again
 * this function shows the actual state of the EBC or other node.
 *
 * \return void
 */
typedef void (* EB_EVENT_EBCMON_T)(UNSIGNED8 node, UNSIGNED8 nmtState);


/** \brief function pointer to SYNC event
 * \param none
 * 
 * This function is called after a sync message was received
 *
 * \return void
 */
typedef void (* EB_EVENT_SYNC_T)(void);


/** \brief function pointer to state change event
 * \param newState - new state
 * \param devNr - device number 1..n
 * 
 * This function is called,
 * if the state (energyBus state or device specific state)
 * has been changed.
 *
 * \return void
 */
typedef void (* EB_EVENT_STATE_T)(UNSIGNED16 newState, UNSIGNED8 devNr);


/** \brief function pointer to check change event
 * \param newState - new state
 * \param devNr - device number 1..n
 * 
 * This function is called,
 * if a new state (energyBus state or device specific state)
 * shall be entered.
 *
 * \return CO_TRUE
 *	enter new state
 * \return CO_FALSE
 *	discard state change
 */
typedef BOOL_T (* EB_EVENT_CHECK_STATE_T)(UNSIGNED16 newState, UNSIGNED8 devNr);


/** \brief function pointer to PDO event
 * \param pdoNr - PDO number
 * 
 * This function is called, if a PDO was received.
 *
 * \return void
 */
typedef void (* EB_PDO_T)(UNSIGNED16 pdoNr);


/** \brief function pointer to SDO write indication
 * \param index - index object written
 * \param subIndex - subIndex object written
 * 
 * This function is called, after a new value was written
 * to the given object
 *
 * \return void
 */
typedef void (* EB_SDO_WRITE_T)(UNSIGNED16 index, UNSIGNED8 subindex);


/** \brief function pointer to SDO read indication
 * \param index - index object written
 * \param subIndex - subIndex object written
 * 
 * This function is called, before a new value is read
 *
 * \return void
 */
typedef void (* EB_SDO_READ_T)(UNSIGNED16 index, UNSIGNED8 subindex);


/** \brief function pointer to communication event
 * \param event - communication event
 * 
 * This function is called, if the communication state
 * (buffer full/empty)
 * has been occured.
 *
 * \return void
 */
typedef void (* EB_COMM_EVENT_T)(CO_COMM_STATE_EVENT_T event);


/** \brief function pointer to CAN event
 * \param state - new CAN state
 * 
 * This function is called, if a new CAN event has been occured.
 * Can be BUS_OFF, PASSIVE or ACTIVE.
 *
 * \return void
 */
typedef void (* EB_CAN_STATE_T)(CO_CAN_STATE_T state);


/** \brief function pointer to sleep event
 * \param mode - sleep mode
 * 
 * This function is called, if a sleep event should be executed.
 * The application can inhibit this event by returning an value != 0.
 *
 * \return 0 - continue sleep event
 * \return >0 - abort event
 */
typedef UNSIGNED8 (* EB_SLEEP_T)(CO_SLEEP_MODE_T mode);


/** \brief function pointer for data requests over EBC indication
 * \param answerCode- ok or error code
 * \param devType	- device type
 * \param gin		- global instance number/node-id
 * \param idx		- object index
 * \param subIdx	- object subindex
 * \param *pData	- poiner to data
 * \param len		- data len
 * 
 * This function is called, if the data transfer over the EBC was finished.
*
 * \return void
 */
typedef void (* EB_REQ_DATA_FCT_T)(UNSIGNED32, UNSIGNED8, UNSIGNED8, UNSIGNED16, UNSIGNED8, UNSIGNED8 *, UNSIGNED8); /*lint !e960 customer specific parameter names */


/** \brief function pointer for data write requests over EBC indication
 * \param answerCode- ok or error code
 * \param devType	- device type
 * \param gin		- global instance number/node-id
 * \param idx		- object index
 * \param subIdx	- object subindex
 * 
 * This function is called, if the data transfer over the EBC was finished.
*
 * \return void
 */
typedef void (* EB_REQ_DATA_WRITE_FCT_T)(UNSIGNED32, UNSIGNED8, UNSIGNED8, UNSIGNED16, UNSIGNED8); /*lint !e960 customer specific parameter names */


/** \brief function pointer for node-id requests over EBC indication
 * \param answerCode- ok or error code
 * \param devType	- device type
 * \param gin		- global instance number
 * \param nodeID	- node id
 * 
 * This function is called, if the data transfer over the EBC was finished.
*
 * \return void
 */
typedef void (* EB_REQ_NODEID_FCT_T)(UNSIGNED32, UNSIGNED8, UNSIGNED8, UNSIGNED8); /*lint !e960 customer specific parameter names */


/* external data */

/* function prototypes */

EB_RET_T	ebFrameworkInit(UNSIGNED16 bitRate, UNSIGNED32 timerInterval,
				CO_EVENT_STORE_T pLoadFunction);
EB_RET_T	ebRegister_NMT(EB_EVENT_NMT_T pFunction);
EB_RET_T	ebRegister_EBCMON(EB_EVENT_EBCMON_T pFunction);
EB_RET_T	ebRegister_SYNC(EB_EVENT_SYNC_T pFunction);
EB_RET_T	ebRegister_SYNC_FINISHED(EB_EVENT_SYNC_T pFunction);
EB_RET_T	ebRegister_STATE(EB_EVENT_CHECK_STATE_T pCheckFunction,
				EB_EVENT_STATE_T pFunction);
EB_RET_T	ebRegister_SDO_WRITE(EB_SDO_WRITE_T pFunction);
EB_RET_T	ebRegister_SDO_READ(EB_SDO_READ_T pFunction);
EB_RET_T	ebRegister_PDO(EB_PDO_T pFunction);
EB_RET_T	ebRegister_COMM_EVENT(EB_COMM_EVENT_T pFunction);
EB_RET_T	ebRegister_CAN_STATE(EB_CAN_STATE_T pFunction);
EB_RET_T 	ebRegister_ERRCTRL(EB_EVENT_EBCMON_T	pFunction);
EB_RET_T 	ebRegister_SLEEP(EB_SLEEP_T	pFunction);

EB_RET_T	ebStateChange(UNSIGNED16 newState, UNSIGNED8 devIdx);
EB_RET_T	ebStateTransition(UNSIGNED16 ctrlWord, UNSIGNED8 devIdx);

EB_RET_T	ebRequestEbDataRead(UNSIGNED8 deviceIdx, UNSIGNED8 deviceType,
				UNSIGNED8 gin, UNSIGNED16 index, EB_REQ_DATA_FCT_T pFct);
EB_RET_T	ebRequestObjectRead(UNSIGNED8 deviceIdx, UNSIGNED8 nodeId,
				UNSIGNED16 index, UNSIGNED8 subIndex, EB_REQ_DATA_FCT_T pFct);
EB_RET_T	ebRequestEbDataWrite(UNSIGNED8 deviceIdx, UNSIGNED8 deviceType,
				UNSIGNED8 gin, UNSIGNED16 index,
				UNSIGNED8 *pData, UNSIGNED8 len, EB_REQ_DATA_WRITE_FCT_T pFct);
EB_RET_T	ebRequestObjectWrite(UNSIGNED8 deviceIdx, UNSIGNED8 nodeId,
				UNSIGNED16 index, UNSIGNED8 subIndex,
				UNSIGNED8 *pData, UNSIGNED8 len, EB_REQ_DATA_WRITE_FCT_T pFct);
EB_RET_T	ebRequestNodeId(UNSIGNED8 devIdx, UNSIGNED8 deviceType,
				UNSIGNED8 gin, EB_REQ_NODEID_FCT_T pFct);
EB_DATA_T	*ebGetEbDataPtr(UNSIGNED16 deviceType, UNSIGNED8 devIdx);

EB_RET_T	ebSetupRecPdoMSN_1(UNSIGNED16 pdoOffs,
		EB_PDO_MAP_ENTRY_T	*pDevStatus, EB_PDO_MAP_ENTRY_T	*pDynCurrInpLimit,
		EB_PDO_MAP_ENTRY_T	*pDynCurrOutpLimit, EB_PDO_MAP_ENTRY_T	*pDynVoltLimit);
EB_RET_T	ebSetupRecPdoMSN_2(UNSIGNED16	pdoOffs,
		EB_PDO_MAP_ENTRY_T	*pActualCurrent, EB_PDO_MAP_ENTRY_T	*pActualVoltage);
EB_RET_T	ebSetupRecPdoMSN_3(UNSIGNED16	pdoOffs,
		EB_PDO_MAP_ENTRY_T	*pBatCapacity, EB_PDO_MAP_ENTRY_T	*pBatTemp,
		EB_PDO_MAP_ENTRY_T	*pElectronicTemp);

void	ebSleepAwake(void);
#endif /* EB_FCT_H */

