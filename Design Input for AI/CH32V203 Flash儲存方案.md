# 多功能生化測試儀CH32V203 Flash儲存方案

## 目錄
1. [硬體特性](#1-硬體特性)
2. [記憶體分配](#2-記憶體分配)
3. [參數表區域細分](#3-參數表區域細分)
4. [參數表資料結構設計](#4-參數表資料結構設計)
5. [參數存取函數設計](#5-參數存取函數設計)
6. [參數使用範例](#6-參數使用範例)
7. [結構重組的優勢](#7-結構重組的優勢)

## 1. 硬體特性
CH32V203G8R6 微控制器具有以下 Flash 記憶體特性：
- 總容量：64KB (0x10000 位元組)
- 頁大小：1KB (0x400 位元組)
- 寫入單位：半字 (2 位元組)
- 最小擦除單位：1 頁 (1KB)
- 擦寫壽命：約 10,000 次

## 2. 記憶體分配

| 區域名稱 | 地址範圍 | 大小 | 用途 |
|---------|---------|------|------|
| 主程式區 | 0x08000000 - 0x0800CFFF | 52KB | 系統韌體和主程式 |
| 參數區 A | 0x0800D000 - 0x0800DFFF | 4KB | 參數表主要儲存區域 |
| 參數區 B | 0x0800E000 - 0x0800EFFF | 4KB | 參數表備份區域 |
| 測試記錄區 | 0x0800F000 - 0x0800FFFF | 4KB | 最近測試記錄儲存 |

## 3. 參數表區域細分

根據多功能生化測試儀參數代碼表，參數表區域細分為以下功能區塊：

| 功能區塊 | 參數範圍 | 大小 | 更新頻率 | 描述 |
|---------|---------|------|---------|------|
| 基本系統區 | 0-19 | 20 bytes | 高 | 系統基本參數、時間、環境等 |
| 硬體校準區 | 20-35 | 16 bytes | 低 | 電極電壓、硬體校準、溫度/電池偏移等 |
| 血糖(GLV)參數區 | 36-198 | 163 bytes | 低 | 血糖相關所有測量參數 |
| 尿酸(U)參數區 | 199-331 | 133 bytes | 低 | 尿酸相關所有測量參數 |
| 總膽固醇(C)參數區 | 332-464 | 133 bytes | 低 | 膽固醇相關所有測量參數 |
| 三酸甘油脂(TG)參數區 | 465-597 | 133 bytes | 低 | 三酸甘油脂相關所有測量參數 |
| 血糖(GAV)參數區 | 598-808 | 211 bytes | 低 | GAV血糖測量參數 |
| 保留與校驗區 | 809-847 | 39 bytes | 高 | 保留區域和校驗參數 |

總大小：847 bytes，遠小於4KB的參數區大小，確保有足夠擴展空間。

## 4. 參數表資料結構設計

根據參數代碼表，重新設計參數存儲結構如下：

```c
#pragma pack(1)  // 確保結構體緊密封裝

// 參數表頭部結構
typedef struct {
    uint16_t magic;          // 魔術數字 (0xA55A)，表示有效的參數區
    uint16_t version;        // 參數表版本號，對應 Code_Table_V
    uint32_t writeCounter;   // 寫入次數計數器
    uint32_t timestamp;      // 最後更新時間戳記
} ParameterHeader;

// 基本系統參數區塊
typedef struct {
    // 系統基本參數 (9個參數)
    uint8_t lbt;             // 低電池閾值
    uint8_t obt;             // 電池耗盡閾值
    uint8_t factory;         // 儀器操作模式
    uint8_t modelNo;         // 產品型號
    uint8_t fwNo;            // 韌體版本號
    uint16_t testCount;      // 測試次數
    uint16_t codeTableVer;   // 代碼表版本
    
    // 時間設定參數 (6個參數)
    uint8_t year;            // 年份設定 (0-99 for 2000-2099)
    uint8_t month;           // 月份設定 (1-12)
    uint8_t date;            // 日期設定 (1-31)
    uint8_t hour;            // 小時設定 (0-23)
    uint8_t minute;          // 分鐘設定 (0-59)
    uint8_t second;          // 秒數設定 (0-59)
    
    // 測試環境參數 (5個參數)
    uint8_t tempLowLimit;    // 操作溫度下限
    uint8_t tempHighLimit;   // 操作溫度上限
    uint8_t measureUnit;     // 測量單位設定
    uint8_t defaultEvent;    // 預設事件
    uint8_t stripType;       // 測試項目類型
} BasicSystemBlock;

// 硬體校準參數區塊
typedef struct {
    uint16_t evT3Trigger;    // EV_T3觸發電壓
    uint8_t evWorking;       // 測量工作電極電壓
    uint8_t evT3;            // 血液檢測電極電壓
    uint8_t calTolerance;    // OPS/OPI校準容差
    float ops;               // OPA校準斜率
    float opi;               // OPA校準截距
    uint8_t qct;             // QCT校準測試低位元組
    int8_t tempOffset;       // 溫度校準偏移
    int8_t batteryOffset;    // 電池校準偏移
} HardwareCalibBlock;

// 血糖(GLV)專用參數區塊
typedef struct {
    // 試片參數 (12個參數)
    uint8_t glvCsuTolerance;     // 試片檢查容差
    uint16_t glvNdl;             // 新試片檢測水平
    uint16_t glvUdl;             // 已使用試片檢測水平
    uint16_t glvBloodIn;         // 血液檢測水平
    uint16_t glvEvWBackground;   // W電極背景值
    uint16_t glvEvWPwmDuty;      // W電極PWM占空比
    uint8_t glvCountDownTime;    // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t glvL;                // 血糖值下限
    uint8_t glvH;                // 血糖值上限
    
    // 測試時序參數 (12個參數)
    uint16_t glvTPL1;            // 時間脈衝低 (第一組)
    uint16_t glvTRD1;            // 原始數據時間 (第一組)
    uint16_t glvEVWidth1;        // 燃燒時間 (第一組)
    uint16_t glvTPL2;            // 時間脈衝低 (第二組)
    uint16_t glvTRD2;            // 原始數據時間 (第二組)
    uint16_t glvEVWidth2;        // 燃燒時間 (第二組)
    
    // 計算式補償參數 (110個參數)
    uint8_t glvS2;                // 試片補償參數
    uint8_t glvI2;                // 試片補償參數
    uint8_t glvSq;                // 品管液QC補償參數Sq
    uint8_t glvIq;                // 品管液QC補償參數Iq
    float glvSr;                  // OPA硬體迴路補償(斜率)
    float glvIr;                  // OPA硬體迴路補償(截距)
    uint16_t glvGLU[7];           // 多段校正參數(量測分段點)
    uint16_t glvGOAL[7];          // 多段校正參數(目標值分段點)
    uint8_t glvS3[30];            // 試片補償參數S3(1-30)
    uint8_t glvI3[30];            // 試片補償參數I3(1-30)
    
    // 溫度補償參數 (28個參數)
    uint8_t glvTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t glvCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t glvTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t glvCTO[7];            // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t glvCVq;               // CV Level of QC
    uint8_t glvAq;                // A of QC Compensation
    uint8_t glvBq;                // B of QC Compensation
    uint8_t glvtCV;               // Time of BG Optimize
    uint8_t glvCVbg;              // CV Level of BG
    uint8_t glvAbg;               // A of BG Compensation
    uint8_t glvBbg;               // B of BG Compensation
    uint8_t glvAq5;               // A of QC Compensation Level 5
    uint8_t glvBq5;               // B of QC Compensation Level 5
} GLVParameterBlock;

// 尿酸(U)專用參數區塊
typedef struct {
    // 試片參數 (12個參數)
    uint8_t uCsuTolerance;     // 試片檢查容差
    uint16_t uNdl;             // 新試片檢測水平
    uint16_t uUdl;             // 已使用試片檢測水平
    uint16_t uBloodIn;         // 血液檢測水平
    uint16_t uEvWBackground;   // W電極背景值
    uint16_t uEvWPwmDuty;      // W電極PWM占空比
    uint8_t uCountDownTime;    // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t uL;                // 尿酸值下限
    uint8_t uH;                // 尿酸值上限
    
    // 測試時序參數 (12個參數)
    uint16_t uTPL1;            // 時間脈衝低 (第一組)
    uint16_t uTRD1;            // 原始數據時間 (第一組)
    uint16_t uEVWidth1;        // 燃燒時間 (第一組)
    uint16_t uTPL2;            // 時間脈衝低 (第二組)
    uint16_t uTRD2;            // 原始數據時間 (第二組)
    uint16_t uEVWidth2;        // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t uS2;                // 試片補償參數
    uint8_t uI2;                // 試片補償參數
    uint8_t uSq;                // 品管液QC補償參數Sq
    uint8_t uIq;                // 品管液QC補償參數Iq
    float uSr;                  // OPA硬體迴路補償(斜率)
    float uIr;                  // OPA硬體迴路補償(截距)
    uint16_t uGLU[7];           // 多段校正參數(量測分段點)
    uint16_t uGOAL[7];          // 多段校正參數(目標值分段點)
    uint8_t uS3[15];            // 試片補償參數S3(1-15)
    uint8_t uI3[15];            // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t uTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t uCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t uTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t uCTO[7];            // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t uCVq;               // CV Level of QC
    uint8_t uAq;                // A of QC Compensation
    uint8_t uBq;                // B of QC Compensation
    uint8_t utCV;               // Time of BG Optimize
    uint8_t uCVbg;              // CV Level of BG
    uint8_t uAbg;               // A of BG Compensation
    uint8_t uBbg;               // B of BG Compensation
    uint8_t uAq5;               // A of QC Compensation Level 5
    uint8_t uBq5;               // B of QC Compensation Level 5
} UParameterBlock;

// 總膽固醇(C)專用參數區塊
typedef struct {
    // 試片參數 (12個參數)
    uint8_t cCsuTolerance;     // 試片檢查容差
    uint16_t cNdl;             // 新試片檢測水平
    uint16_t cUdl;             // 已使用試片檢測水平
    uint16_t cBloodIn;         // 血液檢測水平
    uint16_t cEvWBackground;   // W電極背景值
    uint16_t cEvWPwmDuty;      // W電極PWM占空比
    uint8_t cCountDownTime;    // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t cL;                // 膽固醇值下限
    uint8_t cH;                // 膽固醇值上限
    
    // 測試時序參數 (12個參數)
    uint16_t cTPL1;            // 時間脈衝低 (第一組)
    uint16_t cTRD1;            // 原始數據時間 (第一組)
    uint16_t cEVWidth1;        // 燃燒時間 (第一組)
    uint16_t cTPL2;            // 時間脈衝低 (第二組)
    uint16_t cTRD2;            // 原始數據時間 (第二組)
    uint16_t cEVWidth2;        // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t cS2;                // 試片補償參數
    uint8_t cI2;                // 試片補償參數
    uint8_t cSq;                // 品管液QC補償參數Sq
    uint8_t cIq;                // 品管液QC補償參數Iq
    float cSr;                  // OPA硬體迴路補償(斜率)
    float cIr;                  // OPA硬體迴路補償(截距)
    uint16_t cGLU[7];           // 多段校正參數(量測分段點)
    uint16_t cGOAL[7];          // 多段校正參數(目標值分段點)
    uint8_t cS3[15];            // 試片補償參數S3(1-15)
    uint8_t cI3[15];            // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t cTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t cCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t cTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t cCTO[7];            // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t cCVq;               // CV Level of QC
    uint8_t cAq;                // A of QC Compensation
    uint8_t cBq;                // B of QC Compensation
    uint8_t ctCV;               // Time of BG Optimize
    uint8_t cCVbg;              // CV Level of BG
    uint8_t cAbg;               // A of BG Compensation
    uint8_t cBbg;               // B of BG Compensation
    uint8_t cAq5;               // A of QC Compensation Level 5
    uint8_t cBq5;               // B of QC Compensation Level 5
} CParameterBlock;

// 三酸甘油脂(TG)專用參數區塊
typedef struct {
    // 試片參數 (12個參數)
    uint8_t tgCsuTolerance;     // 試片檢查容差
    uint16_t tgNdl;             // 新試片檢測水平
    uint16_t tgUdl;             // 已使用試片檢測水平
    uint16_t tgBloodIn;         // 血液檢測水平
    uint16_t tgEvWBackground;   // W電極背景值
    uint16_t tgEvWPwmDuty;      // W電極PWM占空比
    uint8_t tgCountDownTime;    // 倒數時間
    
    // 測量範圍參數 (2個參數)
    uint8_t tgL;                // 三酸甘油脂值下限
    uint8_t tgH;                // 三酸甘油脂值上限
    
    // 測試時序參數 (12個參數)
    uint16_t tgTPL1;            // 時間脈衝低 (第一組)
    uint16_t tgTRD1;            // 原始數據時間 (第一組)
    uint16_t tgEVWidth1;        // 燃燒時間 (第一組)
    uint16_t tgTPL2;            // 時間脈衝低 (第二組)
    uint16_t tgTRD2;            // 原始數據時間 (第二組)
    uint16_t tgEVWidth2;        // 燃燒時間 (第二組)
    
    // 計算式補償參數 (80個參數)
    uint8_t tgS2;                // 試片補償參數
    uint8_t tgI2;                // 試片補償參數
    uint8_t tgSq;                // 品管液QC補償參數Sq
    uint8_t tgIq;                // 品管液QC補償參數Iq
    float tgSr;                  // OPA硬體迴路補償(斜率)
    float tgIr;                  // OPA硬體迴路補償(截距)
    uint16_t tgGLU[7];           // 多段校正參數(量測分段點)
    uint16_t tgGOAL[7];          // 多段校正參數(目標值分段點)
    uint8_t tgS3[15];            // 試片補償參數S3(1-15)
    uint8_t tgI3[15];            // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t tgTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t tgCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t tgTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t tgCTO[7];            // QC溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t tgCVq;               // CV Level of QC
    uint8_t tgAq;                // A of QC Compensation
    uint8_t tgBq;                // B of QC Compensation
    uint8_t tgtCV;               // Time of BG Optimize
    uint8_t tgCVbg;              // CV Level of BG
    uint8_t tgAbg;               // A of BG Compensation
    uint8_t tgBbg;               // B of BG Compensation
    uint8_t tgAq5;               // A of QC Compensation Level 5
    uint8_t tgBq5;               // B of QC Compensation Level 5
} TGParameterBlock;

// 血糖(GAV)專用參數區塊
typedef struct {
    // 試片參數 (12個參數)
    uint8_t gavCsuTolerance;     // 試片檢查容差
    uint16_t gavNdl;             // 新試片檢測水平
    uint16_t gavUdl;             // 已使用試片檢測水平
    uint16_t gavBloodIn;         // 血液檢測水平
    uint16_t gavEvWBackground;   // W電極背景值
    uint16_t gavEvWPwmDuty;      // W電極PWM占空比
    uint8_t gavCountDownTime;    // 倒數時間
    
    // 測量範圍參數 (3個參數)
    uint8_t gavL;                // 血糖值下限
    uint8_t gavH;                // 血糖值上限
    uint16_t gavT3E37;           // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t gavTPL1;            // 時間脈衝低 (第一組)
    uint16_t gavTRD1;            // 原始數據時間 (第一組)
    uint16_t gavEVWidth1;        // 燃燒時間 (第一組)
    uint16_t gavTPL2;            // 時間脈衝低 (第二組)
    uint16_t gavTRD2;            // 原始數據時間 (第二組)
    uint16_t gavEVWidth2;        // 燃燒時間 (第二組)
    
    // 計算式補償參數 (110個參數)
    uint8_t gavS2;                // 試片補償參數
    uint8_t gavI2;                // 試片補償參數
    uint8_t gavSq;                // 品管液QC補償參數Sq
    uint8_t gavIq;                // 品管液QC補償參數Iq
    float gavSr;                  // OPA硬體迴路補償(斜率)
    float gavIr;                  // OPA硬體迴路補償(截距)
    uint16_t gavGLU[7];           // 多段校正參數(量測分段點)
    uint16_t gavGOAL[7];          // 多段校正參數(目標值分段點)
    uint8_t gavS3[30];            // 試片補償參數S3(1-30)
    uint8_t gavI3[30];            // 試片補償參數I3(1-30)
    
    // AC補償參數
    uint16_t gavAcLWaveL;         // AC振幅判斷
    uint16_t gavAcHWaveL;         // AC振幅判斷
    uint8_t gavS4[7];             // AC補償 (HCT10-70)
    uint8_t gavI4[7];             // AC補償 (HCT10-70)
    uint16_t gavAcAdc[7];         // AC補償判定 (HCT10-70)
    
    // 溫度補償參數 (42個參數)
    uint8_t gavTF[7];             // AC/PC溫度補償斜率(10-40°C)
    uint8_t gavCTF[7];            // QC溫度補償斜率(10-40°C)
    uint8_t gavTO[7];             // AC/PC溫度補償截距(10-40°C)
    uint8_t gavCTO[7];            // QC溫度補償截距(10-40°C)
    uint8_t gavAcTF[7];           // AC(交流)溫度補償斜率(10-40°C)
    uint8_t gavAcTO[7];           // AC(交流)溫度補償截距(10-40°C)
    
    // 計算後優化補償參數 (9個參數)
    uint8_t gavCVq;               // CV Level of QC
    uint8_t gavAq;                // A of QC Compensation
    uint8_t gavBq;                // B of QC Compensation
    uint8_t gavtCV;               // Time of BG Optimize
    uint8_t gavCVbg;              // CV Level of BG
    uint8_t gavAbg;               // A of BG Compensation
    uint8_t gavBbg;               // B of BG Compensation
    uint8_t gavAq5;               // A of QC Compensation Level 5
    uint8_t gavBq5;               // B of QC Compensation Level 5
} GAVParameterBlock;

// 保留與校驗區塊
typedef struct {
    uint8_t reserved[36];    // 保留區域供未來擴展
    uint16_t checksum;       // 參數校驗和
    uint8_t crc16;           // CRC16校驗
} ReservedAndChecksumBlock;

// 完整參數表結構
typedef struct {
    BasicSystemBlock        basicSystem;    // 基本系統參數區塊
    HardwareCalibBlock      hardwareCalib;  // 硬體校準參數區塊
    GLVParameterBlock       glvParams;      // 血糖(GLV)專用參數區塊
    UParameterBlock         uParams;        // 尿酸(U)專用參數區塊
    CParameterBlock         cParams;        // 總膽固醇(C)專用參數區塊
    TGParameterBlock        tgParams;       // 三酸甘油脂(TG)專用參數區塊
    GAVParameterBlock       gavParams;      // 血糖(GAV)專用參數區塊
    ReservedAndChecksumBlock reserveAndCsum;// 保留與校驗區塊
} ParameterTable;

// 完整參數區塊
typedef struct {
    ParameterHeader header;  // 參數表頭部
    ParameterTable params;   // 參數表內容
} ParameterBlock;

#pragma pack()  // 恢復默認對齊方式
```

