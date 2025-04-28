/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : ���O�ęC�������������΄�ϵ�y��ʼ��
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

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

// P14_V2.1 �{��Pin�_���x
#define STRIP_DETECT_3_PIN      GPIO_Pin_11    // PB11
#define T3_IN_SEL_PIN           GPIO_Pin_10    // PB10
#define V2P5_ENABLE_PIN         GPIO_Pin_14    // PA14
#define STRIP_DETECT_5_PIN      GPIO_Pin_15    // PA15
#define V_BACK_C_PIN            GPIO_Pin_13    // PA13
#define VBUS_GET_PIN            GPIO_Pin_12    // PA12

/**********************************************************************
 * @fn      P14_GPIOInit
 *
 * @brief   P14_V2.1 GPIO��ʼ���������·�D�cPin�_���x�O��
 *
 * @return  none
 */
void P14_GPIOInit(void)
{
    // UART1 - �cCH32V203ͨӍ (PA8-RX, PA9-TX)
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);  // PA9 - TXD1
    GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);       // PA8 - RXD1

    // UART0 - �c�ⲿ�O��1ͨӍ (PB7-TX, PB4-RX)
    GPIOB_ModeCfg(bTXD0, GPIO_ModeOut_PP_5mA);  // PB7 - TXD0
    GPIOB_ModeCfg(bRXD0, GPIO_ModeIN_PU);       // PB4 - RXD0

    // UART3 - �c�ⲿ�O��2ͨӍ (PA5-TX, PA4-RX)
    GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);  // PA5 - TXD3
    GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);       // PA4 - RXD3

    // ԇƬ�z�y�c����
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);  // PB11 - ԇƬ����z�y
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);  // PB10 - T3늘O����
    GPIOB_ResetBits(T3_IN_SEL_PIN);  // �A�O�P�]

    // �Դ����
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);  // PA14 - CH32V203��늿���
    GPIOA_SetBits(V2P5_ENABLE_PIN);  // �A�O�_�����

    // ����IO��ʼ��
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);  // PA15 - ԇƬ����z�y
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);   // PA13 - UART Level Shift����
    GPIOA_ResetBits(V_BACK_C_PIN);  // �A�O���ƽ
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);  // PA12 - �ⲿ��늙z�y
}

/**********************************************************************
 * @fn      P14_UARTInit
 *
 * @brief   ��ʼ������UART����
 *
 * @return  none
 */
void P14_UARTInit(void)
{
    // UART1�cCH32V203ͨӍ
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    // UART0�c�ⲿ�O��1ͨӍ
    UART0_DefInit();
    UART0_BaudRateCfg(115200);

    // UART3�c�ⲿ�O��2ͨӍ
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
}

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   ��ѭ�h
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
 * @brief   ������
 *
 * @return  none
 */
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);  // �O��ϵ�y�r犞�60MHz

#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif

    // ��ʼ��P14_V2.1����GPIO����
    P14_GPIOInit();

    // ��ʼ��UARTͨӍ
    P14_UARTInit();

    // Debug�YӍݔ��
    PRINT("P14_V2.1 CH582F ��ʼ�����\n");
    PRINT("ϵ�y�r�: 60MHz\n");

    // ��ʼ���{������
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    // �M����ѭ�h
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
