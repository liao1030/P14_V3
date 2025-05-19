/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table_example.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數表使用範例
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_table.h"
#include "ch32v20x.h"
#include <stdio.h>

/*********************************************************************
 * @fn      DisplaySystemParameters
 *
 * @brief   顯示系統參數
 *
 * @return  none
 */
void DisplaySystemParameters(void)
{
    /* 檢查參數表是否已初始化 */
    if (!PARAM_IsInitialized()) {
        printf("參數表尚未初始化!\r\n");
        return;
    }
    
    /* 顯示參數表標頭信息 */
    printf("參數表版本: %u\r\n", g_pParamTable->header.version);
    printf("參數表寫入次數: %lu\r\n", g_pParamTable->header.writeCounter);
    
    /* 顯示基本系統參數 */
    printf("\r\n基本系統參數:\r\n");
    printf("低電池閾值: %.2fV\r\n", g_pParamTable->basic.lbt / 100.0f);
    printf("電池耗盡閾值: %.2fV\r\n", g_pParamTable->basic.obt / 100.0f);
    printf("操作模式: %s\r\n", g_pParamTable->basic.factory == 0 ? "使用者" : "工廠");
    printf("產品型號: %u\r\n", g_pParamTable->basic.modelNo);
    printf("韌體版本: %u\r\n", g_pParamTable->basic.fwNo);
    printf("測試次數: %u\r\n", g_pParamTable->basic.testCount);
    printf("代碼表版本: %u\r\n", g_pParamTable->basic.codeTableVer);
    
    /* 顯示時間設定 */
    printf("\r\n時間設定:\r\n");
    printf("日期: 20%02u/%02u/%02u\r\n", 
           g_pParamTable->basic.year,
           g_pParamTable->basic.month,
           g_pParamTable->basic.date);
    printf("時間: %02u:%02u:%02u\r\n", 
           g_pParamTable->basic.hour,
           g_pParamTable->basic.minute,
           g_pParamTable->basic.second);
           
    /* 顯示測試環境參數 */
    printf("\r\n測試環境參數:\r\n");
    printf("操作溫度範圍: %u.0°C ~ %u.0°C\r\n", 
           g_pParamTable->basic.tempLowLimit,
           g_pParamTable->basic.tempHighLimit);
    
    /* 顯示測量單位設定 */
    printf("測量單位: ");
    switch (g_pParamTable->basic.measureUnit) {
        case 0:
            printf("mmol/L\r\n");
            break;
        case 1:
            printf("mg/dL\r\n");
            break;
        case 2:
            printf("gm/dL\r\n");
            break;
        default:
            printf("未知\r\n");
    }
    
    /* 顯示預設事件 */
    printf("預設事件: ");
    switch (g_pParamTable->basic.defaultEvent) {
        case 0:
            printf("QC (品管液)\r\n");
            break;
        case 1:
            printf("AC (餐前)\r\n");
            break;
        case 2:
            printf("PC (餐後)\r\n");
            break;
        default:
            printf("未知\r\n");
    }
    
    /* 顯示當前測試項目類型 */
    printf("測試項目類型: ");
    switch (g_pParamTable->basic.stripType) {
        case STRIP_TYPE_GLV:
            printf("GLV (血糖)\r\n");
            break;
        case STRIP_TYPE_U:
            printf("U (尿酸)\r\n");
            break;
        case STRIP_TYPE_C:
            printf("C (總膽固醇)\r\n");
            break;
        case STRIP_TYPE_TG:
            printf("TG (三酸甘油脂)\r\n");
            break;
        case STRIP_TYPE_GAV:
            printf("GAV (血糖AC)\r\n");
            break;
        default:
            printf("未知\r\n");
    }
}

