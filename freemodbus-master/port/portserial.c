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

/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_usart.h"
#include "mik32_hal_irq.h"
#include "mik32_hal_timer32.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Declaration --------------------------------------*/
USART_HandleTypeDef husart;
extern TIMER32_HandleTypeDef htimer32;

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
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

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
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

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    husart.Instance->TXDATA = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
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