/****************************************Copyright (c)**************************************************                         

**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* ��ֲ�������ʱ��Ҫ�޸����µĺ������ */

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : init control pin to the flash
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void SPI_FLASH_Init(void)
#ifdef PROTO_A
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_DeInit(SPI3);
    // Enable SPI and GPIO clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3);        
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3); 
    ////SPI3 Set PC10,11,12 as Output push-pull - SCK, MISO and MOSI
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

	SPI_Init(SPI3, &SPI_InitStructure);   //set SPI3
	
	RCC_AHB1PeriphClockCmd(RCC_FLASH_CSN, ENABLE);   //enable CS GPIO clock
    GPIO_InitStructure.GPIO_Pin = FLASH_CSN;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

//	SPI_FLASH_CS_HIGH();
	/* SPI2 Config -------------------------------------------------------------*/ 								  
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI3, &SPI_InitStructure);
	/* Enable SPI3 */ 
	SPI_Cmd(SPI3, ENABLE); 
}
#endif
#ifdef EVAL_BOARD_V
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_DeInit(SPI1);
    // Enable SPI and GPIO clocks
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);        
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 
    ////SPI�ڳ�ʼ�� Set PC10,11,12 as Output push-pull - SCK, MISO and MOSI
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	SPI_Init(SPI1, &SPI_InitStructure);   //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   //ʹ��GPIO��ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);   //ʹ��GPIO��ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


//	SPI_FLASH_CS_HIGH();
	/* SPI2 Config -------------------------------------------------------------*/ 								  
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1 */ 
	SPI_Cmd(SPI1, ENABLE); 
}
#endif
#ifdef EVAL_BOARD_Z
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_DeInit(SPI3);
    // Enable SPI and GPIO clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SPI3);        
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SPI3); 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI3); 
    ////SPI�ڳ�ʼ�� Set PC10,11,12 as Output push-pull - SCK, MISO and MOSI
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	SPI_Init(SPI3, &SPI_InitStructure);   //init SPI3
	
	RCC_AHB1PeriphClockCmd(RCC_FLASH_CSN, ENABLE);   //init CS  GPIO clock
    GPIO_InitStructure.GPIO_Pin = FLASH_CSN;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

//	SPI_FLASH_CS_HIGH();
	/* SPI2 Config -------------------------------------------------------------*/ 								  
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI3, &SPI_InitStructure);
	/* Enable SPI3 */ 
	SPI_Cmd(SPI3, ENABLE); 
}

#endif
/*******************************************************************************
* Function Name  : Flash_ReadWriteByte
* Description    :through SPI send one byte to SST25VF016B
* Input          : - data: send data
* Output         : None
* Return         : SST25VF016B return data
* Attention		 : None
*******************************************************************************/
uint8_t Flash_ReadWriteByte(uint8_t data)		
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI3 peripheral */
  SPI_I2S_SendData(FLASH_SPI, data);
    
  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(FLASH_SPI);                                              
}


/* ���º�������ֲʱ�����޸� */
/*******************************************************************************
* Function Name  : SSTF016B_RD
* Description    : SST25VF016B�Ķ�����,��ѡ���ID�Ͷ����ݲ���
* Input          : - Dst: Ŀ���ַ,��Χ 0x0 - MAX_ADDR��MAX_ADDR = 0x1FFFFF��
*                  - RcvBufPt: ���ջ����ָ��
*                  - NByte: Ҫ��ȡ�������ֽ���	
* Output         : �����ɹ��򷵻�OK,ʧ���򷵻�ERROR
* Return         : SST25VF016B ���ص�����
* Attention		 : ��ĳ������,ĳһ��ڲ�����Ч,���ڸ���ڲ�������Invalid���ò�����������
*******************************************************************************/
uint8_t SSTF016B_RD(uint32_t Dst, uint8_t* RcvBufPt ,uint32_t NByte)
{
	uint32_t i = 0;
	if ((Dst+NByte > MAX_ADDR)||(NByte == 0))	return (ERROR);	 /*	�����ڲ��� */
	
    SPI_FLASH_CS_LOW();
	Flash_ReadWriteByte(0x0B); 						/* ���Ͷ����� */
	Flash_ReadWriteByte(((Dst & 0xFFFFFF) >> 16));	/* ���͵�ַ��Ϣ:�õ�ַ��3���ֽ����	*/
	Flash_ReadWriteByte(((Dst & 0xFFFF) >> 8));
	Flash_ReadWriteByte(Dst & 0xFF);
	Flash_ReadWriteByte(0xFF);						/* ����һ�����ֽ��Զ�ȡ����	*/
	for (i = 0; i < NByte; i++)		
	{
       RcvBufPt[i] = Flash_ReadWriteByte(0xFF);		
	}
    SPI_FLASH_CS_HIGH();
	return (ENABLE);
}

