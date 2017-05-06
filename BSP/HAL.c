#include "main.h"
#include "task_main.h"
#include "HAL.h"
#define ON 1
#define OFF 0
void relayControl(uint8_t rl, uint8_t status);
UINT8 getRelayStatus( UINT8 relayName);
void sstrMainRLCtrl(UINT8 sid,UINT8 status){
	switch(sid){
		case 0:
			if(status) LS3_ON;
			else LS3_OFF;
			relayControl(RELAY_SSTR_1, status);
			break;
		case 1:
			relayControl(RELAY_SSTR_2, status);
			break;
		case 2:
			relayControl(RELAY_SSTR_3, status);
			break;
		case 3:
			relayControl(RELAY_SSTR_4, status);
			break;
		case 4:
			relayControl(RELAY_SSTR_5, status);
			break;
		case 5:
			relayControl(RELAY_SSTR_6, status);
			break;
		case 6:
			relayControl(RELAY_SSTR_7, status);
			break;
		case 7:
			relayControl(RELAY_SSTR_8, status);
			break;
		default:
			break;
	}

}
void sstrBalRLCtrl(UINT8 sid,UINT8 status){
	switch(sid){
		case 0:
			relayControl(RELAY_BAL_1, status);
			break;
		case 1:
			relayControl(RELAY_BAL_2, status);
			break;
		case 2:
			relayControl(RELAY_BAL_3, status);
			break;
		case 3:
			relayControl(RELAY_BAL_4, status);
			break;
		default:
			break;
	}

}
void relayControl(UINT8 relayName, UINT8 status){
	#define RELAY_DELAY_MILLISECONDS	500
	int i;

	switch(relayName){
		case RL_MAIN_POS:
			if(status==1){
				sysInfo.mainRelayPosInfo.actionCmd=1;
				LS1_ON;
			}else{
				sysInfo.mainRelayPosInfo.actionCmd=0;
				LS1_OFF;
			}
			break;
		case RL_MAIN_NEG:
			if(status==1){
				sysInfo.mainRelayNegInfo.actionCmd=1;
				LS1_ON;
			}else{
				sysInfo.mainRelayNegInfo.actionCmd=0;
				LS1_OFF;
			}
			break;
		case RL_PCHG:
			if(status==1){
				sysInfo.pchgRlInfo.actionCmd=1;
				HS3_ON;
			}else{
				sysInfo.pchgRlInfo.actionCmd=0;
				HS3_OFF;
			}
			break;
		case RL_CHG_POS:
			if((status==ON) && (!sysInfo.chgRelayPosInfo.actionCmd)){	//now is open
				sysInfo.chgRelayPosInfo.closeTick=OSTime;
				sysInfo.chgRelayPosInfo.actionTick=OSTime;
				LS2_ON;
			}else if((status==OFF) && sysInfo.chgRelayPosInfo.actionCmd){	//now is close
				sysInfo.chgRelayPosInfo.openTick=OSTime;
				sysInfo.chgRelayPosInfo.actionTick=OSTime;
				LS2_OFF;
			}
 			sysInfo.chgRelayPosInfo.actionCmd=status;
			break; 
		case CHG_RELAY_NEG:
			if((status==ON) && (!sysInfo.chgRelayNegInfo.actionCmd)){	//now is open
				sysInfo.chgRelayNegInfo.closeTick=OSTime;
				sysInfo.chgRelayNegInfo.actionTick=OSTime;
			}else if((status==OFF) && sysInfo.chgRelayNegInfo.actionCmd){	//now is close
				sysInfo.chgRelayNegInfo.openTick=OSTime;
				sysInfo.chgRelayNegInfo.actionTick=OSTime;
			}
 			sysInfo.chgRelayNegInfo.actionCmd=status;
			break;
		case RELAY_BAL_1:
			if((status==ON) && (!sysInfo.balRLInfo[0].actionCmd)){	//now is open
				sysInfo.balRLInfo[0].closeTick=OSTime;
				sysInfo.balRLInfo[0].actionTick=OSTime;
			}else if((status==OFF) && sysInfo.balRLInfo[0].actionCmd){	//now is close
				sysInfo.balRLInfo[0].openTick=OSTime;
				sysInfo.balRLInfo[0].actionTick=OSTime;
			}
 			sysInfo.balRLInfo[0].actionCmd=status;
			break;
		case RELAY_BAL_2:
 			sysInfo.balRLInfo[1].actionCmd=status;
			break;
		case RELAY_BAL_3:
 			sysInfo.balRLInfo[2].actionCmd=status;
			break;
		case RELAY_BAL_4:
 			sysInfo.balRLInfo[3].actionCmd=status;
			break;
		case RELAY_SSTR_1:
			if((status==ON) && (!sysInfo.strRL[0].actionCmd)){	//now is open
				sysInfo.strRL[0].closeTick=OSTime;
				sysInfo.strRL[0].actionTick=OSTime;
				HS1_ON;
			}else if((status==OFF) && sysInfo.strRL[0].actionCmd){	//now is close
				sysInfo.strRL[0].openTick=OSTime;
				sysInfo.strRL[0].actionTick=OSTime;
				HS1_OFF;
			}
 			sysInfo.strRL[0].actionCmd=status;
			break;
		case RELAY_SSTR_2:
			if((status==ON) && (!sysInfo.strRL[1].actionCmd)){	//now is open
				sysInfo.strRL[1].closeTick=OSTime;
				sysInfo.strRL[1].actionTick=OSTime;
				HS2_ON;
			}else if((status==OFF) && sysInfo.strRL[1].actionCmd){	//now is close
				sysInfo.strRL[1].openTick=OSTime;
				sysInfo.strRL[1].actionTick=OSTime;
				HS2_OFF;
			}
 			sysInfo.strRL[1].actionCmd=status;
			break;
		case RELAY_SSTR_3:
			if((status==ON) && (!sysInfo.strRL[2].actionCmd)){	//now is open
				sysInfo.strRL[2].closeTick=OSTime;
				sysInfo.strRL[2].actionTick=OSTime;
				HS3_ON;
			}else if((status==OFF) && sysInfo.strRL[2].actionCmd){	//now is close
				sysInfo.strRL[2].openTick=OSTime;
				sysInfo.strRL[2].actionTick=OSTime;
				HS3_OFF;
			}
 			sysInfo.strRL[2].actionCmd=status;
			break;
		case RELAY_SSTR_4:
			if((status==ON) && (!sysInfo.strRL[3].actionCmd)){	//now is open
				sysInfo.strRL[3].closeTick=OSTime;
				sysInfo.strRL[3].actionTick=OSTime;
				HS4_ON;
			}else if((status==OFF) && sysInfo.strRL[3].actionCmd){	//now is close
				sysInfo.strRL[3].openTick=OSTime;
				sysInfo.strRL[3].actionTick=OSTime;
				HS4_OFF;
			}
 			sysInfo.strRL[3].actionCmd=status;
			break;
		case RELAY_SSTR_5:
			if((status==ON) && (!sysInfo.strRL[4].actionCmd)){	//now is open
				sysInfo.strRL[4].closeTick=OSTime;
				sysInfo.strRL[4].actionTick=OSTime;
				
			}else if((status==OFF) && sysInfo.strRL[4].actionCmd){	//now is close
				sysInfo.strRL[4].openTick=OSTime;
				sysInfo.strRL[4].actionTick=OSTime;
				
			}
			sysInfo.strRL[4].actionCmd=status;
		break;
		case RELAY_SSTR_6:
			if((status==ON) && (!sysInfo.strRL[5].actionCmd)){	//now is open
				sysInfo.strRL[5].closeTick=OSTime;
				sysInfo.strRL[5].actionTick=OSTime;
				
			}else if((status==OFF) && sysInfo.strRL[5].actionCmd){	//now is close
				sysInfo.strRL[5].openTick=OSTime;
				sysInfo.strRL[5].actionTick=OSTime;
			}
			sysInfo.strRL[5].actionCmd=status;
			break;
		case RELAY_SSTR_7:
			if((status==ON) && (!sysInfo.strRL[6].actionCmd)){	//now is open
				sysInfo.strRL[6].closeTick=OSTime;
				sysInfo.strRL[6].actionTick=OSTime;
				
			}else if((status==OFF) && sysInfo.strRL[6].actionCmd){	//now is close
				sysInfo.strRL[6].openTick=OSTime;
				sysInfo.strRL[6].actionTick=OSTime;
			}
			sysInfo.strRL[6].actionCmd=status;
			break;
		case RELAY_SSTR_8:
			if((status==ON) && (!sysInfo.strRL[7].actionCmd)){	//now is open
				sysInfo.strRL[7].closeTick=OSTime;
				sysInfo.strRL[7].actionTick=OSTime;
				
			}else if((status==OFF) && sysInfo.strRL[7].actionCmd){	//now is close
				sysInfo.strRL[7].openTick=OSTime;
				sysInfo.strRL[7].actionTick=OSTime;
			}
			sysInfo.strRL[7].actionCmd=status;
			break;

		case RELAY_COLLING_1:
			break;
		case RELAY_COLLING_2:
			break;
		case RELAY_COLLING_3:
			break;
		case RELAY_COLLING_4:
			break;
		case RELAY_CAB_FAN:
			break;
		case RELAY_PUMP:
			break;
		case RL_HEATER_1:
			if(status) HS1_ON;
			else HS1_OFF;
			break;
		case RL_HEATER_2:
			if(status) HS2_ON;
			else HS2_OFF;
			break;
		case RL_DCDC:
			if(status) HS3_ON;
			else HS3_OFF;
			break;
		case RELAY_FAN_PWR:
			break;
		default:
			break;
	}
}


