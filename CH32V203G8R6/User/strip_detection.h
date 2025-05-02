/**
 * @file strip_detection.h
 * @brief 試片類型判別相關定義和函數聲明
 */

#ifndef __STRIP_DETECTION_H
#define __STRIP_DETECTION_H

#include "ch32v20x.h"

// 試片類型定義
typedef enum {
    STRIP_TYPE_DET_UNKNOWN = 0,  // 未知類型
    STRIP_TYPE_DET_GLV,          // 血糖(GLV試片)
    STRIP_TYPE_DET_U,            // 尿酸
    STRIP_TYPE_DET_C,            // 總膽固醇
    STRIP_TYPE_DET_TG,           // 三酸甘油脂
    STRIP_TYPE_DET_GAV           // 血糖(GAV試片)
} StripType_t;

// 試片插入狀態定義
typedef enum {
    STRIP_STATUS_NONE = 0,   // 無試片
    STRIP_STATUS_INSERTED,   // 已插入
    STRIP_STATUS_IDENTIFIED, // 已識別類型
    STRIP_STATUS_TESTING,    // 測試中
    STRIP_STATUS_COMPLETE    // 測試完成
} StripStatus_t;

// 通訊命令定義
#define CMD_WAKEUP_MCU              0xA1    // 喚醒CH32V203的命令
#define CMD_STRIP_INSERTED          0xA2    // 通知試片已插入的命令
#define CMD_REQUEST_STRIP_TYPE      0xA3    // 請求試片類型判別的命令
#define CMD_STRIP_TYPE_RESULT       0xB1    // 試片類型判別結果的命令

// 初始化試片類型判別功能
void Strip_Detection_Init(void);

// 開始試片類型判別
void Strip_StartTypeDetection(void);

// 獲取試片狀態
StripStatus_t Strip_GetStatus(void);

// 獲取試片類型
StripType_t Strip_GetType(void);

// UART接收處理函數
void Strip_UART_Process(uint8_t receivedByte);

#endif /* __STRIP_DETECTION_H */