/*******************************************************************************
* Function Name  : SSTF016B_RdID
* Description    : SST25VF016B�Ķ�ID����,��ѡ���ID�Ͷ����ݲ���
* Input          : - IDType: ID���͡��û�����Jedec_ID,Dev_ID,Manu_ID������ѡ��
* Output         : - RcvbufPt: �洢ID������ָ��
* Return         : �����ɹ��򷵻�OK,ʧ���򷵻�ERROR
* Attention		 : ������Ĳ���������Ҫ���򷵻�ERROR
*******************************************************************************/
uint8_t SSTF016B_RdID(idtype IDType,uint32_t* RcvbufPt)
{
	uint32_t temp = 0;
              
	if (IDType == Jedec_ID)
	{
		SPI_FLASH_CS_LOW();	
				
		Flash_ReadWriteByte(0x9F);		 		         /* ���Ͷ�JEDEC ID����(9Fh)	*/
    	        
        temp = (temp | Flash_ReadWriteByte(0x00)) << 8;  /* �������� */
		temp = (temp | Flash_ReadWriteByte(0x00)) << 8;	
        temp = (temp | Flash_ReadWriteByte(0x00)); 	     /* �ڱ�����,temp��ֵӦΪ0xBF2541 */

        SPI_FLASH_CS_HIGH();

		*RcvbufPt = temp;
		return (ENABLE);
	}
	
	if ((IDType == Manu_ID) || (IDType == Dev_ID) )
	{
	    SPI_FLASH_CS_LOW();	
		
		Flash_ReadWriteByte(0x90);				/* ���Ͷ�ID���� (90h or ABh) */
    	Flash_ReadWriteByte(0x00);				/* ���͵�ַ	*/
		Flash_ReadWriteByte(0x00);				/* ���͵�ַ	*/
		Flash_ReadWriteByte(IDType);		    /* ���͵�ַ - ����00H����01H */
		temp = Flash_ReadWriteByte(0x00);	    /* ���ջ�ȡ�������ֽ� */

        SPI_FLASH_CS_HIGH();

		*RcvbufPt = temp;
		return (ENABLE);
	}
	else
	{
		return (ERROR);	
	}
}


