/*
* co_drv.h - contains defines for driver
*
* Copyright (c) 2012-2014 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: co_drv.h 15743 2016-10-24 11:15:22Z hil $
*
*-------------------------------------------------------------------
*
*
*
*/

/**
* \brief defines for driver
*
* \file co_drv.h - contains defines for driver
*
*/

#ifndef CO_DRV_H
#define CO_DRV_H 1

#ifdef CO_CANFD_SUPPORTED
# define CO_CAN_MAX_DATA_LEN	64u
# define CO_CAN_FD_MAX_DLC		15u
#else /* CO_CANFD_SUPPORTED*/
# define CO_CAN_MAX_DATA_LEN	8u
#endif /* CO_CANFD_SUPPORTED*/


typedef	void *		LIBDRV_HANDLE_T;

/* datatypes 
-----------------------------------------------------------*/
/** CAN cob structure */
typedef struct {
	UNSIGNED32			canId;			/**< can identifier */
	UNSIGNED32			ignore;			/**< ignore mask for id */
	UNSIGNED16			canChan;		/**< reserved for driver */
	BOOL_T				extended;		/**< extended id  */
	BOOL_T				rtr;			/**< rtr */
	BOOL_T				enabled;		/**< cob enabled/disabled */
} CO_CAN_COB_T;


/** CAN message structure */
typedef struct {
	LIBDRV_HANDLE_T		handle;			/**< library internal handle */
	CO_CAN_COB_T		canCob;			/**< cob infos */
	UNSIGNED8			len;			/**< msg len */
	UNSIGNED8			data[CO_CAN_MAX_DATA_LEN];	/**< data */
} CO_CAN_MSG_T;



/** bittiming table entries */
typedef struct {
	UNSIGNED16 bitRate; 	/**< bitrate in kbit/s */
	UNSIGNED16 pre; 		/**< prescaler */
	UNSIGNED8 prop;			/**< propagation segment */
	UNSIGNED8 seg1; 		/**< segment 1 */
	UNSIGNED8 seg2; 		/**< segment 2 */
} CO_NV_STORAGE CODRV_BTR_T;


/* function prototypes */

EXTERN_DECL void codrvWaitForEvent(UNSIGNED32	msecTimeout);
EXTERN_DECL void codrvWaitForCanEvent(UNSIGNED32	waitTimeOut);
EXTERN_DECL void codrvHardwareInit(void);

/* CAN
-----------------------------------------------------------*/
EXTERN_DECL RET_T	codrvCanInit(UNSIGNED16 bitRate);
EXTERN_DECL RET_T	codrvCanReInit(UNSIGNED16	bitRate);
EXTERN_DECL RET_T	codrvCanSetBitRate(UNSIGNED16	bitRate);
EXTERN_DECL RET_T	codrvCanStartTransmission(void);
EXTERN_DECL void	codrvCanDriverHandler(void);
EXTERN_DECL RET_T	codrvCanEnable(void);
EXTERN_DECL RET_T	codrvCanDisable(void);
EXTERN_DECL RET_T	codrvCanDeinit(void);
EXTERN_DECL int		codrvGetCanFd(void);

/* QUEUE
-----------------------------------------------------------*/
EXTERN_DECL CO_CAN_MSG_T *coQueueGetReceiveBuffer(void);
EXTERN_DECL void	coQueueReceiveBufferIsFilled(void);
EXTERN_DECL void	coQueueMsgTransmitted(const CO_CAN_MSG_T *pBuf);
EXTERN_DECL CO_CAN_MSG_T *coQueueGetNextTransmitMessage(void);
EXTERN_DECL BOOL_T coQueueReceiveMessageAvailable(void);

/* GATEWAY
-----------------------------------------------------------*/
EXTERN_DECL void coGatewayTransmitMessage(const CO_CAN_MSG_T *pMsg);
EXTERN_DECL void coQueueRecMsgFromGw(CO_CAN_MSG_T *pMsg);

/* CAN IRQ
-----------------------------------------------------------*/
EXTERN_DECL void codrvCanEnableInterrupt(void);
EXTERN_DECL void codrvCanDisableInterrupt(void);

/* CAN - FullCAN functionality (Acceptance filter)
-----------------------------------------------------------*/
EXTERN_DECL RET_T codrvCanSetFilter(CO_CAN_COB_T *pCanCob);

/* timer
-----------------------------------------------------------*/
EXTERN_DECL RET_T codrvTimerSetup(UNSIGNED32 timerInterval);

#endif /* CO_DRV_H */

