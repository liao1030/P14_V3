/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : �๦�������yԇ�x�������a�팍�F�ļ�
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_table.h"
#include "string.h"
#include "ch32v20x_flash.h"

/* ���x��������Flash�еĴ惦λ�� */
#define PARAM_TABLE_FLASH_ADDR   0x08010000  // ���O��Flash��64KB̎���慢����

/*********************************************************************
 * @fn      ParamTable_Init
 *
 * @brief   ��ʼ���������O�����������A�Oֵ
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  none
 */
void ParamTable_Init(ParamTable_TypeDef *param_table)
{
    /* ������Ѕ������O��0 */
    memset(param_table, 0, sizeof(ParamTable_TypeDef));

    /* 1. ��ʼ��ϵ�y�������� */
    param_table->LBT = 58;                // 2.58V (늳؈Dʾ�W�q)
    param_table->OBT = 55;                // 2.55V (늳؈Dʾ����)
    param_table->FACTORY = MODE_USER;     // ʹ����ģʽ
    param_table->MODEL_NO = 1;            // �aƷ��̖ (�������H�O��)
    param_table->FW_NO = 10;              // �g�w�汾̖ V1.0
    param_table->NOT = 0;                 // �yԇ�Δ���0�_ʼ
    param_table->Code_Table_V = 1;        // ���a��汾��̖

    /* 2. ��ʼ���r�g�O������ (�O�Þ鮔ǰ�r�g���A�Oֵ) */
    param_table->YEAR = 24;       // 2024��
    param_table->MONTH = 9;       // 9��
    param_table->DATE = 18;       // 18��
    param_table->HOUR = 12;       // 12�r
    param_table->MINUTE = 0;      // 0��
    param_table->SECOND = 0;      // 0��

    /* 3. ��ʼ���yԇ�h������ */
    param_table->TLL = 10;               // �����ضȹ������� 10.0��C
    param_table->TLH = 40;               // �����ضȹ������� 40.0��C
    param_table->MGDL = UNIT_MG_DL;      // �A�O��λ mg/dL
    param_table->EVENT = EVENT_AC;       // �A�O�¼���� ��ǰ�yԇ
    param_table->Strip_Type = STRIP_TYPE_GLV; // �A�O�yԇ�Ŀ Ѫ��

    /* 4. ��ʼ��Ӳ�wУ�ʅ��� */
    param_table->EV_T3_TRG = 180;         // EV_T3�|�l늉� 1800mV
    param_table->EV_WORKING = 164;        // �y������늘O늉�
    param_table->EV_T3 = 164;             // ѪҺ�z�y늘O늉�
    param_table->DACO = 0;                // DACƫ���a��
    param_table->DACDO = 0;               // DACУ�����텢��
    param_table->CC211NoDone = 0;         // CC211δ������
    param_table->CAL_TOL = 8;             // OPS/OPIУ���ݲ� 0.8%
    param_table->OPS = 1.0f;              // OPAУ��б��
    param_table->OPI = 0.0f;              // OPAУ�ʽؾ�
    param_table->QCT = 0;                 // QCTУ�ʜyԇ��λԪ�M
    param_table->TOFFSET = 0;             // �ض�У��ƫ��
    param_table->BOFFSET = 0;             // 늳�У��ƫ��

    /* ��ʼ����헜yԇ������Ĭ�Jֵ */
    /* 5. Ѫ�ǌ��Å��� */
    param_table->BG_CSU_TOL = 10;         // ԇƬ�z���ݲ�
    param_table->BG_NDL = 100;            // ��ԇƬ�z�yˮƽ
    param_table->BG_UDL = 90;             // ��ʹ��ԇƬ�z�yˮƽ
    param_table->BG_BLOOD_IN = 200;       // ѪҺ�z�yˮƽ
    /* �O��Ĭ�J��̖ */
    strcpy((char *)param_table->BG_Strip_Lot, "GLV20240918");
    
    param_table->BG_L = 20;               // Ѫ��ֵ���� 20 mg/dL
    param_table->BG_H = 600;              // Ѫ��ֵ���� 600 mg/dL
    param_table->BG_T3_E37 = 2000;        // T3 ADV�e�`37�ֵ

    /* �O�Üyԇ�r�򅢔� */
    param_table->BG_TPL_1 = 10;           // 1��
    param_table->BG_TRD_1 = 50;           // 5��
    param_table->BG_EV_Width_1 = 2;       // 0.2��
    param_table->BG_TPL_2 = 10;           // 1��
    param_table->BG_TRD_2 = 30;           // 3��
    param_table->BG_EV_Width_2 = 2;       // 0.2��

    /* �O��Ʒ��Һ���� */
    param_table->BG_Sq = 100;
    param_table->BG_Iq = 0;
    param_table->BG_CVq = 100;            // �P�]QC����
    param_table->BG_Aq = 5;
    param_table->BG_Bq = 5;
    param_table->BG_Aq5 = 5;
    param_table->BG_Bq5 = 5;
    
    /* �O��Ӌ��ʽ�a������ */
    param_table->BG_S2 = 100;
    param_table->BG_I2 = 0;
    param_table->BG_Sr = 1.0f;
    param_table->BG_Ir = 0.0f;

    /* 6. ���ጣ�Å��� */
    param_table->U_CSU_TOL = 10;
    param_table->U_NDL = 100;
    param_table->U_UDL = 90;
    param_table->U_BLOOD_IN = 200;
    strcpy((char *)param_table->U_Strip_Lot, "UA20240918");
    
    param_table->U_L = 3;
    param_table->U_H = 20;
    param_table->U_T3_E37 = 2000;

    param_table->U_TPL_1 = 10;
    param_table->U_TRD_1 = 50;
    param_table->U_EV_Width_1 = 2;
    param_table->U_TPL_2 = 10;
    param_table->U_TRD_2 = 30;
    param_table->U_EV_Width_2 = 2;

    param_table->U_Sq = 100;
    param_table->U_Iq = 0;
    param_table->U_CVq = 100;
    param_table->U_Aq = 5;
    param_table->U_Bq = 5;
    
    param_table->U_S2 = 100;
    param_table->U_I2 = 0;
    param_table->U_Sr = 1.0f;
    param_table->U_Ir = 0.0f;

    /* 7. ��đ�̴����Å��� */
    param_table->C_CSU_TOL = 10;
    param_table->C_NDL = 100;
    param_table->C_UDL = 90;
    param_table->C_BLOOD_IN = 200;
    strcpy((char *)param_table->C_Strip_Lot, "CHOL20240918");
    
    param_table->C_L = 100;
    param_table->C_H = 400;
    param_table->C_T3_E37 = 2000;

    param_table->C_TPL_1 = 10;
    param_table->C_TRD_1 = 50;
    param_table->C_EV_Width_1 = 2;
    param_table->C_TPL_2 = 10;
    param_table->C_TRD_2 = 30;
    param_table->C_EV_Width_2 = 2;

    param_table->C_Sq = 100;
    param_table->C_Iq = 0;
    param_table->C_CVq = 100;
    param_table->C_Aq = 5;
    param_table->C_Bq = 5;
    
    param_table->C_S2 = 100;
    param_table->C_I2 = 0;
    param_table->C_Sr = 1.0f;
    param_table->C_Ir = 0.0f;

    /* 8. �������֬���Å��� */
    param_table->TG_CSU_TOL = 10;
    param_table->TG_NDL = 100;
    param_table->TG_UDL = 90;
    param_table->TG_BLOOD_IN = 200;
    strcpy((char *)param_table->TG_Strip_Lot, "TG20240918");
    
    param_table->TG_L = 50;
    param_table->TG_H = 500;
    param_table->TG_T3_E37 = 2000;

    param_table->TG_TPL_1 = 10;
    param_table->TG_TRD_1 = 50;
    param_table->TG_EV_Width_1 = 2;
    param_table->TG_TPL_2 = 10;
    param_table->TG_TRD_2 = 30;
    param_table->TG_EV_Width_2 = 2;

    param_table->TG_Sq = 100;
    param_table->TG_Iq = 0;
    param_table->TG_CVq = 100;
    param_table->TG_Aq = 5;
    param_table->TG_Bq = 5;
    
    param_table->TG_S2 = 100;
    param_table->TG_I2 = 0;
    param_table->TG_Sr = 1.0f;
    param_table->TG_Ir = 0.0f;

    /* Ӌ��У���cCRC */
    uint16_t checksum = ParamTable_CalculateChecksum(param_table);
    param_table->SUM_L = (uint8_t)(checksum & 0xFF);
    param_table->SUM_H = (uint8_t)((checksum >> 8) & 0xFF);
    param_table->CRC16 = ParamTable_CalculateCRC16(param_table);
}

/*********************************************************************
 * @fn      ParamTable_CalculateChecksum
 *
 * @brief   Ӌ�ㅢ����У��(��ַ0~674)
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  У��ֵ
 */
uint16_t ParamTable_CalculateChecksum(ParamTable_TypeDef *param_table)
{
    uint16_t checksum = 0;
    uint8_t *ptr = (uint8_t *)param_table;
    
    /* Ӌ���ַ0~674��У�� */
    for(uint16_t i = 0; i < PARAM_CHECKSUM_OFFSET; i++)
    {
        checksum += ptr[i];
    }
    
    return checksum;
}

/*********************************************************************
 * @fn      ParamTable_CalculateCRC16
 *
 * @brief   Ӌ�ㅢ����CRC16(������)
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  CRC16У�ֵ
 */
uint8_t ParamTable_CalculateCRC16(ParamTable_TypeDef *param_table)
{
    /* ���Ό��F�����H�ϑ�ʹ�Ø˜�CRC16�㷨 */
    uint8_t crc = 0;
    uint8_t *ptr = (uint8_t *)param_table;
    
    /* Ӌ���������XOR����CRC (������) */
    for(uint16_t i = 0; i < PARAM_TABLE_SIZE - 1; i++)
    {
        crc ^= ptr[i];
    }
    
    return crc;
}

