/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter code table for biochemical analyzer.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

/* Parameter table size definition */
#define PARAM_TABLE_SIZE          678    /* Total parameter table size in bytes */
#define PARAM_TABLE_BASE_ADDR     0x0800F000    /* Flash storage base address */

/* Parameter type definitions */
#define STRIP_TYPE_GLV    0       /* 血糖(GLV) */
#define STRIP_TYPE_U      1       /* 尿酸(U) */
#define STRIP_TYPE_C      2       /* 總膽固醇(C) */
#define STRIP_TYPE_TG     3       /* 三酸甘油脂(TG) */
#define STRIP_TYPE_GAV    4       /* 血糖(GAV) */

/* Factory mode definition */
#define USER_MODE         0       /* 使用者模式 */
#define FACTORY_MODE      210     /* 工廠模式 */

/* Event type definition */
#define EVENT_QC          0       /* QC (Quality Control) */
#define EVENT_AC          1       /* AC (Before meal) */
#define EVENT_PC          2       /* PC (After meal) */

/* Measurement unit definition */
#define UNIT_MMOL_L       0       /* mmol/L */
#define UNIT_MG_DL        1       /* mg/dL */
#define UNIT_GM_DL        2       /* gm/dL */

/* Parameter IDs for Param_GetValue/Param_SetValue */
typedef enum {
    /* System basic parameters */
    PARAM_LBT = 0,            /* 低電池閾值 */
    PARAM_OBT,                /* 電池耗盡閾值 */
    PARAM_FACTORY,            /* 儀器操作模式 */
    PARAM_MODEL_NO,           /* 產品型號 */
    PARAM_FW_NO,              /* 韌體版本號 */
    PARAM_NOT,                /* 測試次數 */
    PARAM_CODE_TABLE_V,       /* 代碼表版本號 */
    
    /* Time setting parameters */
    PARAM_YEAR,               /* 年份設定 (0-99) */
    PARAM_MONTH,              /* 月份設定 (1-12) */
    PARAM_DATE,               /* 日期設定 (1-31) */
    PARAM_HOUR,               /* 小時設定 (0-23) */
    PARAM_MINUTE,             /* 分鐘設定 (0-59) */
    PARAM_SECOND,             /* 秒數設定 (0-59) */
    
    /* Measurement display parameters */
    PARAM_TLL,                /* 操作溫度範圍下限 */
    PARAM_TLH,                /* 操作溫度範圍上限 */
    PARAM_MGDL,               /* 測量單位設定 */
    PARAM_EVENT,              /* 隨餐事件類型 */
    PARAM_STRIP_TYPE,         /* 測試項目 */
    
    /* Hardware calibration parameters */
    PARAM_EV_T3_TRG,          /* EV_T3觸發電壓 */
    PARAM_EV_WORKING,         /* 測量工作電壓電壓 */
    PARAM_EV_T3,              /* 血液注入電壓電壓 */
    PARAM_DACO,               /* DAC偏移靈敏 */
    PARAM_DACDO,              /* DAC校正管理參數 */
    PARAM_CC211_NO_DONE,      /* CC211未完成旗標 */
    PARAM_CAL_TOL,            /* OPS/OPI校正容差 */
    PARAM_TOFFSET,            /* 溫度校正偏移 */
    PARAM_BOFFSET,            /* 電池校正偏移 */
    
    /* Blood Glucose specific parameters */
    PARAM_BG_L,               /* 血糖值下限 */
    PARAM_BG_H,               /* 血糖值上限 */
    
    /* Uric Acid specific parameters */
    PARAM_U_L,                /* 尿酸值下限 */
    PARAM_U_H,                /* 尿酸值上限 */
    
    /* Cholesterol specific parameters */
    PARAM_C_L,                /* 膽固醇值下限 */
    PARAM_C_H,                /* 膽固醇值上限 */
    
    /* Triglycerides specific parameters */
    PARAM_TG_L,               /* 三酸甘油脂值下限 */
    PARAM_TG_H,               /* 三酸甘油脂值上限 */
    
    /* Total number of parameters */
    PARAM_COUNT
} Param_ID_TypeDef;

