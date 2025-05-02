/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_parameter.h
 * Author             : WCH & HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化儀Flash參數儲存方案
*********************************************************************************
* Copyright (c) 2025 HMD Co., Ltd.
* All rights reserved.
*******************************************************************************/

#ifndef __FLASH_PARAMETER_H
#define __FLASH_PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include "ch32v20x_flash.h"
#include <stdbool.h>
#include <string.h>

/* 參數區域分配 */
#define PARAM_BLOCK_A_ADDR       ((uint32_t)0x0800D800)  // 參數區A的起始地址
#define PARAM_BLOCK_B_ADDR       ((uint32_t)0x0800E000)  // 參數區B的起始地址
#define RECORD_AREA_ADDR         ((uint32_t)0x0800E800)  // 測試記錄區的起始地址
#define PARAM_BLOCK_SIZE         ((uint32_t)0x0800)      // 參數區大小 (2KB)
#define RECORD_AREA_SIZE         ((uint32_t)0x1800)      // 測試記錄區大小 (6KB)

/* 參數塊標識符 */
#define PARAM_MAGIC_NUMBER       ((uint16_t)0xA55A)      // 參數塊魔數
#define PARAM_VERSION            ((uint8_t)0x01)         // 參數版本

/* 參數區塊類型定義 */
#define BLOCK_BASIC_SYSTEM       ((uint8_t)0x00)         // 基本系統參數
#define BLOCK_HARDWARE_CALIB     ((uint8_t)0x01)         // 硬體校準參數
#define BLOCK_GLUCOSE            ((uint8_t)0x02)         // 血糖(GLV/GAV)參數
#define BLOCK_URIC_ACID          ((uint8_t)0x03)         // 尿酸(U)參數
#define BLOCK_CHOLESTEROL        ((uint8_t)0x04)         // 總膽固醇(C)參數
#define BLOCK_TRIGLYCERIDE       ((uint8_t)0x05)         // 三酸甘油脂(TG)參數
#define BLOCK_RESERVED           ((uint8_t)0x06)         // 保留參數
#define BLOCK_CHECKSUM           ((uint8_t)0x07)         // 校驗參數

/* 參數塊頭部結構 */
typedef struct {
    uint16_t magic;              // 魔數 (0xA55A)
    uint8_t  version;            // 版本號
    uint16_t writeCounter;       // 寫入次數計數器
    uint32_t timestamp;          // 時間戳
    uint16_t checksum;           // 校驗和
} ParamBlockHeader;

/* 基本系統參數結構 */
typedef struct {
    uint8_t  lbt;                // 低電池閾值 (單位: 0.01V)
    uint8_t  obt;                // 工作電池閾值 (單位: 0.01V)
    uint8_t  factory;            // 工廠模式標誌 (0=使用者模式, 1=工廠模式)
    uint8_t  modelNo;            // 產品型號
    uint8_t  fwNo;               // 韌體版本號
    uint32_t testCount;          // 測試次數累計
    uint8_t  codeTableVer;       // 參數表版本號
    uint8_t  year;               // 年 (0-99)
    uint8_t  month;              // 月 (1-12)
    uint8_t  date;               // 日 (1-31)
    uint8_t  hour;               // 時 (0-23)
    uint8_t  minute;             // 分 (0-59)
    uint8_t  second;             // 秒 (0-59)
    uint8_t  tempLowLimit;       // 溫度下限 (單位: °C)
    uint8_t  tempHighLimit;      // 溫度上限 (單位: °C)
    uint8_t  measureUnit;        // 測量單位 (0=mmol/L, 1=mg/dL)
    uint8_t  defaultEvent;       // 默認事件 (0=GP, 1=AC, 2=PC)
    uint8_t  stripType;          // 試片類型 (0=GLV, 1=U, 2=C, 3=TG, 4=GAV)
    uint8_t  reserved[2];        // 保留
} BasicSystemBlock;

/* 硬體校準參數結構 */
typedef struct {
    uint16_t evT3Trigger;        // EV電壓閾值
    uint16_t evWorking;          // EV工作電壓
    uint16_t evT3;               // EV T3電壓
    int8_t   dacoOffset;         // DACO偏移
    int8_t   dacdo;              // DACDO偏移
    uint8_t  cc211Status;        // CC211狀態
    uint8_t  calTolerance;       // 校準容差 (單位: 0.1%)
    float    ops;                // 運算放大器校準斜率
    float    opi;                // 運算放大器校準截距
    uint8_t  qct;                // 品管測試結果
    int8_t   tempOffset;         // 溫度偏移補償 (單位: 0.1°C)
    int8_t   batteryOffset;      // 電池偏移補償 (單位: 10mV)
    uint8_t  reserved[3];        // 保留
} HardwareCalibBlock;

