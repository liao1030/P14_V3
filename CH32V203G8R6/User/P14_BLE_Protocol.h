/**
 * @file P14_BLE_Protocol.h
 * @brief 多功能生化檢測儀藍牙UART通訊協議定義
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>

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

/* 設備狀態結構定義 */
typedef struct {
    uint16_t testType;      // 測試項目: 0x0000: GLV(血糖), 0x0001: U(尿酸), 0x0002: C(總膽固醇), 0x0003: TG(三酸甘油脂), 0x0004: GAV(血糖)
    uint16_t stripStatus;   // 試紙狀態: 0x0000: 正常, 非0: 錯誤代碼
    uint16_t batteryVoltage;// 電池電壓 (mV), 高位在前 (例 0x0B01 表示 2817mV = 2.817V)
    uint16_t temperature;   // 環境溫度 (放大10倍), 高位在前 (例 0x0119 表示 281 = 28.1度)
} DeviceStatus_TypeDef;

/* 測試結果結構定義 */
typedef struct {
    uint16_t resultStatus;  // 結果狀態: 0x0000: 成功, 非0: 錯誤代碼
    uint16_t testValue;     // 測試結果值
    uint16_t testType;      // 測試項目類型
    uint16_t event;         // 事件類型: 0x0000: 無, 0x0001: AC(飯前), 0x0002: PC(飯後), 0x0003: QC
    uint8_t  code;          // 試紙校正碼
    uint16_t year;          // 年份: 2000+年值 (例 0x0019 表示 2025年)
    uint16_t month;         // 月份: 1-12
    uint16_t date;          // 日期: 1-31
    uint16_t hour;          // 小時: 0-23
    uint16_t minute;        // 分鐘: 0-59
    uint16_t second;        // 秒: 0-59
    uint16_t batteryVoltage;// 電池電壓 (mV)
    uint16_t temperature;   // 環境溫度 (放大10倍)
} TestResult_TypeDef;

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

/* RTC時間日期結構體定義 */
typedef struct {
    uint16_t Year;      // 年份 (0-99)
    uint8_t Month;      // 月份 (1-12)
    uint8_t Day;        // 日期 (1-31)
    uint8_t Hour;       // 小時 (0-23)
    uint8_t Min;        // 分鐘 (0-59)
    uint8_t Sec;        // 秒 (0-59)
} RTC_DateTimeTypeDef;

/* 協議封包結構 */
typedef struct {
    uint8_t start;          // 起始標記 0xAA
    uint8_t command;        // 指令ID
    uint8_t length;         // 資料長度
    uint8_t data[MAX_DATA_SIZE]; // 資料區
    uint8_t checksum;       // 校驗和
    uint8_t end;            // 結束標記 0x55
} BLEPacket_TypeDef;

/* 函數宣告 */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_SendPacket(BLEPacket_TypeDef *packet);
void BLE_ProtocolInit(void);

#endif /* __P14_BLE_PROTOCOL_H */ 