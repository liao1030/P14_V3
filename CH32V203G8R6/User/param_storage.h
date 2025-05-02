/**
 * @file param_storage.h
 * @brief CH32V203G8R6多功能參數Flash儲存方案頭文件
 * @version 1.0
 * @date 2024-09-25
 * 
 * @copyright Copyright (c) 2024 HMD Biomedical Inc.
 */

#ifndef __PARAM_STORAGE_H__
#define __PARAM_STORAGE_H__

#include "ch32v20x.h"
#include "param_table.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 參數區域相關定義 */
#define PARAM_BLOCK_A_ADDR      0x0800D800  // 參數區A起始地址
#define PARAM_BLOCK_B_ADDR      0x0800E000  // 參數區B起始地址
#define PARAM_RECORD_ADDR       0x0800E800  // 測試記錄區起始地址
#define PARAM_BLOCK_SIZE        2048        // 參數區大小(2KB)

/* 參數區塊識別碼 */
#define BLOCK_BASIC_SYSTEM      0x00        // 基本系統區
#define BLOCK_HARDWARE_CALIB    0x01        // 硬體校準區
#define BLOCK_BG_PARAMS         0x02        // 血糖(GLV/GAV)參數區
#define BLOCK_UA_PARAMS         0x03        // 尿酸(U)參數區
#define BLOCK_CHOL_PARAMS       0x04        // 總膽固醇(C)參數區
#define BLOCK_TG_PARAMS         0x05        // 三酸甘油脂(TG)參數區
#define BLOCK_RESERVED          0x06        // 保留區域
#define BLOCK_MAX               0x07        // 最大區塊識別碼

/* 參數區塊Header定義 */
typedef struct {
    uint16_t magic;               // 魔數(0xA55A)
    uint8_t  version;             // 版本號
    uint8_t  writeCounter;        // 寫入計數器
    uint32_t timestamp;           // 寫入時間戳
} BlockHeader_TypeDef;

/* 基本系統參數區塊 */
typedef struct {
    uint8_t  lbt;                 // 低電池閾值
    uint8_t  obt;                 // 電池耗盡閾值
    uint8_t  factory;             // 儀器操作模式
    uint8_t  modelNo;             // 產品型號
    uint8_t  fwNo;                // 韌體版本號
    uint16_t testCount;           // 測試次數
    uint16_t codeTableVer;        // 代碼表版本編號
    uint8_t  year;                // 年份設定
    uint8_t  month;               // 月份設定
    uint8_t  date;                // 日期設定
    uint8_t  hour;                // 小時設定
    uint8_t  minute;              // 分鐘設定
    uint8_t  second;              // 秒數設定
    uint8_t  tempLowLimit;        // 操作溫度範圍下限
    uint8_t  tempHighLimit;       // 操作溫度範圍上限
    uint8_t  measureUnit;         // 測量單位設定
    uint8_t  defaultEvent;        // 預設事件類型
    uint8_t  stripType;           // 測試項目
} BasicSystemBlock;

/* 硬體校準參數區塊 */
typedef struct {
    uint16_t evT3Trigger;         // EV_T3觸發電壓
    uint8_t  evWorking;           // 測量工作電極電壓
    uint8_t  evT3;                // 血液檢測電極電壓
    uint8_t  dacoOffset;          // DAC偏移補償
    uint8_t  dacdo;               // DAC校正管理參數
    uint8_t  cc211Status;         // CC211狀態旗標
    uint8_t  calTolerance;        // OPS/OPI校準容差
    float    ops;                 // OPA校準斜率
    float    opi;                 // OPA校準截距
    uint8_t  qct;                 // QCT校準測試低位元組
    int8_t   tempOffset;          // 溫度校準偏移
    int8_t   batteryOffset;       // 電池校準偏移
} HardwareCalibBlock;

