/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數表頭文件
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include <stdint.h>

/* Flash參數區域定義 */
#define PARAM_AREA_A_ADDR          ((uint32_t)0x0800D000)   // 參數表主要儲存區域地址
#define PARAM_AREA_B_ADDR          ((uint32_t)0x0800E000)   // 參數表備份區域地址
#define TEST_RECORD_AREA_ADDR      ((uint32_t)0x0800F000)   // 測試記錄區域地址

#define PARAM_AREA_SIZE            ((uint32_t)0x1000)       // 參數區大小 (4KB)
#define FLASH_PAGE_SIZE            ((uint32_t)0x400)        // Flash頁大小 (1KB)

/* 魔術數字和版本定義 */
#define PARAM_MAGIC                ((uint16_t)0xA55A)       // 參數區魔術數字
#define PARAM_VERSION              ((uint16_t)0x0001)       // 參數表版本號

/* 參數表錯誤碼 */
typedef enum {
    PARAM_OK = 0,                  // 操作成功
    PARAM_ERR_INVALID_AREA,        // 無效的參數區
    PARAM_ERR_FLASH_WRITE,         // Flash寫入錯誤
    PARAM_ERR_CRC,                 // CRC校驗錯誤
    PARAM_ERR_INVALID_PARAM,       // 無效參數
    PARAM_ERR_NOT_INITIALIZED      // 參數未初始化
} ParamError_TypeDef;

/* 參數區域標識 */
typedef enum {
    PARAM_AREA_MAIN = 0,           // 主參數區
    PARAM_AREA_BACKUP              // 備份參數區
} ParamArea_TypeDef;

/* 測試項目類型 */
typedef enum {
    STRIP_TYPE_GLV = 0,            // 血糖 (GLV)
    STRIP_TYPE_U = 1,              // 尿酸 (U)
    STRIP_TYPE_C = 2,              // 總膽固醇 (C)
    STRIP_TYPE_TG = 3,             // 三酸甘油脂 (TG) 
    STRIP_TYPE_GAV = 4,            // 血糖 (GAV)
    STRIP_TYPE_MAX                 // 最大試片類型數（無效類型）
} StripType_TypeDef;

#pragma pack(1)  // 確保結構體緊密封裝

/* 參數表頭部結構 */
typedef struct {
    uint16_t magic;                // 魔術數字 (0xA55A)，表示有效的參數區
    uint16_t version;              // 參數表版本號，對應 Code_Table_V
    uint32_t writeCounter;         // 寫入次數計數器
    uint32_t timestamp;            // 最後更新時間戳記
} ParameterHeader;

/* 基本系統參數區塊 */
typedef struct {
    // 系統基本參數 (9個參數)
    uint8_t lbt;                   // 低電池閾值
    uint8_t obt;                   // 電池耗盡閾值
    uint8_t factory;               // 儀器操作模式
    uint8_t modelNo;               // 產品型號
    uint8_t fwNo;                  // 韌體版本號
    uint16_t testCount;            // 測試次數
    uint16_t codeTableVer;         // 代碼表版本
    
    // 時間設定參數 (6個參數)
    uint8_t year;                  // 年份設定 (0-99 for 2000-2099)
    uint8_t month;                 // 月份設定 (1-12)
    uint8_t date;                  // 日期設定 (1-31)
    uint8_t hour;                  // 小時設定 (0-23)
    uint8_t minute;                // 分鐘設定 (0-59)
    uint8_t second;                // 秒數設定 (0-59)
    
    // 測試環境參數 (5個參數)
    uint8_t tempLowLimit;          // 操作溫度下限
    uint8_t tempHighLimit;         // 操作溫度上限
    uint8_t measureUnit;           // 測量單位設定
    uint8_t defaultEvent;          // 預設事件
    uint8_t stripType;             // 測試項目類型
} BasicSystemBlock;

