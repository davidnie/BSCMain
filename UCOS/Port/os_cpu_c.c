/*
 * 文件路径：
 *      UCOSII/Ports/os_cpu_c.c
 * 修改时间：
 *      2013-6-20
 * 版本：
 *      V 1.0
 * 作者：
 *      Eamonn (c) www.eamonning.com
 * 联系：
 *      i@eamonning.com
 */

#include "ucosii_inc.h"

//用于定时器的辅助变量
#if OS_TMR_EN > 0u
static  INT16U  OSTmrCtr;
#endif

//初始化开始，该函数在OSInit中调用，调用期间应该禁止中断
void  OSInitHookBegin (void)
{
	//禁止中断
	CPU_IntDisable();

	//定时器需要用到的时间控制变量
	#if OS_TMR_EN > 0u
	    OSTmrCtr = 0u;
	#endif 
}

//初始化结束，该函数在OSInit中调用
void  OSInitHookEnd (void)
{  
	//开启中断
	CPU_IntEnable();
}

//任务创建Hook
void  OSTaskCreateHook (OS_TCB *ptcb)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TaskCreateHook(ptcb);
	#else
	    (void)ptcb;
	#endif
}

//任务删除Hook
void  OSTaskDelHook (OS_TCB *ptcb)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TaskDelHook(ptcb);
	#else
	    (void)ptcb;
	#endif
}

//空闲任务Hook，可以再如果该任务被调用，说明没有别的任务在运行
//通常用于，可以暂停CPU工作，以达到节能目的等等
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskIdleHook();
#endif
}

//当一个任务返回时调用
//一个正常的任务，应该是死循环，或者结束时删除自己，返回的话就会调用这个函数
void  OSTaskReturnHook (OS_TCB  *ptcb)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TaskReturnHook(ptcb);
	#else
	    (void)ptcb;
	#endif
}

//当开启了任务统计时，每秒该函数被调用
void  OSTaskStatHook (void)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TaskStatHook();
	#endif
}

/*
*********************************************************************************************************
*                                        初始化任务栈
*
* 说明       : This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* 参数       : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*              2) All tasks run in Thread mode, using process stack.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK *stk;
	INT32U i;


    (void)opt;                                   /* 'opt' is not used, prevent warning                 */
    stk       = ptos;                            /* Load stack pointer                                 */

	#if(__FPU_PRESENT == 1)&&(__FPU_USED == 1)
		*(stk) = (INT32U)0xCCUL;	//unknow register
	    *(--stk) = (INT32U)0xDDUL;  //FPSCR	
	    for(i=0;i<16;i++){
	      *(--stk) = (INT32U)0xAAAAAAAAUL; //S15-S0
	    }
	#else
		stk++;
	#endif
                                                 /* Registers stacked as if auto-saved on exception    */
    *(--stk)    = (INT32U)0x01000000uL;            /* xPSR                                               */
    *(--stk)  = (INT32U)task;                    /* Entry Point                                        */
    *(--stk)  = (INT32U)OS_TaskReturn;           /* R14 (LR)                                           */
    *(--stk)  = (INT32U)0x12121212uL;            /* R12                                                */
    *(--stk)  = (INT32U)0x03030303uL;            /* R3                                                 */
    *(--stk)  = (INT32U)0x02020202uL;            /* R2                                                 */
    *(--stk)  = (INT32U)0x01010101uL;            /* R1                                                 */
    *(--stk)  = (INT32U)p_arg;                   /* R0 : argument                                      */

                                                 /* Remaining registers saved on process stack         */
    *(--stk)  = (INT32U)0x11111111uL;            /* R11                                                */
    *(--stk)  = (INT32U)0x10101010uL;            /* R10                                                */
    *(--stk)  = (INT32U)0x09090909uL;            /* R9                                                 */
    *(--stk)  = (INT32U)0x08080808uL;            /* R8                                                 */
    *(--stk)  = (INT32U)0x07070707uL;            /* R7                                                 */
    *(--stk)  = (INT32U)0x06060606uL;            /* R6                                                 */
    *(--stk)  = (INT32U)0x05050505uL;            /* R5                                                 */
    *(--stk)  = (INT32U)0x04040404uL;            /* R4                                                 */

	#if(__FPU_PRESENT == 1)&&(__FPU_USED == 1)
		for (i = 0u; i < 16; i++){    
	      	*(--stk) = (INT32U)0xBBBBBBBBuL;      //S16-S31
	    }
	#endif

    return (stk);
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void  OSTaskSwHook (void)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TaskSwHook();
	#endif
}

