
#include "mbconfig.h"

#if MB_SLAVE_RTU_ENABLED > 0
/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_timer32.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "port.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);
TIMER32_HandleTypeDef htimer32;

/* ----------------------- Start implementation -----------------------------*/
uint8_t xMBPortTimersInit(uint16_t usTim1Timerout50us)
{
    htimer32.Instance = TIMER32_0;
    htimer32.Top = 25*usTim1Timerout50us;
    htimer32.State = TIMER32_STATE_DISABLE;
    htimer32.Clock.Source = TIMER32_SOURCE_PRESCALER;
    htimer32.Clock.Prescaler = 63;
    htimer32.InterruptMask = 0;
    htimer32.CountMode = TIMER32_COUNTMODE_FORWARD;
    HAL_Timer32_Init(&htimer32);
    return TRUE;
}

void vMBPortTimersEnable()
{
    HAL_Timer32_Base_Start_IT(&htimer32);
}

void vMBPortTimersDisable()
{
    HAL_Timer32_Base_Stop_IT(&htimer32);
}

void prvvTIMERExpiredISR(void)
{
    (void) pxMBPortCBTimerExpired();
}

void Timer32_IRQHandler(void)
{
    prvvTIMERExpiredISR();
    HAL_TIMER32_INTERRUPTFLAGS_CLEAR(&htimer32);
}
#endif