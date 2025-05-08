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
#define PARAM_EV_WORKING      22     // 濃度工作電壓
#define PARAM_EV_T3           23     // 血液測濃電極電壓
#define PARAM_DACO            24     // DAC偏移補償
#define PARAM_DACDO           25     // DAC校正管理參數
#define PARAM_CC211NODONE     26     // CC211正常完成
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
#define PARAM_BG_STRIP_LOT    46     // 試片批號 (16 bytes)
#define PARAM_BG_L            62     // 血糖值下限
#define PARAM_BG_H            63     // 血糖值上限
#define PARAM_BG_T3_E37       64     // T3 ADV超E37閾值 (2 bytes)
/* 血糖測試時序參數 */
#define PARAM_BG_TPL1         66     // 測試間格1 (2 bytes)
#define PARAM_BG_TRD1         68     // 讀取延遲1 (2 bytes)
#define PARAM_BG_EVWIDTH1     70     // 工作電壓寬度1 (2 bytes)
#define PARAM_BG_TPL2         72     // 測試間格2 (2 bytes)
#define PARAM_BG_TRD2         74     // 讀取延遲2 (2 bytes)
#define PARAM_BG_EVWIDTH2     76     // 工作電壓寬度2 (2 bytes)
/* 血糖標準品曲線參數 */
#define PARAM_BG_CAL_A1       78     // 標準品曲線係數A1 (4 bytes)
#define PARAM_BG_CAL_B1       82     // 標準品曲線係數B1 (4 bytes)
#define PARAM_BG_CAL_C1       86     // 標準品曲線係數C1 (4 bytes)
/* 血糖溫度補償參數 */
#define PARAM_BG_TEMP_A1      90     // 溫度補償係數A1 (4 bytes)
#define PARAM_BG_TEMP_B1      94     // 溫度補償係數B1 (4 bytes)
#define PARAM_BG_TEMP_C1      98     // 溫度補償係數C1 (4 bytes)
/* 血糖血量補償參數 */
#define PARAM_BG_BLOOD_A1     102    // 血量補償係數A1 (4 bytes)
#define PARAM_BG_BLOOD_B1     106    // 血量補償係數B1 (4 bytes)
#define PARAM_BG_BLOOD_C1     110    // 血量補償係數C1 (4 bytes)
/* 血糖測試參數保留區 */
#define PARAM_BG_RESERVED     114    // 保留區 (99 bytes)

/* 尿酸(U)專用參數地址定義 */
#define PARAM_U_CSU_TOL       213    // 試片檢查容差
#define PARAM_U_NDL           214    // 新試片測濃水平 (2 bytes)
#define PARAM_U_UDL           216    // 已使用試片測濃水平 (2 bytes)
#define PARAM_U_BLOOD_IN      218    // 血液測濃水平 (2 bytes)
#define PARAM_U_STRIP_LOT     220    // 試片批號 (16 bytes)
#define PARAM_U_L             236    // 尿酸值下限
#define PARAM_U_H             237    // 尿酸值上限
#define PARAM_U_T3_E37        238    // T3 ADV超E37閾值 (2 bytes)
/* 尿酸測試時序參數 */
#define PARAM_U_TPL1          240    // 測試間格1 (2 bytes)
#define PARAM_U_TRD1          242    // 讀取延遲1 (2 bytes)
#define PARAM_U_EVWIDTH1      244    // 工作電壓寬度1 (2 bytes)
#define PARAM_U_TPL2          246    // 測試間格2 (2 bytes)
#define PARAM_U_TRD2          248    // 讀取延遲2 (2 bytes)
#define PARAM_U_EVWIDTH2      250    // 工作電壓寬度2 (2 bytes)
/* 尿酸標準品曲線參數 */
#define PARAM_U_CAL_A1        252    // 標準品曲線係數A1 (4 bytes)
#define PARAM_U_CAL_B1        256    // 標準品曲線係數B1 (4 bytes)
#define PARAM_U_CAL_C1        260    // 標準品曲線係數C1 (4 bytes)
/* 尿酸溫度補償參數 */
#define PARAM_U_TEMP_A1       264    // 溫度補償係數A1 (4 bytes)
#define PARAM_U_TEMP_B1       268    // 溫度補償係數B1 (4 bytes)
#define PARAM_U_TEMP_C1       272    // 溫度補償係數C1 (4 bytes)
/* 尿酸血量補償參數 */
#define PARAM_U_BLOOD_A1      276    // 血量補償係數A1 (4 bytes)
#define PARAM_U_BLOOD_B1      280    // 血量補償係數B1 (4 bytes)
#define PARAM_U_BLOOD_C1      284    // 血量補償係數C1 (4 bytes)
/* 尿酸測試參數保留區 */
#define PARAM_U_RESERVED      288    // 保留區 (67 bytes)