/* 血糖(GLV/GAV)參數區塊 */
typedef struct {
    uint8_t  bgCsuTolerance;      // 試片檢查容差
    uint16_t bgNdl;               // 新試片檢測水平
    uint16_t bgUdl;               // 已使用試片檢測水平
    uint16_t bgBloodIn;           // 血液檢測水平
    uint8_t  bgStripLot[16];      // 試片批號
    uint8_t  bgL;                 // 血糖值下限
    uint16_t bgH;                 // 血糖值上限
    uint16_t bgT3E37;             // T3 ADV錯誤37閾值
    uint16_t bgTPL1;              // 時間脈衝低(第一組)
    uint16_t bgTRD1;              // 原始數據時間(第一組)
    uint16_t bgEVWidth1;          // 燃燒時間(第一組)
    uint16_t bgTPL2;              // 時間脈衝低(第二組)
    uint16_t bgTRD2;              // 原始數據時間(第二組)
    uint16_t bgEVWidth2;          // 燃燒時間(第二組)
    uint8_t  bgSq;                // QC補償參數Sq
    uint8_t  bgIq;                // QC補償參數Iq
    uint8_t  bgCVq;               // QC水平的CV
    uint8_t  bgAq;                // QC補償參數A
    uint8_t  bgBq;                // QC補償參數B
    uint8_t  bgAq5;               // QC補償參數A(等級5)
    uint8_t  bgBq5;               // QC補償參數B(等級5)
    uint8_t  bgQC[30];            // QC標準範圍參數
    uint8_t  bgS2;                // 試片補償參數
    uint8_t  bgI2;                // 試片補償參數
    float    bgSr;                // OPA硬體迴路補償(斜率)
    float    bgIr;                // OPA硬體迴路補償(截距)
    uint8_t  bgS3[30];            // 試片補償參數S3(1-30)
    uint8_t  bgI3[30];            // 試片補償參數I3(1-30)
    uint8_t  bgTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t  bgCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t  bgTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t  bgCTO[7];            // QC溫度補償截距(10-40°C)
} BloodGlucoseBlock;

/* 尿酸(U)參數區塊 */
typedef struct {
    uint8_t  uCsuTolerance;       // 試片檢查容差
    uint16_t uNdl;                // 新試片檢測水平
    uint16_t uUdl;                // 已使用試片檢測水平
    uint16_t uBloodIn;            // 血液檢測水平
    uint8_t  uStripLot[16];       // 試片批號
    uint8_t  uL;                  // 尿酸值下限
    uint16_t uH;                  // 尿酸值上限
    uint16_t uT3E37;              // T3 ADV錯誤37閾值
    uint16_t uTPL1;               // 時間脈衝低(第一組)
    uint16_t uTRD1;               // 原始數據時間(第一組)
    uint16_t uEVWidth1;           // 燃燒時間(第一組)
    uint16_t uTPL2;               // 時間脈衝低(第二組)
    uint16_t uTRD2;               // 原始數據時間(第二組)
    uint16_t uEVWidth2;           // 燃燒時間(第二組)
    uint8_t  uSq;                 // QC補償參數Sq
    uint8_t  uIq;                 // QC補償參數Iq
    uint8_t  uCVq;                // QC水平的CV
    uint8_t  uAq;                 // QC補償參數A
    uint8_t  uBq;                 // QC補償參數B
    uint8_t  uQC[30];             // QC標準範圍參數
    uint8_t  uS2;                 // 試片補償參數
    uint8_t  uI2;                 // 試片補償參數
    float    uSr;                 // OPA硬體迴路補償(斜率)
    float    uIr;                 // OPA硬體迴路補償(截距)
    uint8_t  uS3[15];             // 試片補償參數S3(1-15)
    uint8_t  uI3[15];             // 試片補償參數I3(1-15)
    uint8_t  uTF[7];              // AC/PC溫度補償斜率(10-40°C)
    uint8_t  uCTF[7];             // QC溫度補償斜率(10-40°C)
    uint8_t  uTO[7];              // AC/PC溫度補償截距(10-40°C)
    uint8_t  uCTO[7];             // QC溫度補償截距(10-40°C)
} UricAcidBlock;

/* 總膽固醇(C)參數區塊 */
typedef struct {
    uint8_t  cCsuTolerance;       // 試片檢查容差
    uint16_t cNdl;                // 新試片檢測水平
    uint16_t cUdl;                // 已使用試片檢測水平
    uint16_t cBloodIn;            // 血液檢測水平
    uint8_t  cStripLot[16];       // 試片批號
    uint8_t  cL;                  // 膽固醇值下限
    uint16_t cH;                  // 膽固醇值上限
    uint16_t cT3E37;              // T3 ADV錯誤37閾值
    uint16_t cTPL1;               // 時間脈衝低(第一組)
    uint16_t cTRD1;               // 原始數據時間(第一組)
    uint16_t cEVWidth1;           // 燃燒時間(第一組)
    uint16_t cTPL2;               // 時間脈衝低(第二組)
    uint16_t cTRD2;               // 原始數據時間(第二組)
    uint16_t cEVWidth2;           // 燃燒時間(第二組)
    uint8_t  cSq;                 // QC補償參數Sq
    uint8_t  cIq;                 // QC補償參數Iq
    uint8_t  cCVq;                // QC水平的CV
    uint8_t  cAq;                 // QC補償參數A
    uint8_t  cBq;                 // QC補償參數B
    uint8_t  cQC[30];             // QC標準範圍參數
    uint8_t  cS2;                 // 試片補償參數
    uint8_t  cI2;                 // 試片補償參數
    float    cSr;                 // OPA硬體迴路補償(斜率)
    float    cIr;                 // OPA硬體迴路補償(截距)
    uint8_t  cS3[15];             // 試片補償參數S3(1-15)
    uint8_t  cI3[15];             // 試片補償參數I3(1-15)
    uint8_t  cTF[7];              // AC/PC溫度補償斜率(10-40°C)
    uint8_t  cCTF[7];             // QC溫度補償斜率(10-40°C)
    uint8_t  cTO[7];              // AC/PC溫度補償截距(10-40°C)
    uint8_t  cCTO[7];             // QC溫度補償截距(10-40°C)
} CholesterolBlock;

