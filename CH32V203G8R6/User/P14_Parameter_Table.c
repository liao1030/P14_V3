/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Parameter_Table.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : �๦�������yԇ�x�������a�팍�F
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Parameter_Table.h"
#include "debug.h"
#include <string.h>

/* ������ȫ��׃�� */
static uint8_t g_paramTable[PARAM_TABLE_SIZE];

/* Ĭ�J����ֵ */
static const uint8_t g_defaultParamTable[PARAM_TABLE_SIZE] = {
    /* 1. ϵ�y�������� */
    58,     // PARAM_LBT = 2.58V (��늳��ֵ)
    55,     // PARAM_OBT = 2.55V (늳غıM�ֵ)
    0,      // PARAM_FACTORY = 0 (ʹ����ģʽ)
    1,      // PARAM_MODEL_NO = 1 (�aƷ��̖)
    1,      // PARAM_FW_NO = 1 (�g�w�汾̖)
    0, 0,   // PARAM_NOT = 0 (�y���Δ���2 bytes)
    1, 0,   // PARAM_CODE_TABLE_V = 1 (��̖��汾��̖��2 bytes)
    
    /* 2. �r�g�O������ */
    24,     // PARAM_YEAR = 24 (2024��)
    8,      // PARAM_MONTH = 8 (8��)
    12,     // PARAM_DATE = 12 (12��)
    0,      // PARAM_HOUR = 0 (0�r)
    0,      // PARAM_MINUTE = 0 (0��)
    0,      // PARAM_SECOND = 0 (0��)
    
    /* 3. �y���h������ */
    1,      // PARAM_TLL = 1.0�� (�����ضȹ�������)
    40,     // PARAM_TLH = 40.0�� (�����ضȹ�������)
    0,      // PARAM_MGDL = 0 (mmol/L)
    0,      // PARAM_EVENT = 0 (QC)
    0,      // PARAM_STRIP_TYPE = 0 (Ѫ��)
};

/*********************************************************************
 * @fn      P14_ParamTable_Init
 *
 * @brief   ��ʼ��������
 *
 * @return  none
 */
void P14_ParamTable_Init(void)
{
    printf("��ʼ���������a��...\r\n");
    
    /* ��ʼ���������0 */
    memset(g_paramTable, 0, PARAM_TABLE_SIZE);
    
    /* �Lԇ�Ĵ惦���d�녢���� */
    P14_ParamTable_Load();
    
    /* �z�锵����Ч�ԣ�����oЧ�t�d��Ĭ�Jֵ */
    if (!P14_ParamTable_VerifyChecksum()) {
        printf("������У�ʧ�����d��Ĭ�Jֵ\r\n");
        P14_ParamTable_LoadDefault();
    } else {
        printf("������У�ɹ�\r\n");
    }
    
    /* �@ʾ����������Ϣ */
    printf("�aƷ��̖: %d\r\n", P14_ParamTable_Read(PARAM_MODEL_NO));
    printf("�g�w�汾: %d\r\n", P14_ParamTable_Read(PARAM_FW_NO));
    printf("���a��汾: %d\r\n", P14_ParamTable_Read16(PARAM_CODE_TABLE_V));
}

/*********************************************************************
 * @fn      P14_ParamTable_LoadDefault
 *
 * @brief   �d��Ĭ�J����ֵ
 *
 * @return  none
 */
void P14_ParamTable_LoadDefault(void)
{
    /* ��ؐĬ�J������������ */
    memcpy(g_paramTable, g_defaultParamTable, sizeof(g_defaultParamTable));
    
    /* ����У�� */
    P14_ParamTable_UpdateChecksum();
    
    /* ���浽�惦�� */
    P14_ParamTable_Save();
}

/*********************************************************************
 * @fn      P14_ParamTable_Read
 *
 * @brief   �xȡ���ֹ�����
 *
 * @param   address - ������ַ
 *
 * @return  ����ֵ
 */
uint8_t P14_ParamTable_Read(uint16_t address)
{
    if (address < PARAM_TABLE_SIZE) {
        return g_paramTable[address];
    }
    return 0;
}

