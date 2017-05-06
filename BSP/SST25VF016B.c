/****************************************Copyright (c)**************************************************                         

**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* ÒÆÖ²±¾Èí¼þ°üÊ±ÐèÒªÐÞ¸ÄÒÔÏÂµÄº¯Êý»òºê */

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
    ////SPI¿Ú³õÊ¼»¯ Set PC10,11,12 as Output push-pull - SCK, MISO and MOSI
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	SPI_Init(SPI1, &SPI_InitStructure);   //¸ù¾ÝSPI_InitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯ÍâÉèSPI2¼Ä´æÆ÷
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   //Ê¹ÄÜGPIOµÄÊ±ÖÓ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);   //Ê¹ÄÜGPIOµÄÊ±ÖÓ
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
    ////SPI¿Ú³õÊ¼»¯ Set PC10,11,12 as Output push-pull - SCK, MISO and MOSI
	
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


/* ÒÔÏÂº¯ÊýÔÚÒÆÖ²Ê±ÎÞÐèÐÞ¸Ä */
/*******************************************************************************
* Function Name  : SSTF016B_RD
* Description    : SST25VF016BµÄ¶Áº¯Êý,¿ÉÑ¡Ôñ¶ÁIDºÍ¶ÁÊý¾Ý²Ù×÷
* Input          : - Dst: Ä¿±êµØÖ·,·¶Î§ 0x0 - MAX_ADDR£¨MAX_ADDR = 0x1FFFFF£©
*                  - RcvBufPt: ½ÓÊÕ»º´æµÄÖ¸Õë
*                  - NByte: Òª¶ÁÈ¡µÄÊý¾Ý×Ö½ÚÊý	
* Output         : ²Ù×÷³É¹¦Ôò·µ»ØOK,Ê§°ÜÔò·µ»ØERROR
* Return         : SST25VF016B ·µ»ØµÄÊý¾Ý
* Attention		 : ÈôÄ³¹¦ÄÜÏÂ,Ä³Ò»Èë¿Ú²ÎÊýÎÞÐ§,¿ÉÔÚ¸ÃÈë¿Ú²ÎÊý´¦ÌîInvalid£¬¸Ã²ÎÊý½«±»ºöÂÔ
*******************************************************************************/
uint8_t SSTF016B_RD(uint32_t Dst, uint8_t* RcvBufPt ,uint32_t NByte)
{
	uint32_t i = 0;
	if ((Dst+NByte > MAX_ADDR)||(NByte == 0))	return (ERROR);	 /*	¼ì²éÈë¿Ú²ÎÊý */
	
    SPI_FLASH_CS_LOW();
	Flash_ReadWriteByte(0x0B); 						/* ·¢ËÍ¶ÁÃüÁî */
	Flash_ReadWriteByte(((Dst & 0xFFFFFF) >> 16));	/* ·¢ËÍµØÖ·ÐÅÏ¢:¸ÃµØÖ·ÓÉ3¸ö×Ö½Ú×é³É	*/
	Flash_ReadWriteByte(((Dst & 0xFFFF) >> 8));
	Flash_ReadWriteByte(Dst & 0xFF);
	Flash_ReadWriteByte(0xFF);						/* ·¢ËÍÒ»¸öÑÆ×Ö½ÚÒÔ¶ÁÈ¡Êý¾Ý	*/
	for (i = 0; i < NByte; i++)		
	{
       RcvBufPt[i] = Flash_ReadWriteByte(0xFF);		
	}
    SPI_FLASH_CS_HIGH();
	return (ENABLE);
}

