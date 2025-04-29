/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *P14_V2.1 ����ʽ
 *CH32V203G8R6 MCU��ʼ���c��Ҫ���܌��F
 */

#include "debug.h"
#include "param_table.h"
#include "param_test.h"

/* ȫ�ֶ��x */
#define LED_RED_PIN     GPIO_Pin_6
#define LED_GREEN_PIN   GPIO_Pin_7
#define KEY_1_PIN       GPIO_Pin_11
#define KEY_2_PIN       GPIO_Pin_10
#define T1_ENABLE_PIN   GPIO_Pin_8
#define WE_ENABLE_PIN   GPIO_Pin_15

/* ������ */
void GPIO_Config(void);
void UART2_Config(void);
void Key_Process(void);
void ParamTable_Test(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    /* �Д����ȼ��ֽM�O�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* ϵ�y�r犳�ʼ�� - ʹ��HSI�r�Դ�\����16MHz */
    SystemCoreClockUpdate();
    
    /* ���t��ʼ�� */
    Delay_Init();
    
    /* GPIO ��ʼ�� */
    GPIO_Config();
    
    /* UART2 ��ʼ�� - �cCH582FͨӍ */
    UART2_Config();
    
    /* �{ԇ��UART��ʼ�� */
    USART_Printf_Init(115200);
    printf("P14_V2.1 MCU ��ʼ�����\r\n");
    printf("SystemClk: %d MHz\r\n", SystemCoreClock/1000000);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

    /* �������ʼ�� */
    if (Param_Init() == 0) {
        printf("Parameter table initialized successfully.\r\n");
        GPIO_SetBits(GPIOB, LED_GREEN_PIN); /* �GɫLED�c����ʾ�������ʼ���ɹ� */
    } else {
        printf("Parameter table initialization failed.\r\n");
        GPIO_SetBits(GPIOB, LED_RED_PIN); /* �tɫLED�c����ʾ�������ʼ��ʧ�� */
    }

    /* ����һ�΅�����yԇ */
    ParamTable_Test();

    while(1)
    {
        /* ̎���I */
        Key_Process();
        
        /* ���t */
        Delay_Ms(50);
    }
}

/*********************************************************************
 * @fn      GPIO_Config
 *
 * @brief   GPIO��ʼ������
 *
 * @return  none
 */
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* ʹ��GPIOA��GPIOB�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* LED�������_���� - PB6(�tɫ)��PB7(�Gɫ) */
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ���Iݔ������ - PB10��PB11 (���Iݔ�룬����ݔ��) */
    GPIO_InitStructure.GPIO_Pin = KEY_1_PIN | KEY_2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* T1늘O���y���ܿ��� - PA8 (���ƽ����) */
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W늘O늉�PWM���� - PB15 */
    GPIO_InitStructure.GPIO_Pin = WE_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* �ض����ݔ�� - PA1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ԇƬT1늘O늉�ݔ�� - PA6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ԇƬT3늘O늉�ݔ�� - PB0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ���������Y��ݔ�� - PA4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W늘O늉�ݔ�� - PA7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 늳�늉��z�y - PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ��ʼ�O��ݔ����B */
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);    /* T1늘O���y����(���ƽ) */
    GPIO_SetBits(GPIOB, WE_ENABLE_PIN);    /* W늘O늉�PWMݔ������(���ƽ) */
    GPIO_ResetBits(GPIOB, LED_RED_PIN);    /* �tɫLED�P�] */
    GPIO_ResetBits(GPIOB, LED_GREEN_PIN);  /* �GɫLED�P�] */
}

/*********************************************************************
 * @fn      UART2_Config
 *
 * @brief   UART2���� - �cCH582FͨӍ
 *
 * @return  none
 */
void UART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    
    /* ʹ��GPIOA��UART2�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* UART2 TX(PA2)���_���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2 RX(PA3)���_���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2�������� */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    /* ����UART2 */
    USART_Init(USART2, &USART_InitStructure);
    
    /* ʹ��UART2 */
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      ParamTable_Test
 *
 * @brief   �yԇ�������ܣ���ӡ��ǰ�����O���K�޸�ĳ����
 *
 * @return  none
 */
void ParamTable_Test(void);

/*********************************************************************
 * @fn      Key_Process
 *
 * @brief   ̎���Iݔ��
 *
 * @return  none
 */
void Key_Process(void)
{
    static uint8_t key1_last = 1;
    static uint8_t key2_last = 1;
    uint8_t key1_curr, key2_curr;
    
    /* �xȡ��ǰ���I��B */
    key1_curr = GPIO_ReadInputDataBit(GPIOB, KEY_1_PIN);
    key2_curr = GPIO_ReadInputDataBit(GPIOB, KEY_2_PIN);
    
    /* �z�yKEY1���� (�½���) */
    if (key1_last == 1 && key1_curr == 0) {
        printf("KEY1 pressed - Entering parameter test menu\r\n");
        Param_TestMenu(); /* �M�녢����yԇ�x�� */
    }
    
    /* �z�yKEY2���� (�½���) */
    if (key2_last == 1 && key2_curr == 0) {
        printf("KEY2 pressed - Toggling LED\r\n");
        
        /* �ГQLED��B */
        if (GPIO_ReadOutputDataBit(GPIOB, LED_GREEN_PIN)) {
            GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
        } else {
            GPIO_SetBits(GPIOB, LED_GREEN_PIN);
        }
    }
    
    /* ���°��I��B */
    key1_last = key1_curr;
    key2_last = key2_curr;
}
