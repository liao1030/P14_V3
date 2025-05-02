/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : 多功能生化測試儀參數代碼表定義頭文件
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 參數表總大小（位元組） */
#define PARAM_TABLE_SIZE            678

/* 參數區域偏移 */
#define PARAM_SYSTEM_OFFSET         0    // 系統基本參數 (0-8)
#define PARAM_TIME_OFFSET           9    // 時間設定參數 (9-14)
#define PARAM_ENV_OFFSET            15   // 測試環境參數 (15-19)
#define PARAM_CAL_OFFSET            20   // 硬體校準參數 (20-38)
#define PARAM_BG_OFFSET             39   // 血糖專用參數 (39-212)
#define PARAM_UA_OFFSET             213  // 尿酸專用參數 (213-354)
#define PARAM_CHOL_OFFSET           355  // 總膽固醇專用參數 (355-496)
#define PARAM_TG_OFFSET             497  // 三酸甘油脂專用參數 (497-638)
#define PARAM_RESERVED_OFFSET       639  // 保留區域 (639-674)
#define PARAM_CHECKSUM_OFFSET       675  // 校驗區 (675-677)

/* 測試項目類型 */
#define STRIP_TYPE_GLV              0    // 血糖(GLV)
#define STRIP_TYPE_UA               1    // 尿酸(U)
#define STRIP_TYPE_CHOL             2    // 總膽固醇(C)
#define STRIP_TYPE_TG               3    // 三酸甘油脂(TG)
#define STRIP_TYPE_GAV              4    // 血糖(GAV)

/* 測量單位設定 */
#define UNIT_MMOL_L                 0    // mmol/L
#define UNIT_MG_DL                  1    // mg/dL
#define UNIT_GM_DL                  2    // gm/dl

/* 事件類型 */
#define EVENT_QC                    0    // 品管液測試
#define EVENT_AC                    1    // 餐前測試
#define EVENT_PC                    2    // 餐後測試

/* 儀器操作模式 */
#define MODE_USER                   0    // 使用者模式
#define MODE_FACTORY                210  // 工廠模式

