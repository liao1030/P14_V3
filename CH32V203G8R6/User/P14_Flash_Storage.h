/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : 多功能生化測試儀Flash參數儲存方案定義
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_FLASH_STORAGE_H__
#define __P14_FLASH_STORAGE_H__

#include "ch32v20x.h"
#include "P14_Parameter_Table.h"
#include <stdbool.h>

/* 參數表區域定義 */
#define PARAM_BLOCK_A_ADDR       0x0800D800  // 參數表A區起始地址
#define PARAM_BLOCK_B_ADDR       0x0800E000  // 參數表B區起始地址
#define RECORD_ZONE_ADDR         0x0800E800  // 測量記錄區起始地址
#define PARAM_PAGE_SIZE          1024        // Flash頁大小(1KB)
#define PARAM_BLOCK_SIZE         2048        // 參數表區大小(2KB)

/* 參數表區塊標頭定義 */
typedef struct {
    uint16_t magic;         // 魔數,固定為0xA55A
    uint16_t version;       // 參數表版本號
    uint32_t writeCounter;  // 寫入計數器
    uint32_t timestamp;     // 上次更新時間戳
    uint16_t checksum;      // 參數數據校驗和
    uint16_t crc16;         // 完整區塊CRC校驗
} ParamBlockHeader;

/* 基本系統參數區 */
typedef struct {
    uint8_t lbt;            // 低電池閾值
    uint8_t obt;            // 電池耗盡閾值
    uint8_t factory;        // 儀器操作模式
    uint8_t modelNo;        // 產品型號
    uint8_t fwNo;           // 韌體版本號
    uint16_t testCount;     // 測量次數
    uint16_t codeTableVer;  // 代號表版本編號
    
    uint8_t year;           // 年
    uint8_t month;          // 月
    uint8_t date;           // 日
    uint8_t hour;           // 時
    uint8_t minute;         // 分
    uint8_t second;         // 秒
    
    uint8_t tempLowLimit;   // 操作溫度下限
    uint8_t tempHighLimit;  // 操作溫度上限
    uint8_t measureUnit;    // 測量單位設定
    uint8_t defaultEvent;   // 預設事件類型
    uint8_t stripType;      // 測試項目類型
} BasicSystemBlock;

/* 硬體校準參數區 */
typedef struct {
    uint16_t evT3Trigger;   // EV_T3觸發電壓
    uint8_t evWorking;      // 測量工作電極電壓
    uint8_t evT3;           // 血液流速電極電壓
    uint8_t dacoOffset;     // DAC偏移敏感
    uint8_t dacdo;          // DAC校正管理參數
    uint8_t cc211Status;    // CC211未完成旗標
    uint8_t calTolerance;   // OPS/OPI校正容差
    float ops;              // OPA校正斜率
    float opi;              // OPA校正截距
    uint8_t qct;            // QCT校正測量
    int8_t tempOffset;      // 溫度校正偏移
    int8_t batteryOffset;   // 電池校正偏移
} HardwareCalibBlock;

/* 測試項目參數區 (以血糖為例) */
typedef struct {
    uint8_t bgCsuTolerance;     // 試片檢查容差
    uint16_t bgNdl;             // 新試片測量水平
    uint16_t bgUdl;             // 已使用試片測量水平
    uint16_t bgBloodIn;         // 血液測量水平
    char bgStripLot[16];        // 試片批號
    
    uint8_t bgL;                // 血糖值下限
    uint8_t bgH;                // 血糖值上限
    uint16_t bgT3E37;           // T3 ADV溢37閾值
    
    uint16_t bgTPL1;            // 時間脈衝寬度(第一組)
    uint16_t bgTRD1;            // 原始數據時間(第一組)
    uint16_t bgEVWidth1;        // 熔燒時間(第一組)
    uint16_t bgTPL2;            // 時間脈衝寬度(第二組)
    uint16_t bgTRD2;            // 原始數據時間(第二組)
    uint16_t bgEVWidth2;        // 熔燒時間(第二組)
    
    // QC參數區
    uint8_t bgSq;               // QC敏感參數Sq
    uint8_t bgIq;               // QC敏感參數Iq
    uint8_t bgCvq;              // QC水平的CV
    uint8_t bgAq;               // QC敏感參數A
    uint8_t bgBq;               // QC敏感參數B
    uint8_t bgAq5;              // QC敏感參數A(等級5)
    uint8_t bgBq5;              // QC敏感參數B(等級5)
    uint8_t bgQc[30];           // QC模板範圍參數
    
    // 靈敏度參數區
    uint8_t bgS2;               // 試片敏感參數
    uint8_t bgI2;               // 試片敏感參數
    float bgSr;                 // OPA硬體迴路敏感(斜率)
    float bgIr;                 // OPA硬體迴路敏感(截距)
    uint8_t bgS3[30];           // 試片敏感參數S3
    uint8_t bgI3[30];           // 試片敏感參數I3
    
    // 溫度敏感參數區
    uint8_t bgTf[7];            // AC/PC溫度敏感斜率
    uint8_t bgCtf[7];           // QC溫度敏感斜率
    uint8_t bgTo[7];            // AC/PC溫度敏感截距
    uint8_t bgCto[7];           // QC溫度敏感截距
} BGParameterBlock;

