/***************************************************/
/*******	INCLUDING FILES DEFINATION SECTION	****/
/***************************************************/

#include "HAL.h"
#include "UDSLayerApp.h"

#include "Battery.h"
#include "Controller.h"
#include "UDS-PID.h"
#include "UDSLib.h"
#include "Vehicle.h"
#include "charge.h"
#include "ecu.h"




#ifdef BOOT_MODE
#else

typedef union
{
	UINT8   uiDataByte; 
	struct{
		UINT8 	Bit0:1,
				Bit1:1,
 		    		Bit2:1, 
 		     		Bit3:1, 
 		     		Bit4:1, 
 		     		Bit5:1, 
 		     		Bit6:1, 
 		     		Bit7:1; 
	};
}StsByte;

static const UINT8 uaPIDChkTbl[PID_MAX] = {
	0x00,	// HVIL Status
				// HVIL Source Current
				// HVIL Return Current
	0x01,	// BPCM 12 Volt connector inputs
				// BPCM Contactor Driver -status
	0x02,	// DisCharge AmpHours
				// Charge AmpHours
				// DisCharge KW*hours
				// Charge KW*hours
				// Miles or HV Battery Miles?
	0x04,	// Diagnostic Dis\Enabler
				// Contactor Enabler(Service)
				//Contactor Enabler(Engineering-Manufacturing)
	0x42,	// System(12)Volts
	0x05,	// HV SOC Time Stats
				// HV Temperature Time Stats	
				// HV Cell Temperature Differentials Time Stats	
				// HV Cell Volt Differentials Time Stats	
				// HV Battery Voltage Exceeded	
	0x06,	// MEC Counter(Manufactures Enable Counter)
	0x07,	// Shipped packs to plant
				// Shipped packs to Chrysler
	0x08,	// Number Of Opens
				// Number Of Closes
				// Number Of Opens_Under_Load
				// Number Of Impending_Opens
				// Number Of Open_Requests
				// Number Of loss_12V(Faults)
				// Number Of PreCharge_Failures
				// Number Of Failure to write to eeprom(moved to Default)
	0x09,	// Max voltage cell
				// Min voltage cell
				// Max Cell Voltage
				// Min Cell Voltage
				// Max Temp cell
				// Min Temp cell
				// Max Cell Temp
				// Min Cell Temp
	0x10,	// State of Charge
				// Power-up State of Charge
				// Battery Impedance
				// Max Cell Resistance
	0x11,	// Battery Voltage(cell sum)
				// Battery Voltage Sensor 
				// Precharge Voltage
				// BPCM Precharge Voltage Source
				// Bus Voltage
				// Bus Voltage Source
				// Bus Voltage Sensor
				// Battery Positive to Relay Negative Voltage Sensor
				// Relay Positive to Battery Negative Voltage Sensor
				// Charger Voltage Sensor
				// Charger Positive to Battery Negative Voltage Sensor
				// Charger Negative to Battery Positive Voltage Sensor
	0x12,	// HV Battery Current
				// HV Battery Current-Sensor volts
	0x13,	// HV cooling Speed
				// HV cooling Speed Commanded
	0x14,	// Contactor Status
				// Contactor Commanded(wire)				
				// HCP Contactor Command				
				// Contactor Commanded Status				
				// Precharge Penalty Time
				// BPCM Status Flags
	0x15,	// Hybrid Battery Reasons Contactors Opened
				// Stored -Hybrid Battery Reasons Contactors Opened	
	0x16,	// Hybrid Battery Reasons Contactors not Closing when Commanded
				// Stored -Reasons Contactors not Closing when Commanded	
	0x17,	// Isolation Diagnostic Status
				// Isolation Impedance
	0x18,	// Current Sensor 1
				// Current Sensor 2
				// Current Sensor 3
	0x20,	// Discharge Power Available
				// Charge Power Available	
	0x21,	// Battery Cooling Inlet Temp
				// Battery Cooling Inlet Temp Sensor volts	
				// Battery Cooling Outlet Temp 
				// Battery Cooling Outlet Temp Sensor volts	
	0x22,	// Hybrid Battery Reasons Precharge did not complete
	0x23,	// CAN dbc file name
	0x24,	// EOL Scratch Pad
	0x25,	// State Of Charge Adjustment In progress
	0x26,	// Threshold Cals 
	0x27,	// Plug in charge Status
	0x28,	//PID_BALANCE_STATUS_PER_KEY_CYCLE
	0x29,	//HIDDEN SOC
	0x30,	//CAN LABLE 
	0x31,	//Force to write
	0x40,	//read DTC enabler
	0x50,	//coil status
	0x51,	//lose comm status
	0x52,//cell sensor status
	0x53,
	//for isolation 11.41.01
	0x54,
	0x55,
};

extern UINT8 uaUDSReqPara[8];
extern UINT8 uaBuffer[UDS_DATA_TRANSMIT_LEN];
extern UINT16 uiSendSize;
UINT16 uiLastStoredSOC=0;

UINT8 uiCheckPIDTbl(void);
UINT8 uiGetBalanceCellNo(void);
extern void uvCopyBuffer(UINT8 * pCopyTo,UINT8 * pCopyFrom, UINT16 uiSize);

