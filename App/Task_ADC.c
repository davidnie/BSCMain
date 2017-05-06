/***********************************************************************
***********************************************************************/
#include "main.h"  
#include "task_ADC.h"
#include "task_main.h"
#include "battery_gt.h"
AD_RESULT ad_res;
float f_curr_adc_0;

void Task_ADC(void );
unsigned int i = 0;

//float f_adc_value[ADC_CHANNLE_NUM];
float f_adc_value[16];
// condition: 3.067V Vdd, 10K resistor.   MV Voltage of sensor
uint32_t Temprature_Table[30]={2985,2963,2924,2888,2828,
2774,2688,2611,2495,2395,
2253,2134,1974,1844,1679,
1549,1391,1268,1127,1019,
899,808,709,636,558,
499,438,392,344,182};
uint32_t temp_table_JT103[18]={3987,3905,3777,3590,3323, 
	3009,2636,2243,1857,1505,
	1254,948,745,585,460,
	363,289,231};
int lookUpTable(uint32_t *ptTbl, UINT8 tblLen, int valMin,UINT8 elmtGap,uint32_t value);

int lookUpTable_UP(uint32_t *ptTbl, UINT8 tblLen, int valMin,UINT8 elmtGap,uint32_t value);
extern uint32_t dma_trig_cnt;
extern uint32_t adc_curr;

/***********************************************************************
name : void Task_ADC(void *pdata)
function : calcultate AD result
***********************************************************************/
void Task_ADC(void){
if (ADC_dma_ready_flag==1){
			for(i = 0;i < 9;i ++)
			{
				f_adc_value[i] = ADC_REV_Instant[i] * 3.3 / 4096.0;
				if(dma_trig_cnt>0)		{
					parseADC(i,ADC_REV_SUM[i]/dma_trig_cnt);
				}
				ADC_REV_SUM[i]=0;
			}
			dma_trig_cnt=0;
			DMA_Cmd(DMA2_Stream0, ENABLE); 
			
			ADC_SoftwareStartConv(ADC1);
			ADC_dma_ready_flag=0;
}
}
parseADC(UNS8 ch,UNS16 val){
	float ft;
	ft=(float)val*3300/4096;
	switch(ch){
		case 0:
			//ADC 1  ouput is 2.833V for some reason. should be 3.3V
			ad_res.T_1=lookUpTable((uint32_t *)&temp_table_JT103, 18, -50,10,(UINT16)((UINT32)val*208/195));
			break;
		case 1:
			ad_res.T_2=lookUpTable((uint32_t *)&temp_table_JT103, 18, -50,10,val);
			break;
		case 2:
			ad_res.T_3=lookUpTable((uint32_t *)&temp_table_JT103, 18, -50,10,val);
			break;
		case 3: 	//channel 6, PA6, DC battery voltage
			ft *=21;
			ad_res.V_DC_Batt=(UNS16)ft;
			bpInfo.VBat=ad_res.V_DC_Batt;
			break;
		case 5: 	//channel 9, PB1, DC Busvoltage
			ft *=21;
			ad_res.V_DC_Bus=(UNS16)ft;
			bpInfo.VBus=ad_res.V_DC_Bus;
			break;
		case 4: //channel 8 PB0
			if(gFkStat==FK_INIT){
				f_curr_adc_0=ft;
				ad_res.I_curr=0;
			}else{
				bpInfo.bp_curr=(INT32)((ft-f_curr_adc_0)*gSysCfg.uiCAL_CS_AMP);
								ad_res.I_curr=abso(bpInfo.bp_curr);

			}
			break;
		case 6:
			ft *=21;
			ad_res.V_HV_in=(UNS16)ft;
			break;
		case 7:	//channel 12. 5V output
			ft *=5/3;
			ad_res.V_5VOut=(UNS16)ft;
			break;
		case 8: //channael 13. 12V output
			ft *=4;
			ad_res.V_12VOut=(UNS16)ft;
			break;
	}
}


int lookUpTable(uint32_t *ptTbl, UINT8 tblLen, int valMin,UINT8 elmtGap,uint32_t value){
	 int i,j,k;
	 UINT8 index_table_i,index_table_j,index_table_k;
	 uint32_t count=0;
	 float tem;
	 int intTemp;
	  i=valMin;
	  j=valMin+ (int)(tblLen-1)*elmtGap;
  	tem=*ptTbl;
	count=*ptTbl;
	count=ptTbl[0];
	tem=*(ptTbl+tblLen-1);
	tem=*(ptTbl+1);
	if(value> *ptTbl){
	      intTemp= valMin-5;
      }else  if (value<*(ptTbl+tblLen-1)){
	      intTemp= valMin+ (int)tblLen*elmtGap +5;
      }else{
	  for (;;){
		    count++;
	 	    index_table_i=(i-valMin)/elmtGap;
	 	    index_table_j=(j-valMin)/elmtGap;
		    index_table_k=(index_table_i+index_table_j)/2;
		    k=index_table_k*elmtGap+valMin;
		    if (elmtGap>=j-i){
		   		//tem=i+(float)(Temprature_Table[index_table_i]-value)/(float)(Temprature_Table[index_table_i]-Temprature_Table[index_table_j])*elmtGap;
				tem=i+(float)(*(ptTbl+index_table_i)-value)/(float)(*(ptTbl+index_table_i)-*(ptTbl+index_table_j))*elmtGap;
				intTemp=tem;
				break;
		    }else if  (value>*(ptTbl+index_table_k)) {
		   	   j=k;
		    } else if (value<*(ptTbl+index_table_k)){
			     i=k;
		    }else {    // it's equal to the mid-value
		      tem=k;
	     	 intTemp=tem;
			 break;
 		    }  
    	}
   }
   return intTemp;
}

/* value in talbe is go up */
int lookUpTable_UP(uint32_t *ptTbl, UINT8 tblLen, int valMin,UINT8 elmtGap,uint32_t value){
	 int i,j,k;
long tbl[11];
	 UINT8 index_table_i,index_table_j,index_table_k;
	 uint32_t count=0;
	 float tem;
	 int intTemp;
	 for(i=0;i<11;i++){
		tbl[i]=*(ptTbl+i);
	}
	  i=valMin;
	  j=valMin+ (int)(tblLen-1)*elmtGap;

count=*ptTbl;
count=ptTbl[0];
tem=*(ptTbl+tblLen-1);
tem=*(ptTbl+1);
	if(value< *ptTbl){
	      intTemp= valMin-5;
      }else  if (value>*(ptTbl+tblLen-1)){
	      intTemp= valMin+ (int)tblLen*elmtGap +5;
      }else{
	  for (;;){
		    count++;
	 	    index_table_i=(i-valMin)/elmtGap;
	 	    index_table_j=(j-valMin)/elmtGap;
		    index_table_k=(index_table_i+index_table_j)/2;
		    k=index_table_k*elmtGap+valMin;
		    if (elmtGap>=j-i){
		   		tem=i+(float)(value - tbl[index_table_i])/(float)(tbl[index_table_j]-tbl[index_table_i])*elmtGap;
				intTemp=tem;
				break;
		    }else if  (value<tbl[index_table_k]) {
		   	   j=k;
		    } else if (value>tbl[index_table_k]){
			     i=k;
		    }else {    // it's equal to the mid-value
		      tem=k;
	     	 intTemp=tem;
			 break;
 		    }  
    	}
   }
   return intTemp;
}