/*******************************************************************************
* Function Name  : SSTF016B_WR
* Description    : SST25VF016B��д��������д1���Ͷ�����ݵ�ָ����ַ
* Input          : - Dst: Ŀ���ַ,��Χ 0x0 - MAX_ADDR��MAX_ADDR = 0x1FFFFF��
*                  - SndbufPt: ���ͻ�����ָ��
*                  - NByte: Ҫд�������ֽ���
* Output         : None
* Return         : �����ɹ��򷵻�OK,ʧ���򷵻�ERROR
* Attention		 : ��ĳ������,ĳһ��ڲ�����Ч,���ڸ���ڲ�������Invalid���ò�����������
*******************************************************************************/
uint8_t SSTF016B_WR(uint32_t Dst, uint8_t* SndbufPt,uint32_t NByte)
{
	uint8_t temp = 0,StatRgVal = 0;
	uint32_t i = 0;
	if (( (Dst+NByte-1 > MAX_ADDR)||(NByte == 0) ))
	{
		return (ERROR);	 /*	�����ڲ��� */
	}

	
	SPI_FLASH_CS_LOW();				 
	Flash_ReadWriteByte(0x05);							    /* ���Ͷ�״̬�Ĵ�������	*/
	temp = Flash_ReadWriteByte(0xFF);					    /* ������õ�״̬�Ĵ���ֵ */
	SPI_FLASH_CS_HIGH();								

	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x50);							    /* ʹ״̬�Ĵ�����д	*/
	SPI_FLASH_CS_HIGH();	
		
	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x01);							    /* ����д״̬�Ĵ���ָ�� */
	Flash_ReadWriteByte(0);								    /* ��0BPxλ��ʹFlashоƬȫ����д */
	SPI_FLASH_CS_HIGH();			
	
	for(i = 0; i < NByte; i++)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ����дʹ������ */
		SPI_FLASH_CS_HIGH();			

		SPI_FLASH_CS_LOW();					
		Flash_ReadWriteByte(0x02); 						    /* �����ֽ�������д����	*/
		Flash_ReadWriteByte((((Dst+i) & 0xFFFFFF) >> 16));  /* ����3���ֽڵĵ�ַ��Ϣ */
		Flash_ReadWriteByte((((Dst+i) & 0xFFFF) >> 8));
		Flash_ReadWriteByte((Dst+i) & 0xFF);
		Flash_ReadWriteByte(SndbufPt[i]);					/* ���ͱ���д������	*/
		SPI_FLASH_CS_HIGH();			

		do
		{
		  	SPI_FLASH_CS_LOW();					 
			Flash_ReadWriteByte(0x05);					    /* ���Ͷ�״̬�Ĵ������� */
			StatRgVal = Flash_ReadWriteByte(0xFF);			/* ������õ�״̬�Ĵ���ֵ */
			SPI_FLASH_CS_HIGH();							
  		}
		while (StatRgVal == 0x03 );					          /* һֱ�ȴ���ֱ��оƬ����	*/

	}

	SPI_FLASH_CS_LOW();					
	Flash_ReadWriteByte(0x06);							    /* ����дʹ������ */
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();					
	Flash_ReadWriteByte(0x50);							    /* ʹ״̬�Ĵ�����д	*/
	SPI_FLASH_CS_HIGH();
			
	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x01);							    /* ����д״̬�Ĵ���ָ�� */
	Flash_ReadWriteByte(temp);						     	/* �ָ�״̬�Ĵ���������Ϣ */
	SPI_FLASH_CS_HIGH();

	return (ENABLE);		
}


