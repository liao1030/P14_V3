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

/*********************************************************************
 * CONSTANTS
 */
// 任務事件定義
#define STRIP_DETECT_EVT                 0x0020

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

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* STRIP_DETECT_H */