/* 參數表結構體定義 */
typedef struct {
    /* 1. 系統基本參數 */
    uint8_t  LBT;                       // 低電池閾值
    uint8_t  OBT;                       // 電池耗盡閾值
    uint8_t  FACTORY;                   // 儀器操作模式
    uint8_t  MODEL_NO;                  // 產品型號
    uint8_t  FW_NO;                     // 韌體版本號
    uint16_t NOT;                       // 測試次數
    uint16_t Code_Table_V;              // 代碼表版本編號

    /* 2. 時間設定參數 */
    uint8_t  YEAR;                      // 年份設定 (0-99, 2000-2099年)
    uint8_t  MONTH;                     // 月份設定 (1-12)
    uint8_t  DATE;                      // 日期設定 (1-31)
    uint8_t  HOUR;                      // 小時設定 (0-23)
    uint8_t  MINUTE;                    // 分鐘設定 (0-59)
    uint8_t  SECOND;                    // 秒數設定 (0-59)

    /* 3. 測試環境參數 */
    uint8_t  TLL;                       // 操作溫度範圍下限
    uint8_t  TLH;                       // 操作溫度範圍上限
    uint8_t  MGDL;                      // 測量單位設定
    uint8_t  EVENT;                     // 預設事件類型
    uint8_t  Strip_Type;                // 測試項目

    /* 4. 硬體校準參數 */
    uint16_t EV_T3_TRG;                 // EV_T3觸發電壓
    uint8_t  EV_WORKING;                // 測量工作電極電壓
    uint8_t  EV_T3;                     // 血液檢測電極電壓
    uint8_t  DACO;                      // DAC偏移補償
    uint8_t  DACDO;                     // DAC校正管理參數
    uint8_t  CC211NoDone;               // CC211未完成旗標
    uint8_t  CAL_TOL;                   // OPS/OPI校準容差
    float    OPS;                       // OPA校準斜率
    float    OPI;                       // OPA校準截距
    uint8_t  QCT;                       // QCT校準測試低位元組
    uint8_t  TOFFSET;                   // 溫度校準偏移
    uint8_t  BOFFSET;                   // 電池校準偏移

    /* 5. 血糖(GLV/GAV)專用參數 */
    /* 5.1 試片參數 */
    uint8_t  BG_CSU_TOL;                // 試片檢查容差
    uint16_t BG_NDL;                    // 新試片檢測水平
    uint16_t BG_UDL;                    // 已使用試片檢測水平
    uint16_t BG_BLOOD_IN;               // 血液檢測水平
    uint8_t  BG_Strip_Lot[16];          // 試片批號
    
    /* 5.2 測量範圍參數 */
    uint8_t  BG_L;                      // 血糖值下限
    uint16_t  BG_H;                      // 血糖值上限
    uint16_t BG_T3_E37;                 // T3 ADV錯誤37閾值

    /* 5.3 測試時序參數 */
    uint16_t BG_TPL_1;                  // 時間脈衝低(第一組)
    uint16_t BG_TRD_1;                  // 原始數據時間(第一組)
    uint16_t BG_EV_Width_1;             // 燃燒時間(第一組)
    uint16_t BG_TPL_2;                  // 時間脈衝低(第二組)
    uint16_t BG_TRD_2;                  // 原始數據時間(第二組)
    uint16_t BG_EV_Width_2;             // 燃燒時間(第二組)

    /* 5.4 品管液(QC)參數 */
    uint8_t  BG_Sq;                     // QC補償參數Sq
    uint8_t  BG_Iq;                     // QC補償參數Iq
    uint8_t  BG_CVq;                    // QC水平的CV
    uint8_t  BG_Aq;                     // QC補償參數A
    uint8_t  BG_Bq;                     // QC補償參數B
    uint8_t  BG_Aq5;                    // QC補償參數A(等級5)
    uint8_t  BG_Bq5;                    // QC補償參數B(等級5)
    uint8_t  BG_QC[30];                 // QC標準範圍參數

    /* 5.5 計算式補償參數 */
    uint8_t  BG_S2;                     // 試片補償參數
    uint8_t  BG_I2;                     // 試片補償參數
    float    BG_Sr;                     // OPA硬體迴路補償(斜率)
    float    BG_Ir;                     // OPA硬體迴路補償(截距)
    uint8_t  BG_S3[30];                 // 試片補償參數S3(1-30)
    uint8_t  BG_I3[30];                 // 試片補償參數I3(1-30)

    /* 5.6 溫度補償參數 */
    uint8_t  BG_TF[7];                  // AC/PC溫度補償斜率(10-40°C)
    uint8_t  BG_CTF[7];                 // QC溫度補償斜率(10-40°C)
    uint8_t  BG_TO[7];                  // AC/PC溫度補償截距(10-40°C)
    uint8_t  BG_CTO[7];                 // QC溫度補償截距(10-40°C)

    /* 6. 尿酸(U)專用參數 */
    /* 6.1 試片參數 */
    uint8_t  U_CSU_TOL;                 // 試片檢查容差
    uint16_t U_NDL;                     // 新試片檢測水平
    uint16_t U_UDL;                     // 已使用試片檢測水平
    uint16_t U_BLOOD_IN;                // 血液檢測水平
    uint8_t  U_Strip_Lot[16];           // 試片批號
    
    /* 6.2 測量範圍參數 */
    uint8_t  U_L;                       // 尿酸值下限
    uint16_t  U_H;                       // 尿酸值上限
    uint16_t U_T3_E37;                  // T3 ADV錯誤37閾值

    /* 6.3 測試時序參數 */
    uint16_t U_TPL_1;                   // 時間脈衝低(第一組)
    uint16_t U_TRD_1;                   // 原始數據時間(第一組)
    uint16_t U_EV_Width_1;              // 燃燒時間(第一組)
    uint16_t U_TPL_2;                   // 時間脈衝低(第二組)
    uint16_t U_TRD_2;                   // 原始數據時間(第二組)
    uint16_t U_EV_Width_2;              // 燃燒時間(第二組)

    /* 6.4 品管液(QC)參數 */
    uint8_t  U_Sq;                      // QC補償參數Sq
    uint8_t  U_Iq;                      // QC補償參數Iq
    uint8_t  U_CVq;                     // QC水平的CV
    uint8_t  U_Aq;                      // QC補償參數A
    uint8_t  U_Bq;                      // QC補償參數B
    uint8_t  U_QC[30];                  // QC標準範圍參數

    /* 6.5 計算式補償參數 */
    uint8_t  U_S2;                      // 試片補償參數
    uint8_t  U_I2;                      // 試片補償參數
    float    U_Sr;                      // OPA硬體迴路補償(斜率)
    float    U_Ir;                      // OPA硬體迴路補償(截距)
    uint8_t  U_S3[15];                  // 試片補償參數S3(1-15)
    uint8_t  U_I3[15];                  // 試片補償參數I3(1-15)

    /* 6.6 溫度補償參數 */
    uint8_t  U_TF[7];                   // AC/PC溫度補償斜率(10-40°C)
    uint8_t  U_CTF[7];                  // QC溫度補償斜率(10-40°C)
    uint8_t  U_TO[7];                   // AC/PC溫度補償截距(10-40°C)
    uint8_t  U_CTO[7];                  // QC溫度補償截距(10-40°C)

    /* 7. 總膽固醇(C)專用參數 */
    /* 7.1 試片參數 */
    uint8_t  C_CSU_TOL;                 // 試片檢查容差
    uint16_t C_NDL;                     // 新試片檢測水平
    uint16_t C_UDL;                     // 已使用試片檢測水平
    uint16_t C_BLOOD_IN;                // 血液檢測水平
    uint8_t  C_Strip_Lot[16];           // 試片批號
    
    /* 7.2 測量範圍參數 */
    uint8_t  C_L;                       // 膽固醇值下限
    uint16_t  C_H;                       // 膽固醇值上限
    uint16_t C_T3_E37;                  // T3 ADV錯誤37閾值

    /* 7.3 測試時序參數 */
    uint16_t C_TPL_1;                   // 時間脈衝低(第一組)
    uint16_t C_TRD_1;                   // 原始數據時間(第一組)
    uint16_t C_EV_Width_1;              // 燃燒時間(第一組)
    uint16_t C_TPL_2;                   // 時間脈衝低(第二組)
    uint16_t C_TRD_2;                   // 原始數據時間(第二組)
    uint16_t C_EV_Width_2;              // 燃燒時間(第二組)

    /* 7.4 品管液(QC)參數 */
    uint8_t  C_Sq;                      // QC補償參數Sq
    uint8_t  C_Iq;                      // QC補償參數Iq
    uint8_t  C_CVq;                     // QC水平的CV
    uint8_t  C_Aq;                      // QC補償參數A
    uint8_t  C_Bq;                      // QC補償參數B
    uint8_t  C_QC[30];                  // QC標準範圍參數

    /* 7.5 計算式補償參數 */
    uint8_t  C_S2;                      // 試片補償參數
    uint8_t  C_I2;                      // 試片補償參數
    float    C_Sr;                      // OPA硬體迴路補償(斜率)
    float    C_Ir;                      // OPA硬體迴路補償(截距)
    uint8_t  C_S3[15];                  // 試片補償參數S3(1-15)
    uint8_t  C_I3[15];                  // 試片補償參數I3(1-15)

    /* 7.6 溫度補償參數 */
    uint8_t  C_TF[7];                   // AC/PC溫度補償斜率(10-40°C)
    uint8_t  C_CTF[7];                  // QC溫度補償斜率(10-40°C)
    uint8_t  C_TO[7];                   // AC/PC溫度補償截距(10-40°C)
    uint8_t  C_CTO[7];                  // QC溫度補償截距(10-40°C)

    /* 8. 三酸甘油脂(TG)專用參數 */
    /* 8.1 試片參數 */
    uint8_t  TG_CSU_TOL;                // 試片檢查容差
    uint16_t TG_NDL;                    // 新試片檢測水平
    uint16_t TG_UDL;                    // 已使用試片檢測水平
    uint16_t TG_BLOOD_IN;               // 血液檢測水平
    uint8_t  TG_Strip_Lot[16];          // 試片批號
    
    /* 8.2 測量範圍參數 */
    uint8_t  TG_L;                      // 三酸甘油脂值下限
    uint16_t  TG_H;                      // 三酸甘油脂值上限
    uint16_t TG_T3_E37;                 // T3 ADV錯誤37閾值

    /* 8.3 測試時序參數 */
    uint16_t TG_TPL_1;                  // 時間脈衝低(第一組)
    uint16_t TG_TRD_1;                  // 原始數據時間(第一組)
    uint16_t TG_EV_Width_1;             // 燃燒時間(第一組)
    uint16_t TG_TPL_2;                  // 時間脈衝低(第二組)
    uint16_t TG_TRD_2;                  // 原始數據時間(第二組)
    uint16_t TG_EV_Width_2;             // 燃燒時間(第二組)

    /* 8.4 品管液(QC)參數 */
    uint8_t  TG_Sq;                     // QC補償參數Sq
    uint8_t  TG_Iq;                     // QC補償參數Iq
    uint8_t  TG_CVq;                    // QC水平的CV
    uint8_t  TG_Aq;                     // QC補償參數A
    uint8_t  TG_Bq;                     // QC補償參數B
    uint8_t  TG_QC[30];                 // QC標準範圍參數

    /* 8.5 計算式補償參數 */
    uint8_t  TG_S2;                     // 試片補償參數
    uint8_t  TG_I2;                     // 試片補償參數
    float    TG_Sr;                     // OPA硬體迴路補償(斜率)
    float    TG_Ir;                     // OPA硬體迴路補償(截距)
    uint8_t  TG_S3[15];                 // 試片補償參數S3(1-15)
    uint8_t  TG_I3[15];                 // 試片補償參數I3(1-15)

    /* 8.6 溫度補償參數 */
    uint8_t  TG_TF[7];                  // AC/PC溫度補償斜率(10-40°C)
    uint8_t  TG_CTF[7];                 // QC溫度補償斜率(10-40°C)
    uint8_t  TG_TO[7];                  // AC/PC溫度補償截距(10-40°C)
    uint8_t  TG_CTO[7];                 // QC溫度補償截距(10-40°C)

    /* 9. 保留與校驗區 */
    uint8_t  Reserved[36];              // 保留區域供未來擴展
    uint8_t  SUM_L;                     // 地址0~674校驗和(低)
    uint8_t  SUM_H;                     // 地址0~674校驗和(高)
    uint8_t  CRC16;                     // 完整參數表CRC校驗
} ParamTable_TypeDef;

/* 函數宣告 */
void ParamTable_Init(ParamTable_TypeDef *param_table);
uint16_t ParamTable_CalculateChecksum(ParamTable_TypeDef *param_table);
uint8_t ParamTable_CalculateCRC16(ParamTable_TypeDef *param_table);
uint8_t ParamTable_Verify(ParamTable_TypeDef *param_table);
void ParamTable_SaveToFlash(ParamTable_TypeDef *param_table);
uint8_t ParamTable_LoadFromFlash(ParamTable_TypeDef *param_table);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */ 