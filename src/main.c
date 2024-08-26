#include "mik32_hal_usart.h"
#include "mik32_hal_timer16.h"
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "user_mb_app.h"
#include "mik32_hal_gpio.h"

extern USART_HandleTypeDef husart0;
extern Timer16_HandleTypeDef htimer16_0;
void SystemClock_Config(void);
extern void Timer16_0_IRQHandler(Timer16_HandleTypeDef *htimer16);
extern void UART_1_IRQHandler(USART_HandleTypeDef *husart);
extern void Timer16_0_Init(void);
void GPIO_Init();
static USHORT usRegInputStart = S_REG_INPUT_START;
static USHORT usRegInputBuf[S_REG_INPUT_NREGS];

extern unsigned long __TEXT_START__; //это "метка" для обработчика прерываний?!
void trap_handler(void) //сам обработчик всех прерываний
{
    if(TIMER16_0->ISR & 2)
        Timer16_0_IRQHandler(&htimer16_0);
    if(UART_1->FLAGS & 0xA0)
        UART_1_IRQHandler(&husart0);
}

int main()
{
    write_csr(mtvec, &__TEXT_START__); //это настраивает вектор прерываний?!
    SystemClock_Config();

    Timer16_0_Init(); 

    GPIO_Init();

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(HAL_EPIC_UART_1_MASK | HAL_EPIC_TIMER16_0_MASK); 
    HAL_IRQ_EnableInterrupts();

    HAL_DelayMs(2000);
    xMBPortTimersInit(20000);
    vMBPortTimersEnable();
    //eMBErrorCode eStatus;
    //eStatus = eMBInit(MB_RTU, 0x11, 1, 38400, MB_PAR_NONE);
   // eStatus = eMBEnable();

    while (1)
    {
       //(void) eMBPoll();
      // usRegInputBuf[0]++; 
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

void GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
    __HAL_PCC_GPIO_IRQ_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);
}