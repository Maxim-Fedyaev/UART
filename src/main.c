#include "mik32_hal_usart.h"
#include "mik32_hal_timer16.h"
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "user_mb_app.h"
#include "mik32_hal_gpio.h"


extern USART_HandleTypeDef husart0;
void SystemClock_Config(void);
extern void UART_1_IRQHandler(USART_HandleTypeDef *husart);
extern void Timer16_1_IRQHandler(void);

//static USHORT usRegInputStart = S_REG_INPUT_START;
static USHORT usRegInputBuf[S_REG_INPUT_NREGS] = {'M', 'F', 77, 1};

extern unsigned long __TEXT_START__; //это "метка" для обработчика прерываний?!
volatile void trap_handler(void) //сам обработчик всех прерываний
{
    if(HAL_EPIC_GetStatus() & HAL_EPIC_TIMER16_1_MASK)
    {
        Timer16_1_IRQHandler();
        HAL_EPIC_Clear(HAL_EPIC_TIMER16_1_MASK);
    }
    if(HAL_EPIC_GetStatus() & HAL_EPIC_UART_1_MASK)
    {
        UART_1_IRQHandler(&husart0);
        HAL_EPIC_Clear(HAL_EPIC_UART_1_MASK);
    }
}

int main()
{
    write_csr(mtvec, &__TEXT_START__); //это настраивает вектор прерываний?!

    SystemClock_Config();

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(HAL_EPIC_UART_1_MASK | HAL_EPIC_TIMER16_1_MASK); 
    HAL_IRQ_EnableInterrupts();

    volatile eMBErrorCode eStatus;
    eStatus = eMBInit(MB_RTU, 0x11, 1, 38400, MB_PAR_NONE);
    eStatus = eMBEnable();

    while (1)
    {
       eStatus = eMBPoll();
       usRegInputBuf[2]++; 

    }
}

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