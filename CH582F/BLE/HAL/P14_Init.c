/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Init.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/01
 * Description        : P14項目CH582F初始化程式
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_Init.h"

/*********************************************************************
 * @fn      P14_CH582F_GPIO_Init
 *
 * @brief   CH582F的GPIO初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_GPIO_Init(void)
{
    /* BLE_RX/BLE_TX (PA8/PA9 - UART1與CH32V203通訊) */
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RX, 上拉輸入
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TX, 推挽輸出
    
    /* Strip_Detect_3 (PB11 - 試紙插入檢測, 下降沿中斷) */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);     // 上拉輸入
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge); // 設置為下降沿觸發中斷
    
    /* T3_IN_SEL (PB10 - T3電極量測致能控制) */
    GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA); // 推挽輸出
    GPIOB_ResetBits(GPIO_Pin_10);                   // 初始設為低電平
    
    /* D-_UART_TX_MCU/D+_UART_RX_MCU (PB7/PB4 - UART0與外部設備1通訊) */
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RX, 上拉輸入
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TX, 推挽輸出
    
    /* D-_UART_RX_MCU/D+_UART_TX_MCU (PA4/PA5 - UART3與外部設備2通訊) */
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RX, 上拉輸入
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TX, 推挽輸出
    
    /* Strip_Detect_5 (PA15 - 試紙插入檢測, 下降沿中斷) */
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);     // 上拉輸入
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge); // 設置為下降沿觸發中斷
    
    /* V2P5_ENABLE (PA14 - 供電至CH32V203致能控制) */
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA); // 推挽輸出
    GPIOA_SetBits(GPIO_Pin_14);                     // 初始設為高電平
    
    /* V_back_C (PA13 - 外部UART設備Rx Level Shift電源選擇控制) */
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA); // 推挽輸出
    GPIOA_ResetBits(GPIO_Pin_13);                   // 初始設為低電平
    
    /* VBUS_Get (PA12 - 量測外部設備是否供電) */
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating); // 浮空輸入
}

/*********************************************************************
 * @fn      P14_CH582F_UART_Init
 *
 * @brief   CH582F的UART初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_UART_Init(void)
{
    /* UART1初始化 - 與CH32V203通訊 */
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO開啟，觸發點4位元組
    R8_UART1_LCR = RB_LCR_WORD_SZ; // 8位數據位，1位停止位，無校驗位
    R8_UART1_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // 啟用發送和接收中斷
    R8_UART1_DIV = 1;
    
    /* UART0初始化 - 與外部設備1通訊 */
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO開啟，觸發點4位元組
    R8_UART0_LCR = RB_LCR_WORD_SZ; // 8位數據位，1位停止位，無校驗位
    R8_UART0_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // 啟用發送和接收中斷
    R8_UART0_DIV = 1;
    
    /* UART3初始化 - 與外部設備2通訊 */
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO開啟，觸發點4位元組
    R8_UART3_LCR = RB_LCR_WORD_SZ; // 8位數據位，1位停止位，無校驗位
    R8_UART3_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // 啟用發送和接收中斷
    R8_UART3_DIV = 1;
}

/*********************************************************************
 * @fn      P14_CH582F_System_Init
 *
 * @brief   CH582F系統初始化
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_System_Init(void)
{
    /* 設置系統時鐘為60MHz */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* GPIO初始化 */
    P14_CH582F_GPIO_Init();
    
    /* UART初始化 */
    P14_CH582F_UART_Init();
    
    /* 啟用全局中斷 */
    PFIC_EnableAllIRQ();
    
    /* 校準內部32K時鐘 */
    Calibration_LSI(Level_64);
} 