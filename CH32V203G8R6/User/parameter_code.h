/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_code.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀參數代碼表定義
 *********************************************************************************
 * Copyright (c) 2025 HMD.
 *******************************************************************************/

#ifndef __PARAMETER_CODE_H
#define __PARAMETER_CODE_H

#include "ch32v20x.h"

/* 參數類型定義 */
// 1. 系統基本參數
#define PARAM_LBT          0   // 低電池閾值
#define PARAM_OBT          1   // 電池耗盡閾值
#define PARAM_FACTORY      2   // 儀器操作模式
#define PARAM_MODEL_NO     3   // 產品型號
#define PARAM_FW_NO        4   // 韌體版本號
#define PARAM_NOT          5   // 測量次數 (2 bytes)
#define PARAM_CODE_TABLE_V 7   // 代碼表版本號 (2 bytes)

// 2. 時間設定參數
#define PARAM_YEAR         9   // 年份設定
#define PARAM_MONTH        10  // 月份設定
#define PARAM_DATE         11  // 日期設定
#define PARAM_HOUR         12  // 小時設定
#define PARAM_MINUTE       13  // 分鐘設定
#define PARAM_SECOND       14  // 秒數設定

// 3. 測量顯示參數
#define PARAM_TLL          15  // 操作溫度範圍下限
#define PARAM_TLH          16  // 操作溫度範圍上限
#define PARAM_MGDL         17  // 測量單位設定
#define PARAM_EVENT        18  // 調節事件類型
#define PARAM_STRIP_TYPE   19  // 測量項目

// 4. 硬體校準參數
#define PARAM_EV_T3_TRG    20  // EV_T3觸發電位 (2 bytes)
#define PARAM_EV_WORKING   22  // 測量工作電位電壓
#define PARAM_EV_T3        23  // 血液導入電位電壓
#define PARAM_DACO         24  // DAC偏移調感
#define PARAM_DACDO        25  // DAC校準管理參數
#define PARAM_CC211NoDone  26  // CC211未完成旗標
#define PARAM_CAL_TOL      27  // OPS/OPI校準容差
#define PARAM_OPS          28  // OPA校準斜率 (4 bytes)
#define PARAM_OPI          32  // OPA校準截距 (4 bytes)
#define PARAM_QCT          36  // QCT校準測試低位元組
#define PARAM_TOFFSET      37  // 溫度校準偏移
#define PARAM_BOFFSET      38  // 電池校準偏移

/* 試片類型定義 */
#define STRIP_TYPE_GLV     0   // 血糖
#define STRIP_TYPE_U       1   // 尿酸
#define STRIP_TYPE_C       2   // 總膽固醇
#define STRIP_TYPE_TG      3   // 三酸甘油脂
#define STRIP_TYPE_GAV     4   // 血糖

/* 全域參數表結構定義 */
#define PARAM_TABLE_SIZE   678  // 參數表總大小

/* 函數宣告 */
void Parameter_Init(void);
uint8_t Parameter_Read(uint16_t addr);
void Parameter_Write(uint16_t addr, uint8_t value);
uint16_t Parameter_Read16(uint16_t addr);
void Parameter_Write16(uint16_t addr, uint16_t value);
uint32_t Parameter_Read32(uint16_t addr);
void Parameter_Write32(uint16_t addr, uint32_t value);
void Parameter_Save(void);
void Parameter_Load(void);
uint8_t Parameter_CheckSum(void);
void Parameter_Reset(void);
void Parameter_SetDefaultValues(void);
const char* Parameter_GetStripTypeStr(uint8_t type);
void Parameter_PrintInfo(void);

#endif /* __PARAMETER_CODE_H */