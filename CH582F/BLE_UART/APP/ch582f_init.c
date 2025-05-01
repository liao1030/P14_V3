/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch582f_init.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH582F��ʼ�����ã�����P14_V2.1_Pin�_���x�c��ʼ���O��
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
 * @brief   CH582F GPIO��ʼ������
 *
 * @return  none
 */
void CH582F_GPIO_Init(void)
{
    // PA8/RXD1/AIN12 - BLE_RX��UART1 Rx (�cCH32V203ͨӍ)
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    
    // PA9/TMR0/TXD1/AIN13 - BLE_TX��UART1 Tx (�cCH32V203ͨӍ)
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    
    // PB11/UD+/TMR2_ - Strip_Detect_3��ԇ������ɜy(Falling Edge Interrupt)
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge); // �½����Д�
    
    // PB10/UD-/TMR1_ - T3_IN_SEL��T3늘O���y���ܿ��� (High Enable)
    GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA);
    GPIOB_ResetBits(GPIO_Pin_10); // ��ʼ������ƽ
    
    // PB7/TXD0/PWM9 - D-_UART_TX_MCU��UART0 Tx (�c�ⲿ�O��1ͨӍ)
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
    
    // PB4/RXD0/PWM7 - D+_UART_RX_MCU��UART0 Rx (�c�ⲿ�O��1ͨӍ)
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    
    // PA4/RXD3/AIN0 - D-_UART_RX_MCU��UART3 Rx (�c�ⲿ�O��2ͨӍ)
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    
    // PA5/TXD3/AIN1 - D+_UART_TX_MCU��UART3 Tx (�c�ⲿ�O��2ͨӍ)
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);
    
    // PA15/MISO/RXD0_/AIN5 - Strip_Detect_5��ԇ������ɜy(Falling Edge Interrupt)
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge); // �½����Д�
    
    // PA14/MOSI/TXD0_/AIN4 - V2P5_ENABLE����늽oCH32V203���ܿ��� (High Enable)
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    GPIOA_SetBits(GPIO_Pin_14); // ��ʼ������ƽ�����ù��
    
    // PA13/SCK0/PWM5/AIN3 - V_back_C���ⲿUART�O��Rx Level Shift�Դ�x�����
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(GPIO_Pin_13); // ��ʼ������ƽ
    
    // PA12/SCS/PWM4/AIN2 - VBUS_Get�����y�ⲿ�O���Ƿ��
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
}

/*********************************************************************
 * @fn      CH582F_UART_Init
 *
 * @brief   CH582F UART��ʼ������
 *
 * @return  none
 */
void CH582F_UART_Init(void)
{
    // UART1 ��ʼ�� - �cCH32V203ͨӍ
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    UART1_ByteTrigCfg(UART_7BYTE_TRIG);
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY); // ���ý����Д�
    
    // UART0 ��ʼ�� - �c�ⲿ�O��1ͨӍ
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    UART0_ByteTrigCfg(UART_7BYTE_TRIG);
    UART0_INTCfg(ENABLE, RB_IER_RECV_RDY); // ���ý����Д�
    
    // UART3 ��ʼ�� - �c�ⲿ�O��2ͨӍ
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    UART3_ByteTrigCfg(UART_7BYTE_TRIG);
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY); // ���ý����Д�
}

/*********************************************************************
 * @fn      CH582F_Init
 *
 * @brief   CH582F������ʼ��
 *
 * @return  none
 */
void CH582F_Init(void)
{
    // ϵ�y�r��O����60MHz (ʹ��PLL)
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // GPIO��ʼ��
    CH582F_GPIO_Init();
    
    // UART��ʼ��
    CH582F_UART_Init();
    
    // ����ȫ���Д�
    PFIC_EnableAllIRQ();
    
    // ���ÿ��T�� - ʹ���ԬF�д��a��ʼ�� (���ó���)
    WWDG_SetCounter(0);       // �O�ÿ��T��Ӌ����ֵ
    WWDG_ResetCfg(ENABLE);    // ���ÿ��T����λ����
    WWDG_ITCfg(ENABLE);       // ���ÿ��T���Д๦��
} 