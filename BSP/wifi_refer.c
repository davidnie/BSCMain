/*************************************************************************
 Copyright (C), 2009, forsight Technology Co., Ltd.
 Version:        1.00   
  Author:         Henry                   
  Date:           2013-3-26 11:41
 ????:   Wifi?? 
 Check Sum:      0x17C6CC
  Chip Type:      
  Contact:       
*************************************************************************/
#define BUD9600
//#define BUD4800


#define    ONLINE    PB.6     // output,????LED
#define    LINK      PA.0

#define    R         PA.4   
#define    G         PC.5
#define    B         PC.4
#define    C         PC.1
#define    W         PC.0

#define    RESETKEY  PB.0

#define    p_TXD     PA.3     // output,RS232-TX
#define    p_RXD     PA.6     // input,RS232-RX
#define    p_READY   PA.5     // input, ?????,????????,???????,??????	
#define    p_MODE    PA.2     // output,?????,???300ms?????,??????????
#define    p_RESET   PA.7     // output,????,???

.RAMADR	0
word RamSize;
.RAMADR SYSTEM

word  T16Data;
byte  Timer3s;
byte  Timer400ms;
byte  Timer500ms;
byte  HalfSecond;
byte  Timer20s;
word  KeyDelay;
byte  NetDelay;
byte  TimerComm;
byte  Timer250ms;


bit   b2ms0;
bit   b2ms1;
bit   b2ms2;
bit   bFactorySet;            //????
bit   bNetOk;
bit   bKeyPressed;
bit   bQuick;
bit   bWorking;
bit   bTouch;                 //????
bit   bTopLight;
bit   bCommOK;
bit   bRGB;

byte  LightLevel;
byte  SpeedLevel;
byte  Mode;

byte  CWLight;
byte  CWValue;

byte  Red;
byte  Green;
byte  Blue;
byte  Cold;
byte  Warm;

byte  RedValue;
byte  GreenValue;
byte  BlueValue;
byte  ColdValue;
byte  WarmValue;

word  RedCount;
word  GreenCount;
word  BlueCount;
word  ColdCount;
word  WarmCount;

byte  Count;
word  SpeedBuff;
word  LightFlow;

word  SpeedPtr;
const word SpeedTable1[7]=
{
	4800,2400,1200,600,300,150,75
};
const word SpeedTable2[7]=
{
	12,10,8,6,4,2,1
};



byte  SendFlag;               //????????

//??????----------------------------------------------------------
enum 
{ 
	START, 
	DATA, 
	STOP 
};

#define STCNT  2
#define DTCNT  4

byte rStatus;                 //????
byte TimeCnt;                 //????
byte BitCount;                //?????
byte DataBuff;                //??????
#define MAXLENGTH 23
byte DataRecord[MAXLENGTH];   //?????
word Ptr;                     //??
byte Number;                  //??(??)????
byte DataCount;               //????????
byte ReceiveTimer;            //??????????

bit bReceiveOk;
bit bSendEn;
//--------------------------------------------------------------------

void VM_RS232_RX(void)        // ??232???? 
{ 
   switch(rStatus) 
   { 
      case START: 
        if(p_RXD) 
        { 
        	TimeCnt = 0; 
        } 
        else 
        { 
        	TimeCnt++;
          if(TimeCnt >= STCNT) 
          { 
          	TimeCnt = 0; 
          	BitCount = 0; 
          	rStatus = DATA; 
          } 
        } 
        break; 
      case DATA: 
      	TimeCnt++;
        if(TimeCnt >= DTCNT) 
        { 
          TimeCnt = 0; 
          if(p_RXD) 
          {
          	DataBuff |= 0x80; 
          } 
          else 
          { 
          	DataBuff &= 0x7F; 
          } 
          
          BitCount++;
          if(BitCount < 8) 
          { 
          	DataBuff >>= 1; 
          } 
          else 
          { 
          	BitCount = 0; 
          	rStatus = STOP; 
          } 
        } 
        break; 
      case STOP: 
        if(p_RXD) 
        {
          if (!bReceiveOk)
          {
        		if(Number==0) Ptr=DataRecord;
        		*Ptr=DataBuff;
        		Number++;
        		if(Number<MAXLENGTH)
        		{
        			Ptr++;
        		}
        		ReceiveTimer=6;
          }
          rStatus = START; 
        } 
        break; 
      default: 
        TimeCnt = 0; 
        rStatus = START; 
        break; 
   } 
}

