/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Parameter_Table.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : 多功能生化測試儀參數代碼表定義
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_PARAMETER_TABLE_H__
#define __P14_PARAMETER_TABLE_H__

#include "ch32v20x.h"

/* 參數表總大小 */
#define PARAM_TABLE_SIZE 678

/* 1. 系統基本參數地址定義 */
#define PARAM_LBT            0    // 低電池閾值
#define PARAM_OBT            1    // 電池耗盡閾值
#define PARAM_FACTORY        2    // 儀器操作模式
#define PARAM_MODEL_NO       3    // 產品型號
#define PARAM_FW_NO          4    // 韌體版本號
#define PARAM_NOT            5    // 測量次數 (2 bytes)
#define PARAM_CODE_TABLE_V   7    // 代號表版本編號 (2 bytes)

/* 2. 時間設定參數地址定義 */
#define PARAM_YEAR           9    // 年份設定
#define PARAM_MONTH          10   // 月份設定
#define PARAM_DATE           11   // 日期設定
#define PARAM_HOUR           12   // 小時設定
#define PARAM_MINUTE         13   // 分鐘設定
#define PARAM_SECOND         14   // 秒數設定

/* 3. 測量環境參數地址定義 */
#define PARAM_TLL            15   // 操作溫度範圍下限
#define PARAM_TLH            16   // 操作溫度範圍上限
#define PARAM_MGDL           17   // 測量單位設定
#define PARAM_EVENT          18   // 調節事件類型
#define PARAM_STRIP_TYPE     19   // 測試項目

/* 4. 硬體校正參數地址定義 */
#define PARAM_EV_T3_TRG      20   // EV_T3觸發電壓 (2 bytes)
#define PARAM_EV_WORKING     22   // 測量工作電極電壓
#define PARAM_EV_T3          23   // 血液流速電極電壓
#define PARAM_DACO           24   // DAC偏移敏感
#define PARAM_DACDO          25   // DAC校正管理參數
#define PARAM_CC211NO_DONE   26   // CC211未完成旗標
#define PARAM_CAL_TOL        27   // OPS/OPI校正容差
#define PARAM_OPS            28   // OPA校正斜率 (4 bytes)
#define PARAM_OPI            32   // OPA校正截距 (4 bytes)
#define PARAM_QCT            36   // QCT校正測量低位元組
#define PARAM_TOFFSET        37   // 溫度校正偏移
#define PARAM_BOFFSET        38   // 電池校正偏移

/* 5. 血糖(GLV/GAV)專用參數地址定義 */
/* 5.1 試片參數 */
#define PARAM_BG_CSU_TOL     39   // 試片檢查容差
#define PARAM_BG_NDL         40   // 新試片測量水平 (2 bytes)
#define PARAM_BG_UDL         42   // 已使用試片測量水平 (2 bytes)
#define PARAM_BG_BLOOD_IN    44   // 血液測量水平 (2 bytes)
#define PARAM_BG_STRIP_LOT   46   // 試片批號 (16字節)

/* 5.2 測量範圍參數 */
#define PARAM_BG_L           62   // 血糖值下限
#define PARAM_BG_H           63   // 血糖值上限
#define PARAM_BG_T3_E37      64   // T3 ADV溢37閾值 (2 bytes)

/* 5.3 測試時序參數 */
#define PARAM_BG_TPL_1       66   // 時間脈衝寬度(第一組) (2 bytes)
#define PARAM_BG_TRD_1       68   // 原始數據時間(第一組) (2 bytes)
#define PARAM_BG_EV_WIDTH_1  70   // 熔燒時間(第一組) (2 bytes)
#define PARAM_BG_TPL_2       72   // 時間脈衝寬度(第二組) (2 bytes)
#define PARAM_BG_TRD_2       74   // 原始數據時間(第二組) (2 bytes)
#define PARAM_BG_EV_WIDTH_2  76   // 熔燒時間(第二組) (2 bytes)

/* 5.4 品管液(QC)參數 */
#define PARAM_BG_SQ          78   // QC敏感參數Sq
#define PARAM_BG_IQ          79   // QC敏感參數Iq
#define PARAM_BG_CVQ         80   // QC水平的CV
#define PARAM_BG_AQ          81   // QC敏感參數A
#define PARAM_BG_BQ          82   // QC敏感參數B
#define PARAM_BG_AQ5         83   // QC敏感參數A(等級5)
#define PARAM_BG_BQ5         84   // QC敏感參數B(等級5)
#define PARAM_BG_QC          85   // QC模板範圍參數 (30 bytes)

