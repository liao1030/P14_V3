/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_utils.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數工具函數頭文件
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_UTILS_H
#define __PARAM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "param_table.h"
#include "param_code_table.h"

/* 測量單位枚舉 */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL = 1,       // mg/dL
    UNIT_GM_DL = 2,       // gm/dL
    UNIT_MAX
} Unit_TypeDef;

/* 事件類型枚舉 */
typedef enum {
    EVENT_NONE = 0,               // 無事件
    EVENT_BEFORE_MEAL = 1,        // 飯前
    EVENT_AFTER_MEAL = 2,         // 飯後
    EVENT_FASTING = 3,            // 空腹
    EVENT_BEDTIME = 4,            // 睡前
    EVENT_AC = 5,                 // AC
    EVENT_PC = 6,                 // PC
    EVENT_QC = 7,                 // QC (品管液)
    EVENT_MAX
} Event_TypeDef;

/* 輔助函數原型 */
uint8_t PARAM_GetByte(uint16_t paramAddress);
uint16_t PARAM_GetWord(uint16_t paramAddress);
float PARAM_GetFloat(uint16_t paramAddress);
ParamError_TypeDef PARAM_SetByte(uint16_t paramAddress, uint8_t value);
ParamError_TypeDef PARAM_SetWord(uint16_t paramAddress, uint16_t value);
ParamError_TypeDef PARAM_SetFloat(uint16_t paramAddress, float value);
ParamError_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date,
                                    uint8_t *hour, uint8_t *minute, uint8_t *second);
ParamError_TypeDef PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date,
                                    uint8_t hour, uint8_t minute, uint8_t second);
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);
const char* Event_GetName(Event_TypeDef event);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_UTILS_H */
