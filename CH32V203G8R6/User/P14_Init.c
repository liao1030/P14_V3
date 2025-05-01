/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Init.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/01
 * Description        : P14項目CH32V203G8R6初始化程式
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Init.h"

/*********************************************************************
 * @fn      P14_CH32V203_GPIO_Init
 *
 * @brief   CH32V203的GPIO初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 啟用GPIO時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* Red LED (PB6, High Enable) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // 初始設為低電平
    
    /* Green LED (PB7, High Enable) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // 初始設為低電平
    
    /* TM_IN (PA1, 溫度電阻輸入) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* MCU_TX/MCU_RX (PA2/PA3, 與CH582F通訊) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* T1_OUT (PA6, 電極T1電阻測量) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* T3_ADC (PB0, 電極T3電阻測量) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* GLU_OUT (PA4, 生化反應結果) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* EV_WE (PA7, W電極電壓) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* KEY_2 (PB10, 按鈕輸入) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* KEY_1 (PB11, 按鈕輸入) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Battery (PB1, 電池電壓檢測) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* WE_ENABLE (PB15, W電極電壓PWM控制) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // 初始設為高電平
    
    /* T1_ENABLE (PA8, T1電極量測致能控制) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // 初始設為高電平 (Low Enable)
}

/*********************************************************************
 * @fn      P14_CH32V203_UART_Init
 *
 * @brief   CH32V203的UART初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_UART_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    /* 啟用UART時鐘 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* USART2 配置 (與CH582F通訊) */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    
    /* 啟用USART2接收中斷 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* 啟用USART2 */
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      P14_CH32V203_ParameterTable_Init
 *
 * @brief   參數代碼表初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_ParameterTable_Init(void)
{
    /* 初始化參數表 */
    P14_ParamTable_Init();
    
    /* 顯示當前測試模式 */
    if (P14_ParamTable_Read(PARAM_FACTORY) == MODE_FACTORY) {
        printf("當前模式: 工廠模式\r\n");
    } else {
        printf("當前模式: 使用者模式\r\n");
    }
    
    /* 顯示當前測試項目 */
    StripType_TypeDef stripType = (StripType_TypeDef)P14_ParamTable_Read(PARAM_STRIP_TYPE);
    printf("當前測試項目: %s\r\n", P14_ParamTable_GetStripTypeName(stripType));
    
    /* 顯示測量單位 */
    Unit_TypeDef unit = (Unit_TypeDef)P14_ParamTable_Read(PARAM_MGDL);
    printf("當前測量單位: %s\r\n", P14_ParamTable_GetUnitName(unit));
    
    /* 顯示測量次數 */
    printf("累計測量次數: %d\r\n", P14_ParamTable_Read16(PARAM_NOT));
}

/*********************************************************************
 * @fn      P14_CH32V203_Flash_Storage_Init
 *
 * @brief   Flash參數儲存系統初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_Flash_Storage_Init(void)
{
    /* 初始化Flash參數儲存系統 */
    if (PARAM_Init()) {
        printf("Flash參數儲存系統初始化成功\r\n");
    } else {
        printf("Flash參數儲存系統初始化失敗，已載入默認值\r\n");
    }
    
    /* 獲取基本系統參數 */
    BasicSystemBlock basicParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        /* 顯示當前測試模式 */
        if (basicParams.factory == MODE_FACTORY) {
            printf("當前模式: 工廠模式\r\n");
        } else {
            printf("當前模式: 使用者模式\r\n");
        }
        
        /* 顯示當前測試項目 */
        StripType_TypeDef stripType = (StripType_TypeDef)basicParams.stripType;
        printf("當前測試項目: %s\r\n", P14_ParamTable_GetStripTypeName(stripType));
        
        /* 顯示測量單位 */
        Unit_TypeDef unit = (Unit_TypeDef)basicParams.measureUnit;
        printf("當前測量單位: %s\r\n", P14_ParamTable_GetUnitName(unit));
        
        /* 顯示測量次數 */
        printf("累計測量次數: %d\r\n", basicParams.testCount);
    }
    
    /* 獲取測試記錄數量 */
    uint16_t recordCount = PARAM_GetTestRecordCount();
    printf("測試記錄數量: %d\r\n", recordCount);
}

/*********************************************************************
 * @fn      P14_CH32V203_System_Init
 *
 * @brief   CH32V203系統初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_System_Init(void)
{
    /* 設置NVIC分組 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* 更新系統時鐘 */
    SystemCoreClockUpdate();
    
    /* 初始化延遲函數 */
    Delay_Init();
    
    /* GPIO初始化 */
    P14_CH32V203_GPIO_Init();
    
    /* UART初始化 */
    P14_CH32V203_UART_Init();
    
    /* Flash參數儲存系統初始化 */
    P14_CH32V203_Flash_Storage_Init();
    
    /* 顯示初始化完成訊息 */
    printf("P14 CH32V203G8R6 系統初始化完成\r\n");
} 