void PortInit(void)
{
	PA  = 0b00001000;           //data
	PAC = 0b10011111;           //input or  output
	PAPH= 0b01001000;           //pull up
	
	PAOD= 0b00000000;           //????
	PADIDR= 0b00000000;
	
	PB  = 0b00000000;           //data
	PBC = 0b11111110;           //input or  output
	PBPH= 0b00000001;           //pull up
	PBDIDR= 0b00000000;
	
	PC  = 0b00000000;           //data
	PCC = 0b11111111;           //input or  output
	PCPH= 0b00000000;           //pull up
	PCDIDR=0xfc;
}

/*  2M ??
void TimerInit(void)
{
  T16M=0b00101111;            // cLk/4
  T16Data=31770;              //32768-1000   2ms
  stt16 T16Data;
  //T16M=0b00001111;          //disable cLk/4
  
  TM2CT = 0;                  
  TM2C=0x00;                //disable 
  //TM2C=0b00010000;            //system clock
#ifdef BUD4800
  TM2S=0b00100001;            //4*(1+1)/2m=4us
  TM2B = 12;                  //?? 4*13=52us
#else
  TM2S=0b00100000;            //4*(1+0)/2m=2us
  TM2B = 12;                  //26us
#endif
}
*/

void TimerInit(void)
{
  T16M=0b00101111;            // cLk/4
  T16Data=30770;              //32768-2000   2ms
  stt16 T16Data;
  //T16M=0b00001111;          //disable cLk/4
  
  TM2CT = 0;                  
  //TM2C=0x00;                //disable 
  TM2C=0b00010000;            //system clock
#ifdef BUD4800
  TM2S=0b00100011;            //4*(3+1)/4m=4us
  TM2B = 12;                  //?? 4*13=52us
#else
  TM2S=0b00100001;            //4*(1+0)/4m=2us
  TM2B = 12;                  //26us
#endif
}

#ifdef CONFIGMODE
void CmdMode(void)
{                             //????
	$ p_RESET	Out, Low;
	$ p_MODE	Out, High;
	p_TXD=1;
	p_MODE=1;
	p_RESET=0;
	Timer400ms=200;
	while(Timer400ms) wdreset;
	//.delay 200000
	p_RESET=1;
	//$ p_RESET	In, High;
	//$ p_MODE	In, Low;
	Timer400ms=200;
	while(Timer400ms) wdreset;
	//.delay 200000
	
}
#endif

void AutoMode(void)
{                             //????
	$ p_RESET	Out, Low;
	$ p_MODE	Out, Low;
	p_TXD=1;                    //???????????
	p_MODE=0;
	p_RESET=0;
	Timer400ms=200;
	while(Timer400ms) wdreset;
	//.delay 200000
	p_RESET=1;
	//$ p_RESET	In, High;
	//$ p_MODE	In, Low;
	Timer400ms=200;
	while(Timer400ms) wdreset;
	//.delay 200000
}

void ExitTransmitMode(void)
{                             //????
	DataRecord[0]='+';
	DataRecord[1]='+';
	DataRecord[2]='+';
	DataCount=3;
	SendFlag=1;
	bSendEn=1;
}

void FactorySet(void)
{                             //??????
	DataRecord[0]='A';
	DataRecord[1]='T';
	DataRecord[2]='+';
	DataRecord[3]='R';
	DataRecord[4]='S';
	DataRecord[5]='T';
	DataRecord[6]='F';
	DataRecord[7]=0x0d;
	DataCount=8;
	bSendEn=1;
}

void ResetModule(void)
{                             //??
	DataRecord[0]='A';
	DataRecord[1]='T';
	DataRecord[2]='+';
	DataRecord[3]='Z';
	DataRecord[4]=0x0d;
	DataCount=5;
	bSendEn=1;
}

