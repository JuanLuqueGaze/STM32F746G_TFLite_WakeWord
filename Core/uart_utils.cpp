// filepath: c:\Users\gie-5\STM32CubeIDE\workspace_1.17.0\STM32F746G_TFLite_WakeWord\Core\uart_utils.cpp
#include "uart_utils.h"
#include "stm32f7xx_hal.h"  // Adjust this include based on your STM32 series
#include <cstring>

extern UART_HandleTypeDef DebugUartHandler;  // Ensure this is declared elsewhere

void PrintToUart(const char* message) {
    HAL_UART_Transmit(&DebugUartHandler, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}