typedef struct {
    uint8_t uCsuTolerance;      // 試片檢查容差
    uint16_t uNdl;              // 新試片測量水平
    uint16_t uUdl;              // 已使用試片測量水平
    uint16_t uBloodIn;          // 血液測量水平
    char uStripLot[16];         // 試片批號
    
    uint8_t uL;                 // 尿酸值下限
    uint8_t uH;                 // 尿酸值上限
    uint16_t uT3E37;            // T3 ADV溢37閾值
    
    uint16_t uTPL1;             // 時間脈衝寬度(第一組)
    uint16_t uTRD1;             // 原始數據時間(第一組)
    uint16_t uEVWidth1;         // 熔燒時間(第一組)
    uint16_t uTPL2;             // 時間脈衝寬度(第二組)
    uint16_t uTRD2;             // 原始數據時間(第二組)
    uint16_t uEVWidth2;         // 熔燒時間(第二組)
    
    // QC參數區
    uint8_t uSq;                // QC敏感參數Sq
    uint8_t uIq;                // QC敏感參數Iq
    uint8_t uCvq;               // QC水平的CV
    uint8_t uAq;                // QC敏感參數A
    uint8_t uBq;                // QC敏感參數B
    uint8_t uQc[30];            // QC模板範圍參數
    
    // 靈敏度參數區
    uint8_t uS2;                // 試片敏感參數
    uint8_t uI2;                // 試片敏感參數
    float uSr;                  // OPA硬體迴路敏感(斜率)
    float uIr;                  // OPA硬體迴路敏感(截距)
    uint8_t uS3[15];            // 試片敏感參數S3
    uint8_t uI3[15];            // 試片敏感參數I3
    
    // 溫度敏感參數區
    uint8_t uTf[7];             // AC/PC溫度敏感斜率
    uint8_t uCtf[7];            // QC溫度敏感斜率
    uint8_t uTo[7];             // AC/PC溫度敏感截距
    uint8_t uCto[7];            // QC溫度敏感截距
} UricAcidParameterBlock;

typedef struct {
    uint8_t cCsuTolerance;      // 試片檢查容差
    uint16_t cNdl;              // 新試片測量水平
    uint16_t cUdl;              // 已使用試片測量水平
    uint16_t cBloodIn;          // 血液測量水平
    char cStripLot[16];         // 試片批號
    
    uint8_t cL;                 // 膽固醇值下限
    uint8_t cH;                 // 膽固醇值上限
    uint16_t cT3E37;            // T3 ADV溢37閾值
    
    uint16_t cTPL1;             // 時間脈衝寬度(第一組)
    uint16_t cTRD1;             // 原始數據時間(第一組)
    uint16_t cEVWidth1;         // 熔燒時間(第一組)
    uint16_t cTPL2;             // 時間脈衝寬度(第二組)
    uint16_t cTRD2;             // 原始數據時間(第二組)
    uint16_t cEVWidth2;         // 熔燒時間(第二組)
    
    // QC參數區
    uint8_t cSq;                // QC敏感參數Sq
    uint8_t cIq;                // QC敏感參數Iq
    uint8_t cCvq;               // QC水平的CV
    uint8_t cAq;                // QC敏感參數A
    uint8_t cBq;                // QC敏感參數B
    uint8_t cQc[30];            // QC模板範圍參數
    
    // 靈敏度參數區
    uint8_t cS2;                // 試片敏感參數
    uint8_t cI2;                // 試片敏感參數
    float cSr;                  // OPA硬體迴路敏感(斜率)
    float cIr;                  // OPA硬體迴路敏感(截距)
    uint8_t cS3[15];            // 試片敏感參數S3
    uint8_t cI3[15];            // 試片敏感參數I3
    
    // 溫度敏感參數區
    uint8_t cTf[7];             // AC/PC溫度敏感斜率
    uint8_t cCtf[7];            // QC溫度敏感斜率
    uint8_t cTo[7];             // AC/PC溫度敏感截距
    uint8_t cCto[7];            // QC溫度敏感截距
} CholesterolParameterBlock;

