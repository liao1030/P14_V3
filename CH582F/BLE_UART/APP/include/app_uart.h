/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_uart.h
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2024/05/08
 * Description        : �๦�������x�{��UART���Ì�
 *********************************************************************************
 * Copyright (c) 2024 HMD. All rights reserved.
 *******************************************************************************/

#ifndef app_uart_H
#define app_uart_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "CONFIG.h"
#include "app_drv_fifo.h"
#include "ble_uart_service.h"
#include "protocol_handler.h"

/*********************************************************************
 * CONSTANTS
 */

extern uint8_t to_test_buffer[BLE_BUFF_MAX_LEN - 4 - 3];

extern app_drv_fifo_t app_uart_tx_fifo;
extern app_drv_fifo_t app_uart_rx_fifo;

//interupt uart rx flag ,clear at main loop
extern bool uart_rx_flag;

//for interrupt rx blcak hole ,when uart rx fifo full
extern uint8_t for_uart_rx_black_hole;

//fifo length less that MTU-3, retry times
extern uint32_t uart_to_ble_send_evt_cnt;

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

extern void app_uart_process(void);

extern void app_uart_init(void);

extern void app_uart_tx_data(uint8_t *data, uint16_t length);

extern void on_bleuartServiceEvt(uint16_t connection_handle, ble_uart_evt_t *p_evt);

// ��UART�����l����BLE�ĺ���
void uart_to_ble_send(void);

// ͨ�^BLE֪ͨ�l�͔���
void ble_uart_notify_data(uint8_t *data, uint16_t length);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
