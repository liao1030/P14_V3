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
#include "parameter_table.h"
#include "parameter_test.h"

/* Global typedef */

/* Global define */
#define RUN_EXTENDED_TESTS      0   // �O��0�P�]�����yԇ

/* Global Variable */

/**
 * @brief ���Ѕ�����yԇ
 */
void RunParameterTests(void)
{
    printf("\r\n===== �๦�������yԇ�x�������a��yԇ =====\r\n");
    
    // �xȡϵ�y��������
    printf("��늳��ֵ: %d (%.2fV)\r\n", PARAM_Read(PARAM_LBT), PARAM_Read(PARAM_LBT) * 0.01);
    printf("늳غıM�ֵ: %d (%.2fV)\r\n", PARAM_Read(PARAM_OBT), PARAM_Read(PARAM_OBT) * 0.01);
    printf("�x������ģʽ: %d\r\n", PARAM_Read(PARAM_FACTORY));
    printf("�aƷ��̖: %d\r\n", PARAM_Read(PARAM_MODEL_NO));
    printf("�g�w�汾̖: %d\r\n", PARAM_Read(PARAM_FW_NO));
    
    uint16_t testCount = PARAM_Read16(PARAM_NOT);
    printf("�yԇ�Δ�: %d\r\n", testCount);
    
    // ���Ӝyԇ�Δ��K����
    testCount++;
    PARAM_Write16(PARAM_NOT, testCount);
    printf("���yԇ�Δ���: %d\r\n", testCount);
    
    // �xȡ�r�g�O��
    printf("\r\n�r�g�O��: %02d/%02d/%02d %02d:%02d:%02d\r\n",
           PARAM_Read(PARAM_YEAR) + 2000,
           PARAM_Read(PARAM_MONTH),
           PARAM_Read(PARAM_DATE),
           PARAM_Read(PARAM_HOUR),
           PARAM_Read(PARAM_MINUTE),
           PARAM_Read(PARAM_SECOND));
    
    // �xȡ�yԇ�h������
    printf("\r\n�����ضȹ���: %.1f~%.1f��C\r\n", 
           PARAM_Read(PARAM_TLL) * 1.0,
           PARAM_Read(PARAM_TLH) * 1.0);
    
    // ��Ȇ�λ�O���@ʾ
    uint8_t mgdl = PARAM_Read(PARAM_MGDL);
    printf("��Ȇ�λ�O��: ");
    switch(mgdl) {
        case 0: printf("mmol/L\r\n"); break;
        case 1: printf("mg/dL\r\n"); break;
        case 2: printf("gm/dl\r\n"); break;
        default: printf("δ֪\r\n"); break;
    }
    
    // �xȡ�yԇ�Ŀ
    uint8_t stripType = PARAM_Read(PARAM_STRIP_TYPE);
    printf("�yԇ�Ŀ: ");
    switch(stripType) {
        case STRIP_TYPE_GLV: printf("Ѫ��(GLV)\r\n"); break;
        case STRIP_TYPE_U: printf("����(U)\r\n"); break;
        case STRIP_TYPE_C: printf("��đ�̴�(C)\r\n"); break;
        case STRIP_TYPE_TG: printf("�������֬(TG)\r\n"); break;
        case STRIP_TYPE_GAV: printf("Ѫ��(GAV)\r\n"); break;
        default: printf("δ֪\r\n"); break;
    }
    
    // ��CУ��
    printf("\r\nУ�Y��: %s\r\n", PARAM_VerifyChecksum() ? "���_" : "�e�`");
    
    // ����У�́K���慢��
    PARAM_UpdateChecksum();
    PARAM_Save();
    
#if RUN_EXTENDED_TESTS
    // �~��Ĝyԇ
    Delay_Ms(1000); // ���t1�룬�Ա��^��yԇݔ��
    
    // �@ʾ��Ҫ����
    PARAM_Test_PrintMainParameters();
    
    // �޸�ԇƬ��̖
    Delay_Ms(1000);
    PARAM_Test_SetStripLot(STRIP_TYPE_GLV, "BG202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_U, "UA202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_C, "TC202407001");
    PARAM_Test_SetStripLot(STRIP_TYPE_TG, "TG202407001");
    
    // �鿴�޸ĽY��
    Delay_Ms(1000);
    PARAM_Test_PrintMainParameters();
    
    // �޸ĝ�Ȇ�λ
    Delay_Ms(1000);
    printf("\r\n�޸ĝ�Ȇ�λ�yԇ:\r\n");
    PARAM_Test_SetConcentrationUnit(1); // mg/dL
    
    // �����Ԝyԇ
    Delay_Ms(1000);
    PARAM_Test_RunIntegrityTest();
    
    // ���ܜyԇ
    Delay_Ms(1000);
    PARAM_Test_RunPerformanceTest();
#endif
    
    printf("\r\n�������a��yԇ���\r\n");
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
    // �M��������ʼ��
    CH32V203_Init();
    
    printf("CH32V203G8R6 ��ʼ�����\r\n");
    printf("ϵ�y�r�: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
    
    // ��ʼ���������a��
    PARAM_Init();
    
    // ���Ѕ�����yԇ
    RunParameterTests();

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
