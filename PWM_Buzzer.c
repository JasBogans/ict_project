/**********************************************************************************************************************
 * \file PWM_Buzzer.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are solely in the form of
 * machine-executable object code generated by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "PWM_Buzzer.h"
#include "Bsp.h"
#include "IfxGtm_Tom_Pwm.h"
#include "IfxGtm_Tom_Timer.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define PWM_BUZZER_PRIO     0                                   /* Interrupt priority number                        */
#define VOLUME_LEVEL        2                                   /* Volume level in percentage                       */
#define MIN_RESOLUTION      0                                   /* Ignore minimum resolution parameter              */
#define TRIGGER_POINT       0xffff                              /* Trigger point for GTM timer                      */
#define FREQ_NULL           0                                   /* Initial frequency for GTM timer                  */
#define WAIT_TIME           1                                   /* Number of milliseconds to wait during play time  */

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
Ifx_TickTime g_ticksFor1ms;     /* Variable to store the number of ticks to wait for 1 second delay                 */
IfxGtm_Tom_Timer g_buzzer;

/*********************************************************************************************************************/
/*-----------------------------------------------Function Prototypes-------------------------------------------------*/
/*********************************************************************************************************************/
/* Set the volume of the Buzzer */
void setVolume(uint8 volumeLevel);

/* Start the PWM and hence the buzzer */
//void playTone(float32 frequency, sint64 length_ms);

/*********************************************************************************************************************/
/*--------------------------------------------Function Implementations-----------------------------------------------*/
/*********************************************************************************************************************/
/* This function initializes the PWM for the buzzer */
void initPWMBuzzer()
{
    /* Initialize the time variable */
    g_ticksFor1ms = IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, WAIT_TIME);

    IfxGtm_enable(&MODULE_GTM); /* Enable GTM */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK); /* Enable FX clock */

    IfxGtm_Tom_Timer_Config timerCfg; /* Create an instance of configuration structure */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM); /* Initialize the timer with default values */

    timerCfg.tom = IfxGtm_Tom_1;                                            /* Set TOM to be used                   */
    timerCfg.timerChannel = IfxGtm_Tom_Ch_4;                                /* Set the output channel               */
    timerCfg.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk2;                        /* Set the CMUFX clock                  */
    timerCfg.triggerOut = &IfxGtm_TOM1_4_TOUT22_P33_0_OUT;                  /* Set port and pin for PWM output      */
    timerCfg.base.trigger.enabled = TRUE;                                   /* Enable TOM channel                   */
    timerCfg.base.trigger.outputEnabled = TRUE;                             /* Enable TOM channel output            */
    timerCfg.base.trigger.risingEdgeAtPeriod = TRUE;                        /* Starting PWM with rising edge        */
    IfxGtm_Tom_Timer_init(&g_buzzer, &timerCfg);                            /* Initialize the timer                 */

    /* Enable channel update */
    IfxGtm_Tom_Tgc_enableChannelsUpdate((Ifx_GTM_TOM_TGC *)&g_buzzer.tom->TGC0_GLB_CTRL, 1 << IfxGtm_TOM1_4_TOUT22_P33_0_OUT.channel, 0);
}

/* This function set the volume of the buzzer */
void setVolume(uint8 volumeLevel)
{
    /* This function allows you to trigger the PWM and adjust the duty cycle to control the volume of the buzzer */
    IfxGtm_Tom_Timer_setTrigger(&g_buzzer, (volumeLevel * (g_buzzer.base.period)) / 100); /* volumeLevel is in percentage */
}

 /* This function plays a tone */
void playTone(float32 frequency, sint64 duration_ms)
{
    if(frequency == 0) /* If frequency is zero then the PWM will not start */
    {
        waitTime(duration_ms * g_ticksFor1ms);                              /* Wait during play time                */
    }
    else
    {
        IfxGtm_Tom_Timer_run(&g_buzzer);                                    /* Start the timer                      */
        IfxGtm_Tom_Timer_setFrequency(&g_buzzer,frequency);                 /* Set frequency to the desired value   */
        setVolume(VOLUME_LEVEL);                                            /* Set volume to VOLUME_LEVEL           */
        waitTime(duration_ms * g_ticksFor1ms);                              /* Wait during play time                */
        setVolume(0);                                                       /* Set volume to 0                      */
        IfxGtm_Tom_Timer_stop(&g_buzzer);                                   /* Stop the timer                       */
    }
}

void startBuzzerSong(void)
{

    /* Frequency table in Hertz for sound */
    float32 frequencies[] = {
        659, 659, 0.0, 659, 0.0, 523, 659, 784, 0.0, 392, 0.0, 523, 392,  0.0, 330,  330,  440, 494, 494, 466, 440,
        392, 659, 784, 880, 698, 784, 0.0, 659, 523, 587, 494, 0.0, 523,  392, 0.0,  330,  330, 440, 494, 466, 440,
        392, 659, 784, 880, 698, 784, 0.0, 659, 523, 587, 494, 0.0, 0.0,  784, 740,  698,  622, 659, 0.0, 415, 440,
        523, 0.0, 440, 523, 587, 0.0, 784, 740, 698, 622, 659, 0.0, 1047, 0.0, 1047, 1047, 0.0, 784, 740, 698, 622,
        659, 0.0, 415, 440, 523, 0.0, 440, 523, 587, 0.0, 622, 0.0, 587,  0.0, 523,  0.0,  523, 523, 523, 0.0, 523,
        587, 659, 523, 440, 392, 523, 523, 523, 0.0, 523, 587, 659, 0.0,  523, 523,  523,  0.0, 523, 587, 659, 523,
        440, 392, 659, 659, 0.0, 659, 0.0, 523, 659, 784, 0.0, 392, 0.0,  523, 392,  0.0,  330, 330, 440, 494, 494,
        466, 440, 392, 659, 784, 880, 698, 784, 0.0, 659, 523, 587, 494,  0.0, 523,  392,  0.0, 330, 330, 440, 494,
        494, 466, 440, 392, 659, 784, 880, 698, 784, 0.0, 659, 523, 587,  494, 0.0,  659,  523, 392, 0.0, 415, 440,
        698, 698, 440, 494, 880, 880, 880, 784, 698, 659, 523, 440, 392,  659, 523,  392,  0.0, 415, 440, 698, 698,
        440, 494, 698, 698, 698, 659, 587, 523, 262, 523, 523, 523, 0.0,  523, 587,  659,  523, 440, 392, 523, 523,
        523, 0.0, 523, 587, 659, 0.0, 523, 523, 523, 0.0, 523, 587, 659,  523, 440,  392,  659, 659, 0.0, 659, 0.0,
        523, 659, 784, 0.0, 392, 0.0, 659, 523, 392, 0.0, 415, 440, 698,  698, 440,  494,  880, 880, 880, 784, 698,
        659, 523, 440, 392, 659, 523, 392, 0.0, 415, 440, 698, 698, 440,  494, 698,  698,  698, 659, 587, 523, 262,
        523, 392, 392, 330, 440, 494, 440, 415, 466, 415, 392, 0.0
    };

    /* Duration table for as milliseconds sound */
    float32 noteDurations[] = {
        150, 150, 150, 150, 150, 150, 300, 300,  300, 300, 300, 450, 150, 300, 300, 150, 300,  150, 150, 150, 300, 200,
        200, 200, 300, 150, 150, 150, 300, 150,  150, 300, 150, 450, 150, 300, 300, 150, 300,  300, 150, 300, 200, 200,
        200, 300, 150, 150, 150, 300, 150, 150,  300, 150, 300, 150, 150, 150, 300, 150, 150,  150, 150, 150, 150, 150,
        150, 150, 300, 150, 150, 150, 300, 150,  150, 150, 150, 150, 600, 300, 150, 150, 150,  300, 150, 150, 150, 150,
        150, 150, 150, 150, 150, 300, 300, 150,  300, 150, 600, 600, 150, 300, 150, 150, 150,  300, 150, 300, 150, 600,
        150, 300, 150, 150, 150, 150, 150, 1200, 150, 300, 150, 150, 150, 300, 150, 300, 150,  600, 150, 150, 150, 150,
        150, 150, 300, 300, 300, 300, 300, 450,  150, 300, 300, 150, 300, 150, 150, 150, 300,  200, 200, 200, 300, 150,
        150, 150, 300, 150, 150, 300, 150, 450,  150, 300, 300, 150, 300, 150, 150, 150, 300,  200, 200, 200, 300, 150,
        150, 150, 300, 150, 150, 300, 150, 150,  300, 150, 300, 300, 150, 300, 150, 600, 200,  200, 200, 200, 200, 200,
        150, 300, 150, 600, 150, 300, 150, 300,  300, 150, 300, 150, 600, 200, 200, 200, 200,  200, 200, 600, 600, 150,
        300, 150, 150, 150, 300, 150, 300, 150,  600, 150, 300, 150, 150, 150, 150, 150, 1200, 150, 300, 150, 150, 150,
        300, 150, 300, 150, 600, 150, 150, 150,  150, 150, 150, 300, 300, 300, 300, 300, 150,  300, 150, 300, 300, 150,
        300, 150, 600, 200, 200, 200, 200, 200,  200, 150, 300, 150, 600, 150, 300, 150, 300,  300, 150, 300, 150, 600,
        200, 200, 200, 200, 200, 200, 600, 600,  450, 150, 300, 300, 200, 200, 200, 200, 200,  200, 1200, 0
    };

    for(uint16 i = 0; i < sizeof(frequencies) / sizeof(float32); i++)
    {
        playTone(frequencies[i], noteDurations[i]); /* Launch the GTM PWM timer to the frequency the desired value */
    }
}


