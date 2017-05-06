#ifndef __WIFI_H
#define __WIFI_H

#include "stm32f4xx.h"
#include "string.h"

#include "stdio.h"
 
//Wifi配置状态标志
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
	char* SSID; 	//网络名称

	char* KEY;	//key

	char* ObjIP;	//目标IP

	char* ObjPort; //目标端口

	char TorU;

}Wifi_InitTypeDef;

extern u16 RecvLen;
extern char RecvBUF[MAX_RECV_LEN];

void wifi_init(void);//初始化wifi端口usart2；
void Wifi_ProcessAtResp(u8 Res);
void procBUFF(char* buf, u16 len, char* argv[], u16 *argc);

void SendToUSARTx(USART_TypeDef* USARTx, char* pdata, u16 len);
s8 EnterCMDState(USART_TypeDef* USARTx);
//根据参数配置wifi模块
u16 WIFI_Config(USART_TypeDef* USARTx, Wifi_InitTypeDef* Wifi_InitStruct);


s8 wifiInit(void);


#endif