/*******************************************************************************
* Function Name  : SSTF016B_Erase
* Description    : �earse sector in flash
* Input          : - sec1: start sector nu (0~511)
*                  - sec2: end sector no(0~511)
* Output         : None
* Return         : OK- success, FAIL-fail
* Attention		 : None
*******************************************************************************/
uint8_t SSTF016B_Erase(uint32_t sec1, uint32_t sec2)
{
	uint8_t  temp1 = 0,temp2 = 0,StatRgVal = 0;
    uint32_t SecnHdAddr = 0;	  			
	uint32_t no_SecsToEr = 0;				   			    /* sector number to be earsed */
	uint32_t CurSecToEr = 0;	  						    /* current sector to be earsed */
	
	/*  �����ڲ��� */
	if ((sec1 > SEC_MAX)||(sec2 > SEC_MAX))	
	{
		return (ERROR);	
	}
   	
   	SPI_FLASH_CS_LOW();			 
	Flash_ReadWriteByte(0x05);								/*send read regiestor command	*/
	temp1 = Flash_ReadWriteByte(0x00);						/* save read data */
	SPI_FLASH_CS_HIGH();								

	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x50);								/* enable write/read�	*/
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();								  	
	Flash_ReadWriteByte(0x01);								/* send write regiestor command	*/
	Flash_ReadWriteByte(0);									/* clear 0BPx tit� */
	SPI_FLASH_CS_HIGH();
	
	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x06);								/*send write/read enable command */
	SPI_FLASH_CS_HIGH();			

	/* adjust start sec and end sec */
	if (sec1 > sec2)
	{
	   temp2 = sec1;
	   sec1  = sec2;
	   sec2  = temp2;
	} 
	/*earse singal sector */
	if (sec1 == sec2)	
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* send write enable command */
		SPI_FLASH_CS_HIGH();			

	    SecnHdAddr = SEC_SIZE * sec1;				          /* calculate start address	*/
	    SPI_FLASH_CS_LOW();	
    	Flash_ReadWriteByte(0x20);							  /* send earse command */
	    Flash_ReadWriteByte(((SecnHdAddr & 0xFFFFFF) >> 16)); /* send 3 byte infromation to set start address */
   		Flash_ReadWriteByte(((SecnHdAddr & 0xFFFF) >> 8));
   		Flash_ReadWriteByte(SecnHdAddr & 0xFF);
  		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* send read regiestor command*/
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ������õ�״̬�Ĵ���ֵ	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);				              /* wait untill chip idle	*/
		return (ENABLE);			
	}
	
    /* fast earse based on start sector and end sector */	
	
	if (sec2 - sec1 == SEC_MAX)	
	{
		SPI_FLASH_CS_LOW();			
		Flash_ReadWriteByte(0x60);							  /* send earse command(60h or C7h) */
		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* send read regiestor command */
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* save read data	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);					          /* wait untill chip idle	*/
   		return (ENABLE);
	}
	
	no_SecsToEr = sec2 - sec1 +1;					          /* get sector number to be earsed */
	CurSecToEr  = sec1;								          /* set start sector	*/
	
	/* ����������֮��ļ���������ȡ8���������㷨 */
	while (no_SecsToEr >= 8)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ����дʹ������ */
		SPI_FLASH_CS_HIGH();			

	    SecnHdAddr = SEC_SIZE * CurSecToEr;			          /* ������������ʼ��ַ */
	    SPI_FLASH_CS_LOW();	
	    Flash_ReadWriteByte(0x52);							  /* ����32KB����ָ�� */
	    Flash_ReadWriteByte(((SecnHdAddr & 0xFFFFFF) >> 16)); /* ����3���ֽڵĵ�ַ��Ϣ */
   		Flash_ReadWriteByte(((SecnHdAddr & 0xFFFF) >> 8));
   		Flash_ReadWriteByte(SecnHdAddr & 0xFF);
  		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* ���Ͷ�״̬�Ĵ������� */
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ������õ�״̬�Ĵ���ֵ	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);					          /* һֱ�ȴ���ֱ��оƬ����	*/
		CurSecToEr  += 8;
		no_SecsToEr -=  8;
	}
	/* �������������㷨����ʣ������� */
	while (no_SecsToEr >= 1)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ����дʹ������ */
		SPI_FLASH_CS_HIGH();			

	    SecnHdAddr = SEC_SIZE * CurSecToEr;			          /* ������������ʼ��ַ */
	    SPI_FLASH_CS_LOW();	
    	Flash_ReadWriteByte(0x20);							  /* ������������ָ�� */
	    Flash_ReadWriteByte(((SecnHdAddr & 0xFFFFFF) >> 16)); /* ����3���ֽڵĵ�ַ��Ϣ */
   		Flash_ReadWriteByte(((SecnHdAddr & 0xFFFF) >> 8));
   		Flash_ReadWriteByte(SecnHdAddr & 0xFF);
  		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* ���Ͷ�״̬�Ĵ������� */
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ������õ�״̬�Ĵ���ֵ	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);					          /* һֱ�ȴ���ֱ��оƬ���� */
		CurSecToEr  += 1;
		no_SecsToEr -=  1;
	}
    /* ��������,�ָ�״̬�Ĵ�����Ϣ */
	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x06);								  /* ����дʹ������ */
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x50);								  /* ʹ״̬�Ĵ�����д */
	SPI_FLASH_CS_HIGH();			
	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x01);								  /* ����д״̬�Ĵ���ָ�� */
	Flash_ReadWriteByte(temp1);								  /* �ָ�״̬�Ĵ���������Ϣ */
	SPI_FLASH_CS_HIGH();    
	return (ENABLE);
}


/*******************************************************************************
* Function Name  : SPI_FLASH_Test
* Description    : ��ȡSST25VF016B ID 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
0x00BF258E �鿴ChipID��ֵ�Ƿ�0xBF258E
*******************************************************************************/
void SPI_FLASH_Test(void)
{
    uint32_t  ChipID = 0;
  	SSTF016B_RdID(Jedec_ID, &ChipID);                                   /*  �������е��˴�ʱ, ��Watch��*/

	ChipID &= ~0xff000000;						                        /*  ��������24λ����            */
	if (ChipID != 0x00BF2541)
	 {											/*  ID����ȷ������ѭ��          */
           while(1);
    }
}