/* 硬體校準參數區塊 */
typedef struct {
    uint16_t evT3Trigger;          // EV_T3觸發電壓
    uint8_t evWorking;             // 測量工作電極電壓
    uint8_t evT3;                  // 血液檢測電極電壓
    uint8_t calTolerance;          // OPS/OPI校準容差
    float ops;                     // OPA校準斜率
    float opi;                     // OPA校準截距
    uint8_t qct;                   // QCT校準測試低位元組
    int8_t tempOffset;             // 溫度校準偏移
    int8_t batteryOffset;          // 電池校準偏移
} HardwareCalibBlock;

/* 血糖(GLV)專用參數區塊 */
typedef struct {
    // 試片參數 (12個參數)
    uint8_t glvCsuTolerance;       // 試片檢查容差
    uint16_t glvNdl;               // 新試片檢測水平
    uint16_t glvUdl;               // 已使用試片檢測水平
    uint16_t glvBloodIn;           // 血液檢測水平
    uint16_t glvEvWBackground;     // W電極背景值
    uint16_t glvEvWPwmDuty;        // W電極PWM占空比
    uint8_t glvCountDownTime;      // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t glvL;                  // 血糖值下限
    uint8_t glvH;                  // 血糖值上限
    
    // 測試時序參數 (12個參數)
    uint16_t glvTPL1;              // 時間脈衝低 (第一組)
    uint16_t glvTRD1;              // 原始數據時間 (第一組)
    uint16_t glvEVWidth1;          // 燃燒時間 (第一組)
    uint16_t glvTPL2;              // 時間脈衝低 (第二組)
    uint16_t glvTRD2;              // 原始數據時間 (第二組)
    uint16_t glvEVWidth2;          // 燃燒時間 (第二組)
    
    // 計算式補償參數 (110個參數)
    uint8_t glvS2;                 // 試片補償參數
    uint8_t glvI2;                 // 試片補償參數
    uint8_t glvSq;                 // 品管液QC補償參數Sq
    uint8_t glvIq;                 // 品管液QC補償參數Iq
    float glvSr;                   // OPA硬體迴路補償(斜率)
    float glvIr;                   // OPA硬體迴路補償(截距)
    uint16_t glvGLU[7];            // 多段校正參數(量測分段點)
    uint16_t glvGOAL[7];           // 多段校正參數(目標值分段點)
    uint8_t glvS3[30];             // 試片補償參數S3(1-30)
    uint8_t glvI3[30];             // 試片補償參數I3(1-30)
    
    // 溫度補償參數 (28個參數)
    uint8_t glvTF[7];              // AC/PC溫度補償斜率(10-40°C)
    uint8_t glvCTF[7];             // QC溫度補償斜率(10-40°C)
    uint8_t glvTO[7];              // AC/PC溫度補償截距(10-40°C)
    uint8_t glvCTO[7];             // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t glvCVq;                // CV Level of QC
    uint8_t glvAq;                 // A of QC Compensation
    uint8_t glvBq;                 // B of QC Compensation
    uint8_t glvtCV;                // Time of BG Optimize
    uint8_t glvCVbg;               // CV Level of BG
    uint8_t glvAbg;                // A of BG Compensation
    uint8_t glvBbg;                // B of BG Compensation
    uint8_t glvAq5;                // A of QC Compensation Level 5
    uint8_t glvBq5;                // B of QC Compensation Level 5
} GLVParameterBlock;

