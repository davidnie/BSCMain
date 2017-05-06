
#include "main.h"

/***********************************************************************
BSP_SCI.c
***********************************************************************/

volatile unsigned char RS232_REC_Flag = 0;
volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE];//用于接收数据
volatile unsigned int RS232_rec_counter = 0;//用于RS232接收计数
	NVIC_InitTypeDef   NVIC_InitStructure;

unsigned short RS232_send_data_length = 0;
extern OS_EVENT  *sem_RS232_rec_flag;			//RS232接收完一桢数据信号量定义


volatile unsigned char U4_REC_Flag = 0;
volatile unsigned char U4_buff[RS232_REC_BUFF_SIZE];//用于接收数据
volatile unsigned int U4_rec_counter = 0;//用于RS232接收计数

unsigned short U4_send_data_length = 0;
extern OS_EVENT  *sem_U4_rec_flag;			//RS232接收完一桢数据信号量定义

/***********************************************************************
函name: void UART1_Configuration(void) 
input:
output:
function:
note:    wifi confige to use USART1
***********************************************************************/
void UART1_Configuration(void)
{ 
  
	GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO_InitTypeDef类型的结构体成员GPIO_InitStructure

	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	//enable Port A closd
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD, ENABLE);
	//enable UART1 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// reset USART1
	USART_DeInit(USART1);
	
	USART_StructInit(&USART_InitStructure);//define a structure to same the configuration
	USART_ClockStructInit(&USART_ClockInitStruct);//载入默认USART参数        
	//config pin  PA9 USART1_TX PA10 USART1_RX    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //config to use aux function
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //output mode
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //config to use aux function
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);        //PA10 as Rx

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //config to use aux function
	GPIO_Init(GPIOA, &GPIO_InitStructure);                                                                                                                 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	
	USART_ClockInit(USART1,&USART_ClockInitStruct);


	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure); 
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);  
	USART_ITConfig(USART1,USART_IT_IDLE,DISABLE);
 	USART_ITConfig(USART1, USART_IT_TC, DISABLE);// 
	
	   //Usart1 NVIC 
		 #if 1
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//interrupt priority 3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//sub priority
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ channel enable
	NVIC_Init(&NVIC_InitStructure);	
//use DMA to receive data
//nie	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); 
#endif
	
	
	USART_Cmd(USART1, ENABLE); 
	USART_ClearITPendingBit(USART1, USART_IT_TC);//清除中断TC位	

}

/***********************************************************************
函数名称：void USART1_IRQHandler(void) 
功    能：完成SCI的数据的接收，并做标识
输入参数：
输出参数：
编写时间：2012.11.22
编 写 人：
注    意：RS232用的是USART1
***********************************************************************/
void USART1_IRQHandler_DMA(void)  
{

	unsigned char temp = 0;
	INT8U err;
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();    // 关中断                               
    OSIntNesting++;	   		//中断嵌套层数，通知ucos
    OS_EXIT_CRITICAL();	   	//开中断
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{	
		temp = USART1->SR;  
		temp = USART1->DR; 												//清USART_IT_IDLE标志 
		RS232_REC_Flag = 1;	   											//DMA接收标志
		err = OSSemPost(sem_RS232_rec_flag);  //抛出一个信号量表示RS232已经接收完成一帧数据
		DMA_Cmd(DMA2_Stream5, DISABLE); 							  	//读取数据长度先关闭DMA 
		RS232_rec_counter = RS232_REC_BUFF_SIZE - DMA_GetCurrDataCounter(DMA2_Stream5);//获取DMA接收的数据长度，
		
		DMA_SetCurrDataCounter(DMA2_Stream5,RS232_REC_BUFF_SIZE);		//设置传输数据长度    
		DMA_Cmd(DMA2_Stream5, ENABLE);
	}
	if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)					//串口发送中断
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		RS232_dma_send_flag = 0;										//允许再次发送
	}	
	OSIntExit();//中断退出，通知ucos，（该句必须加）	
}

/***********************************************************************
函数名称：RS232_DMA_Send(unsigned char *send_buff,unsigned int length)
功    能：RS232  DMA方式发送字符串
输入参数：send_buff:待发送的数据指针；length：发送的数据长度（字符个数）
输出参数：
编写时间：2012.11.22
编 写 人：
注    意：
***********************************************************************/
void RS232_DMA_Send(unsigned char *send_buff,unsigned int length)
{
	
	unsigned int counter_232 = 0;
	while(RS232_dma_send_flag != 0)					//等待上次的DMA传输完成
	{
		counter_232 ++;
		if(counter_232 >= 0xffff)
		{
			break;
		}
	}						 
	RS232_dma_send_flag = 1;
	DMA2_Stream7 -> M0AR = (u32)send_buff;
	DMA_SetCurrDataCounter(DMA2_Stream7,length);  	//设置传输长度
	DMA_Cmd(DMA2_Stream7,ENABLE);					//启动DMA传输
}

