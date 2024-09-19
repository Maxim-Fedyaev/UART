
#include "mbconfig.h"

#if MB_MASTER_RTU_ENABLED > 0
/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_timer32.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_m.h"
#include "mbport.h"

/* ----------------------- Variables ----------------------------------------*/
static void prvvTIMERExpiredISR(void);
TIMER32_HandleTypeDef htimer32;

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);

/* ----------------------- Start implementation -----------------------------*/
uint8_t xMBMasterPortTimersInit(uint16_t usTimeOut50us)
{
    htimer32.Instance = TIMER32_0;
    htimer32.Top = 25*usTimeOut50us;
    htimer32.State = TIMER32_STATE_DISABLE;
    htimer32.Clock.Source = TIMER32_SOURCE_PRESCALER;
    htimer32.Clock.Prescaler = 63;
    htimer32.InterruptMask = 0;
    htimer32.CountMode = TIMER32_COUNTMODE_FORWARD;
    HAL_Timer32_Init(&htimer32);
    return TRUE;
}

void vMBMasterPortTimersT35Enable()
{
    HAL_Timer32_Base_Start_IT(&htimer32);
}

void vMBMasterPortTimersConvertDelayEnable()
{

}

void vMBMasterPortTimersRespondTimeoutEnable()
{

}

void vMBMasterPortTimersDisable()
{
    HAL_Timer32_Base_Stop_IT(&htimer32);
}

void prvvTIMERExpiredISR(void)
{
    (void) pxMBMasterPortCBTimerExpired();
}

void Timer32_IRQHandler(void)
{
    prvvTIMERExpiredISR();
    HAL_TIMER32_INTERRUPTFLAGS_CLEAR(&htimer32);
}
#endif
