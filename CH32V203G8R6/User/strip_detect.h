/********************************** (C) COPYRIGHT *******************************
 * File Name          : strip_detect.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : 試片插入偵測與自動判別試片類型相關功能
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef __STRIP_DETECT_H
#define __STRIP_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"
#include "ch32v20x.h"
#include "param_table.h"
#include "ch32v20x_tim.h"  // 添加TIM函數聲明
#include "system_state.h"   // 系統狀態定義

/* 試片狀態定義 */
typedef enum {
    STRIP_STATE_NONE = 0,      // 未插入
    STRIP_STATE_INSERTED = 1,  // 已插入，待判別
    STRIP_STATE_DETECTED = 2,  // 已判別類型
    STRIP_STATE_ERROR = 3      // 錯誤狀態
} StripState_TypeDef;

/* SystemState_TypeDef 定義在 system_state.h 中 */

/* 試片插入狀態結構 */
typedef struct {
    StripState_TypeDef state;         // 試片狀態
    StripType_TypeDef type;           // 試片類型
    uint8_t pin3Status;               // 第3腳狀態
    uint8_t pin5Status;               // 第5腳狀態
    float t1Voltage;                  // T1_OUT電壓
    uint32_t insertTime;              // 插入時間戳
    uint8_t detectionComplete;        // 偵測完成標誌
    uint8_t ackSent;                  // 回應已發送
} StripDetectInfo_TypeDef;

/* 函數宣告 */
void STRIP_DETECT_Init(void);
void STRIP_DETECT_Process(void);
uint8_t STRIP_DETECT_IsInserted(void);
StripType_TypeDef STRIP_DETECT_GetStripType(void);
void STRIP_DETECT_HandleInsertedEvent(void);
void STRIP_DETECT_SetStripType(StripType_TypeDef type);
void STRIP_DETECT_SetPinStatus(uint8_t pin3, uint8_t pin5);

/* 外部函數聲明 */
extern void Delay_Ms(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* __STRIP_DETECT_H */
