/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : ����ӻ�Ӧ��������������ϵͳ��ʼ��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
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
u8C MacAddr[6] = {0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x21};  // �A�OMAC��ַ���ɸ���
#endif

// ��MAC��ַ�����O�����Q
void GenerateDeviceName(uint8_t *name, uint8_t *macAddr) {
    // ����MAC��ַ������_�@ʾ�������ֹ�
    sprintf((char*)name, "P14-%02X%02X%02X", macAddr[5], macAddr[4], macAddr[3]);
    PRINT("�O�����Q: %s\n", name);
}

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : ��ѭ�h
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
 * Description    : ������
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // ����UART1���{ԇ�˿�
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    
    // �@ʾMAC��ַ
    PRINT("ʹ��MAC��ַ: %02X:%02X:%02X:%02X:%02X:%02X\n",
          MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    
    // ��ʼ���{����
    CH58X_BLEInit();
    HAL_Init();
    
    // �O���O�����Q��P14-XXXXXX��ʽ
    uint8_t deviceName[20];
    GenerateDeviceName(deviceName, (uint8_t*)MacAddr);
    // �O���O�����Q (ʹ��GGS_SetParameter���GAPRole_SetParameter)
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, strlen((char*)deviceName), deviceName);
    
    // ��ʼ�����O��ɫ
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    // ��ʼ��UARTͨӍ
    app_uart_init();
    
    // ��ѭ�h
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
