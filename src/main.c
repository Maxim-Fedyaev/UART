/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_usart.h"
#include "mik32_hal_timer32.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "user_mb_app.h"

/* ----------------------- Declaration --------------------------------------*/
extern USART_HandleTypeDef husart0;
extern TIMER32_HandleTypeDef htimer32_0;
void SystemClock_Config(void);
extern void UART_IRQHandler(void);
extern void Timer32_IRQHandler(void);

extern unsigned long __TEXT_START__; // это "метка" для обработчика прерываний
volatile void trap_handler(void)     // сам обработчик всех прерываний
{
    if(HAL_EPIC_GetStatus() & HAL_EPIC_TIMER32_0_MASK)
    {
        Timer32_IRQHandler();
        HAL_EPIC_Clear(HAL_EPIC_TIMER32_0_MASK);
    }
    if(HAL_EPIC_GetStatus() & HAL_EPIC_UART_1_MASK)
    {
        UART_IRQHandler();
        HAL_EPIC_Clear(HAL_EPIC_UART_1_MASK);
    }
}

/* ----------------------- Основная программа --------------------------------*/
int main()
{
    write_csr(mtvec, &__TEXT_START__); // операция, настраивающая вектор прерываний

    SystemClock_Config();

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(HAL_EPIC_UART_1_MASK | HAL_EPIC_TIMER32_0_MASK); 
    HAL_IRQ_EnableInterrupts();

    eMBInit(MB_RTU, 0x11, 1, 38400, MB_PAR_NONE);
    eMBEnable();

    while (1)
    {
       eMBPoll();
    }
}

/* ----------------------- Инициализация тактирования -------------------------*/
void SystemClock_Config(void)
{
    PCC_InitTypeDef PCC_OscInit = {0};

    PCC_OscInit.OscillatorEnable = PCC_OSCILLATORTYPE_ALL;
    PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
    PCC_OscInit.FreqMon.ForceOscSys = PCC_FORCE_OSC_SYS_UNFIXED;
    PCC_OscInit.FreqMon.Force32KClk = PCC_FREQ_MONITOR_SOURCE_OSC32K;
    PCC_OscInit.AHBDivider = 0;
    PCC_OscInit.APBMDivider = 0;
    PCC_OscInit.APBPDivider = 0;
    PCC_OscInit.HSI32MCalibrationValue = 128;
    PCC_OscInit.LSI32KCalibrationValue = 128;
    PCC_OscInit.RTCClockSelection = PCC_RTC_CLOCK_SOURCE_AUTO;
    PCC_OscInit.RTCClockCPUSelection = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;
    HAL_PCC_Config(&PCC_OscInit);
}