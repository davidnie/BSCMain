/***********************************************************************
***********************************************************************/
#ifndef _BSP_CAN_H_
#define _BSP_CAN_H_

#define CAN_BAUD_NUM    18		//
#define CAN_MSG_SEND_BUFFER_SIZE 30
typedef struct {
 
		unsigned char	SourceAddress;
		unsigned char	PDUSpecific;
		unsigned char	PDUFormat;					// CA should use only PDUFormat.
		unsigned int	DataPage			: 1;
		unsigned int	Res					: 1;
		unsigned int	Priority			: 3;
}J1939_ID_STRUCT;
typedef union {
	J1939_ID_STRUCT j1939_id;
	unsigned int eid;
}J1939_ID_UNION;
//typedef union J1939_ID_UNION J1939_ID;
typedef struct{
	J1939_ID_UNION jid;
	unsigned int pgn;
	unsigned char dlc;
	unsigned char data[8];
}J1939_MESSAGE;

extern CanRxMsg CAN1RxMsg;
extern CanRxMsg CAN2RxMsg;
extern unsigned char can1_rec_flag;
extern unsigned char CAN2_data[8];
extern unsigned char can2_rec_flag;
extern CanTxMsg CAN1TxBuf[CAN_MSG_SEND_BUFFER_SIZE];
extern CanTxMsg CAN2TxBuf[CAN_MSG_SEND_BUFFER_SIZE];
extern unsigned char C1_wait_num,C2_wait_num;
void CAN1_Configuration(void);
void CAN2_Configuration(void);
void CAN1_WriteData(unsigned int ID, unsigned char *data, unsigned char dlc, unsigned char ext);
//void CAN2_WriteData(unsigned int ID);
void CAN2_WriteData(unsigned int ID, unsigned char *data, unsigned char dlc, unsigned char ext);

void CAN_Baud_Process(unsigned int Baud,CAN_InitTypeDef *CAN_InitStructure);
#endif
