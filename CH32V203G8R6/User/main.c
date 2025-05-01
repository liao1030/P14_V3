/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/07/23
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "ch32v203_init.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    // �M��������ʼ��
    CH32V203_Init();
    
    printf("CH32V203G8R6 ��ʼ�����\r\n");
    printf("ϵ�y�r�: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

    while(1)
    {
        // ��ѭ�h
        // ���ڴ�̎��ӑ��ó�ʽ��߉݋
        
        // �W�qLEDָʾϵ�y�����\��
        GPIO_SetBits(GPIOB, GPIO_Pin_7); // �GɫLED��
        Delay_Ms(500);
        GPIO_ResetBits(GPIOB, GPIO_Pin_7); // �GɫLED��
        Delay_Ms(500);
    }
}
