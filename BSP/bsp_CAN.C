/***********************************************************************
文件名称：CAN.C
功    能：
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
#include "main.h"
#include "bsp_can.h"
CanRxMsg CAN1RxMsg;
CanRxMsg CAN2RxMsg;
CanTxMsg CAN1TxBuf[CAN_MSG_SEND_BUFFER_SIZE], CAN2TxBuf[CAN_MSG_SEND_BUFFER_SIZE];
UNS8 C1_wait_num=0;
UNS8 C2_wait_num=0;
unsigned char can1_rec_flag = 0;
unsigned char CAN2_data[8];
unsigned char can2_rec_flag = 0;
//格式： 波特率  CAN_SJW 	CAN_BS1  CAN_BS2 CAN_Prescaler 参考CAN_Baud_Process(unsigned int Baud,CAN_InitTypeDef *CAN_InitStructure)
const unsigned int CAN_baud_table[CAN_BAUD_NUM][5] = 
{
//波特率， CAN_SJW，   CAN_BS1，    CAN_BS2，CAN_Prescaler 
	{5,   CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_2tq,450},		//未通			
	{10,  CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_2tq, 400},		//未通			
	{15,  CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_2tq,150},		//15K  未通
	{20,  CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_2tq,200},		//20k //未通
	{25,  CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_8tq,112},		//25k  未通
	{40,  CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_2tq,100},		//40k  未通
	{50,  CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_8tq,56},			//50k	ok
	{62,  CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_2tq,36},			//62.5k
	{80,  CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_2tq,50},			//80k   未通
	{100, CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_8tq,28},			//100K	ok
	{125, CAN_SJW_1tq,CAN_BS1_13tq, CAN_BS2_2tq,18},		//125K 未通
	{200, CAN_SJW_1tq,CAN_BS1_6tq, CAN_BS2_8tq,14},			//200K  ok
	{250, CAN_SJW_1tq,CAN_BS1_15tq,CAN_BS2_5tq,8},		    //250k  ok
//	{250, CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_6tq,10},		    //250k  ok
	{400, CAN_SJW_1tq,CAN_BS1_15tq, CAN_BS2_5tq,5},			//400K  ok
	{500, CAN_SJW_1tq,CAN_BS1_15tq,CAN_BS2_5tq,4},			//500K	ok
//	{500, CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_6tq,5},			//500K	ok
	{666, CAN_SJW_1tq,CAN_BS1_5tq, CAN_BS2_2tq,8},			//未通
	{800, CAN_SJW_1tq,CAN_BS1_8tq, CAN_BS2_3tq,14},			//800K 未通
	{1000,CAN_SJW_1tq,CAN_BS1_15tq,CAN_BS2_5tq,2},			//1000K	ok
};
/***********************************************************************
函数名称：CAN_Configuration(void)
功    能：完成can的配置
输入参数：
输出参数：
编写时间：
编 写 人：
注    意：
***********************************************************************/
void CAN1_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	/* CAN GPIOs configuration **************************************************/

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect CAN pins to AF9 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1); 

	/* CAN configuration ********************************************************/  
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/* CAN register init */
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;



	CAN_Baud_Process(500,&CAN_InitStructure);
	CAN_Init(CAN1, &CAN_InitStructure);

	CAN_FilterInitStructure.CAN_FilterNumber = 0;	   //CAN1 filter id from 0 to 13

	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	   //滤波屏蔽模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;	//not filter any ID
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;		//not filter any ID
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;	  // /*!< Specifies the FIFO (0 or 1) which will be assigned to the filter.
													  //This parameter can be a value of @ref CAN_filter_FIFO */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure); 

	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}
