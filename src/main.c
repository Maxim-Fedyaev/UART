#include "mik32_hal_usart.h"
#include "mik32_hal_timer16.h"
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "user_mb_app.h"

Timer16_HandleTypeDef htimer16_0;
extern USART_HandleTypeDef husart0;

void SystemClock_Config(void);
void Timer16_0_Init(void);
extern void UART_1_IRQHandler(USART_HandleTypeDef *husart);

static USHORT usRegInputStart = 1000;
static USHORT usRegInputBufp[4];

extern unsigned long __TEXT_START__; //это "метка" для обработчика прерываний?!
void trap_handler(void) //сам обработчик всех прерываний
{
  UART_1_IRQHandler(&husart0);
}

int main()
{
    write_csr(mtvec, &__TEXT_START__); //это настраивает вектор прерываний?!
    SystemClock_Config();

    Timer16_0_Init(); 

    __HAL_PCC_EPIC_CLK_ENABLE();
    HAL_EPIC_MaskEdgeSet(HAL_EPIC_UART_1_MASK); 
    HAL_IRQ_EnableInterrupts();

    eMBErrorCode eStatus;
    eStatus = eMBInit(MB_RTU, 0x11, 0, 3840, MB_PAR_NONE);
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

void Timer16_0_Init(void)
{
    htimer16_0.Instance = TIMER16_0;

    /* Настройка тактирования */
    htimer16_0.Clock.Source = TIMER16_SOURCE_INTERNAL_SYSTEM;
    htimer16_0.CountMode = TIMER16_COUNTMODE_INTERNAL; /* При тактировании от Input1 не имеет значения */
    htimer16_0.Clock.Prescaler = TIMER16_PRESCALER_1;
    htimer16_0.ActiveEdge = TIMER16_ACTIVEEDGE_RISING; /* Выбирается при тактировании от Input1 */

    /* Настройка режима обновления регистра ARR и CMP */
    htimer16_0.Preload = TIMER16_PRELOAD_AFTERWRITE;

    /* Настройки фильтра */
    htimer16_0.Filter.ExternalClock = TIMER16_FILTER_NONE;
    htimer16_0.Filter.Trigger = TIMER16_FILTER_NONE;

    htimer16_0.Waveform.Enable = TIMER16_WAVEFORM_GENERATION_ENABLE;
    htimer16_0.Waveform.Polarity = TIMER16_WAVEFORM_POLARITY_NONINVERTED;

    HAL_Timer16_Init(&htimer16_0);
}
