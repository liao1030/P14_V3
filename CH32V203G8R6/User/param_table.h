/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/08
 * Description        : 多功能生化測試儀參數代碼表
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

/* 參數表大小定義 */
#define PARAM_TABLE_SIZE      678    // 總參數大小（0~677）

/* 系統基本參數地址定義 */
#define PARAM_LBT             0      // 低電池閾值
#define PARAM_OBT             1      // 電池耗盡閾值
#define PARAM_FACTORY         2      // 儀器操作模式
#define PARAM_MODEL_NO        3      // 產品型號
#define PARAM_FW_NO           4      // 韌體版本號
#define PARAM_NOT             5      // 測試次數 (2 bytes)
#define PARAM_CODE_TABLE_V    7      // 代碼表版本號 (2 bytes)

/* 時間設定參數地址定義 */
#define PARAM_YEAR            9      // 年份設定
#define PARAM_MONTH           10     // 月份設定
#define PARAM_DATE            11     // 日期設定
#define PARAM_HOUR            12     // 小時設定
#define PARAM_MINUTE          13     // 分鐘設定
#define PARAM_SECOND          14     // 秒數設定

/* 測試條件參數地址定義 */
#define PARAM_TLL             15     // 操作溫度範圍下限
#define PARAM_TLH             16     // 操作溫度範圍上限
#define PARAM_MGDL            17     // 濃度單位設定
#define PARAM_EVENT           18     // 量測事件類型
#define PARAM_STRIP_TYPE      19     // 測試項目

/* 硬體校準參數地址定義 */
#define PARAM_EV_T3_TRG       20     // EV_T3觸發電壓 (2 bytes)
#define PARAM_EV_WORKING      22     // 濃度工作電壓電壓
#define PARAM_EV_T3           23     // 血液測濃電極電壓
#define PARAM_DACO            24     // DAC偏移補償
#define PARAM_DACDO           25     // DAC校正管理參數
#define PARAM_CC211NODONE     26     // CC211未完成旗標
#define PARAM_CAL_TOL         27     // OPS/OPI校準容差
#define PARAM_OPS             28     // OPA校準斜率 (4 bytes)
#define PARAM_OPI             32     // OPA校準截距 (4 bytes)
#define PARAM_QCT             36     // QCT校準測試低位元組
#define PARAM_TOFFSET         37     // 溫度校準偏移
#define PARAM_BOFFSET         38     // 電池校準偏移

/* 血糖(GLV/GAV)專用參數地址定義 */
#define PARAM_BG_CSU_TOL      39     // 試片檢查容差
#define PARAM_BG_NDL          40     // 新試片測濃水平 (2 bytes)
#define PARAM_BG_UDL          42     // 已使用試片測濃水平 (2 bytes)
#define PARAM_BG_BLOOD_IN     44     // 血液測濃水平 (2 bytes)
#define PARAM_BG_STRIP_LOT    46     // 試片批號(16字元) (16 bytes)
#define PARAM_BG_L            62     // 血糖值下限
#define PARAM_BG_H            63     // 血糖值上限
#define PARAM_BG_T3_E37       64     // T3 ADV超E37閾值 (2 bytes)
/* ... 更多血糖參數定義 ... */

/* 尿酸(U)專用參數地址定義 */
#define PARAM_U_CSU_TOL       213    // 試片檢查容差
#define PARAM_U_NDL           214    // 新試片測濃水平 (2 bytes)
#define PARAM_U_UDL           216    // 已使用試片測濃水平 (2 bytes)
#define PARAM_U_BLOOD_IN      218    // 血液測濃水平 (2 bytes)
#define PARAM_U_STRIP_LOT     220    // 試片批號(16字元) (16 bytes)
#define PARAM_U_L             236    // 尿酸值下限
#define PARAM_U_H             237    // 尿酸值上限
#define PARAM_U_T3_E37        238    // T3 ADV超E37閾值 (2 bytes)
/* ... 更多尿酸參數定義 ... */

