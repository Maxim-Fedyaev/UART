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
#include "port.h"
#include "mik32_hal_timer16.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);
extern Timer16_HandleTypeDef htimer16_0;
uint16_t timeout = 0;
volatile uint16_t counter = 0;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    timeout = usTim1Timerout50us;
    return TRUE;
}

void vMBPortTimersEnable()
{
    counter = 0;
    HAL_Timer16_Counter_Start(&htimer16_0, 1600);
}

void vMBPortTimersDisable()
{
    HAL_Timer16_Disable(&htimer16_0);
}

void prvvTIMERExpiredISR(void)
{
    (void) pxMBPortCBTimerExpired();
}

void HAL_TIM_PeriodElapsedCallback(Timer16_HandleTypeDef *htimer16_0)
{
    if((++counter) >= timeout)
    {
        prvvTIMERExpiredISR();
    }
}
