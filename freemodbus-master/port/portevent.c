
#include "mbconfig.h"

#if MB_SLAVE_RTU_ENABLED > 0
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Variables ----------------------------------------*/
static eMBEventType eQueuedEvent;
static uint8_t     xEventInQueue;

/* ----------------------- Start implementation -----------------------------*/
uint8_t
xMBPortEventInit( void )
{
    xEventInQueue = FALSE;
    return TRUE;
}

uint8_t
xMBPortEventPost( eMBEventType eEvent )
{
    xEventInQueue = TRUE;
    eQueuedEvent = eEvent;
    return TRUE;
}

uint8_t
xMBPortEventGet( eMBEventType * eEvent )
{
    uint8_t            xEventHappened = FALSE;

    if( xEventInQueue )
    {
        *eEvent = eQueuedEvent;
        xEventInQueue = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}
#endif