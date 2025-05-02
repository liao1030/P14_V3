/**************************************************************************************************
 * 檔案名稱: strip_detection.h
 * 功能說明: 試片插入偵測相關定義和函數聲明
 **************************************************************************************************/

#ifndef __STRIP_DETECTION_H
#define __STRIP_DETECTION_H

#include "CH58x_common.h"

// 試片類型定義
typedef enum {
    STRIP_TYPE_UNKNOWN = 0,  // 未知類型
    STRIP_TYPE_GLV,          // 血糖(GLV試片)
    STRIP_TYPE_U,            // 尿酸
    STRIP_TYPE_C,            // 總膽固醇
    STRIP_TYPE_TG,           // 三酸甘油脂
    STRIP_TYPE_GAV           // 血糖(GAV試片)
} StripType_t;

// 試片插入狀態定義
typedef enum {
    STRIP_STATUS_NONE = 0,   // 無試片
    STRIP_STATUS_INSERTED,   // 已插入
    STRIP_STATUS_READY,      // 準備就緒
    STRIP_STATUS_TESTING,    // 測試中
    STRIP_STATUS_COMPLETE    // 測試完成
} StripStatus_t;

// 初始化試片插入偵測
void Strip_Detection_Init(void);

// 檢查試片插入狀態(處理除彈跳)
void Strip_CheckInsertionStatus(void);

// 獲取試片狀態
StripStatus_t Strip_GetStatus(void);

// 獲取試片類型
StripType_t Strip_GetType(void);

// 設置試片類型 (由CH32V203通過UART通知)
void Strip_SetType(StripType_t type);

// 測試完成處理函數
void Strip_TestComplete(void);

#endif /* __STRIP_DETECTION_H */