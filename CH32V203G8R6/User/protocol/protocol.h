/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀藍牙與UART通訊協議標頭檔
 ********************************************************************************/

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* 協議相關常數定義 */
#define PROTOCOL_START_MARKER     0xAA
#define PROTOCOL_END_MARKER       0x55

/* 指令ID定義 */
/* APP→Meter方向指令 */
#define CMD_SYNC_TIME             0x01    // 同步時間
#define CMD_GET_DEVICE_STATUS     0x02    // 請求裝置狀態
#define CMD_SET_CODE_EVENT        0x03    // 設定CODE和EVENT
#define CMD_BLOOD_STATUS_REQ      0x04    // 檢測狀態請求
#define CMD_REQUEST_RESULT        0x05    // 請求測量結果
#define CMD_REQUEST_RAW_DATA      0x06    // 請求RAW DATA

/* Meter→APP方向指令 */
#define CMD_SYNC_TIME_ACK         0x81    // 時間同步確認
#define CMD_DEVICE_STATUS_ACK     0x82    // 裝置狀態回應
#define CMD_CODE_EVENT_ACK        0x83    // CODE/EVENT設定確認
#define CMD_BLOOD_STATUS_NOTIFY   0x84    // 血液檢測通知
#define CMD_RESULT_RESPONSE       0x85    // 測量結果回應
#define CMD_RAW_DATA_RESPONSE     0x86    // RAW DATA回應
#define CMD_ERROR_RESPONSE        0xFF    // 錯誤回應

/* 錯誤代碼 */
#define ERR_BATTERY_LOW           0x01    // 電池電量過低
#define ERR_TEMP_HIGH             0x02    // 溫度過高
#define ERR_TEMP_LOW              0x03    // 溫度過低
#define ERR_STRIP_EXPIRED         0x04    // 試片過期或損壞
#define ERR_STRIP_USED            0x05    // 試片已使用
#define ERR_STRIP_INSERT_ERROR    0x06    // 試片插入錯誤
#define ERR_BLOOD_SAMPLE_SHORT    0x07    // 血液樣本不足
#define ERR_MEASURE_TIMEOUT       0x08    // 測量超時
#define ERR_CAL_ERROR             0x09    // 校驗錯誤
#define ERR_HARDWARE_ERROR        0x0A    // 硬體錯誤
#define ERR_COMM_ERROR            0x0B    // 通訊錯誤
#define ERR_DATA_FORMAT           0x0C    // 資料格式錯誤
#define ERR_CHECKSUM              0x0D    // 校驗和錯誤
#define ERR_CMD_NOT_SUPPORTED     0x0E    // 指令不支援
#define ERR_RESULT_OUT_OF_RANGE   0x0F    // 測量結果超出範圍

/* 測量項目定義 */
#define ITEM_GLV                  0x0000  // 血糖
#define ITEM_U                    0x0001  // 尿酸
#define ITEM_C                    0x0002  // 總膽固醇
#define ITEM_TG                   0x0003  // 三酸甘油脂
#define ITEM_GAV                  0x0004  // 血糖

/* EVENT定義 */
#define EVENT_NONE                0x0000  // 無
#define EVENT_AC                  0x0001  // 飯前
#define EVENT_PC                  0x0002  // 飯後
#define EVENT_QC                  0x0003  // QC

/* 封包結構 */
typedef struct {
    uint8_t cmd;             // 指令ID
    uint8_t length;          // 資料長度
    uint8_t *data;           // 資料指標
    uint8_t checksum;        // 校驗和
} Protocol_Packet_t;

/* 設備狀態結構 */
typedef struct {
    uint16_t measure_item;    // 測量項目
    uint16_t strip_status;    // 試片狀態
    uint16_t battery_voltage; // 電池電壓 (mV)
    uint16_t temperature;     // 溫度 (放大10倍，如25.7度為257)
} Device_Status_t;

/* 測量結果結構 */
typedef struct {
    uint16_t result_status;  // 結果狀態 (0: 成功, 非0: 錯誤代碼)
    uint16_t measure_value;  // 測量結果值
    uint16_t measure_item;   // 測量項目
    uint16_t event;          // 事件類型
    uint8_t  code;           // 試片校正碼
    uint16_t year;           // 年
    uint16_t month;          // 月
    uint16_t day;            // 日
    uint16_t hour;           // 時
    uint16_t minute;         // 分
    uint16_t second;         // 秒
    uint16_t battery_voltage;// 電池電壓 (mV)
    uint16_t temperature;    // 溫度 (放大10倍)
} Measure_Result_t;

/* 時間結構 */
typedef struct {
    uint16_t year;           // 年
    uint8_t  month;          // 月
    uint8_t  day;            // 日
    uint8_t  hour;           // 時
    uint8_t  minute;         // 分
    uint8_t  second;         // 秒
} Time_t;

/* 協議功能API */
uint8_t Protocol_Init(void);
uint8_t Protocol_ProcessReceivedData(uint8_t *data, uint16_t length);
uint8_t Protocol_SendPacket(uint8_t cmd, uint8_t *data, uint8_t length);
uint8_t Protocol_SendErrorResponse(uint8_t original_cmd, uint8_t error_code);
uint8_t Protocol_SendDeviceStatus(Device_Status_t *status);
uint8_t Protocol_SendMeasureResult(Measure_Result_t *result);
uint8_t Protocol_SendSyncTimeAck(uint8_t status);
uint8_t Protocol_SendCodeEventAck(uint8_t status);
uint8_t Protocol_SendBloodStatusNotify(uint8_t countdown);
uint8_t Protocol_SendRawData(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */