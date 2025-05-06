/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : 多功能生化測試儀主程式
 *********************************************************************************
 * Copyright (c) 2025 HMD Biomedical Inc.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "param_code_table.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      PrintParamInfo
 *
 * @brief   印出參數表信息
 *
 * @return  none
 */
void PrintParamInfo(void)
{
    printf("\r\n===== 多功能生化測試儀參數表信息 =====\r\n");
    printf("參數表版本: %d.%d\r\n", ParamCodeTable.System.Code_Table_V >> 8, 
                                    ParamCodeTable.System.Code_Table_V & 0xFF);
    printf("韌體版本: V%d.%d\r\n", ParamCodeTable.System.FW_NO / 10, 
                                    ParamCodeTable.System.FW_NO % 10);
    printf("產品型號: %d\r\n", ParamCodeTable.System.MODEL_NO);
    printf("測試次數: %d\r\n", ParamCodeTable.System.NOT);
    
    printf("\r\n系統時間: 20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
            ParamCodeTable.Time.YEAR, 
            ParamCodeTable.Time.MONTH, 
            ParamCodeTable.Time.DATE,
            ParamCodeTable.Time.HOUR,
            ParamCodeTable.Time.MINUTE,
            ParamCodeTable.Time.SECOND);
    
    /* 顯示當前測試類型 */
    printf("\r\n當前測試類型: %s\r\n", GetStripTypeName(ParamCodeTable.Display.Strip_Type));
    
    /* 根據測試類型顯示相關參數 */
    switch(ParamCodeTable.Display.Strip_Type)
    {
        case 0: /* GLV */
        case 4: /* GAV */
            printf("血糖測試範圍: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.BG.L / 10, ParamCodeTable.BG.L % 10,
                    ParamCodeTable.BG.H / 10, ParamCodeTable.BG.H % 10);
            break;
            
        case 1: /* U */
            printf("尿酸測試範圍: %d - %d umol/L\r\n", 
                    ParamCodeTable.U.L * 10, ParamCodeTable.U.H * 10);
            break;
            
        case 2: /* C */
            printf("總膽固醇測試範圍: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.C.L / 10, ParamCodeTable.C.L % 10,
                    ParamCodeTable.C.H / 10, ParamCodeTable.C.H % 10);
            break;
            
        case 3: /* TG */
            printf("三酸甘油脂測試範圍: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.TG.L / 10, ParamCodeTable.TG.L % 10,
                    ParamCodeTable.TG.H / 10, ParamCodeTable.TG.H % 10);
            break;
    }
    
    printf("\r\n操作溫度範圍: %d - %d°C\r\n", 
            ParamCodeTable.Display.TLL, ParamCodeTable.Display.TLH);
    printf("========================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主程式
 *
 * @return  none
 */
int main(void)
{
    uint8_t currentMode;
    
    /* 初始化系統設置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("\r\n========================================\r\n");
    printf("多功能生化測試儀 - 系統啟動\r\n");
    printf("系統時鐘: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("晶片ID: 0x%08x\r\n", DBGMCU_GetCHIPID());
    
    /* 初始化參數代碼表 */
    printf("初始化參數代碼表...\r\n");
    ParamCodeTable_Init();
    
    /* 顯示參數信息 */
    PrintParamInfo();
    
    /* 檢查操作模式 */
    currentMode = ParamCodeTable.System.FACTORY;
    if(currentMode == 210)
    {
        printf("工廠模式啟動\r\n");
        /* 執行工廠模式初始化 */
    }
    else
    {
        printf("使用者模式啟動\r\n");
        /* 執行使用者模式初始化 */
    }
    
    printf("系統初始化完成\r\n");
    printf("========================================\r\n");
    
    while(1)
    {
        /* 主循環處理 */
        Delay_Ms(1000);
    }
}
