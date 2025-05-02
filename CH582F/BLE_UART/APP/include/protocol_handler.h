/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.h
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2023/04/28
 * Description        : P14多功能生化儀讀數藍牙與UART通訊協議處理
 *********************************************************************************
 * Copyright (c) 2023 HMD. All rights reserved.
 *******************************************************************************/

#ifndef __PROTOCOL_HANDLER_H
#define __PROTOCOL_HANDLER_H

#include "CH58x_common.h"

// 協議常量定義
#define PROTOCOL_START_MARKER    0xAA    // 起始標記
#define PROTOCOL_END_MARKER      0x55    // 結束標記

// 命令ID定義
#define CMD_SYNC_TIME            0x01    // 同步時間
#define CMD_SYNC_TIME_ACK        0x81    // 時間同步確認
#define CMD_REQ_DEVICE_STATUS    0x02    // 請求設備狀態
#define CMD_DEVICE_STATUS_ACK    0x82    // 設備狀態回應
#define CMD_SET_CODE_EVENT       0x03    // 設定CODE和EVENT
#define CMD_CODE_EVENT_ACK       0x83    // CODE/EVENT設定確認
#define CMD_BLOOD_SAMPLE_STATUS  0x04    // 檢測狀態請求
#define CMD_BLOOD_SAMPLE_ACK     0x84    // 血液檢測通知
#define CMD_REQ_RESULT           0x05    // 請求檢測結果
#define CMD_RESULT_ACK           0x85    // 檢測結果回應
#define CMD_REQ_RAW_DATA         0x06    // 請求RAW DATA
#define CMD_RAW_DATA_ACK         0x86    // RAW DATA回應
#define CMD_ERROR_ACK            0xFF    // 錯誤回應

// 錯誤代碼
#define ERR_LOW_BATTERY          0x01    // 電池電量過低
#define ERR_TEMP_HIGH            0x02    // 溫度過高
#define ERR_TEMP_LOW             0x03    // 溫度過低
#define ERR_STRIP_EXPIRED        0x04    // 試紙過期或損壞
#define ERR_STRIP_USED           0x05    // 試紙已使用
#define ERR_STRIP_ERROR          0x06    // 試紙插入錯誤
#define ERR_BLOOD_SAMPLE_INSUF   0x07    // 血液樣本不足
#define ERR_MEASURE_TIMEOUT      0x08    // 檢測超時
#define ERR_CALIB_ERROR          0x09    // 校正錯誤
#define ERR_HARDWARE_ERROR       0x0A    // 硬件錯誤
#define ERR_COMM_ERROR           0x0B    // 通訊錯誤
#define ERR_DATA_FORMAT          0x0C    // 資料格式錯誤
#define ERR_CHECKSUM             0x0D    // 校驗和錯誤
#define ERR_COMMAND_UNSUPPORTED  0x0E    // 指令不支持
#define ERR_RESULT_OUT_OF_RANGE  0x0F    // 檢測結果超出範圍

// 檢測項目
#define MEASURE_ITEM_GLV         0x0000  // 血糖
#define MEASURE_ITEM_U           0x0001  // 尿酸
#define MEASURE_ITEM_C           0x0002  // 總膽固醇
#define MEASURE_ITEM_TG          0x0003  // 三酸甘油脂
#define MEASURE_ITEM_GAV         0x0004  // 血糖

// 事件類型
#define EVENT_NONE               0x0000  // 無
#define EVENT_AC                 0x0001  // AC(飯前)
#define EVENT_PC                 0x0002  // PC(飯後)
#define EVENT_QC                 0x0003  // QC

// 最大封包長度定義
#define MAX_PACKET_SIZE          64      // 最大封包長度

// 協議封包結構
typedef struct {
    uint8_t startMarker;         // 起始標記 (0xAA)
    uint8_t cmdId;               // 指令ID
    uint8_t dataLen;             // 數據段長度(N)
    uint8_t data[MAX_PACKET_SIZE]; // 數據
    uint8_t checksum;            // 校驗和
    uint8_t endMarker;           // 結束標記 (0x55)
} protocol_packet_t;

// 設備狀態結構
typedef struct {
    uint16_t measureItem;        // 檢測項目
    uint16_t stripStatus;        // 試紙狀態
    uint16_t batteryVoltage;     // 電池電壓 (mV)
    uint16_t temperature;        // 溫度 (放大10倍)
} device_status_t;

// 時間結構
typedef struct {
    uint16_t year;               // 年
    uint8_t  month;              // 月
    uint8_t  day;                // 日
    uint8_t  hour;               // 時
    uint8_t  minute;             // 分
    uint8_t  second;             // 秒
} time_info_t;

// 測量結果結構
typedef struct {
    uint16_t resultStatus;       // 結果狀態
    uint16_t measureValue;       // 測量值
    uint16_t measureItem;        // 測量項目
    uint16_t event;              // 事件類型
    uint8_t  code;               // 試紙校碼
    time_info_t time;            // 測量時間
    uint16_t batteryVoltage;     // 電池電壓
    uint16_t temperature;        // 溫度
} measure_result_t;

// 函數聲明
void protocol_init(void);
void protocol_process_uart_data(uint8_t *data, uint16_t len);
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
uint8_t protocol_send_error(uint8_t originalCmdId, uint8_t errorCode);
uint8_t protocol_calculate_checksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
void protocol_handle_ble_data(uint8_t *data, uint16_t len);

#endif /* __PROTOCOL_HANDLER_H */ 