/* 血糖參數結構 */
typedef struct {
    uint8_t  bgCsuTolerance;     // 血糖CSU容差 (單位: 0.1%)
    uint16_t bgNdl;              // 新試片檢測水平
    uint16_t bgUdl;              // 已使用試片檢測水平
    uint16_t bgBloodIn;          // 血液檢測水平
    char     bgStripLot[16];     // 試片批號
    uint8_t  bgL;                // 低血糖顯示閾值 (單位: 10 mg/dL)
    uint8_t  bgH;                // 高血糖顯示閾值 (單位: 10 mg/dL)
    uint16_t bgT3E37;            // T3 ADV錯誤閾值

    // 測試時序參數 (組1)
    uint16_t bgTPL1;             // 時序TPL1 (單位: 0.1秒)
    uint16_t bgTRD1;             // 時序TRD1 (單位: 0.1秒)
    uint16_t bgEVWidth1;         // 時序EVWidth1 (單位: 0.1秒)

    // 測試時序參數 (組2)
    uint16_t bgTPL2;             // 時序TPL2 (單位: 0.1秒)
    uint16_t bgTRD2;             // 時序TRD2 (單位: 0.1秒)
    uint16_t bgEVWidth2;         // 時序EVWidth2 (單位: 0.1秒)

    // 溫度補償係數
    float    bgTempA;            // 溫度補償係數A
    float    bgTempB;            // 溫度補償係數B
    float    bgTempC;            // 溫度補償係數C

    // 海拔補償係數
    float    bgAltA;             // 海拔補償係數A
    float    bgAltB;             // 海拔補償係數B
    float    bgAltC;             // 海拔補償係數C

    // 血比容補償係數
    float    bgHctA;             // 血比容補償係數A
    float    bgHctB;             // 血比容補償係數B
    float    bgHctC;             // 血比容補償係數C

    // 補償截距和比例係數
    float    bgCompS;            // 補償比例係數
    float    bgCompI;            // 補償截距係數

    // 保留
    uint8_t  reserved[10];       // 保留
} GlucoseParamBlock;

/* 尿酸參數結構 */
typedef struct {
    uint8_t  uaCsuTolerance;     // 尿酸CSU容差 (單位: 0.1%)
    uint16_t uaNdl;              // 新試片檢測水平
    uint16_t uaUdl;              // 已使用試片檢測水平
    uint16_t uaBloodIn;          // 血液檢測水平
    char     uaStripLot[16];     // 試片批號
    uint8_t  uaL;                // 低尿酸顯示閾值 (單位: 0.1 mg/dL)
    uint8_t  uaH;                // 高尿酸顯示閾值 (單位: 0.1 mg/dL)
    uint16_t uaT3E37;            // T3 ADV錯誤閾值

    // 測試時序參數 (組1)
    uint16_t uaTPL1;             // 時序TPL1 (單位: 0.1秒)
    uint16_t uaTRD1;             // 時序TRD1 (單位: 0.1秒)
    uint16_t uaEVWidth1;         // 時序EVWidth1 (單位: 0.1秒)

    // 測試時序參數 (組2)
    uint16_t uaTPL2;             // 時序TPL2 (單位: 0.1秒)
    uint16_t uaTRD2;             // 時序TRD2 (單位: 0.1秒)
    uint16_t uaEVWidth2;         // 時序EVWidth2 (單位: 0.1秒)

    // 溫度補償係數
    float    uaTempA;            // 溫度補償係數A
    float    uaTempB;            // 溫度補償係數B
    float    uaTempC;            // 溫度補償係數C

    // 海拔補償係數
    float    uaAltA;             // 海拔補償係數A
    float    uaAltB;             // 海拔補償係數B
    float    uaAltC;             // 海拔補償係數C

    // 補償截距和比例係數
    float    uaCompS;            // 補償比例係數
    float    uaCompI;            // 補償截距係數

    // 保留
    uint8_t  reserved[10];       // 保留
} UricAcidParamBlock;

