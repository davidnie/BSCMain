/***********************************************************************
***********************************************************************/
#include "main.h"
#include "def.h"
	
	OS_STK Stk_Task_AVChg[400];
	OS_EVENT  *sem_U4_rec_flag = 0; 		//UART4 recevied data
	
extern unsigned char U4_send_data[RS232_REC_BUFF_SIZE];

static void u4_buf_clr(void);
static 	uint8_t getCrc8(uint8_t * da,uint8_t length);
static uint8_t isValidCmd();
static void sendResponse(uint8_t * revData);

/***********************************************************************
***********************************************************************/
	void Task_AVChg(void *pdata)
	{		 
		unsigned  char	os_err,i,isCmdIn;
		Message RxMSG;
		unsigned char sdData[10]={1,2,3,4,6,7,8,9};
/*		while(1){
			
			OSTimeDly(1000);//
			U4_DMA_Send(sdData,9);
			U4_Send(sdData,4);
		}
*/
		sem_U4_rec_flag = OSSemCreate(1); //create a semphere. 
		while(1)
		{  
			OSSemPend(sem_U4_rec_flag,100,&os_err); 		//wait sem_CAN1_rec_flag being set in ISR
			if(U4_REC_Flag== 1)							//check if received valid message
			{
				U4_REC_Flag=0;
				if(U4_rec_counter>=4){
					isCmdIn=isValidCmd();
					if(isCmdIn){
						sendResponse(U4_buff);
						u4_buf_clr();
					}
				}
			}else{
				u4_buf_clr();
			}
		}
	}
