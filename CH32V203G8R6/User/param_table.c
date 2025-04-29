/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : �๦�������yԇ�x�������a�팍�F�ļ�
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "param_table.h"
#include "string.h"
#include "debug.h"
#include "ch32v20x_flash.h"

/* Flash��������ʼ��ַ */
#define PARAM_TABLE_FLASH_ADDR  0x0800F000  // ʹ��Flash����һ�����惦������

/* ���xȫ�օ�����׃�� */
Param_Table_TypeDef g_ParamTable;

/* ������Ĭ�Jֵ�O�� */
static const Param_Table_TypeDef DefaultParamTable = {
    /* 1. ϵ�y�������� */
    .LBT = 58,                    // ��늳��ֵ: 2.58V
    .OBT = 55,                    // 늳غĽ��ֵ: 2.55V
    .FACTORY = 0,                 // �x������ģʽ: ʹ����ģʽ
    .MODEL_NO = 10,               // �aƷ��̖: 10
    .FW_NO = 1,                   // �g�w�汾̖: 1
    .NOT = 0,                     // �yԇ�Δ�: 0
    .Code_Table_V = 1,            // ���a��汾̖: 1
    
    /* 2. �r�g�O������ */
    .YEAR = 25,                   // ����O��: 2025��
    .MONTH = 4,                   // �·��O��: 4��
    .DATE = 29,                   // �����O��: 29��
    .HOUR = 12,                   // С�r�O��: 12�r
    .MINUTE = 0,                  // ����O��: 0��
    .SECOND = 0,                  // �딵�O��: 0��
    
    /* 3. �yԇ�h������ */
    .TLL = 1,                     // �����ضȹ�������: 1.0��C
    .TLH = 40,                    // �����ضȹ�������: 40.0��C
    .MGDL = UNIT_MMOL_L,          // ��Ȇ�λ�O��: mmol/L
    .EVENT = EVENT_AC,            // �O���¼����: �ո�
    .Strip_Type = STRIP_TYPE_GLV, // �yԇ�Ŀ: Ѫ��(GLV)
    
    /* 4. Ӳ�wУ������ */
    .EV_T3_TRG = 120,             // EV_T3�|�l늉�: 1200mV
    .EV_WORKING = 164,            // �y������늉�늉�
    .EV_T3 = 164,                 // ѪҺ����늘O늉�
    .DACO = 0,                    // DACƫ���a��
    .DACDO = 0,                   // DACУ�����텢��
    .CC211NoDone = 0,             // CC211δ������ (����)
    .CAL_TOL = 8,                 // OPS/OPIУ���ݲ� (0.8%)
    .OPS = 1.0f,                  // OPAУ��б��
    .OPI = 0.0f,                  // OPAУ���ؾ�
    .QCT = 0,                     // QCTУ���yԇ��λԪ�M
    .TOFFSET = 0,                 // �ض�У��ƫ��
    .BOFFSET = 0,                 // 늳�У��ƫ��
    
    /* 5. Ѫ��(GLV/GAV)���Å��� */
    /* 5.1 ԇƬ���� */
    .BG_CSU_TOL = 10,             // ԇƬ�z���ݲ� (3%)
    .BG_NDL = 1000,               // ��ԇƬˮƽ
    .BG_UDL = 800,                // ��ʹ��ԇƬˮƽ
    .BG_BLOOD_IN = 1500,          // ѪҺˮƽ
    .BG_Strip_Lot = "BG2025042900001",  // ԇƬ��̖
    
    /* 5.2 �y���������� */
    .BG_L = 10,                   // Ѫ��ֵ���� (1.0 mmol/L��10 mg/dl)
    .BG_H = 250,                  // Ѫ��ֵ���� (������250��ԭ��333����uint8_t����)
    .BG_T3_E37 = 2000,            // T3 ADV�E37�ֵ
    
    /* 5.3 �yԇ�r�򅢔� (�r�g��λ: 100ms) */
    .BG_TPL_1 = 10,               // �r�g�}�n���� (��һ�M): 1000ms
    .BG_TRD_1 = 20,               // ԭʼ�����r�g (��һ�M): 2000ms
    .BG_EV_Width_1 = 5,           // �}�n�r�g (��һ�M): 500ms
    .BG_TPL_2 = 10,               // �r�g�}�n���� (�ڶ��M): 1000ms
    .BG_TRD_2 = 35,               // ԭʼ�����r�g (�ڶ��M): 3500ms
    .BG_EV_Width_2 = 10,          // �}�n�r�g (�ڶ��M): 1000ms
    
    /* �A�O��������ֵ... */
};