/*********************************************************************
 * @fn      UpdateSystemTime
 *
 * @brief   更新系統時間
 *
 * @param   year - 年份(0-99)
 * @param   month - 月份(1-12)
 * @param   date - 日期(1-31)
 * @param   hour - 小時(0-23)
 * @param   minute - 分鐘(0-59)
 * @param   second - 秒(0-59)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef UpdateSystemTime(uint8_t year, uint8_t month, uint8_t date, 
                                 uint8_t hour, uint8_t minute, uint8_t second)
{
    ParamError_TypeDef result;
    
    /* 驗證時間參數 */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 更新參數表中的時間設定 */
    result = PARAM_SetValue(9, &year, 1);   // YEAR
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(10, &month, 1);  // MONTH
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(11, &date, 1);   // DATE
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(12, &hour, 1);   // HOUR
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(13, &minute, 1); // MINUTE
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(14, &second, 1); // SECOND
    if (result != PARAM_OK) return result;
    
    /* 將更改保存到Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* 同時更新備份區 */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      SetMeasurementUnit
 *
 * @brief   設置測量單位
 *
 * @param   unit - 測量單位(0:mmol/L, 1:mg/dL, 2:gm/dL)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef SetMeasurementUnit(uint8_t unit)
{
    ParamError_TypeDef result;
    
    /* 驗證單位參數 */
    if (unit > 2) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 更新參數表中的測量單位 */
    result = PARAM_SetValue(17, &unit, 1);   // MGDL
    if (result != PARAM_OK) return result;
    
    /* 將更改保存到Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* 同時更新備份區 */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      SetStripType
 *
 * @brief   設置測試項目類型
 *
 * @param   type - 測試項目類型
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef SetStripType(StripType_TypeDef type)
{
    ParamError_TypeDef result;
    uint8_t typeValue = (uint8_t)type;
    
    /* 驗證測試項目類型 */
    if (typeValue > STRIP_TYPE_GAV) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 更新參數表中的測試項目類型 */
    result = PARAM_SetValue(19, &typeValue, 1);   // Strip_Type
    if (result != PARAM_OK) return result;
    
    /* 將更改保存到Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* 同時更新備份區 */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      IncrementTestCount
 *
 * @brief   增加測試次數
 *
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef IncrementTestCount(void)
{
    ParamError_TypeDef result;
    uint16_t testCount;
    
    /* 讀取當前測試次數 */
    result = PARAM_GetValue(5, &testCount, 2);   // NOT
    if (result != PARAM_OK) return result;
    
    /* 增加測試次數 */
    testCount++;
    
    /* 更新測試次數 */
    result = PARAM_SetValue(5, &testCount, 2);   // NOT
    if (result != PARAM_OK) return result;
    
    /* 將更改保存到Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* 同時更新備份區 */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      ParamTableExample
 *
 * @brief   參數表使用範例
 *
 * @return  none
 */
void ParamTableExample(void)
{
    ParamError_TypeDef result;
    
    /* 初始化參數表 */
    result = PARAM_Init();
    if (result != PARAM_OK) {
        printf("參數表初始化失敗! 錯誤代碼: %d\r\n", result);
        return;
    }
    
    printf("參數表初始化成功!\r\n\r\n");
    
    /* 顯示系統參數 */
    DisplaySystemParameters();
    
    printf("\r\n開始執行範例操作:\r\n");
    
    /* 更新系統時間 */
    printf("更新系統時間...\r\n");
    result = UpdateSystemTime(25, 5, 19, 14, 30, 0);
    if (result != PARAM_OK) {
        printf("更新時間失敗! 錯誤代碼: %d\r\n", result);
    }
    
    /* 設置測量單位 */
    printf("設置測量單位為mg/dL...\r\n");
    result = SetMeasurementUnit(1);
    if (result != PARAM_OK) {
        printf("設置測量單位失敗! 錯誤代碼: %d\r\n", result);
    }
    
    /* 設置測試項目類型 */
    printf("設置測試項目為尿酸(U)...\r\n");
    result = SetStripType(STRIP_TYPE_U);
    if (result != PARAM_OK) {
        printf("設置測試項目失敗! 錯誤代碼: %d\r\n", result);
    }
    
    /* 增加測試次數 */
    printf("增加測試次數...\r\n");
    result = IncrementTestCount();
    if (result != PARAM_OK) {
        printf("增加測試次數失敗! 錯誤代碼: %d\r\n", result);
    }
    
    printf("\r\n更新後的參數:\r\n");
    DisplaySystemParameters();
    
    printf("\r\n參數表使用範例完成!\r\n");
}
