/**
 * @file P14_BLE_Protocol.h
 * @brief 多功能生化檢測儀CH582F藍牙通訊協議定義
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>
#include "ble_uart_service.h"  // 添加引用以取得 ble_uart_evt_t 類型

/* 協議包頭尾標記 */
#define PROTOCOL_START_MARKER      0xAA
#define PROTOCOL_END_MARKER        0x55

/* 封包最大長度 */
#define MAX_PACKET_SIZE            64
#define MAX_DATA_SIZE              60

/* 命令ID定義 */
/* APP → Meter命令 */
#define CMD_SYNC_TIME              0x01  // 同步時間
#define CMD_GET_DEVICE_STATUS      0x02  // 請求裝置狀態
#define CMD_SET_CODE_EVENT         0x03  // 設定CODE和EVENT參數
#define CMD_BLOOD_SAMPLE_STATUS    0x04  // 檢測血液狀態
#define CMD_GET_TEST_RESULT        0x05  // 請求測試結果
#define CMD_GET_RAW_DATA           0x06  // 請求RAW DATA

/* Meter → APP回應 */
#define CMD_SYNC_TIME_ACK          0x81  // 時間同步確認
#define CMD_DEVICE_STATUS_ACK      0x82  // 裝置狀態回應
#define CMD_CODE_EVENT_ACK         0x83  // CODE/EVENT設定確認
#define CMD_BLOOD_SAMPLE_ACK       0x84  // 血液檢測通知
#define CMD_TEST_RESULT_ACK        0x85  // 測試結果回應
#define CMD_RAW_DATA_ACK           0x86  // RAW DATA回應
#define CMD_ERROR_ACK              0xFF  // 錯誤回應

/* 錯誤代碼定義 */
#define ERR_BATTERY_LOW            0x01  // 電池電量過低
#define ERR_TEMP_TOO_HIGH          0x02  // 溫度過高
#define ERR_TEMP_TOO_LOW           0x03  // 溫度過低
#define ERR_STRIP_EXPIRED          0x04  // 試紙過期或損壞
#define ERR_STRIP_USED             0x05  // 試紙已使用
#define ERR_STRIP_INSERT           0x06  // 試紙插入錯誤
#define ERR_BLOOD_INSUFFICIENT     0x07  // 血液樣本不足
#define ERR_TEST_TIMEOUT           0x08  // 測量超時
#define ERR_CALIBRATION            0x09  // 校正錯誤
#define ERR_HARDWARE               0x0A  // 硬體錯誤
#define ERR_COMMUNICATION          0x0B  // 通訊錯誤
#define ERR_DATA_FORMAT            0x0C  // 資料格式錯誤
#define ERR_CHECKSUM               0x0D  // 校驗和錯誤
#define ERR_UNSUPPORTED_CMD        0x0E  // 指令不支援
#define ERR_RESULT_OUT_OF_RANGE    0x0F  // 測量結果超出範圍

/* 協議封包結構 */
typedef struct {
    uint8_t start;          // 起始標記 0xAA
    uint8_t command;        // 指令ID
    uint8_t length;         // 資料長度
    uint8_t data[MAX_DATA_SIZE]; // 資料區
    uint8_t checksum;       // 校驗和
    uint8_t end;            // 結束標記 0x55
} BLEPacket_TypeDef;

/* BLE設備信息 */
#define BLE_DEVICE_NAME        "P14-"       // 設備名前綴，後加MAC地址後6位
#define BLE_SERVICE_UUID       0xFFE0       // 服務UUID
#define BLE_CHAR_UUID_RX       0xFFE1       // 特徵UUID (接收通知)
#define BLE_CHAR_UUID_TX       0xFFE2       // 特徵UUID (發送通知)

/* 函數宣告 */
/**
 * @brief 初始化藍牙協議
 */
void P14_BLE_ProtocolInit(void);

/**
 * @brief 處理來自APP的命令
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_ProcessAppCommand(uint8_t *data, uint16_t length);

/**
 * @brief 處理來自MCU的數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_ProcessMcuData(uint8_t *data, uint16_t length);

/**
 * @brief 向APP發送數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_SendToApp(uint8_t *data, uint16_t length);

/**
 * @brief 向MCU發送數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_SendToMCU(uint8_t *data, uint16_t length);

/**
 * @brief 藍牙UART服務回調處理
 * 
 * @param connection_handle 連接句柄
 * @param p_evt 事件指針
 */
void P14_BLE_UartCallback(uint16_t connection_handle, ble_uart_evt_t *p_evt);

/* 協議處理函數 */
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);

#endif /* __P14_BLE_PROTOCOL_H */ 