typedef struct {
    uint8_t tgCsuTolerance;     // 試片檢查容差
    uint16_t tgNdl;             // 新試片測量水平
    uint16_t tgUdl;             // 已使用試片測量水平
    uint16_t tgBloodIn;         // 血液測量水平
    char tgStripLot[16];        // 試片批號
    
    uint8_t tgL;                // 三酸甘油脂值下限
    uint8_t tgH;                // 三酸甘油脂值上限
    uint16_t tgT3E37;           // T3 ADV溢37閾值
    
    uint16_t tgTPL1;            // 時間脈衝寬度(第一組)
    uint16_t tgTRD1;            // 原始數據時間(第一組)
    uint16_t tgEVWidth1;        // 熔燒時間(第一組)
    uint16_t tgTPL2;            // 時間脈衝寬度(第二組)
    uint16_t tgTRD2;            // 原始數據時間(第二組)
    uint16_t tgEVWidth2;        // 熔燒時間(第二組)
    
    // QC參數區
    uint8_t tgSq;               // QC敏感參數Sq
    uint8_t tgIq;               // QC敏感參數Iq
    uint8_t tgCvq;              // QC水平的CV
    uint8_t tgAq;               // QC敏感參數A
    uint8_t tgBq;               // QC敏感參數B
    uint8_t tgQc[30];           // QC模板範圍參數
    
    // 靈敏度參數區
    uint8_t tgS2;               // 試片敏感參數
    uint8_t tgI2;               // 試片敏感參數
    float tgSr;                 // OPA硬體迴路敏感(斜率)
    float tgIr;                 // OPA硬體迴路敏感(截距)
    uint8_t tgS3[15];           // 試片敏感參數S3
    uint8_t tgI3[15];           // 試片敏感參數I3
    
    // 溫度敏感參數區
    uint8_t tgTf[7];            // AC/PC溫度敏感斜率
    uint8_t tgCtf[7];           // QC溫度敏感斜率
    uint8_t tgTo[7];            // AC/PC溫度敏感截距
    uint8_t tgCto[7];           // QC溫度敏感截距
} TriglycerideParameterBlock;

/* 保留區與校驗區 */
typedef struct {
    uint8_t reserved[36];       // 保留區域供未來擴展
    uint16_t checksum;          // 校驗和
    uint16_t crc16;             // CRC校驗
} ReservedBlock;

/* 所有參數區塊組合 */
typedef struct {
    BasicSystemBlock basicSystem;
    HardwareCalibBlock hardwareCalib;
    BGParameterBlock bgParams;
    UricAcidParameterBlock uParams;
    CholesterolParameterBlock cParams;
    TriglycerideParameterBlock tgParams;
    ReservedBlock reserved;
} ParameterData;

/* 完整參數表區塊 */
typedef struct {
    ParamBlockHeader header;
    ParameterData params;
} ParameterBlock;

/* 測量記錄結構 */
typedef struct {
    uint16_t flag;              // 記錄有效標記 (0xA55A=有效)
    uint16_t resultStatus;      // 測量結果狀態 (0=成功，其他=錯誤)
    uint16_t resultValue;       // 測量結果值
    uint8_t testType;           // 測量項目類型
    uint8_t event;              // 事件類型
    uint8_t stripCode;          // 試片校正碼
    uint8_t year;               // 年
    uint8_t month;              // 月
    uint8_t date;               // 日
    uint8_t hour;               // 時
    uint8_t minute;             // 分
    uint8_t second;             // 秒
    uint16_t batteryVoltage;    // 電池電壓
    uint16_t temperature;       // 環境溫度
} TestRecord;

/* 功能函數宣告 */
bool PARAM_Init(void);
bool PARAM_LoadDefault(void);
bool PARAM_ReadBlock(uint8_t blockType, void *buffer, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_IncreaseTestCount(void);
bool PARAM_SaveTestRecord(uint16_t resultValue, uint16_t resultStatus, uint8_t event, uint8_t code, uint16_t batteryVoltage, uint16_t temperature);
uint16_t PARAM_GetTestCount(void);
bool PARAM_GetTestRecord(uint16_t index, TestRecord *record);
uint16_t PARAM_GetTestRecordCount(void);
bool PARAM_EraseAllTestRecords(void);

/* Flash操作相關函數 */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *buffer, uint32_t size);
uint16_t CalcChecksum(const uint8_t *data, uint32_t size);
uint16_t CalcCRC16(const uint8_t *data, uint32_t size);
void UpdateChecksum(ParameterBlock *block);

#define BLOCK_BASIC_SYSTEM     0
#define BLOCK_HARDWARE_CALIB   1
#define BLOCK_BG_PARAMS        2
#define BLOCK_U_PARAMS         3
#define BLOCK_C_PARAMS         4
#define BLOCK_TG_PARAMS        5

#endif /* __P14_FLASH_STORAGE_H__ */ 