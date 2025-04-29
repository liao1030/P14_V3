/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UART通訊協議頭文件
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "debug.h"

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

/* 測量類型定義 */
#define MEASURE_TYPE_GLV                0x0000
#define MEASURE_TYPE_U                  0x0001
#define MEASURE_TYPE_C                  0x0002
#define MEASURE_TYPE_TG                 0x0003
#define MEASURE_TYPE_GAV                0x0004

/* 事件類型定義 */
#define UART_EVENT_NONE                 0x0000
#define UART_EVENT_AC                   0x0001
#define UART_EVENT_PC                   0x0002
#define UART_EVENT_QC                   0x0003

/* 協議報文最大長度 */
#define MAX_FRAME_LENGTH                64
#define MAX_DATA_LENGTH                 50

/* 重傳機制相關參數 */
#define MAX_RETRY_COUNT                 3
#define RETRY_TIMEOUT_MS                500

/* RTC時間結構 */
typedef struct {
    uint16_t year;                      // 年份
    uint8_t month;                      // 月份
    uint8_t day;                        // 日期
    uint8_t hour;                       // 小時
    uint8_t minute;                     // 分鐘
    uint8_t second;                     // 秒
} RTC_TimeStruct_t;

/* 設備狀態結構定義 */
typedef struct {
    uint16_t measure_type;              // 測量項目
    uint16_t strip_status;              // 試片狀態
    uint16_t battery_voltage;           // 電池電壓(mV)
    uint16_t temperature;               // 環境溫度(x10)
} Device_Status_t;

/* 測量結果結構定義 */
typedef struct {
    uint16_t result_status;             // 結果狀態
    uint16_t measure_value;             // 測量結果值
    uint16_t measure_type;              // 測量項目
    uint16_t event_type;                // 事件類型
    uint8_t  code;                      // 試片校驗碼
    RTC_TimeStruct_t time;              // 測量時間
    uint16_t battery_voltage;           // 電池電壓
    uint16_t temperature;               // 環境溫度
} Measure_Result_t;

/* 測量參數結構定義 */
typedef struct {
    uint8_t code;                       // 試片校驗碼
    uint16_t event;                     // 事件類型
} Measure_Param_t;

/* 協議函數聲明 */
void UART_Protocol_Init(void);
uint8_t UART_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length);
uint16_t UART_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer);
void UART_Protocol_SendFrame(uint8_t cmd_id, uint8_t *data, uint8_t length);
void UART_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code);
uint8_t UART_Protocol_Parse(uint8_t *data, uint16_t length);
void UART_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* 處理指令的回調函數 */
void UART_Protocol_HandleSyncTime(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length);
void UART_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length);
void UART_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestResult(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length);

/* 主動發送函數 */
void UART_Protocol_SendBloodSampleNotify(uint8_t countdown_seconds);
void UART_Protocol_SendStatus(Device_Status_t *status);
void UART_Protocol_SendResult(Measure_Result_t *result);
void UART_Protocol_SendRawData(uint8_t *data, uint16_t length);

#endif /* UART_PROTOCOL_H */ 