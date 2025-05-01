/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch582f_init.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH582F初始化配置，根據P14_V2.1_Pin腳定義與初始化設定
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"
#include "ch582f_init.h"

/*********************************************************************
 * @fn      CH582F_GPIO_Init
 *
 * @brief   CH582F GPIO初始化配置
 *
 * @return  none
 */
void CH582F_GPIO_Init(void)
{
    // PA8/RXD1/AIN12 - BLE_RX，UART1 Rx (與CH32V203通訊)
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    
    // PA9/TMR0/TXD1/AIN13 - BLE_TX，UART1 Tx (與CH32V203通訊)
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    
    // PB11/UD+/TMR2_ - Strip_Detect_3，試紙插入偵測(Falling Edge Interrupt)
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge); // 下降沿中斷
    
    // PB10/UD-/TMR1_ - T3_IN_SEL，T3電極量測致能控制 (High Enable)
    GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA);
    GPIOB_ResetBits(GPIO_Pin_10); // 初始化為低電平
    
    // PB7/TXD0/PWM9 - D-_UART_TX_MCU，UART0 Tx (與外部設備1通訊)
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
    
    // PB4/RXD0/PWM7 - D+_UART_RX_MCU，UART0 Rx (與外部設備1通訊)
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    
    // PA4/RXD3/AIN0 - D-_UART_RX_MCU，UART3 Rx (與外部設備2通訊)
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    
    // PA5/TXD3/AIN1 - D+_UART_TX_MCU，UART3 Tx (與外部設備2通訊)
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);
    
    // PA15/MISO/RXD0_/AIN5 - Strip_Detect_5，試紙插入偵測(Falling Edge Interrupt)
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge); // 下降沿中斷
    
    // PA14/MOSI/TXD0_/AIN4 - V2P5_ENABLE，供電給CH32V203致能控制 (High Enable)
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    GPIOA_SetBits(GPIO_Pin_14); // 初始化為高電平，啟用供電
    
    // PA13/SCK0/PWM5/AIN3 - V_back_C，外部UART設備Rx Level Shift電源選擇控制
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(GPIO_Pin_13); // 初始化為低電平
    
    // PA12/SCS/PWM4/AIN2 - VBUS_Get，量測外部設備是否供電
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
}

/*********************************************************************
 * @fn      CH582F_UART_Init
 *
 * @brief   CH582F UART初始化配置
 *
 * @return  none
 */
void CH582F_UART_Init(void)
{
    // UART1 初始化 - 與CH32V203通訊
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    UART1_ByteTrigCfg(UART_7BYTE_TRIG);
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY); // 啟用接收中斷
    
    // UART0 初始化 - 與外部設備1通訊
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    UART0_ByteTrigCfg(UART_7BYTE_TRIG);
    UART0_INTCfg(ENABLE, RB_IER_RECV_RDY); // 啟用接收中斷
    
    // UART3 初始化 - 與外部設備2通訊
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    UART3_ByteTrigCfg(UART_7BYTE_TRIG);
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY); // 啟用接收中斷
}

/*********************************************************************
 * @fn      CH582F_Init
 *
 * @brief   CH582F完整初始化
 *
 * @return  none
 */
void CH582F_Init(void)
{
    // 系統時鐘設定為60MHz (使用PLL)
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // GPIO初始化
    CH582F_GPIO_Init();
    
    // UART初始化
    CH582F_UART_Init();
    
    // 啟用全局中斷
    PFIC_EnableAllIRQ();
    
    // 啟用看門狗 - 使用以現有代碼初始化 (不用常數)
    WWDG_SetCounter(0);       // 設置看門狗計數初值
    WWDG_ResetCfg(ENABLE);    // 啟用看門狗復位功能
    WWDG_ITCfg(ENABLE);       // 啟用看門狗中斷功能
} 