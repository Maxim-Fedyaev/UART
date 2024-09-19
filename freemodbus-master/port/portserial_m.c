
#include "mbconfig.h"

#if MB_MASTER_RTU_ENABLED > 0
/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_usart.h"
#include "mik32_hal_irq.h"
#include "mik32_hal_timer32.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_m.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Static variables ---------------------------------*/
USART_HandleTypeDef husart;
extern TIMER32_HandleTypeDef htimer32;

/* ----------------------- Defines ------------------------------------------*/


/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

/* ----------------------- Start implementation -----------------------------*/
uint8_t xMBMasterPortSerialInit(uint8_t ucPORT, uint32_t ulBaudRate, uint8_t ucDataBits,
        eMBParity eParity)
{
    // выбор UART
    switch (ucPORT) {
    case 0:
        husart.Instance = UART_0;
        __HAL_PCC_UART_0_CLK_ENABLE();        
        break;
    case 1:
        husart.Instance = UART_1;
        __HAL_PCC_UART_1_CLK_ENABLE();        
        break;
    }

    husart.transmitting = Enable;
    husart.receiving = Enable;

    // скорость   
    husart.baudrate = ulBaudRate;

    // размер слова
    switch (ucDataBits) {
    case 8:
        husart.frame = Frame_8bit;
        break;
    case 9:
        husart.frame = Frame_9bit;
        break;
    }

    //бит четности
    switch(eParity) {
    case MB_PAR_NONE: 
        husart.parity_bit = Disable;
        husart.parity_bit_inversion = Disable;
        break;
    case MB_PAR_ODD: 
        husart.parity_bit = Enable;
        husart.parity_bit_inversion = Disable;
        break;
    case MB_PAR_EVEN: 
        husart.parity_bit = Enable;
        husart.parity_bit_inversion = Enable;
        break;
    }
    HAL_USART_Init(&husart);   

    return TRUE;
}

void vMBMasterPortSerialEnable(uint8_t xRxEnable, uint8_t xTxEnable)
{
    if (xRxEnable)
    {
        /* enable RX interrupt */
        HAL_USART_RXNE_EnableInterrupt(&husart);
    }
    else
    {
        /* disable RX interrupt */
        HAL_USART_RXNE_DisableInterrupt(&husart);
    }
    if (xTxEnable)
    {
        /* start serial transmit */
        HAL_USART_TXE_EnableInterrupt(&husart);
    }
    else
    {
        /* stop serial transmit */
        HAL_USART_TXE_DisableInterrupt(&husart);
    }
}

uint8_t xMBMasterPortSerialPutByte(int8_t ucByte)
{
    husart.Instance->TXDATA = ucByte;
    return TRUE;
}

uint8_t xMBMasterPortSerialGetByte(int8_t * pucByte)
{
    *pucByte = husart.Instance->RXDATA;
    return TRUE;
}

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
    pxMBMasterFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
    pxMBMasterFrameCBByteReceived();
}

void UART_IRQHandler() //подпрограама обработки прерываний от UART
{
    /* UART in mode Transmitter ------------------------------------------------*/
    if((HAL_USART_TXE_ReadFlag(&husart) != 0) && ((husart.Instance->CONTROL1 & (1<<7)) != 0))
    {
        prvvUARTTxReadyISR(  );
    }
    /* UART in mode Receiver ---------------------------------------------------*/
    if((HAL_USART_RXNE_ReadFlag(&husart) != 0) && ((husart.Instance->CONTROL1 & (1<<5)) != 0))
    { 
        HAL_TIMER32_VALUE_CLEAR(&htimer32);
        prvvUARTRxISR(  ); 
    }    
}
#endif