/*********************************************************************
 * @fn      P14_ParamTable_Write
 *
 * @brief   ������ֹ�����
 *
 * @param   address - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void P14_ParamTable_Write(uint16_t address, uint8_t value)
{
    if (address < PARAM_TABLE_SIZE) {
        g_paramTable[address] = value;
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_Read16
 *
 * @brief   �xȡ�p�ֹ�����
 *
 * @param   address - ������ַ
 *
 * @return  ����ֵ
 */
uint16_t P14_ParamTable_Read16(uint16_t address)
{
    uint16_t value = 0;
    
    if (address < PARAM_TABLE_SIZE - 1) {
        value = g_paramTable[address];
        value |= ((uint16_t)g_paramTable[address + 1] << 8);
    }
    
    return value;
}

/*********************************************************************
 * @fn      P14_ParamTable_Write16
 *
 * @brief   �����p�ֹ�����
 *
 * @param   address - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void P14_ParamTable_Write16(uint16_t address, uint16_t value)
{
    if (address < PARAM_TABLE_SIZE - 1) {
        g_paramTable[address] = (uint8_t)(value & 0xFF);
        g_paramTable[address + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_ReadFloat
 *
 * @brief   �xȡ���c������ (4�ֹ�)
 *
 * @param   address - ������ַ
 *
 * @return  ����ֵ
 */
float P14_ParamTable_ReadFloat(uint16_t address)
{
    float value = 0.0f;
    
    if (address < PARAM_TABLE_SIZE - 3) {
        memcpy(&value, &g_paramTable[address], sizeof(float));
    }
    
    return value;
}

/*********************************************************************
 * @fn      P14_ParamTable_WriteFloat
 *
 * @brief   ���븡�c������ (4�ֹ�)
 *
 * @param   address - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void P14_ParamTable_WriteFloat(uint16_t address, float value)
{
    if (address < PARAM_TABLE_SIZE - 3) {
        memcpy(&g_paramTable[address], &value, sizeof(float));
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_ReadBlock
 *
 * @brief   �xȡ�����K
 *
 * @param   address - ��ʼ��ַ
 * @param   buffer - Ŀ�˾��n�^
 * @param   length - �����L��
 *
 * @return  none
 */
