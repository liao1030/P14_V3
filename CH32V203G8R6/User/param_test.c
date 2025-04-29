/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_test.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter storage test module for CH32V203G8R6.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_test.h"
#include "param_store.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>  /* 用於 abs 函數 */

/* 本模塊函數原型 */
static void PARAM_TestBasicParams(void);
static void PARAM_TestCalibParams(void);
static void PARAM_TestStripParams(void);
static void PARAM_TestDateTime(void);
static void PARAM_TestBackupRecovery(void);
static void PARAM_TestGetFunctions(void);

/*********************************************************************
 * @fn      PARAM_TestAll
 *
 * @brief   執行所有參數儲存模塊測試
 *
 * @return  無
 */
void PARAM_TestAll(void)
{
    printf("開始參數儲存模塊測試...\r\n");
    
    /* 初始化參數儲存模塊 */
    if (PARAM_Init() != 0) {
        printf("參數儲存模塊初始化失敗！\r\n");
        return;
    }
    
    /* 執行各項測試 */
    PARAM_TestBasicParams();
    PARAM_TestCalibParams();
    PARAM_TestStripParams();
    PARAM_TestDateTime();
    PARAM_TestBackupRecovery();
    PARAM_TestGetFunctions();
    
    printf("參數儲存模塊測試完成！\r\n");
}

/*********************************************************************
 * @fn      PARAM_TestBasicParams
 *
 * @brief   測試基本系統參數的讀寫
 *
 * @return  無
 */
static void PARAM_TestBasicParams(void)
{
    printf("\r\n測試基本系統參數...\r\n");
    
    /* 讀取當前參數 */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("讀取基本系統參數失敗！\r\n");
        return;
    }
    
    /* 顯示當前參數 */
    printf("當前基本參數：\r\n");
    printf("電池閾值: 低=%d.%02dV, 耗盡=%d.%02dV\r\n", 
        sysBlock.lbt / 100, sysBlock.lbt % 100, 
        sysBlock.obt / 100, sysBlock.obt % 100);
    printf("型號: %d, 韌體版本: %d.%d\r\n", 
        sysBlock.modelNo, sysBlock.fwNo / 10, sysBlock.fwNo % 10);
    printf("測試次數: %d, 預設事件: %d\r\n", 
        sysBlock.testCount, sysBlock.defaultEvent);
    
    /* 修改參數 */
    uint16_t oldTestCount = sysBlock.testCount;
    uint8_t oldEvent = sysBlock.defaultEvent;
    
    sysBlock.defaultEvent = (sysBlock.defaultEvent + 1) % 3; /* 切換預設事件 */
    sysBlock.testCount += 10; /* 增加測試次數 */
    
    printf("修改參數：事件=%d, 測試次數=%d\r\n", 
        sysBlock.defaultEvent, sysBlock.testCount);
    
    /* 寫入參數 */
    if (PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("更新基本系統參數失敗！\r\n");
        return;
    }
    
    /* 再次讀取確認 */
    memset(&sysBlock, 0, sizeof(BasicSystemBlock));
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("讀取基本系統參數失敗！\r\n");
        return;
    }
    
    /* 檢查參數是否已更新 */
    if (sysBlock.defaultEvent == oldEvent || sysBlock.testCount == oldTestCount) {
        printf("參數更新驗證失敗！\r\n");
    } else {
        printf("參數更新驗證成功！\r\n");
    }
    
    /* 恢復原來的參數 */
    sysBlock.defaultEvent = oldEvent;
    sysBlock.testCount = oldTestCount;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_TestCalibParams
 *
 * @brief   測試硬體校準參數的讀寫
 *
 * @return  無
 */
