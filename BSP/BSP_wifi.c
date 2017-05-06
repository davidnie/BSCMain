#include "wifi.h"


u16 RecvLen = 0;   	//highest bit, indicate if AT response receive finished
char RecvBUF[MAX_RECV_LEN];	//receive buffer

//save AT response in the RevbBUF, set RecvLen bit to 1 when recevied 0x0d 0x0a 0x0d 0x0a
void Wifi_ProcessAtResp(u8 Res)
{   
	u16 len = (RecvLen&0x7fff);
	RecvBUF[len] = Res;
	RecvLen++;
	if(len>=4&&len<MAX_RECV_LEN)
	{
		if((Res == 0x0a)&&(RecvBUF[len-1]==0x0d)&&(RecvBUF[len-2] == 0x0a)&&(RecvBUF[len-3]==0x0d))
			RecvLen|=0x8000;
	}
	else if(len>= MAX_RECV_LEN)
	{
		RecvLen = 0;
	}
}


/*
    function: process received data, 
    return:  1-success 0-fail
    note: not clear RecvLen bit after process data
    */
s8 RecvState()
{
	if(strncmp(RecvBUF,"+OK",3) == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


void USART1_IRQHandler(void)                	//����2�жϷ������
{
	u8 Res;
	OSIntEnter();    
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		Wifi_ProcessAtResp(Res); //(���յ������ݱ�����0x0d 0x0a 0x0d 0x0a��β)		 
     } 
	OSIntExit();  											 
}



//�������ܣ������յ��ַ����ָ�����ɲ���������������
//�����б�
//buf:�������ַ���
//len:�������ַ�������
//argv: ���ص��ַ�������ָ��
//argc�����ص��ַ������鳤��ָ��
//���磺   	char myargv[10][32];int myargc;
//			procBUFF(RecvBUF+4,(RecvLen&0x7ffff)-4,myargv,&myargc); //RecvBUF = "+OK=xxx" xxx���ǽ�Ҫ������ַ���
//
void procBUFF(char* buf, u16 len, char* argv[], u16 *argc)
{
	u16 tempLen,tempIndex=0;
	*argc = 0;

	for(tempLen=0;tempLen<len;tempLen++)
	{
		argv[*argc][tempIndex] = buf[tempLen];
		if(buf[tempLen]==',')
		{
			argv[*argc][tempIndex] = '\0';
			tempIndex = 0;
			(*argc)++;
		}
		else if((buf[tempLen]==0x0a)&&(buf[tempLen-1]==0x0d))
		{
			if(tempIndex>1)
			{
				argv[*argc][tempIndex-1] = '\0';
				tempIndex = 0;
				(*argc)++;
			}
			else
				tempIndex = 0;				
		}
		else
			tempIndex++;			
	}
}

//�������ܣ������Ѷ��������ݵ�����x
//
void SendToUSARTx(USART_TypeDef* USARTx, char* pdata, u16 len)
{
	u16 index = 0;
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TC)!=SET);//�ȴ����ͽ���

	for(index=0;index<len;index++)
	{
		USART_SendData(USARTx, pdata[index]);//�򴮿�2��������
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TC)!=SET);//�ȴ����ͽ���	
	}
}

//�������ܣ� ������������ģʽ
//ָ�+++
//����ֵ���ɹ�����0��ʧ�ܷ���-1��
s8 EnterCMDState(USART_TypeDef* USARTx)
{
	u8 i=0;
	SendToUSARTx(USARTx,"+++",3);

	RecvLen = 0;	//��0�����ַ������峤��

OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
	for(i=0; i<3; i++)
	{
		SendToUSARTx(USARTx,"AT+\r\n",3);	 //Test wifi module state
OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if((RecvLen&0x7fff)>0)	//Receive Data	RecvLen use bit:[0-14], bit15 is the state;
		{
			if(RecvState() == 0)		   //Receive "+OK",success.
				return 0;
			else
			{
				SendToUSARTx(USARTx,"+++",3);
				RecvLen = 0;	//��0�����ַ������峤��
OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
				}	
		}
	}
	return -1;	//Cannot receive "+OK"
}


