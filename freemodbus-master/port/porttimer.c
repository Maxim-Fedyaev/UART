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
 * File: $Id: porttimer.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "mik32_hal_timer32.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "port.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);
void Timer32_0_IRQHandler(void);
TIMER32_HandleTypeDef htimer32;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    htimer32.Instance = TIMER32_0;
    htimer32.Top = 25*usTim1Timerout50us;
    htimer32.State = TIMER32_STATE_DISABLE;
    htimer32.Clock.Source = TIMER32_SOURCE_PRESCALER;
    htimer32.Clock.Prescaler = 63;
    htimer32.InterruptMask = 0;
    htimer32.CountMode = TIMER32_COUNTMODE_FORWARD;
    HAL_Timer32_Init(&htimer32);
    return TRUE;
}

void vMBPortTimersEnable()
{
    HAL_Timer32_Base_Start_IT(&htimer32);
}

void vMBPortTimersDisable()
{
    HAL_Timer32_Base_Stop_IT(&htimer32);
}

void prvvTIMERExpiredISR(void)
{
    (void) pxMBPortCBTimerExpired();
}

void Timer32_IRQHandler(void)
{
    prvvTIMERExpiredISR();
    HAL_TIMER32_INTERRUPTFLAGS_CLEAR(&htimer32);
}