void SetNetParamter(void)
{                             //??UDP??,????50000
	DataRecord[0]='A';
	DataRecord[1]='T';
	DataRecord[2]='+';
	DataRecord[3]='A';
	DataRecord[4]='T';
	DataRecord[5]='R';
	DataRecord[6]='M';
	DataRecord[7]='=';
	DataRecord[8]='!';
	DataRecord[9]='1';
	DataRecord[10]=',';
	DataRecord[11]='1';
	DataRecord[12]=',';
	DataRecord[13]='1';
	DataRecord[14]='2';
	DataRecord[15]='0';
	DataRecord[16]=',';
	DataRecord[17]='5';
	DataRecord[18]='0';
	DataRecord[19]='0';
	DataRecord[20]='0';
	DataRecord[21]='0';
	DataRecord[22]=0x0d;
	DataCount=23;
	bSendEn=1;
}

void TimerCtrl(void) 
{                             //3S??
	if(ReceiveTimer) return;    //??????
	if(bSendEn) return;         //???? 
	if(!bFactorySet) return;    //????
	if(Timer3s && !bQuick) return;
	Timer3s=3;
	bQuick=0;
	
	if(!Timer20s) 
	{
		Timer20s=20;
		AutoMode();
		SendFlag=1;
	}
	
	switch(SendFlag)
	{
		case 1:                   //????
			ExitTransmitMode();
			break;
		case 2:                   //??????
			FactorySet();
			break;
		case 3:                   //??????
			SetNetParamter();
			break;
		case 4:                   //??wifi??
			ResetModule();
			break;
		default:
			SendFlag=1;
			break;
	}
}

BYTE	mul_y1, mul_x1;
WORD	mul_t2;
void	Byte_Mul_Byte (void)
{	//	mul_t2[W]	=	mul_x1[B] * mul_y1[B]
	mul_t2$1	=	0;
	BYTE	cnt;
	cnt	=	8;

	do
	{
		mul_x1	>>=	1;
		if (CF)
		{
			mul_t2	+=	(mul_y1 << 8);
		}
		mul_t2	>>>=	1;
	} while (--cnt);
}

WORD	div_src2;
BYTE	div_val;
static	void	Word_Div_Byte (void)
{	//	div_src2[W] / div_val[B]	=	div_src2[W] * div_val[B] + div_res[B]
	BYTE	div_cnt, div_tmp, div_res;
	div_cnt	=	0;
	div_res	=	0;
	goto	div_shf;

	do
	{
		slc		div_res;
		slc		div_tmp;
		div_cnt++;
		div_res	-=	div_val;

		if (!div_tmp.0 && CF)
		{
			div_res	+=	div_val;
			CF	=	0;
		}
		else	CF	=	1;
div_shf:
		div_src2	<<<=	1;
	} while (! div_cnt.4);
}