/* 三酸甘油脂(TG)參數區塊 */
typedef struct {
    uint8_t  tgCsuTolerance;      // 試片檢查容差
    uint16_t tgNdl;               // 新試片檢測水平
    uint16_t tgUdl;               // 已使用試片檢測水平
    uint16_t tgBloodIn;           // 血液檢測水平
    uint8_t  tgStripLot[16];      // 試片批號
    uint8_t  tgL;                 // 三酸甘油脂值下限
    uint16_t tgH;                 // 三酸甘油脂值上限
    uint16_t tgT3E37;             // T3 ADV錯誤37閾值
    uint16_t tgTPL1;              // 時間脈衝低(第一組)
    uint16_t tgTRD1;              // 原始數據時間(第一組)
    uint16_t tgEVWidth1;          // 燃燒時間(第一組)
    uint16_t tgTPL2;              // 時間脈衝低(第二組)
    uint16_t tgTRD2;              // 原始數據時間(第二組)
    uint16_t tgEVWidth2;          // 燃燒時間(第二組)
    uint8_t  tgSq;                // QC補償參數Sq
    uint8_t  tgIq;                // QC補償參數Iq
    uint8_t  tgCVq;               // QC水平的CV
    uint8_t  tgAq;                // QC補償參數A
    uint8_t  tgBq;                // QC補償參數B
    uint8_t  tgQC[30];            // QC標準範圍參數
    uint8_t  tgS2;                // 試片補償參數
    uint8_t  tgI2;                // 試片補償參數
    float    tgSr;                // OPA硬體迴路補償(斜率)
    float    tgIr;                // OPA硬體迴路補償(截距)
    uint8_t  tgS3[15];            // 試片補償參數S3(1-15)
    uint8_t  tgI3[15];            // 試片補償參數I3(1-15)
    uint8_t  tgTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t  tgCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t  tgTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t  tgCTO[7];            // QC溫度補償截距(10-40°C)
} TriglycerideBlock;

/* 保留與校驗區塊 */
typedef struct {
    uint8_t  reserved[36];        // 保留區域
    uint16_t checksum;            // 校驗和
    uint8_t  crc;                 // CRC校驗
} ReservedBlock;

/* 參數區塊聯合體 */
typedef union {
    BasicSystemBlock  basicSystem;
    HardwareCalibBlock hardwareCalib;
    BloodGlucoseBlock bgParams;
    UricAcidBlock     uaParams;
    CholesterolBlock  cholParams;
    TriglycerideBlock tgParams;
    ReservedBlock     reserved;
} ParameterBlockData;

/* 參數區塊完整結構 */
typedef struct {
    BlockHeader_TypeDef header;
    ParameterBlockData  params;
} ParameterBlock;

/* 測試記錄結構 */
typedef struct {
    uint16_t resultStatus;    // 測試結果狀態 (0=成功，其他=錯誤)
    uint16_t resultValue;     // 測試結果值
    uint16_t testType;        // 測試項目類型
    uint16_t eventType;       // 事件類型
    uint8_t  stripCode;       // 試片校正碼
    uint16_t year;            // 年
    uint16_t month;           // 月
    uint16_t date;            // 日
    uint16_t hour;            // 時
    uint16_t minute;          // 分
    uint16_t second;          // 秒
    uint16_t batteryVoltage;  // 電池電壓
    uint16_t temperature;     // 環境溫度
} TestRecord;

/* 函數聲明 */
void PARAM_Init(void);
bool PARAM_ReadBlock(uint8_t blockId, void *data, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockId, const void *data, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
bool PARAM_GetTestCount(uint16_t *count);
bool PARAM_ResetToFactory(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_SaveTestRecord(const TestRecord *record);
bool PARAM_ReadTestRecords(TestRecord *records, uint16_t startIdx, uint16_t count, uint16_t *actualCount);
uint16_t PARAM_GetTestRecordCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_STORAGE_H__ */ 