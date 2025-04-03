/**
  ******************************************************************************
  * @file    debug_log.cpp
  * @author  Fahad Mirza
  * @brief   This file provides TensorFLow DebugLog() implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tensorflow/lite/micro/debug_log.h"

#include <stdio.h>


#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"




extern "C" void DebugLog(const char* s) { fprintf(stderr, "%s", s); }
