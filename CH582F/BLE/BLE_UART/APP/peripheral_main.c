/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : ���O�ęC���ó��򣬳�ʼ���V���B�Ӆ�����Ȼ��V�����B�����C��ͨ�^�Զ��x���Ղ�ݔ����
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app_uart.h"
#include "P14_Init.h"
#include "P14_BLE_Protocol.h"  // ����{���f�h�^�ļ�

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/* �������� */
void BLE_Protocol_Init(void);
static void RegisterBLECallback(void);

/**
 * @brief �]���{�����{����
 */
static void RegisterBLECallback(void)
{
    // �]���{��UART���{����
    ble_uart_set_callback(P14_BLE_UartCallback);
}

/**
 * @brief ��ʼ���{���f�h̎��ģ�K
 */
void BLE_Protocol_Init(void)
{
    // ��ʼ���{���f�h��B�C
    // �����fޒ�{��
    P14_BLE_ProtocolInit();
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
    /* P14 V2.1 ϵ�y��ʼ�� */
    P14_CH582F_System_Init();
    
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    
    /* BLE���P��ʼ�� */
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    /* ��ʼ��P14�{���f�h */
    BLE_Protocol_Init();
    PRINT("P14 BLE Protocol Initialized\r\n");
    
    /* ��ʼ��UART */
    app_uart_init();
    PRINT("UART Initialized\r\n");
    
    /* �M����ѭ�h */
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