void ModeCtrl(void)
{
	word Buff;
	
	if(!bWorking) return;
	if(!bRGB) return;           //???
	if(bTouch) return;
	if(!b2ms2) return;
	b2ms2=0;
	switch(Mode)
	{
Step1:
		case 0://red
			Red=255;
			Green=0;
			Blue=0;
			break;
Step2:
		case 1://green
			Red=0;
			Green=255;
			Blue=0;
			break;
Step3:
		case 2://blue
			Red=0;
			Green=0;
			Blue=255;
			break;
Step4:
		case 3://yellow
			Red=255;
			Green=255;
			Blue=0;
			break;
Step5:
		case 4://purple
			Red=255;
			Green=0;
			Blue=255;
			break;
Step6:
		case 5://cyan
			Red=0;
			Green=255;
			Blue=255;
			break;
Step7:
		case 6://white
			Red=255;
			Green=255;
			Blue=255;
			break;
		case 7:
			SpeedBuff++;
			SpeedPtr=SpeedTable1+SpeedLevel;
			Buff=*SpeedPtr$W;
			if(SpeedBuff>=Buff)
			{
				SpeedBuff=0;
				Count++;
				if(Count>=3) Count=0;
			}
			if(Count==0)
			{
				goto Step1;
			}
			else if(Count==1)
			{
				goto Step2;
			}
			else
			{
				goto Step3;
			}
			break;
		case 8:
			SpeedBuff++;
			SpeedPtr=SpeedTable1+SpeedLevel;
			Buff=*SpeedPtr$W;
			if(SpeedBuff>=Buff)
			{
				SpeedBuff=0;
				Count++;
				if(Count>=7) Count=0;
			}
			if(Count==0)
			{
				goto Step1;
			}
			else if(Count==1)
			{
				goto Step2;
			}
			else if(Count==2)
			{
				goto Step3;
			}
			else if(Count==3)
			{
				goto Step4;
			}
			else if(Count==4)
			{
				goto Step5;
			}
			else if(Count==5)
			{
				goto Step6;
			}
			else
			{
				goto Step7;
			}
			break;
		case 9:
			SpeedBuff++;
			SpeedPtr=SpeedTable2+SpeedLevel;
			Buff=*SpeedPtr$W;
			if(SpeedBuff>=Buff)
			{
				SpeedBuff=0;
				if(!bTopLight)
				{
					if(LightFlow<255)  
					{
						LightFlow++;
						if(LightFlow>=255) 
						{
							LightFlow=255;
							bTopLight=1;
						}
					}
				}
				else
				{
					if(LightFlow)  LightFlow--;
					if(!LightFlow) 
					{
						bTopLight=0; 
						Count++;
						if(Count>=3) 
						{
							Count=0;
						}
					}
				}
			}
			if(Count==0)
			{
				Red=LightFlow;
				Green=0;
				Blue=0;
			}
			else if(Count==1)
			{
				Red=0;
				Green=LightFlow;
				Blue=0;
			}
			else 
			{
				Red=0;
				Green=0;
				Blue=LightFlow;
			}			
			break;
		case 10:
			SpeedBuff++;
			SpeedPtr=SpeedTable2+SpeedLevel;
			Buff=*SpeedPtr$W;
			if(SpeedBuff>=Buff)
			{
				SpeedBuff=0;
				if(!bTopLight)
				{
					if(LightFlow<255)  
					{
						LightFlow++;
						if(LightFlow>=255) 
						{
							LightFlow=255;
							bTopLight=1;
						}
					}
				}
				else
				{
					if(LightFlow)  LightFlow--;
					if(!LightFlow) 
					{
						bTopLight=0; 
						Count++;
						if(Count>=7) 
						{
							Count=0;
						}
					}
				}
			}
			if(Count==0)
			{
				Red=LightFlow;
				Green=0;
				Blue=0;
			}
			else if(Count==1)
			{
				Red=0;
				Green=LightFlow;
				Blue=0;
			}
			else if(Count==2)
			{
				Red=0;
				Green=0;
				Blue=LightFlow;
			}	
			else if(Count==3)
			{
				Red=LightFlow;
				Green=LightFlow;
				Blue=0;
			}
			else if(Count==4)
			{
				Red=LightFlow;
				Green=0;
				Blue=LightFlow;
			}	
			else if(Count==5)
			{
				Red=0;
				Green=LightFlow;
				Blue=LightFlow;
			}
			else
			{
				Red=LightFlow;
				Green=LightFlow;
				Blue=LightFlow;
			}	
			break;
		case 11:
			SpeedBuff++;
			SpeedPtr=SpeedTable2+SpeedLevel;
			Buff=*SpeedPtr$W;
			if(SpeedBuff>=Buff)
			{
				SpeedBuff=0;
				if(LightFlow<255) LightFlow++;
				else
				{
					LightFlow=0;
					Count++;
					if(Count>=6) Count=0;
				}
			}
			if(Count==0)
			{
				Red=255;
				Green=LightFlow;
				Blue=0;
			}
			else if(Count==1)
			{
				Red=255-LightFlow;
				Green=255;
				Blue=0;
			}
			else if(Count==2)
			{
				Red=0;
				Green=255;
				Blue=LightFlow;
			}	
			else if(Count==3)
			{
				Red=0;
				Green=255-LightFlow;
				Blue=255;
			}
			else if(Count==4)
			{
				Red=LightFlow;
				Green=0;
				Blue=255;
			}	
			else
			{
				Red=255;
				Green=0;
				Blue=255-LightFlow;
			}
			break;
		default:
			mode=11;
			break;
	}
}

