/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : CH32V203 Flash參數儲存方案頭文件
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __P14_FLASH_STORAGE_H__
#define __P14_FLASH_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ch32v20x.h"
#include "parameter_table.h"

/* 參數表儲存區配置 */
#define PARAM_FLASH_BASE          0x08000000
#define PARAM_BLOCK_A_ADDR        0x0800D800  // 參數區A起始地址 (55KB)
#define PARAM_BLOCK_B_ADDR        0x0800E000  // 參數區B起始地址 (56KB)
#define PARAM_BLOCK_SIZE          0x800       // 參數區大小 (2KB)
#define PARAM_SECTOR_SIZE         0x400       // 擦除扇區大小 (1KB)

/* 參數區域區塊類型 */
#define BLOCK_BASIC_SYSTEM        0  // 基本系統參數
#define BLOCK_HARDWARE_CALIB      1  // 硬體校正參數
#define BLOCK_GLUCOSE_PARAMS      2  // 血糖(GLV/GAV)參數
#define BLOCK_URIC_ACID_PARAMS    3  // 尿酸(U)參數
#define BLOCK_CHOLESTEROL_PARAMS  4  // 總膽固醇(C)參數
#define BLOCK_TRIGLYCERIDE_PARAMS 5  // 三酸甘油脂(TG)參數
#define BLOCK_RESERVED            6  // 保留區塊

/* 參數表結構定義 */
#pragma pack(1)

/* 參數表頭部結構 */
typedef struct {
    uint16_t magic;         // 魔術碼 (0xA55A)
    uint8_t version;        // 參數表版本
    uint16_t writeCounter;  // 寫入計數器
    uint32_t timestamp;     // 時間戳記
    uint16_t checksum;      // 校驗和
} ParamHeader_TypeDef;

/* 基本系統參數區塊 */
typedef struct {
    uint8_t lbt;              // 低電池閾值
    uint8_t obt;              // 電池耗盡閾值
    uint8_t factory;          // 儀器操作模式
    uint8_t modelNo;          // 產品型號
    uint8_t fwNo;             // 韌體版本號
    uint16_t testCount;       // 測試次數
    uint16_t codeTableVer;    // 代碼表版本號
    uint8_t year;             // 年份設定
    uint8_t month;            // 月份設定
    uint8_t date;             // 日期設定
    uint8_t hour;             // 小時設定
    uint8_t minute;           // 分鐘設定
    uint8_t second;           // 秒數設定
    uint8_t tempLowLimit;     // 操作溫度範圍下限
    uint8_t tempHighLimit;    // 操作溫度範圍上限
    uint8_t measureUnit;      // 濃度單位設定
    uint8_t defaultEvent;     // 餐飲事件類型
    uint8_t stripType;        // 測試項目
    uint8_t reserved[2];      // 預留位置
} BasicSystemBlock;

/* 硬體校正參數區塊 */
typedef struct {
    uint16_t evT3Trigger;    // EV_T3觸發電壓
    uint8_t evWorking;       // 濃度工作電流電壓
    uint8_t evT3;            // 血液消耗電流電壓
    uint8_t dacoOffset;      // DAC偏移補償
    uint8_t dacdo;           // DAC校正管理參數
    uint8_t cc211Status;     // CC211未完成旗標
    uint8_t calTolerance;    // OPS/OPI校正容差
    float ops;               // OPA校正斜率
    float opi;               // OPA校正截距
    uint8_t qct;             // QCT校正測試低位元組
    int8_t tempOffset;       // 溫度校正偏移
    int8_t batteryOffset;    // 電池校正偏移
    uint8_t reserved[5];     // 預留位置
} HardwareCalibBlock;

/* 血糖參數區塊 */
typedef struct {
    uint8_t bgCsuTolerance;  // 試片檢查容差
    uint16_t bgNdl;          // 新試片濃度水平
    uint16_t bgUdl;          // 已使用試片濃度水平
    uint16_t bgBloodIn;      // 血液濃度水平
    uint8_t bgStripLot[16];  // 試片批號
    uint8_t bgL;             // 血糖值下限
    uint16_t bgH;            // 血糖值上限
    uint16_t bgT3E37;        // T3 ADV超E37閾值
    uint16_t bgTPL1;         // 第一組時序參數TPL
    uint16_t bgTRD1;         // 第一組時序參數TRD
    uint16_t bgEVWidth1;     // 第一組時序參數EV寬度
    uint16_t bgTPL2;         // 第二組時序參數TPL
    uint16_t bgTRD2;         // 第二組時序參數TRD
    uint16_t bgEVWidth2;     // 第二組時序參數EV寬度
    uint8_t bgCompA;         // 補償參數A
    uint8_t bgCompB;         // 補償參數B
    uint8_t bgCompC;         // 補償參數C
    uint8_t bgCompD;         // 補償參數D
    uint8_t reserved[128];   // 預留位置
} GlucoseParamBlock;

