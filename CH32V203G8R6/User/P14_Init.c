/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Init.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/01
 * Description        : P14�ĿCH32V203G8R6��ʼ����ʽ
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Init.h"

/*********************************************************************
 * @fn      P14_CH32V203_GPIO_Init
 *
 * @brief   CH32V203��GPIO��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* ����GPIO�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* Red LED (PB6, High Enable) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // ��ʼ�O����ƽ
    
    /* Green LED (PB7, High Enable) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // ��ʼ�O����ƽ
    
    /* TM_IN (PA1, �ض����ݔ��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* MCU_TX/MCU_RX (PA2/PA3, �cCH582FͨӍ) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* T1_OUT (PA6, 늘OT1���y��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* T3_ADC (PB0, 늘OT3���y��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* GLU_OUT (PA4, ���������Y��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* EV_WE (PA7, W늘O늉�) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* KEY_2 (PB10, ���oݔ��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* KEY_1 (PB11, ���oݔ��) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Battery (PB1, 늳�늉��z�y) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* WE_ENABLE (PB15, W늘O늉�PWM����) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // ��ʼ�O����ƽ
    
    /* T1_ENABLE (PA8, T1늘O���y���ܿ���) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // ��ʼ�O����ƽ (Low Enable)
}

/*********************************************************************
 * @fn      P14_CH32V203_UART_Init
 *
 * @brief   CH32V203��UART��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_UART_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    /* ����UART�r� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* USART2 ���� (�cCH582FͨӍ) */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    
    /* ����USART2�����Д� */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* ����USART2 */
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      P14_CH32V203_ParameterTable_Init
 *
 * @brief   �������a���ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_ParameterTable_Init(void)
{
    /* ��ʼ�������� */
    P14_ParamTable_Init();
    
    /* �@ʾ��ǰ�yԇģʽ */
    if (P14_ParamTable_Read(PARAM_FACTORY) == MODE_FACTORY) {
        printf("��ǰģʽ: ���Sģʽ\r\n");
    } else {
        printf("��ǰģʽ: ʹ����ģʽ\r\n");
    }
    
    /* �@ʾ��ǰ�yԇ�Ŀ */
    StripType_TypeDef stripType = (StripType_TypeDef)P14_ParamTable_Read(PARAM_STRIP_TYPE);
    printf("��ǰ�yԇ�Ŀ: %s\r\n", P14_ParamTable_GetStripTypeName(stripType));
    
    /* �@ʾ�y����λ */
    Unit_TypeDef unit = (Unit_TypeDef)P14_ParamTable_Read(PARAM_MGDL);
    printf("��ǰ�y����λ: %s\r\n", P14_ParamTable_GetUnitName(unit));
    
    /* �@ʾ�y���Δ� */
    printf("��Ӌ�y���Δ�: %d\r\n", P14_ParamTable_Read16(PARAM_NOT));
}

/*********************************************************************
 * @fn      P14_CH32V203_Flash_Storage_Init
 *
 * @brief   Flash��������ϵ�y��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_Flash_Storage_Init(void)
{
    /* ��ʼ��Flash��������ϵ�y */
    if (PARAM_Init()) {
        printf("Flash��������ϵ�y��ʼ���ɹ�\r\n");
    } else {
        printf("Flash��������ϵ�y��ʼ��ʧ�������d��Ĭ�Jֵ\r\n");
    }
    
    /* �@ȡ����ϵ�y���� */
    BasicSystemBlock basicParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        /* �@ʾ��ǰ�yԇģʽ */
        if (basicParams.factory == MODE_FACTORY) {
            printf("��ǰģʽ: ���Sģʽ\r\n");
        } else {
            printf("��ǰģʽ: ʹ����ģʽ\r\n");
        }
        
        /* �@ʾ��ǰ�yԇ�Ŀ */
        StripType_TypeDef stripType = (StripType_TypeDef)basicParams.stripType;
        printf("��ǰ�yԇ�Ŀ: %s\r\n", P14_ParamTable_GetStripTypeName(stripType));
        
        /* �@ʾ�y����λ */
        Unit_TypeDef unit = (Unit_TypeDef)basicParams.measureUnit;
        printf("��ǰ�y����λ: %s\r\n", P14_ParamTable_GetUnitName(unit));
        
        /* �@ʾ�y���Δ� */
        printf("��Ӌ�y���Δ�: %d\r\n", basicParams.testCount);
    }
    
    /* �@ȡ�yԇӛ䛔��� */
    uint16_t recordCount = PARAM_GetTestRecordCount();
    printf("�yԇӛ䛔���: %d\r\n", recordCount);
}

/*********************************************************************
 * @fn      P14_CH32V203_System_Init
 *
 * @brief   CH32V203ϵ�y��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void P14_CH32V203_System_Init(void)
{
    /* �O��NVIC�ֽM */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* ����ϵ�y�r� */
    SystemCoreClockUpdate();
    
    /* ��ʼ�����t���� */
    Delay_Init();
    
    /* GPIO��ʼ�� */
    P14_CH32V203_GPIO_Init();
    
    /* UART��ʼ�� */
    P14_CH32V203_UART_Init();
    
    /* Flash��������ϵ�y��ʼ�� */
    P14_CH32V203_Flash_Storage_Init();
    
    /* �@ʾ��ʼ�����ӍϢ */
    printf("P14 CH32V203G8R6 ϵ�y��ʼ�����\r\n");
} 