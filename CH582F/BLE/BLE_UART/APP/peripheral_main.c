/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外設從機應用程序，初始化廣播連接參數，然後廣播，連接主機后，通過自定義服務傳輸數據
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
#include "app_uart.h"
#include "P14_Init.h"
#include "P14_BLE_Protocol.h"  // 添加藍牙協議頭文件

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/* 函數宣告 */
void BLE_Protocol_Init(void);
static void RegisterBLECallback(void);

/**
 * @brief 註冊藍牙回調函數
 */
static void RegisterBLECallback(void)
{
    // 註冊藍牙UART回調函數
    ble_uart_set_callback(P14_BLE_UartCallback);
}

/**
 * @brief 初始化藍牙協議處理模塊
 */
void BLE_Protocol_Init(void)
{
    // 初始化藍牙協議狀態機
    // 避免遞迴調用
    P14_BLE_ProtocolInit();
}

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : 主循環
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        app_uart_process();
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函數
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    /* P14 V2.1 系統初始化 */
    P14_CH582F_System_Init();
    
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    
    /* BLE相關初始化 */
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    /* 初始化P14藍牙協議 */
    BLE_Protocol_Init();
    PRINT("P14 BLE Protocol Initialized\r\n");
    
    /* 初始化UART */
    app_uart_init();
    PRINT("UART Initialized\r\n");
    
    /* 進入主循環 */
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