/* Parameter table structure */
typedef struct {
    /* 1. System basic parameters */
    uint8_t LBT;                  /* 低電池閾值 */
    uint8_t OBT;                  /* 電池耗盡閾值 */
    uint8_t FACTORY;              /* 儀器操作模式 */
    uint8_t MODEL_NO;             /* 產品型號 */
    uint8_t FW_NO;                /* 韌體版本號 */
    uint16_t NOT;                 /* 測試次數 */
    uint16_t Code_Table_V;        /* 代碼表版本號 */

    /* 2. Time setting parameters */
    uint8_t YEAR;                 /* 年份設定 (0-99) */
    uint8_t MONTH;                /* 月份設定 (1-12) */
    uint8_t DATE;                 /* 日期設定 (1-31) */
    uint8_t HOUR;                 /* 小時設定 (0-23) */
    uint8_t MINUTE;               /* 分鐘設定 (0-59) */
    uint8_t SECOND;               /* 秒數設定 (0-59) */

    /* 3. Measurement display parameters */
    uint8_t TLL;                  /* 操作溫度範圍下限 */
    uint8_t TLH;                  /* 操作溫度範圍上限 */
    uint8_t MGDL;                 /* 測量單位設定 */
    uint8_t EVENT;                /* 隨餐事件類型 */
    uint8_t Strip_Type;           /* 測試項目 */

    /* 4. Hardware calibration parameters */
    uint16_t EV_T3_TRG;           /* EV_T3觸發電壓 */
    uint8_t EV_WORKING;           /* 測量工作電壓電壓 */
    uint8_t EV_T3;                /* 血液注入電壓電壓 */
    uint8_t DACO;                 /* DAC偏移靈敏 */
    uint8_t DACDO;                /* DAC校正管理參數 */
    uint8_t CC211NoDone;          /* CC211未完成旗標 */
    uint8_t CAL_TOL;              /* OPS/OPI校正容差 */
    float OPS;                    /* OPA校正斜率 */
    float OPI;                    /* OPA校正截距 */
    uint8_t QCT;                  /* QCT校正測試低位元組 */
    int8_t TOFFSET;               /* 溫度校正偏移 */
    int8_t BOFFSET;               /* 電池校正偏移 */

    /* 5. GLV/GAV specific parameters */
    uint8_t BG_CSU_TOL;           /* 試片檢查容差 */
    uint16_t BG_NDL;              /* 新試片電壓水平 */
    uint16_t BG_UDL;              /* 已使用試片電壓水平 */
    uint16_t BG_BLOOD_IN;         /* 血液電壓水平 */
    uint8_t BG_Strip_Lot[16];     /* 試片批號 (16字節) */
    uint8_t BG_L;                 /* 血糖值下限 */
    uint16_t BG_H;                /* 血糖值上限 */
    uint16_t BG_T3_E37;           /* T3 ADV錯誤37閾值 */
    uint16_t BG_TPL_1;            /* 時間脈衝寬度 (第一組) */
    uint16_t BG_TRD_1;            /* 原始數據採集時間 (第一組) */
    uint16_t BG_EV_Width_1;       /* 激勵時間 (第一組) */
    uint16_t BG_TPL_2;            /* 時間脈衝寬度 (第二組) */
    uint16_t BG_TRD_2;            /* 原始數據採集時間 (第二組) */
    uint16_t BG_EV_Width_2;       /* 激勵時間 (第二組) */
    uint8_t BG_Sq;                /* QC靈敏參數Sq */
    uint8_t BG_Iq;                /* QC靈敏參數Iq */
    uint8_t BG_CVq;               /* QC水平的CV */
    uint8_t BG_Aq;                /* QC靈敏參數A */
    uint8_t BG_Bq;                /* QC靈敏參數B */
    uint8_t BG_Aq5;               /* QC靈敏參數A(等級5) */
    uint8_t BG_Bq5;               /* QC靈敏參數B(等級5) */
    uint8_t BG_QC[30];            /* QC模式範圍參數 */
    uint8_t BG_S2;                /* 試片靈敏參數 */
    uint8_t BG_I2;                /* 試片靈敏參數 */
    float BG_Sr;                  /* OPA硬體迴路靈敏(斜率) */
    float BG_Ir;                  /* OPA硬體迴路靈敏(截距) */
    uint8_t BG_S3[30];            /* 試片靈敏參數S3(1-30) */
    uint8_t BG_I3[30];            /* 試片靈敏參數I3(1-30) */
    uint8_t BG_TF[7];             /* AC/PC溫度靈敏斜率(10-40°C) */
    uint8_t BG_CTF[7];            /* QC溫度靈敏斜率(10-40°C) */
    uint8_t BG_TO[7];             /* AC/PC溫度靈敏截距(10-40°C) */
    uint8_t BG_CTO[7];            /* QC溫度靈敏截距(10-40°C) */

    /* 6. U (尿酸) specific parameters */
    uint8_t U_CSU_TOL;            /* 試片檢查容差 */
    uint16_t U_NDL;               /* 新試片電壓水平 */
    uint16_t U_UDL;               /* 已使用試片電壓水平 */
    uint16_t U_BLOOD_IN;          /* 血液電壓水平 */
    uint8_t U_Strip_Lot[16];      /* 試片批號 (16字節) */
    uint8_t U_L;                  /* 尿酸值下限 */
    uint8_t U_H;                  /* 尿酸值上限 */
    uint16_t U_T3_E37;            /* T3 ADV錯誤37閾值 */
    uint16_t U_TPL_1;             /* 時間脈衝寬度 (第一組) */
    uint16_t U_TRD_1;             /* 原始數據採集時間 (第一組) */
    uint16_t U_EV_Width_1;        /* 激勵時間 (第一組) */
    uint16_t U_TPL_2;             /* 時間脈衝寬度 (第二組) */
    uint16_t U_TRD_2;             /* 原始數據採集時間 (第二組) */
    uint16_t U_EV_Width_2;        /* 激勵時間 (第二組) */
    uint8_t U_Sq;                 /* QC靈敏參數Sq */
    uint8_t U_Iq;                 /* QC靈敏參數Iq */
    uint8_t U_CVq;                /* QC水平的CV */
    uint8_t U_Aq;                 /* QC靈敏參數A */
    uint8_t U_Bq;                 /* QC靈敏參數B */
    uint8_t U_QC[30];             /* QC模式範圍參數 */
    uint8_t U_S2;                 /* 試片靈敏參數 */
    uint8_t U_I2;                 /* 試片靈敏參數 */
    float U_Sr;                   /* OPA硬體迴路靈敏(斜率) */
    float U_Ir;                   /* OPA硬體迴路靈敏(截距) */
    uint8_t U_S3[15];             /* 試片靈敏參數S3(1-15) */
    uint8_t U_I3[15];             /* 試片靈敏參數I3(1-15) */
    uint8_t U_TF[7];              /* AC/PC溫度靈敏斜率(10-40°C) */
    uint8_t U_CTF[7];             /* QC溫度靈敏斜率(10-40°C) */
    uint8_t U_TO[7];              /* AC/PC溫度靈敏截距(10-40°C) */
    uint8_t U_CTO[7];             /* QC溫度靈敏截距(10-40°C) */

    /* 7. C (總膽固醇) specific parameters */
    uint8_t C_CSU_TOL;            /* 試片檢查容差 */
    uint16_t C_NDL;               /* 新試片電壓水平 */
    uint16_t C_UDL;               /* 已使用試片電壓水平 */
    uint16_t C_BLOOD_IN;          /* 血液電壓水平 */
    uint8_t C_Strip_Lot[16];      /* 試片批號 (16字節) */
    uint8_t C_L;                  /* 膽固醇值下限 */
    uint16_t C_H;                 /* 膽固醇值上限 */
    uint16_t C_T3_E37;            /* T3 ADV錯誤37閾值 */
    uint16_t C_TPL_1;             /* 時間脈衝寬度 (第一組) */
    uint16_t C_TRD_1;             /* 原始數據採集時間 (第一組) */
    uint16_t C_EV_Width_1;        /* 激勵時間 (第一組) */
    uint16_t C_TPL_2;             /* 時間脈衝寬度 (第二組) */
    uint16_t C_TRD_2;             /* 原始數據採集時間 (第二組) */
    uint16_t C_EV_Width_2;        /* 激勵時間 (第二組) */
    uint8_t C_Sq;                 /* QC靈敏參數Sq */
    uint8_t C_Iq;                 /* QC靈敏參數Iq */
    uint8_t C_CVq;                /* QC水平的CV */
    uint8_t C_Aq;                 /* QC靈敏參數A */
    uint8_t C_Bq;                 /* QC靈敏參數B */
    uint8_t C_QC[30];             /* QC模式範圍參數 */
    uint8_t C_S2;                 /* 試片靈敏參數 */
    uint8_t C_I2;                 /* 試片靈敏參數 */
    float C_Sr;                   /* OPA硬體迴路靈敏(斜率) */
    float C_Ir;                   /* OPA硬體迴路靈敏(截距) */
    uint8_t C_S3[15];             /* 試片靈敏參數S3(1-15) */
    uint8_t C_I3[15];             /* 試片靈敏參數I3(1-15) */
    uint8_t C_TF[7];              /* AC/PC溫度靈敏斜率(10-40°C) */
    uint8_t C_CTF[7];             /* QC溫度靈敏斜率(10-40°C) */
    uint8_t C_TO[7];              /* AC/PC溫度靈敏截距(10-40°C) */
    uint8_t C_CTO[7];             /* QC溫度靈敏截距(10-40°C) */

    /* 8. TG (三酸甘油脂) specific parameters */
    uint8_t TG_CSU_TOL;           /* 試片檢查容差 */
    uint16_t TG_NDL;              /* 新試片電壓水平 */
    uint16_t TG_UDL;              /* 已使用試片電壓水平 */
    uint16_t TG_BLOOD_IN;         /* 血液電壓水平 */
    uint8_t TG_Strip_Lot[16];     /* 試片批號 (16字節) */
    uint8_t TG_L;                 /* 三酸甘油脂值下限 */
    uint16_t TG_H;                /* 三酸甘油脂值上限 */
    uint16_t TG_T3_E37;           /* T3 ADV錯誤37閾值 */
    uint16_t TG_TPL_1;            /* 時間脈衝寬度 (第一組) */
    uint16_t TG_TRD_1;            /* 原始數據採集時間 (第一組) */
    uint16_t TG_EV_Width_1;       /* 激勵時間 (第一組) */
    uint16_t TG_TPL_2;            /* 時間脈衝寬度 (第二組) */
    uint16_t TG_TRD_2;            /* 原始數據採集時間 (第二組) */
    uint16_t TG_EV_Width_2;       /* 激勵時間 (第二組) */
    uint8_t TG_Sq;                /* QC靈敏參數Sq */
    uint8_t TG_Iq;                /* QC靈敏參數Iq */
    uint8_t TG_CVq;               /* QC水平的CV */
    uint8_t TG_Aq;                /* QC靈敏參數A */
    uint8_t TG_Bq;                /* QC靈敏參數B */
    uint8_t TG_QC[30];            /* QC模式範圍參數 */
    uint8_t TG_S2;                /* 試片靈敏參數 */
    uint8_t TG_I2;                /* 試片靈敏參數 */
    float TG_Sr;                  /* OPA硬體迴路靈敏(斜率) */
    float TG_Ir;                  /* OPA硬體迴路靈敏(截距) */
    uint8_t TG_S3[15];            /* 試片靈敏參數S3(1-15) */
    uint8_t TG_I3[15];            /* 試片靈敏參數I3(1-15) */
    uint8_t TG_TF[7];             /* AC/PC溫度靈敏斜率(10-40°C) */
    uint8_t TG_CTF[7];            /* QC溫度靈敏斜率(10-40°C) */
    uint8_t TG_TO[7];             /* AC/PC溫度靈敏截距(10-40°C) */
    uint8_t TG_CTO[7];            /* QC溫度靈敏截距(10-40°C) */

    /* 9. Reserved and checksum */
    uint8_t Reserved[36];         /* 保留區域供未來擴展 */
    uint8_t SUM_L;                /* 地址0~674校驗和(低) */
    uint8_t SUM_H;                /* 地址0~674校驗和(高) */
    uint8_t CRC16;                /* 完整參數表CRC校驗 */
} ParamTable_TypeDef;

/* Function prototypes */
uint8_t Param_Init(void);
uint8_t Param_LoadDefault(void);
uint8_t Param_Save(void);
uint8_t Param_Verify(void);
uint16_t Param_CalculateChecksum(void);
uint8_t Param_CalculateCRC16(void);

/* Parameter utility functions */
int32_t Param_GetValue(Param_ID_TypeDef id);
uint8_t Param_SetValue(Param_ID_TypeDef id, int32_t value);
const char* Param_GetStripTypeName(uint8_t type);
const char* Param_GetEventTypeName(uint8_t event);
const char* Param_GetUnitName(uint8_t unit);

/* External variables */
extern ParamTable_TypeDef gParamTable;

/*
 * Test function for parameter table operations
 */
void ParamTable_Test(void);

#endif /* __PARAM_TABLE_H */ 