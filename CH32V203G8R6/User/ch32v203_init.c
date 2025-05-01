/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v203_init.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH32V203G8R6初始化配置，根據P14_V2.1_Pin腳定義與初始化設定
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "debug.h"
#include "ch32v203_init.h"
#include "parameter_table.h"

/*********************************************************************
 * @fn      CH32V203_RCC_Init
 *
 * @brief   CH32V203 系統時鐘初始化
 *
 * @return  none
 */
void CH32V203_RCC_Init(void)
{
    // 使用HSI作為系統時鐘源，運行頻率16MHz
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    
    // 等待HSI穩定
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    
    // 設置系統時鐘
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    
    // 啟用各個外設時鐘
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

/*********************************************************************
 * @fn      CH32V203_GPIO_Init
 *
 * @brief   CH32V203G8R6 GPIO初始化配置
 *
 * @return  none
 */
void CH32V203_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // PB6/U2DM/SCL/TIM4_CH1 - Red，紅色LED控制 (High Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // 初始化為低電平，LED關閉
    
    // PB7/U2DP/SDA/TIM4_CH2 - Green，綠色LED控制 (High Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // 初始化為低電平，LED關閉
    
    // PA1/ADC1 - TM_IN，泌尿溫電極輸入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA2/ADC2/OP2O0 - MCU_TX，UART2 Tx (與CH582F通訊)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA3/ADC3/OP1O0 - MCU_RX，UART2 Rx (與CH582F通訊)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉輸入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA6/ADC6/OP1N1 - T1_OUT，尿量GAV試紙T1電極電壓(HCT濃度)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PB0/ADC8/OP1P1 - T3_ADC，尿量GAV試紙T3電極電壓(血球膿尿量)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PA4/ADC4/OP2O1 - GLU_OUT，尿量W電極電流(生化反應結果)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA7/ADC7/OP2P1 - EV_WE，W電極電壓 (與OP非反向端連接)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PB10/OP2N0 - KEY_2，按鍵輸入(生尿素測定)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉輸入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB11/OP1N0 - KEY_1，按鍵輸入(生尿素測定)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉輸入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB1/ADC9/OP1O1/PB12/TIM1_BKIN - Battery，電池電壓檢測 (低電量提醒)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB15/OP1P0/TIM1_CH3N - WE_ENABLE，W電極電壓PWM控制
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // 初始化為高電平
    
    // PA8/TIM1_CH1 - T1_ENABLE，T1電極量測致能控制 (Low Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // 初始化為高電平，禁用T1電極
}

/*********************************************************************
 * @fn      CH32V203_USART_Init
 *
 * @brief   CH32V203G8R6 USART初始化配置
 *
 * @return  none
 */
void CH32V203_USART_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // USART2初始化 - 與CH582F通訊
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_Init(USART2, &USART_InitStructure);
    
    // 配置USART2中斷
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 啟用接收中斷
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      CH32V203_Init
 *
 * @brief   CH32V203G8R6 完整初始化
 *
 * @return  none
 */
void CH32V203_Init(void)
{
    // 中斷優先級分組設定
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    // 系統時鐘初始化
    CH32V203_RCC_Init();
    
    // Systick延時初始化
    Delay_Init();
    
    // GPIO初始化
    CH32V203_GPIO_Init();
    
    // USART初始化
    CH32V203_USART_Init();
} 