void LightAdjust(void)
{
	if(!bWorking) 
	{
		RedValue=0;
		GreenValue=0;
		BlueValue=0;
		ColdValue=0;
		WarmValue=0;
		Count=0;
		SpeedBuff=0;
		bTopLight=0;
		LightFlow=0;
		return;
	}
	if(!bRGB)
	{
		mul_y1=Cold;
		mul_x1=CWLight+1;
		Byte_Mul_Byte();
		div_src2=mul_t2;
		div_val=10;
		Word_Div_Byte();
		ColdValue=div_src2;
		
		mul_y1=Warm;
		mul_x1=CWLight+1;
		Byte_Mul_Byte();
		div_src2=mul_t2;
		div_val=10;
		Word_Div_Byte();
		WarmValue=div_src2;
		
		RedValue=0;
		GreenValue=0;
		BlueValue=0;
	}
	else
	{
		ColdValue=0;
		WarmValue=0;
		if(!bTouch)
		{
			if((Mode==9) || (Mode==10) || (Mode==11) )
			{
				RedValue=Red;
				GreenValue=Green;
				BlueValue=Blue;
				return;
			}
		}
		mul_y1=Red;
		mul_x1=LightLevel+1;
		Byte_Mul_Byte();
		div_src2=mul_t2;
		div_val=10;
		Word_Div_Byte();
		RedValue=div_src2;
		
		mul_y1=Green;
		mul_x1=LightLevel+1;
		Byte_Mul_Byte();
		div_src2=mul_t2;
		div_val=10;
		Word_Div_Byte();
		GreenValue=div_src2;
		
		mul_y1=Blue;
		mul_x1=LightLevel +1;
		Byte_Mul_Byte();
		div_src2=mul_t2;
		div_val=10;
		Word_Div_Byte();
		BlueValue=div_src2;
	}
}

void GetCWValue(void)
{
	if(CWValue==0)
	{
		Warm=0;
		Cold=255;
	}
	else if(CWValue==9)
	{
		Warm=255;
		Cold=0;
	}
	else
	{
		mul_y1=25;
		mul_x1=CWValue+1;
		Byte_Mul_Byte();
		if(mul_t2>=250) mul_t2=255;
		Warm=mul_t2;
		Cold=255-mul_t2;
	}
}