/* 5.5 計算式靈敏度參數 */
#define PARAM_BG_S2          115  // 試片敏感參數
#define PARAM_BG_I2          116  // 試片敏感參數
#define PARAM_BG_SR          117  // OPA硬體迴路敏感(斜率) (4 bytes)
#define PARAM_BG_IR          121  // OPA硬體迴路敏感(截距) (4 bytes)
#define PARAM_BG_S3          125  // 試片敏感參數S3 (30 bytes)
#define PARAM_BG_I3          155  // 試片敏感參數I3 (30 bytes)

/* 5.6 溫度敏感參數 */
#define PARAM_BG_TF          185  // AC/PC溫度敏感斜率 (7 bytes)
#define PARAM_BG_CTF         192  // QC溫度敏感斜率 (7 bytes)
#define PARAM_BG_TO          199  // AC/PC溫度敏感截距 (7 bytes)
#define PARAM_BG_CTO         206  // QC溫度敏感截距 (7 bytes)

/* 6. 尿酸(U)專用參數地址定義 */
/* 6.1 試片參數 */
#define PARAM_U_CSU_TOL      213  // 試片檢查容差
#define PARAM_U_NDL          214  // 新試片測量水平 (2 bytes)
#define PARAM_U_UDL          216  // 已使用試片測量水平 (2 bytes)
#define PARAM_U_BLOOD_IN     218  // 血液測量水平 (2 bytes)
#define PARAM_U_STRIP_LOT    220  // 試片批號 (16字節)

/* 6.2 測量範圍參數 */
#define PARAM_U_L            236  // 尿酸值下限
#define PARAM_U_H            237  // 尿酸值上限
#define PARAM_U_T3_E37       238  // T3 ADV溢37閾值 (2 bytes)

/* 6.3 測試時序參數 */
#define PARAM_U_TPL_1        240  // 時間脈衝寬度(第一組) (2 bytes)
#define PARAM_U_TRD_1        242  // 原始數據時間(第一組) (2 bytes)
#define PARAM_U_EV_WIDTH_1   244  // 熔燒時間(第一組) (2 bytes)
#define PARAM_U_TPL_2        246  // 時間脈衝寬度(第二組) (2 bytes)
#define PARAM_U_TRD_2        248  // 原始數據時間(第二組) (2 bytes)
#define PARAM_U_EV_WIDTH_2   250  // 熔燒時間(第二組) (2 bytes)

/* 6.4 品管液(QC)參數 */
#define PARAM_U_SQ           252  // QC敏感參數Sq
#define PARAM_U_IQ           253  // QC敏感參數Iq
#define PARAM_U_CVQ          254  // QC水平的CV
#define PARAM_U_AQ           255  // QC敏感參數A
#define PARAM_U_BQ           256  // QC敏感參數B
#define PARAM_U_QC           257  // QC模板範圍參數 (30 bytes)

/* 6.5 計算式靈敏度參數 */
#define PARAM_U_S2           287  // 試片敏感參數
#define PARAM_U_I2           288  // 試片敏感參數
#define PARAM_U_SR           289  // OPA硬體迴路敏感(斜率) (4 bytes)
#define PARAM_U_IR           293  // OPA硬體迴路敏感(截距) (4 bytes)
#define PARAM_U_S3           297  // 試片敏感參數S3 (15 bytes)
#define PARAM_U_I3           312  // 試片敏感參數I3 (15 bytes)

/* 6.6 溫度敏感參數 */
#define PARAM_U_TF           327  // AC/PC溫度敏感斜率 (7 bytes)
#define PARAM_U_CTF          334  // QC溫度敏感斜率 (7 bytes)
#define PARAM_U_TO           341  // AC/PC溫度敏感截距 (7 bytes)
#define PARAM_U_CTO          348  // QC溫度敏感截距 (7 bytes)