/*********************************************************************
 * @fn      ParamTable_Verify
 *
 * @brief   ��C�������У�ͺ�CRC�Ƿ����_
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  0: У�ɹ�, 1: У�ʧ��
 */
uint8_t ParamTable_Verify(ParamTable_TypeDef *param_table)
{
    uint16_t calculated_checksum = ParamTable_CalculateChecksum(param_table);
    uint16_t stored_checksum = ((uint16_t)param_table->SUM_H << 8) | param_table->SUM_L;
    
    /* �z��У�� */
    if(calculated_checksum != stored_checksum)
    {
        return 1; // У�ʧ��
    }
    
    /* �z��CRC16 */
    uint8_t calculated_crc = ParamTable_CalculateCRC16(param_table);
    if(calculated_crc != param_table->CRC16)
    {
        return 1; // У�ʧ��
    }
    
    return 0; // У�ɹ�
}

/*********************************************************************
 * @fn      ParamTable_SaveToFlash
 *
 * @brief   ���������浽Flash�惦��
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  none
 */
void ParamTable_SaveToFlash(ParamTable_TypeDef *param_table)
{
    /* ����Ӌ��У���cCRC */
    uint16_t checksum = ParamTable_CalculateChecksum(param_table);
    param_table->SUM_L = (uint8_t)(checksum & 0xFF);
    param_table->SUM_H = (uint8_t)((checksum >> 8) & 0xFF);
    param_table->CRC16 = ParamTable_CalculateCRC16(param_table);
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ����Ŀ��Flash�ȅ^ */
    FLASH_ErasePage(PARAM_TABLE_FLASH_ADDR);
    
    /* �������팑��Flash */
    uint32_t *source = (uint32_t *)param_table;
    uint32_t address = PARAM_TABLE_FLASH_ADDR;
    
    for(uint16_t i = 0; i < (sizeof(ParamTable_TypeDef) + 3) / 4; i++)
    {
        FLASH_ProgramWord(address, source[i]);
        address += 4;
    }
    
    /* �i��Flash */
    FLASH_Lock();
}

/*********************************************************************
 * @fn      ParamTable_LoadFromFlash
 *
 * @brief   ��Flash�惦�����d������
 *
 * @param   param_table - ������Y���wָ�
 *
 * @return  0: ���d�ɹ�, 1: У�ʧ�����ѳ�ʼ����Ĭ�Jֵ
 */
uint8_t ParamTable_LoadFromFlash(ParamTable_TypeDef *param_table)
{
    /* ��Flash�xȡ������ */
    memcpy(param_table, (void *)PARAM_TABLE_FLASH_ADDR, sizeof(ParamTable_TypeDef));
    
    /* ��C������ */
    if(ParamTable_Verify(param_table) != 0)
    {
        /* У�ʧ������ʼ����Ĭ�Jֵ */
        ParamTable_Init(param_table);
        return 1;
    }
    
    return 0;
} 