uint8_t isValidCmd(){
	uint8_t cmd,crc8,ret,pg;
	if(U4_buff[0]<0x20){
		cmd=U4_buff[0];
	}else{
		cmd=U4_buff[0]& 0xF0;
		pg=U4_buff[0] & 0x0F;
	}
	if((cmd==0x60) &&(U4_rec_counter<5)){  //write 2 bytes
		return 0;
	}else if((cmd==0x80) &&(U4_rec_counter<7)){
		return 0;
	}else if(U4_rec_counter<4){
		return 0;
	}

			
	ret=0;
	switch(cmd){
		case 0x60: //request error status
			crc8=getCrc8(U4_buff,4);
			if(crc8=U4_buff[4]){
				ret=1;
			}else{
				ret=0;
			}
			break;
		case 0x80: //request error status
			crc8=getCrc8(U4_buff,6);
			if(crc8=U4_buff[6]){
				ret=1;
			}else{
				ret=0;
			}
			break;
		default: 
			crc8=getCrc8(U4_buff,3);
			if(crc8=U4_buff[3]){
				ret=1;
			}else{
				ret=0;
			}
			break;
	}
	return ret;
	
}
void sendResponse(uint8_t * revData){
	uint8_t cmd,EEAddr,pg;
	cmd=revData[0];
	if(revData[0]<0x20){
		cmd=revData[0];
	}else{
		cmd=revData[0]& 0xF0;
		pg=revData[0] & 0x0F;
	}
	switch(cmd){
		case 2:
			U4_send_data[0]=0;
			U4_send_data[1]=0;
			U4_send_data[2]=getCrc8(U4_send_data,2);
			U4_Send(U4_send_data,3);
			break;
		case 4:
			if(U4_buff[1]=0){
				U4_send_data[0]=1;
				U4_send_data[1]=1;
				U4_send_data[2]=0;
				U4_send_data[3]=getCrc8(U4_send_data,3);
				U4_Send(U4_send_data,4);
			}else{
				U4_send_data[0]=0;
				U4_send_data[1]=0;
				U4_send_data[2]=0;
				U4_send_data[3]=getCrc8(U4_send_data,3);
				U4_Send(U4_send_data,4);
			}
			break;
			case 7: //report voltage
				U4_send_data[0]=0x5F;	//low byte fraction parte. =0.37*256
				U4_send_data[1]=54; 	//high byte. whole part. 54.37V total
				U4_send_data[2]=0;
				U4_send_data[3]=getCrc8(U4_send_data,3);
				U4_Send(U4_send_data,4);
				break;
			case 0x18: //report voltage
				break;
			case 0x30: // write one byte to eeprom page
				EEAddr=revData[1];
				setEEData(&revData[2],EEAddr,pg,1);
				U4_send_data[0]=U4_buff[1];
				U4_send_data[1]=U4_buff[2];
				U4_send_data[2]=getCrc8(U4_send_data,2);
				U4_Send(U4_send_data,3);
				break;
			case 0x40: // read one byte from pg
				EEAddr=revData[1];
				U4_send_data[0]=EEAddr;
				getEEData(&U4_send_data[1],EEAddr,pg,1);
				U4_send_data[2]=getCrc8(U4_send_data,2);
				U4_Send(U4_send_data,3);
				break;
			case 0x50: // write protect EEPROM
				U4_send_data[0]=U4_buff[1];
				U4_send_data[1]=0;
				U4_send_data[2]=getCrc8(U4_send_data,2);
				U4_Send(U4_send_data,3);
				break;
			case 0x60: // write one byte to eeprom page
				EEAddr=revData[1];
				setEEData(&revData[2],EEAddr,pg,2);
				U4_send_data[0]=U4_buff[1];
				U4_send_data[1]=U4_buff[2];
				U4_send_data[2]=U4_buff[3];
				U4_send_data[3]=getCrc8(U4_send_data,3);
				U4_Send(U4_send_data,4);
				break;
			case 0x70: // read 2 byte from pg
				EEAddr=revData[1];
				U4_send_data[0]=EEAddr;
				getEEData(&U4_send_data[1],EEAddr,pg,2);
				U4_send_data[3]=getCrc8(U4_send_data,3);
				U4_Send(U4_send_data,4);
				break;
			case 0x80: // write 4 byte to eeprom page
				EEAddr=revData[1];
				setEEData(&revData[2],EEAddr,pg,2);
				U4_send_data[0]=U4_buff[1];
				U4_send_data[1]=U4_buff[2];
				U4_send_data[2]=U4_buff[3];
				U4_send_data[3]=U4_buff[4];
				U4_send_data[4]=U4_buff[5];
				U4_send_data[5]=getCrc8(U4_send_data,5);
				U4_Send(U4_send_data,6);
				break;
			case 0x90: // read 4 byte from pg
				EEAddr=revData[1];
				U4_send_data[0]=EEAddr;
				getEEData(&U4_send_data[1],EEAddr,pg,4);
				U4_send_data[5]=getCrc8(U4_send_data,5);
				U4_Send(U4_send_data,6);
				break;
			default:
				break;
	}
			
}
void u4_buf_clr(void){
	U4_rec_counter=0;
	U4_buff[0]=0;
}
uint8_t getCrc8(uint8_t * da,uint8_t length){
	uint16_t cword=0;
	uint8_t i,j,byte_low,byte_high,daj,byte_index,crc8;
	uint16_t icrctb[256]={0, 4129, 8258, 12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290,45419, 49548, 53677, 57806, 61935, 4657, 528, 12915, 8786, 21173,17044, 
		29431, 25302, 37689, 33560, 45947, 41818, 54205, 50076, 62463,58334, 9314, 13379,
		1056, 5121, 25830, 29895, 17572, 21637, 42346,46411, 34088, 38153, 58862, 62927, 50604, 54669, 13907, 9842, 5649,1584, 30423, 26358, 22165, 18100, 46939, 
		42874, 38681, 34616, 63455,59390, 55197, 51132, 18628, 22757, 26758, 30887, 2112, 6241, 10242,14371, 51660, 55789, 59790, 63919, 
		35144, 39273, 43274, 47403, 23285,19156, 31415, 27286, 6769, 2640, 14899, 10770, 56317, 52188, 64447,60318, 39801, 35672, 47931, 43802, 27814, 31879, 19684, 
		23749, 11298,15363, 3168, 7233, 60846, 64911, 52716, 56781, 44330, 48395, 36200,40265, 32407, 28342, 24277, 
		20212, 15891, 11826, 7761, 3696, 65439,61374, 57309, 53244, 48923, 44858, 40793, 36728, 37256, 33193, 45514,
		41451, 53516, 49453, 61774, 57711, 4224, 161, 12482, 8419, 20484,16421, 28742, 24679, 33721, 37784, 41979, 
		46042, 49981, 54044, 58239,62302, 689, 4752, 8947, 13010, 16949, 21012, 25207, 29270, 46570,42443, 38312, 
		34185, 62830, 58703, 54572, 50445, 13538, 9411, 5280,1153, 29798, 25671, 21540, 17413, 42971, 47098, 34713, 
		38840, 59231,63358, 50973, 55100, 9939, 14066, 1681, 5808, 26199, 30326, 17941,22068, 55628, 51565, 63758,
		59695, 39368, 35305, 47498, 43435, 22596,18533, 30726, 26663, 6336, 2273, 14466, 10403, 52093, 56156, 60223,
		64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,6864, 10931, 14994, 64814, 60687, 56684, 
		52557, 48554, 44427, 40424,36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392, 3265, 61215,65342, 53085, 
57212, 44955, 49082, 36825, 40952, 28183, 32310, 20053,24180, 11923, 16050, 3793, 7920};
	uint16_t icrctbj,xorbytelow;
	for(j=0;j<length;j++){
		byte_low = (uint8_t)(cword & 0xFF);
		byte_high = (uint8_t)((cword>>8)&0xFF);
		daj=da[j];
		icrctbj=icrctb[j];
		xorbytelow=(uint8_t)byte_low<<8;
		byte_index=da[j]^ byte_high;
		cword = (icrctb[byte_index]) ^((uint16_t)byte_low<<8);
		}
	crc8=(uint8_t)cword;
	return crc8;
}

getEEData(uint8_t * dest,uint8_t start_addr, uint8_t pageid,uint8_t len){
	uint8_t pg_f[100];
	uint8_t pg_8[200];
	uint8_t * scr;
	if(pageid==0xf){
		scr=pg_f;
	}else{
		scr=pg_8;
	}
	memcpy(dest,scr+start_addr,len);
}

setEEData(uint8_t * scr,uint8_t dest_addr, uint8_t pageid,uint8_t len){
	uint8_t pg_f[100];
	uint8_t pg_8[200];
	uint8_t * dest;
	if(pageid==0xf){
		dest=pg_f+dest_addr;
	}else{
		dest=pg_8+dest_addr;
	}
	memcpy(dest,scr,len);
}

