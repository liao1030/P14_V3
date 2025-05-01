/**
 * @file P14_UART.h
 * @brief �๦�������z�y�xUARTͨӍ����
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

/* UART���n�^��С */
#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

/* �������� */
void UART1_Init(uint32_t baudRate);
void UART1_SendData(uint8_t *data, uint16_t len);
void UART1_ProcessReceivedData(void);

/* UART�¼����{���� */
void UART1_RxCallback(uint8_t *data, uint16_t length);

#endif /* __P14_UART_H */ 