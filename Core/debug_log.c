/**
  ******************************************************************************
  * @file    debug_log.cpp
  * @author  Fahad Mirza
  * @brief   This file provides TensorFLow DebugLog() implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tensorflow/lite/micro/debug_log.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#include <stdio.h>

/* Extern Variables ---------------------------------------------------------*/
extern UART_HandleTypeDef DebugUartHandler;


int __io_putchar(int ch)
{
	HAL_UART_Transmit(&DebugUartHandler, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


// Used by TFLite error_reporter
void DebugLog(const char *s)
{
	fprintf(stderr, "%s", s);
	// NOTE: fprintf uses _write(), which is defined in syscall.c
	//       _write() uses __io_putchar(), which is a weak function
	//       and needs to be implemented by user.
}