/* 尿酸(U)專用參數區塊 */
typedef struct {
    // 試片參數 (12個參數)
    uint8_t uCsuTolerance;         // 試片檢查容差
    uint16_t uNdl;                 // 新試片檢測水平
    uint16_t uUdl;                 // 已使用試片檢測水平
    uint16_t uBloodIn;             // 血液檢測水平
    uint16_t uEvWBackground;       // W電極背景值
    uint16_t uEvWPwmDuty;          // W電極PWM占空比
    uint8_t uCountDownTime;        // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t uL;                    // 尿酸值下限
    uint8_t uH;                    // 尿酸值上限
    
    // 測試時序參數 (12個參數)
    uint16_t uTPL1;                // 時間脈衝低 (第一組)
    uint16_t uTRD1;                // 原始數據時間 (第一組)
    uint16_t uEVWidth1;            // 燃燒時間 (第一組)
    uint16_t uTPL2;                // 時間脈衝低 (第二組)
    uint16_t uTRD2;                // 原始數據時間 (第二組)
    uint16_t uEVWidth2;            // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t uS2;                   // 試片補償參數
    uint8_t uI2;                   // 試片補償參數
    uint8_t uSq;                   // 品管液QC補償參數Sq
    uint8_t uIq;                   // 品管液QC補償參數Iq
    float uSr;                     // OPA硬體迴路補償(斜率)
    float uIr;                     // OPA硬體迴路補償(截距)
    uint16_t uGLU[7];              // 多段校正參數(量測分段點)
    uint16_t uGOAL[7];             // 多段校正參數(目標值分段點)
    uint8_t uS3[15];               // 試片補償參數S3(1-15)
    uint8_t uI3[15];               // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t uTF[7];                // AC/PC溫度補償斜率(10-40°C)
    uint8_t uCTF[7];               // QC溫度補償斜率(10-40°C)
    uint8_t uTO[7];                // AC/PC溫度補償截距(10-40°C)
    uint8_t uCTO[7];               // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t uCVq;                  // CV Level of QC
    uint8_t uAq;                   // A of QC Compensation
    uint8_t uBq;                   // B of QC Compensation
    uint8_t utCV;                  // Time of BG Optimize
    uint8_t uCVbg;                 // CV Level of BG
    uint8_t uAbg;                  // A of BG Compensation
    uint8_t uBbg;                  // B of BG Compensation
    uint8_t uAq5;                  // A of QC Compensation Level 5
    uint8_t uBq5;                  // B of QC Compensation Level 5
} UParameterBlock;

/* 總膽固醇(C)專用參數區塊 */
typedef struct {
    // 試片參數 (12個參數)
    uint8_t cCsuTolerance;         // 試片檢查容差
    uint16_t cNdl;                 // 新試片檢測水平
    uint16_t cUdl;                 // 已使用試片檢測水平
    uint16_t cBloodIn;             // 血液檢測水平
    uint16_t cEvWBackground;       // W電極背景值
    uint16_t cEvWPwmDuty;          // W電極PWM占空比
    uint8_t cCountDownTime;        // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t cL;                    // 膽固醇值下限
    uint8_t cH;                    // 膽固醇值上限
    
    // 測試時序參數 (12個參數)
    uint16_t cTPL1;                // 時間脈衝低 (第一組)
    uint16_t cTRD1;                // 原始數據時間 (第一組)
    uint16_t cEVWidth1;            // 燃燒時間 (第一組)
    uint16_t cTPL2;                // 時間脈衝低 (第二組)
    uint16_t cTRD2;                // 原始數據時間 (第二組)
    uint16_t cEVWidth2;            // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t cS2;                   // 試片補償參數
    uint8_t cI2;                   // 試片補償參數
    uint8_t cSq;                   // 品管液QC補償參數Sq
    uint8_t cIq;                   // 品管液QC補償參數Iq
    float cSr;                     // OPA硬體迴路補償(斜率)
    float cIr;                     // OPA硬體迴路補償(截距)
    uint16_t cGLU[7];              // 多段校正參數(量測分段點)
    uint16_t cGOAL[7];             // 多段校正參數(目標值分段點)
    uint8_t cS3[15];               // 試片補償參數S3(1-15)
    uint8_t cI3[15];               // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t cTF[7];                // AC/PC溫度補償斜率(10-40°C)
    uint8_t cCTF[7];               // QC溫度補償斜率(10-40°C)
    uint8_t cTO[7];                // AC/PC溫度補償截距(10-40°C)
    uint8_t cCTO[7];               // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t cCVq;                  // CV Level of QC
    uint8_t cAq;                   // A of QC Compensation
    uint8_t cBq;                   // B of QC Compensation
    uint8_t ctCV;                  // Time of BG Optimize
    uint8_t cCVbg;                 // CV Level of BG
    uint8_t cAbg;                  // A of BG Compensation
    uint8_t cBbg;                  // B of BG Compensation
    uint8_t cAq5;                  // A of QC Compensation Level 5
    uint8_t cBq5;                  // B of QC Compensation Level 5
} CParameterBlock;

