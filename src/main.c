/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_usart.h"
#include "mik32_hal_timer32.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "mb_m.h"
#include "user_mb_app.h"

/* -------------------------- Defines ---------------------------------------*/
// Маска прерываний 
#define UART_EPIC_MASK      HAL_EPIC_UART_1_MASK
#define TIMER32_EPIC_MASK   HAL_EPIC_TIMER32_0_MASK

// Настройки Modbus
#define MB_BaudRate         57600
#define MB_UART             1

/* ------------------------ Fuction -----------------------------------------*/
void SystemClock_Config(void);
extern void UART_IRQHandler(void);
extern void Timer32_IRQHandler(void);

extern unsigned long __TEXT_START__; // это "метка" для обработчика прерываний
volatile void trap_handler(void)     // сам обработчик всех прерываний
{
    if(HAL_EPIC_GetStatus() & TIMER32_EPIC_MASK)
    {
        Timer32_IRQHandler();
        HAL_EPIC_Clear(TIMER32_EPIC_MASK);
    }
    if(HAL_EPIC_GetStatus() & UART_EPIC_MASK)
    {
        UART_IRQHandler();
        HAL_EPIC_Clear(UART_EPIC_MASK);
    }
}

/* ----------------------- Основная программа --------------------------------*/
int main()
{
    write_csr(mtvec, &__TEXT_START__); // операция, настраивающая вектор прерываний

    SystemClock_Config();

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(UART_EPIC_MASK | TIMER32_EPIC_MASK); 
    HAL_IRQ_EnableInterrupts();
#if MB_SLAVE_RTU_ENABLED
    eMBInit(MB_SlaveAddress, MB_UART, MB_BaudRate, MB_PAR_NONE);
    eMBEnable();

    while (1)
    {
       eMBPoll();
    }
#endif
#if MB_MASTER_RTU_ENABLED   
    eMBMasterInit(MB_UART, MB_BaudRate, MB_PAR_NONE);
    eMBMasterEnable();

    while (1)
    {
        //eMBMasterReqWriteHoldingRegister(MB_SlaveAddress, 0x1234, 0x5678, 1000);
        eMBMasterReqReadInputRegister( MB_SlaveAddress, 30000, 4, 1000 );
        eMBMasterPoll();
        HAL_DelayMs(10000);
        
    }
#endif
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