/* 總膽固醇參數結構 */
typedef struct {
    uint8_t  chCsuTolerance;     // 膽固醇CSU容差 (單位: 0.1%)
    uint16_t chNdl;              // 新試片檢測水平
    uint16_t chUdl;              // 已使用試片檢測水平
    uint16_t chBloodIn;          // 血液檢測水平
    char     chStripLot[16];     // 試片批號
    uint8_t  chL;                // 低膽固醇顯示閾值 (單位: mg/dL)
    uint16_t chH;                // 高膽固醇顯示閾值 (單位: mg/dL)
    uint16_t chT3E37;            // T3 ADV錯誤閾值

    // 測試時序參數 (組1)
    uint16_t chTPL1;             // 時序TPL1 (單位: 0.1秒)
    uint16_t chTRD1;             // 時序TRD1 (單位: 0.1秒)
    uint16_t chEVWidth1;         // 時序EVWidth1 (單位: 0.1秒)

    // 測試時序參數 (組2)
    uint16_t chTPL2;             // 時序TPL2 (單位: 0.1秒)
    uint16_t chTRD2;             // 時序TRD2 (單位: 0.1秒)
    uint16_t chEVWidth2;         // 時序EVWidth2 (單位: 0.1秒)

    // 溫度補償係數
    float    chTempA;            // 溫度補償係數A
    float    chTempB;            // 溫度補償係數B
    float    chTempC;            // 溫度補償係數C

    // 補償截距和比例係數
    float    chCompS;            // 補償比例係數
    float    chCompI;            // 補償截距係數

    // 保留
    uint8_t  reserved[20];       // 保留
} CholesterolParamBlock;

/* 三酸甘油脂參數結構 */
typedef struct {
    uint8_t  tgCsuTolerance;     // 三酸甘油脂CSU容差 (單位: 0.1%)
    uint16_t tgNdl;              // 新試片檢測水平
    uint16_t tgUdl;              // 已使用試片檢測水平
    uint16_t tgBloodIn;          // 血液檢測水平
    char     tgStripLot[16];     // 試片批號
    uint8_t  tgL;                // 低三酸甘油脂顯示閾值 (單位: mg/dL)
    uint16_t tgH;                // 高三酸甘油脂顯示閾值 (單位: mg/dL)
    uint16_t tgT3E37;            // T3 ADV錯誤閾值

    // 測試時序參數 (組1)
    uint16_t tgTPL1;             // 時序TPL1 (單位: 0.1秒)
    uint16_t tgTRD1;             // 時序TRD1 (單位: 0.1秒)
    uint16_t tgEVWidth1;         // 時序EVWidth1 (單位: 0.1秒)

    // 測試時序參數 (組2)
    uint16_t tgTPL2;             // 時序TPL2 (單位: 0.1秒)
    uint16_t tgTRD2;             // 時序TRD2 (單位: 0.1秒)
    uint16_t tgEVWidth2;         // 時序EVWidth2 (單位: 0.1秒)

    // 溫度補償係數
    float    tgTempA;            // 溫度補償係數A
    float    tgTempB;            // 溫度補償係數B
    float    tgTempC;            // 溫度補償係數C

    // 補償截距和比例係數
    float    tgCompS;            // 補償比例係數
    float    tgCompI;            // 補償截距係數

    // 保留
    uint8_t  reserved[20];       // 保留
} TriglycerideParamBlock;

/* 保留和校驗參數結構 */
typedef struct {
    uint8_t  reserved[39];       // 保留
} ReservedBlock;

/* 完整參數數據結構 */
typedef struct {
    BasicSystemBlock     basicSystem;    // 基本系統參數
    HardwareCalibBlock   hardwareCalib;  // 硬體校準參數
    GlucoseParamBlock    bgParams;       // 血糖參數
    UricAcidParamBlock   uaParams;       // 尿酸參數
    CholesterolParamBlock chParams;      // 總膽固醇參數
    TriglycerideParamBlock tgParams;     // 三酸甘油脂參數
    ReservedBlock         reserved;      // 保留參數
} ParameterData;

/* 完整參數塊結構 */
typedef struct {
    ParamBlockHeader header;      // 參數塊頭部
    ParameterData    params;      // 參數數據
} ParameterBlock;

/* 測試記錄結構 */
typedef struct {
    uint16_t resultStatus;        // 測試結果狀態 (0=成功，其他=錯誤)
    uint16_t resultValue;         // 測試結果值
    uint16_t testType;            // 測試項目類型
    uint16_t event;               // 事件類型
    uint8_t  code;                // 試片批次碼
    uint16_t year;                // 年
    uint16_t month;               // 月
    uint16_t date;                // 日
    uint16_t hour;                // 時
    uint16_t minute;              // 分
    uint16_t second;              // 秒
    uint16_t batteryVoltage;      // 電池電壓
    uint16_t temperature;         // 環境溫度
} TestRecord;

/* 函數聲明 */
void PARAM_Init(void);
bool PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, void *data, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
uint32_t PARAM_GetTestCount(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_ResetToDefault(void);
bool PARAM_SaveTestRecord(TestRecord *record);
bool PARAM_GetTestRecord(uint32_t index, TestRecord *record);
uint32_t PARAM_GetTestRecordCount(void);
bool PARAM_EraseAllTestRecords(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_PARAMETER_H */