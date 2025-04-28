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
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_usart.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/* CH32V203 初始化函數 */
void CH32V203_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 系統時鐘初始化，使用HSI作為系統時鐘源，16MHz */
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    
    /* GPIO 時鐘使能 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* UART2 時鐘使能 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* 配置LED引腳 (PB6, PB7) 為輸出模式，初始狀態為低電平 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* LED初始狀態設為低電平(熄滅) */
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // 紅色LED
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // 綠色LED
    
    /* 配置WE_ENABLE (PB15) 和 T1_ENABLE (PA8) 為輸出模式，初始狀態為高電平 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // WE_ENABLE 高電平
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // T1_ENABLE 高電平
    
    /* 配置按鍵輸入 KEY_1 (PB11) 和 KEY_2 (PB10) 為上拉輸入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置ADC輸入引腳為浮空輸入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置UART2引腳 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // TX引腳
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // RX引腳
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2初始化 - 波特率115200、8位元資料、1位元停止位、無同位檢查 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    
    /* 初始化延遲函數 */
    Delay_Init();
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
    /* 初始化CH32V203 */
    CH32V203_Init();
    
    while(1)
    {
        // 主循環程式
    }
}
