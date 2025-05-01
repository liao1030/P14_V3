/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_test.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : 多功能生化測試儀參數代碼表測試函數
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_table.h"
#include "debug.h"
#include <string.h>

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