static void PARAM_TestCalibParams(void)
{
    printf("\r\n測試硬體校準參數...\r\n");
    
    /* 讀取當前參數 */
    HardwareCalibBlock calibBlock;
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("讀取硬體校準參數失敗！\r\n");
        return;
    }
    
    /* 顯示當前參數 */
    printf("當前校準參數：\r\n");
    printf("EV_T3觸發: %dmV, 工作電壓: %d, T3電壓: %d\r\n", 
        calibBlock.evT3Trigger, calibBlock.evWorking, calibBlock.evT3);
    printf("溫度偏移: %d.%01d°C, 電池偏移: %dmV\r\n", 
        calibBlock.tempOffset / 10, abs(calibBlock.tempOffset) % 10, 
        calibBlock.batteryOffset * 10);
    
    /* 修改參數 */
    int8_t oldTempOffset = calibBlock.tempOffset;
    int8_t oldBattOffset = calibBlock.batteryOffset;
    
    calibBlock.tempOffset = 5;  /* 設置溫度偏移為0.5°C */
    calibBlock.batteryOffset = 3; /* 設置電池偏移為30mV */
    
    printf("修改參數：溫度偏移=%d.%01d°C, 電池偏移=%dmV\r\n", 
        calibBlock.tempOffset / 10, abs(calibBlock.tempOffset) % 10, 
        calibBlock.batteryOffset * 10);
    
    /* 寫入參數 */
    if (PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("更新硬體校準參數失敗！\r\n");
        return;
    }
    
    /* 再次讀取確認 */
    memset(&calibBlock, 0, sizeof(HardwareCalibBlock));
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("讀取硬體校準參數失敗！\r\n");
        return;
    }
    
    /* 檢查參數是否已更新 */
    if (calibBlock.tempOffset != 5 || calibBlock.batteryOffset != 3) {
        printf("參數更新驗證失敗！\r\n");
    } else {
        printf("參數更新驗證成功！\r\n");
    }
    
    /* 恢復原來的參數 */
    calibBlock.tempOffset = oldTempOffset;
    calibBlock.batteryOffset = oldBattOffset;
    PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock));
}

/*********************************************************************
 * @fn      PARAM_TestStripParams
 *
 * @brief   測試試片參數的讀寫
 *
 * @return  無
 */
static void PARAM_TestStripParams(void)
{
    printf("\r\n測試試片參數...\r\n");
    
    /* 讀取血糖試片參數 */
    TestParamBaseBlock bgBlock;
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("讀取血糖參數失敗！\r\n");
        return;
    }
    
    /* 顯示當前參數 */
    printf("當前血糖試片參數：\r\n");
    printf("檢測閾值: 新試片=%d, 已用=%d, 血液=%d\r\n", 
        bgBlock.ndl, bgBlock.udl, bgBlock.bloodIn);
    printf("測量範圍: %d-%dmg/dL\r\n", bgBlock.lowLimit, bgBlock.highLimit);
    printf("測試時序1: TPL=%dms, TRD=%dms, EV=%dms\r\n", 
        bgBlock.tpl1 * 100, bgBlock.trd1 * 100, bgBlock.evWidth1 * 100);
    
    /* 修改參數 */
    uint16_t oldNdl = bgBlock.ndl;
    uint16_t oldTpl1 = bgBlock.tpl1;
    
    bgBlock.ndl = 120;  /* 修改新試片閾值 */
    bgBlock.tpl1 = 15;  /* 修改測試時序 */
    
    printf("修改參數：新試片閾值=%d, TPL1=%dms\r\n", 
        bgBlock.ndl, bgBlock.tpl1 * 100);
    
    /* 寫入參數 */
    if (PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("更新血糖參數失敗！\r\n");
        return;
    }
    
    /* 再次讀取確認 */
    memset(&bgBlock, 0, sizeof(TestParamBaseBlock));
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("讀取血糖參數失敗！\r\n");
        return;
    }
    
    /* 檢查參數是否已更新 */
    if (bgBlock.ndl != 120 || bgBlock.tpl1 != 15) {
        printf("參數更新驗證失敗！\r\n");
    } else {
        printf("參數更新驗證成功！\r\n");
    }
    
    /* 恢復原來的參數 */
    bgBlock.ndl = oldNdl;
    bgBlock.tpl1 = oldTpl1;
    PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock));
}

/*********************************************************************
 * @fn      PARAM_TestDateTime
 *
 * @brief   測試日期時間參數的讀寫
 *
 * @return  無
 */
