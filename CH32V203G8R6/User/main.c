/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2024/09/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2024 HMD Biomedical Inc.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *多功能生化測試儀Flash參數儲存方案程式:
 *此示例程式演示如何初始化、讀取和保存多功能生化測試儀的參數和測試記錄。
 *USART1_Tx(PA9) - 用於輸出調試信息
 *
 */

#include "debug.h"
#include "param_storage.h"
#include <stdbool.h>

/* 全局變數定義 */
static BasicSystemBlock g_basicParams;
static HardwareCalibBlock g_calibParams;
static BloodGlucoseBlock g_bgParams;
static TestRecord g_testRecord;

/*********************************************************************
 * @fn      printBasicSystemInfo
 *
 * @brief   打印基本系統參數信息
 *
 * @param   basicParams - 基本系統參數指針
 *
 * @return  none
 */
void printBasicSystemInfo(const BasicSystemBlock *basicParams)
{
    printf("\r\n====== 多功能生化測試儀基本參數信息 ======\r\n");
    printf("韌體版本: V%d.%d\r\n", basicParams->fwNo / 10, basicParams->fwNo % 10);
    printf("代碼表版本: %d\r\n", basicParams->codeTableVer);
    printf("測試次數: %d\r\n", basicParams->testCount);
    printf("操作模式: %s\r\n", (basicParams->factory == 0) ? "使用者模式" : "工廠模式");
    
    printf("\r\n-- 日期時間設定 --\r\n");
    printf("日期: 20%02d/%02d/%02d\r\n", basicParams->year, basicParams->month, basicParams->date);
    printf("時間: %02d:%02d:%02d\r\n", basicParams->hour, basicParams->minute, basicParams->second);
    
    printf("\r\n-- 測試環境參數 --\r\n");
    printf("操作溫度範圍: %d°C ~ %d°C\r\n", basicParams->tempLowLimit, basicParams->tempHighLimit);
    
    char *unit_str = "未知";
    switch(basicParams->measureUnit) {
        case UNIT_MMOL_L: unit_str = "mmol/L"; break;
        case UNIT_MG_DL: unit_str = "mg/dL"; break;
        case UNIT_GM_DL: unit_str = "gm/dl"; break;
    }
    printf("測量單位: %s\r\n", unit_str);
    
    char *strip_type_str = "未知";
    switch(basicParams->stripType) {
        case STRIP_TYPE_GLV: strip_type_str = "血糖(GLV)"; break;
        case STRIP_TYPE_UA: strip_type_str = "尿酸(U)"; break;
        case STRIP_TYPE_CHOL: strip_type_str = "總膽固醇(C)"; break;
        case STRIP_TYPE_TG: strip_type_str = "三酸甘油脂(TG)"; break;
        case STRIP_TYPE_GAV: strip_type_str = "血糖(GAV)"; break;
    }
    printf("預設測試項目: %s\r\n", strip_type_str);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      printHardwareCalibInfo
 *
 * @brief   打印硬體校準參數信息
 *
 * @param   calibParams - 硬體校準參數指針
 *
 * @return  none
 */
void printHardwareCalibInfo(const HardwareCalibBlock *calibParams)
{
    printf("\r\n====== 硬體校準參數信息 ======\r\n");
    printf("EV_T3觸發電壓: %d mV\r\n", calibParams->evT3Trigger);
    printf("測量工作電極電壓: %d\r\n", calibParams->evWorking);
    printf("血液檢測電極電壓: %d\r\n", calibParams->evT3);
    printf("OPA校準斜率: %.4f\r\n", calibParams->ops);
    printf("OPA校準截距: %.4f\r\n", calibParams->opi);
    printf("溫度校準偏移: %.1f°C\r\n", (float)calibParams->tempOffset / 10.0f);
    printf("電池校準偏移: %d mV\r\n", calibParams->batteryOffset * 10);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      printBloodGlucoseInfo
 *
 * @brief   打印血糖參數信息
 *
 * @param   bgParams - 血糖參數指針
 *
 * @return  none
 */
void printBloodGlucoseInfo(const BloodGlucoseBlock *bgParams)
{
    printf("\r\n====== 血糖參數信息 ======\r\n");
    printf("批號: %s\r\n", bgParams->bgStripLot);
    printf("新試片檢測水平: %d\r\n", bgParams->bgNdl);
    printf("已使用試片檢測水平: %d\r\n", bgParams->bgUdl);
    printf("血液檢測水平: %d\r\n", bgParams->bgBloodIn);
    printf("測量範圍: %d - %d mg/dL\r\n", bgParams->bgL * 10, bgParams->bgH * 10);
    
    printf("\r\n-- 測試時序參數 --\r\n");
    printf("第一組: TPL=%d, TRD=%d, EVWidth=%d\r\n", 
           bgParams->bgTPL1, bgParams->bgTRD1, bgParams->bgEVWidth1);
    printf("第二組: TPL=%d, TRD=%d, EVWidth=%d\r\n", 
           bgParams->bgTPL2, bgParams->bgTRD2, bgParams->bgEVWidth2);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      simulateTest
 *
 * @brief   模擬一次測試並保存記錄
 *
 * @param   none
 *
 * @return  none
 */
void simulateTest(void)
{
    static uint16_t testValue = 120; // 初始測試值(mg/dL)
    
    // 獲取當前時間
    uint8_t year, month, date, hour, minute, second;
    PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second);
    
    // 獲取當前測試項目
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // 準備測試記錄
    g_testRecord.resultStatus = 0; // 成功
    g_testRecord.resultValue = testValue;
    g_testRecord.testType = basicParams.stripType;
    g_testRecord.eventType = basicParams.defaultEvent;
    g_testRecord.stripCode = 1; // 假設的試片校正碼
    g_testRecord.year = 2000 + year;
    g_testRecord.month = month;
    g_testRecord.date = date;
    g_testRecord.hour = hour;
    g_testRecord.minute = minute;
    g_testRecord.second = second;
    g_testRecord.batteryVoltage = 320; // 模擬值: 3.2V
    g_testRecord.temperature = 250; // 模擬值: 25.0°C
    
    // 保存測試記錄
    if (PARAM_SaveTestRecord(&g_testRecord)) {
        printf("\r\n已保存測試記錄: 類型=%d, 值=%d mg/dL, 時間=20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
               g_testRecord.testType, g_testRecord.resultValue,
               year, month, date, hour, minute, second);
    } else {
        printf("\r\n保存測試記錄失敗!\r\n");
    }
    
    // 增加測試次數
    PARAM_IncreaseTestCount();
    
    // 改變下次測試值
    testValue += 5;
    if (testValue > 350) testValue = 80;
}

/*********************************************************************
 * @fn      printTestRecords
 *
 * @brief   打印測試記錄
 *
 * @param   none
 *
 * @return  none
 */
void printTestRecords(void)
{
    uint16_t count = PARAM_GetTestRecordCount();
    
    printf("\r\n====== 測試記錄 (共%d筆) ======\r\n", count);
    
    if (count > 0) {
        // 最多顯示近10筆記錄
        uint16_t maxShow = (count > 10) ? 10 : count;
        uint16_t startIdx = (count > 10) ? (count - 10) : 0;
        TestRecord records[10];
        uint16_t actualCount = 0;
        
        if (PARAM_ReadTestRecords(records, startIdx, maxShow, &actualCount)) {
            for (uint16_t i = 0; i < actualCount; i++) {
                TestRecord *rec = &records[i];
                
                char *typeStr = "未知";
                switch (rec->testType) {
                    case STRIP_TYPE_GLV: typeStr = "血糖(GLV)"; break;
                    case STRIP_TYPE_UA: typeStr = "尿酸(U)"; break;
                    case STRIP_TYPE_CHOL: typeStr = "總膽固醇(C)"; break;
                    case STRIP_TYPE_TG: typeStr = "三酸甘油脂(TG)"; break;
                    case STRIP_TYPE_GAV: typeStr = "血糖(GAV)"; break;
                }
                
                char *eventStr = "未知";
                switch (rec->eventType) {
                    case EVENT_QC: eventStr = "QC"; break;
                    case EVENT_AC: eventStr = "AC"; break;
                    case EVENT_PC: eventStr = "PC"; break;
                }
                
                printf("%d. [%s] %d mg/dL (%s) %04d/%02d/%02d %02d:%02d:%02d\r\n",
                       startIdx + i + 1, typeStr, rec->resultValue, eventStr,
                       rec->year, rec->month, rec->date, rec->hour, rec->minute, rec->second);
            }
        } else {
            printf("讀取測試記錄失敗!\r\n");
        }
    } else {
        printf("沒有測試記錄\r\n");
    }
    
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主程序
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("系統時鐘頻率: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("晶片ID: %08x\r\n", DBGMCU_GetCHIPID());
    printf("多功能生化測試儀Flash參數儲存方案程式\r\n");
    
    /* 初始化參數儲存系統 */
    PARAM_Init();
    printf("參數儲存系統初始化完成\r\n");
    
    /* 讀取各參數區塊 */
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &g_basicParams, sizeof(BasicSystemBlock))) {
        printBasicSystemInfo(&g_basicParams);
    } else {
        printf("讀取基本系統參數失敗!\r\n");
    }
    
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &g_calibParams, sizeof(HardwareCalibBlock))) {
        printHardwareCalibInfo(&g_calibParams);
    } else {
        printf("讀取硬體校準參數失敗!\r\n");
    }
    
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &g_bgParams, sizeof(BloodGlucoseBlock))) {
        printBloodGlucoseInfo(&g_bgParams);
    } else {
        printf("讀取血糖參數失敗!\r\n");
    }
    
    /* 模擬測試操作 */
    for (int i = 0; i < 3; i++) {
        simulateTest();
        Delay_Ms(1000);
    }
    
    /* 打印測試記錄 */
    printTestRecords();
    
    /* 主循環 */
    while(1)
    {
        Delay_Ms(1000);
    }
}
