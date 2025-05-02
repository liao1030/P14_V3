/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2024 HMD Biomedical Inc.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *�๦�������yԇ�x�������a���ʽ:
 *��ʾ����ʽ��ʾ��γ�ʼ�����xȡ�ͱ���๦�������yԇ�x�ą������a��
 *USART1_Tx(PA9) - ���ݔ���{ԇ��Ϣ
 *
 */

#include "debug.h"
#include "param_table.h"

/* ȫ��׃�����x */
ParamTable_TypeDef g_ParamTable;

/*********************************************************************
 * @fn      printParamTableInfo
 *
 * @brief   ��ӡ�����������Ϣ
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  none
 */
void printParamTableInfo(ParamTable_TypeDef *param_table)
{
    printf("\r\n====== �๦�������yԇ�x��������Ϣ ======\r\n");
    printf("�g�w�汾: V%d.%d\r\n", param_table->FW_NO / 10, param_table->FW_NO % 10);
    printf("������汾: %d\r\n", param_table->Code_Table_V);
    printf("�yԇ�Δ�: %d\r\n", param_table->NOT);
    printf("����ģʽ: %s\r\n", (param_table->FACTORY == MODE_USER) ? "ʹ����ģʽ" : "���Sģʽ");
    
    printf("\r\n-- ���ڕr�g�O�� --\r\n");
    printf("����: 20%02d/%02d/%02d\r\n", param_table->YEAR, param_table->MONTH, param_table->DATE);
    printf("�r�g: %02d:%02d:%02d\r\n", param_table->HOUR, param_table->MINUTE, param_table->SECOND);
    
    printf("\r\n-- �yԇ�h������ --\r\n");
    printf("�����ضȹ���: %.1f��C ~ %.1f��C\r\n", (float)param_table->TLL / 10, (float)param_table->TLH / 10);
    
    char *unit_str = "δ֪";
    switch(param_table->MGDL) {
        case UNIT_MMOL_L: unit_str = "mmol/L"; break;
        case UNIT_MG_DL: unit_str = "mg/dL"; break;
        case UNIT_GM_DL: unit_str = "gm/dl"; break;
    }
    printf("�y����λ: %s\r\n", unit_str);
    
    char *strip_type_str = "δ֪";
    switch(param_table->Strip_Type) {
        case STRIP_TYPE_GLV: strip_type_str = "Ѫ��(GLV)"; break;
        case STRIP_TYPE_UA: strip_type_str = "����(U)"; break;
        case STRIP_TYPE_CHOL: strip_type_str = "��đ�̴�(C)"; break;
        case STRIP_TYPE_TG: strip_type_str = "�������֬(TG)"; break;
        case STRIP_TYPE_GAV: strip_type_str = "Ѫ��(GAV)"; break;
    }
    printf("�A�O�yԇ�Ŀ: %s\r\n", strip_type_str);
    
    printf("\r\n-- ��ǰ�yԇ�ĿԇƬ��Ϣ --\r\n");
    switch(param_table->Strip_Type) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            printf("��̖: %s\r\n", param_table->BG_Strip_Lot);
            printf("�y������: %d - %d mg/dL\r\n", param_table->BG_L, param_table->BG_H);
            break;
        case STRIP_TYPE_UA:
            printf("��̖: %s\r\n", param_table->U_Strip_Lot);
            printf("�y������: %d - %d mg/dL\r\n", param_table->U_L, param_table->U_H);
            break;
        case STRIP_TYPE_CHOL:
            printf("��̖: %s\r\n", param_table->C_Strip_Lot);
            printf("�y������: %d - %d mg/dL\r\n", param_table->C_L, param_table->C_H);
            break;
        case STRIP_TYPE_TG:
            printf("��̖: %s\r\n", param_table->TG_Strip_Lot);
            printf("�y������: %d - %d mg/dL\r\n", param_table->TG_L, param_table->TG_H);
            break;
    }
    
    printf("\r\n-- У���Ϣ --\r\n");
    uint16_t stored_checksum = ((uint16_t)param_table->SUM_H << 8) | param_table->SUM_L;
    printf("У��: 0x%04X\r\n", stored_checksum);
    printf("CRC16: 0x%02X\r\n", param_table->CRC16);
    
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main(void)
{
    uint8_t load_result;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("ϵ�y�r��l��: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("��ƬID: %08x\r\n", DBGMCU_GetCHIPID());
    printf("�๦�������yԇ�x�������a���ʽ\r\n");
    
    /* ��Flash���d���������У�ʧ���t��ʼ����Ĭ�Jֵ */
    load_result = ParamTable_LoadFromFlash(&g_ParamTable);
    
    if(load_result != 0) {
        printf("������У�ʧ�����ѳ�ʼ����Ĭ�Jֵ\r\n");
        /* ��ʼ���ᱣ�浽Flash */
        ParamTable_SaveToFlash(&g_ParamTable);
    } else {
        printf("�ɹ���Flash���d������\r\n");
    }
    
    /* ���yԇ�Δ� */
    g_ParamTable.NOT++;
    
    /* ���������ą����� */
    ParamTable_SaveToFlash(&g_ParamTable);
    
    /* ��ӡ��������Ϣ */
    printParamTableInfo(&g_ParamTable);
    
    /* ��ѭ�h */
    while(1)
    {
    }
}
