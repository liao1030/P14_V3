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
 * Tx����(֪ͨ) UUID: 0xFFE1
 * Rx����(����) UUID: 0xFFE2
 */

// ���ն��x
#define BLE_UART_SERVICE_UUID    0xFFE0

// ���Զ��x
#define BLE_UART_TX_CHAR_UUID    0xFFE1  // ֪ͨ����
#define BLE_UART_RX_CHAR_UUID    0xFFE2  // ��������

// ���վ��n�^��С
#define BLE_UART_RX_BUFF_SIZE    20  // ���υf�hҪ��

/**
 * BLE UART�¼����
 */
typedef enum {
    BLE_UART_EVT_TX_NOTI_DISABLED,        // ֪ͨ�ѽ���
    BLE_UART_EVT_TX_NOTI_ENABLED,         // ֪ͨ�ц���
    BLE_UART_EVT_BLE_DATA_RECIEVED,       // �յ�BLE����
} ble_uart_evt_type_t;

/**
 * BLE UART�����Y��
 */
typedef struct {
    uint8_t const *p_data;  // ����ָ�
    uint16_t      length;   // �����L��
} ble_uart_data_t;

/**
 * BLE UART�¼��Y��
 */
typedef struct {
    ble_uart_evt_type_t type;         // �¼����
    ble_uart_data_t     data;         // �¼�����
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
