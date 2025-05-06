/********************************** (C) COPYRIGHT  *******************************
* File Name          : param_code_table.h
* Author             : WCH / HMD
* Version            : V1.0.0
* Date               : 2025/05/06
* Description        : 多功能生化測試儀參數代碼表定義與操作
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#ifndef __PARAM_CODE_TABLE_H
#define __PARAM_CODE_TABLE_H

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 參數代碼表結構定義 */

/* 1. 系統基本參數 */
typedef struct {
    uint8_t LBT;                /* 低電池閾值 */
    uint8_t OBT;                /* 電池耗盡閾值 */
    uint8_t FACTORY;            /* 儀器操作模式 */
    uint8_t MODEL_NO;           /* 產品型號 */
    uint8_t FW_NO;              /* 韌體版本號 */
    uint16_t NOT;               /* 測試次數 */
    uint16_t Code_Table_V;      /* 代碼表版本號 */
} SystemBasicParams_TypeDef;

/* 2. 時間設定參數 */
typedef struct {
    uint8_t YEAR;               /* 年份設定 (0~99, 2000~2099年) */
    uint8_t MONTH;              /* 月份設定 (1~12) */
    uint8_t DATE;               /* 日期設定 (1~31) */
    uint8_t HOUR;               /* 小時設定 (0~23) */
    uint8_t MINUTE;             /* 分鐘設定 (0~59) */
    uint8_t SECOND;             /* 秒數設定 (0~59) */
} TimeSettingParams_TypeDef;

/* 3. 測試顯示參數 */
typedef struct {
    uint8_t TLL;                /* 操作溫度範圍下限 */
    uint8_t TLH;                /* 操作溫度範圍上限 */
    uint8_t MGDL;               /* 濃度單位設定 (0:mmol/L; 1:mg/dL; 2:gm/dl) */
    uint8_t EVENT;              /* 選項事件類型 (0:QC, 1:AC, 2:PC) */
    uint8_t Strip_Type;         /* 測試項目 (0:GLV, 1:U, 2:C, 3:TG, 4:GAV) */
} TestDisplayParams_TypeDef;

/* 4. 硬體校正參數 */
typedef struct {
    uint16_t EV_T3_TRG;         /* EV_T3觸發電壓 */
    uint8_t EV_WORKING;         /* 濃度工作電路電壓 */
    uint8_t EV_T3;              /* 血液波濃度電路電壓 */
    int8_t DACO;                /* DAC偏移補償 */
    uint8_t DACDO;              /* DAC校正管理參數 */
    uint8_t CC211NoDone;        /* CC211未完成旗標 */
    uint8_t CAL_TOL;            /* OPS/OPI校正容差 */
    float OPS;                  /* OPA校正斜率 */
    float OPI;                  /* OPA校正截距 */
    uint8_t QCT;                /* QCT校正測試低位元組 */
    int8_t TOFFSET;             /* 溫度校正偏移 */
    int8_t BOFFSET;             /* 電池校正偏移 */
} HardwareCalibParams_TypeDef;

