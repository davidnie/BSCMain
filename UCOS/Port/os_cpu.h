/*
 * �ļ�·����
 *      UCOSII/Ports/os_cpu.h
 * �޸�ʱ�䣺
 *      2013-6-20
 * �汾��
 *      V 1.0
 * ���ߣ�
 *      Eamonn (c) www.eamonning.com
 * ��ϵ��
 *      i@eamonning.com
 */

#ifndef  _UCOSII_PORTS_OS_CPU_H_
#define  _UCOSII_PORTS_OS_CPU_H_


 
#include "M_Global.h"
/*
//��������M_Global.h���涨������Щ���ͣ���������Ͳ�Ҫ������
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

//��ջ���ͣ���ջ��ȡ����32λ���
typedef unsigned int   OS_STK;

//���ڱ���CPU״̬�Ĵ�������
typedef unsigned int   OS_CPU_SR;

//�ٽ������õ����ַ�ʽ
//ʵ���ϣ���һ���ַ����������⣬���� http://www.eamonning.com/blog/view/407
#define  OS_CRITICAL_METHOD   3u

//���屣��״̬�Ĵ�����
#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}

//ջ��������
#define  OS_STK_GROWTH        1u                  /* Stack grows from HIGH to LOW memory on ARM        */

//�л�������
#define  OS_TASK_SW()         OSCtxSw()

//����ͻָ�״̬�Ĵ����ĺ���
OS_CPU_SR OS_CPU_SR_Save(void);
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);

//�����л�����
void OSCtxSw(void);

void OSIntCtxSw(void);
void OSStartHighRdy(void);

void OS_CPU_PendSVHandler(void);
void OS_CPU_SysTickHandler(void);
void OS_CPU_SysTickInit (INT32U  cnts);

void CPU_IntDisable(void);
void CPU_IntEnable(void);

#endif
