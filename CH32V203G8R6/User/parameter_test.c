/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_test.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : Flash參數儲存方案測試程式
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_test.h"
#include "P14_Flash_Storage.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

/**
 * @brief 獲取系統時間計數器值 (毫秒)
 * @return 系統時間計數
 */
static uint32_t GET_TICK_COUNT(void)
{
    // 使用靜態變數來記錄經過的毫秒數
    static uint32_t tick_count = 0;
    
    // 每次調用增加1毫秒，實際應用中應使用適合的計時器
    tick_count++;
    
    return tick_count;
}

/**
 * @brief 顯示各種測試項目的主要參數
 */
void PARAM_Test_PrintMainParameters(void)
{
    printf("\r\n===== 各測試項目主要參數 =====\r\n");
    
    /* 顯示血糖參數 */
    printf("\r\n[血糖參數]\r\n");
    printf("試片檢查容差: %d %%\r\n", PARAM_Read(PARAM_BG_CSU_TOL) / 10);
    printf("血糖值下限: %d mg/dL\r\n", PARAM_Read(PARAM_BG_L) * 10);
    printf("血糖值上限: %d mg/dL\r\n", PARAM_Read(PARAM_BG_H) * 10);
    
    char stripLot[17] = {0};
    PARAM_ReadBlock(PARAM_BG_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("試片批號: %s\r\n", stripLot);
    
    /* 顯示尿酸參數 */
    printf("\r\n[尿酸參數]\r\n");
    printf("試片檢查容差: %d %%\r\n", PARAM_Read(PARAM_U_CSU_TOL) / 10);
    printf("尿酸值下限: %d mg/dL\r\n", PARAM_Read(PARAM_U_L));
    printf("尿酸值上限: %d mg/dL\r\n", PARAM_Read(PARAM_U_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_U_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("試片批號: %s\r\n", stripLot);
    
    /* 顯示總膽固醇參數 */
    printf("\r\n[總膽固醇參數]\r\n");
    printf("試片檢查容差: %d %%\r\n", PARAM_Read(PARAM_C_CSU_TOL) / 10);
    printf("膽固醇值下限: %d mg/dL\r\n", PARAM_Read(PARAM_C_L));
    printf("膽固醇值上限: %d mg/dL\r\n", PARAM_Read(PARAM_C_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_C_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("試片批號: %s\r\n", stripLot);
    
    /* 顯示三酸甘油脂參數 */
    printf("\r\n[三酸甘油脂參數]\r\n");
    printf("試片檢查容差: %d %%\r\n", PARAM_Read(PARAM_TG_CSU_TOL) / 10);
    printf("三酸甘油脂值下限: %d mg/dL\r\n", PARAM_Read(PARAM_TG_L));
    printf("三酸甘油脂值上限: %d mg/dL\r\n", PARAM_Read(PARAM_TG_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_TG_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("試片批號: %s\r\n", stripLot);
}

/**
 * @brief 設置試片批號
 * @param stripType: 試片類型
 * @param lotNumber: 批號字串
 */
void PARAM_Test_SetStripLot(uint8_t stripType, const char* lotNumber)
{
    uint16_t lotAddr = 0;
    
    switch(stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            lotAddr = PARAM_BG_STRIP_LOT;
            break;
        case STRIP_TYPE_U:
            lotAddr = PARAM_U_STRIP_LOT;
            break;
        case STRIP_TYPE_C:
            lotAddr = PARAM_C_STRIP_LOT;
            break;
        case STRIP_TYPE_TG:
            lotAddr = PARAM_TG_STRIP_LOT;
            break;
        default:
            printf("未知試片類型!\r\n");
            return;
    }
    
    // 填充批號
    char paddedLot[16] = {0};
    strncpy(paddedLot, lotNumber, 15);
    
    // 寫入批號
    PARAM_WriteBlock(lotAddr, (const uint8_t*)paddedLot, 16);
    
    // 更新校驗和
    PARAM_UpdateChecksum();
    
    printf("已設置批號: %s\r\n", lotNumber);
}

/**
 * @brief 更新測試次數
 */
void PARAM_Test_UpdateTestCount(void)
{
    uint16_t count = PARAM_Read16(PARAM_NOT);
    count++;
    PARAM_Write16(PARAM_NOT, count);
    PARAM_UpdateChecksum();
    printf("已更新測試次數: %d\r\n", count);
}

/**
 * @brief 設置操作溫度範圍
 * @param lower: 下限溫度 (°C)
 * @param upper: 上限溫度 (°C)
 */
void PARAM_Test_SetTemperatureRange(uint8_t lower, uint8_t upper)
{
    if (lower >= upper) {
        printf("溫度範圍設置錯誤!\r\n");
        return;
    }
    
    PARAM_Write(PARAM_TLL, lower);
    PARAM_Write(PARAM_TLH, upper);
    PARAM_UpdateChecksum();
    
    printf("已設置操作溫度範圍: %d°C ~ %d°C\r\n", lower, upper);
}

/**
 * @brief 設置濃度單位
 * @param unit: 0:mmol/L, 1:mg/dL, 2:gm/dl
 */
void PARAM_Test_SetConcentrationUnit(uint8_t unit)
{
    if (unit > 2) {
        printf("單位設置錯誤!\r\n");
        return;
    }
    
    PARAM_Write(PARAM_MGDL, unit);
    PARAM_UpdateChecksum();
    
    const char* unitStr[] = {"mmol/L", "mg/dL", "gm/dl"};
    printf("已設置濃度單位: %s\r\n", unitStr[unit]);
}

/**
 * @brief 執行參數表完整性測試
 */
void PARAM_Test_RunIntegrityTest(void)
{
    printf("\r\n===== 參數表完整性測試 =====\r\n");
    
    // 校驗正確性驗證
    printf("初始校驗結果: %s\r\n", PARAM_VerifyChecksum() ? "通過" : "失敗");
    
    // 修改一個值並驗證校驗是否失敗
    uint8_t originalValue = PARAM_Read(PARAM_MODEL_NO);
    uint8_t newValue = originalValue + 1;
    
    printf("修改產品型號: %d -> %d\r\n", originalValue, newValue);
    PARAM_Write(PARAM_MODEL_NO, newValue);
    
    printf("未更新校驗和時驗證結果: %s (應為失敗)\r\n", PARAM_VerifyChecksum() ? "通過" : "失敗");
    
    // 更新校驗和並再次驗證
    PARAM_UpdateChecksum();
    printf("更新校驗和後驗證結果: %s (應為通過)\r\n", PARAM_VerifyChecksum() ? "通過" : "失敗");
    
    // 恢復原值
    PARAM_Write(PARAM_MODEL_NO, originalValue);
    PARAM_UpdateChecksum();
    
    printf("恢復原值後驗證結果: %s\r\n", PARAM_VerifyChecksum() ? "通過" : "失敗");
}

/**
 * @brief 執行參數表性能測試
 */
void PARAM_Test_RunPerformanceTest(void)
{
    printf("\r\n===== 參數表性能測試 =====\r\n");
    
    uint32_t startTime, endTime, elapsedTime;
    uint16_t testCount = 1000;
    uint8_t dummySum = 0; // 使用此變數避免優化
    
    // 測試單字節讀取性能
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < testCount; i++) {
        dummySum += PARAM_Read(PARAM_MODEL_NO);
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("單字節讀取 x %d: %ld ms (%.2f ?s/次)\r\n", 
           testCount, elapsedTime, (float)elapsedTime * 1000 / testCount);
    
    // 測試單字節寫入性能
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < testCount; i++) {
        PARAM_Write(PARAM_MODEL_NO, i & 0xFF);
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("單字節寫入 x %d: %ld ms (%.2f ?s/次)\r\n", 
           testCount, elapsedTime, (float)elapsedTime * 1000 / testCount);
    
    // 防止編譯器優化掉
    if (dummySum == 0xFF) {
        printf("Dummy operation to prevent optimization\r\n");
    }
    
    // 測試校驗和計算性能
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < 100; i++) {
        PARAM_UpdateChecksum();
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("校驗和計算 x 100: %ld ms (%.2f ms/次)\r\n", 
           elapsedTime, (float)elapsedTime / 100);
    
    // 恢復原始值
    PARAM_Reset();
}

/**
 * @brief 列印參數區塊資訊
 */
static void PrintParameterInfo(void)
{
    BasicSystemBlock basicParams;
    HardwareCalibBlock calibParams;
    
    /* 讀取基本系統參數 */
    if (PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("=== 基本系統參數 ===\r\n");
        printf("產品型號: %d\r\n", basicParams.modelNo);
        printf("韌體版本: %d.%d\r\n", basicParams.fwNo/10, basicParams.fwNo%10);
        printf("測試次數: %d\r\n", basicParams.testCount);
        printf("日期時間: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
               basicParams.year, basicParams.month, basicParams.date,
               basicParams.hour, basicParams.minute, basicParams.second);
        printf("溫度範圍: %d°C ~ %d°C\r\n", basicParams.tempLowLimit, basicParams.tempHighLimit);
        printf("測試項目: %d\r\n", basicParams.stripType);
        printf("濃度單位: %s\r\n", basicParams.measureUnit ? "mg/dL" : "mmol/L");
    } else {
        printf("讀取基本系統參數失敗\r\n");
    }
    
    /* 讀取硬體校正參數 */
    if (PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("\r\n=== 硬體校正參數 ===\r\n");
        printf("EV_T3觸發電壓: %d mV\r\n", calibParams.evT3Trigger);
        printf("溫度補償: %d (0.1°C)\r\n", calibParams.tempOffset);
        printf("電池補償: %d (10mV)\r\n", calibParams.batteryOffset);
        printf("OPA斜率: %.4f\r\n", calibParams.ops);
        printf("OPA截距: %.4f\r\n", calibParams.opi);
    } else {
        printf("讀取硬體校正參數失敗\r\n");
    }
    
    printf("\r\n");
}

/**
 * @brief 測試系統參數更新
 */
static void TestSystemParamUpdate(void)
{
    BasicSystemBlock basicParams;
    
    /* 讀取當前參數 */
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("讀取系統參數失敗\r\n");
        return;
    }
    
    /* 顯示原始參數 */
    printf("原始測試次數: %d\r\n", basicParams.testCount);
    
    /* 更新測試次數 */
    if (PARAM_IncreaseTestCount()) {
        printf("測試次數+1成功\r\n");
    } else {
        printf("測試次數+1失敗\r\n");
        return;
    }
    
    /* 重新讀取參數 */
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("讀取系統參數失敗\r\n");
        return;
    }
    
    /* 顯示更新後參數 */
    printf("更新後測試次數: %d\r\n", basicParams.testCount);
    printf("\r\n");
}

/**
 * @brief 測試日期時間更新
 */
static void TestDateTimeUpdate(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* 讀取當前時間 */
    if (!PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second)) {
        printf("讀取日期時間失敗\r\n");
        return;
    }
    
    /* 顯示原始時間 */
    printf("原始時間: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
           year, month, date, hour, minute, second);
    
    /* 更新時間 */
    if (PARAM_UpdateDateTime(24, 7, 25, 15, 30, 0)) {
        printf("更新日期時間成功\r\n");
    } else {
        printf("更新日期時間失敗\r\n");
        return;
    }
    
    /* 重新讀取時間 */
    if (!PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second)) {
        printf("讀取日期時間失敗\r\n");
        return;
    }
    
    /* 顯示更新後時間 */
    printf("更新後時間: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
           year, month, date, hour, minute, second);
    printf("\r\n");
}

/**
 * @brief 測試參數抓取
 */
static void TestParametersFetch(void)
{
    uint16_t ndl, udl, bloodIn;
    uint8_t lowLimit;
    uint16_t highLimit;
    uint16_t tpl1, trd1, evWidth1;
    uint16_t tpl2, trd2, evWidth2;
    
    /* 測試血糖參數 */
    printf("=== 血糖參數測試 ===\r\n");
    
    /* 讀取試片參數 */
    if (PARAM_GetStripParametersByStripType(STRIP_TYPE_GLV, &ndl, &udl, &bloodIn)) {
        printf("血糖試片參數: NDL=%d, UDL=%d, BloodIn=%d\r\n", ndl, udl, bloodIn);
    } else {
        printf("讀取血糖試片參數失敗\r\n");
    }
    
    /* 讀取測量範圍 */
    if (PARAM_GetMeasurementRangeByStripType(STRIP_TYPE_GLV, &lowLimit, &highLimit)) {
        printf("血糖測量範圍: %d ~ %d mg/dL\r\n", lowLimit, highLimit);
    } else {
        printf("讀取血糖測量範圍失敗\r\n");
    }
    
    /* 讀取時序參數(組1) */
    if (PARAM_GetTimingParametersByStripType(STRIP_TYPE_GLV, &tpl1, &trd1, &evWidth1, 1)) {
        printf("血糖時序參數(組1): TPL=%d, TRD=%d, EVWidth=%d\r\n", tpl1, trd1, evWidth1);
    } else {
        printf("讀取血糖時序參數(組1)失敗\r\n");
    }
    
    /* 讀取時序參數(組2) */
    if (PARAM_GetTimingParametersByStripType(STRIP_TYPE_GLV, &tpl2, &trd2, &evWidth2, 2)) {
        printf("血糖時序參數(組2): TPL=%d, TRD=%d, EVWidth=%d\r\n", tpl2, trd2, evWidth2);
    } else {
        printf("讀取血糖時序參數(組2)失敗\r\n");
    }
    
    /* 測試尿酸參數 */
    printf("\r\n=== 尿酸參數測試 ===\r\n");
    
    /* 讀取試片參數 */
    if (PARAM_GetStripParametersByStripType(STRIP_TYPE_U, &ndl, &udl, &bloodIn)) {
        printf("尿酸試片參數: NDL=%d, UDL=%d, BloodIn=%d\r\n", ndl, udl, bloodIn);
    } else {
        printf("讀取尿酸試片參數失敗\r\n");
    }
    
    /* 讀取測量範圍 */
    if (PARAM_GetMeasurementRangeByStripType(STRIP_TYPE_U, &lowLimit, &highLimit)) {
        printf("尿酸測量範圍: %d ~ %d mg/dL\r\n", lowLimit, highLimit);
    } else {
        printf("讀取尿酸測量範圍失敗\r\n");
    }
    
    printf("\r\n");
}

/**
 * @brief 測試硬體校正參數更新
 */
static void TestHardwareCalibUpdate(void)
{
    HardwareCalibBlock calibParams;
    
    /* 讀取當前參數 */
    if (!PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("讀取硬體校正參數失敗\r\n");
        return;
    }
    
    /* 顯示原始參數 */
    printf("原始溫度補償: %d (0.1°C)\r\n", calibParams.tempOffset);
    printf("原始電池補償: %d (10mV)\r\n", calibParams.batteryOffset);
    
    /* 更新校正參數 */
    calibParams.tempOffset = 5;     // 增加0.5°C
    calibParams.batteryOffset = -10; // 減少100mV
    
    if (PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("更新硬體校正參數成功\r\n");
    } else {
        printf("更新硬體校正參數失敗\r\n");
        return;
    }
    
    /* 重新讀取參數 */
    if (!PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("讀取硬體校正參數失敗\r\n");
        return;
    }
    
    /* 顯示更新後參數 */
    printf("更新後溫度補償: %d (0.1°C)\r\n", calibParams.tempOffset);
    printf("更新後電池補償: %d (10mV)\r\n", calibParams.batteryOffset);
    printf("\r\n");
}

/**
 * @brief 重置參數測試
 */
static void TestParameterReset(void)
{
    printf("執行重置參數測試...\r\n");
    
    /* 顯示重置前參數 */
    printf("=== 重置前參數 ===\r\n");
    PrintParameterInfo();
    
    /* 重置參數 */
    PARAM_ResetToDefault();
    
    /* 顯示重置後參數 */
    printf("=== 重置後參數 ===\r\n");
    PrintParameterInfo();
}

/**
 * @brief 執行參數儲存測試
 */
void RunParameterTest(void)
{
    printf("\r\n===== 開始Flash參數儲存方案測試 =====\r\n\r\n");
    
    /* 初始化參數儲存系統 */
    FLASH_Storage_Init();
    
    /* 列印參數資訊 */
    PrintParameterInfo();
    
    /* 測試系統參數更新 */
    printf("測試系統參數更新...\r\n");
    TestSystemParamUpdate();
    
    /* 測試日期時間更新 */
    printf("測試日期時間更新...\r\n");
    TestDateTimeUpdate();
    
    /* 測試參數抓取 */
    printf("測試參數抓取...\r\n");
    TestParametersFetch();
    
    /* 測試硬體校正參數更新 */
    printf("測試硬體校正參數更新...\r\n");
    TestHardwareCalibUpdate();
    
    /* 重置參數測試 */
    printf("測試參數重置...\r\n");
    TestParameterReset();
    
    printf("\r\n===== Flash參數儲存方案測試完成 =====\r\n");
} 