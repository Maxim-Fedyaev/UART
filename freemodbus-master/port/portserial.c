/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

#include "port.h"
#include "mik32_hal_usart.h"
#include "mik32_hal_irq.h"
#include "mik32_hal_timer16.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Static variables ---------------------------------*/
USART_HandleTypeDef husart0;
extern Timer16_HandleTypeDef htimer16_1;
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    __HAL_PCC_UART_1_CLK_ENABLE(); 
    // set serial configure parameter 
    switch (ucPORT) {
    case 0:
        husart0.Instance = UART_0;
        __HAL_PCC_UART_0_CLK_ENABLE();        
        break;
    case 1:
        husart0.Instance = UART_1;
        __HAL_PCC_UART_1_CLK_ENABLE();        
        break;
    }

    husart0.transmitting = Enable;
    husart0.receiving = Enable;

    husart0.Interrupt.ctsie = Disable;
    husart0.Interrupt.eie = Disable;
    husart0.Interrupt.idleie = Disable;
    husart0.Interrupt.lbdie = Disable;
    husart0.Interrupt.peie = Disable;
    husart0.Interrupt.rxneie = Disable;
    husart0.Interrupt.tcie = Disable;
    husart0.Interrupt.txeie = Disable;

    // скорость   
    husart0.baudrate = ulBaudRate;

    // размер слова
    switch (ucDataBits) {
    case 8:
        husart0.frame = Frame_8bit;
        break;
    case 9:
        husart0.frame = Frame_9bit;
        break;
    }

    switch(eParity){
    case MB_PAR_NONE: 
        husart0.parity_bit = Disable;
        husart0.parity_bit_inversion = Disable;
        break;
    case MB_PAR_ODD: 
        husart0.parity_bit = Enable;
        husart0.parity_bit_inversion = Disable;
        break;
    case MB_PAR_EVEN: 
        husart0.parity_bit = Enable;
        husart0.parity_bit_inversion = Enable;
        break;
    }
    HAL_USART_Init(&husart0);   

    return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        /* enable RX interrupt */
        HAL_USART_RXNE_EnableInterrupt(&husart0);
    }
    else
    {
        /* disable RX interrupt */
        HAL_USART_RXNE_DisableInterrupt(&husart0);
    }
    if (xTxEnable)
    {
        /* start serial transmit */
        HAL_USART_TXE_EnableInterrupt(&husart0);
    }
    else
    {
        /* stop serial transmit */
        HAL_USART_TXE_DisableInterrupt(&husart0);
    }
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    husart0.Instance->TXDATA = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
    *pucByte = husart0.Instance->RXDATA;
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
    pxMBFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

void UART_1_IRQHandler()
{
    EPIC->CLEAR |= HAL_EPIC_UART_1_MASK;

    /* UART in mode Transmitter ------------------------------------------------*/
    if((HAL_USART_TXE_ReadFlag(&husart0) != 0) && ((UART_1->CONTROL1 & (1<<7)) != 0))
    {
        prvvUARTTxReadyISR(  );
    }
    /* UART in mode Receiver ---------------------------------------------------*/
    if((HAL_USART_RXNE_ReadFlag(&husart0) != 0) && ((UART_1->CONTROL1 & (1<<5)) != 0))
    { 
        HAL_Timer16_Disable(&htimer16_1);
        HAL_Timer16_Enable(&htimer16_1);
        __HAL_TIMER16_START_CONTINUOUS(&htimer16_1);
        prvvUARTRxISR(  ); 
    }    
}