/* 7. 總膽固醇(C)專用參數地址定義 */
/* 7.1 試片參數 */
#define PARAM_C_CSU_TOL      355  // 試片檢查容差
#define PARAM_C_NDL          356  // 新試片測量水平 (2 bytes)
#define PARAM_C_UDL          358  // 已使用試片測量水平 (2 bytes)
#define PARAM_C_BLOOD_IN     360  // 血液測量水平 (2 bytes)
#define PARAM_C_STRIP_LOT    362  // 試片批號 (16字節)

/* 7.2 測量範圍參數 */
#define PARAM_C_L            378  // 膽固醇值下限
#define PARAM_C_H            379  // 膽固醇值上限
#define PARAM_C_T3_E37       380  // T3 ADV溢37閾值 (2 bytes)

/* 7.3 測試時序參數 */
#define PARAM_C_TPL_1        382  // 時間脈衝寬度(第一組) (2 bytes)
#define PARAM_C_TRD_1        384  // 原始數據時間(第一組) (2 bytes)
#define PARAM_C_EV_WIDTH_1   386  // 熔燒時間(第一組) (2 bytes)
#define PARAM_C_TPL_2        388  // 時間脈衝寬度(第二組) (2 bytes)
#define PARAM_C_TRD_2        390  // 原始數據時間(第二組) (2 bytes)
#define PARAM_C_EV_WIDTH_2   392  // 熔燒時間(第二組) (2 bytes)

/* 7.4 品管液(QC)參數 */
#define PARAM_C_SQ           394  // QC敏感參數Sq
#define PARAM_C_IQ           395  // QC敏感參數Iq
#define PARAM_C_CVQ          396  // QC水平的CV
#define PARAM_C_AQ           397  // QC敏感參數A
#define PARAM_C_BQ           398  // QC敏感參數B
#define PARAM_C_QC           399  // QC模板範圍參數 (30 bytes)

/* 7.5 計算式靈敏度參數 */
#define PARAM_C_S2           429  // 試片敏感參數
#define PARAM_C_I2           430  // 試片敏感參數
#define PARAM_C_SR           431  // OPA硬體迴路敏感(斜率) (4 bytes)
#define PARAM_C_IR           435  // OPA硬體迴路敏感(截距) (4 bytes)
#define PARAM_C_S3           439  // 試片敏感參數S3 (15 bytes)
#define PARAM_C_I3           454  // 試片敏感參數I3 (15 bytes)

/* 7.6 溫度敏感參數 */
#define PARAM_C_TF           469  // AC/PC溫度敏感斜率 (7 bytes)
#define PARAM_C_CTF          476  // QC溫度敏感斜率 (7 bytes)
#define PARAM_C_TO           483  // AC/PC溫度敏感截距 (7 bytes)
#define PARAM_C_CTO          490  // QC溫度敏感截距 (7 bytes)

/* 8. 三酸甘油脂(TG)專用參數地址定義 */
/* 8.1 試片參數 */
#define PARAM_TG_CSU_TOL     497  // 試片檢查容差
#define PARAM_TG_NDL         498  // 新試片測量水平 (2 bytes)
#define PARAM_TG_UDL         500  // 已使用試片測量水平 (2 bytes)
#define PARAM_TG_BLOOD_IN    502  // 血液測量水平 (2 bytes)
#define PARAM_TG_STRIP_LOT   504  // 試片批號 (16字節)

/* 8.2 測量範圍參數 */
#define PARAM_TG_L           520  // 三酸甘油脂值下限
#define PARAM_TG_H           521  // 三酸甘油脂值上限
#define PARAM_TG_T3_E37      522  // T3 ADV溢37閾值 (2 bytes)

/* 8.3 測試時序參數 */
#define PARAM_TG_TPL_1       524  // 時間脈衝寬度(第一組) (2 bytes)
#define PARAM_TG_TRD_1       526  // 原始數據時間(第一組) (2 bytes)
#define PARAM_TG_EV_WIDTH_1  528  // 熔燒時間(第一組) (2 bytes)
#define PARAM_TG_TPL_2       530  // 時間脈衝寬度(第二組) (2 bytes)
#define PARAM_TG_TRD_2       532  // 原始數據時間(第二組) (2 bytes)
#define PARAM_TG_EV_WIDTH_2  534  // 熔燒時間(第二組) (2 bytes)

