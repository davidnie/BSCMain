/*
 * 文件路径：
 *      UCOSII/Ports/os_cpu.h
 * 修改时间：
 *      2013-6-20
 * 版本：
 *      V 1.0
 * 作者：
 *      Eamonn (c) www.eamonning.com
 * 联系：
 *      i@eamonning.com
 */

#ifndef  _UCOSII_PORTS_OS_CPU_H_
#define  _UCOSII_PORTS_OS_CPU_H_


 
#include "M_Global.h"
/*
//这里我在M_Global.h里面定义了这些类型，所以这里就不要定义了
typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;
typedef signed   char  INT8S;
typedef unsigned short INT16U;
typedef signed   short INT16S;
typedef unsigned int   INT32U;
typedef signed   int   INT32S;
typedef float          FP32;
typedef double         FP64;
*/

//堆栈类型，堆栈存取都是32位宽度
typedef unsigned int   OS_STK;

//用于保存CPU状态寄存器类型
typedef unsigned int   OS_CPU_SR;

//临界区采用第三种方式
//实际上，第一二种方法都有问题，参照 http://www.eamonning.com/blog/view/407
#define  OS_CRITICAL_METHOD   3u

//定义保存状态寄存器宏
#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}

//栈增长方向
#define  OS_STK_GROWTH        1u                  /* Stack grows from HIGH to LOW memory on ARM        */

//切换任务函数
#define  OS_TASK_SW()         OSCtxSw()

//保存和恢复状态寄存器的函数
OS_CPU_SR OS_CPU_SR_Save(void);
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);

//任务切换函数
void OSCtxSw(void);

void OSIntCtxSw(void);
void OSStartHighRdy(void);

void OS_CPU_PendSVHandler(void);
void OS_CPU_SysTickHandler(void);
void OS_CPU_SysTickInit (INT32U  cnts);

void CPU_IntDisable(void);
void CPU_IntEnable(void);

#endif