void P14_ParamTable_ReadBlock(uint16_t address, uint8_t *buffer, uint16_t length)
{
    if (address + length <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(buffer, &g_paramTable[address], length);
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_WriteBlock
 *
 * @brief   ���녢���K
 *
 * @param   address - ��ʼ��ַ
 * @param   buffer - Դ�������n�^
 * @param   length - �����L��
 *
 * @return  none
 */
void P14_ParamTable_WriteBlock(uint16_t address, const uint8_t *buffer, uint16_t length)
{
    if (address + length <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(&g_paramTable[address], buffer, length);
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_VerifyChecksum
 *
 * @brief   ��C������У��
 *
 * @return  0: У�ʧ��, 1: У�ɹ�
 */
uint8_t P14_ParamTable_VerifyChecksum(void)
{
    uint16_t calcSum = 0;
    uint16_t storedSum;
    
    /* Ӌ��0-674��ַ�Ŀ��� */
    for (uint16_t i = 0; i < PARAM_SUM_L; i++) {
        calcSum += g_paramTable[i];
    }
    
    /* �@ȡ�惦��У�� */
    storedSum = g_paramTable[PARAM_SUM_L] | ((uint16_t)g_paramTable[PARAM_SUM_H] << 8);
    
    /* ���^Ӌ���У���c�惦��У�� */
    return (calcSum == storedSum) ? 1 : 0;
}

/*********************************************************************
 * @fn      P14_ParamTable_UpdateChecksum
 *
 * @brief   ��������У��
 *
 * @return  none
 */
void P14_ParamTable_UpdateChecksum(void)
{
    uint16_t sum = 0;
    
    /* Ӌ��0-674��ַ�Ŀ��� */
    for (uint16_t i = 0; i < PARAM_SUM_L; i++) {
        sum += g_paramTable[i];
    }
    
    /* �惦У�� */
    g_paramTable[PARAM_SUM_L] = (uint8_t)(sum & 0xFF);
    g_paramTable[PARAM_SUM_H] = (uint8_t)((sum >> 8) & 0xFF);
    
    /* Ӌ��CRC16У�ֵ (�������F�����H�Ŀ������Ҫ�˜�CRC16�㷨) */
    uint8_t crc = 0;
    for (uint16_t i = 0; i < PARAM_CRC16; i++) {
        crc ^= g_paramTable[i];
    }
    g_paramTable[PARAM_CRC16] = crc;
}

/*********************************************************************
 * @fn      P14_ParamTable_Save
 *
 * @brief   ���慢�����惦��
 *
 * @return  none
 */
void P14_ParamTable_Save(void)
{
    /* ��̎�����F���������浽�Ǔ]�l�Դ惦����߉݋ */
    /* ����Ӳ�w���ã�������EEPROM��Flash�� */
    printf("��������\r\n");
    
    /* ʾ��: ���ʹ�ÃȲ�Flash����Ҫ���Ȳ��������^��Ȼ�጑�딵�� */
    /* ���H���F��Ҫ����Ӳ�w�����޸� */
}

/*********************************************************************
 * @fn      P14_ParamTable_Load
 *
 * @brief   �Ĵ惦���d�녢����
 *
 * @return  none
 */
void P14_ParamTable_Load(void)
{
    /* ��̎�����F�ķǓ]�l�Դ惦���xȡ�������߉݋ */
    /* ����Ӳ�w���ã�������EEPROM��Flash�� */
    printf("�������d��\r\n");
    
    /* ʾ��: ���ʹ�ÃȲ�Flash����Ҫ��ָ����ַ�xȡ���� */
    /* ���H���F��Ҫ����Ӳ�w�����޸� */
}

/*********************************************************************
 * @fn      P14_ParamTable_GetStripTypeName
 *
 * @brief   �@ȡԇƬ������Q
 *
 * @param   type - ԇƬ���
 *
 * @return  ������Q�ַ���
 */
const char* P14_ParamTable_GetStripTypeName(StripType_TypeDef type)
{
    switch (type) {
        case STRIP_TYPE_GLV: return "Ѫ��";
        case STRIP_TYPE_U:   return "����";
        case STRIP_TYPE_C:   return "��đ�̴�";
        case STRIP_TYPE_TG:  return "�������֬";
        case STRIP_TYPE_GAV: return "Ѫ��(GAV)";
        default:             return "δ֪";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_GetEventName
 *
 * @brief   �@ȡ�¼�������Q
 *
 * @param   event - �¼����
 *
 * @return  �¼����Q�ַ���
 */
const char* P14_ParamTable_GetEventName(Event_TypeDef event)
{
    switch (event) {
        case EVENT_QC: return "Ʒ��Һ�yԇ";
        case EVENT_AC: return "��ǰ�yԇ";
        case EVENT_PC: return "����yԇ";
        default:       return "δ֪";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_GetUnitName
 *
 * @brief   �@ȡ�y����λ���Q
 *
 * @param   unit - �y����λ
 *
 * @return  ��λ���Q�ַ���
 */
const char* P14_ParamTable_GetUnitName(Unit_TypeDef unit)
{
    switch (unit) {
        case UNIT_MMOL_L: return "mmol/L";
        case UNIT_MG_DL:  return "mg/dL";
        case UNIT_GM_DL:  return "gm/dl";
        default:          return "δ֪";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_IncrementTestCount
 *
 * @brief   ���Ӝyԇ�Δ�Ӌ����
 *
 * @return  none
 */
void P14_ParamTable_IncrementTestCount(void)
{
    uint16_t count = P14_ParamTable_Read16(PARAM_NOT);
    count++;
    P14_ParamTable_Write16(PARAM_NOT, count);
    
    /* �Ԅӱ��������ą����� */
    P14_ParamTable_UpdateChecksum();
    P14_ParamTable_Save();
}

/*********************************************************************
 * @fn      P14_ParamTable_ResetTestCount
 *
 * @brief   ���Üyԇ�Δ�Ӌ����
 *
 * @return  none
 */
void P14_ParamTable_ResetTestCount(void)
{
    P14_ParamTable_Write16(PARAM_NOT, 0);
    
    /* �Ԅӱ��������ą����� */
    P14_ParamTable_UpdateChecksum();
    P14_ParamTable_Save();
} 