/* 8.4 品管液(QC)參數 */
#define PARAM_TG_SQ          536  // QC敏感參數Sq
#define PARAM_TG_IQ          537  // QC敏感參數Iq
#define PARAM_TG_CVQ         538  // QC水平的CV
#define PARAM_TG_AQ          539  // QC敏感參數A
#define PARAM_TG_BQ          540  // QC敏感參數B
#define PARAM_TG_QC          541  // QC模板範圍參數 (30 bytes)

/* 8.5 計算式靈敏度參數 */
#define PARAM_TG_S2          571  // 試片敏感參數
#define PARAM_TG_I2          572  // 試片敏感參數
#define PARAM_TG_SR          573  // OPA硬體迴路敏感(斜率) (4 bytes)
#define PARAM_TG_IR          577  // OPA硬體迴路敏感(截距) (4 bytes)
#define PARAM_TG_S3          581  // 試片敏感參數S3 (15 bytes)
#define PARAM_TG_I3          596  // 試片敏感參數I3 (15 bytes)

/* 8.6 溫度敏感參數 */
#define PARAM_TG_TF          611  // AC/PC溫度敏感斜率 (7 bytes)
#define PARAM_TG_CTF         618  // QC溫度敏感斜率 (7 bytes)
#define PARAM_TG_TO          625  // AC/PC溫度敏感截距 (7 bytes)
#define PARAM_TG_CTO         632  // QC溫度敏感截距 (7 bytes)

/* 9. 保留與校驗區 */
#define PARAM_RESERVED       639  // 保留區域供未來擴展 (36 bytes)
#define PARAM_SUM_L          675  // 地址0~674校驗和(低)
#define PARAM_SUM_H          676  // 地址0~674校驗和(高)
#define PARAM_CRC16          677  // 完整參數表CRC校驗

/* 測試項目類型定義 */
typedef enum {
    STRIP_TYPE_GLV = 0,   // 血糖
    STRIP_TYPE_U   = 1,   // 尿酸
    STRIP_TYPE_C   = 2,   // 總膽固醇
    STRIP_TYPE_TG  = 3,   // 三酸甘油脂
    STRIP_TYPE_GAV = 4,   // 血糖(另一種方法)
    STRIP_TYPE_UNKNOWN = 255 // 未知或錯誤
} StripType_TypeDef;

/* 事件類型定義 */
typedef enum {
    EVENT_QC = 0,  // 品管液測試
    EVENT_AC = 1,  // 餐前測試
    EVENT_PC = 2   // 餐後測試
} Event_TypeDef;

/* 測量單位定義 */
typedef enum {
    UNIT_MMOL_L = 0,  // mmol/L
    UNIT_MG_DL  = 1,  // mg/dL
    UNIT_GM_DL  = 2   // gm/dl
} Unit_TypeDef;

/* 操作模式定義 */
typedef enum {
    MODE_USER    = 0,    // 使用者模式
    MODE_FACTORY = 210   // 工廠模式
} Mode_TypeDef;

/* 函數聲明 */
void P14_ParamTable_Init(void);
void P14_ParamTable_LoadDefault(void);
uint8_t P14_ParamTable_Read(uint16_t address);
void P14_ParamTable_Write(uint16_t address, uint8_t value);
uint16_t P14_ParamTable_Read16(uint16_t address);
void P14_ParamTable_Write16(uint16_t address, uint16_t value);
float P14_ParamTable_ReadFloat(uint16_t address);
void P14_ParamTable_WriteFloat(uint16_t address, float value);
void P14_ParamTable_ReadBlock(uint16_t address, uint8_t *buffer, uint16_t length);
void P14_ParamTable_WriteBlock(uint16_t address, const uint8_t *buffer, uint16_t length);
uint8_t P14_ParamTable_VerifyChecksum(void);
void P14_ParamTable_UpdateChecksum(void);
void P14_ParamTable_Save(void);
void P14_ParamTable_Load(void);

/* 新增加的函數聲明 */
const char* P14_ParamTable_GetStripTypeName(StripType_TypeDef type);
const char* P14_ParamTable_GetEventName(Event_TypeDef event);
const char* P14_ParamTable_GetUnitName(Unit_TypeDef unit);
void P14_ParamTable_IncrementTestCount(void);
void P14_ParamTable_ResetTestCount(void);

#endif /* __P14_PARAMETER_TABLE_H__ */ 