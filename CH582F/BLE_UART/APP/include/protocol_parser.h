/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_parser.h
 * Description        : 多功能生化儀藍牙通訊協議解析器
 *******************************************************************************/

#ifndef __PROTOCOL_PARSER_H
#define __PROTOCOL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 協議常量
#define PACKET_START_MARKER     0xAA
#define PACKET_END_MARKER       0x55

// 指令ID定義
#define CMD_SYNC_TIME                 0x01
#define CMD_SYNC_TIME_CONFIRM         0x81
#define CMD_REQUEST_DEVICE_STATUS     0x02
#define CMD_DEVICE_STATUS_RESPONSE    0x82
#define CMD_SET_CODE_EVENT            0x03
#define CMD_CODE_EVENT_CONFIRM        0x83
#define CMD_BLOOD_STATUS_REQUEST      0x04
#define CMD_BLOOD_STATUS_NOTIFY       0x84
#define CMD_REQUEST_RESULT            0x05
#define CMD_RESULT_RESPONSE           0x85
#define CMD_REQUEST_RAW_DATA          0x06
#define CMD_RAW_DATA_RESPONSE         0x86
#define CMD_ERROR_RESPONSE            0xFF

// 錯誤代碼
#define ERR_BATTERY_LOW               0x01
#define ERR_TEMP_TOO_HIGH             0x02
#define ERR_TEMP_TOO_LOW              0x03
#define ERR_STRIP_EXPIRED             0x04
#define ERR_STRIP_USED                0x05
#define ERR_STRIP_INSERT_ERROR        0x06
#define ERR_BLOOD_INSUFFICIENT        0x07
#define ERR_TEST_TIMEOUT              0x08
#define ERR_CALIBRATION_ERROR         0x09
#define ERR_HARDWARE_ERROR            0x0A
#define ERR_COMM_ERROR                0x0B
#define ERR_DATA_FORMAT_ERROR         0x0C
#define ERR_CHECKSUM_ERROR            0x0D
#define ERR_COMMAND_NOT_SUPPORTED     0x0E
#define ERR_RESULT_OUT_OF_RANGE       0x0F

// 測試項目
#define TEST_ITEM_GLU                 0x0000
#define TEST_ITEM_U                   0x0001
#define TEST_ITEM_C                   0x0002
#define TEST_ITEM_TG                  0x0003
#define TEST_ITEM_GAV                 0x0004

// 事件類型
#define EVENT_NONE                    0x0000
#define EVENT_AC                      0x0001
#define EVENT_PC                      0x0002
#define EVENT_QC                      0x0003

// 協議封包頭結構
typedef struct {
    uint8_t start;      // 起始標記 0xAA
    uint8_t cmd;        // 指令ID
    uint8_t length;     // 資料長度
    // 資料部分動態長度，不包含在此結構中
    // uint8_t checksum;   // 校驗和
    // uint8_t end;        // 結束標記 0x55
} __attribute__((packed)) PacketHeader_t;

// 協議封包尾結構
typedef struct {
    uint8_t checksum;   // 校驗和
    uint8_t end;        // 結束標記 0x55
} __attribute__((packed)) PacketFooter_t;

// 同步時間指令資料結構
typedef struct {
    uint16_t year;      // 年份
    uint8_t month;      // 月份 (1-12)
    uint8_t day;        // 日期 (1-31)
    uint8_t hour;       // 小時 (0-23)
    uint8_t minute;     // 分鐘 (0-59)
    uint8_t second;     // 秒 (0-59)
} __attribute__((packed)) SyncTimeData_t;

// 設置CODE和EVENT資料結構
typedef struct {
    uint8_t code;       // 試紙校正碼
    uint16_t event;     // 事件類型
} __attribute__((packed)) CodeEventData_t;

// 設備狀態回應資料結構
typedef struct {
    uint16_t testItem;      // 測試項目
    uint16_t stripStatus;   // 試紙狀態
    uint16_t batteryLevel;  // 電池電壓 (mV)
    uint16_t temperature;   // 環境溫度 (x10)
} __attribute__((packed)) DeviceStatusData_t;

// 測試結果資料結構
typedef struct {
    uint16_t resultStatus;  // 結果狀態
    uint16_t testValue;     // 測試結果值
    uint16_t testItem;      // 測試項目
    uint16_t event;         // 事件類型
    uint8_t code;           // 試紙校正碼
    uint16_t year;          // 年份
    uint16_t month;         // 月份
    uint16_t day;           // 日期
    uint16_t hour;          // 小時
    uint16_t minute;        // 分鐘
    uint16_t second;        // 秒
    uint16_t batteryLevel;  // 電池電壓 (mV)
    uint16_t temperature;   // 環境溫度 (x10)
} __attribute__((packed)) TestResultData_t;

// RAW DATA 資料結構
typedef struct {
    uint16_t dataLength;    // RAW DATA總長度
    // 後續為可變長度的RAW DATA
} __attribute__((packed)) RawDataHeader_t;

// 錯誤回應資料結構
typedef struct {
    uint8_t originalCmd;    // 原始指令ID
    uint8_t errorCode;      // 錯誤代碼
} __attribute__((packed)) ErrorResponseData_t;

// 血液狀態通知資料結構
typedef struct {
    uint8_t countdown;      // 倒數秒數
} __attribute__((packed)) BloodStatusData_t;

// 協議解析器結構
typedef struct {
    uint8_t rxBuffer[256];       // 接收緩衝區
    uint16_t rxIndex;            // 接收索引
    uint8_t rxState;             // 接收狀態機
    uint8_t isPacketComplete;    // 封包完成標誌
    uint8_t expectedLength;      // 預期資料長度
    
    // 封包資料欄位
    uint8_t cmdId;               // 指令ID
    uint8_t dataLength;          // 資料長度
    uint8_t checksum;            // 校驗和
    uint8_t calculatedChecksum;  // 計算得到的校驗和
} ProtocolParser_t;

// 函數聲明
void protocol_parser_init(ProtocolParser_t *parser);
void protocol_parser_reset(ProtocolParser_t *parser);
int protocol_parser_parse(ProtocolParser_t *parser, uint8_t byte);
uint8_t calculate_checksum(uint8_t *data, uint16_t length);
int protocol_build_packet(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t *buffer, uint16_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_PARSER_H */ 