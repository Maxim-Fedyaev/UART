#include "mik32_hal_usart.h"
#include "mik32_hal_timer16.h"
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "user_mb_app.h"
#include "mik32_hal_gpio.h"


extern USART_HandleTypeDef husart0;
Timer16_HandleTypeDef htimer16_1;
void SystemClock_Config(void);
extern void UART_1_IRQHandler(USART_HandleTypeDef *husart);
extern void Timer16_1_IRQHandler(void);
void Timer16_1_Init(void);
void GPIO_Init();

static USHORT usRegInputStart = S_REG_INPUT_START;
static USHORT usRegInputBuf[S_REG_INPUT_NREGS];

extern unsigned long __TEXT_START__; //это "метка" для обработчика прерываний?!
void trap_handler(void) //сам обработчик всех прерываний
{
    if(TIMER16_1->ISR & 2)
        Timer16_1_IRQHandler();
    if(UART_1->FLAGS & 0xA0)
        UART_1_IRQHandler(&husart0);
}

int main()
{
    write_csr(mtvec, &__TEXT_START__); //это настраивает вектор прерываний?!

    SystemClock_Config();

    Timer16_1_Init(); 

    GPIO_Init();

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(HAL_EPIC_UART_1_MASK | HAL_EPIC_TIMER16_1_MASK); 
    HAL_IRQ_EnableInterrupts();

    eMBErrorCode eStatus;
    eStatus = eMBInit(MB_RTU, 0x11, 1, 38400, MB_PAR_NONE);
    eStatus = eMBEnable();

    while (1)
    {
       (void) eMBPoll();
       usRegInputBuf[0]++; 
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

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIO_0, GPIO_PIN_10, 1);
}

void Timer16_1_Init(void)
{
    htimer16_1.Instance = TIMER16_1;

    /* Настройка тактирования */
    htimer16_1.Clock.Source = TIMER16_SOURCE_INTERNAL_OSC32M;
    htimer16_1.CountMode = TIMER16_COUNTMODE_INTERNAL; /* При тактировании от Input1 не имеет значения */
    htimer16_1.Clock.Prescaler = TIMER16_PRESCALER_1;
    htimer16_1.ActiveEdge = TIMER16_ACTIVEEDGE_RISING; /* Выбирается при тактировании от Input1 */

    /* Настройка режима обновления регистра ARR и CMP */
    htimer16_1.Preload = TIMER16_PRELOAD_AFTERWRITE;

    /* Настройка триггера */
    htimer16_1.Trigger.Source = TIMER16_TRIGGER_TIM1_GPIO1_9;
    htimer16_1.Trigger.ActiveEdge = TIMER16_TRIGGER_ACTIVEEDGE_SOFTWARE; /* При использовании триггера значение должно быть отлично от software */
    htimer16_1.Trigger.TimeOut = TIMER16_TIMEOUT_DISABLE;                /* Разрешить повторное срабатывание триггера */

    /* Настройки фильтра */
    htimer16_1.Filter.ExternalClock = TIMER16_FILTER_NONE;
    htimer16_1.Filter.Trigger = TIMER16_FILTER_NONE;

    /* Настройка режима энкодера */
    htimer16_1.EncoderMode = TIMER16_ENCODER_ENABLE;

    htimer16_1.Waveform.Enable = TIMER16_WAVEFORM_GENERATION_ENABLE;
    htimer16_1.Waveform.Polarity = TIMER16_WAVEFORM_POLARITY_NONINVERTED;

    HAL_Timer16_Init(&htimer16_1);
    
    HAL_Timer16_SetInterruptARRM(&htimer16_1);
}