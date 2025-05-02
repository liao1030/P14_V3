/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_comm.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x UART ͨӍ�Ә��^�n
 ********************************************************************************/

#ifndef __UART_COMM_H
#define __UART_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include "protocol.h"

/* UARTͨӍ���P���� */
#define UART_BAUDRATE        115200          // ������
#define UART_BUFFER_SIZE     256             // ���n�^��С

/* UARTͨӍ���r���P���� */
#define UART_TIMEOUT_MS      500             // ͨӍ���r(ms)
#define UART_MAX_RETRIES     3               // �����ԇ�Δ�

/* UARTͨӍ���ջ��{������� */
typedef void (*UART_RxCallback_t)(uint8_t *data, uint16_t length);

/* UARTͨӍ��ʼ�� */
void UART_Comm_Init(void);

/* UART�����l�ͺ��� */
uint8_t UART_SendData(uint8_t *data, uint16_t length);

/* UART��������̎�� */
void UART_ProcessRxData(void);

/* �O�ý��ջ��{���� */
void UART_SetRxCallback(UART_RxCallback_t callback);

/* UART�Д�̎���� */
void UART_IRQHandler(void);

/* UART�Д�ʹ��/ʧ�� */
void UART_EnableRxInt(FunctionalState state);
void UART_EnableTxInt(FunctionalState state);

#ifdef __cplusplus
}
#endif

#endif /* __UART_COMM_H */