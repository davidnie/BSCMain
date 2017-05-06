/***********************************************************************
文件名称：SCI.h
功    能：
编写时间：2012.11.22
编 写 人：
注    意：
***********************************************************************/
#ifndef  _SCI_H_
#define  _SCI_H_

#define RS232_REC_BUFF_SIZE				1024

extern volatile unsigned char RS232_REC_Flag ;
extern volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE] ;//用于接收数据
extern volatile unsigned int RS232_rec_counter ;//用于RS232接收计数
extern unsigned short RS232_send_data_length;

extern volatile unsigned char U4_REC_Flag ;
extern volatile unsigned char U4_buff[RS232_REC_BUFF_SIZE] ;//用于接收数据
extern volatile unsigned int U4_rec_counter ;//用于RS232接收计数
extern unsigned short U4_send_data_length;


void RS232_Send_Data(unsigned char *send_buff,unsigned int length);
void RS232_DMA_Send(unsigned char *send_buff,unsigned int length);
void UART1_Configuration(void);
void U4_Send(unsigned char *send_buff,unsigned int length);
void U4_DMA_Send(unsigned char *send_buff,unsigned int length);
void UART4_Configuration(void);
static void RS232_Delay(uint32_t nCount);
#endif
