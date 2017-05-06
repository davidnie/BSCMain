/***********************************************************************
文件名称：DMA.H
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
#ifndef _DMA_H_
#define _DMA_H_



extern  unsigned char ADC_dma_ready_flag;

extern volatile unsigned char RS232_dma_send_flag;
extern 	volatile unsigned char U4_dma_send_flag;
void RS232_DMA_Init(void);
void DMA_RS232_Send_Configuration(unsigned char *send_buff,unsigned int send_size);

#endif