/***********************************************************************
函数名称：RS232_Send_Data(unsigned char *send_buff,unsigned int length)
功    能：RS232发送字符串
输入参数：send_buff:待发送的数据指针；length：发送的数据长度（字符个数）
输出参数：
编写时间：2012.11.22
编 写 人：
注    意：
***********************************************************************/
void RS232_Send_Data(unsigned char *send_buff,unsigned int length)
{
 	unsigned int i = 0;
	for(i = 0;i < length;i ++)
	{			
		USART1->DR = send_buff[i];
		while((USART1->SR&0X40)==0);	
	}
}
/***********************************************************************
函name: void USART_4_Configuration(void) 
input:
output:
function: PC10 as UART4_Tx, PC11 as UART4_Rx
note:    RS232 confige to use USART1
***********************************************************************/
void UART4_Configuration(void)
{ 
  
	GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO_InitTypeDef类型的结构体成员GPIO_InitStructure

	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	//enable Port C closd
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);
	//enable UART4 clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	// reset USART4
	USART_DeInit(UART4);
	
	USART_StructInit(&USART_InitStructure);//define a structure to same the configuration
	USART_ClockStructInit(&USART_ClockInitStruct);//载入默认USART参数        
	//config pin PC10 USART4_TX PC11 USART4_RX    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //output mode
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //config to use aux function
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //config to use aux function
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);        //PC10 as Tx
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4);        //PA11 as Rx
	
	USART_ClockInit(UART4,&USART_ClockInitStruct);


	USART_InitStructure.USART_BaudRate = 4800;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4,&USART_InitStructure); 
	
	//USART_ITConfig(UART4,USART_IT_RXNE,DISABLE);  
	USART_ITConfig(UART4,USART_IT_RXNE,ENABLE);  //receive interrupt
 	USART_ITConfig(UART4, USART_IT_TC, DISABLE);// send interrupt
	
	//use DMA to receive data
	USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); 
	USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE); 
	
	
	USART_ClearITPendingBit(UART4, USART_IT_TC);//
	USART_Cmd(UART4, ENABLE);

}

/***********************************************************************
***********************************************************************/
void UART4_IRQHandler(void)  
{

	unsigned char temp = 0;
	INT8U err;
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();    // 关中断                               
    OSIntNesting++;	   		//中断嵌套层数，通知ucos
    OS_EXIT_CRITICAL();	   	//开中断
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{	
		temp = UART4->SR;  
		temp = UART4->DR; 												//清USART_IT_IDLE标志 
		U4_REC_Flag = 1;	   											//DMA接收标志
		err = OSSemPost(sem_U4_rec_flag);  //抛出一个信号量表示RS232已经接收完成一帧数据
		DMA_Cmd(DMA2_Stream4, DISABLE); 							  	//读取数据长度先关闭DMA 
		U4_rec_counter +=1;
		U4_buff[U4_rec_counter-1]=temp;
	}
	if(USART_GetITStatus(UART4, USART_IT_TC) != RESET)					//串口发送中断
	{
		USART_ClearITPendingBit(UART4, USART_IT_TC);
		U4_dma_send_flag = 0;										//允许再次发送
	}	
	OSIntExit();//中断退出，通知ucos，（该句必须加）	
}

void U4_DMA_Send(unsigned char *send_buff,unsigned int length)
{
	
	unsigned int counter_232 = 0;
	while(U4_dma_send_flag != 0)					//等待上次的DMA传输完成
	{
		counter_232 ++;
		if(counter_232 >= 0xffff)
		{
			break;
		}
	}						 
	U4_dma_send_flag = 1;
	DMA2_Stream4 -> M0AR = (u32)send_buff;
	DMA_SetCurrDataCounter(DMA2_Stream4,length);  	//设置传输长度
	DMA_Cmd(DMA2_Stream4,ENABLE);					//启动DMA传输
}

/***********************************************************************

***********************************************************************/
void U4_Send(unsigned char *send_buff,unsigned int length)
{
 	unsigned int i = 0;
	unsigned int srv;
	unsigned char b;
	for(i = 0;i < length;i ++)
	{			
		UART4->DR = send_buff[i];
		srv=UART4->SR;
		b=UART4->SR&0X40;
		while((UART4->SR&0X40)==0){
			srv=UART4->SR;
			b=srv&0X40;
	}
}
}
