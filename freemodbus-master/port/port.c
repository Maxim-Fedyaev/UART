
/* ----------------------- System includes --------------------------------*/
#include "mik32_hal_irq.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"

/* ----------------------- Variables ----------------------------------------*/
static uint32_t lock_nesting_count = 0;

/* ----------------------- Start implementation -----------------------------*/
void EnterCriticalSection(void)
{
    HAL_IRQ_DisableInterrupts();
    ++lock_nesting_count;
}

void ExitCriticalSection(void)
{
    --lock_nesting_count;
    if (lock_nesting_count == 0)
    {
        HAL_IRQ_EnableInterrupts();
    }
}

