/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/07/23
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "ch32v203_init.h"
#include "parameter_table.h"
#include "parameter_test.h"

/* Global typedef */

/* Global define */
#define RUN_EXTENDED_TESTS      0   // 設為0關閉更多測試

/* Global Variable */

/**
 * @brief 執行參數表測試
 */
void RunParameterTests(void)
{
    printf("\r\n===== 多功能生化測試儀參數代碼表測試 =====\r\n");
    
    // 讀取系統基本參數
    printf("低電池閾值: %d (%.2fV)\r\n", PARAM_Read(PARAM_LBT), PARAM_Read(PARAM_LBT) * 0.01);
    printf("電池耗盡閾值: %d (%.2fV)\r\n", PARAM_Read(PARAM_OBT), PARAM_Read(PARAM_OBT) * 0.01);
    printf("儀器操作模式: %d\r\n", PARAM_Read(PARAM_FACTORY));
    printf("產品型號: %d\r\n", PARAM_Read(PARAM_MODEL_NO));
    printf("韌體版本號: %d\r\n", PARAM_Read(PARAM_FW_NO));
    
    uint16_t testCount = PARAM_Read16(PARAM_NOT);
    printf("測試次數: %d\r\n", testCount);
    
    // 增加測試次數並保存
    testCount++;
    PARAM_Write16(PARAM_NOT, testCount);
    printf("更新測試次數為: %d\r\n", testCount);
    
    // 讀取時間設定
    printf("\r\n時間設定: %02d/%02d/%02d %02d:%02d:%02d\r\n",
           PARAM_Read(PARAM_YEAR) + 2000,
           PARAM_Read(PARAM_MONTH),
           PARAM_Read(PARAM_DATE),
           PARAM_Read(PARAM_HOUR),
           PARAM_Read(PARAM_MINUTE),
           PARAM_Read(PARAM_SECOND));
    
    // 讀取測試環境參數
    printf("\r\n操作溫度範圍: %.1f~%.1f°C\r\n", 
           PARAM_Read(PARAM_TLL) * 1.0,
           PARAM_Read(PARAM_TLH) * 1.0);
    
    // 濃度單位設定顯示
    uint8_t mgdl = PARAM_Read(PARAM_MGDL);
    printf("濃度單位設定: ");
    switch(mgdl) {
        case 0: printf("mmol/L\r\n"); break;
        case 1: printf("mg/dL\r\n"); break;
        case 2: printf("gm/dl\r\n"); break;
        default: printf("未知\r\n"); break;
    }
    
    // 讀取測試項目
    uint8_t stripType = PARAM_Read(PARAM_STRIP_TYPE);
    printf("測試項目: ");
    switch(stripType) {
        case STRIP_TYPE_GLV: printf("血糖(GLV)\r\n"); break;
        case STRIP_TYPE_U: printf("尿酸(U)\r\n"); break;
        case STRIP_TYPE_C: printf("總膽固醇(C)\r\n"); break;
        case STRIP_TYPE_TG: printf("三酸甘油脂(TG)\r\n"); break;
        case STRIP_TYPE_GAV: printf("血糖(GAV)\r\n"); break;
        default: printf("未知\r\n"); break;
    }
    
    // 驗證校驗和
    printf("\r\n校驗結果: %s\r\n", PARAM_VerifyChecksum() ? "正確" : "錯誤");
    
    // 更新校驗和並保存參數
    PARAM_UpdateChecksum();
    PARAM_Save();
    
#if RUN_EXTENDED_TESTS
    // 額外的測試
    Delay_Ms(1000); // 延遲1秒，以便觀察測試輸出
    
    // 顯示主要參數
    PARAM_Test_PrintMainParameters();
    
    // 修改試片批號
    Delay_Ms(1000);
    PARAM_Test_SetStripLot(STRIP_TYPE_GLV, "BG202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_U, "UA202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_C, "TC202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_TG, "TG202407001");
    
    // 查看修改結果
    Delay_Ms(1000);
    PARAM_Test_PrintMainParameters();
    
    // 修改濃度單位
    Delay_Ms(1000);
    printf("\r\n修改濃度單位測試:\r\n");
    PARAM_Test_SetConcentrationUnit(1); // mg/dL
    
    // 完整性測試
    Delay_Ms(1000);
    PARAM_Test_RunIntegrityTest();
    
    // 性能測試
    Delay_Ms(1000);
    PARAM_Test_RunPerformanceTest();
#endif
    
    printf("\r\n參數代碼表測試完成\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    // 進行完整初始化
    CH32V203_Init();
    
    printf("CH32V203G8R6 初始化完成\r\n");
    printf("系統時鐘: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
    
    // 初始化參數代碼表
    PARAM_Init();
    
    // 執行參數表測試
    RunParameterTests();

    while(1)
    {
        // 主循環
        // 可在此處添加應用程式主邏輯
        
        // 閃爍LED指示系統正常運行
        GPIO_SetBits(GPIOB, GPIO_Pin_7); // 綠色LED亮
        Delay_Ms(500);
        GPIO_ResetBits(GPIOB, GPIO_Pin_7); // 綠色LED滅
        Delay_Ms(500);
    }
}
