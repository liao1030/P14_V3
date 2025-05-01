/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v203_init.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH32V203G8R6��ʼ�����ã�����P14_V2.1_Pin�_���x�c��ʼ���O��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "debug.h"
#include "ch32v203_init.h"
#include "parameter_table.h"

/*********************************************************************
 * @fn      CH32V203_RCC_Init
 *
 * @brief   CH32V203 ϵ�y�r犳�ʼ��
 *
 * @return  none
 */
void CH32V203_RCC_Init(void)
{
    // ʹ��HSI����ϵ�y�r�Դ���\���l��16MHz
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    
    // �ȴ�HSI����
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    
    // �O��ϵ�y�r�
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    
    // ���ø������O�r�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

/*********************************************************************
 * @fn      CH32V203_GPIO_Init
 *
 * @brief   CH32V203G8R6 GPIO��ʼ������
 *
 * @return  none
 */
void CH32V203_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // PB6/U2DM/SCL/TIM4_CH1 - Red���tɫLED���� (High Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // ��ʼ������ƽ��LED�P�]
    
    // PB7/U2DP/SDA/TIM4_CH2 - Green���GɫLED���� (High Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // ��ʼ������ƽ��LED�P�]
    
    // PA1/ADC1 - TM_IN�������늘Oݔ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA2/ADC2/OP2O0 - MCU_TX��UART2 Tx (�cCH582FͨӍ)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA3/ADC3/OP1O0 - MCU_RX��UART2 Rx (�cCH582FͨӍ)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // ����ݔ��
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA6/ADC6/OP1N1 - T1_OUT������GAVԇ��T1늘O늉�(HCT���)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PB0/ADC8/OP1P1 - T3_ADC������GAVԇ��T3늘O늉�(Ѫ��ē����)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PA4/ADC4/OP2O1 - GLU_OUT������W늘O���(���������Y��)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA7/ADC7/OP2P1 - EV_WE��W늘O늉� (�cOP�Ƿ�����B��)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PB10/OP2N0 - KEY_2�����Iݔ��(�����؜y��)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // ����ݔ��
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB11/OP1N0 - KEY_1�����Iݔ��(�����؜y��)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // ����ݔ��
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB1/ADC9/OP1O1/PB12/TIM1_BKIN - Battery��늳�늉��z�y (���������)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB15/OP1P0/TIM1_CH3N - WE_ENABLE��W늘O늉�PWM����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // ��ʼ������ƽ
    
    // PA8/TIM1_CH1 - T1_ENABLE��T1늘O���y���ܿ��� (Low Enable)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // ��ʼ������ƽ������T1늘O
}

/*********************************************************************
 * @fn      CH32V203_USART_Init
 *
 * @brief   CH32V203G8R6 USART��ʼ������
 *
 * @return  none
 */
void CH32V203_USART_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // USART2��ʼ�� - �cCH582FͨӍ
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_Init(USART2, &USART_InitStructure);
    
    // ����USART2�Д�
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // ���ý����Д�
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      CH32V203_Init
 *
 * @brief   CH32V203G8R6 ������ʼ��
 *
 * @return  none
 */
void CH32V203_Init(void)
{
    // �Д����ȼ��ֽM�O��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    // ϵ�y�r犳�ʼ��
    CH32V203_RCC_Init();
    
    // Systick�ӕr��ʼ��
    Delay_Init();
    
    // GPIO��ʼ��
    CH32V203_GPIO_Init();
    
    // USART��ʼ��
    CH32V203_USART_Init();
} 