/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/

#include "IfxPort.h"
#include "Bsp.h"
#include "Ifx_Types.h"
#include "IfxGtm_Tim_In.h"
#include "IfxGtm_Tom_Pwm.h"
#include "SPI_CPU.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define LED_D107    &MODULE_P13,0                                           /* LED D107: Port, Pin definition       */
#define TRIGGER     &MODULE_P21,2
//#define ECHO        &MODULE_P21,3
#define ECHO IfxGtm_TIM0_1_P21_3_IN
/* LED port pin */
#define LED_D110                    &MODULE_P13,3   /* LED D110 Port, Pin definition

#define PWM_OUT         IfxGtm_TOM0_11_TOUT3_P02_3_OUT   Port pin which is driven by the PWM                      */
//#define PWM_IN          IfxGtm_TIM0_0_P02_0_IN          /* Input port pin for the PWM signal                        */
#define TICK2PERIOD     3.33e-9
#define SOUNDSPEED      343.1/2.0
/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

float32 measuredTicks = 0.0;                      /* Global variable for ticks counting     */
IfxGtm_Tim_In g_driverTIM;                              /* TIM driver structure                                     */
float distance = 0.0;


/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* This function initializes the port pin which drives the LED */
void initLED(void)
{

    /* Initialization of the LED used in this example */
    IfxPort_setPinModeOutput(LED_D107, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);


    /* Initialization of TRIGGER and ECHO for HCSR04*/
    IfxPort_setPinMode(TRIGGER, IfxPort_Mode_outputPushPullGeneral);

    //IfxPort_setPinMode(ECHO, IfxPort_Mode_inputPullUp);

    /* Switch OFF the LED (low-level active) */
    IfxPort_setPinLow(LED_D107);

    /* Set the port pin 13.3 (to which the LED D110 is connected) to output push-pull mode */
    IfxPort_setPinModeOutput(LED_D110, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Turn off LED (LED is low-level active) */
    IfxPort_setPinHigh(LED_D110);
}



/* This function toggles the port pin and wait 500 milliseconds */
int blinkLED(void)
{
    //______________10us burst____________________________________
    /* Trigger pin is initially low */
    IfxPort_setPinState(TRIGGER, IfxPort_State_low);

    /* Wait 2 microseconds */
    waitTime(IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER,2));

    /* Trigger pin is set high */
    IfxPort_setPinState(TRIGGER, IfxPort_State_high);

    /* Wait 10 microseconds */
    waitTime(IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER,10));

    /* After ten millisecond pin goes low again */
    IfxPort_setPinState(TRIGGER, IfxPort_State_low);


//_____________________________Measure time____________________________________
    IfxGtm_Tim_In_update(&g_driverTIM);                                         /* Update the measured data         */

    measuredTicks =  IfxGtm_Tim_In_getPulseLengthTick(&g_driverTIM);
    distance = (TICK2PERIOD*measuredTicks)*SOUNDSPEED*3; // distance in meter


    return (int)(distance*5) ;


}

/* This function initializes the TIM to capture a PWM signals */
void init_TIM(void)
{
    IfxGtm_enable(&MODULE_GTM);                                         /* Enable the GTM                           */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_CLK0);        /* Enable the CMU clock 0                   */

    IfxGtm_Tim_In_Config configTIM;

    IfxGtm_Tim_In_initConfig(&configTIM, &MODULE_GTM);                  /* Initialize default parameters            */
    configTIM.filter.inputPin = &ECHO;                                  /* Select input port pin                    */
    configTIM.filter.inputPinMode = IfxPort_InputMode_pullDown;         /* Select input port pin mode               */
    IfxGtm_Tim_In_init(&g_driverTIM, &configTIM);                       /* Initialize the TIM                       */
}
