#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "stdint.h"

//#define EVAL_BOARD_Z
//#define EVAL_BOARD_V
#define BMS_SW_ID 0x0084
#define BMS_HW_ID 0x0022
#define DEFAULT_MOD_SN 0x1

//#define PRO_TKS
#define PRO_FORK



#define PROTO_A
#ifdef PROTO_A
	#define FLASH_CSN      	GPIO_Pin_9
	#define GPIO_FLASH_CSN  GPIOC
	#define RCC_FLASH_CSN  	RCC_AHB1Periph_GPIOE
	#define FLASH_SPI      	SPI3
#endif
#ifdef EVAL_BOARD_Z
	#define FLASH_CSN      	GPIO_Pin_4
	#define GPIO_FLASH_CSN  GPIOE
	#define RCC_FLASH_CSN  	RCC_AHB1Periph_GPIOE
	#define FLASH_SPI      	SPI3
#endif

#ifdef EVAL_BOARD_V
	#define FLASH_CSN      	GPIO_Pin_0
	#define GPIO_FLASH_CSN  GPIOB
	#define RCC_FLASH_CSN  	RCC_AHB1Periph_GPIOB

	#define FLASH_SPI      	SPI1
#endif
//SPI CS 
#define SPI_FLASH_CS_LOW()     {GPIO_ResetBits(GPIO_FLASH_CSN, FLASH_CSN);}
#define SPI_FLASH_CS_HIGH()     {GPIO_SetBits(GPIO_FLASH_CSN, FLASH_CSN);}
#define LED_OFF()     {GPIO_SetBits(GPIOC,GPIO_Pin_6);}
#define LED_ON()    {GPIO_ResetBits(GPIOC,GPIO_Pin_6);}

#define ADDR_CONFIG_IN_NVM 0x1000
#define ADDR_SYS_IN_NVM 0x2000
#define ADDR_BM_IN_NVM 0x3000
#define ADDR_BP_IN_NVM 0x4000
#define ADDR_LOG_IN_NVM 0x20000
#define ms10 0x1

#define SC_TORO_SW_ID 0x84
#define SC_TORO_HW_ID 0x22
#define BP_FW_VER_MAJOR 0x00
#define BP_FW_VER_MINOR 0x1
#define BP_FW_VER_PATCH 0x4



#define NUM_BMU_TOTAL  8
#define NUM_STR		8
#define NUM_BUM_PER_STR 1
#define NUM_CV_IN_BMU	10
#define NUM_CT_IN_BMU	3
#define CAP_CELL_FULL	44	//AHr
#define VLT_STR_NORM    (uint32_t)3650*NUM_CV_IN_BMU*NUM_BUM_PER_STR
#define VLT_STR_MIN    (uint32_t)3000*NUM_CV_IN_BMU*NUM_BUM_PER_STR
#define VLT_STR_MAX    (uint32_t)4150*NUM_CV_IN_BMU*NUM_BUM_PER_STR
#define NUM_ACTIVE_STR_MIN  6	//minimum active string number to maintain the system run normal
/*calibration parameter*/
//I=(Vmesu-CAL_CS_OFFSET)*CAL_CS_AMP
#define CAL_CS_OFFSET 0
#define CAL_CS_AMP 33

/* MAC ADcontinue;continue;DRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0   2
#define MAC_ADDR1   0
#define MAC_ADDR2   0
#define MAC_ADDR3   0
#define MAC_ADDR4   0
#define MAC_ADDR5   0
 
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   10
#define IP_ADDR1   10
#define IP_ADDR2   10
#define IP_ADDR3   253
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1  

#define MDTCP_PORT   502
#define HTTP_PORT   80

/*over voltage */
#define THD_V_FLT_SET_H	4200
#define THD_TM_V_FLT_SET_H	3000	//ms
#define THD_V_FLT_CLR_H	3900
#define THD_TM_V_FLT_CLR_H	3000	//ms
#define THD_V_ALM_SET_H	4150
#define THD_TM_V_ALM_SET_H	3000	//ms
#define THD_V_ALM_CLR_H	3900
#define THD_TM_V_ALM_CLR_H	3000	//ms
#define THD_V_WRN_SET_H	4100
#define THD_TM_V_WRN_SET_H	3000	//ms
#define THD_V_WRN_CLR_H	3900
#define THD_TM_V_WRN_CLR_H	3000	//ms
/*under voltage */
#define THD_V_FLT_SET_L	2900
#define THD_TM_V_FLT_SET_L	3000	//ms
#define THD_V_FLT_CLR_L	3100
#define THD_TM_V_FLT_CLR_L	3000	//ms
#define THD_V_ALM_SET_L	3050
#define THD_TM_V_ALM_SET_L	3000	//ms
#define THD_V_ALM_CLR_L	3150
#define THD_TM_V_ALM_CLR_L	3000	//ms
#define THD_V_WRN_SET_L	3100
#define THD_TM_V_WRN_SET_L	3000	//ms
#define THD_V_WRN_CLR_L	3150
#define THD_TM_V_WRN_CLR_L	3000	//ms