/**
 * @brief  ��ʼ��������
 * @note   ����Flash���Ƿ������Ч������Q����ʼ����ʽ
 * @param  None
 * @retval None
 */
void PARAM_TABLE_Init(void)
{
    if (PARAM_TABLE_Verify() == 0) {
        // Flash�Л]����Ч�������O��Ĭ�Jֵ
        PARAM_TABLE_SetDefaults();
        PARAM_TABLE_SaveToFlash();
        printf("��ʼ��������: ʹ��Ĭ�Jֵ\r\n");
    } else {
        // ��Flash���d�녢����
        PARAM_TABLE_LoadFromFlash();
        printf("��ʼ��������: ��Flash���d��\r\n");
    }
}

/**
 * @brief  �O�Å������Ĭ�Jֵ
 * @param  None
 * @retval None
 */
void PARAM_TABLE_SetDefaults(void)
{
    // �}�uĬ�J������ȫ��׃��
    memcpy(&g_ParamTable, &DefaultParamTable, sizeof(Param_Table_TypeDef));
    
    // Ӌ��У��
    uint16_t checksum = PARAM_TABLE_CalculateChecksum();
    g_ParamTable.SUM_L = checksum & 0xFF;
    g_ParamTable.SUM_H = (checksum >> 8) & 0xFF;
    g_ParamTable.CRC16 = 1;  // ����CRC�����H��Ӌ��CRC16
}

/**
 * @brief  ���������浽Flash
 * @param  None
 * @retval None
 */
void PARAM_TABLE_SaveToFlash(void)
{
    uint32_t flashAddr = PARAM_TABLE_FLASH_ADDR;
    uint32_t *src = (uint32_t *)&g_ParamTable;
    uint16_t len = (sizeof(Param_Table_TypeDef) + 3) / 4;  // ��4�ֹ����λ
    uint32_t i;
    
    FLASH_Unlock();
    
    // ������ʹ��FLASH_ErasePage���治���ڵ�FLASH_ErasePages
    FLASH_ErasePage(PARAM_TABLE_FLASH_ADDR);  // ����һ�Flash
    
    // ��4�ֹ����λ����Flash
    for (i = 0; i < len; i++) {
        FLASH_ProgramWord(flashAddr, *src);
        flashAddr += 4;
        src++;
    }
    
    FLASH_Lock();
    
    printf("�������ѱ��浽Flash\r\n");
}

/**
 * @brief  ��Flash�d�녢����
 * @param  None
 * @retval None
 */
void PARAM_TABLE_LoadFromFlash(void)
{
    memcpy(&g_ParamTable, (void *)PARAM_TABLE_FLASH_ADDR, sizeof(Param_Table_TypeDef));
    printf("�������я�Flash�d��\r\n");
}

/**
 * @brief  Ӌ�ㅢ����У�� (0-674λ��)
 * @param  None
 * @retval У��
 */
uint16_t PARAM_TABLE_CalculateChecksum(void)
{
    uint8_t *tableBytes = (uint8_t *)&g_ParamTable;
    uint16_t sum = 0;
    uint16_t i;
    
    for (i = 0; i < PARAM_TABLE_CHECK_SIZE; i++) {
        sum += tableBytes[i];
    }
    
    return sum;
}

/**
 * @brief  ��C�������Ƿ���Ч
 * @param  None
 * @retval 1: ��Ч; 0: �oЧ
 */
