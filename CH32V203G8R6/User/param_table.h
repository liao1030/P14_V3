/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : 多功能生化測試儀參數代碼表頭文件
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* 參數表總大小 (bytes) */
#define PARAM_TABLE_SIZE 678
/* 參數表校驗總大小 (0~674) */
#define PARAM_TABLE_CHECK_SIZE 675

/* 試片類型定義 */
typedef enum {
    STRIP_TYPE_GLV = 0, // 血糖(GLV)
    STRIP_TYPE_U = 1,   // 尿酸(U)
    STRIP_TYPE_C = 2,   // 總膽固醇(C)
    STRIP_TYPE_TG = 3,  // 三酸甘油脂(TG)
    STRIP_TYPE_GAV = 4  // 血糖(GAV)
} StripType_TypeDef;

/* 事件類型定義 */
typedef enum {
    EVENT_QC = 0,  // 品管液
    EVENT_AC = 1,  // 空腹
    EVENT_PC = 2   // 餐後
} Event_TypeDef;

/* 濃度單位定義 */
typedef enum {
    UNIT_MMOL_L = 0,  // mmol/L
    UNIT_MG_DL = 1,   // mg/dL
    UNIT_GM_DL = 2    // gm/dl
} Unit_TypeDef;

/* 工廠模式狀態 */
#define FACTORY_USER_MODE 0     // 使用者模式
#define FACTORY_FACTORY_MODE 210 // 工廠模式