/* 試片通用參數結構 */
typedef struct {
    uint8_t CSU_TOL;            /* 試片檢查容差 */
    uint16_t NDL;               /* 新試片濃度水平 */
    uint16_t UDL;               /* 已使用試片濃度水平 */
    uint16_t BLOOD_IN;          /* 血液濃度水平 */
    uint8_t Strip_Lot[16];      /* 試片批號(16字節) */
    uint8_t L;                  /* 數值下限 */
    uint8_t H;                  /* 數值上限 */
    uint16_t T3_E37;            /* T3 ADV超37閾值 */
    
    /* 測試時序參數 */
    uint16_t TPL_1;             /* 時間脈衝寬 (第一組) */
    uint16_t TRD_1;             /* 原始數據時間 (第一組) */
    uint16_t EV_Width_1;        /* 烈性時間 (第一組) */
    uint16_t TPL_2;             /* 時間脈衝寬 (第二組) */
    uint16_t TRD_2;             /* 原始數據時間 (第二組) */
    uint16_t EV_Width_2;        /* 烈性時間 (第二組) */
    
    /* 品管液(QC)參數 */
    uint8_t Sq;                 /* QC靈敏參數Sq */
    uint8_t Iq;                 /* QC靈敏參數Iq */
    uint8_t CVq;                /* QC水平的CV */
    uint8_t Aq;                 /* QC靈敏參數A */
    uint8_t Bq;                 /* QC靈敏參數B */
    uint8_t Aq5;                /* QC靈敏參數A(等級5) - 僅血糖使用 */
    uint8_t Bq5;                /* QC靈敏參數B(等級5) - 僅血糖使用 */
    
    /* 計算式各項參數 */
    uint8_t S2;                 /* 試片靈敏參數 */
    uint8_t I2;                 /* 試片靈敏參數 */
    float Sr;                   /* OPA硬體迴路靈敏(斜率) */
    float Ir;                   /* OPA硬體迴路靈敏(截距) */
    
    /* 溫度靈敏參數 */
    uint8_t TF[7];              /* AC/PC溫度靈敏斜率(10-40°C) */
    uint8_t CTF[7];             /* QC溫度靈敏斜率(10-40°C) */
    uint8_t TO[7];              /* AC/PC溫度靈敏截距(10-40°C) */
    uint8_t CTO[7];             /* QC溫度靈敏截距(10-40°C) */
} StripCommonParams_TypeDef;

/* 血糖特有參數 */
typedef struct {
    uint8_t QC[5][6];           /* QC標準範圍參數 */
    uint8_t S3[30];             /* 試片靈敏參數S3(1-30) */
    uint8_t I3[30];             /* 試片靈敏參數I3(1-30) */
} BloodGlucoseParams_TypeDef;

/* 尿酸/總膽固醇/三酸甘油脂特有參數 */
typedef struct {
    uint8_t QC[5][6];           /* QC標準範圍參數 */
    uint8_t S3[15];             /* 試片靈敏參數S3(1-15) */
    uint8_t I3[15];             /* 試片靈敏參數I3(1-15) */
} OtherTestParams_TypeDef;

/* 參數代碼表完整結構 */
typedef struct {
    SystemBasicParams_TypeDef System;
    TimeSettingParams_TypeDef Time;
    TestDisplayParams_TypeDef Display;
    HardwareCalibParams_TypeDef Hardware;
    
    /* 血糖(GLV/GAV)參數 */
    StripCommonParams_TypeDef BG;
    BloodGlucoseParams_TypeDef BG_Special;
    
    /* 尿酸(U)參數 */
    StripCommonParams_TypeDef U;
    OtherTestParams_TypeDef U_Special;
    
    /* 總膽固醇(C)參數 */
    StripCommonParams_TypeDef C;
    OtherTestParams_TypeDef C_Special;
    
    /* 三酸甘油脂(TG)參數 */
    StripCommonParams_TypeDef TG;
    OtherTestParams_TypeDef TG_Special;
    
    /* 校驗碼 */
    uint8_t SUM_L;              /* 地址0~674校驗和(低) */
    uint8_t SUM_H;              /* 地址0~674校驗和(高) */
    uint8_t CRC16;              /* 完整參數表CRC校驗 */
} ParamCodeTable_TypeDef;

/* 參數代碼表操作函數 */
void ParamCodeTable_Init(void);
ErrorStatus ParamCodeTable_Save(void);
ErrorStatus ParamCodeTable_Load(void);
ErrorStatus ParamCodeTable_Reset(void);
uint16_t ParamCodeTable_CalculateChecksum(void);
ErrorStatus ParamCodeTable_VerifyChecksum(void);

/* 獲取單個參數的輔助函數 */
uint8_t GetSystemParam(uint16_t paramAddress);
ErrorStatus SetSystemParam(uint16_t paramAddress, uint8_t value);

/* 檢查試片類型 */
uint8_t GetStripType(void);
const char* GetStripTypeName(uint8_t stripType);

extern ParamCodeTable_TypeDef ParamCodeTable;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_CODE_TABLE_H */