//���ݲ�������wifiģ��
u16 WIFI_Config(USART_TypeDef* USARTx, Wifi_InitTypeDef* Wifi_InitStruct)
{
	u16 state = 0;
	char *arrtmp;
	char arr[30];
	arrtmp=arr;
	
	//SendToUSARTx(USARTx,"AT+\r\n",5);	 //Test wifi module state
	if(EnterCMDState(USARTx)==0)		  //��������ģʽ
	{
		RecvLen = 0;	//��0�����ַ������峤��
		arrtmp = "AT+WPRT=0\r\n";		  //infra
#if 1
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));

OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= WPRT_STATE;
		
		RecvLen = 0;	//��0�����ַ������峤��
arrtmp = "AT+NIP=0\r\n";		 //DHCP enable
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= NIP_STATE;
		
		RecvLen = 0;	//clear buffer
arrtmp = "AT+ATM=0\r\n";		 //automatic work mode
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= ATM_STATE;
		
		RecvLen = 0;	//clear buffer
		sprintf(arrtmp,"AT+ATRM=%d,0,\"%s\",%s,6000\r\n",Wifi_InitStruct->TorU,Wifi_InitStruct->ObjIP,Wifi_InitStruct->ObjPort); 
sprintf(arrtmp,"AT+ATRM=%d,0,\"%s\",%s,6000\r\n",Wifi_InitStruct->TorU,Wifi_InitStruct->ObjIP,Wifi_InitStruct->ObjPort); 
		//UDP, Client, target IP, target port, local port
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= ATRM_STATE;
		
		RecvLen = 0;	//��0�����ַ������峤��
		sprintf(arrtmp,"AT+SSID=\"%s\"\r\n", Wifi_InitStruct->SSID);	//SSID
arrtmp = "AT+SSID=\"summit\"\r\n";
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= SSID_STATE;	

		RecvLen = 0;	//��0�����ַ������峤��
arrtmp = "AT+ENCRY=7\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= ENCRY_STATE;

		RecvLen = 0;	//��0�����ַ������峤��
		sprintf(arrtmp,"AT+KEY=1,0,\"%s\"\r\n", Wifi_InitStruct->KEY);	//ascii format, index-0,
		sprintf(arr,"AT+KEY=1,0,\"%s\"\r\n", Wifi_InitStruct->KEY);	//ascii format, index-0,
arrtmp = "AT+KEY=1,0,\"abcde\"\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;
		
			RecvLen = 0;	//��0�����ַ������峤��
arrtmp = "AT+WSCAN\r\n";		 //encryption method. WPA1-PSK WPA
				SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 4000);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;	
#if 0
		RecvLen = 0;	//clear receive buffer
arrtmp = "AT+PMTF\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 2000);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;

				RecvLen = 0;	//clear receive buffer
arrtmp = "AT+Z\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 4000);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;
		#endif
		RecvLen = 0;	//��0�����ַ������峤��
arrtmp = "AT+WJOIN\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;

		RecvLen = 0;	//clear receive buffer
arrtmp = "AT+LKSTT=?\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 1000);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;

						RecvLen = 0;	//clear receive buffer
				arrtmp = "AT+SKCLS=1\r\n";		 //encryption method. WPA1-PSK WPA
						SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
						OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
						if(RecvState()!=0)
							state|= KEY_STATE;


				RecvLen = 0;	//clear receive buffer
arrtmp = "AT+SKCT=1,0,192.168.1.101,55005,50001\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;

		
		RecvLen = 0;	//clear receive buffer
arrtmp = "AT+SKSND=1,6\r\n";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
			state|= KEY_STATE;

		RecvLen = 0;	//clear receive buffer
		arrtmp = "abcdef";		 //encryption method. WPA1-PSK WPA
		SendToUSARTx(USARTx,arrtmp,strlen(arrtmp));
		OSTimeDlyHMSM(0, 0, 0, 100);//1000ms
		if(RecvState()!=0)
		state|= KEY_STATE;

#endif
	}
	return state;

}

s8 wifiInit(void)
{
	Wifi_InitTypeDef  Wifi_InitStruct;
	Wifi_InitStruct.SSID ="summit";

	Wifi_InitStruct.KEY = "abcde";	//key

	Wifi_InitStruct.ObjIP ="192.168.1.101" ;	//Ŀ��IP

	Wifi_InitStruct.ObjPort = "55005"; //Ŀ��˿�

	Wifi_InitStruct.TorU = 1;

	if(WIFI_Config(USART1,&Wifi_InitStruct)== 0)
		return 0;
	else
		return -1;
}





