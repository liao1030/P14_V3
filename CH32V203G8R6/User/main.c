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
    // 進行完整初始化
    CH32V203_Init();
    
    printf("CH32V203G8R6 初始化完成\r\n");
    printf("系統時鐘: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

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
