/**
 * @file P14_UART.h
 * @brief 多功能生化檢測儀UART通訊介面
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_UART_H
#define __P14_UART_H

#include <stdint.h>
#include "ch32v20x.h"

/* UART緩衝區大小 */
#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

/* 函數宣告 */
void UART1_Init(uint32_t baudRate);
void UART1_SendData(uint8_t *data, uint16_t len);
void UART1_ProcessReceivedData(void);

/* UART事件回調函數 */
void UART1_RxCallback(uint8_t *data, uint16_t length);

#endif /* __P14_UART_H */ 