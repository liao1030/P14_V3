#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#include "ch32v20x.h"
#include "ch32v20x_usart.h"
#include "param_code_table.h"
#include "flash_param.h"  /* 添加 flash_param.h 以獲取 FLASH_ParamResult_TypeDef 類型定義 */

/* 通訊協議標記與長度定義 */
#define PROTOCOL_START_MARK      0xAA
#define PROTOCOL_END_MARK        0x55
#define PROTOCOL_HEADER_LEN      3     // 起始標記 + 指令ID + 長度
#define PROTOCOL_FOOTER_LEN      2     // 校驗和 + 結束標記
#define PROTOCOL_MAX_DATA_LEN    128   // 資料欄位最大長度
#define PROTOCOL_MAX_PACKET_LEN  (PROTOCOL_HEADER_LEN + PROTOCOL_MAX_DATA_LEN + PROTOCOL_FOOTER_LEN)

/* 通訊指令ID定義 */
/* 接收指令 (從APP到Meter) */
#define CMD_SYNC_TIME            0x01  // 同步時間
#define CMD_REQ_STATUS           0x02  // 請求裝置狀態
#define CMD_SET_CODE_EVENT       0x03  // 設定CODE和EVENT
#define CMD_CHECK_BLOOD          0x04  // 檢測狀態請求
#define CMD_REQ_RESULT           0x05  // 請求測試結果
#define CMD_REQ_RAW_DATA         0x06  // 請求RAW DATA

/* 回應指令 (從Meter到APP) */
#define CMD_SYNC_TIME_ACK        0x81  // 時間同步確認
#define CMD_STATUS_ACK           0x82  // 裝置狀態回應
#define CMD_CODE_EVENT_ACK       0x83  // CODE/EVENT設定確認
#define CMD_BLOOD_DETECTED_ACK   0x84  // 血液檢測通知
#define CMD_RESULT_ACK           0x85  // 測試結果回應
#define CMD_RAW_DATA_ACK         0x86  // RAW DATA回應
#define CMD_ERROR_ACK            0xFF  // 錯誤回應

/* 錯誤代碼定義 */
#define ERR_LOW_BATTERY          0x01  // 電池電量過低
#define ERR_TEMP_HIGH            0x02  // 溫度過高
#define ERR_TEMP_LOW             0x03  // 溫度過低
#define ERR_STRIP_EXPIRED        0x04  // 試片過期或損壞
#define ERR_STRIP_USED           0x05  // 試片已使用
#define ERR_STRIP_ERROR          0x06  // 試片插入錯誤
#define ERR_BLOOD_INSUFFICIENT   0x07  // 血液樣本不足
#define ERR_TIMEOUT              0x08  // 測量超時
#define ERR_CALIBRATION          0x09  // 校準錯誤
#define ERR_HARDWARE             0x0A  // 硬體錯誤
#define ERR_COMMUNICATION        0x0B  // 通訊錯誤
#define ERR_DATA_FORMAT          0x0C  // 資料格式錯誤
#define ERR_CHECKSUM             0x0D  // 校驗和錯誤
#define ERR_COMMAND_UNSUPPORTED  0x0E  // 指令不支援
#define ERR_RESULT_OUT_OF_RANGE  0x0F  // 測量結果超出範圍

/* 事件類型定義 */
#define EVENT_NONE               0x0000  // 無
#define EVENT_AC                 0x0001  // 飯前
#define EVENT_PC                 0x0002  // 飯後
#define EVENT_QC                 0x0003  // QC

/* 測試項目類型定義 */
#define STRIP_TYPE_GLV           0x0000  // 血糖
#define STRIP_TYPE_U             0x0001  // 尿酸
#define STRIP_TYPE_C             0x0002  // 總膽固醇
#define STRIP_TYPE_TG            0x0003  // 三酸甘油脂
#define STRIP_TYPE_GAV           0x0004  // 血糖(另一種)

/* 測試記錄結構定義 */
typedef struct {
    uint16_t EVENT;            // 事件類型
    uint8_t Strip_Code;        // 試片批號碼
} TestRecord_TypeDef;

/* 通訊協議結構體定義 */
typedef struct {
    uint8_t startMark;           // 起始標記 (0xAA)
    uint8_t cmdId;               // 指令ID
    uint8_t dataLen;             // 資料長度
    uint8_t data[PROTOCOL_MAX_DATA_LEN]; // 資料
    uint8_t checksum;            // 校驗和
    uint8_t endMark;             // 結束標記 (0x55)
} UartProtocol_TypeDef;

/* UART 接收狀態機定義 */
typedef enum {
    UART_RX_WAIT_START,          // 等待起始標記
    UART_RX_WAIT_CMD,            // 等待指令ID
    UART_RX_WAIT_LEN,            // 等待資料長度
    UART_RX_WAIT_DATA,           // 等待資料
    UART_RX_WAIT_CHECKSUM,       // 等待校驗和
    UART_RX_WAIT_END             // 等待結束標記
} UartRxState_TypeDef;

/* 功能函數宣告 */
void UART_Protocol_Init(void);
void UART_Protocol_Process(uint8_t rxByte);
void UART_Send_Packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
void UART_Send_Error(uint8_t originalCmd, uint8_t errorCode);
uint8_t UART_Calculate_Checksum(uint8_t cmdId, uint8_t dataLen, uint8_t *data);

/* 指令處理函數宣告 */
void UART_Handle_SyncTime(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqStatus(uint8_t *data, uint8_t dataLen);
void UART_Handle_SetCodeEvent(uint8_t *data, uint8_t dataLen);
void UART_Handle_CheckBlood(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqResult(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqRawData(uint8_t *data, uint8_t dataLen);

/* Flash參數保存函數宣告 */
FLASH_ParamResult_TypeDef PARAM_SaveParameters(void);
FLASH_ParamResult_TypeDef PARAM_SaveTime(void);

#endif /* __UART_PROTOCOL_H */