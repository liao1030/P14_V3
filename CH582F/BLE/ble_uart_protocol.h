/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_protocol.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : BLE UART通訊協議頭文件
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#ifndef BLE_UART_PROTOCOL_H
#define BLE_UART_PROTOCOL_H

#include "CH58x_common.h"

/* 通訊協議常數定義 */
#define FRAME_START_BYTE                0xAA
#define FRAME_END_BYTE                  0x55

/* 指令ID定義 */
#define CMD_SYNC_TIME                   0x01
#define CMD_SYNC_TIME_ACK               0x81
#define CMD_REQUEST_STATUS              0x02
#define CMD_STATUS_RESPONSE             0x82
#define CMD_SET_CODE_EVENT              0x03
#define CMD_CODE_EVENT_ACK              0x83
#define CMD_BLOOD_SAMPLE_CHECK          0x04
#define CMD_BLOOD_SAMPLE_NOTIFY         0x84
#define CMD_REQUEST_RESULT              0x05
#define CMD_RESULT_RESPONSE             0x85
#define CMD_REQUEST_RAW_DATA            0x06
#define CMD_RAW_DATA_RESPONSE           0x86
#define CMD_START_T1_MEASUREMENT        0x10
#define CMD_T1_MEASUREMENT_RESULT       0x90
#define CMD_STRIP_DETECTED              0x11
#define CMD_STRIP_TYPE_CONFIRM          0x91
#define CMD_ERROR_RESPONSE              0xFF

/* 錯誤代碼定義 */
#define ERR_BATTERY_LOW                 0x01
#define ERR_TEMPERATURE_HIGH            0x02
#define ERR_TEMPERATURE_LOW             0x03
#define ERR_STRIP_EXPIRED               0x04
#define ERR_STRIP_USED                  0x05
#define ERR_STRIP_INSERT                0x06
#define ERR_BLOOD_INSUFFICIENT          0x07
#define ERR_MEASURE_TIMEOUT             0x08
#define ERR_CALIBRATION                 0x09
#define ERR_HARDWARE                    0x0A
#define ERR_COMMUNICATION               0x0B
#define ERR_DATA_FORMAT                 0x0C
#define ERR_CHECKSUM                    0x0D
#define ERR_COMMAND_NOTSUPPORT          0x0E
#define ERR_RESULT_OUTOFRANGE           0x0F
#define ERR_STRIP_TYPE_UNKNOWN          0x10

/* 測量類型定義 */
#define MEASURE_TYPE_GLV                0x0000
#define MEASURE_TYPE_U                  0x0001
#define MEASURE_TYPE_C                  0x0002
#define MEASURE_TYPE_TG                 0x0003
#define MEASURE_TYPE_GAV                0x0004
#define MEASURE_TYPE_UNKNOWN            0xFFFF

/* 試片類型定義 - 協議用 */
#define PROTOCOL_STRIP_UNKNOWN          0x00
#define PROTOCOL_STRIP_GLV              0x01
#define PROTOCOL_STRIP_U                0x02
#define PROTOCOL_STRIP_C                0x03
#define PROTOCOL_STRIP_TG               0x04
#define PROTOCOL_STRIP_GAV              0x05

/* 事件類型定義 */
#define EVENT_NONE                      0x0000
#define EVENT_AC                        0x0001
#define EVENT_PC                        0x0002
#define EVENT_QC                        0x0003

/* 協議報文最大長度 */
#define MAX_FRAME_LENGTH                64
#define MAX_DATA_LENGTH                 50

/* BLE服務UUID */
#define BLE_SERVICE_UUID                0xFFE0
#define BLE_CHAR_RX_UUID                0xFFE1
#define BLE_CHAR_TX_UUID                0xFFE2

/* 數據包結構定義 (協議請求/響應) */
typedef struct {
    uint8_t cmd_id;             // 指令ID
    uint8_t length;             // 數據區長度
    uint8_t data[MAX_FRAME_LENGTH]; // 數據區
    uint8_t checksum;           // 校驗和
} Protocol_Frame_t;

/* 設備狀態結構定義 */
typedef struct {
    uint16_t measure_type;      // 測量項目
    uint16_t strip_status;      // 試片狀態
    uint16_t battery_voltage;   // 電池電壓(mV)
    uint16_t temperature;       // 環境溫度(x10)
} Device_Status_t;

/* 測量結果結構定義 */
typedef struct {
    uint16_t result_status;     // 結果狀態
    uint16_t measure_value;     // 測量結果值
    uint16_t measure_type;      // 測量項目
    uint16_t event_type;        // 事件類型
    uint8_t  code;              // 試片校驗碼
    uint16_t year;              // 年份
    uint16_t month;             // 月份
    uint16_t day;               // 日期
    uint16_t hour;              // 小時
    uint16_t minute;            // 分鐘
    uint16_t second;            // 秒
    uint16_t battery_voltage;   // 電池電壓
    uint16_t temperature;       // 環境溫度
} Measure_Result_t;

/* 函數聲明 */
void BLE_Protocol_Init(void);
uint8_t BLE_Protocol_Parse(uint8_t *data, uint16_t length);
uint16_t BLE_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer);
uint8_t BLE_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length);
void BLE_Protocol_SendResponse(uint8_t cmd_id, uint8_t *data, uint8_t length);
void BLE_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code);
void BLE_UART_SendData(uint8_t *data, uint16_t length);

/* UART和BLE數據轉發函數 */
void BLE_To_UART_Forward(uint8_t *data, uint16_t length);
void UART_To_BLE_Forward(uint8_t *data, uint16_t length);

/* 協議函數聲明 */
void BLE_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* 處理指令的回調函數 */
void BLE_Protocol_HandleSyncTime(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestResult(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleT1MeasurementResult(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleStripTypeConfirm(uint8_t *data, uint8_t length);

#endif /* BLE_UART_PROTOCOL_H */ 