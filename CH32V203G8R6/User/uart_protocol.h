/* 通訊協議標記 */
#define PROTOCOL_START_MARK   0xAA    // 起始標記
#define PROTOCOL_END_MARK     0x55    // 結束標記

/* 指令碼定義 */
#define CMD_SYNC_TIME         0x01    // 同步時間
#define CMD_SYNC_TIME_ACK     0x81    // 時間同步確認
#define CMD_GET_STATUS        0x02    // 請求裝置狀態
#define CMD_STATUS_ACK        0x82    // 裝置狀態回應
#define CMD_SET_PARAM         0x03    // 設定CODE和EVENT
#define CMD_SET_PARAM_ACK     0x83    // 設定CODE和EVENT回應
#define CMD_CHECK_BLOOD       0x04    // 檢測血液
#define CMD_BLOOD_NOTIFY      0x84    // 血液檢測通知
#define CMD_GET_RESULT        0x05    // 請求測試結果
#define CMD_RESULT_ACK        0x85    // 測試結果回應
#define CMD_GET_RAW_DATA      0x06    // 請求RAW DATA
#define CMD_RAW_DATA_ACK      0x86    // RAW DATA回應
#define CMD_ERROR_ACK         0x8F    // 錯誤回應

/* 試片相關指令 */
#define CMD_STRIP_INSERTED    0x20    // 試片插入通知
#define CMD_STRIP_TYPE_ACK    0xA0    // 試片類型回應

//-------------------------------------------------
//  * File Name          : uart_protocol.h
//  * Author             : HMD Team
//  * Version            : V1.0.0
//  * Date               : 2025/05/12
//  * Description        : 多功能生化測試儀UART通訊協議
//-------------------------------------------------

#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#include "ch32v20x.h"
#include "param_table.h"

/* 通訊協議標記 */
#define PROTOCOL_START_MARK   0xAA    // 起始標記
#define PROTOCOL_END_MARK     0x55    // 結束標記

/* 指令碼定義 */
#define CMD_SYNC_TIME         0x01    // 同步時間
#define CMD_SYNC_TIME_ACK     0x81    // 時間同步確認
#define CMD_GET_STATUS        0x02    // 請求裝置狀態
#define CMD_STATUS_ACK        0x82    // 裝置狀態回應
#define CMD_SET_PARAM         0x03    // 設定CODE和EVENT
#define CMD_SET_PARAM_ACK     0x83    // 設定CODE和EVENT回應
#define CMD_CHECK_BLOOD       0x04    // 檢測血液
#define CMD_BLOOD_NOTIFY      0x84    // 血液檢測通知
#define CMD_GET_RESULT        0x05    // 請求測試結果
#define CMD_RESULT_ACK        0x85    // 測試結果回應
#define CMD_GET_RAW_DATA      0x06    // 請求RAW DATA
#define CMD_RAW_DATA_ACK      0x86    // RAW DATA回應
#define CMD_ERROR_ACK         0x8F    // 錯誤回應

/* 試片相關指令 */
#define CMD_STRIP_INSERTED    0x20    // 試片插入通知
#define CMD_STRIP_TYPE_ACK    0xA0    // 試片類型回應

/* 錯誤代碼定義 */
#define ERR_BATTERY_LOW       0x01    // 電池電量過低
#define ERR_TEMP_HIGH         0x02    // 溫度過高
#define ERR_TEMP_LOW          0x03    // 溫度過低
#define ERR_STRIP_EXPIRED     0x04    // 試片過期或損壞
#define ERR_STRIP_USED        0x05    // 試片已使用
#define ERR_STRIP_ERROR       0x06    // 試片插入錯誤
#define ERR_BLOOD_NOT_ENOUGH  0x07    // 血液樣本不足
#define ERR_TEST_TIMEOUT      0x08    // 測量超時
#define ERR_CAL_ERROR         0x09    // 校正錯誤
#define ERR_HARDWARE_ERROR    0x0A    // 硬體錯誤
#define ERR_COMM_ERROR        0x0B    // 通訊錯誤
#define ERR_DATA_FORMAT       0x0C    // 資料格式錯誤
#define ERR_CHECKSUM_ERROR    0x0D    // 校驗和錯誤
#define ERR_COMMAND_NOTSUPP   0x0E    // 指令不支援
#define ERR_RESULT_OUTRANGE   0x0F    // 測量結果超出範圍