/***********************************************************************
name: CAN_Configuration(void)
function: config CAN2

***********************************************************************/
void CAN2_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	/* CAN GPIOs configuration **************************************************/

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect CAN pins to AF9 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2); 

	/* CAN configuration ********************************************************/  
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2, ENABLE);//用can2时，can1时钟也要开启

	/* CAN register init */
	CAN_DeInit(CAN2);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;


	CAN_Baud_Process(250,&CAN_InitStructure);
	CAN_Init(CAN2, &CAN_InitStructure);

	CAN_FilterInitStructure.CAN_FilterNumber = 14;	   //CAN2 filter ID from 14 to 27

	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	   // filter mode
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;	//don't filter any ID
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;		//don't filter any ID
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;	  // /*!< Specifies the FIFO (0 or 1) which will be assigned to the filter.
													  //This parameter can be a value of @ref CAN_filter_FIFO */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure); 

	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);
}
/***********************************************************************
name: CAN_Baud_Process(unsigned int Baud,CAN_InitTypeDef *CAN_InitStructure)
function: set CAN port baudrate
CAN_SJW : CAN_SJW_1tq - CAN_SJW_4tq	  不能比任何一相位缓冲段长
CAN_BS1 : CAN_BS1_1tq - CAN_BS1_16tq
CAN_BS2 : CAN_BS2_1tq - CAN_BS2_8tq
CAN_Prescaler : 1 - 1024
CAN_SJW + CAN_BS1 / (CAN_SJW + CAN_BS1 + CAN_BS2)
	0.75     baud > 800k
	0.80     baud > 500k
	0.875    baud <= 500k
	baud = 42M / (CAN_SJW + CAN_BS1 + CAN_BS2) / CAN_Prescaler
***********************************************************************/
void CAN_Baud_Process(unsigned int Baud,CAN_InitTypeDef *CAN_InitStructure)
{
	unsigned int i = 0;
	for(i = 0;i < CAN_BAUD_NUM;i ++)
	{
		if(Baud == CAN_baud_table[i][0])
		{
			CAN_InitStructure->CAN_SJW = CAN_baud_table[i][1];
			CAN_InitStructure->CAN_BS1 = CAN_baud_table[i][2];
			CAN_InitStructure->CAN_BS2 = CAN_baud_table[i][3];
			CAN_InitStructure->CAN_Prescaler = CAN_baud_table[i][4];
			return;	
		}
	}	
}
/***********************************************************************
name: CAN1_RX0_IRQHandler(void)
function: CAN1 receive interrupt function
***********************************************************************/
void CAN1_RX0_IRQHandler(void)
{
	unsigned int i = 0;
	INT8U err;
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR */
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	CAN_Receive(CAN1, CAN_FIFO0, &CAN1RxMsg);
	codrvCanReceiveInterrupt(&CAN1RxMsg);
	can1_rec_flag = 1;
	err = OSSemPost(sem_CAN1_rec_flag);  //post a semphore to tell appliction message received
	CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);  /*clear interrupt flag */
	OSIntExit();
}
/***********************************************************************
function: save to be sent data to buffer. not real send
***********************************************************************/
	void CAN1_WriteData(unsigned int ID, unsigned char *data, unsigned char dlc, unsigned char ext)
{
		
	unsigned char i;
	uint8_t transmit_mailbox = 0;
	CanTxMsg TxMessage;
	static UNS8 wtPt=0;

	if (ext==CAN_Id_Extended){
		TxMessage.ExtId = ID;
	}else{
		TxMessage.StdId = ID;
	}
	TxMessage.IDE = ext;
	TxMessage.RTR = CAN_RTR_DATA; /*set to datea frame */
	TxMessage.DLC = 8;            /* data length */
	for(i = 0;i < 8; i ++)
	{
		TxMessage.Data[i] = data[i];
	}
	//save to Tx Buffer
	
	memcpy(&CAN1TxBuf[wtPt],&TxMessage,sizeof(CanTxMsg));
	if(++wtPt>=CAN_MSG_SEND_BUFFER_SIZE) wtPt=0;
	C1_wait_num++;
//	transmit_mailbox = CAN_Transmit(CAN1,&TxMessage);  /* return mailbox number 0,1,2. if no return no_box */	
// 	while((CAN_TransmitStatus(CAN1, transmit_mailbox)  !=  CANTXOK) && (i  !=  0xFFFF))
// 	{
// 		i ++;
// 	}

}
/***********************************************************************
name: CAN2_RX0_IRQHandler(void)
function: CAN2 Receive interrupt response
input : void
return: void
data in CAN2_data[8]
***********************************************************************/
void CAN2_RX0_IRQHandler(void)
{
	unsigned int i = 0;
	INT8U err;
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR */
 	OSIntNesting++;
	OS_EXIT_CRITICAL();	
	CAN_Receive(CAN2, CAN_FIFO0, &CAN2RxMsg);
	err = OSSemPost(sem_CAN2_rec_flag);  //trigger a enent, let Task_CAN2_Rev start receive
	can2_rec_flag = 1;
	CAN_ClearITPendingBit(CAN2,CAN_IT_FMP0);  /* clear interrupt */
	OSIntExit();
}
/***********************************************************************
name: CAN2_WriteData(void)
function: CAN2 send extended CAN message
input: ID and CAN2_data
output: 
time:
programmer:
note: extend Id frame
***********************************************************************/
void CAN2_WriteData(unsigned int ID, unsigned char *data, unsigned char dlc, unsigned char ext)
{
		
	unsigned char i;
	uint8_t transmit_mailbox = 0;
	CanTxMsg TxMessage;	
	static UNS8 wtPt=0;
	INT8U err;
	if (ext==CAN_Id_Extended){
		TxMessage.ExtId = ID;
	}else{
		TxMessage.StdId = ID;
	}
	TxMessage.IDE = ext;
	TxMessage.RTR = CAN_RTR_DATA; 
	TxMessage.DLC = dlc;           
	for(i = 0;i < dlc; i ++)
	{
		TxMessage.Data[i] = data[i];
	}
	//save to Tx Buffer
	
	memcpy(&CAN2TxBuf[wtPt],&TxMessage,sizeof(CanTxMsg));
	if(++wtPt>=CAN_MSG_SEND_BUFFER_SIZE) wtPt=0;
	C2_wait_num++;
	
	err = OSSemPost(sem_CAN2_send_flag);  //trigger a enent, let Task_CAN2_Send start send
//	transmit_mailbox = CAN_Transmit(CAN2,&TxMessage); 
// 	while((CAN_TransmitStatus(CAN2, transmit_mailbox)  !=  CANTXOK) && (i  !=  0xFFFF))
// 	{
// 		i ++;
// 	}
}
