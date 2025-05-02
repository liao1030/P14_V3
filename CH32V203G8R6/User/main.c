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
    
    // 打印參數表信息
    Parameter_PrintInfo();
    
    // 主循環
    while(1)
    {
        // 主要程式邏輯
    }
}