/* 總膽固醇(C)專用參數地址定義 */
#define PARAM_C_CSU_TOL       355    // 試片檢查容差
#define PARAM_C_NDL           356    // 新試片測濃水平 (2 bytes)
#define PARAM_C_UDL           358    // 已使用試片測濃水平 (2 bytes)
#define PARAM_C_BLOOD_IN      360    // 血液測濃水平 (2 bytes)
#define PARAM_C_STRIP_LOT     362    // 試片批號(16字元) (16 bytes)
#define PARAM_C_L             378    // 總膽固醇值下限
#define PARAM_C_H             379    // 總膽固醇值上限
#define PARAM_C_T3_E37        380    // T3 ADV超E37閾值 (2 bytes)
/* ... 更多總膽固醇參數定義 ... */

/* 三酸甘油脂(TG)專用參數地址定義 */
#define PARAM_TG_CSU_TOL      497    // 試片檢查容差
#define PARAM_TG_NDL          498    // 新試片測濃水平 (2 bytes)
#define PARAM_TG_UDL          500    // 已使用試片測濃水平 (2 bytes)
#define PARAM_TG_BLOOD_IN     502    // 血液測濃水平 (2 bytes)
#define PARAM_TG_STRIP_LOT    504    // 試片批號(16字元) (16 bytes)
#define PARAM_TG_L            520    // 三酸甘油脂值下限
#define PARAM_TG_H            521    // 三酸甘油脂值上限
#define PARAM_TG_T3_E37       522    // T3 ADV超E37閾值 (2 bytes)
/* ... 更多三酸甘油脂參數定義 ... */

/* 保留與校驗區 */
#define PARAM_RESERVED        639    // 保留區域供未來擴展 (36 bytes)
#define PARAM_SUM_L           675    // 地址0~674校驗和(低)
#define PARAM_SUM_H           676    // 地址0~674校驗和(高)
#define PARAM_CRC16           677    // 完整參數表CRC校驗

/* 試片類型定義 */
typedef enum {
    STRIP_TYPE_GLV = 0,   // 血糖
    STRIP_TYPE_U = 1,     // 尿酸
    STRIP_TYPE_C = 2,     // 總膽固醇
    STRIP_TYPE_TG = 3,    // 三酸甘油脂
    STRIP_TYPE_GAV = 4,   // 血糖(另一種)
    STRIP_TYPE_MAX = 5
} StripType_TypeDef;

/* 濃度單位定義 */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL = 1,       // mg/dL
    UNIT_GM_DL = 2,       // gm/dl
    UNIT_MAX = 3
} Unit_TypeDef;

/* 事件類型定義 */
typedef enum {
    EVENT_QC = 0,         // 品管液
    EVENT_AC = 1,         // 飯前 
    EVENT_PC = 2,         // 飯後
    EVENT_MAX = 3
} Event_TypeDef;

/* 參數讀寫函數聲明 */
void PARAM_Init(void);
uint8_t PARAM_GetByte(uint16_t addr);
void PARAM_SetByte(uint16_t addr, uint8_t value);
uint16_t PARAM_GetWord(uint16_t addr);
void PARAM_SetWord(uint16_t addr, uint16_t value);
float PARAM_GetFloat(uint16_t addr);
void PARAM_SetFloat(uint16_t addr, float value);
void PARAM_GetString(uint16_t addr, uint8_t *buffer, uint8_t length);
void PARAM_SetString(uint16_t addr, uint8_t *buffer, uint8_t length);
uint8_t PARAM_VerifyChecksum(void);
void PARAM_UpdateChecksum(void);
void PARAM_SaveToFlash(void);
void PARAM_LoadFromFlash(void);
void PARAM_SetDefault(void);

/* 工具函數聲明 */
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);

#endif /* __PARAM_TABLE_H */