/* 協議封包結構定義 (最大大小) */
#define MAX_PACKET_SIZE       64      // 最大封包大小
#define MAX_DATA_SIZE         60      // 最大資料區大小

/* 測試資料結構 */
typedef struct {
    uint16_t resultStatus;    // 結果狀態 (0=成功, 非0=錯誤代碼)
    uint16_t testValue;       // 測試結果值
    uint16_t stripType;       // 測試項目
    uint16_t eventType;       // 事件類型
    uint16_t stripCode;       // 試片校正碼
    uint16_t year;            // 年份
    uint16_t month;           // 月份
    uint16_t date;            // 日期
    uint16_t hour;            // 小時
    uint16_t minute;          // 分鐘
    uint16_t second;          // 秒數
    uint16_t batteryVoltage;  // 電池電壓 (mV)
    uint16_t temperature;     // 環境溫度 (放大10倍)
} TestResult_TypeDef;

/* 裝置狀態結構 */
typedef struct {
    uint16_t stripType;       // 測試項目
    uint16_t stripStatus;     // 試片狀態 (0=正常, 非0=錯誤代碼)
    uint16_t batteryVoltage;  // 電池電壓 (mV)
    uint16_t temperature;     // 環境溫度 (放大10倍)
} DeviceStatus_TypeDef;

/* RAW DATA結構 */
typedef struct {
    uint16_t resultStatus;    // 結果狀態
    uint16_t testValue;       // 測試值
    uint16_t year;            // 年份
    uint16_t month;           // 月份
    uint16_t date;            // 日期
    uint16_t hour;            // 小時
    uint16_t minute;          // 分鐘
    uint16_t second;          // 秒數
    uint16_t stripType;       // 測試項目
    uint16_t eventType;       // 事件類型
    uint16_t batteryVoltage;  // 電池電壓
    uint16_t temperature;     // 環境溫度
    uint16_t w1Adv;           // W1通道ADC值
    uint16_t w2Adv;           // W2通道ADC值
    uint16_t t1Ampl;          // T1溫度感測振幅
    uint16_t t1AcMax;         // T1 AC最大值
    uint16_t t1AcMin;         // T1 AC最小值
    uint16_t t3Adc;           // T3通道ADC值
    uint16_t testCount;       // 測試次數
    uint16_t stripCode;       // 試片校正碼
    char operatorId[10];      // 操作者ID
    char chartNo[10];         // 圖表編號
} RawData_TypeDef;

/* 公共函數宣告 */
void UART_Protocol_Init(void);
void UART_Protocol_Process(void);

/* 封包處理函數 */
uint8_t UART_ProcessPacket(uint8_t *packet, uint16_t length);

/* 特定指令處理函數 */
uint8_t UART_ProcessSyncTime(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetStatus(uint8_t *data, uint8_t length);
uint8_t UART_ProcessSetParam(uint8_t *data, uint8_t length);
uint8_t UART_ProcessCheckBlood(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetResult(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetRawData(uint8_t *data, uint8_t length);
uint8_t UART_ProcessStripInsertedCmd(uint8_t *data, uint8_t length);

/* 回應封包產生函數 */
uint8_t UART_SendSyncTimeAck(uint8_t status);
uint8_t UART_SendStatusAck(DeviceStatus_TypeDef *status);
uint8_t UART_SendParamAck(uint8_t status);
uint8_t UART_SendBloodNotify(uint8_t countdown);
uint8_t UART_SendResultAck(TestResult_TypeDef *result);
uint8_t UART_SendRawDataAck(RawData_TypeDef *rawData);
uint8_t UART_SendErrorAck(uint8_t cmdId, uint8_t errorCode);
uint8_t UART_SendStripTypeAck(StripType_TypeDef stripType);

/* 輔助功能 */
uint8_t UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t length);
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t length, uint8_t *data);
uint8_t UART_VerifyChecksum(uint8_t *packet, uint16_t length);

/* 接收中斷處理函數 */
void UART2_Receive_Byte_ISR(uint8_t byte);

#endif /* __UART_PROTOCOL_H */
