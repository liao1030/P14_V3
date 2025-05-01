/**
 * @file P14_BLE_Protocol.h
 * @brief 多功能生化檢測儀UART通訊協議定義
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>
#include "P14_Parameter_Table.h" // 包含參數表頭文件，取得StripType_TypeDef定義

/* 協議起止標記 */
#define PROTOCOL_START_MARKER      0xAA
#define PROTOCOL_END_MARKER        0x55

/* 緩衝長度 */
#define MAX_PACKET_SIZE            64
#define MAX_DATA_SIZE              60

/* 命令ID定義 */
/* APP 到 Meter命令 */
#define CMD_SYNC_TIME              0x01  // 同步時間
#define CMD_GET_DEVICE_STATUS      0x02  // 請求設備狀態
#define CMD_SET_CODE_EVENT         0x03  // 設置CODE和EVENT參數
#define CMD_BLOOD_SAMPLE_STATUS    0x04  // 檢測血液狀態
#define CMD_GET_TEST_RESULT        0x05  // 請求測試結果
#define CMD_GET_RAW_DATA           0x06  // 請求RAW DATA

/* Meter 到 APP回應 */
#define CMD_SYNC_TIME_ACK          0x81  // 時間同步確認
#define CMD_DEVICE_STATUS_ACK      0x82  // 設備狀態回應
#define CMD_CODE_EVENT_ACK         0x83  // CODE/EVENT設置確認
#define CMD_BLOOD_SAMPLE_ACK       0x84  // 血液檢測通知
#define CMD_TEST_RESULT_ACK        0x85  // 測試結果回應
#define CMD_RAW_DATA_ACK           0x86  // RAW DATA回應
#define CMD_ERROR_ACK              0xFF  // 錯誤回應

/* 設備狀態結構定義 */
typedef struct {
    uint16_t testType;      // 測試項目: 0x0000: GLV(血糖), 0x0001: U(尿酸), 0x0002: C(總膽固醇), 0x0003: TG(三酸甘油脂), 0x0004: GAV(血糖)
    uint16_t stripStatus;   // 試片狀態: 0x0000: 正常, 非0: 錯誤代碼
    uint16_t batteryVoltage;// 電池電壓 (mV), 高位在前 (如 0x0B01 表示 2817mV = 2.817V)
    uint16_t temperature;   // 環境溫度 (放大10倍), 高位在前 (如 0x0119 表示 281 = 28.1℃)
} DeviceStatus_TypeDef;

/* 測試結果結構定義 */
typedef struct {
    uint16_t resultStatus;  // 結果狀態: 0x0000: 成功, 非0: 錯誤代碼
    uint16_t testValue;     // 測試結果值
    uint16_t testType;      // 測試項目類型
    uint16_t event;         // 事件類型: 0x0000: 無, 0x0001: AC(餐前), 0x0002: PC(餐後), 0x0003: QC
    uint8_t  code;          // 試片校正碼
    uint16_t year;          // 年份: 2000+數值 (如 0x0019 表示 2025年)
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
#define ERR_STRIP_EXPIRED          0x04  // 試片過期或劣化
#define ERR_STRIP_USED             0x05  // 試片已使用
#define ERR_STRIP_INSERT           0x06  // 試片插入錯誤
#define ERR_BLOOD_INSUFFICIENT     0x07  // 血液樣本不足
#define ERR_TEST_TIMEOUT           0x08  // 測量超時
#define ERR_CALIBRATION            0x09  // 校準錯誤
#define ERR_HARDWARE               0x0A  // 硬體錯誤
#define ERR_COMMUNICATION          0x0B  // 通訊錯誤
#define ERR_DATA_FORMAT            0x0C  // 數據格式錯誤
#define ERR_CHECKSUM               0x0D  // 校驗錯誤
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
    uint8_t start;          // 開始標記 0xAA
    uint8_t command;        // 指令ID
    uint8_t length;         // 數據長度
    uint8_t data[MAX_DATA_SIZE]; // 數據段
    uint8_t checksum;       // 校驗
    uint8_t end;            // 結束標記 0x55
} BLEPacket_TypeDef;

/* 功能函數 */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_SendPacket(BLEPacket_TypeDef *packet);
void BLE_ProtocolInit(void);

/* CH582F-CH32V203通訊定義 */
#define CMD_START_MARKER        0xAA    // 命令開始標記
#define CMD_END_MARKER          0xBB    // 命令結束標記

/* 命令類型 */
#define CMD_MEASURE_T1_OUT      0x01    // 測量T1_OUT電壓
#define CMD_STRIP_INSERTED      0x02    // 試片插入通知
#define CMD_STRIP_TYPE_RESULT   0x03    // 試片類型判斷結果
#define CMD_START_MEASUREMENT   0x04    // 開始測量

/* T1電極測量結果定義 */
#define T1_OUT_NEAR_0V          0       // T1_OUT接近0V
#define T1_OUT_NEAR_2_5V        1       // T1_OUT接近2.5V

/* 命令接收狀態機 */
typedef enum {
    PROTOCOL_STATE_IDLE,            // 等待命令開始
    PROTOCOL_STATE_CMD_TYPE,        // 接收命令類型
    PROTOCOL_STATE_DATA,            // 接收數據
    PROTOCOL_STATE_END              // 等待命令結束
} ProtocolState_TypeDef;

/* 協議處理函數 */
void BLE_ProtocolInit(void);
void BLE_ProtocolRxHandler(uint8_t rx_data);
void BLE_SendADCValue(uint16_t adc_value);
void BLE_NotifyStripType(StripType_TypeDef type);
void BLE_ProcessCommand(void);

/* 試片偵測與判別相關函數 */
void BLE_MeasureT1Out(void);
void BLE_ProcessStripInsertion(StripType_TypeDef type);
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v);

/* 函數聲明 */
void BLE_ProtocolInit(void);
void BLE_ProtocolRxHandler(uint8_t rx_data);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_MeasureT1Out(void);
void BLE_SendADCValue(uint16_t adc_value);
void BLE_ProcessStripInsertion(StripType_TypeDef type);
void BLE_NotifyStripType(StripType_TypeDef type);
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v);
float Get_Chip_Temperature(void);

#endif /* __P14_BLE_PROTOCOL_H */ 