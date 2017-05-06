#ifndef __UDS_APP_H
#define __UDS_APP_H


/***************************************************/
/*******	MACRO DEFINATION SECTION             	****/
/***************************************************/
//For APPLICATION
//#define GATEWAY_SUPPORT
#define STORE_DATA_TO_EXTERNAL_FLASH
//#define STORE_DATA_TO_EEPROM

#define BMS_MODE
//#define BATT_MODE

//For BOOTLOADER
//#define BOOT_MODE
//#define STORE_DATA_TO_FLASH 

//#define DS_MODEL
#define SOFT_VERSION_YEAR			14
#define SOFT_VERSION_WEEK			02
#define SOFT_VERSION_PATCH			01

#define HARD_VERSION_YEAR			10
#define HARD_VERSION_WEEK			22
#define HARD_VERSION_PATCH		0x01
#define BOOT_VERSION_YEAR			11
#define BOOT_VERSION_WEEK			1
#define BOOT_VERSION_PATCH		1

//#define TEMP_SENSOR_96
#define TEMP_SENSOR_16


/***************************************************/
/*******	FUNCTIONS DEFINATION SECTION            	****/
/***************************************************/

//	The main function's defination of UDS Diagnostic Layer
#ifdef BOOT_MODE
#else
void  nvUDSTask(void);
#endif

#endif