/*Over temperature */
#define THD_T_FLT_SET_H	55			/ C
#define THD_TM_T_FLT_SET_H	10000	//ms
#define THD_T_FLT_CLR_H	0xFFFF
#define THD_TM_T_FLT_CLR_H	10000	//ms
#define THD_T_ALM_SET_H	50
#define THD_TM_T_ALM_SET_H	3000	//ms
#define THD_T_ALM_CLR_H	45
#define THD_TM_T_ALM_CLR_H	3000	//ms
#define THD_T_WRN_SET_H	42
#define THD_TM_T_WRN_SET_H	3000	//ms
#define THD_T_WRN_CLR_H	40
#define THD_TM_T_WRN_CLR_H	3000	//ms
/*under temperature */
#define THD_T_FLT_SET_L	-40
#define THD_TM_T_FLT_SET_L	10000	//ms
#define THD_T_FLT_CLR_L	0xFFFF
#define THD_TM_T_FLT_CLR_L	10000	//ms
#define THD_T_ALM_SET_L	-20
#define THD_TM_T_ALM_SET_L	3000	//ms
#define THD_T_ALM_CLR_L	-10
#define THD_TM_T_ALM_CLR_L	3000	//ms
#define THD_T_WRN_SET_L	-15
#define THD_TM_T_WRN_SET_L	3000	//ms
#define THD_T_WRN_CLR_L	-10
#define THD_TM_T_WRN_CLR_L	3000	//ms

/*discharge over current */
#define THD_DOC_FLT_SET	45000	//mA
#define THD_TM_DOC_FLT_SET	1000	//ms
#define THD_DOC_FLT_CLR	0xFFFF
#define THD_TM_DOC_FLT_CLR	0xFFFF	//ms
#define THD_DOC_ALM_SET	40000
#define THD_TM_DOC_ALM_SET	10000	//ms
#define THD_DOC_ALM_CLR	3500
#define THD_TM_DOC_ALM_CLR	3000	//ms
#define THD_DOC_WRN_SET	32000
#define THD_TM_DOC_WRN_SET	3000	//ms
#define THD_DOC_WRN_CLR	30000
#define THD_TM_DOC_WRN_CLR	3000	//ms
/*charge over current */
#define THD_COC_FLT_SET	40000
#define THD_TM_COC_FLT_SET	10000	//ms
#define THD_COC_FLT_CLR	0xFFFF
#define THD_TM_COC_FLT_CLR	0xFFFF	//ms
#define THD_COC_ALM_SET	30000
#define THD_TM_COC_ALM_SET	3000	//ms
#define THD_COC_ALM_CLR	25000
#define THD_TM_COC_ALM_CLR	3000	//ms
#define THD_COC_WRN_SET	28000
#define THD_TM_COC_WRN_SET	3000	//ms
#define THD_COC_WRN_CLR	25000
#define THD_TM_COC_WRN_CLR	3000	//ms

/* save all configureable parameters. can be modified by end user, save in NVM */
typedef struct{
	uint8_t ucNUM_BMU_TOTAL;
uint8_t ucNUM_STR;
uint8_t ucNUM_BUM_PER_STR;
uint8_t ucNUM_CV_IN_BMU;
uint8_t ucNUM_CT_IN_BMU;
uint8_t ucNUM_ACTIVE_STR_MIN;
uint16_t uiCAP_CELL_FULL;	//AHr
	/* MAC ADcontinue;continue;DRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
uint8_t ucMAC_ADDR0;
uint8_t ucMAC_ADDR1;
uint8_t ucMAC_ADDR2;
uint8_t ucMAC_ADDR3;
uint8_t ucMAC_ADDR4;
uint8_t ucMAC_ADDR5;
	
	/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
uint8_t ucIP_ADDR0;
uint8_t ucIP_ADDR1;
uint8_t ucIP_ADDR2;
uint8_t ucIP_ADDR3;
	   
	/*NETMASK*/
uint8_t ucNETMASK_ADDR0;
uint8_t ucNETMASK_ADDR1;
uint8_t ucNETMASK_ADDR2;
uint8_t ucNETMASK_ADDR3;
	
	/*Gateway Address*/
uint8_t ucGW_ADDR0;
uint8_t ucGW_ADDR1;
uint8_t ucGW_ADDR2;
uint8_t ucGW_ADDR3;
	
uint16_t uiMDTCP_PORT;
uint16_t uiHTTP_PORT;
	
	/*over voltage */
uint16_t uiTHD_V_FLT_SET_H;	//mv
uint16_t uiTHD_TM_V_FLT_SET_H;	//ms
uint16_t uiTHD_V_FLT_CLR_H;
uint16_t uiTHD_TM_V_FLT_CLR_H;	//ms
uint16_t uiTHD_V_ALM_SET_H;
uint16_t uiTHD_TM_V_ALM_SET_H;	//ms
uint16_t uiTHD_V_ALM_CLR_H;
uint16_t uiTHD_TM_V_ALM_CLR_H;	//ms
uint16_t uiTHD_V_WRN_SET_H;
uint16_t uiTHD_TM_V_WRN_SET_H;	//ms
uint16_t uiTHD_V_WRN_CLR_H;
uint16_t uiTHD_TM_V_WRN_CLR_H;	//ms
	/*under voltage */
uint16_t uiTHD_V_FLT_SET_L;
uint16_t uiTHD_TM_V_FLT_SET_L;	//ms
uint16_t uiTHD_V_FLT_CLR_L;
uint16_t uiTHD_TM_V_FLT_CLR_L;	//ms
uint16_t uiCAL_CS_OFFSET;
uint16_t uiCAL_CS_AMP;

uint16_t cfgCRC;
}SYS_CONFIG_CONST_ST;
extern SYS_CONFIG_CONST_ST gSysCfg;
extern void SCInit(void);
#endif
