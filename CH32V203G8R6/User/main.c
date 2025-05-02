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
#include "strip_detection.h"  // ����ԇƬ����Єe�^�ļ�

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
    
    // ��ʼ��ԇƬ����Єe����
    Strip_Detection_Init();
    
    // ��ӡ��������Ϣ
    Parameter_PrintInfo();
    
    printf("ϵ�y��ʼ����ɣ��ȴ�ԇƬ����...\r\n");
    
    // ��ѭ�h
    while(1)
    {
        // ��Ҫ��ʽ߉݋
        // ����ԇƬ��B���в�ͬ����
        switch(Strip_GetStatus()) {
            case STRIP_STATUS_IDENTIFIED:
                // ԇƬ������R�e�������_ʼ�yԇ����
                printf("ԇƬ������R�e���ʂ��_ʼ�yԇ...\r\n");
                // �@�e���Լ���yԇ���P�ĳ�ʽ�a
                break;
                
            default:
                // ������B����Ҫ�؄e̎��
                break;
        }
        
        // �����ӕr������CPUؓ���^��
        Delay_Ms(10);
    }
}
