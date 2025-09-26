/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : 試片插入偵測模組頭文件
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef STRIP_DETECT_H
#define STRIP_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include <stdint.h>

// 定義 bool 類型（如果編譯器不支援）
#ifndef bool
typedef uint8_t bool;
#define true  1
#define false 0
#endif

/*********************************************************************
 * CONSTANTS
 */
// 任務事件定義
#define STRIP_PERIODIC_CHECK_EVT         0x0040
#define STRIP_SEND_MESSAGE_EVT           0x0080
#define STRIP_ENTER_SLEEP_EVT            0x0100

// 試片類型定義
#define STRIP_TYPE_GLV                   0x00
#define STRIP_TYPE_U                     0x01
#define STRIP_TYPE_C                     0x02
#define STRIP_TYPE_TG                    0x03
#define STRIP_TYPE_GAV                   0x04
#define STRIP_TYPE_UNKNOWN               0xFF

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * 初始化試片偵測模組
 */
extern void StripDetect_Init(tmosTaskID task_id);

/*
 * 試片偵測事件處理
 */
extern uint16_t StripDetect_ProcessEvent(tmosTaskID task_id, uint16_t events);

/*
 * 設定試片類型 (由MCU反饋試片類型時調用)
 */
extern void StripDetect_SetStripType(uint8_t type);

/*
 * 檢查試片是否插入
 */
extern bool StripDetect_IsStripInserted(void);

/*
 * 獲取試片類型
 */
extern uint8_t StripDetect_GetStripType(void);

/*
 * 進入休眠模式
 */
extern void StripDetect_EnterSleepMode(void);

/*
 * 配置GPIO中斷用於休眠喚醒
 */
extern void StripDetect_ConfigureWakeupInterrupt(void);

/*
 * 從休眠模式喚醒後的處理
 */
extern void StripDetect_WakeupFromSleep(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* STRIP_DETECT_H */
