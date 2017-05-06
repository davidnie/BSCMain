/*
 * modu.c
 *
 * Code generation for model "modu".
 *
 * Model version              : 1.4
 * Simulink Coder version : 8.6 (R2014a) 27-Dec-2013
 * C source code generated on : Mon Sep 05 01:18:20 2016
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: 32-bit Generic
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */
#include "modu.h"
#include "modu_private.h"

/* Named constants for Chart: '<Root>/Chart1' */
#define modu_IN_NO_ACTIVE_CHILD        ((uint8_T)0U)
#define modu_IN_green                  ((uint8_T)1U)
#define modu_IN_red                    ((uint8_T)2U)
#define modu_IN_yellow                 ((uint8_T)3U)

/* Block signals (auto storage) */
B_modu_T modu_B;

/* Block states (auto storage) */
DW_modu_T modu_DW;

/* Previous zero-crossings (trigger) states */
PrevZCX_modu_T modu_PrevZCX;

/* Real-time model */
RT_MODEL_modu_T modu_M_;
RT_MODEL_modu_T *const modu_M = &modu_M_;

/* Model step function */
void modu_step(void)
{
  real_T rtb_PulseGenerator;
  ZCEventType zcEvent;

  /* DiscretePulseGenerator: '<Root>/Pulse Generator' */
  rtb_PulseGenerator = (modu_DW.clockTickCounter < modu_P.PulseGenerator_Duty) &&
    (modu_DW.clockTickCounter >= 0) ? modu_P.PulseGenerator_Amp : 0.0;
  if (modu_DW.clockTickCounter >= modu_P.PulseGenerator_Period - 1.0) {
    modu_DW.clockTickCounter = 0;
  } else {
    modu_DW.clockTickCounter++;
  }

  /* End of DiscretePulseGenerator: '<Root>/Pulse Generator' */

  /* Chart: '<Root>/Chart1' incorporates:
   *  TriggerPort: '<S1>/tick'
   */
  zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING,&modu_PrevZCX.Chart1_Trig_ZCE,
                     (rtb_PulseGenerator));
  if (zcEvent != NO_ZCEVENT) {
    modu_DW.presentTicks = modu_M->Timing.clockTick0;
    modu_DW.elapsedTicks = modu_DW.presentTicks - modu_DW.previousTicks;
    modu_DW.previousTicks = modu_DW.presentTicks;
    if (modu_DW.temporalCounter_i1 + modu_DW.elapsedTicks <= 255U) {
      modu_DW.temporalCounter_i1 = (uint8_T)(modu_DW.temporalCounter_i1 +
        modu_DW.elapsedTicks);
    } else {
      modu_DW.temporalCounter_i1 = MAX_uint8_T;
    }

    /* Gateway: Chart1 */
    /* Event: '<S1>:12' */
    /* During: Chart1 */
    if (modu_DW.is_active_c1_modu == 0U) {
      /* Entry: Chart1 */
      modu_DW.is_active_c1_modu = 1U;

      /* Entry Internal: Chart1 */
      /* Transition: '<S1>:8' */
      modu_DW.is_c1_modu = modu_IN_red;
      modu_DW.temporalCounter_i1 = 0U;

      /* Entry 'red': '<S1>:1' */
      modu_B.red_led = 1.0;
    } else {
      switch (modu_DW.is_c1_modu) {
       case modu_IN_green:
        /* During 'green': '<S1>:4' */
        if (modu_DW.temporalCounter_i1 >= 60U) {
          /* Transition: '<S1>:7' */
          /* Exit 'green': '<S1>:4' */
          modu_B.green_led = 0.0;
          modu_DW.is_c1_modu = modu_IN_yellow;
          modu_DW.temporalCounter_i1 = 0U;

          /* Entry 'yellow': '<S1>:3' */
          modu_B.yellow_led = 1.0;
        }
        break;

       case modu_IN_red:
        /* During 'red': '<S1>:1' */
        if (modu_DW.temporalCounter_i1 >= 200U) {
          /* Transition: '<S1>:5' */
          /* Exit 'red': '<S1>:1' */
          modu_B.red_led = 0.0;
          modu_DW.is_c1_modu = modu_IN_green;
          modu_DW.temporalCounter_i1 = 0U;

          /* Entry 'green': '<S1>:4' */
          modu_B.green_led = 1.0;
        }
        break;

       default:
        /* During 'yellow': '<S1>:3' */
        if (modu_DW.temporalCounter_i1 >= 60U) {
          /* Transition: '<S1>:6' */
          /* Exit 'yellow': '<S1>:3' */
          modu_B.yellow_led = 0.0;
          modu_DW.is_c1_modu = modu_IN_red;
          modu_DW.temporalCounter_i1 = 0U;

          /* Entry 'red': '<S1>:1' */
          modu_B.red_led = 1.0;
        }
        break;
      }
    }
  }

  /* Matfile logging */
  rt_UpdateTXYLogVars(modu_M->rtwLogInfo, (&modu_M->Timing.taskTime0));

  /* signal main to stop simulation */
  {                                    /* Sample time: [0.05s, 0.0s] */
    if ((rtmGetTFinal(modu_M)!=-1) &&
        !((rtmGetTFinal(modu_M)-modu_M->Timing.taskTime0) >
          modu_M->Timing.taskTime0 * (DBL_EPSILON))) {
      rtmSetErrorStatus(modu_M, "Simulation finished");
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++modu_M->Timing.clockTick0)) {
    ++modu_M->Timing.clockTickH0;
  }

  modu_M->Timing.taskTime0 = modu_M->Timing.clockTick0 *
    modu_M->Timing.stepSize0 + modu_M->Timing.clockTickH0 *
    modu_M->Timing.stepSize0 * 4294967296.0;
}

