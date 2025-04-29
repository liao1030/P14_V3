/************************************* (C) COPYRIGHT *******************************
 * File Name          : param_store.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter storage module for CH32V203G8R6 Flash.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_STORE_H
#define __PARAM_STORE_H

#include "ch32v20x.h"

/* Flash memory mapping */
#define PARAM_BLOCK_A_ADDR      0x0800D800    /* 參數區A起始地址 */
#define PARAM_BLOCK_B_ADDR      0x0800E000    /* 參數區B起始地址 */
#define PARAM_RECORD_ADDR       0x0800E800    /* 測試記錄區起始地址 */
#define FLASH_PAGE_SIZE         0x400         /* 1KB每頁 */

/* 參數區塊標頭結構 */
typedef struct {
    uint16_t magic;         /* 區塊魔術碼 0xA55A */
    uint16_t version;       /* 參數表版本號 */
    uint16_t writeCounter;  /* 寫入次數計數器 */
    uint32_t timestamp;     /* 時間戳記 */
} ParamBlockHeader_TypeDef;

/* 參數區塊類型定義 */
typedef enum {
    BLOCK_BASIC_SYSTEM = 0,     /* 基本系統參數區 */
    BLOCK_HARDWARE_CALIB,       /* 硬體校準參數區 */
    BLOCK_BG_PARAMS,            /* 血糖(GLV/GAV)參數區 */
    BLOCK_UA_PARAMS,            /* 尿酸(U)參數區 */
    BLOCK_CHOL_PARAMS,          /* 總膽固醇(C)參數區 */
    BLOCK_TG_PARAMS,            /* 三酸甘油脂(TG)參數區 */
    BLOCK_RESERVED,             /* 保留區域和校驗區 */
    BLOCK_COUNT                 /* 區塊總數 */
} ParamBlockType;

/* 基本系統參數區塊 */
typedef struct {
    uint8_t lbt;                /* 低電池閾值 */
    uint8_t obt;                /* 電池耗盡閾值 */
    uint8_t factory;            /* 儀器操作模式 */
    uint8_t modelNo;            /* 產品型號 */
    uint8_t fwNo;               /* 韌體版本號 */
    uint16_t testCount;         /* 測試次數 */
    uint16_t codeTableVer;      /* 代碼表版本號 */
    uint8_t year;               /* 年份設定 (0-99) */
    uint8_t month;              /* 月份設定 (1-12) */
    uint8_t date;               /* 日期設定 (1-31) */
    uint8_t hour;               /* 小時設定 (0-23) */
    uint8_t minute;             /* 分鐘設定 (0-59) */
    uint8_t second;             /* 秒數設定 (0-59) */
    uint8_t tempLowLimit;       /* 操作溫度下限 */
    uint8_t tempHighLimit;      /* 操作溫度上限 */
    uint8_t measureUnit;        /* 測量單位設定 */
    uint8_t defaultEvent;       /* 預設事件類型 */
    uint8_t stripType;          /* 測試項目類型 */
    uint8_t reserved[2];        /* 保留 */
} BasicSystemBlock;

/* 硬體校準參數區塊 */
typedef struct {
    uint16_t evT3Trigger;       /* EV_T3觸發電壓 */
    uint8_t evWorking;          /* 測量工作電壓 */
    uint8_t evT3;               /* 血液注入電壓 */
    uint8_t dacoOffset;         /* DAC偏移靈敏 */
    uint8_t dacdo;              /* DAC校正參數 */
    uint8_t cc211Status;        /* CC211狀態 */
    uint8_t calTolerance;       /* 校正容差 */
    float ops;                  /* OPA校正斜率 */
    float opi;                  /* OPA校正截距 */
    uint8_t qct;                /* QCT校正測試 */
    int8_t tempOffset;          /* 溫度校正偏移 */
    int8_t batteryOffset;       /* 電池校正偏移 */
    uint8_t reserved[3];        /* 保留 */
} HardwareCalibBlock;

/* 基本測試參數區塊結構 */
typedef struct {
    uint8_t csuTolerance;       /* 試片檢查容差 */
    uint16_t ndl;               /* 新試片電壓水平 */
    uint16_t udl;               /* 已使用試片電壓水平 */
    uint16_t bloodIn;           /* 血液電壓水平 */
    uint8_t stripLot[16];       /* 試片批號 */
    uint8_t lowLimit;           /* 測量下限 */
    uint16_t highLimit;         /* 測量上限 */
    uint16_t t3E37;             /* T3錯誤37閾值 */
    uint16_t tpl1;              /* 時間脈衝寬度(第一組) */
    uint16_t trd1;              /* 原始數據採集時間(第一組) */
    uint16_t evWidth1;          /* 激勵時間(第一組) */
    uint16_t tpl2;              /* 時間脈衝寬度(第二組) */
    uint16_t trd2;              /* 原始數據採集時間(第二組) */
    uint16_t evWidth2;          /* 激勵時間(第二組) */
    uint8_t sq;                 /* QC參數Sq */
    uint8_t iq;                 /* QC參數Iq */
    uint8_t cvq;                /* QC水平CV */
    uint8_t aq;                 /* QC參數A */
    uint8_t bq;                 /* QC參數B */
    uint8_t reserved[2];        /* 保留 */
} TestParamBaseBlock;

/* 功能型函數原型 */
uint8_t PARAM_Init(void);
uint8_t PARAM_ReadBlock(ParamBlockType blockType, void *buffer, uint16_t size);
uint8_t PARAM_UpdateBlock(ParamBlockType blockType, const void *buffer, uint16_t size);
uint8_t PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
uint8_t PARAM_IncreaseTestCount(void);
uint8_t PARAM_ResetToDefault(void);
uint8_t PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
uint8_t PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
uint8_t PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t seqNum);
uint32_t PARAM_GetActiveBlockAddr(void);

#endif /* __PARAM_STORE_H */ 