/* 參數表結構定義 */
typedef struct {
    /* 1. 系統基本參數: 0-8 (9 bytes) */
    uint8_t LBT;              // 低電池閾值
    uint8_t OBT;              // 電池耗竭閾值
    uint8_t FACTORY;          // 儀器操作模式
    uint8_t MODEL_NO;         // 產品型號
    uint8_t FW_NO;            // 韌體版本號
    uint16_t NOT;             // 測試次數
    uint16_t Code_Table_V;    // 代碼表版本號

    /* 2. 時間設定參數: 9-14 (6 bytes) */
    uint8_t YEAR;             // 年份設定 (0~99, 表示2000~2099年)
    uint8_t MONTH;            // 月份設定 (1~12)
    uint8_t DATE;             // 日期設定 (1~31)
    uint8_t HOUR;             // 小時設定 (0~23)
    uint8_t MINUTE;           // 分鐘設定 (0~59)
    uint8_t SECOND;           // 秒數設定 (0~59)

    /* 3. 測試環境參數: 15-19 (5 bytes) */
    uint8_t TLL;              // 操作溫度範圍下限
    uint8_t TLH;              // 操作溫度範圍上限
    uint8_t MGDL;             // 濃度單位設定
    uint8_t EVENT;            // 設置事件類型
    uint8_t Strip_Type;       // 測試項目

    /* 4. 硬體校正參數: 20-38 (19 bytes) */
    uint16_t EV_T3_TRG;       // EV_T3觸發電壓
    uint8_t EV_WORKING;       // 測量工作電壓電壓
    uint8_t EV_T3;            // 血液滴入電極電壓
    uint8_t DACO;             // DAC偏移補償
    uint8_t DACDO;            // DAC校正管理參數
    uint8_t CC211NoDone;      // CC211未完成旗標
    uint8_t CAL_TOL;          // OPS/OPI校正容差
    float OPS;                // OPA校正斜率
    float OPI;                // OPA校正截距
    uint8_t QCT;              // QCT校正測試低位元組
    uint8_t TOFFSET;          // 溫度校正偏移
    uint8_t BOFFSET;          // 電池校正偏移

    /* 5. 血糖(GLV/GAV)專用參數: 39-212 (174 bytes) */
    /* 5.1 試片參數 */
    uint8_t BG_CSU_TOL;       // 試片檢查容差
    uint16_t BG_NDL;          // 新試片水平
    uint16_t BG_UDL;          // 已使用試片水平
    uint16_t BG_BLOOD_IN;     // 血液水平
    uint8_t BG_Strip_Lot[16]; // 試片批號(16字節)

    /* 5.2 測量範圍參數 */
    uint8_t BG_L;             // 血糖值下限
    uint8_t BG_H;             // 血糖值上限
    uint16_t BG_T3_E37;       // T3 ADV閾E37閾值

    /* 5.3 測試時序參數 */
    uint16_t BG_TPL_1;        // 時間脈衝寬度 (第一組)
    uint16_t BG_TRD_1;        // 原始數據時間 (第一組)
    uint16_t BG_EV_Width_1;   // 脈衝時間 (第一組)
    uint16_t BG_TPL_2;        // 時間脈衝寬度 (第二組)
    uint16_t BG_TRD_2;        // 原始數據時間 (第二組)
    uint16_t BG_EV_Width_2;   // 脈衝時間 (第二組)

    /* 5.4 品管液(QC)參數 */
    uint8_t BG_Sq;            // QC敏感參數Sq
    uint8_t BG_Iq;            // QC敏感參數Iq
    uint8_t BG_CVq;           // QC水平的CV
    uint8_t BG_Aq;            // QC敏感參數A
    uint8_t BG_Bq;            // QC敏感參數B
    uint8_t BG_Aq5;           // QC敏感參數A(等級5)
    uint8_t BG_Bq5;           // QC敏感參數B(等級5)
    uint8_t BG_QC[30];        // QC標準範圍參數 (QC1-QC5)

    /* 5.5 計算式敏感係數 */
    uint8_t BG_S2;            // 試片敏感參數
    uint8_t BG_I2;            // 試片敏感參數
    float BG_Sr;              // OPA硬體迴路敏感(斜率)
    float BG_Ir;              // OPA硬體迴路敏感(截距)
    uint8_t BG_S3[30];        // 試片敏感參數S3(1-30)
    uint8_t BG_I3[30];        // 試片敏感參數I3(1-30)

    /* 5.6 溫度敏感參數 */
    uint8_t BG_TF[7];         // AC/PC溫度敏感斜率(10-40°C)
    uint8_t BG_CTF[7];        // QC溫度敏感斜率(10-40°C)
    uint8_t BG_TO[7];         // AC/PC溫度敏感截距(10-40°C)
    uint8_t BG_CTO[7];        // QC溫度敏感截距(10-40°C)

    /* 6. 尿酸(U)專用參數: 213-354 (142 bytes) */
    /* 6.1 試片參數 */
    uint8_t U_CSU_TOL;        // 試片檢查容差
    uint16_t U_NDL;           // 新試片水平
    uint16_t U_UDL;           // 已使用試片水平
    uint16_t U_BLOOD_IN;      // 血液水平
    uint8_t U_Strip_Lot[16];  // 試片批號(16字節)

    /* 6.2 測量範圍參數 */
    uint8_t U_L;              // 尿酸值下限
    uint8_t U_H;              // 尿酸值上限
    uint16_t U_T3_E37;        // T3 ADV閾E37閾值

    /* 6.3 測試時序參數 */
    uint16_t U_TPL_1;         // 時間脈衝寬度 (第一組)
    uint16_t U_TRD_1;         // 原始數據時間 (第一組)
    uint16_t U_EV_Width_1;    // 脈衝時間 (第一組)
    uint16_t U_TPL_2;         // 時間脈衝寬度 (第二組)
    uint16_t U_TRD_2;         // 原始數據時間 (第二組)
    uint16_t U_EV_Width_2;    // 脈衝時間 (第二組)

    /* 6.4 品管液(QC)參數 */
    uint8_t U_Sq;             // QC敏感參數Sq
    uint8_t U_Iq;             // QC敏感參數Iq
    uint8_t U_CVq;            // QC水平的CV
    uint8_t U_Aq;             // QC敏感參數A
    uint8_t U_Bq;             // QC敏感參數B
    uint8_t U_QC[30];         // QC標準範圍參數 (QC1-QC5)

    /* 6.5 計算式敏感係數 */
    uint8_t U_S2;             // 試片敏感參數
    uint8_t U_I2;             // 試片敏感參數
    float U_Sr;               // OPA硬體迴路敏感(斜率)
    float U_Ir;               // OPA硬體迴路敏感(截距)
    uint8_t U_S3[15];         // 試片敏感參數S3(1-15)
    uint8_t U_I3[15];         // 試片敏感參數I3(1-15)

    /* 6.6 溫度敏感參數 */
    uint8_t U_TF[7];          // AC/PC溫度敏感斜率(10-40°C)
    uint8_t U_CTF[7];         // QC溫度敏感斜率(10-40°C)
    uint8_t U_TO[7];          // AC/PC溫度敏感截距(10-40°C)
    uint8_t U_CTO[7];         // QC溫度敏感截距(10-40°C)

    /* 7. 總膽固醇(C)專用參數: 355-496 (142 bytes) */
    /* 7.1 試片參數 */
    uint8_t C_CSU_TOL;        // 試片檢查容差
    uint16_t C_NDL;           // 新試片水平
    uint16_t C_UDL;           // 已使用試片水平
    uint16_t C_BLOOD_IN;      // 血液水平
    uint8_t C_Strip_Lot[16];  // 試片批號(16字節)

    /* 7.2 測量範圍參數 */
    uint8_t C_L;              // 膽固醇值下限
    uint8_t C_H;              // 膽固醇值上限
    uint16_t C_T3_E37;        // T3 ADV閾E37閾值

    /* 7.3 測試時序參數 */
    uint16_t C_TPL_1;         // 時間脈衝寬度 (第一組)
    uint16_t C_TRD_1;         // 原始數據時間 (第一組)
    uint16_t C_EV_Width_1;    // 脈衝時間 (第一組)
    uint16_t C_TPL_2;         // 時間脈衝寬度 (第二組)
    uint16_t C_TRD_2;         // 原始數據時間 (第二組)
    uint16_t C_EV_Width_2;    // 脈衝時間 (第二組)

    /* 7.4 品管液(QC)參數 */
    uint8_t C_Sq;             // QC敏感參數Sq
    uint8_t C_Iq;             // QC敏感參數Iq
    uint8_t C_CVq;            // QC水平的CV
    uint8_t C_Aq;             // QC敏感參數A
    uint8_t C_Bq;             // QC敏感參數B
    uint8_t C_QC[30];         // QC標準範圍參數 (QC1-QC5)

    /* 7.5 計算式敏感係數 */
    uint8_t C_S2;             // 試片敏感參數
    uint8_t C_I2;             // 試片敏感參數
    float C_Sr;               // OPA硬體迴路敏感(斜率)
    float C_Ir;               // OPA硬體迴路敏感(截距)
    uint8_t C_S3[15];         // 試片敏感參數S3(1-15)
    uint8_t C_I3[15];         // 試片敏感參數I3(1-15)

    /* 7.6 溫度敏感參數 */
    uint8_t C_TF[7];          // AC/PC溫度敏感斜率(10-40°C)
    uint8_t C_CTF[7];         // QC溫度敏感斜率(10-40°C)
    uint8_t C_TO[7];          // AC/PC溫度敏感截距(10-40°C)
    uint8_t C_CTO[7];         // QC溫度敏感截距(10-40°C)

    /* 8. 三酸甘油脂(TG)專用參數: 497-638 (142 bytes) */
    /* 8.1 試片參數 */
    uint8_t TG_CSU_TOL;       // 試片檢查容差
    uint16_t TG_NDL;          // 新試片水平
    uint16_t TG_UDL;          // 已使用試片水平
    uint16_t TG_BLOOD_IN;     // 血液水平
    uint8_t TG_Strip_Lot[16]; // 試片批號(16字節)

    /* 8.2 測量範圍參數 */
    uint8_t TG_L;             // 三酸甘油脂值下限
    uint8_t TG_H;             // 三酸甘油脂值上限
    uint16_t TG_T3_E37;       // T3 ADV閾E37閾值

    /* 8.3 測試時序參數 */
    uint16_t TG_TPL_1;        // 時間脈衝寬度 (第一組)
    uint16_t TG_TRD_1;        // 原始數據時間 (第一組)
    uint16_t TG_EV_Width_1;   // 脈衝時間 (第一組)
    uint16_t TG_TPL_2;        // 時間脈衝寬度 (第二組)
    uint16_t TG_TRD_2;        // 原始數據時間 (第二組)
    uint16_t TG_EV_Width_2;   // 脈衝時間 (第二組)

    /* 8.4 品管液(QC)參數 */
    uint8_t TG_Sq;            // QC敏感參數Sq
    uint8_t TG_Iq;            // QC敏感參數Iq
    uint8_t TG_CVq;           // QC水平的CV
    uint8_t TG_Aq;            // QC敏感參數A
    uint8_t TG_Bq;            // QC敏感參數B
    uint8_t TG_QC[30];        // QC標準範圍參數 (QC1-QC5)

    /* 8.5 計算式敏感係數 */
    uint8_t TG_S2;            // 試片敏感參數
    uint8_t TG_I2;            // 試片敏感參數
    float TG_Sr;              // OPA硬體迴路敏感(斜率)
    float TG_Ir;              // OPA硬體迴路敏感(截距)
    uint8_t TG_S3[15];        // 試片敏感參數S3(1-15)
    uint8_t TG_I3[15];        // 試片敏感參數I3(1-15)

    /* 8.6 溫度敏感參數 */
    uint8_t TG_TF[7];         // AC/PC溫度敏感斜率(10-40°C)
    uint8_t TG_CTF[7];        // QC溫度敏感斜率(10-40°C)
    uint8_t TG_TO[7];         // AC/PC溫度敏感截距(10-40°C)
    uint8_t TG_CTO[7];        // QC溫度敏感截距(10-40°C)

    /* 9. 保留及校驗區: 639-677 (39 bytes) */
    uint8_t Reserved[36];     // 保留區域供未來擴展
    uint8_t SUM_L;            // 地址0~674校驗和(低)
    uint8_t SUM_H;            // 地址0~674校驗和(高)
    uint8_t CRC16;            // 完整參數表CRC校驗
} Param_Table_TypeDef;

/* 函數原型聲明 */
void PARAM_TABLE_Init(void);
void PARAM_TABLE_SetDefaults(void);
void PARAM_TABLE_SaveToFlash(void);
void PARAM_TABLE_LoadFromFlash(void);
uint16_t PARAM_TABLE_CalculateChecksum(void);
uint8_t PARAM_TABLE_Verify(void);
void PARAM_TABLE_PrintInfo(void);

/* 參數表全局變量 */
extern Param_Table_TypeDef g_ParamTable;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */