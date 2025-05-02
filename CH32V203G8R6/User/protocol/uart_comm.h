/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_comm.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀 UART 通訊驅動標頭檔
 ********************************************************************************/

#ifndef __UART_COMM_H
#define __UART_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include "protocol.h"

/* UART通訊相關常數 */
#define UART_BAUDRATE        115200          // 波特率
#define UART_BUFFER_SIZE     256             // 緩衝區大小

/* UART通訊超時相關參數 */
#define UART_TIMEOUT_MS      500             // 通訊超時(ms)
#define UART_MAX_RETRIES     3               // 最大重試次數

/* UART通訊接收回調函數類型 */
typedef void (*UART_RxCallback_t)(uint8_t *data, uint16_t length);

/* UART通訊初始化 */
void UART_Comm_Init(void);

/* UART數據發送函數 */
uint8_t UART_SendData(uint8_t *data, uint16_t length);

/* UART數據接收處理 */
void UART_ProcessRxData(void);

/* 設置接收回調函數 */
void UART_SetRxCallback(UART_RxCallback_t callback);

/* UART中斷處理函數 */
void UART_IRQHandler(void);

/* UART中斷使能/失能 */
void UART_EnableRxInt(FunctionalState state);
void UART_EnableTxInt(FunctionalState state);

#ifdef __cplusplus
}
#endif

#endif /* __UART_COMM_H */