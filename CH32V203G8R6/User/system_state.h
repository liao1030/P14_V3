/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_state.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/22
 * Description        : 系統狀態機相關定義
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* 系統狀態定義 */
typedef enum {
    STATE_IDLE,                // 空閒狀態
    STATE_STRIP_DETECTED,      // 檢測到試片
    STATE_STRIP_VALIDATION,    // 試片驗證階段
    STATE_PARAMETER_SETUP,     // 參數設置階段
    STATE_WAIT_FOR_BLOOD,      // 等待血液滴入
    STATE_MEASURING,           // 測量中
    STATE_RESULT_READY,        // 結果準備好
    STATE_ERROR                // 錯誤狀態
} SystemState_TypeDef;

/* 函數聲明 */
void System_SetState(SystemState_TypeDef newState);
SystemState_TypeDef System_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_STATE_H */
