#include "main.h"
#include "shed_ctrl.h"
static UNS8 shnd_stat;
void schnider_parse(J1939_MESSAGE *msg);

static void c2_snd_chgcfg_11700(void);
static void c2_process_chgCfgBulk_11700(uint8_t scrAddr,uint8_t *ptData);

J1939_MESSAGE jmsg;
CHG_CFG_BULK_STRUCT chgCfgBulk_info[3];

void schnider_ctrl(void){
	switch(shnd_stat){
	case 0:
		c2_snd_chgcfg_11700();
		shnd_stat++;
		break;
	default:
		shnd_stat=0;
		break;
		
	}
}
void schnider_parse(J1939_MESSAGE *revMsg){
	J1939_MESSAGE msg;
	memcpy(&msg,revMsg,sizeof(jmsg));
		if(msg.pgn==0x11700){
			c2_process_chgCfgBulk_11700(msg.jid.j1939_id.SourceAddress,msg.data);
		}
				
	
}
void c2_snd_chgcfg_11700(void){
	static UNS8 snd_id;
	uint8_t invId;
	snd_id++;
	invId=1;
	jmsg.pgn=0x11700;
	J1939IdAssb_pgn(jmsg.pgn,&jmsg);
	jmsg.dlc=8;
	jmsg.jid.j1939_id.Priority=6;
	jmsg.jid.j1939_id.PDUSpecific=2;
	jmsg.jid.j1939_id.SourceAddress=12;

	chgCfgBulk_info[invId].sndId=1;
	chgCfgBulk_info[invId].cfgType=2;
	chgCfgBulk_info[invId].msgCnt=3;
	chgCfgBulk_info[invId].conCurAccCtr=4;
	chgCfgBulk_info[invId].dcSrcId=5;
	chgCfgBulk_info[invId].battType=6;

	chgCfgBulk_info[invId].desiredBulkV=7;
	chgCfgBulk_info[invId].desiredBulkVMin=8;
	chgCfgBulk_info[invId].desiredBulkVMax=9;

	chgCfgBulk_info[invId].desiredBulkI=10;
	chgCfgBulk_info[invId].desiredBulkIMin=11;
	chgCfgBulk_info[invId].desiredBulkIMax=12;
	chgCfgBulk_info[invId].bulkTermV=13;
	chgCfgBulk_info[invId].bulkTermVmin=14;
	chgCfgBulk_info[invId].bulkTermVmax=15;
	chgCfgBulk_info[invId].bulkTermTime=16;
	chgCfgBulk_info[invId].bulkTermTimeMin=17;
	chgCfgBulk_info[invId].bulkTermTimeMax=18;
	chgCfgBulk_info[invId].bulkPeriodTo=19;
	chgCfgBulk_info[invId].bulkPeriodToMin=20;
	chgCfgBulk_info[invId].bulkPeriodToMax=21;

	
	jmsg.data[0]=((snd_id<<5)&0xE0);
	jmsg.data[1]=52;
	jmsg.data[2]=(chgCfgBulk_info[invId].msgCnt<<2)+chgCfgBulk_info[invId].cfgType;
	jmsg.data[3]=(chgCfgBulk_info[invId].conCurAccCtr);
	jmsg.data[4]=chgCfgBulk_info[invId].dcSrcId;
	jmsg.data[5]=chgCfgBulk_info[invId].battType;
	jmsg.data[6]=chgCfgBulk_info[invId].desiredBulkV & 0xFF;
	jmsg.data[7]=(chgCfgBulk_info[invId].desiredBulkV>>8) & 0xFF;
	j1939_send(CAN2,&jmsg);
	jmsg.data[0]=((snd_id<<5)&0xE0)+1;
	jmsg.data[1]=(chgCfgBulk_info[invId].desiredBulkV>>16) & 0xFF;
	jmsg.data[2]=(chgCfgBulk_info[invId].desiredBulkV>>24) & 0xFF;
	jmsg.data[3]=chgCfgBulk_info[invId].desiredBulkVMin & 0xFF;
	jmsg.data[4]=(chgCfgBulk_info[invId].desiredBulkVMin>>8) & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].desiredBulkVMin>>16) & 0xFF;
	jmsg.data[6]=(chgCfgBulk_info[invId].desiredBulkVMin>>24) & 0xFF;
	jmsg.data[7]=chgCfgBulk_info[invId].desiredBulkVMax & 0xFF;
	j1939_send(CAN2,&jmsg);
	jmsg.data[0]=((snd_id<<5)&0xE0)+2;
	jmsg.data[1]=(chgCfgBulk_info[invId].desiredBulkVMax>>8) & 0xFF;
	jmsg.data[2]=(chgCfgBulk_info[invId].desiredBulkVMax>>16) & 0xFF;
	jmsg.data[3]=(chgCfgBulk_info[invId].desiredBulkVMax>>24) & 0xFF;
	jmsg.data[4]=chgCfgBulk_info[invId].desiredBulkI & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].desiredBulkI>>8) & 0xFF;
	jmsg.data[6]=(chgCfgBulk_info[invId].desiredBulkI>>16) & 0xFF;
	jmsg.data[7]=(chgCfgBulk_info[invId].desiredBulkI>>24) & 0xFF;
	j1939_send(CAN2,&jmsg);
	jmsg.data[0]=((snd_id<<5)&0xE0)+3;
	jmsg.data[1]=(chgCfgBulk_info[invId].desiredBulkIMin & 0xFF);
	jmsg.data[2]=(chgCfgBulk_info[invId].desiredBulkIMin>>8) & 0xFF;
	jmsg.data[3]=(chgCfgBulk_info[invId].desiredBulkIMin>>16) & 0xFF;
	jmsg.data[4]=(chgCfgBulk_info[invId].desiredBulkIMin>>24) & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].desiredBulkIMax & 0xFF);
	jmsg.data[6]=(chgCfgBulk_info[invId].desiredBulkIMax>>8) & 0xFF;
	jmsg.data[7]=(chgCfgBulk_info[invId].desiredBulkIMax>>16) & 0xFF;
	j1939_send(CAN2,&jmsg);
	
	jmsg.data[0]=((snd_id<<5)&0xE0)+4;
	jmsg.data[1]=(chgCfgBulk_info[invId].desiredBulkIMax>>24) & 0xFF;
	jmsg.data[2]=(chgCfgBulk_info[invId].bulkTermV & 0xFF);
	jmsg.data[3]=(chgCfgBulk_info[invId].bulkTermV>>8) & 0xFF;
	jmsg.data[4]=(chgCfgBulk_info[invId].bulkTermV>>16) & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].bulkTermV>>24) & 0xFF;
	jmsg.data[6]=(chgCfgBulk_info[invId].bulkTermVmin & 0xFF);
	jmsg.data[7]=(chgCfgBulk_info[invId].bulkTermVmin>>8) & 0xFF;
	j1939_send(CAN2,&jmsg);
	
	jmsg.data[0]=((snd_id<<5)&0xE0)+5;
	
	jmsg.data[1]=(chgCfgBulk_info[invId].bulkTermVmin>>16) & 0xFF;
	jmsg.data[2]=(chgCfgBulk_info[invId].bulkTermVmin>>24) & 0xFF;
	jmsg.data[3]=chgCfgBulk_info[invId].bulkTermVmax & 0xFF;
	jmsg.data[4]=(chgCfgBulk_info[invId].bulkTermVmax>>8) & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].bulkTermVmax>>16) & 0xFF;
	jmsg.data[6]=(chgCfgBulk_info[invId].bulkTermVmax>>24) & 0xFF;
	jmsg.data[7]=(chgCfgBulk_info[invId].bulkTermTime & 0xFF);
	j1939_send(CAN2,&jmsg);
	jmsg.data[0]=((snd_id<<5)&0xE0)+6;
	jmsg.data[1]=(chgCfgBulk_info[invId].bulkTermTime>>8) & 0xFF;
	jmsg.data[2]=(chgCfgBulk_info[invId].bulkTermTime>>16) & 0xFF;
	jmsg.data[3]=(chgCfgBulk_info[invId].bulkTermTime>>24) & 0xFF;
	jmsg.data[4]=chgCfgBulk_info[invId].bulkTermTimeMax & 0xFF;
	jmsg.data[5]=(chgCfgBulk_info[invId].bulkTermTimeMax>>8) & 0xFF;
	jmsg.data[6]=chgCfgBulk_info[invId].bulkPeriodTo & 0xFF;
	jmsg.data[7]=(chgCfgBulk_info[invId].bulkPeriodTo>>8) & 0xFF;
	j1939_send(CAN2,&jmsg);

	jmsg.data[0]=((snd_id<<5)&0xE0)+7;
	jmsg.data[1]=(chgCfgBulk_info[invId].bulkPeriodToMin & 0xFF);
	jmsg.data[2]=(chgCfgBulk_info[invId].bulkPeriodToMin>>8) & 0xFF;
	jmsg.data[3]=(chgCfgBulk_info[invId].bulkPeriodToMax & 0xFF);
	jmsg.data[4]=(chgCfgBulk_info[invId].bulkPeriodToMax>>8) & 0xFF;
	jmsg.data[5]=0xFF;
	jmsg.data[6]=0xFF;
	jmsg.data[7]=0xFF;
	j1939_send(CAN2,&jmsg);
	snd_id++;

}
void c2_process_chgCfgBulk_11700(uint8_t scrAddr,uint8_t *ptData){
	uint8_t invId;
	uint8_t cd[8];
	static uint32_t tempDword;
	memcpy(cd,ptData,8);
	invId=0;
	
	if(((cd[0] & 0x1F)==0) && (cd[1]==52)) {
		chgCfgBulk_info[invId].cfgType=cd[2] & 0x3;
		chgCfgBulk_info[invId].msgCnt=(cd[2]>>2) & 0x3F;
		chgCfgBulk_info[invId].conCurAccCtr=cd[3];
		chgCfgBulk_info[invId].dcSrcId=cd[4];
		chgCfgBulk_info[invId].battType=cd[5];
		chgCfgBulk_info[invId].desiredBulkV=((uint32_t)cd[7]<<8)+cd[6];
	}else if((cd[0] & 0x1F)==1){
		chgCfgBulk_info[invId].desiredBulkV +=((uint32_t)cd[2]<<24)+((uint32_t)cd[1]<<16);
		chgCfgBulk_info[invId].desiredBulkVMin=((uint32_t)cd[4]<<8)+cd[3];
		chgCfgBulk_info[invId].desiredBulkVMin+=((uint32_t)cd[6]<<24)+((uint32_t)cd[5]<<16);
		chgCfgBulk_info[invId].desiredBulkVMax=cd[7];
	}else if((cd[0] & 0x1F)==2) {
		 chgCfgBulk_info[invId].desiredBulkVMax+=((uint32_t)cd[3]<<24)+((uint32_t)cd[2]<<16)+((uint32_t)cd[1]<<8);
		 chgCfgBulk_info[invId].desiredBulkI=((uint32_t)cd[7]<<24)+((uint32_t)cd[6]<<16)+((uint32_t)cd[5]<<8)+((uint32_t)cd[4]);
	}else if((cd[0] & 0x1F)==3) {
		chgCfgBulk_info[invId].desiredBulkIMin=((int32_t)cd[4]<<24)+((int32_t)cd[3]<<16)+((int32_t)cd[2]<<8)+((int32_t)cd[1]);
		tempDword=((uint32_t)cd[7]<<16)+((uint32_t)cd[6]<<8)+((uint32_t)cd[5]);

 	}else if((cd[0] & 0x1F)==4) {
		 chgCfgBulk_info[invId].desiredBulkIMax=((uint32_t)cd[1]<<24)+tempDword;
		 chgCfgBulk_info[invId].bulkTermV=((uint32_t)cd[5]<<24)+((uint32_t)cd[4]<<16)+((uint32_t)cd[3]<<8)+((uint32_t)cd[2]);
		tempDword=((uint32_t)cd[7]<<8)+((uint32_t)cd[6]);
	}else if((cd[0] & 0x1F)==5) {
		 chgCfgBulk_info[invId].bulkTermVmin=((uint32_t)cd[2]<<24)+((uint32_t)cd[14]<<16)+tempDword;
		 chgCfgBulk_info[invId].bulkTermVmax=((uint32_t)cd[6]<<24)+((uint32_t)cd[5]<<16)+((uint32_t)cd[4]<<8)+((uint32_t)cd[3]);
		chgCfgBulk_info[invId].bulkTermTime= (uint32_t)cd[7];
	}else if((cd[0] & 0x1F)==6) {
		chgCfgBulk_info[invId].bulkTermTime+= ((uint32_t)cd[1]<<8);
		chgCfgBulk_info[invId].bulkTermTimeMin= ((uint16_t)cd[3]<<8)+((uint16_t)cd[2]);
		chgCfgBulk_info[invId].bulkTermTimeMax= ((uint16_t)cd[5]<<8)+((uint16_t)cd[4]);
		chgCfgBulk_info[invId].bulkPeriodTo= ((uint16_t)cd[7]<<8)+((uint16_t)cd[6]);

	}else if((cd[0] & 0x1F)==7) {
		chgCfgBulk_info[invId].bulkPeriodToMin= ((uint16_t)cd[2]<<8)+((uint16_t)cd[1]);
		chgCfgBulk_info[invId].bulkPeriodToMax=((uint16_t)cd[4]<<8)+((uint16_t)cd[3]);

	}
}