static void PARAM_TestDateTime(void)
{
    printf("\r\n測試日期時間參數...\r\n");
    
    /* 讀取當前日期時間 */
    uint8_t year, month, date, hour, minute, second;
    if (PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second) != 0) {
        printf("讀取日期時間失敗！\r\n");
        return;
    }
    
    /* 顯示當前日期時間 */
    printf("當前日期時間：20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
        year, month, date, hour, minute, second);
    
    /* 修改日期時間 */
    uint8_t newHour = (hour + 1) % 24;
    uint8_t newMinute = (minute + 30) % 60;
    
    printf("修改時間為：20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
        year, month, date, newHour, newMinute, second);
    
    /* 更新日期時間 */
    if (PARAM_UpdateDateTime(year, month, date, newHour, newMinute, second) != 0) {
        printf("更新日期時間失敗！\r\n");
        return;
    }
    
    /* 再次讀取確認 */
    if (PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second) != 0) {
        printf("讀取日期時間失敗！\r\n");
        return;
    }
    
    /* 檢查時間是否已更新 */
    if (hour != newHour || minute != newMinute) {
        printf("時間更新驗證失敗！\r\n");
    } else {
        printf("時間更新驗證成功！新時間：%02d:%02d:%02d\r\n", 
            hour, minute, second);
    }
}

/*********************************************************************
 * @fn      PARAM_TestBackupRecovery
 *
 * @brief   測試備份區功能
 *
 * @return  無
 */
static void PARAM_TestBackupRecovery(void)
{
    printf("\r\n測試備份區功能...\r\n");
    
    /* 顯示當前活動區域 */
    uint32_t activeAddr = PARAM_GetActiveBlockAddr();
    printf("當前活動區塊地址: 0x%08X\r\n", (unsigned int)activeAddr);
    
    /* 讀取當前參數 */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("讀取基本系統參數失敗！\r\n");
        return;
    }
    
    /* 修改參數以觸發備份寫入 */
    uint16_t oldTestCount = sysBlock.testCount;
    sysBlock.testCount += 1;
    
    /* 更新參數 */
    if (PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("更新基本系統參數失敗！\r\n");
        return;
    }
    
    /* 檢查活動區域是否已切換 */
    uint32_t newActiveAddr = PARAM_GetActiveBlockAddr();
    if (newActiveAddr == activeAddr) {
        printf("活動區塊未切換！\r\n");
        return;
    }
    
    printf("活動區塊已切換：0x%08X -> 0x%08X\r\n", 
        (unsigned int)activeAddr, (unsigned int)newActiveAddr);
    
    /* 驗證測試計數器增加 */
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("讀取基本系統參數失敗！\r\n");
        return;
    }
    
    if (sysBlock.testCount != oldTestCount + 1) {
        printf("備份區參數驗證失敗！\r\n");
    } else {
        printf("備份區參數驗證成功！\r\n");
    }
}

/*********************************************************************
 * @fn      PARAM_TestGetFunctions
 *
 * @brief   測試參數獲取函數
 *
 * @return  無
 */
static void PARAM_TestGetFunctions(void)
{
    printf("\r\n測試參數獲取函數...\r\n");
    
    /* 讀取測試參數 */
    uint16_t ndl, udl, bloodIn;
    if (PARAM_GetStripParametersByStripType(0, &ndl, &udl, &bloodIn) != 0) {
        printf("獲取血糖試片參數失敗！\r\n");
        return;
    }
    
    printf("血糖試片電壓參數：新試片=%d, 已用=%d, 血液=%d\r\n", 
        ndl, udl, bloodIn);
    
    /* 讀取測量範圍 */
    uint8_t lowLimit;
    uint16_t highLimit;
    if (PARAM_GetMeasurementRangeByStripType(0, &lowLimit, &highLimit) != 0) {
        printf("獲取血糖測量範圍失敗！\r\n");
        return;
    }
    
    printf("血糖測量範圍：%d-%dmg/dL\r\n", 
        lowLimit, highLimit);
    
    /* 讀取時序參數 */
    uint16_t tpl, trd, evWidth;
    if (PARAM_GetTimingParametersByStripType(0, &tpl, &trd, &evWidth, 1) != 0) {
        printf("獲取血糖時序參數失敗！\r\n");
        return;
    }
    
    printf("血糖時序參數(組1)：TPL=%dms, TRD=%dms, EV=%dms\r\n", 
        tpl * 100, trd * 100, evWidth * 100);
    
    /* 測試增加測試計數 */
    BasicSystemBlock sysBlock;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
    uint16_t oldCount = sysBlock.testCount;
    
    if (PARAM_IncreaseTestCount() != 0) {
        printf("增加測試計數失敗！\r\n");
        return;
    }
    
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
    printf("測試計數：%d -> %d\r\n", oldCount, sysBlock.testCount);
    
    /* 回復原計數 */
    sysBlock.testCount = oldCount;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
} 