uint8_t PARAM_TABLE_Verify(void)
{
    Param_Table_TypeDef *flashParamTable = (Param_Table_TypeDef *)PARAM_TABLE_FLASH_ADDR;
    uint8_t *tableBytes = (uint8_t *)flashParamTable;
    uint16_t sum = 0;
    uint16_t i;
    uint16_t storedChecksum;
    
    // �z��Flash���Ƿ���څ����� (���Ιz��ĳЩ�P�Iֵ)
    if (flashParamTable->MODEL_NO == 0xFF && flashParamTable->FW_NO == 0xFF) {
        return 0;  // Flashδ��ʼ��
    }
    
    // Ӌ��У��
    for (i = 0; i < PARAM_TABLE_CHECK_SIZE; i++) {
        sum += tableBytes[i];
    }
    
    // �z��У��
    storedChecksum = (flashParamTable->SUM_H << 8) | flashParamTable->SUM_L;
    
    return (sum == storedChecksum) ? 1 : 0;
}

/**
 * @brief  ��ӡ��������Ϣ
 * @param  None
 * @retval None
 */
void PARAM_TABLE_PrintInfo(void)
{
    printf("\r\n==== �๦�������yԇ�x��������Ϣ ====\r\n");
    printf("�aƷ��̖: %d\r\n", g_ParamTable.MODEL_NO);
    printf("�g�w�汾: %d\r\n", g_ParamTable.FW_NO);
    printf("������汾: %d\r\n", g_ParamTable.Code_Table_V);
    printf("�yԇ�Δ�: %d\r\n", g_ParamTable.NOT);
    
    printf("\r\n-- ϵ�y�O�� --\r\n");
    printf("����ģʽ: %s\r\n", (g_ParamTable.FACTORY == FACTORY_FACTORY_MODE) ? "���Sģʽ" : "ʹ����ģʽ");
    printf("��늳��ֵ: %d (�s %d.%02d V)\r\n", g_ParamTable.LBT, g_ParamTable.LBT/100, g_ParamTable.LBT%100);
    printf("늳غĽ��ֵ: %d (�s %d.%02d V)\r\n", g_ParamTable.OBT, g_ParamTable.OBT/100, g_ParamTable.OBT%100);
    
    printf("\r\n-- ���ڕr�g --\r\n");
    printf("���ڕr�g: 20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
           g_ParamTable.YEAR, g_ParamTable.MONTH, g_ParamTable.DATE, 
           g_ParamTable.HOUR, g_ParamTable.MINUTE, g_ParamTable.SECOND);
    
    printf("\r\n-- �yԇ�h�� --\r\n");
    printf("�����ضȹ���: %d.0��C ~ %d.0��C\r\n", g_ParamTable.TLL, g_ParamTable.TLH);
    printf("��Ȇ�λ: %s\r\n", 
           (g_ParamTable.MGDL == UNIT_MMOL_L) ? "mmol/L" : 
           (g_ParamTable.MGDL == UNIT_MG_DL) ? "mg/dL" : "gm/dl");
    
    const char* stripTypes[] = {"Ѫ��(GLV)", "����(U)", "��đ�̴�(C)", "�������֬(TG)", "Ѫ��(GAV)"};
    if (g_ParamTable.Strip_Type < 5) {
        printf("�yԇ�Ŀ: %s\r\n", stripTypes[g_ParamTable.Strip_Type]);
    } else {
        printf("�yԇ�Ŀ: δ֪(%d)\r\n", g_ParamTable.Strip_Type);
    }
    
    const char* eventTypes[] = {"Ʒ��Һ(QC)", "�ո�(AC)", "����(PC)"};
    if (g_ParamTable.EVENT < 3) {
        printf("�¼����: %s\r\n", eventTypes[g_ParamTable.EVENT]);
    } else {
        printf("�¼����: δ֪(%d)\r\n", g_ParamTable.EVENT);
    }
    
    printf("\r\n==== У���Ϣ ====\r\n");
    uint16_t calcChecksum = PARAM_TABLE_CalculateChecksum();
    uint16_t storedChecksum = (g_ParamTable.SUM_H << 8) | g_ParamTable.SUM_L;
    printf("Ӌ��У��: 0x%04X\r\n", calcChecksum);
    printf("�惦У��: 0x%04X\r\n", storedChecksum);
    printf("У�Y��: %s\r\n", (calcChecksum == storedChecksum) ? "��Ч" : "�oЧ");
    
    printf("===================================\r\n\r\n");
}