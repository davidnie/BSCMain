/***********************************************************************
***********************************************************************/
#include "main.h"
#include "task_main.h"
	OS_STK Stk_Task_EFlash[400];
	void SPI_Test(void);
void NVM_BMWrite(void);
void NVM_SysinfoWrite(void);
void NVM_BPInfoWrite(void);
void NVM_AllInfoWrite(void);
void NVM_sysCfgWrite(void);

/***********************************************************************
***********************************************************************/
void Task_EFlash(void *pdata)
{		
UNS16 i;
	SPI_FLASH_Init();
	LED_OFF();

	while(1)
	{  
		SPI_FLASH_CS_HIGH();	
		LED_ON();
		SPI_FLASH_CS_LOW(); 				
		LED_OFF();
		//Flash_ReadWriteByte(0x06);								/* ??????? */
		NVM_BMWrite();

		NVM_SysinfoWrite();
		NVM_BPInfoWrite();

		SPI_FLASH_CS_HIGH();			
	//SPI_Test();

		OSTimeDlyHMSM(0, 0, 0, 10000);//suspend at here for 10s
	}
}
void SPI_Test(void)
{
	unsigned int i = 0;
	unsigned int flash_start_addr = 0x1000;//FLASH???
	unsigned char WriteBuffer[256];
	unsigned char ReadBuffer[256];
	for(i = 0;i < 256;i ++)//????????
	{
		WriteBuffer[i] = i;
	}
	SPI_FLASH_Test();//flash??,????????
	/* ??????? */
	SSTF016B_Erase( flash_start_addr/SEC_SIZE , ( flash_start_addr + sizeof(WriteBuffer) )/SEC_SIZE  );
	df_write_open( flash_start_addr );    /* ????????? */
	df_write( (unsigned char*)WriteBuffer,256); //?WriteBuffer???256?????flash
	df_write_close();							//??? 
	df_read_open(flash_start_addr);    /* ??????? */ 
	df_read(ReadBuffer, 256); //??flash??256?????ReadBuffer?????
}
void NVM_BMWrite(void){
	
	SSTF016B_Erase( ADDR_BM_IN_NVM/SEC_SIZE , ( ADDR_BM_IN_NVM + sizeof(strInfo) )/SEC_SIZE  );
	df_write_open(ADDR_BM_IN_NVM);
//	df_write((UNS8 *)&strInfo,256);
	df_write((UNS8 *)&strInfo,sizeof(strInfo));
	df_read_open(ADDR_BM_IN_NVM);    /* set read point*/ 
	df_read((uint8_t *)&strInfo, sizeof(strInfo)); //read 256 bytes and set to ReadBuffer?????
}
void NVM_SysinfoWrite(void){
	
	SSTF016B_Erase( ADDR_SYS_IN_NVM/SEC_SIZE , ( ADDR_SYS_IN_NVM + sizeof(sysInfo) )/SEC_SIZE  );
	df_write_open(ADDR_SYS_IN_NVM);
	df_write((UNS8 *)&sysInfo,sizeof(sysInfo));
	df_read_open(ADDR_SYS_IN_NVM);    /* set read point*/ 
	df_read((uint8_t *)&sysInfo, sizeof(sysInfo)); //read 256 bytes and set to ReadBuffer?????
}
void NVM_BPInfoWrite(void){
	UNS16 bmsize;

	SSTF016B_Erase( ADDR_BP_IN_NVM/SEC_SIZE , ( ADDR_BP_IN_NVM + sizeof(sysInfo) )/SEC_SIZE );
	df_write_open(ADDR_BP_IN_NVM);
	df_write((UNS8 *)&bpInfo,sizeof(bpInfo));
	df_read_open(ADDR_BP_IN_NVM);    /* set read point*/ 
	df_read((uint8_t *)&bpInfo, sizeof(bpInfo)); //read 256 bytes and set to ReadBuffer?????
}
void NVM_sysCfgWrite(void){
	UNS16 cfgsize;

	SSTF016B_Erase( ADDR_CONFIG_IN_NVM/SEC_SIZE , ( ADDR_CONFIG_IN_NVM + sizeof(gSysCfg) )/SEC_SIZE );
	df_write_open(ADDR_CONFIG_IN_NVM);
	df_write((UNS8 *)&gSysCfg,sizeof(gSysCfg));
	df_read_open(ADDR_CONFIG_IN_NVM);    /* set read point*/ 
	df_read((uint8_t *)&gSysCfg, sizeof(gSysCfg)); //read 256 bytes and set to ReadBuffer?????
}

void NVM_AllInfoWrite(void){
	NVM_BPInfoWrite();
	NVM_BMWrite();
	NVM_SysinfoWrite();
	NVM_sysCfgWrite();
	
}
