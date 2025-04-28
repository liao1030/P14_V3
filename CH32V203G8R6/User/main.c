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
 *P14_V2.1 CH32V203G8R6 ��ʼ����ʽ:
 *����ϵ�y�r犡�GPIO��UART������
 *
 */

#include "debug.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_usart.h"
#include "ch32v20x_rcc.h"

/* Global typedef */

/* Global define */
// LED�������_���x
#define LED_RED_PORT    GPIOB
#define LED_RED_PIN     GPIO_Pin_6
#define LED_GREEN_PORT  GPIOB
#define LED_GREEN_PIN   GPIO_Pin_7

// ���Iݔ�����_���x
#define KEY1_PORT       GPIOB
#define KEY1_PIN        GPIO_Pin_11
#define KEY2_PORT       GPIOB
#define KEY2_PIN        GPIO_Pin_10

// 늘O�����c�z�y���_���x
#define T1_ENABLE_PORT  GPIOA
#define T1_ENABLE_PIN   GPIO_Pin_8
#define WE_ENABLE_PORT  GPIOB
#define WE_ENABLE_PIN   GPIO_Pin_15

/* Global Variable */

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   GPIO��ʼ������
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // ���������GPIO�r�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    // LED��ʼ�� (ݔ�����ƽ)
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_RED_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(LED_RED_PORT, LED_RED_PIN); // �A�O�P�]LED

    GPIO_InitStructure.GPIO_Pin = LED_GREEN_PIN;
    GPIO_Init(LED_GREEN_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN); // �A�O�P�]LED

    // ���I��ʼ�� (����ݔ��)
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY2_PIN;
    GPIO_Init(KEY2_PORT, &GPIO_InitStructure);

    // T1늘O���� (ݔ�����ƽ��Low Enable)
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(T1_ENABLE_PORT, &GPIO_InitStructure);
    GPIO_SetBits(T1_ENABLE_PORT, T1_ENABLE_PIN); // �A�O����T1늘O

    // WE늘O���� (ݔ�����ƽ��High Enable)
    GPIO_InitStructure.GPIO_Pin = WE_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(WE_ENABLE_PORT, &GPIO_InitStructure);
    GPIO_SetBits(WE_ENABLE_PORT, WE_ENABLE_PIN); // �A�O����WE늘O

    // �������_��ʼ�� (����ݔ��)
    // TM_IN (PA1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // T1_OUT (PA6)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // T3_ADC (PB0)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // GLU_OUT (PA4)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // EV_WE (PA7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Battery (PB1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      USART2_Init
 *
 * @brief   USART2��ʼ������ (����cCH582FͨӍ)
 *
 * @return  none
 */
void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // USART2_TX (PA2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART2_RX (PA3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    // ��ʼ��ϵ�y�r犺��Д��������M
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // ��ʼ��GPIO��USART
    GPIO_Toggle_INIT();
    USART_Printf_Init(115200); // Debug UART
    USART2_Init(); // ����cCH582FͨӍ��UART

    printf("P14_V2.1 CH32V203G8R6 ��ʼ�����\r\n");
    printf("ϵ�y�r�: %d MHz\r\n", SystemCoreClock/1000000);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

    while(1)
    {
        // �W�q�GɫLED��ʾϵ�y�\������
        GPIO_SetBits(LED_GREEN_PORT, LED_GREEN_PIN);
        Delay_Ms(500);
        GPIO_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN);
        Delay_Ms(500);
    }
}
