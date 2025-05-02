/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *多功能生化測試儀：
 *使用CH32V203G8R6實現多功能生化測試儀的參數代碼表管理。
 *
 */

#include "debug.h"
#include "parameter_code.h"
#include "strip_detection.h"  // 引入試片類型判別頭文件

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("多功能生化測試儀初始化中...\r\n");
    
    // 初始化參數表
    Parameter_Init();
    printf("參數表初始化完成\r\n");
    
    // 初始化試片類型判別功能
    Strip_Detection_Init();
    
    // 打印參數表信息
    Parameter_PrintInfo();
    
    printf("系統初始化完成，等待試片插入...\r\n");
    
    // 主循環
    while(1)
    {
        // 主要程式邏輯
        // 根據試片狀態執行不同操作
        switch(Strip_GetStatus()) {
            case STRIP_STATUS_IDENTIFIED:
                // 試片類型已識別，可以開始測試流程
                printf("試片類型已識別，準備開始測試...\r\n");
                // 這裡可以加入測試相關的程式碼
                break;
                
            default:
                // 其他狀態不需要特別處理
                break;
        }
        
        // 簡單延時，避免CPU負擔過重
        Delay_Ms(10);
    }
}