UINT8 getSStrRelayStatus( UINT8 sid){
	if(sid==0) return getRelayStatus(RELAY_SSTR_1);
	else if(sid==1) return getRelayStatus(RELAY_SSTR_2);
	else if(sid==2) return getRelayStatus(RELAY_SSTR_3);
	else if(sid==3) return getRelayStatus(RELAY_SSTR_4);
}
UINT8 getSStrInterlockStatus( UINT8 sid){

	
}
UINT8 getHWInterLock(){
	/*
	if((sysCANDebugCmd.simByCAN)&&(sysCANDebugCmd.cabId==eeprom_ram_para.cabId)){
		return sysCANDebugCmd.simHWIntLock;
	}else{
		return MCU_HW_INTERLOCK;
	}
	*/
}
UINT8 getSStrFanRelayStatus( UINT8 sid){
	if(sid==0) return getRelayStatus(RELAY_COLLING_1);
	else if(sid==1) return getRelayStatus(RELAY_COLLING_2);
	else if(sid==2) return getRelayStatus(RELAY_COLLING_3);
	else if(sid==3) return getRelayStatus(RELAY_COLLING_4);

}
UINT8 getSStrBalRelayStatus( UINT8 sid){
	if(sid==0) return getRelayStatus(RELAY_BAL_1);
	else if(sid==1) return getRelayStatus(RELAY_BAL_2);
	else if(sid==2) return getRelayStatus(RELAY_BAL_3);
	else if(sid==3) return getRelayStatus(RELAY_BAL_4);
}
UINT8 getRelayStatus( UINT8 relayName){
	UINT8 status;
	switch(relayName){
		case RL_MAIN_NEG:
			status=sysInfo.mainRelayNegInfo.actionCmd;
			break;
		case RL_PCHG:
			status=sysInfo.pchgRlInfo.actionCmd;
			break;
		case RELAY_MOTOR_LIMP:
			break;
		case RL_CHG_POS:
			status=sysInfo.chgRelayPosInfo.actionCmd;
		case CHG_RELAY_NEG:
			status=sysInfo.chgRelayPosInfo.actionCmd;
			break;
		case RELAY_SSTR_1:
			status=sysInfo.strRL[0].actionCmd;
			break;
		case RELAY_SSTR_2:
			status=sysInfo.strRL[1].actionCmd;
			break;
		case RELAY_SSTR_3:
			status=sysInfo.strRL[2].actionCmd;
			break;
		case RELAY_SSTR_4:
			status=sysInfo.strRL[3].actionCmd;
			break;
		case RELAY_BAL_1:
			status=sysInfo.balRLInfo[0].actionCmd;
			break;
		case RELAY_BAL_2:
			status=sysInfo.balRLInfo[1].actionCmd;
			break;
		case RELAY_BAL_3:
			status=sysInfo.balRLInfo[2].actionCmd;
			break;
		case RELAY_BAL_4:
			status=sysInfo.balRLInfo[3].actionCmd;
			break;
		case RELAY_COLLING_1:
			break;
		case RELAY_COLLING_2:
			break;
		case RELAY_COLLING_3:
			break;
		case RELAY_COLLING_4:
			break;
		case RELAY_CAB_FAN:
			break;
		case RELAY_PUMP:
			break;

		default:
			break;
	}
	return status;
}



UINT8 isPlugin(UINT8 packNo){
	UINT8 ret;
//	return TRUE;
	if(AC_PRESENT==1){
		
		return 1;
		
	}else{
		
		return 0;
	}
}



void strFanRlCtrl(UINT8 sid,UINT8 status){
	UINT8 i,ifOpen=1;
	switch(sid){
		case 0:
			relayControl(RELAY_COLLING_1, status);
			break;
		case 1:
			relayControl(RELAY_COLLING_2, status);
			break;
		case 2:
			relayControl(RELAY_COLLING_3, status);
			break;
		case 3:
			relayControl(RELAY_COLLING_4, status);
			break;
			
		default:
			break;
	}
			
}
				
		
void battPowerControl(UINT8 status){
	
	//if((status==ON)&&(BATTERY_BOARD_POWER_STATUS==OFF)){
	if(0){
		//batteryPowerTick=TickGet();
	}
	//if((status==OFF)&&(BATTERY_BOARD_POWER_STATUS==ON)){
		//batteryPowerOffTick=TickGet();
	//}
	
//	BATTERY_BOARD_POWER_SET(status);


}

