/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app_uart.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x21};  // 預設MAC地址，可更新
#endif

// 從MAC地址生成設備名稱
void GenerateDeviceName(uint8_t *name, uint8_t *macAddr) {
    // 修正MAC地址順序，正確顯示後三個字節
    sprintf((char*)name, "P14-%02X%02X%02X", macAddr[5], macAddr[4], macAddr[3]);
    PRINT("設備名稱: %s\n", name);
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
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // 配置UART1為調試端口
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    
    // 顯示MAC地址
    PRINT("使用MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\n",
          MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    
    // 初始化藍牙庫
    CH58X_BLEInit();
    HAL_Init();
    
    // 設置設備名稱為P14-XXXXXX格式
    uint8_t deviceName[20];
    GenerateDeviceName(deviceName, (uint8_t*)MacAddr);
    // 設置設備名稱 (使用GGS_SetParameter替代GAPRole_SetParameter)
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, strlen((char*)deviceName), deviceName);
    
    // 初始化外設角色
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    // 初始化UART通訊
    app_uart_init();
    
    // 主循環
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
