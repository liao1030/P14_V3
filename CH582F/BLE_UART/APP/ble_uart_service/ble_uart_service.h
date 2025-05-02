/********************************** (C) COPYRIGHT *******************************
 * File Name          : simpleGATTprofile.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/19
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef _BLE_UART_SERVICE_H
#define _BLE_UART_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "att.h"
#include "stdint.h"

/**
 * P14-BLE-UART Service UUID: 0xFFE0
 * Tx特性(通知) UUID: 0xFFE1
 * Rx特性(寫入) UUID: 0xFFE2
 */

// 服務定義
#define BLE_UART_SERVICE_UUID    0xFFE0

// 特性定義
#define BLE_UART_TX_CHAR_UUID    0xFFE1  // 通知特性
#define BLE_UART_RX_CHAR_UUID    0xFFE2  // 寫入特性

// 接收緩衝區大小
#define BLE_UART_RX_BUFF_SIZE    20  // 符合協議要求

/**
 * BLE UART事件類型
 */
typedef enum {
    BLE_UART_EVT_TX_NOTI_DISABLED,        // 通知已禁用
    BLE_UART_EVT_TX_NOTI_ENABLED,         // 通知已啟用
    BLE_UART_EVT_BLE_DATA_RECIEVED,       // 收到BLE數據
} ble_uart_evt_type_t;

/**
 * BLE UART數據結構
 */
typedef struct {
    uint8_t const *p_data;  // 數據指針
    uint16_t      length;   // 數據長度
} ble_uart_data_t;

/**
 * BLE UART事件結構
 */
typedef struct {
    ble_uart_evt_type_t type;         // 事件類型
    ble_uart_data_t     data;         // 事件數據
} ble_uart_evt_t;

typedef void (*ble_uart_ProfileChangeCB_t)(uint16_t connection_handle, ble_uart_evt_t *p_evt);

/*********************************************************************
 * API FUNCTIONS
 */

/*
 * ble_uart_AddService- Initializes the raw pass GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t ble_uart_add_service(ble_uart_ProfileChangeCB_t cb);

extern uint8 ble_uart_notify_is_ready(uint16 connHandle);

extern bStatus_t ble_uart_notify(uint16 connHandle, attHandleValueNoti_t *pNoti, uint8 taskId);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _BLE_UART_SERVICE_H */
