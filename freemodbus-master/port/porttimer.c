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
void Timer16_0_Init(void);
void Timer16_0_IRQHandler(Timer16_HandleTypeDef *htimer16);
Timer16_HandleTypeDef htimer16_0;
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
    HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_9);
    counter = 0;
    HAL_Timer16_Enable(&htimer16_0);

    HAL_Timer16_SetARR(&htimer16_0, 50000);
}

void vMBPortTimersDisable()
{
    HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_9);
    HAL_Timer16_Disable(&htimer16_0);
}

void prvvTIMERExpiredISR(void)
{
    (void) pxMBPortCBTimerExpired();
}

void Timer16_0_IRQHandler(Timer16_HandleTypeDef *htimer16_0)
{
    EPIC->CLEAR |= HAL_EPIC_TIMER16_0_MASK;
    if((++counter) >= timeout)
    {
        prvvTIMERExpiredISR();
    }
}

void Timer16_0_Init(void)
{
    htimer16_0.Instance = TIMER16_0;

    /* Настройка тактирования */
    htimer16_0.Clock.Source = TIMER16_SOURCE_INTERNAL_OSC32M;
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
