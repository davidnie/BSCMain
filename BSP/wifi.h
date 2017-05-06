#ifndef __WIFI_H
#define __WIFI_H

#include "stm32f4xx.h"
#include "string.h"

#include "stdio.h"
 
//Wifi����״̬��־
#define WPRT_STATE			(1<<0)
#define NIP_STATE			(1<<1)
#define ATM_STATE 			(1<<2)
#define ATRM_STATE 			(1<<3)
#define SSID_STATE 			(1<<4)
#define ENCRY_STATE 		(1<<5)
#define KEY_STATE 			(1<<6)

	
#define MAX_RECV_LEN 512

typedef struct 
{
	char* SSID; 	//��������

	char* KEY;	//key

	char* ObjIP;	//Ŀ��IP

	char* ObjPort; //Ŀ��˿�

	char TorU;

}Wifi_InitTypeDef;

extern u16 RecvLen;
extern char RecvBUF[MAX_RECV_LEN];

void wifi_init(void);//��ʼ��wifi�˿�usart2��
void Wifi_ProcessAtResp(u8 Res);
void procBUFF(char* buf, u16 len, char* argv[], u16 *argc);

void SendToUSARTx(USART_TypeDef* USARTx, char* pdata, u16 len);
s8 EnterCMDState(USART_TypeDef* USARTx);
//���ݲ�������wifiģ��
u16 WIFI_Config(USART_TypeDef* USARTx, Wifi_InitTypeDef* Wifi_InitStruct);


s8 wifiInit(void);


#endif

