/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Init.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/01
 * Description        : P14�ĿCH582F��ʼ����ʽ
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_Init.h"

/*********************************************************************
 * @fn      P14_CH582F_GPIO_Init
 *
 * @brief   CH582F��GPIO��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_GPIO_Init(void)
{
    /* BLE_RX/BLE_TX (PA8/PA9 - UART1�cCH32V203ͨӍ) */
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RX, ����ݔ��
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TX, ����ݔ��
    
    /* Strip_Detect_3 (PB11 - ԇ������z�y, �½����Д�) */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);     // ����ݔ��
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge); // �O�Þ��½����|�l�Д�
    
    /* T3_IN_SEL (PB10 - T3늘O���y���ܿ���) */
    GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA); // ����ݔ��
    GPIOB_ResetBits(GPIO_Pin_10);                   // ��ʼ�O����ƽ
    
    /* D-_UART_TX_MCU/D+_UART_RX_MCU (PB7/PB4 - UART0�c�ⲿ�O��1ͨӍ) */
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RX, ����ݔ��
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TX, ����ݔ��
    
    /* D-_UART_RX_MCU/D+_UART_TX_MCU (PA4/PA5 - UART3�c�ⲿ�O��2ͨӍ) */
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RX, ����ݔ��
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TX, ����ݔ��
    
    /* Strip_Detect_5 (PA15 - ԇ������z�y, �½����Д�) */
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);     // ����ݔ��
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge); // �O�Þ��½����|�l�Д�
    
    /* V2P5_ENABLE (PA14 - �����CH32V203���ܿ���) */
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA); // ����ݔ��
    GPIOA_SetBits(GPIO_Pin_14);                     // ��ʼ�O����ƽ
    
    /* V_back_C (PA13 - �ⲿUART�O��Rx Level Shift�Դ�x�����) */
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA); // ����ݔ��
    GPIOA_ResetBits(GPIO_Pin_13);                   // ��ʼ�O����ƽ
    
    /* VBUS_Get (PA12 - ���y�ⲿ�O���Ƿ��) */
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating); // ����ݔ��
}

/*********************************************************************
 * @fn      P14_CH582F_UART_Init
 *
 * @brief   CH582F��UART��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_UART_Init(void)
{
    /* UART1��ʼ�� - �cCH32V203ͨӍ */
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�_�����|�l�c4λԪ�M
    R8_UART1_LCR = RB_LCR_WORD_SZ; // 8λ����λ��1λֹͣλ���oУ�λ
    R8_UART1_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // ���ðl�ͺͽ����Д�
    R8_UART1_DIV = 1;
    
    /* UART0��ʼ�� - �c�ⲿ�O��1ͨӍ */
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�_�����|�l�c4λԪ�M
    R8_UART0_LCR = RB_LCR_WORD_SZ; // 8λ����λ��1λֹͣλ���oУ�λ
    R8_UART0_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // ���ðl�ͺͽ����Д�
    R8_UART0_DIV = 1;
    
    /* UART3��ʼ�� - �c�ⲿ�O��2ͨӍ */
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�_�����|�l�c4λԪ�M
    R8_UART3_LCR = RB_LCR_WORD_SZ; // 8λ����λ��1λֹͣλ���oУ�λ
    R8_UART3_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY; // ���ðl�ͺͽ����Д�
    R8_UART3_DIV = 1;
}

/*********************************************************************
 * @fn      P14_CH582F_System_Init
 *
 * @brief   CH582Fϵ�y��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH582F_System_Init(void)
{
    /* �O��ϵ�y�r犞�60MHz */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* GPIO��ʼ�� */
    P14_CH582F_GPIO_Init();
    
    /* UART��ʼ�� */
    P14_CH582F_UART_Init();
    
    /* ����ȫ���Д� */
    PFIC_EnableAllIRQ();
    
    /* У�ʃȲ�32K�r� */
    Calibration_LSI(Level_64);
} 