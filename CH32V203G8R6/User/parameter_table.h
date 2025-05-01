/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_table.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : 多功能生化測試儀參數代碼表頭文件
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAMETER_TABLE_H__
#define __PARAMETER_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* 參數表總大小 */
#define PARAM_TABLE_SIZE           678  // 總參數大小 (0-677)

/* 測試項目類型定義 */
#define STRIP_TYPE_GLV             0    // 血糖
#define STRIP_TYPE_U               1    // 尿酸
#define STRIP_TYPE_C               2    // 總膽固醇
#define STRIP_TYPE_TG              3    // 三酸甘油脂
#define STRIP_TYPE_GAV             4    // 血糖(另一種)

/* 系統基本參數地址 */
#define PARAM_LBT                  0    // 低電池閾值
#define PARAM_OBT                  1    // 電池耗盡閾值
#define PARAM_FACTORY              2    // 儀器操作模式
#define PARAM_MODEL_NO             3    // 產品型號
#define PARAM_FW_NO                4    // 韌體版本號
#define PARAM_NOT                  5    // 測試次數 (2 bytes)
#define PARAM_CODE_TABLE_V         7    // 代碼表版本號 (2 bytes)

/* 時間設定參數地址 */
#define PARAM_YEAR                 9    // 年份設定
#define PARAM_MONTH                10   // 月份設定
#define PARAM_DATE                 11   // 日期設定
#define PARAM_HOUR                 12   // 小時設定
#define PARAM_MINUTE               13   // 分鐘設定
#define PARAM_SECOND               14   // 秒數設定

/* 測試環境參數地址 */
#define PARAM_TLL                  15   // 操作溫度範圍下限
#define PARAM_TLH                  16   // 操作溫度範圍上限
#define PARAM_MGDL                 17   // 濃度單位設定
#define PARAM_EVENT                18   // 餐飲事件類型
#define PARAM_STRIP_TYPE           19   // 測試項目

/* 硬體校正參數地址 */
#define PARAM_EV_T3_TRG            20   // EV_T3觸發電壓 (2 bytes)
#define PARAM_EV_WORKING           22   // 濃度工作電流電壓
#define PARAM_EV_T3                23   // 血液消耗電流電壓
#define PARAM_DACO                 24   // DAC偏移補償
#define PARAM_DACDO                25   // DAC校正管理參數
#define PARAM_CC211NoDone          26   // CC211未完成旗標
#define PARAM_CAL_TOL              27   // OPS/OPI校正容差
#define PARAM_OPS                  28   // OPA校正斜率 (4 bytes)
#define PARAM_OPI                  32   // OPA校正截距 (4 bytes)
#define PARAM_QCT                  36   // QCT校正測試低位元組
#define PARAM_TOFFSET              37   // 溫度校正偏移
#define PARAM_BOFFSET              38   // 電池校正偏移

/* 血糖(GLV/GAV)專用參數地址 */
#define PARAM_BG_CSU_TOL           39   // 試片檢查容差
#define PARAM_BG_NDL               40   // 新試片濃度水平 (2 bytes)
#define PARAM_BG_UDL               42   // 已使用試片濃度水平 (2 bytes)
#define PARAM_BG_BLOOD_IN          44   // 血液濃度水平 (2 bytes)
#define PARAM_BG_STRIP_LOT         46   // 試片批號 (16 bytes)
#define PARAM_BG_L                 62   // 血糖值下限
#define PARAM_BG_H                 63   // 血糖值上限
#define PARAM_BG_T3_E37            64   // T3 ADV超E37閾值 (2 bytes)

/* 尿酸(U)專用參數地址 */
#define PARAM_U_CSU_TOL            213  // 試片檢查容差
#define PARAM_U_NDL                214  // 新試片濃度水平 (2 bytes)
#define PARAM_U_UDL                216  // 已使用試片濃度水平 (2 bytes)
#define PARAM_U_BLOOD_IN           218  // 血液濃度水平 (2 bytes)
#define PARAM_U_STRIP_LOT          220  // 試片批號 (16 bytes)
#define PARAM_U_L                  236  // 尿酸值下限
#define PARAM_U_H                  237  // 尿酸值上限
#define PARAM_U_T3_E37             238  // T3 ADV超E37閾值 (2 bytes)

/* 總膽固醇(C)專用參數地址 */
#define PARAM_C_CSU_TOL            355  // 試片檢查容差
#define PARAM_C_NDL                356  // 新試片濃度水平 (2 bytes)
#define PARAM_C_UDL                358  // 已使用試片濃度水平 (2 bytes)
#define PARAM_C_BLOOD_IN           360  // 血液濃度水平 (2 bytes)
#define PARAM_C_STRIP_LOT          362  // 試片批號 (16 bytes)
#define PARAM_C_L                  378  // 膽固醇值下限
#define PARAM_C_H                  379  // 膽固醇值上限
#define PARAM_C_T3_E37             380  // T3 ADV超E37閾值 (2 bytes)

/* 三酸甘油脂(TG)專用參數地址 */
#define PARAM_TG_CSU_TOL           497  // 試片檢查容差
#define PARAM_TG_NDL               498  // 新試片濃度水平 (2 bytes)
#define PARAM_TG_UDL               500  // 已使用試片濃度水平 (2 bytes)
#define PARAM_TG_BLOOD_IN          502  // 血液濃度水平 (2 bytes)
#define PARAM_TG_STRIP_LOT         504  // 試片批號 (16 bytes)
#define PARAM_TG_L                 520  // 三酸甘油脂值下限
#define PARAM_TG_H                 521  // 三酸甘油脂值上限
#define PARAM_TG_T3_E37            522  // T3 ADV超E37閾值 (2 bytes)

/* 校驗參數地址 */
#define PARAM_SUM_L                675  // 地址0~674校驗和(低)
#define PARAM_SUM_H                676  // 地址0~674校驗和(高) 
#define PARAM_CRC16                677  // 完整參數表CRC校驗

/* 參數表結構定義 */
typedef struct {
    uint8_t data[PARAM_TABLE_SIZE];
} ParameterTable_TypeDef;

/* 函數宣告 */
void PARAM_Init(void);
uint8_t PARAM_Read(uint16_t addr);
void PARAM_Write(uint16_t addr, uint8_t value);
uint16_t PARAM_Read16(uint16_t addr);
void PARAM_Write16(uint16_t addr, uint16_t value);
void PARAM_ReadBlock(uint16_t addr, uint8_t *buffer, uint16_t size);
void PARAM_WriteBlock(uint16_t addr, const uint8_t *buffer, uint16_t size);
uint8_t PARAM_VerifyChecksum(void);
void PARAM_UpdateChecksum(void);
void PARAM_Reset(void);
void PARAM_Save(void);
uint8_t PARAM_Load(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAMETER_TABLE_H__ */ 