extern UINT8 uiDTCLoseModule;
//real environment
UINT8 uvGetPIDValue(void)
{
	UINT8 uiPIDNo;
	UINT8 uiBoardNo;
	UINT8 uiCellNo;
	UINT8 uiTempNo;
	UINT8 uiDataOneByte;
	UINT16 uiDataTwoBytes;
	UINT16 * pDataTwo;
	INT16 uiSignedTwoBytes;
	UINT32 uiDataFourBytes;
	StsByte uiDataBits;
	UINT16 i,j;
#ifdef TEMP_SENSOR_96
	INT8 * pTemp;
#endif
	
	uiPIDNo=uiCheckPIDTbl();
	uiDataOneByte=0;
	uiDataTwoBytes=0;
	uiSignedTwoBytes=0;
	uiDataFourBytes=0;
	uiDataBits.uiDataByte=0;
	for (i=0;i<255;i++){
		uaBuffer[i]=0x00;
	}

	//0x01~0x08 for module8
	if((uiPIDNo>=PID_MAX)&&(uiPIDNo<=PID_MAX+0x07)){
		uiTempNo=uiPIDNo-PID_MAX;
		uiBoardNo=uiTempNo/2;
		uiCellNo=(uiTempNo%2)*12;//start
		for(i=0;i<12;i++){
			uiDataTwoBytes=batteryPackInfo[0].cellVoltage[uiBoardNo][uiCellNo];
			uaBuffer[i*2+0]=((uiDataTwoBytes)>>8)&0xff;
			uaBuffer[i*2+1]=((uiDataTwoBytes))&0xff;
			uiCellNo++;//cell number
		}
		uiSendSize=24;
	}
	//0x09~0x12 for module4
#ifdef TEMP_SENSOR_96
	else if((uiPIDNo>=(PID_MAX+0x08))&&(uiPIDNo<=(PID_MAX+0x08+0x05))){
		uiTempNo=uiPIDNo-PID_MAX-0x08;
		pTemp=&batteryPackInfo[0].temperature[0][0];
		pTemp+=uiTempNo*16;
//		uiBoardNo=uiTempNo;
		for(i=0;i<16;i++){
//			uaBuffer[i]=batteryPackInfo[0].temperatureSensor[uiBoardNo][i];
			uaBuffer[i]=*(pTemp+i);
			//convert data
			uaBuffer[i]+=40;
		}
		uiSendSize=16;
	}
#else	//phase 2. 4sensor each module. 4module total
	else if((uiPIDNo>=(PID_MAX+0x08))&&(uiPIDNo<=(PID_MAX+0x08+0x03))){
		uiTempNo=uiPIDNo-PID_MAX-0x08;
		uiBoardNo=uiTempNo;
		for(i=0;i<4;i++){
				uaBuffer[i]=batteryPackInfo[0].temperature[uiBoardNo][i];
				uaBuffer[i]+=40;
			
		}
		uiSendSize=4;
	}

#endif
	else{
#ifdef TEMP_SENSOR_96
		if((uiPIDNo>=(PID_MAX+0x08+0x06))&&(uiPIDNo<=(PID_MAX+0x08+0x06+0x07))){
			uiSendSize=12;
			for(i=0;i<12;i++){
				uiDataOneByte=uiPIDNo-PID_MAX-0x08-0x06;
				uaBuffer[i]=ModuleFailInfo[uiDataOneByte/2][(uiDataOneByte%2)*12+i].ModuleFail_BYTE;//cell1 ~12

			}
		}
#else
#if 0  	//ver 13.17.01
		//PID: 0xA211 to 0xA218  Module DTC
		if((uiPIDNo>=(PID_MAX+0x08+0x04))&&(uiPIDNo<=(PID_MAX+0x08+0x04+0x07))){
			uiSendSize=12;
			for(i=0;i<12;i++){
				uiDataOneByte=uiPIDNo-PID_MAX-0x08-0x04;
				uaBuffer[i]=ModuleFailInfo[uiDataOneByte/2][(uiDataOneByte%2)*12+i].ModuleFail_BYTE;//cell1 ~12

			}
		}
#else
//PID: 0xA211 to 0xA218  Module DTC
		if((uiPIDNo>=(PID_MAX+0x08+0x04))&&(uiPIDNo<=(PID_MAX+0x08+0x04+0x07))){
			UINT8 dtcMid,mid;
			dtcMid=uiPIDNo-PID_MAX-0x08-0x04;	//module id : 0 to 7
			mid=dtcMid/2;
			if((dtcMid%2)==0){	//module 0,2,4,6
				uiSendSize=10;
				//only 4 cells have temp snesor
				for(i=0;i<4;i++){
					uiDataOneByte=uiPIDNo-PID_MAX-0x08-0x04;
					uaBuffer[i]=ModuleFailInfo[uiDataOneByte/2][(uiDataOneByte%2)*12+i].ModuleFail_BYTE;//cell1 ~12

				}
				uaBuffer[4]=ModuleFailInfo[mid][4].Volt_Sensor_Invalid;
				uaBuffer[4]|=ModuleFailInfo[mid][4].Volt_LowLevelExceeded<<2;
				uaBuffer[4]|=ModuleFailInfo[mid][4].Volt_HighLevelExceeded<<3;
				uaBuffer[4]|=ModuleFailInfo[mid][4].Voltage_Low<<4;
				uaBuffer[4]|=ModuleFailInfo[mid][4].Voltage_High<<5;

				uaBuffer[4]|=ModuleFailInfo[mid][5].Volt_Sensor_Invalid<<6;
				uaBuffer[4]|=ModuleFailInfo[mid][5].Volt_LowLevelExceeded<<7;
				uaBuffer[5]=ModuleFailInfo[mid][5].Volt_HighLevelExceeded;
				uaBuffer[5]|=ModuleFailInfo[mid][5].Voltage_Low<<1;
				uaBuffer[5]|=ModuleFailInfo[mid][5].Voltage_High<<2;

				uaBuffer[5]|=ModuleFailInfo[mid][6].Volt_Sensor_Invalid<<3;
				uaBuffer[5]|=ModuleFailInfo[mid][6].Volt_LowLevelExceeded<<4;
				uaBuffer[5]|=ModuleFailInfo[mid][6].Volt_HighLevelExceeded<<5;
				uaBuffer[5]|=ModuleFailInfo[mid][6].Voltage_Low<<6;
				uaBuffer[5]|=ModuleFailInfo[mid][6].Voltage_High<<6;

				uaBuffer[6]=ModuleFailInfo[mid][7].Volt_Sensor_Invalid;
				uaBuffer[6]|=ModuleFailInfo[mid][7].Volt_LowLevelExceeded<<1;
				uaBuffer[6]|=ModuleFailInfo[mid][7].Volt_HighLevelExceeded<<2;
				uaBuffer[6]|=ModuleFailInfo[mid][7].Voltage_Low<<3;
				uaBuffer[6]|=ModuleFailInfo[mid][7].Voltage_High<<4;

				uaBuffer[6]|=ModuleFailInfo[mid][8].Volt_Sensor_Invalid<<5;
				uaBuffer[6]|=ModuleFailInfo[mid][8].Volt_LowLevelExceeded<<6;
				uaBuffer[6]|=ModuleFailInfo[mid][8].Volt_HighLevelExceeded<<7;
				uaBuffer[7] =ModuleFailInfo[mid][8].Voltage_Low;
				uaBuffer[7]|=ModuleFailInfo[mid][8].Voltage_High<<1;

				uaBuffer[7]|=ModuleFailInfo[mid][9].Volt_Sensor_Invalid<<2;
				uaBuffer[7]|=ModuleFailInfo[mid][9].Volt_LowLevelExceeded<<3;
				uaBuffer[7]|=ModuleFailInfo[mid][9].Volt_HighLevelExceeded<<4;
				uaBuffer[7]|=ModuleFailInfo[mid][9].Voltage_Low<<5;
				uaBuffer[7]|=ModuleFailInfo[mid][9].Voltage_High<<6;

				uaBuffer[7]|=ModuleFailInfo[mid][10].Volt_Sensor_Invalid<<7;
				uaBuffer[8]=ModuleFailInfo[mid][10].Volt_LowLevelExceeded;
				uaBuffer[8]|=ModuleFailInfo[mid][10].Volt_HighLevelExceeded<<1;
				uaBuffer[8]|=ModuleFailInfo[mid][10].Voltage_Low<<2;
				uaBuffer[8]|=ModuleFailInfo[mid][10].Voltage_High<<3;

				uaBuffer[8]|=ModuleFailInfo[mid][11].Volt_Sensor_Invalid<<4;
				uaBuffer[8]|=ModuleFailInfo[mid][11].Volt_LowLevelExceeded<<5;
				uaBuffer[8]|=ModuleFailInfo[mid][11].Volt_HighLevelExceeded<<6;
				uaBuffer[8]|=ModuleFailInfo[mid][11].Voltage_Low<<7;
				uaBuffer[9]=ModuleFailInfo[mid][11].Voltage_High;

			}else{	//module 1,3,57
				uiSendSize=8;
				//no cells have temp snesor
				uaBuffer[0]=ModuleFailInfo[mid][12].Volt_Sensor_Invalid;
				uaBuffer[0]|=ModuleFailInfo[mid][12].Volt_LowLevelExceeded<<1;
				uaBuffer[0]|=ModuleFailInfo[mid][12].Volt_HighLevelExceeded<<2;
				uaBuffer[0]|=ModuleFailInfo[mid][12].Voltage_Low<<3;
				uaBuffer[0]|=ModuleFailInfo[mid][12].Voltage_High<<4;

				uaBuffer[0]|=ModuleFailInfo[mid][13].Volt_Sensor_Invalid<<5;
				uaBuffer[0]|=ModuleFailInfo[mid][13].Volt_LowLevelExceeded<<6;
				uaBuffer[0]|=ModuleFailInfo[mid][13].Volt_HighLevelExceeded<<7;
				uaBuffer[1] =ModuleFailInfo[mid][13].Voltage_Low;
				uaBuffer[1]|=ModuleFailInfo[mid][13].Voltage_High<<1;

				uaBuffer[1]|=ModuleFailInfo[mid][14].Volt_Sensor_Invalid<<3;
				uaBuffer[1]|=ModuleFailInfo[mid][14].Volt_LowLevelExceeded<<4;
				uaBuffer[1]|=ModuleFailInfo[mid][14].Volt_HighLevelExceeded<<5;
				uaBuffer[1]|=ModuleFailInfo[mid][14].Voltage_Low<<6;
				uaBuffer[1]|=ModuleFailInfo[mid][14].Voltage_High<<7;

				uaBuffer[2]=ModuleFailInfo[mid][15].Volt_Sensor_Invalid;				
				uaBuffer[2]|=ModuleFailInfo[mid][15].Volt_LowLevelExceeded<<1;
				uaBuffer[2]|=ModuleFailInfo[mid][15].Volt_HighLevelExceeded<<2;
				uaBuffer[2]|=ModuleFailInfo[mid][15].Voltage_Low<<3;
				uaBuffer[2]|=ModuleFailInfo[mid][15].Voltage_High<<4;

				uaBuffer[2]|=ModuleFailInfo[mid][16].Volt_Sensor_Invalid<<5;
				uaBuffer[2]|=ModuleFailInfo[mid][16].Volt_LowLevelExceeded<<6;
				uaBuffer[2]|=ModuleFailInfo[mid][16].Volt_HighLevelExceeded<<7;
				uaBuffer[3]=ModuleFailInfo[mid][16].Voltage_Low;
				uaBuffer[3]|=ModuleFailInfo[mid][16].Voltage_High<<1;

				uaBuffer[3]|=ModuleFailInfo[mid][17].Volt_Sensor_Invalid<<2;
				uaBuffer[3]|=ModuleFailInfo[mid][17].Volt_LowLevelExceeded<<3;
				uaBuffer[3]|=ModuleFailInfo[mid][17].Volt_HighLevelExceeded<<4;
				uaBuffer[3]|=ModuleFailInfo[mid][17].Voltage_Low<<5;
				uaBuffer[3]|=ModuleFailInfo[mid][17].Voltage_High<<6;

				uaBuffer[3]|=ModuleFailInfo[mid][18].Volt_Sensor_Invalid<<7;
				uaBuffer[4]=ModuleFailInfo[mid][18].Volt_LowLevelExceeded;
				uaBuffer[4]|=ModuleFailInfo[mid][18].Volt_HighLevelExceeded<<1;
				uaBuffer[4]|=ModuleFailInfo[mid][18].Voltage_Low<<2;
				uaBuffer[4]|=ModuleFailInfo[mid][18].Voltage_High<<3;

				uaBuffer[4]|=ModuleFailInfo[mid][19].Volt_Sensor_Invalid<<4;
				uaBuffer[4]|=ModuleFailInfo[mid][19].Volt_LowLevelExceeded<<5;
				uaBuffer[4]|=ModuleFailInfo[mid][19].Volt_HighLevelExceeded<<6;
				uaBuffer[4]|=ModuleFailInfo[mid][19].Voltage_Low<<7;
				uaBuffer[5]=ModuleFailInfo[mid][19].Voltage_High;

				uaBuffer[5]|=ModuleFailInfo[mid][20].Volt_Sensor_Invalid<<1;
				uaBuffer[5]|=ModuleFailInfo[mid][20].Volt_LowLevelExceeded<<2;
				uaBuffer[5]|=ModuleFailInfo[mid][20].Volt_HighLevelExceeded<<3;
				uaBuffer[5]|=ModuleFailInfo[mid][20].Voltage_Low<<4;
				uaBuffer[5]|=ModuleFailInfo[mid][20].Voltage_High<<5;

				uaBuffer[5]|=ModuleFailInfo[mid][21].Volt_Sensor_Invalid<<6;
				uaBuffer[5]|=ModuleFailInfo[mid][21].Volt_LowLevelExceeded<<7;
				uaBuffer[6]=ModuleFailInfo[mid][21].Volt_HighLevelExceeded;
				uaBuffer[6]|=ModuleFailInfo[mid][21].Voltage_Low<<1;
				uaBuffer[6]|=ModuleFailInfo[mid][21].Voltage_High<<2;
				
				uaBuffer[6]|=ModuleFailInfo[mid][22].Volt_Sensor_Invalid<<3;
				uaBuffer[6]|=ModuleFailInfo[mid][22].Volt_LowLevelExceeded<<4;
				uaBuffer[6]|=ModuleFailInfo[mid][22].Volt_HighLevelExceeded<<5;
				uaBuffer[6]|=ModuleFailInfo[mid][22].Voltage_Low<<6;
				uaBuffer[6]|=ModuleFailInfo[mid][22].Voltage_High<<7;

				uaBuffer[7]=ModuleFailInfo[mid][23].Volt_Sensor_Invalid;
				uaBuffer[7]|=ModuleFailInfo[mid][23].Volt_LowLevelExceeded<<1;
				uaBuffer[7]|=ModuleFailInfo[mid][23].Volt_HighLevelExceeded<<2;
				uaBuffer[7]|=ModuleFailInfo[mid][23].Voltage_Low<<3;
				uaBuffer[7]|=ModuleFailInfo[mid][23].Voltage_High<<4;
			
			}
		}

#endif
#endif

		else{
		
		
		switch(uiPIDNo){
			case PID_HVIL_STATUS://8 bits
				uiSendSize=5;
				//HVIL Status		1byte
				//$00 Not sourced, $01 Pass, $02 Fail $03 Invalid, $04 service disconnect is removed
				 if(systemInfo.HVILLoop==CLOSE){
					uiDataOneByte=1;
				}else if(systemInfo.HVILLoop==OPEN){
					uiDataOneByte= 2;
				}else if(systemInfo.HVILLoop==UNKNOWN){
					uiDataOneByte = 0;
				}else{
					uiDataOneByte = 3;
				}
				uaBuffer[0]=uiDataOneByte;
				//Modified by Amanda return HVIL current as 0 when HVIL loop isnot closing on July 29
				 if(systemInfo.HVILLoop==CLOSE){
					//HVIL Source Current		2bytes
					uiSignedTwoBytes=systemInfo.HVIL_CURRENT*10;
					uaBuffer[1]=((uiSignedTwoBytes)>>8)&0xff;
					uaBuffer[2]=((uiSignedTwoBytes))&0xff;
					//HVIL Return Current		2bytes
					uiSignedTwoBytes=-(systemInfo.HVIL_CURRENT*10);
					uaBuffer[3]=((uiSignedTwoBytes)>>8)&0xff;
					uaBuffer[4]=((uiSignedTwoBytes))&0xff;
				 }
				 else {
					//HVIL Source Current		2bytes
					uaBuffer[1]=0;
					uaBuffer[2]=0;
					//HVIL Return Current		2bytes
					uaBuffer[3]=0;
					uaBuffer[4]=0;
				 }
				break;
			case PID_12_CONNECTOR_INPUT://8 bits
				uiSendSize=2;
				//not available for development pack
				if(HCP_HSCOMM_STATUS){
					uiDataBits.Bit0=1;
				}
				if(HCP_ACCESSARY_STATUS){
					uiDataBits.Bit1=1;
				}
//				if(systemInfo.VFan>=90){  //Fan power
				if(systemInfo.V12>=90){//apply system 12v according to Euthie's request
					uiDataBits.Bit2=1;
				}
				if(systemInfo.V12>=90){
					uiDataBits.Bit3=1;
				}
				uaBuffer[0]=uiDataBits.uiDataByte;
				uiDataBits.uiDataByte=0;
				//8 bits PID_CONTACTOR_DRIVER_STATUS
				if(MCU_RELAY_MAIN_POS){
					uiDataBits.Bit0=1;
				}
				if(MCU_RELAY_MAIN_NEG){
					uiDataBits.Bit1=1;
				}
				if(MCU_RELAY_PRE){
					uiDataBits.Bit2=1;
				}
				//Added by Amanda for implementation DDT 02 03 
				if(MCU_RELAY_CHG_POS){
					uiDataBits.Bit3=1;
				}
				if(MCU_RELAY_CHG_NEG){
					uiDataBits.Bit4=1;
				}
				uaBuffer[1]=uiDataBits.uiDataByte;
				break;
			case PID_HIGH_DISCHARGE_AMPHRS:
				uiSendSize=15;
				
				//Charge AmpHours		2bytes
				//32 bits, I*Time uint: 0.01 A.S . valid in Charge precess
//Version 1.065 for DDT 0202 modified by Amanda on July 28
				uiDataFourBytes=eeprom_ram_para.ChgAH;
			//	uiDataFourBytes*=10;
				uaBuffer[0]=(uiDataFourBytes>>16)&0xff;
				uaBuffer[1]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[2]=(uiDataFourBytes)&0xff;
				
				//DisCharge AmpHours		3bytes
				//32 bits, I*Time uint: 0.01A.S valid in Discharge Precess
				uiDataFourBytes=eeprom_ram_para.DisChgAH;
			//	uiDataFourBytes*=10;
				uaBuffer[3]=(uiDataFourBytes>>16)&0xff;
				uaBuffer[4]=(uiDataFourBytes >>8)&0xff;
				uaBuffer[5]=(uiDataFourBytes)&0xff;

//Version 1.065 for DDT 0202 modified by Amanda on July 28 not finish yet
				//DisCharge KW*hours		3byte
				uiDataFourBytes=(eeprom_ram_para.KWattHrs_DisChg/10);//mega kwh
				uaBuffer[6]=(uiDataFourBytes>>16)&0xff;
				uaBuffer[7]=(uiDataFourBytes >>8)&0xff;
				uaBuffer[8]=(uiDataFourBytes)&0xff;
				//Charge KW*hours		3byte
				uiDataFourBytes=(eeprom_ram_para.KWattHrs_Chg/10);//mega kwh

				uaBuffer[9]=(uiDataFourBytes>>16)&0xff;
				uaBuffer[10]=(uiDataFourBytes >>8)&0xff;
				uaBuffer[11]=(uiDataFourBytes)&0xff;
				//Miles					//?
				//value/0.1553425

 //				uiDataFourBytes= vehicleInfo.distance_lifetime*100/160934;//convert from meter to mile
//				uiDataFourBytes=(uiDataFourBytes*10000000/1553425);
 //Modified by Amanda for Mile report incorrect
				uiDataFourBytes= vehicleInfo.distance_lifetime;
				uiDataFourBytes/=250;
				uaBuffer[12]=(uiDataFourBytes>>16)&0xff;
				uaBuffer[13]=(uiDataFourBytes >>8)&0xff;
				uaBuffer[14]=(uiDataFourBytes)&0xff;
				break;
			case PID_DIAG_DIS_ENABLER:
				uiSendSize=1;
				//Diagnostic 4-31 Enabler  1bit
				//Disabled 0x00, Enabled 0x01
				//Contactor Enabler(Engineering-Manufacturing)  1bit
				uaBuffer[0]=UDSWriteInfo.uiContactorEnabler;
				break;
			case PID_SYSTEM_12V://16 bits
				//System(12) Volts			2bytes
//				uiSendSize=2;
				uiSendSize=13;
				uiDataTwoBytes=systemInfo.V12*100;
//				pDataTwo=&uaBuffer[0];
//				*pDataTwo=uiDataTwoBytes;
				uaBuffer[0]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[1]=(uiDataTwoBytes)&0xff;
//for test panels
				uiDataTwoBytes=systemInfo.BackUp_V12*100;
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;
				uiDataTwoBytes=systemInfo.VFan*100;
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;
//V12 AD
				uaBuffer[6]=(averageADCResult[19]>>8) & 0x0F;
				uaBuffer[7]=averageADCResult[19]& 0xFF;
//Backup12v AD
				uaBuffer[8]=(averageADCResult[2]>>8) & 0x0F;
				uaBuffer[9]=averageADCResult[2]& 0xFF;
//Fan12 AD
				uaBuffer[10]=(averageADCResult[4]>>8) & 0x0F;
				uaBuffer[11]=averageADCResult[4]& 0xFF;
				if(MCU_FAN_PWR_RELAY_STAT==OPEN){
					uaBuffer[12]=0;
				}
				else{
					uaBuffer[12]=1;
				}
				break;
			case PID_HV_SOC_TIME_STATS:
				uiSendSize=54;
				//HV SOC Time Stats    2*10 bytes
				//tool is 2*8 bytes
				for (i=0;i<8;i++){
					uiDataTwoBytes=eeprom_ram_para.HV_SOC_CounterCDA[i]/(5);  //convert it to minutes
					uaBuffer[i*2]=(uiDataTwoBytes>>8)&0xff;
					uaBuffer[i*2+1]=(uiDataTwoBytes)&0xff;
				}
				//HV Temperature Time Stats   2*11 bytes
				for (i=0;i<11;i++){
					uiDataTwoBytes=eeprom_ram_para.HV_Temp_CounterCDA[i]/(5);  //convert it to minutes
					uaBuffer[16+i*2]=(uiDataTwoBytes>>8)&0xff;
					uaBuffer[16+i*2+1]=(uiDataTwoBytes)&0xff;
				}
				//HV Module Temperature Differentials Time Stats   6 bytes
				for (i=0;i<3;i++){
					uiDataTwoBytes=eeprom_ram_para.HV_TempDiff_counterCDA[i]/(5);  //convert it to minutes
					uaBuffer[38+i*2]=(uiDataTwoBytes>>8)&0xff;
					uaBuffer[38+i*2+1]=(uiDataTwoBytes)&0xff;
				}
				//HV Module Volt Differentials Time Stats	2bytes
				//spec is cell Volt Differential >1.5v  during time(seconds)
				uiDataTwoBytes=eeprom_ram_para.HV_CellVoltDiff_counterCDA;
				uaBuffer[44]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[45]=(uiDataTwoBytes)&0xff;
		
				//HV Battery Voltage Exceeded   2*4 bytes
				//Pack upper 4000(SW)
				uiDataTwoBytes=eeprom_ram_para.HV_Volt_Counter[11];
				uiDataTwoBytes/=(60*5);
				uaBuffer[46]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[47]=(uiDataTwoBytes)&0xff;
				//Pack lower 3200(SW)--0,1,2
				uiDataTwoBytes=eeprom_ram_para.HV_Volt_Counter[0]+eeprom_ram_para.HV_Volt_Counter[1]+eeprom_ram_para.HV_Volt_Counter[2];
				uiDataTwoBytes/=(60*5);
				uaBuffer[48]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[49]=(uiDataTwoBytes)&0xff;
				//Cell Volt upper & lower
				//upper 4000(SW)
				uaBuffer[50]=0;
				uaBuffer[51]=0;
				//lower 3200(SW)
				uaBuffer[52]=0;
				uaBuffer[53]=0;
				break;
			case PID_MEC_COUNTER:
				//MEC Counter(Manufacturers Enable Counter)	1byte
				uiSendSize=1;
				//ECU's security system armed 0x00
				//ECU's security system not armed 0x01
				uaBuffer[0]=UDSWriteInfo.uiMECCounter;
				break;
			case PID_SHIPPED_PACK_TO_PLANT:
				uiSendSize=24;
				//Shipped packs to plant		14bytes
				//Plant Location				//4 bytes ASCII code
				uaBuffer[0]='U';
				uaBuffer[1]='S';
				uaBuffer[2]=' ';
				uaBuffer[3]=' ';
				//Pack Number				//10 bytes BCD code
				uaBuffer[4]=0b0000;
				uaBuffer[5]=0b0000;
				uaBuffer[6]=0b0000;
				uaBuffer[7]=0b0000;
				uaBuffer[8]=0b0000;
				uaBuffer[9]=0b0000;
				uaBuffer[10]=0b0000;
				uaBuffer[11]=UDSWriteInfo.uaECUSerialNo[9]-'0';
				uaBuffer[12]=UDSWriteInfo.uaECUSerialNo[10]-'0';
				uaBuffer[13]=UDSWriteInfo.uaECUSerialNo[11]-'0';
				//Shipped pack to Chrysler	10bytes
				//Pack Number				//10 bytes BCD code
				uaBuffer[14]=0b0000;
				uaBuffer[15]=0b0000;
				uaBuffer[16]=0b0000;
				uaBuffer[17]=0b0000;
				uaBuffer[18]=0b0000;
				uaBuffer[19]=0b0000;
				uaBuffer[20]=0b0000;
				uaBuffer[21]=UDSWriteInfo.uaECUSerialNo[9]-'0';
				uaBuffer[22]=UDSWriteInfo.uaECUSerialNo[10]-'0';
				uaBuffer[23]=UDSWriteInfo.uaECUSerialNo[11]-'0';
				break;
			case PID_EOL_SCRATCH_PAD:
				uiSendSize=5;
				uiDataBits.uiDataByte=UDSWriteInfo.uiEOLScratchPad[0];
				//High Voltage Isolation Test bit0  Pass:0, Fail:1
				//EOL Tests bit1  Pass:0, Fail:1
				//Rolls Tests bit2  Pass:0, Fail:1
				//OK to Ship bit3  Pass:0, Fail:1
				//Battery Charged bit4  Pass:0, Fail:1
				uaBuffer[0]=uiDataBits.uiDataByte;
				uaBuffer[1]=UDSWriteInfo.uiEOLScratchPad[1];
				uaBuffer[2]=UDSWriteInfo.uiEOLScratchPad[2];
				uaBuffer[3]=UDSWriteInfo.uiEOLScratchPad[3];
				uaBuffer[4]=UDSWriteInfo.uiEOLScratchPad[4];
				break;
			case PID_LIFETIME_NUMBERS:
				uiSendSize=16;
				//Number of Opens			2bytes
				uiDataTwoBytes=eeprom_ram_para.OpenNum;
				uaBuffer[0]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[1]=(uiDataTwoBytes)&0xff;
				//Number of Closes			2bytes
				uiDataTwoBytes=eeprom_ram_para.CloseNum;
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;
				//Number of Opens Under Load	2bytes
				uiDataTwoBytes=eeprom_ram_para.OpnUndLd_Num;
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;
				//Number of Impending Opens	2bytes
				uiDataTwoBytes=eeprom_ram_para.ImpndOpn_Num;
				uaBuffer[6]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[7]=(uiDataTwoBytes)&0xff;
				//Number of Open Requests		2bytes
				uiDataTwoBytes=eeprom_ram_para.OpnRq_Num;
				uaBuffer[8]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[9]=(uiDataTwoBytes)&0xff;
				//Number of loss 12V			2bytes
				uiDataTwoBytes=eeprom_ram_para.LV_Loss_Num;
				uaBuffer[10]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[11]=(uiDataTwoBytes)&0xff;
				//Number of Precharge Failures		2bytes
				uiDataTwoBytes=eeprom_ram_para.PreChgFail_Num;
				uaBuffer[12]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[13]=(uiDataTwoBytes)&0xff;
				//Number of failure to write to EEPROM	2bytes
				uiDataTwoBytes=0;
				uaBuffer[14]=0;//number of failure to write to eeprom
				uaBuffer[15]=0;//number of failure to write to eeprom
				break;
			case PID_MIN_AND_MAX_VOL_BLOCKS:
				uiSendSize=10;
				//Min and Max voltage blocks	2bytes
				uaBuffer[0]=(systemInfo.cellMVID[1][0]*CELL_NUM_PER_MODULE)+systemInfo.cellMVID[1][1]+1; //max volt cell  id
				uaBuffer[1]=(systemInfo.cellMVID[0][0]*CELL_NUM_PER_MODULE)+systemInfo.cellMVID[0][1]+1; //min volt cell  id
				//same
				//Max Block Voltage 			1byte
				uiDataTwoBytes=systemInfo.cellVltMax;//mv
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;
				//Min Block Voltage 			1byte
				uiDataTwoBytes=systemInfo.cellVltMin;//mv
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;

				//Min and Max temp blocks		2bytes
				uaBuffer[6]=(systemInfo.cellMTID[1][0]*TEMP_SENSOR_PER_MOD)+systemInfo.cellMTID[1][1]+1; //max temp cell  id
				uaBuffer[7]=(systemInfo.cellMTID[0][0]*TEMP_SENSOR_PER_MOD)+systemInfo.cellMTID[0][1]+1; //min temp cell  id
				//same
				//Max block Temp				1byte
				uaBuffer[8]=systemInfo.cellTempMax+40;
				//Min block Temp				1byte
				uaBuffer[9]=systemInfo.cellTempMin+40;
				break;
			case PID_BATT_STATE_OF_CHARGE:
				uiSendSize=9;
				//State of Charge				1byte
//Version 1.065 for DDT 0202 modified by Amanda on July 28 not finish yet
				//State of Charge Last Stored
				uiDataFourBytes=uiLastStoredSOC;//last stored
				uaBuffer[0]=uiDataFourBytes*255/1000;
				
				//Power-up State of Charge		1byte
				uiDataFourBytes=systemInfo.pwrtupSOC;
				uaBuffer[1]=uiDataFourBytes*255/1000;
				
				uiDataFourBytes=systemInfo.SOC;
				uaBuffer[2]=uiDataFourBytes*255/1000;

				//DDT 02 04
				//Added by Amanda for record minSOC, maxSOC
				uiDataFourBytes=batteryPackInfo[0].SOCmin;
				uaBuffer[3]=uiDataFourBytes*255/1000;
				uiDataFourBytes=batteryPackInfo[0].SOCmax;
				uaBuffer[4]=uiDataFourBytes*255/1000;
				//Battery Impedance 			2bytes
				uiDataFourBytes=systemInfo.impedence;
				uiDataTwoBytes=uiDataFourBytes/1000;
				uaBuffer[5]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[6]=(uiDataTwoBytes)&0xff;
				//Max Block Resistance 		2bytes  ohms
				
				//max Cell resistance
				//sending data =value*10000
				
				uaBuffer[7]=0;
				uaBuffer[8]=0;
				break;
			case PID_HIGH_BATT_VOL_CALCULATED:
#if 1//Added by Amanda August 5
				uiSendSize=26+1;
#else
				uiSendSize=26;
#endif
				//Battery Voltage(cell sum)		2bytes
				uaBuffer[0]=(batteryPackInfo[0].voltage>>8)&0xff;
				uaBuffer[1]=(batteryPackInfo[0].voltage)&0xff;

				//High Voltage Sensor (Pack)	2byes  //0.1v
				uaBuffer[2]=(HVBMsg.HVBBatteryVolt>>8)&0xff;
				uaBuffer[3]=(HVBMsg.HVBBatteryVolt)&0xff;

				//Charge Voltage				2byes  //0.1v
				uaBuffer[4]=(chargerStatus[0].chargeVoltage>>8)&0xff;
				uaBuffer[5]=(chargerStatus[0].chargeVoltage)&0xff;
				

				//BPCM Precharge Voltage Source	1byes
				uiDataOneByte=systemInfo.voltprechargeBattPackVoltSource;// 1: SUM 0:NONE 2:sensor
				//$0=none, $01=Module;$02=sum,$3 to$ff=reserved
				if(uiDataOneByte==2){
					uiDataOneByte=1;
				}
				else if(uiDataOneByte==1){
					uiDataOneByte=2;
				}
				else {
					uiDataOneByte=0;
				}
				uaBuffer[6]=uiDataOneByte;
				
				//Bus Voltage					2byes
				uaBuffer[7]=(systemInfo.voltBus>>8)&0xff;
				uaBuffer[8]=(systemInfo.voltBus)&0xff;

				//Bus Voltage Source			1byes
				uiDataOneByte=systemInfo.voltBusVoltageMeasSource;// 0:  SENSOR  1:CAN 2:INVALID
				//$0=none, $01=Can;$02=sensor,$3 to$ff=reserved
				if(uiDataOneByte==0){
					uiDataOneByte=2;
				}
				else if(uiDataOneByte==1){
					uiDataOneByte=1;
				}
				else if(uiDataOneByte==2){
					uiDataOneByte=0;
				}
				else{
					uiDataOneByte=3;
				}
				uaBuffer[9]=uiDataOneByte;

				//Charger Negative to Battery Positive Voltage Sensor	2byes     0.1v
				uaBuffer[10]=(HVBMsg.HVBBusBos_chgNegVolt>>8)&0xff;
				uaBuffer[11]=(HVBMsg.HVBBusBos_chgNegVolt)&0xff;

				//Charger Positive to Battery Negative Voltage Sensor	2byes     0.1v
				uaBuffer[12]=(HVBMsg.HVBChgPos_busNegVolt>>8)&0xff;
				uaBuffer[13]=(HVBMsg.HVBChgPos_busNegVolt)&0xff;

				//Positive to GND Voltage Sensor	2byes     0.1v
				uaBuffer[14]=(HVBMsg.HVBBattPos_busNegVolt>>8)&0xff;
				uaBuffer[15]=(HVBMsg.HVBBattPos_busNegVolt)&0xff;
				
				//Negative to GND Voltage Sensor	2byes     0.1v
				uaBuffer[16]=(HVBMsg.HVBBusPos_battNegVolt>>8)&0xff;
				uaBuffer[17]=(HVBMsg.HVBBusPos_battNegVolt)&0xff;
//Add voltage sensor 
				//Heater fuse to bus negative Voltage Sensor	2byes     0.1v
				uaBuffer[18]=(HVBMsg.HVBHeaterFuse_busNegVolt>>8)&0xff;
				uaBuffer[19]=(HVBMsg.HVBHeaterFuse_busNegVolt)&0xff;
				//Charger fuse  to Charger negative Voltage Sensor 	2byes     0.1v
				uaBuffer[20]=(HVBMsg.HVBChgFuse_chgNegVolt>>8)&0xff;
				uaBuffer[21]=(HVBMsg.HVBChgFuse_chgNegVolt)&0xff;
				//Battery Positve  to Chassis Voltage Sensor	2byes     0.1v
				uaBuffer[22]=(HVBMsg.HVBPosGNDVolt>>8)&0xff;
				uaBuffer[23]=(HVBMsg.HVBPosGNDVolt)&0xff;
				//Battery Negative  to Chassis Voltage Sensor	2byes     0.1v
				uaBuffer[24]=(HVBMsg.HVBNegGNDVolt>>8)&0xff;
				uaBuffer[25]=(HVBMsg.HVBNegGNDVolt)&0xff;
#if 1//Added by Amanda August 5
				
				uaBuffer[26]=uaStoredDTC[DTC_CAN_COMM_LOSS_BTM];
#endif
				break;
			case PID_BATT_CURRENT_SENSOR:
//Version 1.065 for DDT 0202 modified by Amanda on July 28
				uiSendSize=7;
//				uiSendSize=5;
				//HV Battery Current 			2bytes
				if(systemInfo.currentDirection==1){
					//discharge
					uiSignedTwoBytes=-(systemInfo.current*2);
				}
				else{
					//charge
					uiSignedTwoBytes=(systemInfo.current*2);
				}
				uaBuffer[0]=(uiSignedTwoBytes>>8)&0xff;
				uaBuffer[1]=(uiSignedTwoBytes)&0xff;
				//HV Battery Current-Sensor volts 	1byte
				uiDataFourBytes=averageADCResult[16];
				uiDataFourBytes=uiDataFourBytes*3300/4096;
				
				uiDataFourBytes=uiDataFourBytes*810/274;
				
				uaBuffer[2]=uiDataFourBytes*255/5000;
//Version 1.065 for DDT 0202 modified by Amanda on July 28
				uiSignedTwoBytes=Iavg_5min*2;
				uaBuffer[3]=(uiSignedTwoBytes>>8)&0xff;
				uaBuffer[4]=(uiSignedTwoBytes)&0xff;
				uaBuffer[5]=(averageADCResult[16]>>8)&0xff;
				uaBuffer[6]=(averageADCResult[16])&0xff;

				
				break;
			case PID_HV_COOLING_SPEED:
/****************DDT 020300***********************************/
				uiSendSize=6;				
				//HV Fan Speed  				1byte
				//HV Fan Speed Commanded  	1byte
				//HV Cooling Speed Commanded Driver's Side
				uiDataTwoBytes=getFanPWM(2);			//Fan3,4
				uaBuffer[4]=uiDataTwoBytes*255/100;
				//HV Cooling Speed Commanded  Passenger's Side
				uiDataTwoBytes=getFanPWM(1);			//Fan1,2
				uaBuffer[5]=uiDataTwoBytes*255/100;
				//rpm
				//sending data=value/25
				//systemInfo.fanSpeed is rps*6
				//Send out is rpm
/*				for (i=0;i<4;i++){
					uiDataTwoBytes=(systemInfo.fanSpeed[i]*10);
					if(uiDataTwoBytes>6375){
						uiDataTwoBytes=6375;
					}
					//HV Cooling Speed  Driver's Side Fan 1
					//HV Cooling Speed  Driver's Side Fan 2
					//HV Cooling Speed  Passenger's Side Fan 1
					//HV Cooling Speed  Passenger's Side Fan 2
					uaBuffer[i]=uiDataTwoBytes/25;	//Fan1 ~Fan4
				}*/
					uiDataTwoBytes=(systemInfo.fanSpeed[2]*10);
					if(uiDataTwoBytes>6375){
						uiDataTwoBytes=6375;
					}
					//HV Cooling Speed  Driver's Side Fan 1
					//HV Cooling Speed  Driver's Side Fan 2
					uaBuffer[0]=uiDataTwoBytes/25;	//Fan1 ~Fan4
					uiDataTwoBytes=(systemInfo.fanSpeed[3]*10);
					if(uiDataTwoBytes>6375){
						uiDataTwoBytes=6375;
					}
					uaBuffer[1]=uiDataTwoBytes/25;	//Fan1 ~Fan4
				
					//HV Cooling Speed  Passenger's Side Fan 1
					//HV Cooling Speed  Passenger's Side Fan 2
					uiDataTwoBytes=(systemInfo.fanSpeed[0]*10);
					if(uiDataTwoBytes>6375){
						uiDataTwoBytes=6375;
					}
					uaBuffer[2]=uiDataTwoBytes/25;	//Fan1 ~Fan4
					uiDataTwoBytes=(systemInfo.fanSpeed[1]*10);
					if(uiDataTwoBytes>6375){
						uiDataTwoBytes=6375;
					}
					uaBuffer[3]=uiDataTwoBytes/25;	//Fan1 ~Fan4

/****************DDT 020300***********************************/

/****************DDT 020200***********************************/
//				uiSendSize=6;				
//				//HV Fan Speed  				1byte
//				//rpm
//				//sending data=value/25
//				uaBuffer[0]=systemInfo.fanSpeed[0]/25;	//Fan1 
//				uaBuffer[1]=systemInfo.fanSpeed[1]/25;	//Fan2 
//				uaBuffer[2]=systemInfo.fanSpeed[2]/25;	//Fan3 
//				uaBuffer[3]=systemInfo.fanSpeed[3]/25;	//Fan4  
//				
//				//HV Fan Speed Commanded  	1byte
//				uiDataTwoBytes=getFanPWM(1);			//Fan1,2
//				uaBuffer[4]=uiDataTwoBytes*255/100;
//				uiDataTwoBytes=getFanPWM(2);			//Fan3,4
//				uaBuffer[5]=uiDataTwoBytes*255/100;
/****************DDT 020200***********************************/

/****************DDT 020100***********************************/
//				uiSendSize=2;				
//				//HV Fan Speed  				1byte
//				uaBuffer[0]=systemInfo.fanSpeed[0]/25;	//Fan1 
//				//HV Fan Speed Commanded  	1byte
//				uiDataTwoBytes=getFanPWM(1);
//				uaBuffer[1]=uiDataTwoBytes*255/100;
/****************DDT 020100***********************************/
				break;
			case PID_BATT_CONTACTOR_STATUS:
			//	uiSendSize=7;
				uiSendSize=9;
				//Contactor Status				3bits
				if((systemStatus==SYS_PRE_CHARGE)){
					uiDataOneByte=1;	//precharging
				}else if((systemStatus==SYS_ALL_PACK_DCHG)||(systemStatus==SYS_DCHG_IMPENDING_OPEN)||(systemStatus==SYS_DCHG_REQUEST_OPEN)||(systemStatus==SYS_DCHG_OPEN_CHECK)){
					uiDataOneByte=2;	//closed
				}else if(systemStatus==SYS_PRECHG_FAIL){
					uiDataOneByte=3;	//precharge failed
				}else if(systemStatus==SYS_PCR_INHIBITED){
					uiDataOneByte=4;	//precharge inhibited
				}else{
					uiDataOneByte=0;	//Open 
				}
				uaBuffer[0]=uiDataOneByte;
				//Contactor Commaned(wire)	1byte
			//	uiDataTwoBytes=100-HCPCmd.hVltCntctrCmdDutyCycle;
				uiDataTwoBytes=HCPCmd.hVltCntctrCmdDutyCycle;				
				uaBuffer[1]=uiDataTwoBytes*255/100;

				//HCP Contactor Command		1byte  OPEN 0  CLOSE 1  INVALID 3  IMPENDING_OPEN 2
				//HCPCmd.RdntHighVltCntctrCmd //OPEN 0  CLOSE 1  INVALID 2  IMPENDING_OPEN 3
				if((HCPCmd.RdntHighVltCntctrCmd==0)||(HCPCmd.RdntHighVltCntctrCmd==1)){
					uiDataOneByte=HCPCmd.RdntHighVltCntctrCmd;	
				}
				else if(HCPCmd.RdntHighVltCntctrCmd==3){
					uiDataOneByte=2;	
				}
				else if(HCPCmd.RdntHighVltCntctrCmd==2){
					uiDataOneByte=3;	
				}
				else{
					uiDataOneByte=3;	
				}
				uaBuffer[2]=uiDataOneByte;

				//Contactor Commanded Status	1byte 
				//False:0 True:1
				//VCMD_OPEN-0 VCMD_CLOSE_UNRESTRICT-1 VCMD_CLOSE_RESTRICT-2 VCMD_INVALID-3
				uiDataBits.uiDataByte=0;
				if(HCPCmd.verifiedCntrCmd==0){
					uiDataBits.Bit0=1;
				}
				else if(HCPCmd.verifiedCntrCmd==1){
					uiDataBits.Bit1=1;
				}
				else if(HCPCmd.verifiedCntrCmd==2){
					uiDataBits.Bit2=1;
				}
				else if(HCPCmd.verifiedCntrCmd==3){
					uiDataBits.Bit3=1;
				}
				//get qualified cmd ???????????????
				//QCMD_OPEN-0 QCMD_CLOSE-1 QCMD_IMPENDING_OPEN-2
				if(HCPCmd.qulifiedCntrCmd==0){
					uiDataBits.Bit4=1;
				}
				else if(HCPCmd.qulifiedCntrCmd==1){
					uiDataBits.Bit5=1;
				}
				else if(HCPCmd.qulifiedCntrCmd==2){
					uiDataBits.Bit6=1;
				}
				//bit0 verified open, bit1 verified close unrestricted, bit2 verified close restricted
				//bit3 verified invalid, bit4 qualified open, bit5 qualified close, bit6 qualified impending open
				uaBuffer[3]=uiDataBits.uiDataByte;

				//Precharge Penalty Time		2byte
				uaBuffer[4]=(systemInfo.prechargePenaltyTime>>8)&0xff;
				uaBuffer[5]=(systemInfo.prechargePenaltyTime)&0xff;
				uiDataBits.uiDataByte=0;
				//BPCM Status Flags			1byte
				//Precharge Thermal Protected	bit0		False:0 True:1
//Amanda				if(1){
				if(systemInfo.prechargePenaltyTime!=0){//25min
					uiDataBits.Bit0=1;
				}				
				//Disconnected Bus Detected	bit1		False:0 True:1
				if(DTCLongTable[DTC_PRE_CHARGE_FAST].DTCStatus==DTC_FAILED){
					uiDataBits.Bit1=1;
				}
				//Successful Discharge			bit2		False:0 True:1
				if(!systemInfo.HVBusCapacityDischargeAlarm){
					uiDataBits.Bit2=1;
				}
				//Detected Bus Short			bit3		False:0 True:1
				if(systemInfo.HBBusShortcut==1){
					uiDataBits.Bit3=1;
				}
				//Contactor Command Validity	bit4		Invalid:0 Valid:1
				if(HCPCmd.isRdntHighVltCntctrCmdGenuine==TRUE){
					uiDataBits.Bit4=1;
				}
				else{
					uiDataBits.Bit4=0;
				}
				uaBuffer[6]=uiDataBits.uiDataByte;
				//for panel test
				//Freq		2byte
				uaBuffer[7]=(HCPCmd.hVltCntctrCmdFreqence>>8)&0xff;
				uaBuffer[8]=(HCPCmd.hVltCntctrCmdFreqence)&0xff;
				break;
			case PID_BATT_REASONS_PRECHARGE_DID_NOT_COMPLETE:
				//not implement yet
				//Hybrid Battery Reasons Precharge did not complete	1byte
				uiSendSize=1;
				uaBuffer[0]=0;
				break;
			case PID_BATT_REASONS_CONTACTOR_OPENED:
				uiSendSize=2;
				//Hybrid Battery Reasons Contactors Opened	1byte
	//			if(systemInfo.contactorOpReqest){//command open  ???
				if(HCPCmd.verifiedCntrCmd==VCMD_OPEN){//contactor open request  ???
					uiDataBits.Bit0=1;
				}

/*				if(systemInfo.contactorOpReqest==NO){
					uiDataBits.Bit0=1;
				}*/
				if(systemStatus==SYS_DCHG_IMPENDING_OPEN){//impending open to open transition
					uiDataBits.Bit1=1;
				}
				if(systemInfo.HBCO_PC_FAIL){
					uiDataBits.Bit2=1;
				}
				if(systemInfo.HBCO_BOT){
					uiDataBits.Bit3=1;
				}
				if(systemInfo.HBCO_LV_HI){
					uiDataBits.Bit4=1;
				}
				if(systemInfo.HBCO_LV_LO){
					uiDataBits.Bit5=1;
				}
				if(systemInfo.HBCO_C_VLT_HI){
					uiDataBits.Bit6=1;
				}
				if(systemInfo.HBCO_C_VLT_LO){
					uiDataBits.Bit7=1;
				}
				uaBuffer[0]=uiDataBits.uiDataByte;
				//Stored-Hybrid Battery Reasons Contactors Opened	1byte
/*				if(systemInfo.contactorOpReqest==NO){
					uiDataBits.Bit0=1;
				}*/
				if(HCPCmd.verifiedCntrCmd==VCMD_OPEN){//contactor open request  ???
					uiDataBits.Bit0=1;
				}
				
				if(systemStatus==SYS_DCHG_IMPENDING_OPEN){//impending open to open transition
					uiDataBits.Bit1=1;
				}
				if(systemInfo.prechargeFailLatching){
					uiDataBits.Bit2=1;
				}
				if((systemInfo.overTemperatureSerious_ltLatching)||(systemInfo.overTemperatureSerious_stLatching)){
					uiDataBits.Bit3=1;
				}
				if(systemInfo.V12OverVoltageLatching){
					uiDataBits.Bit4=1;
				}
				if(systemInfo.V12UnderVoltageLatching){
					uiDataBits.Bit5=1;
				}
				if((systemInfo.overVoltageSerious_ltLatching)||(systemInfo.overVoltageSerious_stLatching)){
					uiDataBits.Bit6=1;
				}
				if((systemInfo.underVoltageSerious_ltLatching)||(systemInfo.underVoltageSerious_sLatchingt)){
					uiDataBits.Bit7=1;
				}

				
				uaBuffer[1]=uiDataBits.uiDataByte;
				break;
			case PID_BATT_REASONS_CONTACTOR_NOT_CLOSING_WHEN_COMMANDED:
				//bit0 precharge inhibit, bit1 active discharge fault, bit2 welded contactor, bit3 service disconnect removed
				//bit4 HVIL failure, bit5 contactor closing disabled, bit6 contactor open request, bit7 open command received during precharge
				//When meet the following condition, bit7 will be set TRUE.a) When "High Voltage 
				//Battery Contactor Status" transitions from PRECHARGING to OPEN- AND - b) "Qualified High Voltage Battery Pack Contactor Command" = OPEN (including IMPENDING OPEN to OPEN transitions)
				//When meet the following condition, bit7 will be set FALSEa) "High Voltage Battery Contactor Status" = PRECHARGING (ie OPEN to PRECHARGING transitions)
				uiSendSize=2;
				//Reasons Contactor not Closing when Commanded		1byte
				if(systemInfo.HBCNC_PC_INHBT){
					uiDataBits.Bit0=1;
				}
				if(systemInfo.HBCNC_ADF){
					uiDataBits.Bit1=1;
				}
				if(systemInfo.HBCNC_WC){
					uiDataBits.Bit2=1;
				}
				if(systemInfo.HBCNC_SD_OUT){//service disconnect removed
					uiDataBits.Bit3=1;
				}
				if(systemInfo.HBCNC_HVIL_F){
					uiDataBits.Bit4=1;
				}
				
				//Disabled 0x00, Enabled 0x01
				//Contactor Enabler(Engineering-Manufacturing)  1bit
				if(UDSWriteInfo.uiContactorEnabler&0x01==TRUE){//contactor closing enable
					//BPCM Contactor Enabler (DID 23)
					//disable 1, enable 0
					uiDataBits.Bit5=0;
				}
				else{
					uiDataBits.Bit5=1;
				}
/*				if(HCPCmd.qulifiedCntrCmd==QCMD_OPEN){//contactor open request  ???
				//need confirm
				//Contactor Open Request
					uiDataBits.Bit6=1;
				}*/
				if(systemInfo.contactorOpReqest==YES){
				//Contactor Open Request
					uiDataBits.Bit6=1;
				}
					
				if(systemInfo.HBCNC_OCDP){//open command received during precharge
					uiDataBits.Bit7=1;
				}
				uaBuffer[0]=uiDataBits.uiDataByte;
				//Stored-Reasons Contactor not Closing when Commanded	1byte
				if(systemInfo.HBCNC_PC_INHBT){
					//no latching error?
					uiDataBits.Bit0=1;
				}
				if(systemInfo.HBCNC_ADF){
					//no latching error?
					uiDataBits.Bit1=1;
				}
				if((systemInfo.contractorNegWeldLatching)||(systemInfo.contractorPcrWeldLatching)||(systemInfo.contractorPosWeldLatching)){
					uiDataBits.Bit2=1;
				}
				if(systemInfo.HBCNC_SD_OUT){//service disconnect removed
					//no latching error?
					uiDataBits.Bit3=1;
				}
				if(systemInfo.HVILFailLatching){
					uiDataBits.Bit4=1;
				}
				if(UDSWriteInfo.uiContactorEnabler&0x01==TRUE){//contactor closing disabled  ?
				//need confirm
				//BPCM Contactor Enabler (DID 23)
				//disable 1, enable 0
					//disable 1, enable 0
					uiDataBits.Bit5=0;
				}
				else{
					uiDataBits.Bit5=1;
					}
/*				if(HCPCmd.qulifiedCntrCmd==QCMD_OPEN){//contactor open request  ???
				//need confirm
				//Contactor Open Request
					uiDataBits.Bit6=1;
				}*/
				if(systemInfo.contactorOpReqest==YES){
				//Contactor Open Request
					uiDataBits.Bit6=1;
				}
					
				
				if(systemInfo.HBCNC_OCDP){//open command received during precharge
					uiDataBits.Bit7=1;
				}
				uaBuffer[1]=uiDataBits.uiDataByte;
				break;
			case PID_ISOLATION_DIAG_STATUS:
				uiSendSize=3;
				//Isolation Diagnostic Status		1byte
				
				if(DTCLongTable[DTC_HV_ISOLATION_FAULT].DTCStatus == DTC_FAILED){
					uiDataOneByte=3;	//Run and Failed
				}
				else if(DTCLongTable[DTC_HV_ISOLATION_FAULT].DTCStatus == DTC_PASSED){
					uiDataOneByte=2;	//Run and Passed
				}
				else if((DTCLongTable[DTC_HV_ISOLATION_FAULT].isDTCInhibited== TRUE)
				){
					uiDataOneByte=1;	//Running
				}
				else if((DTCLongTable[DTC_HV_ISOLATION_FAULT].DTCStatus == DTC_NO_STATUS)
					||(DTCLongTable[DTC_HV_ISOLATION_FAULT].DTCStatus == DTC_CRITICALLY_DISABLED)
				){
					uiDataOneByte=0;	//Not Run
				}
				uaBuffer[0]=uiDataOneByte;
				//Isolation Impedance			2bytes
				if(systemInfo.isolationInfo.Ri>=16000){
					uiDataTwoBytes=16000;
				}
				else{
					uiDataTwoBytes=systemInfo.isolationInfo.Ri;
				}
				uiDataTwoBytes*=4;
				uaBuffer[1]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[2]=(uiDataTwoBytes)&0xff;
				break;
			case PID_BATT_CURRENT_SENSOR_1_2_3:
				uiSendSize=6;
				//current sensor1				2bytes
				uaBuffer[0]=0;
				uaBuffer[1]=0;
				//current sensor2				2bytes
				uaBuffer[2]=0;
				uaBuffer[3]=0;
				//current sensor3				2bytes
				uaBuffer[4]=0;
				uaBuffer[5]=0;
				break;
			case PID_BATT_DISCHARGE_POWER_AVAIABLE:
				uiSendSize=4;
				//Discharge Power Available  	2bytes
				uiDataTwoBytes=systemInfo.powerLimit.D;//UINT32
				uaBuffer[0]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[1]=(uiDataTwoBytes)&0xff;
				//Charge Power Available  		2bytes
				uiDataTwoBytes=systemInfo.powerLimit.C;//UINT32
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;
				break;
			case PID_BATT_COOLING_INLET_TEMP_SENSOR:
				uiSendSize=4;
				//Battery Air Inlet Temp		1byte
				uaBuffer[0]=systemInfo.tempCoolantInlet+40;
				uiDataFourBytes=averageADCResult[1];
				uiDataFourBytes=uiDataFourBytes*3300/4096;
				//Battery Air Inlet Temp Sensor volts	1byte
				uaBuffer[1]=uiDataFourBytes*255/5000;
				//Battery Air outlet Temp		1byte
				uaBuffer[2]=systemInfo.tempCoolantOutlet1+40;
				uiDataFourBytes=averageADCResult[3];
				uiDataFourBytes=uiDataFourBytes*3300/4096;
				//Battery Air outlet Temp Sensor volts	1byte
				uaBuffer[3]=uiDataFourBytes*255/5000;
				break;
			case PID_CAN_DBC_FILE_NAME:
				//CAN dbc file name			3bytes
				uiSendSize=6;
				//-3 bytes BCD code

//can dbc 1047b
				uaBuffer[0]=0b0001;//-1
				uaBuffer[1]=0b0000;//-0
				uaBuffer[2]=0b0100;//-4
				uaBuffer[3]=0b0111;//-7
				uaBuffer[4]=0x20;//-0 blank
				uaBuffer[5]=0x20;//-0 blank
				break;
			case PID_SOC_ADJUSTMENT_IN_PROGRESS:
				uiSendSize=15;
				if(systemInfo.HVBBattSOCAdj==TRUE){
					uiDataBits.Bit0=1;//Run OCV
				}
				if(systemInfo.HVBBattSOCAdj==TRUE){
					uiDataBits.Bit1=1;//Run IEO/EV
				}
				if(systemInfo.HVBBattSOCAdjCriMetOCV==TRUE){
					uiDataBits.Bit2=1;//Criteria Met for OCV
				}
				if(systemInfo.HVBBattSOCAdjCriMetIEOEV==TRUE){
					uiDataBits.Bit3=1;//Criteria Met for IEO/EV
				}
				uaBuffer[0]=uiDataBits.uiDataByte;
				//Target SOC (Goal)
				if(systemInfo.AHrAdjTgt>=0){
					uiDataFourBytes=systemInfo.AHrAdjTgt;	// 1A.s
				}
				else{
					uiDataFourBytes=(UINT32)(-systemInfo.AHrAdjTgt);	// 1A.s
				}
//				uiDataFourBytes=(INT32)systemInfo.AHrAdjTgt;	//0.01A.s
				uiDataFourBytes=uiDataFourBytes*100/systemInfo.fullCapacity;
				uiDataFourBytes*=(703/3);
				uaBuffer[1]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[2]=(uiDataFourBytes)&0xff;
				//Current SOC (BPCM SOC)
				uiDataTwoBytes=systemInfo.SOC/10;
					uiDataTwoBytes*=(703/3);
				uaBuffer[3]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[4]=(uiDataTwoBytes)&0xff;
				//Current Change left
			//	uiDataFourBytes=(INT32)systemInfo.AHrAdjLeft;	//0.01A.s
				if(systemInfo.AHrAdjLeft>=0){
					uiDataFourBytes=systemInfo.AHrAdjLeft;	// 1A.s
				}
				else{
					uiDataFourBytes=(UINT32)(-systemInfo.AHrAdjLeft);	// 1A.s
				}
				uiDataFourBytes=uiDataFourBytes*100/systemInfo.fullCapacity;
				uiDataFourBytes*=(703/3);
				uaBuffer[5]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[6]=(uiDataFourBytes)&0xff;
				//Life Number OCV adjustments
				uiDataTwoBytes=eeprom_ram_para.counterSOCAdjOCV;
				uaBuffer[7]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[8]=(uiDataTwoBytes)&0xff;
				//Life Number EV adjustments
				uiDataTwoBytes=eeprom_ram_para.counterSOCAdjEV;
				uaBuffer[9]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[10]=(uiDataTwoBytes)&0xff;
				uaBuffer[11]=(systemInfo.AHrAdjTgt>>24)&0xff;
				uaBuffer[12]=(systemInfo.AHrAdjTgt>>16)&0xff;
				uaBuffer[13]=(systemInfo.AHrAdjTgt>>8)&0xff;
				uaBuffer[14]=(systemInfo.AHrAdjTgt)&0xff;
				
				break;
			case PID_THRESHOLD_CALS:
				uiSendSize=20;
				//V_min_Lt
	//			uiDataTwoBytes=KE_CellVoltageLowerThreshold_SW;
				uiDataTwoBytes=UV_WARNING;
				uaBuffer[0]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[1]=(uiDataTwoBytes)&0xff;
				//V_min_St
	//			uiDataTwoBytes=KE_CellVoltageLowerThreshold_HW;
	//			if(systemInfo.cellTempMin<(-10)) uiDataTwoBytes=UV_LOW_TEMP_ALARM;
	//			else uiDataTwoBytes=UV_ALARM;
				uiDataTwoBytes=keInfo.KE_VOLT_UV_ALARM;
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;
				//V_max_Lt
	//			uiDataTwoBytes=KE_CellVoltageUpperThreshold_SW;
				uiDataTwoBytes=OV_WARNING;
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;
				//V_max_St
	//			uiDataTwoBytes=KE_CellVoltageUpperThreshold_HW;
				uiDataTwoBytes=OV_ALARM;
				uaBuffer[6]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[7]=(uiDataTwoBytes)&0xff;
				//T_min_Lt
//				uaBuffer[8]=KE_TemperatureSensorLowerThreshold_SW+40;
				uaBuffer[8]=UT_WARNING+40;
				//T_min_St
//				uaBuffer[9]=KE_TemperatureSensorLowerThreshold_HW+40;
				uaBuffer[9]=UT_ALARM+40;
				//T_max_Lt
//				uaBuffer[10]=KE_TemperatureSensorUpperThreshold_SW+40;
				uaBuffer[10]=OT_WARNING+40;
				//T_max_St
//				uaBuffer[11]=KE_TemperatureSensorUpperThreshold_HW+40;
				uaBuffer[11]=OT_ALARM+40;
				//V_Lt_time
				uiDataTwoBytes=KE_Voltage_OutOfRange_time_SW*1000;
				uaBuffer[12]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[13]=(uiDataTwoBytes)&0xff;
				//V_St_time
				uiDataTwoBytes=KE_Voltage_OutOfRange_time_HW;
				uaBuffer[14]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[15]=(uiDataTwoBytes)&0xff;
				//T_St_time
				uiDataTwoBytes=KE_Temperature_OutOfRange_time_HW;
				uaBuffer[16]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[17]=(uiDataTwoBytes)&0xff;
				//T_Lt_time
				uiDataTwoBytes=KE_Temperature_OutOfRange_time_SW*100;
				uaBuffer[18]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[19]=(uiDataTwoBytes)&0xff;
				
				break;
			case PID_PLUG_IN_CHARGE_STATUS:
				uiSendSize=21;
				//plug in status
				uiDataOneByte=0;
#if 0
				if(!isACPresent(0)) uiDataOneByte=0;
				else if((chargerStatus[0].chargeSection==2) ||(chargerStatus[0].chargeSection==3)) uiDataOneByte=2;
				else if(chargerStatus[0].chargeSection==6) uiDataOneByte=3;
				else if(chargerStatus[0].chargeSection==4) uiDataOneByte=4;
				else if(chargerStatus[0].readyToChg) uiDataOneByte=1;
#else
				if(!isACPresent(0)) uiDataOneByte=0;
				else uiDataOneByte=systemInfo.plugInStatus;	
#endif
				uaBuffer[0]=uiDataOneByte;	
				//reserved
				//Number of Cells balancing 
				uaBuffer[1]=uiGetBalanceCellNo();
				
				//Charge time
				uiDataTwoBytes=chargerStatus[0].totalChargeTime*60;
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;

				//Time plugged in
				uiDataTwoBytes=chargerStatus[0].plugInTime*60;
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;

				//Estimated time to complete
				uiDataTwoBytes=chargerStatus[0].spareTime*60;
				uaBuffer[6]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[7]=(uiDataTwoBytes)&0xff;

				//Charge Time during Plug in Charge (I >.1)
				uiDataTwoBytes=chargerStatus[0].ChgTimePlugIn*60;
				uaBuffer[8]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[9]=(uiDataTwoBytes)&0xff;

				//Time plugged in (EERPOM)-
				//life time plugged in 
				uiDataFourBytes=(eeprom_ram_para.TimPluggedInEep/60)*24/10;
				uaBuffer[10]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[11]=(uiDataFourBytes)&0xff;

				//Balance time during Plug in charge 
				uiDataTwoBytes=chargerStatus[0].balanceTime*60;
				uaBuffer[12]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[13]=(uiDataTwoBytes)&0xff;

				//Balance Power ( P=(4V^2)/R)*  # of cells balancing)
				uiDataFourBytes=chargerStatus[0].EngLostToBalance/100;//  unit 1 V.A.S
				uiDataFourBytes=uiDataFourBytes/1000*6536/3600;		//kwh
				uaBuffer[14]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[15]=(uiDataFourBytes)&0xff;

				uiDataFourBytes=chargerStatus[0].BalancePower;
				uiDataFourBytes=uiDataFourBytes*28249/100000;
				//Energy lost to balance
				uaBuffer[16]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[17]=(uiDataFourBytes)&0xff;

				//Volt differential at the end of Plug in Charge
				uiDataTwoBytes=chargerStatus[0].CellDiffChgEnd;
				uaBuffer[18]=(uiDataTwoBytes>>8)&0xff;;				
				uaBuffer[19]=(uiDataTwoBytes)&0xff;;

				//Iterartions - Charge and balance \ Cool
				uaBuffer[20]=(batteryPackInfo[0].cycleCount>=0xff)? 0xff:batteryPackInfo[0].cycleCount;				
				break;
			case PID_BALANCE_STATUS_PER_KEY_CYCLE:
				uiSendSize=18;
				//Balance Status 
//				uaBuffer[0]=0x00;				
				//Number of Cells balancing
				uaBuffer[1]=uiGetBalanceCellNo();
				if(uaBuffer[1]>0){
					uaBuffer[0]=0x01;//balancing
				}
				else{
					uaBuffer[0]=0x00;//not balance
				}
				//Drive time
				uiDataTwoBytes=batteryPackInfo[0].DriveTime*60;
				uaBuffer[2]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[3]=(uiDataTwoBytes)&0xff;	
				//Balance time during key Cycle
				uiDataTwoBytes=batteryPackInfo[0].BalTimKC*60;
				uaBuffer[4]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[5]=(uiDataTwoBytes)&0xff;
				//Balance time during key Cycle (EEPROM)
				uiDataFourBytes=(eeprom_ram_para.BalTimKCEep/60)*24/10;
				uaBuffer[6]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[7]=(uiDataFourBytes)&0xff;

				//Balance time during key Cycle (Plug In)
				uiDataTwoBytes=chargerStatus[0].sumOfBalanceTime*60;
				uaBuffer[8]=(uiDataTwoBytes>>8)&0xff;
				uaBuffer[9]=(uiDataTwoBytes)&0xff;
				//Balance time during key Cycle (Plug In) (EEPROM)
				uiDataFourBytes=(eeprom_ram_para.BalTimKCPlugInEep/60)*24/10;
				uaBuffer[10]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[11]=(uiDataFourBytes)&0xff;
				
				//Balance Power ( P=(4V^2)/R)*  # of cells balancing)
				uiDataFourBytes=batteryPackInfo[0].EngLostToBalance/100;//  unit 1 V.A.S
				uiDataFourBytes=uiDataFourBytes/1000*6536/3600;		//kwh
				uaBuffer[12]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[13]=(uiDataFourBytes)&0xff;

				//Energy lost to balance (EEPROM)
				uiDataFourBytes=batteryPackInfo[0].BalancePower;
				uiDataFourBytes=uiDataFourBytes*28249/100000;
				uaBuffer[14]=(uiDataFourBytes>>8)&0xff;
				uaBuffer[15]=(uiDataFourBytes)&0xff;

				//Target Volt differential at the end of balance
#if 0//to meet eng file
				uaBuffer[15]=0x00;				
				uaBuffer[16]=0x05;		//5mv+0mv		
#else
				uaBuffer[16]=0x00;				
				uaBuffer[17]=0x05;		//5mv+0mv		
#endif
				break;
			case PID_SOC_HIDDEN_WRITTEN:
				//$A029 - State of Charge  - Hidden Written 
				uiSendSize=1;
				//State of Charge
				uaBuffer[0]=UDSWriteInfo.uiHiddenSOC;				
				break;
			case PID_CAN_LABEL_WRITER:
				//$A030 - CAN Label Writer  (This will re-write the label of the pack) 
				uiSendSize=1;
				//Pack Number
				uaBuffer[0]=UDSWriteInfo.uiCANLable_PackNo_H;	
//				uaBuffer[1]=UDSWriteInfo.uiCANLable_PackNo_L;	
				break;
			case PID_FORCED_WRITE_TO_EEPROM:
				uiSendSize=1;
				uaBuffer[0]=UDSWriteInfo.uiForceToWriteFlag;	
				break;
			case PID_DTC_ENABLER:
				uiSendSize=20;
				uvCopyBuffer(&uaBuffer[0], &UDSWriteInfo.uiDTCGroupEnabler[0],uiSendSize);
				break;
			case PID_COIL_STATUS:
				uiSendSize=3;
				uiDataBits.uiDataByte=0;
				uiDataBits.Bit0=systemInfo.mainRelayPosInfo.coilOpenLoad;
				uiDataBits.Bit1=systemInfo.mainRelayPosInfo.coilShort;	
				uiDataBits.Bit2=systemInfo.mainRelayNegInfo.coilOpenLoad;
				uiDataBits.Bit3=systemInfo.mainRelayNegInfo.coilShort;	
				uiDataBits.Bit4=systemInfo.preChargeRelayInfo.coilOpenLoad;
				uiDataBits.Bit5=systemInfo.preChargeRelayInfo.coilShort;	
				uaBuffer[0]=	uiDataBits.uiDataByte;
				uiDataBits.uiDataByte=0;
				uiDataBits.Bit0=systemInfo.chgRelayPosInfo.coilOpenLoad;
				uiDataBits.Bit1=systemInfo.chgRelayPosInfo.coilShort;	
				uiDataBits.Bit2=systemInfo.chgRelayNegInfo.coilOpenLoad;
				uiDataBits.Bit3=systemInfo.chgRelayNegInfo.coilShort;
				uaBuffer[1]=uiDataBits.uiDataByte;
				uiDataBits.uiDataByte=0;
				uiDataBits.Bit0=systemInfo.OCDRelayInfo.coilOpenLoad;
				uiDataBits.Bit1=systemInfo.OCDRelayInfo.coilShort;
				if(MCU_RELAY_OCD==1){//close
					uiDataBits.Bit2=1;
				}
				else{
					uiDataBits.Bit2=0;
				}
				uaBuffer[2]=uiDataBits.uiDataByte;
				break;
			case PID_LOSE_COMM_STATUS:
				uiSendSize=3;
				uaBuffer[0]=DTCLongTable[DTC_CAN_COMM_LOSS_MODULE].DTCStatus;
				uaBuffer[1]=uiDTCLoseModule;
				uaBuffer[2]=DTCLongTable[DTC_CAN_COMM_LOSS_HVB].DTCStatus;
//				uaBuffer[0]=	uiDataBits.uiDataByte;
				break;
			case PID_CELL_SENSOR_STATUS:
				uiSendSize=8;
				
				uaBuffer[0]=DTCLongTable[DTC_MODULE_1_SENSOR_FAILURE].DTCStatus;
				uaBuffer[1]=DTCLongTable[DTC_MODULE_2_SENSOR_FAILURE].DTCStatus;
				uaBuffer[2]=DTCLongTable[DTC_MODULE_3_SENSOR_FAILURE].DTCStatus;
				uaBuffer[3]=DTCLongTable[DTC_MODULE_4_SENSOR_FAILURE].DTCStatus;
				uaBuffer[4]=DTCLongTable[DTC_MODULE_5_SENSOR_FAILURE].DTCStatus;
				uaBuffer[5]=DTCLongTable[DTC_MODULE_6_SENSOR_FAILURE].DTCStatus;
				uaBuffer[6]=DTCLongTable[DTC_MODULE_7_SENSOR_FAILURE].DTCStatus;
				uaBuffer[7]=DTCLongTable[DTC_MODULE_8_SENSOR_FAILURE].DTCStatus;
				break;
			case PID_SENSOR_STATUS:
#if 1//Added by Amanda August 5
				uiSendSize=15+3;				
#else
				uiSendSize=15;				
#endif
				uaBuffer[0]=DTCLongTable[DTC_COOLANT_INLET_OUT_OF_RANGE_HIGH].DTCStatus;
				uaBuffer[1]=DTCLongTable[DTC_COOLANT_INLET_OUT_OF_RANGE_LOW].DTCStatus;
				uaBuffer[2]=DTCLongTable[DTC_COOLANT_OUTLET_OUT_OF_RANGE_HIGH].DTCStatus;
				uaBuffer[3]=DTCLongTable[DTC_COOLANT_OUTLET_OUT_OF_RANGE_LOW].DTCStatus;

				uaBuffer[4]=DTCLongTable[DTC_CURR_SENSOR_H].DTCStatus;
				uaBuffer[5]=DTCLongTable[DTC_CURR_SENSOR_L].DTCStatus;

				uaBuffer[6]=DTCLongTable[DTC_BATTERY_MODULE_OV].DTCStatus;
				uaBuffer[7]=DTCLongTable[DTC_BATTERY_MODULE_UV].DTCStatus;

				uaBuffer[8]=DTCLongTable[DTC_VOL_SENSOR_H].DTCStatus;
				uaBuffer[9]=DTCLongTable[DTC_VOL_SENSOR_L].DTCStatus;
				uaBuffer[10]=DTCLongTable[DTC_CHARGER_VOLTAGE_FAIL].DTCStatus;
				uaBuffer[11]=DTCLongTable[DTC_HEATER_FUSE_FAIL].DTCStatus;
				uaBuffer[12]=DTCLongTable[DTC_CHARGER_FUSE_FAIL].DTCStatus;
				uaBuffer[13]=DTCLongTable[DTC_POSBAT_TO_GROUND_VOL_FAIL].DTCStatus;
				uaBuffer[14]=DTCLongTable[DTC_NEGBAT_TO_GROUND_VOL_FAIL].DTCStatus;
#if 1//Added by Amanda August 5
				uaBuffer[15]=DTCLongTable[DTC_BATT_SENSOR_FAIL].DTCStatus;
				uaBuffer[16]=DTCLongTable[DTC_CHGPOS_TO_BUSNEG_VOL_FAIL].DTCStatus;
				uaBuffer[17]=DTCLongTable[DTC_CHGNEG_TO_BUSPOS_VOL_FAIL].DTCStatus;
#else
#endif
				break;
			case PID_ISOLATION_RI_CLS:	//A054
				uiSendSize=2;
				uaBuffer[0]=(systemInfo.isolationInfo.Ri_cls>>8) & 0xFF;
				uaBuffer[1]=systemInfo.isolationInfo.Ri_cls & 0xFF;
				break;
			case PID_ISOLATION_RI_OPN:	//A055
				uiSendSize=2;
				uaBuffer[0]=(systemInfo.isolationInfo.Ri_opn>>8) & 0xFF;
				uaBuffer[1]=systemInfo.isolationInfo.Ri_opn & 0xFF;
				break;

			default:
				return 0xff;
				break;
		}
		}
	
	}
	return uiPIDNo;
}

UINT8 uiCheckPIDTbl(void)
{
	UINT8 i,j;
	if((UDSRvDataPara.uiSUB[0]==0xA1)&&
		(UDSRvDataPara.uiSUB[1]<=0x07)){//cell max number is 96
		i=PID_MAX+UDSRvDataPara.uiSUB[1];
		return i;
	}
#ifdef TEMP_SENSOR_96
	else if((UDSRvDataPara.uiSUB[0]==0xA2)&&
		(UDSRvDataPara.uiSUB[1]<=0x05)){//sensor max number is 64
		i=PID_MAX+0x08+UDSRvDataPara.uiSUB[1];
		return i;
	}
	else if((UDSRvDataPara.uiSUB[0]==0xA2)&&
		((UDSRvDataPara.uiSUB[1]>=0x11)&&(UDSRvDataPara.uiSUB[1]<=0x18))){//8 Modules
		i=PID_MAX+0x08+0x06+(UDSRvDataPara.uiSUB[1]-0x11);
		return i;
	}
#else
	else if((UDSRvDataPara.uiSUB[0]==0xA2)&&
		((UDSRvDataPara.uiSUB[1]>=0x00)&&(UDSRvDataPara.uiSUB[1]<=0x03))){//sensor max number is 64
		i=PID_MAX+0x08+UDSRvDataPara.uiSUB[1];
		return i;
	}
	else if((UDSRvDataPara.uiSUB[0]==0xA2)&&
		((UDSRvDataPara.uiSUB[1]>=0x11)&&(UDSRvDataPara.uiSUB[1]<=0x18))){//8 Modules
		i=PID_MAX+0x08+0x04+(UDSRvDataPara.uiSUB[1]-0x11);
		return i;
	}
#endif

	else{
		if(UDSRvDataPara.uiSUB[0]==0xA0){
			for(i=0;i<PID_MAX;i++){//max 
				if(UDSRvDataPara.uiSUB[1]==uaPIDChkTbl[i]){
					return i;
				}
			}
		}
		return 0xff;
	}
}
UINT8 uiGetBalanceCellNo(void)
{
	UINT8 uiTemp=0;
	UINT8 i,j;

	for(i=0;i<4;i++){
		for(j=0;j<32;j++){
			if((batteryPackInfo[0].cellBleedingFalg[i]>>j)&0x0001){
				uiTemp++;
			}
		}
	}
	return uiTemp;		

}


#endif
