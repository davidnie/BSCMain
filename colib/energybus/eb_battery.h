/*
* eb_battery.h - contains defines for battery
*
* Copyright (c) 2012-2015 emtas GmbH
*
*-------------------------------------------------------------------
* $Id: eb_battery.h 8131 2015-01-13 07:11:11Z boe $

*-------------------------------------------------------------------
*
* Changelog:
*
*
*/

/**
* \brief defines for battery devices
*
* \file eb_battery.h - contains defines for battery
*/

#ifndef EB_BATTERY_H
#define EB_BATTERY_H 1

#include <co_datatype.h>
#include <eb_fct.h>



/* datatypes */

/* Battery states */
#define EB_BAT_STATE_DO_NOT_ATTACH		((UNSIGNED16)1u << 6)
#define EB_BAT_STATE_READY_TO_ATTACH	((UNSIGNED16)2u << 6)
#define EB_BAT_STATE_NORMAL_OPERATION	((UNSIGNED16)3u << 6)
#define EB_BAT_STATE_PLEASE_DETACH		((UNSIGNED16)4u << 6)
#define EB_BAT_STATE_ERROR				((UNSIGNED16)6u << 6)
#define EB_BAT_STATE_MASK				((UNSIGNED16)0x07u << 6)

//#define EB_BAT_CTRL2_READY_TO_ATTACH 	((UNSIGNED16)2u << 8)
#define EB_BAT_CTRL_DONT_ATTACH	 		((UNSIGNED16)3u << 8)
#define EB_BAT_CTRL_NORMAL_OPERATION	((UNSIGNED16)4u << 8)
//#define EB_BAT_CTRL5_ERROR			 	((UNSIGNED16)5u << 8)
//#define EB_BAT_CTRL6_PLEASE_DETACH	 	((UNSIGNED16)6u << 8)
//#define EB_BAT_CTRL7_NORMAL_OPERATION	((UNSIGNED16)7u << 8)
//#define EB_BAT_CTRL8_DONT_ATTACH	 	((UNSIGNED16)8u << 8)
//#define EB_BAT_CTRL9_ERROR			 	((UNSIGNED16)9u << 8)
//#define EB_BAT_CTRL10_DONT_ATTACH	 	((UNSIGNED16)10u << 8)
//#define EB_BAT_CTRL11_ERROR			 	((UNSIGNED16)11u << 8)


/**
* battery specific data
*
* accessible for application by ebBatData[];
*/
typedef struct {
	UNSIGNED16	type;					/**< data at object 0x6100 */
	UNSIGNED32	packRatedWhCapacity;	/**< data at object 0x6102 */
	INTEGER16	temperature[1];			/**< data at object 0x6105 */

	INTEGER16 	maxChargeTemperature; 	/**< data at object 0x6120 */
	INTEGER16 	minChargeTemperature; 	/**< data at object 0x6121 */

	INTEGER32	packMaximumCellVoltage;	/**< data at object 0x6126 */
	INTEGER32	packMinimumCellVoltage;	/**< data at object 0x6127 */
	UNSIGNED32	packActualWhCapacity;	/**< data at object 0x6160 */
	UNSIGNED32	fullOutputWhCapacity;	/**< data at object 0x6162 */
	UNSIGNED16	relativeWhCapacity;		/**< data at object 0x6164 */
	UNSIGNED16	packSOH; 				/**< data at object 0x6176 */
	UNSIGNED8	testMode;
	REAL32		Cell1;
	REAL32		Cell2;
	REAL32		Cell3;
	REAL32		Cell4;
	REAL32		Cell5;
	REAL32		Cell6;
	REAL32		Cell7;
	REAL32		Cell8;
	REAL32		Cell9;
	REAL32		Cell10;
	REAL32		Cell11;
	REAL32		Cell12;
	REAL32		Cell13;
	REAL32		Cell14;
	REAL32		Cell15;
	UNSIGNED8		BalCell1;
	UNSIGNED8		BalCell2;
	UNSIGNED8		BalCell3;
	UNSIGNED8		BalCell4;
	UNSIGNED8		BalCell5;
	UNSIGNED8		BalCell6;
	UNSIGNED8		BalCell7;
	UNSIGNED8		BalCell8;
	UNSIGNED8		BalCell9;
	UNSIGNED8		BalCell10;
	UNSIGNED8		BalCell11;
	UNSIGNED8		BalCell12;
	UNSIGNED8		BalCell13;
	UNSIGNED8		BalCell14;
	UNSIGNED8		BalCell15;
	UNSIGNED16		packVoltage; /* 0x2004:1 */
	UNSIGNED16		fuseVoltage; /* 0x2004:2 */
	UNSIGNED16		motVoltage; /* 0x2004:3 */
	UNSIGNED16		chargeVoltage; /* 0x2004:4 */
	UNSIGNED16		HVIPL0Voltage; /* 0x2004:5 */
	UNSIGNED16		HVIPL1Voltage; /* 0x2004:6 */
	INTEGER8		Temp1 ; /* 0x2003:1 */
	INTEGER8		Temp2 ; /* 0x2003:2 */
	INTEGER8		Temp3 ; /* 0x2003:3 */
	INTEGER8		Temp4 ; /* 0x2003:4 */
	INTEGER8		Temp5 ; /* 0x2003:4 */
	INTEGER16		packCurrent ; /* 0x2005:1 */
	INTEGER16		Current_12V ; /* 0x2005:2 */
	UNSIGNED8			preChargeDone; /* 0x2006:1 */
	UNSIGNED8			isChargerConnected; /* 0x2006:2 */
} EB_BATTERY_DATA_T;


/* external data */

extern EB_BATTERY_DATA_T ebBatData[];

/* function prototypes */

EB_RET_T	ebRegister_BAT_STATE(EB_EVENT_CHECK_STATE_T pCheckFunction,
				EB_EVENT_STATE_T pFunction);
EB_RET_T	ebBatteryStateTransition(UNSIGNED16 ctrlWord, UNSIGNED8 devIdx);
EB_RET_T	ebBatteryStateChange(UNSIGNED16	newState, UNSIGNED8	devIdx);


#endif /* EB_BATTERY_H */