/* 總膽固醇(C)專用參數地址定義 */
#define PARAM_C_CSU_TOL       355    // 試片檢查容差
#define PARAM_C_NDL           356    // 新試片測濃水平 (2 bytes)
#define PARAM_C_UDL           358    // 已使用試片測濃水平 (2 bytes)
#define PARAM_C_BLOOD_IN      360    // 血液測濃水平 (2 bytes)
#define PARAM_C_STRIP_LOT     362    // 試片批號 (16 bytes)
#define PARAM_C_L             378    // 總膽固醇值下限
#define PARAM_C_H             379    // 總膽固醇值上限 (2 bytes)
#define PARAM_C_T3_E37        381    // T3 ADV超E37閾值 (2 bytes)
/* 總膽固醇測試時序參數 */
#define PARAM_C_TPL1          383    // 測試間格1 (2 bytes)
#define PARAM_C_TRD1          385    // 讀取延遲1 (2 bytes)
#define PARAM_C_EVWIDTH1      387    // 工作電壓寬度1 (2 bytes)
#define PARAM_C_TPL2          389    // 測試間格2 (2 bytes)
#define PARAM_C_TRD2          391    // 讀取延遲2 (2 bytes)
#define PARAM_C_EVWIDTH2      393    // 工作電壓寬度2 (2 bytes)
/* 總膽固醇標準品曲線參數 */
#define PARAM_C_CAL_A1        395    // 標準品曲線係數A1 (4 bytes)
#define PARAM_C_CAL_B1        399    // 標準品曲線係數B1 (4 bytes)
#define PARAM_C_CAL_C1        403    // 標準品曲線係數C1 (4 bytes)
/* 總膽固醇溫度補償參數 */
#define PARAM_C_TEMP_A1       407    // 溫度補償係數A1 (4 bytes)
#define PARAM_C_TEMP_B1       411    // 溫度補償係數B1 (4 bytes)
#define PARAM_C_TEMP_C1       415    // 溫度補償係數C1 (4 bytes)
/* 總膽固醇血量補償參數 */
#define PARAM_C_BLOOD_A1      419    // 血量補償係數A1 (4 bytes)
#define PARAM_C_BLOOD_B1      423    // 血量補償係數B1 (4 bytes)
#define PARAM_C_BLOOD_C1      427    // 血量補償係數C1 (4 bytes)
/* 總膽固醇測試參數保留區 */
#define PARAM_C_RESERVED      431    // 保留區 (66 bytes)