/*******************************************************************************
* Function Name  : SSTF016B_RdID
* Description    : SST25VF016BµÄ¶ÁIDº¯Êý,¿ÉÑ¡Ôñ¶ÁIDºÍ¶ÁÊý¾Ý²Ù×÷
* Input          : - IDType: IDÀàÐÍ¡£ÓÃ»§¿ÉÔÚJedec_ID,Dev_ID,Manu_IDÈýÕßÀïÑ¡Ôñ
* Output         : - RcvbufPt: ´æ´¢ID±äÁ¿µÄÖ¸Õë
* Return         : ²Ù×÷³É¹¦Ôò·µ»ØOK,Ê§°ÜÔò·µ»ØERROR
* Attention		 : ÈôÌîÈëµÄ²ÎÊý²»·ûºÏÒªÇó£¬Ôò·µ»ØERROR
*******************************************************************************/
uint8_t SSTF016B_RdID(idtype IDType,uint32_t* RcvbufPt)
{
	uint32_t temp = 0;
              
	if (IDType == Jedec_ID)
	{
		SPI_FLASH_CS_LOW();	
				
		Flash_ReadWriteByte(0x9F);		 		         /* ·¢ËÍ¶ÁJEDEC IDÃüÁî(9Fh)	*/
    	        
        temp = (temp | Flash_ReadWriteByte(0x00)) << 8;  /* ½ÓÊÕÊý¾Ý */
		temp = (temp | Flash_ReadWriteByte(0x00)) << 8;	
        temp = (temp | Flash_ReadWriteByte(0x00)); 	     /* ÔÚ±¾ÀýÖÐ,tempµÄÖµÓ¦Îª0xBF2541 */

        SPI_FLASH_CS_HIGH();

		*RcvbufPt = temp;
		return (ENABLE);
	}
	
	if ((IDType == Manu_ID) || (IDType == Dev_ID) )
	{
	    SPI_FLASH_CS_LOW();	
		
		Flash_ReadWriteByte(0x90);				/* ·¢ËÍ¶ÁIDÃüÁî (90h or ABh) */
    	Flash_ReadWriteByte(0x00);				/* ·¢ËÍµØÖ·	*/
		Flash_ReadWriteByte(0x00);				/* ·¢ËÍµØÖ·	*/
		Flash_ReadWriteByte(IDType);		    /* ·¢ËÍµØÖ· - ²»ÊÇ00H¾ÍÊÇ01H */
		temp = Flash_ReadWriteByte(0x00);	    /* ½ÓÊÕ»ñÈ¡µÄÊý¾Ý×Ö½Ú */

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
* Description    : SST25VF016BµÄÐ´º¯Êý£¬¿ÉÐ´1¸öºÍ¶à¸öÊý¾Ýµ½Ö¸¶¨µØÖ·
* Input          : - Dst: Ä¿±êµØÖ·,·¶Î§ 0x0 - MAX_ADDR£¨MAX_ADDR = 0x1FFFFF£©
*                  - SndbufPt: ·¢ËÍ»º´æÇøÖ¸Õë
*                  - NByte: ÒªÐ´µÄÊý¾Ý×Ö½ÚÊý
* Output         : None
* Return         : ²Ù×÷³É¹¦Ôò·µ»ØOK,Ê§°ÜÔò·µ»ØERROR
* Attention		 : ÈôÄ³¹¦ÄÜÏÂ,Ä³Ò»Èë¿Ú²ÎÊýÎÞÐ§,¿ÉÔÚ¸ÃÈë¿Ú²ÎÊý´¦ÌîInvalid£¬¸Ã²ÎÊý½«±»ºöÂÔ
*******************************************************************************/
uint8_t SSTF016B_WR(uint32_t Dst, uint8_t* SndbufPt,uint32_t NByte)
{
	uint8_t temp = 0,StatRgVal = 0;
	uint32_t i = 0;
	if (( (Dst+NByte-1 > MAX_ADDR)||(NByte == 0) ))
	{
		return (ERROR);	 /*	¼ì²éÈë¿Ú²ÎÊý */
	}

	
	SPI_FLASH_CS_LOW();				 
	Flash_ReadWriteByte(0x05);							    /* ·¢ËÍ¶Á×´Ì¬¼Ä´æÆ÷ÃüÁî	*/
	temp = Flash_ReadWriteByte(0xFF);					    /* ±£´æ¶ÁµÃµÄ×´Ì¬¼Ä´æÆ÷Öµ */
	SPI_FLASH_CS_HIGH();								

	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x50);							    /* Ê¹×´Ì¬¼Ä´æÆ÷¿ÉÐ´	*/
	SPI_FLASH_CS_HIGH();	
		
	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x01);							    /* ·¢ËÍÐ´×´Ì¬¼Ä´æÆ÷Ö¸Áî */
	Flash_ReadWriteByte(0);								    /* Çå0BPxÎ»£¬Ê¹FlashÐ¾Æ¬È«Çø¿ÉÐ´ */
	SPI_FLASH_CS_HIGH();			
	
	for(i = 0; i < NByte; i++)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ·¢ËÍÐ´Ê¹ÄÜÃüÁî */
		SPI_FLASH_CS_HIGH();			

		SPI_FLASH_CS_LOW();					
		Flash_ReadWriteByte(0x02); 						    /* ·¢ËÍ×Ö½ÚÊý¾ÝÉÕÐ´ÃüÁî	*/
		Flash_ReadWriteByte((((Dst+i) & 0xFFFFFF) >> 16));  /* ·¢ËÍ3¸ö×Ö½ÚµÄµØÖ·ÐÅÏ¢ */
		Flash_ReadWriteByte((((Dst+i) & 0xFFFF) >> 8));
		Flash_ReadWriteByte((Dst+i) & 0xFF);
		Flash_ReadWriteByte(SndbufPt[i]);					/* ·¢ËÍ±»ÉÕÐ´µÄÊý¾Ý	*/
		SPI_FLASH_CS_HIGH();			

		do
		{
		  	SPI_FLASH_CS_LOW();					 
			Flash_ReadWriteByte(0x05);					    /* ·¢ËÍ¶Á×´Ì¬¼Ä´æÆ÷ÃüÁî */
			StatRgVal = Flash_ReadWriteByte(0xFF);			/* ±£´æ¶ÁµÃµÄ×´Ì¬¼Ä´æÆ÷Öµ */
			SPI_FLASH_CS_HIGH();							
  		}
		while (StatRgVal == 0x03 );					          /* Ò»Ö±µÈ´ý£¬Ö±µ½Ð¾Æ¬¿ÕÏÐ	*/

	}

	SPI_FLASH_CS_LOW();					
	Flash_ReadWriteByte(0x06);							    /* ·¢ËÍÐ´Ê¹ÄÜÃüÁî */
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();					
	Flash_ReadWriteByte(0x50);							    /* Ê¹×´Ì¬¼Ä´æÆ÷¿ÉÐ´	*/
	SPI_FLASH_CS_HIGH();
			
	SPI_FLASH_CS_LOW();				
	Flash_ReadWriteByte(0x01);							    /* ·¢ËÍÐ´×´Ì¬¼Ä´æÆ÷Ö¸Áî */
	Flash_ReadWriteByte(temp);						     	/* »Ö¸´×´Ì¬¼Ä´æÆ÷ÉèÖÃÐÅÏ¢ */
	SPI_FLASH_CS_HIGH();

	return (ENABLE);		
}