/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
void  OSTCBInitHook (OS_TCB *ptcb)
{
	#if OS_APP_HOOKS_EN > 0u
	    App_TCBInitHook(ptcb);
	#else
	    (void)ptcb;                                  /* Prevent compiler warning                           */
	#endif
}

/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
	#if OS_APP_HOOKS_EN > 0u
		App_TimeTickHook();	
	#endif
	
	#if OS_TMR_EN > 0u
	    OSTmrCtr++;
	    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
	        OSTmrCtr = 0;
	        OSTmrSignal();
	    }
	#endif
}
#endif

/*
*********************************************************************************************************
*                                          SYS TICK HANDLER
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-II tick
*              interrupt.
*
* Arguments  : none.
*
* Note(s)    : 1) This function MUST be placed on entry 15 of the Cortex-M3 vector table.
*********************************************************************************************************
*/
	   
void OS_CPU_SysTickHandler (void)
{
	
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    OSTimeTick();                                /* Call uC/OS-II's OSTimeTick()                       */

    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
}


/*
*********************************************************************************************************
*                                          SYS TICK DEFINES
*********************************************************************************************************
*/

#define  OS_CPU_CM3_NVIC_ST_CTRL    (*((volatile INT32U *)0xE000E010uL)) /* SysTick Ctrl & Status Reg. */
#define  OS_CPU_CM3_NVIC_ST_RELOAD  (*((volatile INT32U *)0xE000E014uL)) /* SysTick Reload  Value Reg. */
#define  OS_CPU_CM3_NVIC_ST_CURRENT (*((volatile INT32U *)0xE000E018uL)) /* SysTick Current Value Reg. */
#define  OS_CPU_CM3_NVIC_ST_CAL     (*((volatile INT32U *)0xE000E01CuL)) /* SysTick Cal     Value Reg. */
#define  OS_CPU_CM3_NVIC_PRIO_ST    (*((volatile INT8U  *)0xE000ED23uL)) /* SysTick Handler Prio  Reg. */

#define  OS_CPU_CM3_NVIC_ST_CTRL_COUNT                    0x00010000uL   /* Count flag.                */
#define  OS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC                  0x00000004uL   /* Clock Source.              */
#define  OS_CPU_CM3_NVIC_ST_CTRL_INTEN                    0x00000002uL   /* Interrupt enable.          */
#define  OS_CPU_CM3_NVIC_ST_CTRL_ENABLE                   0x00000001uL   /* Counter mode.              */
#define  OS_CPU_CM3_NVIC_PRIO_MIN                               0xFFu    /* Min handler prio.          */

/*
*********************************************************************************************************
*                                          INITIALIZE SYS TICK
*
* Description: Initialize the SysTick.
*
* Arguments  : cnts          is the number of SysTick counts between two OS tick interrupts.
*
* Note(s)    : 1) This function MUST be called after OSStart() & after processor initialization.
*********************************************************************************************************
*/
void  OS_CPU_SysTickInit (INT32U  cnts)
{
    OS_CPU_CM3_NVIC_ST_RELOAD = cnts - 1u;
                                                 /* Set prio of SysTick handler to min prio.           */
    OS_CPU_CM3_NVIC_PRIO_ST   = OS_CPU_CM3_NVIC_PRIO_MIN;
                                                 /* Enable timer.                                      */
    OS_CPU_CM3_NVIC_ST_CTRL  |= OS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC | OS_CPU_CM3_NVIC_ST_CTRL_ENABLE;
                                                 /* Enable timer interrupt.                            */
    OS_CPU_CM3_NVIC_ST_CTRL  |= OS_CPU_CM3_NVIC_ST_CTRL_INTEN;
}