/* 三酸甘油脂(TG)專用參數地址定義 */
#define PARAM_TG_CSU_TOL      497    // 試片檢查容差
#define PARAM_TG_NDL          498    // 新試片測濃水平 (2 bytes)
#define PARAM_TG_UDL          500    // 已使用試片測濃水平 (2 bytes)
#define PARAM_TG_BLOOD_IN     502    // 血液測濃水平 (2 bytes)
#define PARAM_TG_STRIP_LOT    504    // 試片批號 (16 bytes)
#define PARAM_TG_L            520    // 三酸甘油脂值下限
#define PARAM_TG_H            521    // 三酸甘油脂值上限 (2 bytes)
#define PARAM_TG_T3_E37       523    // T3 ADV超E37閾值 (2 bytes)
/* 三酸甘油脂測試時序參數 */
#define PARAM_TG_TPL1         525    // 測試間格1 (2 bytes)
#define PARAM_TG_TRD1         527    // 讀取延遲1 (2 bytes)
#define PARAM_TG_EVWIDTH1     529    // 工作電壓寬度1 (2 bytes)
#define PARAM_TG_TPL2         531    // 測試間格2 (2 bytes)
#define PARAM_TG_TRD2         533    // 讀取延遲2 (2 bytes)
#define PARAM_TG_EVWIDTH2     535    // 工作電壓寬度2 (2 bytes)
/* 三酸甘油脂標準品曲線參數 */
#define PARAM_TG_CAL_A1       537    // 標準品曲線係數A1 (4 bytes)
#define PARAM_TG_CAL_B1       541    // 標準品曲線係數B1 (4 bytes)
#define PARAM_TG_CAL_C1       545    // 標準品曲線係數C1 (4 bytes)
/* 三酸甘油脂溫度補償參數 */
#define PARAM_TG_TEMP_A1      549    // 溫度補償係數A1 (4 bytes)
#define PARAM_TG_TEMP_B1      553    // 溫度補償係數B1 (4 bytes)
#define PARAM_TG_TEMP_C1      557    // 溫度補償係數C1 (4 bytes)
/* 三酸甘油脂血量補償參數 */
#define PARAM_TG_BLOOD_A1     561    // 血量補償係數A1 (4 bytes)
#define PARAM_TG_BLOOD_B1     565    // 血量補償係數B1 (4 bytes)
#define PARAM_TG_BLOOD_C1     569    // 血量補償係數C1 (4 bytes)
/* 三酸甘油脂測試參數保留區 */
#define PARAM_TG_RESERVED     573    // 保留區 (66 bytes)

/* 保留區域 */
#define PARAM_RESERVED        639    // 系統保留 (37 bytes)

/* 校驗區域 */
#define PARAM_SUM_L           676    // 校驗和低位元組
#define PARAM_SUM_H           677    // 校驗和高位元組
#define PARAM_CRC16           675    // CRC16校驗位元組

/* 試片類型定義 */
typedef enum {
    STRIP_TYPE_GLV = 0,   // 血糖試片(正常)
    STRIP_TYPE_U,         // 尿酸試片
    STRIP_TYPE_C,         // 總膽固醇試片
    STRIP_TYPE_TG,        // 三酸甘油脂試片
    STRIP_TYPE_GAV,       // 血糖試片(高精度)
    STRIP_TYPE_MAX
} StripType_TypeDef;

/* 單位定義 */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL,           // mg/dL
    UNIT_GM_DL,           // g/dL
    UNIT_MAX
} Unit_TypeDef;

/* 事件類型定義 */
typedef enum {
    EVENT_GEN = 0,        // 一般測試
    EVENT_AC,             // 餐前測試
    EVENT_PC,             // 餐後測試
    EVENT_QC1,            // 品管液1
    EVENT_QC2,            // 品管液2
    EVENT_QC3,            // 品管液3
    EVENT_MAX
} Event_TypeDef;

/* 功能區塊定義 */
typedef enum {
    BLOCK_BASIC_SYSTEM = 0,    // 基本系統參數
    BLOCK_HARDWARE_CALIB,      // 硬體校準參數
    BLOCK_BG_PARAMS,           // 血糖參數
    BLOCK_U_PARAMS,            // 尿酸參數
    BLOCK_C_PARAMS,            // 總膽固醇參數
    BLOCK_TG_PARAMS,           // 三酸甘油脂參數
    BLOCK_RESERVED,            // 保留區
    BLOCK_MAX
} BlockType_TypeDef;

/* 功能函數聲明 */
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
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);

/* 高級功能函數聲明 */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                         uint8_t *hour, uint8_t *minute, uint8_t *second);
uint8_t PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date, 
                         uint8_t hour, uint8_t minute, uint8_t second);
uint8_t PARAM_IncreaseTestCount(void);
uint16_t PARAM_GetTestCount(void);
uint8_t PARAM_ReadBlock(BlockType_TypeDef block, void *data, uint16_t size);
uint8_t PARAM_UpdateBlock(BlockType_TypeDef block, void *data, uint16_t size);
uint8_t PARAM_GetStripParameters(StripType_TypeDef type, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
uint8_t PARAM_GetTimingParameters(StripType_TypeDef type, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t phase);

#endif /* __PARAM_TABLE_H */