#include "ARMCM4.h" 
void printErrorMsg(const char * errMsg)
{
   while(*errMsg != 'a'){
      ITM_SendChar(*errMsg);
      ++errMsg;
   }
}