/*******************************************************************************
* Function Name  : SSTF016B_Erase
* Description    : ¸earse sector in flash
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
	
	/*  ¼ì²éÈë¿Ú²ÎÊý */
	if ((sec1 > SEC_MAX)||(sec2 > SEC_MAX))	
	{
		return (ERROR);	
	}
   	
   	SPI_FLASH_CS_LOW();			 
	Flash_ReadWriteByte(0x05);								/*send read regiestor command	*/
	temp1 = Flash_ReadWriteByte(0x00);						/* save read data */
	SPI_FLASH_CS_HIGH();								

	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x50);								/* enable write/read´	*/
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();								  	
	Flash_ReadWriteByte(0x01);								/* send write regiestor command	*/
	Flash_ReadWriteByte(0);									/* clear 0BPx tit´ */
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
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ±£´æ¶ÁµÃµÄ×´Ì¬¼Ä´æÆ÷Öµ	*/
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
	
	/* ÈôÁ½¸öÉÈÇøÖ®¼äµÄ¼ä¸ô¹»´ó£¬Ôò²ÉÈ¡8ÉÈÇø²Á³ýËã·¨ */
	while (no_SecsToEr >= 8)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ·¢ËÍÐ´Ê¹ÄÜÃüÁî */
		SPI_FLASH_CS_HIGH();			

	    SecnHdAddr = SEC_SIZE * CurSecToEr;			          /* ¼ÆËãÉÈÇøµÄÆðÊ¼µØÖ· */
	    SPI_FLASH_CS_LOW();	
	    Flash_ReadWriteByte(0x52);							  /* ·¢ËÍ32KB²Á³ýÖ¸Áî */
	    Flash_ReadWriteByte(((SecnHdAddr & 0xFFFFFF) >> 16)); /* ·¢ËÍ3¸ö×Ö½ÚµÄµØÖ·ÐÅÏ¢ */
   		Flash_ReadWriteByte(((SecnHdAddr & 0xFFFF) >> 8));
   		Flash_ReadWriteByte(SecnHdAddr & 0xFF);
  		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* ·¢ËÍ¶Á×´Ì¬¼Ä´æÆ÷ÃüÁî */
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ±£´æ¶ÁµÃµÄ×´Ì¬¼Ä´æÆ÷Öµ	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);					          /* Ò»Ö±µÈ´ý£¬Ö±µ½Ð¾Æ¬¿ÕÏÐ	*/
		CurSecToEr  += 8;
		no_SecsToEr -=  8;
	}
	/* ²ÉÓÃÉÈÇø²Á³ýËã·¨²Á³ýÊ£ÓàµÄÉÈÇø */
	while (no_SecsToEr >= 1)
	{
		SPI_FLASH_CS_LOW();				
		Flash_ReadWriteByte(0x06);						    /* ·¢ËÍÐ´Ê¹ÄÜÃüÁî */
		SPI_FLASH_CS_HIGH();			

	    SecnHdAddr = SEC_SIZE * CurSecToEr;			          /* ¼ÆËãÉÈÇøµÄÆðÊ¼µØÖ· */
	    SPI_FLASH_CS_LOW();	
    	Flash_ReadWriteByte(0x20);							  /* ·¢ËÍÉÈÇø²Á³ýÖ¸Áî */
	    Flash_ReadWriteByte(((SecnHdAddr & 0xFFFFFF) >> 16)); /* ·¢ËÍ3¸ö×Ö½ÚµÄµØÖ·ÐÅÏ¢ */
   		Flash_ReadWriteByte(((SecnHdAddr & 0xFFFF) >> 8));
   		Flash_ReadWriteByte(SecnHdAddr & 0xFF);
  		SPI_FLASH_CS_HIGH();			
		do
		{
		  	SPI_FLASH_CS_LOW();			 
			Flash_ReadWriteByte(0x05);						  /* ·¢ËÍ¶Á×´Ì¬¼Ä´æÆ÷ÃüÁî */
			StatRgVal = Flash_ReadWriteByte(0x00);			  /* ±£´æ¶ÁµÃµÄ×´Ì¬¼Ä´æÆ÷Öµ	*/
			SPI_FLASH_CS_HIGH();								
  		}
		while (StatRgVal == 0x03);					          /* Ò»Ö±µÈ´ý£¬Ö±µ½Ð¾Æ¬¿ÕÏÐ */
		CurSecToEr  += 1;
		no_SecsToEr -=  1;
	}
    /* ²Á³ý½áÊø,»Ö¸´×´Ì¬¼Ä´æÆ÷ÐÅÏ¢ */
	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x06);								  /* ·¢ËÍÐ´Ê¹ÄÜÃüÁî */
	SPI_FLASH_CS_HIGH();			

	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x50);								  /* Ê¹×´Ì¬¼Ä´æÆ÷¿ÉÐ´ */
	SPI_FLASH_CS_HIGH();			
	SPI_FLASH_CS_LOW();			
	Flash_ReadWriteByte(0x01);								  /* ·¢ËÍÐ´×´Ì¬¼Ä´æÆ÷Ö¸Áî */
	Flash_ReadWriteByte(temp1);								  /* »Ö¸´×´Ì¬¼Ä´æÆ÷ÉèÖÃÐÅÏ¢ */
	SPI_FLASH_CS_HIGH();    
	return (ENABLE);
}


/*******************************************************************************
* Function Name  : SPI_FLASH_Test
* Description    : ¶ÁÈ¡SST25VF016B ID 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
0x00BF258E ²é¿´ChipIDµÄÖµÊÇ·ñ0xBF258E
*******************************************************************************/
void SPI_FLASH_Test(void)
{
    uint32_t  ChipID = 0;
  	SSTF016B_RdID(Jedec_ID, &ChipID);                                   /*  µ¥²½ÔËÐÐµ½´Ë´¦Ê±, ÔÚWatchÀï*/

	ChipID &= ~0xff000000;						                        /*  ½ö±£ÁôµÍ24Î»Êý¾Ý            */
	if (ChipID != 0x00BF2541)
	 {											/*  ID²»ÕýÈ·½øÈëËÀÑ­»·          */
           while(1);
    }
}