/* 三酸甘油脂(TG)專用參數區塊 */
typedef struct {
    // 試片參數 (12個參數)
    uint8_t tgCsuTolerance;        // 試片檢查容差
    uint16_t tgNdl;                // 新試片檢測水平
    uint16_t tgUdl;                // 已使用試片檢測水平
    uint16_t tgBloodIn;            // 血液檢測水平
    uint16_t tgEvWBackground;      // W電極背景值
    uint16_t tgEvWPwmDuty;         // W電極PWM占空比
    uint8_t tgCountDownTime;       // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t tgL;                   // 三酸甘油脂值下限
    uint16_t tgH;                  // 三酸甘油脂值上限
    
    // 測試時序參數 (12個參數)
    uint16_t tgTPL1;               // 時間脈衝低 (第一組)
    uint16_t tgTRD1;               // 原始數據時間 (第一組)
    uint16_t tgEVWidth1;           // 燃燒時間 (第一組)
    uint16_t tgTPL2;               // 時間脈衝低 (第二組)
    uint16_t tgTRD2;               // 原始數據時間 (第二組)
    uint16_t tgEVWidth2;           // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t tgS2;                  // 試片補償參數
    uint8_t tgI2;                  // 試片補償參數
    uint8_t tgSq;                  // 品管液QC補償參數Sq
    uint8_t tgIq;                  // 品管液QC補償參數Iq
    float tgSr;                    // OPA硬體迴路補償(斜率)
    float tgIr;                    // OPA硬體迴路補償(截距)
    uint16_t tgGLU[7];             // 多段校正參數(量測分段點)
    uint16_t tgGOAL[7];            // 多段校正參數(目標值分段點)
    uint8_t tgS3[15];              // 試片補償參數S3(1-15)
    uint8_t tgI3[15];              // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t tgTF[7];               // AC/PC溫度補償斜率(10-40°C)
    uint8_t tgCTF[7];              // QC溫度補償斜率(10-40°C)
    uint8_t tgTO[7];               // AC/PC溫度補償截距(10-40°C)
    uint8_t tgCTO[7];              // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t tgCVq;                 // CV Level of QC
    uint8_t tgAq;                  // A of QC Compensation
    uint8_t tgBq;                  // B of QC Compensation
    uint8_t tgtCV;                 // Time of BG Optimize
    uint8_t tgCVbg;                // CV Level of BG
    uint8_t tgAbg;                 // A of BG Compensation
    uint8_t tgBbg;                 // B of BG Compensation
    uint8_t tgAq5;                 // A of QC Compensation Level 5
    uint8_t tgBq5;                 // B of QC Compensation Level 5
} TGParameterBlock;

