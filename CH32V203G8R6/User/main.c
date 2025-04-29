/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

/*
 *@Note
 *P14_V2.1 主程式
 *CH32V203G8R6 MCU初始化與主要功能實現
 */

#include "debug.h"
#include "param_table.h"
#include "param_test.h"
#include "param_store.h"

/* 全局定義 */
#define LED_RED_PIN     GPIO_Pin_6
#define LED_GREEN_PIN   GPIO_Pin_7
#define KEY_1_PIN       GPIO_Pin_11
#define KEY_2_PIN       GPIO_Pin_10
#define T1_ENABLE_PIN   GPIO_Pin_8
#define WE_ENABLE_PIN   GPIO_Pin_15

/* 函數聲明 */
void GPIO_Config(void);
void UART2_Config(void);
void Key_Process(void);
void ParamTable_Test(void);
void USART_Receive_Byte(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    /* 中斷優先級分組設置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* 系統時鐘初始化 - 使用HSI時鐘源運行在16MHz */
    SystemCoreClockUpdate();
    
    /* 延遲初始化 */
    Delay_Init();
    
    /* GPIO 初始化 */
    GPIO_Config();
    
    /* UART2 初始化 - 與CH582F通訊 */
    UART2_Config();
    
    /* 調試用UART初始化 */
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    /* 初始化參數存儲模塊 */
    printf("初始化參數存儲模塊...\r\n");
    if (PARAM_Init() != 0) {
        printf("參數存儲模塊初始化失敗！\r\n");
    } else {
        printf("參數存儲模塊初始化成功！\r\n");
    }
    
    /* 執行參數模塊測試 */
    printf("按任意鍵開始參數存儲測試...\r\n");
    USART_Receive_Byte();
    PARAM_TestAll();
    
    printf("測試完成，系統進入循環\r\n");
    
    while(1)
    {
        /* 處理按鍵 */
        Key_Process();
        
        /* 延遲 */
        Delay_Ms(50);
    }
}

/*********************************************************************
 * @fn      GPIO_Config
 *
 * @brief   GPIO初始化配置
 *
 * @return  none
 */
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* 使能GPIOA、GPIOB時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* LED控制引腳配置 - PB6(紅色)、PB7(綠色) */
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 按鍵輸入配置 - PB10、PB11 (按鍵輸入，上拉輸入) */
    GPIO_InitStructure.GPIO_Pin = KEY_1_PIN | KEY_2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* T1電極量測致能控制 - PA8 (低電平致能) */
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W電極電壓PWM控制 - PB15 */
    GPIO_InitStructure.GPIO_Pin = WE_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 溫度電阻輸入 - PA1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 試片T1電極電壓輸入 - PA6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 試片T3電極電壓輸入 - PB0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 生化反應結果輸入 - PA4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W電極電壓輸入 - PA7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 電池電壓檢測 - PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 初始設定輸出狀態 */
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);    /* T1電極量測禁用(高電平) */
    GPIO_SetBits(GPIOB, WE_ENABLE_PIN);    /* W電極電壓PWM輸出致能(高電平) */
    GPIO_ResetBits(GPIOB, LED_RED_PIN);    /* 紅色LED關閉 */
    GPIO_ResetBits(GPIOB, LED_GREEN_PIN);  /* 綠色LED關閉 */
}

/*********************************************************************
 * @fn      UART2_Config
 *
 * @brief   UART2配置 - 與CH582F通訊
 *
 * @return  none
 */
void UART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    
    /* 使能GPIOA和UART2時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* UART2 TX(PA2)引腳配置 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2 RX(PA3)引腳配置 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2參數配置 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    /* 配置UART2 */
    USART_Init(USART2, &USART_InitStructure);
    
    /* 使能UART2 */
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      ParamTable_Test
 *
 * @brief   測試參數表功能，列印當前參數設定並修改某參數
 *
 * @return  none
 */
void ParamTable_Test(void);

/*********************************************************************
 * @fn      Key_Process
 *
 * @brief   處理按鍵輸入
 *
 * @return  none
 */
void Key_Process(void)
{
    static uint8_t key1_last = 1;
    static uint8_t key2_last = 1;
    uint8_t key1_curr, key2_curr;
    
    /* 讀取當前按鍵狀態 */
    key1_curr = GPIO_ReadInputDataBit(GPIOB, KEY_1_PIN);
    key2_curr = GPIO_ReadInputDataBit(GPIOB, KEY_2_PIN);
    
    /* 檢測KEY1按下 (下降沿) */
    if (key1_last == 1 && key1_curr == 0) {
        printf("KEY1 pressed - Executing parameter test\r\n");
        PARAM_TestAll(); /* 執行參數存儲測試 */
    }
    
    /* 檢測KEY2按下 (下降沿) */
    if (key2_last == 1 && key2_curr == 0) {
        printf("KEY2 pressed - Toggling LED\r\n");
        
        /* 切換LED狀態 */
        if (GPIO_ReadOutputDataBit(GPIOB, LED_GREEN_PIN)) {
            GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
        } else {
            GPIO_SetBits(GPIOB, LED_GREEN_PIN);
        }
    }
    
    /* 更新按鍵狀態 */
    key1_last = key1_curr;
    key2_last = key2_curr;
}

/*********************************************************************
 * @fn      USART_Receive_Byte
 *
 * @brief   Wait and receive a byte from USART.
 *
 * @return  none
 */
void USART_Receive_Byte(void)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    USART_ReceiveData(USART1);
}