/* 尿酸參數區塊 */
typedef struct {
    uint8_t uCsuTolerance;   // 試片檢查容差
    uint16_t uNdl;           // 新試片濃度水平
    uint16_t uUdl;           // 已使用試片濃度水平
    uint16_t uBloodIn;       // 血液濃度水平
    uint8_t uStripLot[16];   // 試片批號
    uint8_t uL;              // 尿酸值下限
    uint16_t uH;             // 尿酸值上限
    uint16_t uT3E37;         // T3 ADV超E37閾值
    uint16_t uTPL1;          // 第一組時序參數TPL
    uint16_t uTRD1;          // 第一組時序參數TRD
    uint16_t uEVWidth1;      // 第一組時序參數EV寬度
    uint16_t uTPL2;          // 第二組時序參數TPL
    uint16_t uTRD2;          // 第二組時序參數TRD
    uint16_t uEVWidth2;      // 第二組時序參數EV寬度
    uint8_t uCompA;          // 補償參數A
    uint8_t uCompB;          // 補償參數B
    uint8_t uCompC;          // 補償參數C
    uint8_t uCompD;          // 補償參數D
    uint8_t reserved[128];   // 預留位置
} UricAcidParamBlock;

/* 總膽固醇參數區塊 */
typedef struct {
    uint8_t cCsuTolerance;   // 試片檢查容差
    uint16_t cNdl;           // 新試片濃度水平
    uint16_t cUdl;           // 已使用試片濃度水平
    uint16_t cBloodIn;       // 血液濃度水平
    uint8_t cStripLot[16];   // 試片批號
    uint8_t cL;              // 膽固醇值下限
    uint16_t cH;             // 膽固醇值上限
    uint16_t cT3E37;         // T3 ADV超E37閾值
    uint16_t cTPL1;          // 第一組時序參數TPL
    uint16_t cTRD1;          // 第一組時序參數TRD
    uint16_t cEVWidth1;      // 第一組時序參數EV寬度
    uint16_t cTPL2;          // 第二組時序參數TPL
    uint16_t cTRD2;          // 第二組時序參數TRD
    uint16_t cEVWidth2;      // 第二組時序參數EV寬度
    uint8_t cCompA;          // 補償參數A
    uint8_t cCompB;          // 補償參數B
    uint8_t cCompC;          // 補償參數C
    uint8_t cCompD;          // 補償參數D
    uint8_t reserved[128];   // 預留位置
} CholesterolParamBlock;

/* 三酸甘油脂參數區塊 */
typedef struct {
    uint8_t tgCsuTolerance;  // 試片檢查容差
    uint16_t tgNdl;          // 新試片濃度水平
    uint16_t tgUdl;          // 已使用試片濃度水平
    uint16_t tgBloodIn;      // 血液濃度水平
    uint8_t tgStripLot[16];  // 試片批號
    uint8_t tgL;             // 三酸甘油脂值下限
    uint16_t tgH;            // 三酸甘油脂值上限
    uint16_t tgT3E37;        // T3 ADV超E37閾值
    uint16_t tgTPL1;         // 第一組時序參數TPL
    uint16_t tgTRD1;         // 第一組時序參數TRD
    uint16_t tgEVWidth1;     // 第一組時序參數EV寬度
    uint16_t tgTPL2;         // 第二組時序參數TPL
    uint16_t tgTRD2;         // 第二組時序參數TRD
    uint16_t tgEVWidth2;     // 第二組時序參數EV寬度
    uint8_t tgCompA;         // 補償參數A
    uint8_t tgCompB;         // 補償參數B
    uint8_t tgCompC;         // 補償參數C
    uint8_t tgCompD;         // 補償參數D
    uint8_t reserved[128];   // 預留位置
} TriglycerideParamBlock;

/* 保留區塊 */
typedef struct {
    uint8_t reserved[120];   // 預留位置
    uint16_t calibChecksum;  // 校驗區校驗和
} ReservedBlock;

/* 參數區總結構 */
typedef struct {
    BasicSystemBlock basicSystem;           // 基本系統參數
    HardwareCalibBlock hardwareCalib;       // 硬體校正參數
    GlucoseParamBlock bgParams;             // 血糖參數
    UricAcidParamBlock uaParams;            // 尿酸參數
    CholesterolParamBlock cholParams;       // 總膽固醇參數
    TriglycerideParamBlock tgParams;        // 三酸甘油脂參數
    ReservedBlock reserved;                 // 保留區塊
} ParameterBlocks;

/* 完整參數表結構 */
typedef struct {
    ParamHeader_TypeDef header;             // 參數表頭部
    ParameterBlocks params;                 // 參數區
} ParameterBlock;

#pragma pack()

/* 函數宣告 */
void FLASH_Storage_Init(void);
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size);
bool FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t size);
bool PARAM_ReadParameterBlock(uint8_t blockType, void *buffer, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
void PARAM_ResetToDefault(void);

#ifdef __cplusplus
}
#endif

#endif /* __P14_FLASH_STORAGE_H__ */ 