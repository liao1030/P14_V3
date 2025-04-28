/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F主程式
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* 全局定義 */
#define STRIP_DETECT_3_PIN     GPIO_Pin_11      /* PB11 試片插入檢測 */
#define T3_IN_SEL_PIN          GPIO_Pin_10      /* PB10 T3電極量測致能控制 */
#define STRIP_DETECT_5_PIN     GPIO_Pin_15      /* PA15 試片插入檢測 */
#define V2P5_ENABLE_PIN        GPIO_Pin_14      /* PA14 供電給CH32V203致能控制 */
#define V_BACK_C_PIN           GPIO_Pin_13      /* PA13 外部UART設備Rx Level Shift電源選擇控制 */
#define VBUS_GET_PIN           GPIO_Pin_12      /* PA12 量測外部設備是否供電 */

/* 函數聲明 */
void SystemInit(void);
void GPIO_Init(void);
void GPIO_INT_Init(void);
void UART_Init(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   主函數
 *
 * @return  none
 */
int main(void)
{
    /* 系統基本初始化 */
    SystemInit();
    
    /* GPIO初始化 */
    GPIO_Init();
    
    /* GPIO中斷初始化 */
    GPIO_INT_Init();
    
    /* UART初始化 */
    UART_Init();
    
    /* 基本測試輸出 */
    PRINT("P14_V2.1 BLE 初始化完成\n");
    PRINT("系統時鐘: %d MHz\n", GetSysClock() / 1000000);
    
    /* 主循環 */
    while(1)
    {
        /* 主循環代碼 */
    }
}

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   系統時鐘初始化
 *
 * @return  none
 */
void SystemInit(void)
{
    /* 解除外設複位 */
    PERIPHERAL_RESET();
    
    /* 系統時鐘初始化 */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* 系統中斷初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* 開啟時鐘 */
    GPIOA_ModeCfg(PIN_WAKEUP_GPIO_ALL, Pin_ModeIN_PD);
    LowPower_Idle();
}

/*********************************************************************
 * @fn      GPIO_Init
 *
 * @brief   GPIO初始化配置
 *
 * @return  none
 */
void GPIO_Init(void)
{
    /* 試片插入檢測引腳 - PB11 */
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    
    /* T3電極量測致能控制 - PB10 (高電平致能) */
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 試片插入檢測引腳 - PA15 */
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    
    /* 供電給CH32V203致能控制 - PA14 (高電平致能) */
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 外部UART設備Rx Level Shift電源選擇控制 - PA13 */
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 量測外部設備是否供電 - PA12 */
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);
    
    /* 初始設定輸出狀態 */
    GPIOB_ResetBits(T3_IN_SEL_PIN);     /* T3電極量測禁用(低電平) */
    GPIOA_SetBits(V2P5_ENABLE_PIN);     /* 供電給CH32V203致能(高電平) */
    GPIOA_ResetBits(V_BACK_C_PIN);      /* UART Level Shift電源選擇(低電平) */
}

/*********************************************************************
 * @fn      GPIO_INT_Init
 *
 * @brief   GPIO中斷初始化配置
 *
 * @return  none
 */
void GPIO_INT_Init(void)
{
    /* 啟用試片插入檢測引腳中斷 - PB11 */
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    
    /* 啟用試片插入檢測引腳中斷 - PA15 */
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    
    /* 啟用GPIO中斷 */
    PFIC_EnableIRQ(GPIO_IRQn);
}

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   UART初始化配置
 *
 * @return  none
 */
void UART_Init(void)
{
    /* UART1初始化 - 與CH32V203通訊 */
    GPIOA_SetBits(GPIO_Pin_8);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);         /* RX (PA8) */
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);    /* TX (PA9) */
    
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    /* UART0初始化 - 與外部設備1通訊 */
    GPIOB_SetBits(GPIO_Pin_4);
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);         /* RX (PB4) */
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);    /* TX (PB7) */
    
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    
    /* UART3初始化 - 與外部設備2通訊 */
    GPIOA_SetBits(GPIO_Pin_4);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);         /* RX (PA4) */
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);    /* TX (PA5) */
    
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    
    /* 啟用UART1中斷接收 */
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
} 