void ReceiveProcessing(void)
{
	if(!b2ms1) return;
	b2ms1=0;
	if(ReceiveTimer)
	{                           //????
		ReceiveTimer--;
		if(!ReceiveTimer)
		{
			bQuick=1;
			bReceiveOk=1;
			DataCount=Number;
			Number=0;
			switch(SendFlag)
			{
				case 0:               //????(?????????)
					if((DataCount==3) && (DataRecord[2]==0x55))
					{
						bRGB=1;
						bCommOK=1;
						TimerComm=250;
						if(!bWorking)
						{
							if((DataRecord[0]!=34) && (DataRecord[0]!=53)) break;
						}
						switch(DataRecord[0])
						{
							case 53:
								bRGB=0;
							case 34:
								bWorking=1;
								break;
							case 57:
								bRGB=0;
							case 33:
								bWorking=0;
								Count=0;
								SpeedBuff=0;
								bTopLight=0;
								LightFlow=0;
								break;
							case 35:
								LightLevel++;
								if(LightLevel>=9) LightLevel=9;
								break;
							case 36:
								if(LightLevel) LightLevel--;
								break;
							case 37:
								SpeedLevel++;
								if(SpeedLevel>=6) SpeedLevel=6;
								break;
							case 38:
								if(SpeedLevel) SpeedLevel--;
								break;
							case 39:
								bTouch=0;
								if(Mode<11)
								{
									Mode++;
									Count=0;
									SpeedBuff=0;
									bTopLight=0;
									LightFlow=0;
								}
								Red=0;
								Green=0;
								Blue=0;
								break;
							case 40:
								bTouch=0;
								if(Mode) 
								{
									Mode--;
									Count=0;
									SpeedBuff=0;
									bTopLight=0;
									LightFlow=0;
								}
								Red=0;
								Green=0;
								Blue=0;
								break;
								
							case 52:        //??-
								bRGB=0;
								if(CWLight) CWLight--;
								break;
							case 60:        //??+
								bRGB=0;
								CWLight++;
								if(CWLight>=9) CWLight=9;
								break;
							case 62:        //??
								bRGB=0;
								if(CWValue) CWValue--;
								GetCWValue();
								break;
							case 63:        //??
								bRGB=0;
								CWValue++;
								if(CWValue>=9) CWValue=9; 
								GetCWValue();
								break;
							case 56:
								bRGB=0;
								break;
							case 59:
								bRGB=0;
								break;
							case 61:
								bRGB=0;
								break;
							case 51:
								bRGB=0;
								break;
							case 55:
								bRGB=0;
								break;
							case 58:
								bRGB=0;
								break;
							case 50:
								bRGB=0;
								break;
							case 54:
								bRGB=0;
								break;
							case 32:
								bTouch=1;
								if(DataRecord[1]<44)
								{
									Red=0;
									mul_y1=6;
									mul_x1=DataRecord[1];
									Byte_Mul_Byte();
									Green=mul_t2;
									if(Green>=255) Green=255;
									Blue=255;
								}
								else if(DataRecord[1]<87)
								{
									Red=0;
									Green=255;										
									mul_y1=6;
									mul_x1=DataRecord[1]-43;
									Byte_Mul_Byte();
									if(mul_t2>=255) mul_t2=255;
									Blue=255-mul_t2;
								}
								else if(DataRecord[1]<130)
								{								
									mul_y1=6;
									mul_x1=DataRecord[1]-86;
									Byte_Mul_Byte();
									if(mul_t2>=255) mul_t2=255;
									Red=mul_t2;	
									Green=255;
									Blue=0;
								}
								else if(DataRecord[1]<173)
								{			
									Red=255;					
									mul_y1=6;
									mul_x1=DataRecord[1]-129;
									Byte_Mul_Byte();
									if(mul_t2>=255) mul_t2=255;
									Green=255-mul_t2;	
									Blue=0;
								}
								else if(DataRecord[1]<216)
								{			
									Red=255;	
									Green=0;				
									mul_y1=6;
									mul_x1=DataRecord[1]-172;
									Byte_Mul_Byte();
									if(mul_t2>=255) mul_t2=255;
									Blue=mul_t2;	
								}
								else
								{
									mul_y1=6;
									mul_x1=DataRecord[1]-212;
									Byte_Mul_Byte();
									if(mul_t2>=255) mul_t2=255;
									Red=255-mul_t2;	
									Green=0;				
									Blue=255;	
								}
								break;
							default:
								break;
						}
					}
					break;
				case 1:               //????
					if( (DataRecord[0]=='+') && (DataRecord[1]=='O') && (DataRecord[2]=='K'))
					{
						SendFlag=2;
					}
					break;
				case 2:               //??????
					if( (DataRecord[0]=='+') && (DataRecord[1]=='O') && (DataRecord[2]=='K'))
					{
						SendFlag=3;
					}
					break;
				case 3:               //??????
					if( (DataRecord[0]=='+') && (DataRecord[1]=='O') && (DataRecord[2]=='K'))
					{
						SendFlag=4;
					}
					break;
				case 4:               //??
					if( (DataRecord[0]=='+') && (DataRecord[1]=='O') && (DataRecord[2]=='K'))
					{
						bFactorySet=0;
						SendFlag=0;
					}
					break;
				default:
					SendFlag=1;
					break;
			}		

			bReceiveOk=0;
		}
	}
}

void KeyCtrl(void)
{
	if(!b2ms0) return;
	b2ms0=0;
	
	if(bFactorySet)
	{
		if(HalfSecond.0) ONLINE=1;
		else ONLINE=0;
	}
	else
	{
		if(!p_READY)
		{                           //????????
			if(!bNetOk)
			{
				NetDelay++;
				if(NetDelay>=250)
				{
					bNetOk=1;
					ONLINE=1;
				}
			}
			else NetDelay=250;
		}
		else
		{
			if(bNetOk)
			{
				if(NetDelay) NetDelay--;
				if(!NetDelay) 
				{
					bNetOk=0;
					ONLINE=0;
				}
			}
			else
			{
				NetDelay=0;
			}
		}
	}
	
	if(RESETKEY)
	{
		if(bKeyPressed)
		{
			if(KeyDelay) KeyDelay--;
			if(!KeyDelay)  bKeyPressed=0;
		}
		else
		{
			KeyDelay=0;
		}
	}
	else
	{
		if(bKeyPressed) return;
		if(!RESETKEY)
		{
			KeyDelay++;
			if(KeyDelay>=1500)
			{
				KeyDelay=50;
				bKeyPressed=1;
				AutoMode();
				bFactorySet=1;
				ExitTransmitMode();
				Timer20s=20;
			}
		}
		else 
		{
			KeyDelay=0;
		}
	}
}

//===============================================================================
 void	FPPA0 (void)
{
	.ADJUST_OTP_IHRCR	4MIPS		  
	disgint
	
	RamSize=_SYS(RAM_SIZE) - 1;
	A = 0;
	do
	{
		*RamSize = A;
	} while (RamSize--);
	
	PortInit();
	TimerInit();
  
	fppen	=	0x03;   
	
	$ INTEN TM2,T16             // ?????2??
	clkmd.1	=	1;                //	WatchDog Enable
	engint
	
	AutoMode();
	
	SpeedLevel=4;
	LightLevel=5;
	Mode=3;
	CWValue=4;
	CWLight=5;
	GetCWValue();
	
	
	while(1)
	{
		wdreset;
		TimerCtrl();              //3s
		ReceiveProcessing();
		KeyCtrl();
		ModeCtrl();
		LightAdjust();
	}
}

void	FPPA1 (void)
{
	while(1)
	{
		if(bSendEn)
		{
			TM2C=0x00;              //disable tm2
			Ptr=DataRecord;
			while( (Number<MAXLENGTH) && (Number<DataCount))
			{
				Number++;
				DataBuff=*Ptr;
				Ptr++;
				
				BitCount=0;
				p_TXD=0;
				#ifdef BUD4800
				.delay 410            //208us
				#else
				.delay 202            //104us
				#endif				
				while(BitCount<8)
				{
					if(DataBuff.0) 
						p_TXD=1;
					else
						p_TXD=0;
					#ifdef BUD4800
					.delay 407          //208us
					#else	
					.delay 199          //104us
					#endif
					DataBuff >>= 1;
					BitCount++;
				}
				p_TXD=1;
				#ifdef BUD4800
				.delay 415            //208us
				#else
				.delay 207            //104us
				#endif
			}
			Number=0;
			TM2C=0b00010000;        //system clock
			bSendEn=0;
		}
		else if(!bFactorySet)
		{
			if(!bWorking)
			{
				R=0;
				G=0;
				B=0;
				C=0;
				W=0;
				RedCount=0;
				GreenCount=0;
				BlueCount=0;
				ColdCount=0;
				WarmCount=0;
			}
			else
			{
				RedCount++;
				if(RedCount<=RedValue) 
					R=1;
				else if(RedCount<=255)
					R=0;
				else 
					RedCount=0;
					
				GreenCount++;
				if(GreenCount<=GreenValue) 
					G=1;
				else if(GreenCount<=255)
					G=0;
				else 
					GreenCount=0;
					
				BlueCount++;
				if(BlueCount<=BlueValue) 
					B=1;
				else if(BlueCount<=255)
					B=0;
				else 
					BlueCount=0;
					
				ColdCount++;
				if(ColdCount<=ColdValue) 
					C=1;
				else if(ColdCount<=255)
					C=0;
				else 
					ColdCount=0;
				
				WarmCount++;
				if(WarmCount<=WarmValue) 
					W=1;
				else if(WarmCount<=255)
					W=0;
				else 
					WarmCount=0;
			}
		}
		else
		{
			R=0;
			G=0;
			B=0;
			C=0;
			W=0;
		}
	}
}

void	Interrupt (void)
{
	pushaf		                  //	At PDK80CXX, not support the command
	
	if(Intrq.T16)
	{                           //2ms
		Intrq.T16=0;
		stt16 T16Data;
		
		b2ms0=1;
		b2ms1=1;
		b2ms2=1;
		if(Timer400ms) Timer400ms--;
		if(TimerComm) 
		{
			TimerComm--;
			if(!TimerComm)
			{
				bCommOK=0;
				LINK=0;
			}
		}
		Timer250ms++;
		if(Timer250ms>=50)
		{
			Timer250ms=0;
			if(bCommOK) tog LINK;
		}
		Timer500ms++;
		if(Timer500ms>=250)
		{
			Timer500ms=0;
			HalfSecond++;
			if(HalfSecond.0)
			{
				if(Timer3s) Timer3s--;
				if(Timer20s) Timer20s--;
			}
		}
	}
	
	if(Intrq.TM2)
	{                           //52us(4800) or 26us(9600)
		Intrq.TM2=0;
		VM_RS232_RX();
	}
  
	if (Intrq.PA0)
	{	
		Intrq.PA0	=	0;
	}
	if (Intrq.PB0)
	{	
		Intrq.PB0	=	0;
	}

	popaf		                    //	At PDK80CXX, not support the command
}

