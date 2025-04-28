/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外設從機應用主函數及任務系統初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

// P14_V2.1 藍牙Pin腳定義
#define STRIP_DETECT_3_PIN      GPIO_Pin_11    // PB11
#define T3_IN_SEL_PIN           GPIO_Pin_10    // PB10
#define V2P5_ENABLE_PIN         GPIO_Pin_14    // PA14
#define STRIP_DETECT_5_PIN      GPIO_Pin_15    // PA15
#define V_BACK_C_PIN            GPIO_Pin_13    // PA13
#define VBUS_GET_PIN            GPIO_Pin_12    // PA12

/**********************************************************************
 * @fn      P14_GPIOInit
 *
 * @brief   P14_V2.1 GPIO初始化，根據電路圖與Pin腳定義設定
 *
 * @return  none
 */
void P14_GPIOInit(void)
{
    // UART1 - 與CH32V203通訊 (PA8-RX, PA9-TX)
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);  // PA9 - TXD1
    GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);       // PA8 - RXD1

    // UART0 - 與外部設備1通訊 (PB7-TX, PB4-RX)
    GPIOB_ModeCfg(bTXD0, GPIO_ModeOut_PP_5mA);  // PB7 - TXD0
    GPIOB_ModeCfg(bRXD0, GPIO_ModeIN_PU);       // PB4 - RXD0

    // UART3 - 與外部設備2通訊 (PA5-TX, PA4-RX)
    GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);  // PA5 - TXD3
    GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);       // PA4 - RXD3

    // 試片檢測與控制
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);  // PB11 - 試片插入檢測
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);  // PB10 - T3電極控制
    GPIOB_ResetBits(T3_IN_SEL_PIN);  // 預設關閉

    // 電源控制
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);  // PA14 - CH32V203供電控制
    GPIOA_SetBits(V2P5_ENABLE_PIN);  // 預設開啟供電

    // 其他IO初始化
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);  // PA15 - 試片插入檢測
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);   // PA13 - UART Level Shift控制
    GPIOA_ResetBits(V_BACK_C_PIN);  // 預設低電平
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);  // PA12 - 外部供電檢測
}

/**********************************************************************
 * @fn      P14_UARTInit
 *
 * @brief   初始化所有UART介面
 *
 * @return  none
 */
void P14_UARTInit(void)
{
    // UART1與CH32V203通訊
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    // UART0與外部設備1通訊
    UART0_DefInit();
    UART0_BaudRateCfg(115200);

    // UART3與外部設備2通訊
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
}

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循環
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函數
 *
 * @return  none
 */
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);  // 設定系統時鐘為60MHz

#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif

    // 初始化P14_V2.1專用GPIO配置
    P14_GPIOInit();

    // 初始化UART通訊
    P14_UARTInit();

    // Debug資訊輸出
    PRINT("P14_V2.1 CH582F 初始化完成\n");
    PRINT("系統時鐘: 60MHz\n");

    // 初始化藍牙功能
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    // 進入主循環
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
