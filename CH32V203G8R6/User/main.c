/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : �๦�������yԇ�x����ʽ
 *********************************************************************************
 * Copyright (c) 2025 HMD Biomedical Inc.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "param_code_table.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      PrintParamInfo
 *
 * @brief   ӡ����������Ϣ
 *
 * @return  none
 */
void PrintParamInfo(void)
{
    printf("\r\n===== �๦�������yԇ�x��������Ϣ =====\r\n");
    printf("������汾: %d.%d\r\n", ParamCodeTable.System.Code_Table_V >> 8, 
                                    ParamCodeTable.System.Code_Table_V & 0xFF);
    printf("�g�w�汾: V%d.%d\r\n", ParamCodeTable.System.FW_NO / 10, 
                                    ParamCodeTable.System.FW_NO % 10);
    printf("�aƷ��̖: %d\r\n", ParamCodeTable.System.MODEL_NO);
    printf("�yԇ�Δ�: %d\r\n", ParamCodeTable.System.NOT);
    
    printf("\r\nϵ�y�r�g: 20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
            ParamCodeTable.Time.YEAR, 
            ParamCodeTable.Time.MONTH, 
            ParamCodeTable.Time.DATE,
            ParamCodeTable.Time.HOUR,
            ParamCodeTable.Time.MINUTE,
            ParamCodeTable.Time.SECOND);
    
    /* �@ʾ��ǰ�yԇ��� */
    printf("\r\n��ǰ�yԇ���: %s\r\n", GetStripTypeName(ParamCodeTable.Display.Strip_Type));
    
    /* �����yԇ����@ʾ���P���� */
    switch(ParamCodeTable.Display.Strip_Type)
    {
        case 0: /* GLV */
        case 4: /* GAV */
            printf("Ѫ�ǜyԇ����: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.BG.L / 10, ParamCodeTable.BG.L % 10,
                    ParamCodeTable.BG.H / 10, ParamCodeTable.BG.H % 10);
            break;
            
        case 1: /* U */
            printf("����yԇ����: %d - %d umol/L\r\n", 
                    ParamCodeTable.U.L * 10, ParamCodeTable.U.H * 10);
            break;
            
        case 2: /* C */
            printf("��đ�̴��yԇ����: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.C.L / 10, ParamCodeTable.C.L % 10,
                    ParamCodeTable.C.H / 10, ParamCodeTable.C.H % 10);
            break;
            
        case 3: /* TG */
            printf("�������֬�yԇ����: %d.%d - %d.%d mmol/L\r\n", 
                    ParamCodeTable.TG.L / 10, ParamCodeTable.TG.L % 10,
                    ParamCodeTable.TG.H / 10, ParamCodeTable.TG.H % 10);
            break;
    }
    
    printf("\r\n�����ضȹ���: %d - %d��C\r\n", 
            ParamCodeTable.Display.TLL, ParamCodeTable.Display.TLH);
    printf("========================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ����ʽ
 *
 * @return  none
 */
int main(void)
{
    uint8_t currentMode;
    
    /* ��ʼ��ϵ�y�O�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("\r\n========================================\r\n");
    printf("�๦�������yԇ�x - ϵ�y����\r\n");
    printf("ϵ�y�r�: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("��ƬID: 0x%08x\r\n", DBGMCU_GetCHIPID());
    
    /* ��ʼ���������a�� */
    printf("��ʼ���������a��...\r\n");
    ParamCodeTable_Init();
    
    /* �@ʾ������Ϣ */
    PrintParamInfo();
    
    /* �z�����ģʽ */
    currentMode = ParamCodeTable.System.FACTORY;
    if(currentMode == 210)
    {
        printf("���Sģʽ����\r\n");
        /* ���й��Sģʽ��ʼ�� */
    }
    else
    {
        printf("ʹ����ģʽ����\r\n");
        /* ����ʹ����ģʽ��ʼ�� */
    }
    
    printf("ϵ�y��ʼ�����\r\n");
    printf("========================================\r\n");
    
    while(1)
    {
        /* ��ѭ�h̎�� */
        Delay_Ms(1000);
    }
}