/* 血糖(GAV)專用參數區塊 */
typedef struct {
    // 試片參數 (12個參數)
    uint8_t gavCsuTolerance;       // 試片檢查容差
    uint16_t gavNdl;               // 新試片檢測水平
    uint16_t gavUdl;               // 已使用試片檢測水平
    uint16_t gavBloodIn;           // 血液檢測水平
    uint16_t gavEvWBackground;     // W電極背景值
    uint16_t gavEvWPwmDuty;        // W電極PWM占空比
    uint8_t gavCountDownTime;      // 倒數時間
    
    // 測量範圍參數 (3個參數)
    uint8_t gavL;                  // 血糖值下限
    uint16_t gavH;                 // 血糖值上限
    uint16_t gavT3E37;             // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t gavTPL1;              // 時間脈衝低 (第一組)
    uint16_t gavTRD1;              // 原始數據時間 (第一組)
    uint16_t gavEVWidth1;          // 燃燒時間 (第一組)
    uint16_t gavTPL2;              // 時間脈衝低 (第二組)
    uint16_t gavTRD2;              // 原始數據時間 (第二組)
    uint16_t gavEVWidth2;          // 燃燒時間 (第二組)
    
    // 計算式補償參數 (110個參數)
    uint8_t gavS2;                 // 試片補償參數
    uint8_t gavI2;                 // 試片補償參數
    uint8_t gavSq;                 // 品管液QC補償參數Sq
    uint8_t gavIq;                 // 品管液QC補償參數Iq
    float gavSr;                   // OPA硬體迴路補償(斜率)
    float gavIr;                   // OPA硬體迴路補償(截距)
    uint16_t gavGLU[7];            // 多段校正參數(量測分段點)
    uint16_t gavGOAL[7];           // 多段校正參數(目標值分段點)
    uint8_t gavS3[30];             // 試片補償參數S3(1-30)
    uint8_t gavI3[30];             // 試片補償參數I3(1-30)
    
    // AC補償參數 (28個參數)
    uint16_t gavAcLWaveL;          // AC振幅判斷(低)
    uint16_t gavAcHWaveL;          // AC振幅判斷(高)
    uint8_t gavS4[7];              // AC補償參數S4(HCT10-HCT70)
    uint8_t gavI4[7];              // AC補償參數I4(HCT10-HCT70)
    uint16_t gavAcAdc[7];          // AC補償判定值(HCT10-HCT70)
    
    // 溫度補償參數 (42個參數)
    uint8_t gavTF[7];              // AC/PC溫度補償斜率(10-40°C)
    uint8_t gavCTF[7];             // QC溫度補償斜率(10-40°C)
    uint8_t gavTO[7];              // AC/PC溫度補償截距(10-40°C)
    uint8_t gavCTO[7];             // QC溫度補償截距(10-40°C)
    uint8_t gavAcTF[7];            // AC(交流)溫度補償斜率(10-40°C)
    uint8_t gavAcTO[7];            // AC(交流)溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t gavCVq;                // CV Level of QC
    uint8_t gavAq;                 // A of QC Compensation
    uint8_t gavBq;                 // B of QC Compensation
    uint8_t gavtCV;                // Time of BG Optimize
    uint8_t gavCVbg;               // CV Level of BG
    uint8_t gavAbg;                // A of BG Compensation
    uint8_t gavBbg;                // B of BG Compensation
    uint8_t gavAq5;                // A of QC Compensation Level 5
    uint8_t gavBq5;                // B of QC Compensation Level 5
} GAVParameterBlock;

/* 完整參數表結構 */
typedef struct {
    ParameterHeader header;         // 參數表頭部
    BasicSystemBlock basic;         // 基本系統參數
    HardwareCalibBlock hardware;    // 硬體校準參數
    GLVParameterBlock glv;          // 血糖(GLV)參數
    UParameterBlock u;              // 尿酸(U)參數
    CParameterBlock c;              // 膽固醇(C)參數
    TGParameterBlock tg;            // 三酸甘油脂(TG)參數
    GAVParameterBlock gav;          // 血糖(GAV)參數
    uint8_t reserved[36];           // 保留區域供未來擴展
    uint16_t checksum;              // 校驗和 (0~844位址的總和)
    uint8_t crc16;                  // CRC16校驗值
} ParameterTable;

#pragma pack()  // 恢復默認對齊

/* 全局參數表指針 */
extern ParameterTable *g_pParamTable;

/* 參數表函數原型 */
ParamError_TypeDef PARAM_Init(void);
ParamError_TypeDef PARAM_LoadFromFlash(ParamArea_TypeDef area);
ParamError_TypeDef PARAM_SaveToFlash(ParamArea_TypeDef area);
ParamError_TypeDef PARAM_LoadDefaults(void);
ParamError_TypeDef PARAM_GetValue(uint16_t paramAddress, void *pValue, uint8_t size);
ParamError_TypeDef PARAM_SetValue(uint16_t paramAddress, void *pValue, uint8_t size);
ParamError_TypeDef PARAM_Backup(void);
ParamError_TypeDef PARAM_Restore(void);
uint8_t PARAM_IsInitialized(void);
uint16_t PARAM_CalculateChecksum(void);
uint8_t PARAM_CalculateCRC16(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */
