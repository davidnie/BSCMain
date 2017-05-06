
/* Includes ------------------------------------------------------------------*/
#include "main.h"


/*******************************************************************************
* Function Name  : I2C_Configuration
* Description    : EEPROM pin configuation
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void I2C_Configuration(void)
{
   I2C_InitTypeDef  I2C_InitStructure;
   GPIO_InitTypeDef  GPIO_InitStructure; 


	/* PB6-I2C1_SCL PB7-I2C1_SDA*/  
#ifdef PROTO_A
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//enable port clock
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);    
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);///config PB6 and PB7 for I2C communication
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;     
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //config as aux function 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;// open drain  
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;// enable pull up
	GPIO_Init(GPIOB, &GPIO_InitStructure);//initlize port B

#endif
#ifdef EVAL_BOARD_Z
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//enable port clock
//	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);    
//	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);///config PB6 and PB7 for I2C communication

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_0; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //config as aux function 
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //config as aux function 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;// open drain  
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;// enable pull up
	GPIO_Init(GPIOB, &GPIO_InitStructure);//initlize port B
#endif
#ifdef EVAL_BOARD_V
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//enable port clock
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);    
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);///config PB6 and PB7 for I2C communication
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;     
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //config as aux function 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;// open drain  
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;// enable pull up
	GPIO_Init(GPIOB, &GPIO_InitStructure);//initlize port B

#endif

/** Enable I2C1 clock */   
#if 1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);  /** CODEC_I2C peripheral configuration */  
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;   
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;  
	I2C_InitStructure.I2C_OwnAddress1 = 0x0A;  
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;   
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  
	I2C_InitStructure.I2C_ClockSpeed = 1000000;  /** Enable the I2C peripheral */  
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_Cmd(I2C1, ENABLE);
#endif
}
 
/*******************************************************************************
* Function Name  : I2C_delay
* Description    : time delay
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void I2C_delay(uint16_t cnt)
{
	while(cnt--);
}

/*******************************************************************************
* Function Name  : I2C_AcknowledgePolling
* Description    : wait for get I2C bus control 
* Input          : - I2Cx:I2C register base address
*                  - I2C_Addr: I2C device address
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void I2C_AcknowledgePolling(I2C_TypeDef *I2Cx,uint8_t I2C_Addr)
{
  vu16 SR1_Tmp;
  do
  {   
    I2C_GenerateSTART(I2Cx, ENABLE); /*send start bit*/
    /*read ¨¢SR1*/
    SR1_Tmp = I2C_ReadRegister(I2Cx, I2C_Register_SR1);
    /*device address*/
#ifdef AT24C01A

	I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Transmitter);
#else

	I2C_Send7bitAddress(I2Cx, 0, I2C_Direction_Transmitter);
#endif

  }while(!(I2C_ReadRegister(I2Cx, I2C_Register_SR1) & 0x0002));
  
  I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
    
  I2C_GenerateSTOP(I2Cx, ENABLE);  /*send stop bit*/  
}


/*******************************************************************************
* Function Name  : I2C_Read
* Description    : read muliti byte from 
* Input          : - I2Cx:I2C regiester address
*                  - I2C_Addr: slave device address
*                  - addr: read start address
*                  - *buf: save start address
*                  - num: read number
* Output         : None
* Return         : result 
* Attention		 : None
*******************************************************************************/
uint8_t I2C_Read(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t addr,uint8_t *buf,uint16_t num)
{
    if(num==0)
	return 1;
	
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
		
	/*enable I2C req/ack mode*/
	I2C_AcknowledgeConfig(I2Cx, ENABLE);


	/* send start bit */
    I2C_GenerateSTART(I2Cx, ENABLE);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,master mode*/

#ifdef AT24C01A	
    /*send slave device address*/
    I2C_Send7bitAddress(I2Cx,  I2C_Addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/*send read start address*/
	I2C_SendData(I2Cx, addr);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));/*wait*/
		
	/*start*/
	I2C_GenerateSTART(I2Cx, ENABLE);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
	
	/*send read command*/
	I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		
#else	
	/*send read command to 24C01*/
	I2C_Send7bitAddress(I2Cx, addr<<1, I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
#endif
	
    while (num)
    {
		if(num==1)
		{
     		I2C_AcknowledgeConfig(I2Cx, DISABLE);	/* ¡Á?o¨®¨°???o¨®¨°a1?¡À?¨®|¡äe¦Ì? */
    		I2C_GenerateSTOP(I2Cx, ENABLE);			/* ¡¤¡é?¨ª¨ª¡ê?1?? */
		}
	    
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));  /* EV7 */
	    *buf = I2C_ReceiveData(I2Cx);
	    buf++;
	    /* Decrement the read bytes counter */
	    num--;
    }
	/* ?¨´¡ä??¨ºD¨ª¨®|¡äe?¡ê¨º? */
	I2C_AcknowledgeConfig(I2Cx, ENABLE);

	return 0;
}	

/*******************************************************************************
* Function Name  : I2C_WriteOneByte
* Description    : ¨ª¡§1y???¡§I2C?¨®?¨²D¡ä¨¨?¨°???¡Á??¨²¨ºy?Y
* Input          : - I2Cx:I2C??¡ä??¡Â?¨´?¡¤
*                  - I2C_Addr:¡ä¨®?¡Â?t¦Ì??¡¤
*                  - addr:?¡èD¡ä¨¨?¡Á??¨²¦Ì??¡¤
*                  - value:D¡ä¨¨?¨ºy?Y
* Output         : None
* Return         : 3¨¦1|¡¤¦Ì??0
* Attention		 : None
*******************************************************************************/
uint8_t I2C_WriteOneByte(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t addr,uint8_t value)
{
    /* ?e¨º??? */
  	I2C_GenerateSTART(I2Cx, ENABLE);
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));  

#ifdef AT24C01A
  	/* ¡¤¡é?¨ª?¡Â?t¦Ì??¡¤(D¡ä)*/
  	I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Transmitter);
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
  	/*¡¤¡é?¨ª¦Ì??¡¤*/
  	I2C_SendData(I2Cx, addr);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

#else	
	I2C_Send7bitAddress(I2Cx, addr<<1, I2C_Direction_Transmitter);
 	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
#endif

	/* D¡ä¨°???¡Á??¨²*/
  	I2C_SendData(I2Cx, value); 
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
  	/* ¨ª¡ê?1??*/
  	I2C_GenerateSTOP(I2Cx, ENABLE);
  
  	I2C_AcknowledgePolling(I2Cx,I2C_Addr);

	I2C_delay(1000);

	return 0;
}


/*******************************************************************************
* Function Name  : I2C_Write
* Description    : write multi bytes to EEPROM through I2C interface
* Input          : - I2Cx:I2C base address
*                  - I2C_Addr:slave device address
*                  - addr:write to 
*                  - *buf: bytes start address
*                  - num:write number
* Output         : None
* Return         : result
* Attention		 : None
*******************************************************************************/
uint8_t I2C_Write(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t addr,uint8_t *buf,uint16_t num)
{
	uint8_t err=0;
	
	while(num--)
	{
		if(I2C_WriteOneByte(I2Cx, I2C_Addr,addr++,*buf++))
		{
			err++;
		}
	}
	if(err)
		return 1;
	else 
		return 0;	
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
