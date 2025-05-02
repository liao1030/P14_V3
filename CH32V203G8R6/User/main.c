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
 *�๦�������yԇ�x��
 *ʹ��CH32V203G8R6���F�๦�������yԇ�x�ą������a�����
 *
 */

#include "debug.h"
#include "parameter_code.h"

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
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("�๦�������yԇ�x��ʼ����...\r\n");
    
    // ��ʼ��������
    Parameter_Init();
    printf("�������ʼ�����\r\n");
    
    // ��ӡ��������Ϣ
    Parameter_PrintInfo();
    
    // ��ѭ�h
    while(1)
    {
        // ��Ҫ��ʽ߉݋
    }
}