/* Model initialize function */
void modu_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize real-time model */
  (void) memset((void *)modu_M, 0,
                sizeof(RT_MODEL_modu_T));
  rtmSetTFinal(modu_M, 110.0);
  modu_M->Timing.stepSize0 = 0.05;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    modu_M->rtwLogInfo = &rt_DataLoggingInfo;
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(modu_M->rtwLogInfo, (NULL));
    rtliSetLogXSignalPtrs(modu_M->rtwLogInfo, (NULL));
    rtliSetLogT(modu_M->rtwLogInfo, "tout");
    rtliSetLogX(modu_M->rtwLogInfo, "");
    rtliSetLogXFinal(modu_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(modu_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(modu_M->rtwLogInfo, 0);
    rtliSetLogMaxRows(modu_M->rtwLogInfo, 1000);
    rtliSetLogDecimation(modu_M->rtwLogInfo, 1);
    rtliSetLogY(modu_M->rtwLogInfo, "");
    rtliSetLogYSignalInfo(modu_M->rtwLogInfo, (NULL));
    rtliSetLogYSignalPtrs(modu_M->rtwLogInfo, (NULL));
  }

  /* block I/O */
  (void) memset(((void *) &modu_B), 0,
                sizeof(B_modu_T));

  /* states (dwork) */
  (void) memset((void *)&modu_DW, 0,
                sizeof(DW_modu_T));

  /* Matfile logging */
  rt_StartDataLoggingWithStartTime(modu_M->rtwLogInfo, 0.0, rtmGetTFinal(modu_M),
    modu_M->Timing.stepSize0, (&rtmGetErrorStatus(modu_M)));

  /* Start for DiscretePulseGenerator: '<Root>/Pulse Generator' */
  modu_DW.clockTickCounter = 0;
  modu_PrevZCX.Chart1_Trig_ZCE = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for Chart: '<Root>/Chart1' */
  modu_DW.temporalCounter_i1 = 0U;
  modu_DW.is_active_c1_modu = 0U;
  modu_DW.is_c1_modu = modu_IN_NO_ACTIVE_CHILD;
  modu_DW.presentTicks = 0U;
  modu_DW.elapsedTicks = 0U;
  modu_DW.previousTicks = 0U;

  /* Enable for Chart: '<Root>/Chart1' */
  modu_DW.presentTicks = modu_M->Timing.clockTick0;
  modu_DW.previousTicks = modu_DW.presentTicks;
}

/* Model terminate function */
void modu_terminate(void)
{
  /* (no terminate code required) */
}
