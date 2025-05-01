/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_table.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : �๦�������yԇ�x�������a�팍�F
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_table.h"
#include "debug.h"
#include <string.h>

/* �����팍�� */
static ParameterTable_TypeDef paramTable;

/* Ĭ�J����ֵ */
static const uint8_t defaultParamTable[PARAM_TABLE_SIZE] = {
    /* ϵ�y�������� */
    58,     // PARAM_LBT: ��늳��ֵ (2.58V)
    55,     // PARAM_OBT: 늳غıM�ֵ (2.55V)
    0,      // PARAM_FACTORY: ʹ����ģʽ
    1,      // PARAM_MODEL_NO: �aƷ��̖ (�ɏS���޸�)
    1,      // PARAM_FW_NO: �g�w�汾̖ (�ɏS���޸�)
    0, 0,   // PARAM_NOT: �yԇ�Δ���ʼ��0
    1, 0,   // PARAM_CODE_TABLE_V: ���a��汾̖ (1.0)
    
    /* �r�g�O������ */
    24,     // PARAM_YEAR: ����O�� (2024��)
    7,      // PARAM_MONTH: �·��O�� (7��)
    23,     // PARAM_DATE: �����O�� (23��)
    12,     // PARAM_HOUR: С�r�O�� (12�r)
    0,      // PARAM_MINUTE: ����O�� (0��)
    0,      // PARAM_SECOND: �딵�O�� (0��)
    
    /* �yԇ�h������ */
    1,      // PARAM_TLL: �����ضȹ������� (1.0��C)
    40,     // PARAM_TLH: �����ضȹ������� (40.0��C)
    0,      // PARAM_MGDL: ��Ȇ�λ�O�� (mmol/L)
    0,      // PARAM_EVENT: ����¼���� (QC)
    0,      // PARAM_STRIP_TYPE: �yԇ�Ŀ (Ѫ��)
    
    /* Ӳ�wУ������ */
    40, 1,  // PARAM_EV_T3_TRG: EV_T3�|�l늉� (296 = 1.2V)
    164,    // PARAM_EV_WORKING: ��ȹ������늉�
    164,    // PARAM_EV_T3: ѪҺ�������늉�
    0,      // PARAM_DACO: DACƫ���a��
    0,      // PARAM_DACDO: DACУ�����텢��
    0,      // PARAM_CC211NoDone: CC211δ������
    8,      // PARAM_CAL_TOL: OPS/OPIУ���ݲ� (0.8%)
    /* PARAM_OPS: OPAУ��б�� (4 bytes) */
    0, 0, 0, 0,
    /* PARAM_OPI: OPAУ���ؾ� (4 bytes) */
    0, 0, 0, 0,
    0,      // PARAM_QCT: QCTУ���yԇ��λԪ�M
    0,      // PARAM_TOFFSET: �ض�У��ƫ��
    0,      // PARAM_BOFFSET: 늳�У��ƫ��
};

/* �o�B������ */
static uint16_t PARAM_CalculateChecksum(void);
static uint16_t PARAM_Calculate_CRC16(const uint8_t *data, uint16_t size);

/**
 * @brief ��ʼ��������
 */
void PARAM_Init(void)
{
    /* ��ʼ����Ĭ�Jֵ */
    memcpy(paramTable.data, defaultParamTable, PARAM_TABLE_SIZE);
    
    /* ��Flash��EEPROM�d�녢�����d��ʧ���tʹ��Ĭ�J���� */
    if (!PARAM_Load()) {
        PARAM_Reset();
        PARAM_Save();
    }
    
    printf("�������a���ʼ����ɣ��汾: %d.%d\r\n", 
           paramTable.data[PARAM_CODE_TABLE_V], 
           paramTable.data[PARAM_CODE_TABLE_V + 1]);
}

/**
 * @brief �xȡ8λԪ����
 * @param addr: ������ַ
 * @return ����ֵ
 */
uint8_t PARAM_Read(uint16_t addr)
{
    if (addr < PARAM_TABLE_SIZE) {
        return paramTable.data[addr];
    }
    return 0;
}

/**
 * @brief ����8λԪ����
 * @param addr: ������ַ
 * @param value: ����ֵ
 */
void PARAM_Write(uint16_t addr, uint8_t value)
{
    if (addr < PARAM_TABLE_SIZE) {
        paramTable.data[addr] = value;
    }
}

/**
 * @brief �xȡ16λԪ���� (С�˸�ʽ)
 * @param addr: ������ַ
 * @return ����ֵ
 */
uint16_t PARAM_Read16(uint16_t addr)
{
    if (addr < PARAM_TABLE_SIZE - 1) {
        return (uint16_t)(paramTable.data[addr]) | 
               ((uint16_t)(paramTable.data[addr + 1]) << 8);
    }
    return 0;
}

/**
 * @brief ����16λԪ���� (С�˸�ʽ)
 * @param addr: ������ַ
 * @param value: ����ֵ
 */
void PARAM_Write16(uint16_t addr, uint16_t value)
{
    if (addr < PARAM_TABLE_SIZE - 1) {
        paramTable.data[addr] = (uint8_t)(value & 0xFF);
        paramTable.data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/**
 * @brief �xȡ�����K
 * @param addr: ��ʼ��ַ
 * @param buffer: ���n�^
 * @param size: ��С
 */
void PARAM_ReadBlock(uint16_t addr, uint8_t *buffer, uint16_t size)
{
    if (addr + size <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(buffer, &paramTable.data[addr], size);
    }
}

/**
 * @brief ���녢���K
 * @param addr: ��ʼ��ַ
 * @param buffer: ���n�^
 * @param size: ��С
 */
void PARAM_WriteBlock(uint16_t addr, const uint8_t *buffer, uint16_t size)
{
    if (addr + size <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(&paramTable.data[addr], buffer, size);
    }
}

/**
 * @brief ���Å������Ĭ�Jֵ
 */
void PARAM_Reset(void)
{
    /* �}�uĬ�J����ֵ */
    memcpy(paramTable.data, defaultParamTable, PARAM_TABLE_SIZE);
    
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    printf("�����������Þ�Ĭ�Jֵ\r\n");
}

/**
 * @brief Ӌ�ㅢ����У��
 * @return У��
 */
static uint16_t PARAM_CalculateChecksum(void)
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < PARAM_SUM_L; i++) {
        sum += paramTable.data[i];
    }
    return sum;
}

/**
 * @brief ��������У��
 */
void PARAM_UpdateChecksum(void)
{
    uint16_t checksum = PARAM_CalculateChecksum();
    paramTable.data[PARAM_SUM_L] = checksum & 0xFF;
    paramTable.data[PARAM_SUM_H] = (checksum >> 8) & 0xFF;
    
    /* Ӌ��CRC16 */
    uint16_t crc = PARAM_Calculate_CRC16(paramTable.data, PARAM_CRC16);
    paramTable.data[PARAM_CRC16] = crc & 0xFF;
}

/**
 * @brief ��C������У��
 * @return 1:У�ɹ� 0:У�ʧ��
 */
uint8_t PARAM_VerifyChecksum(void)
{
    uint16_t calculatedChecksum = PARAM_CalculateChecksum();
    uint16_t storedChecksum = (uint16_t)paramTable.data[PARAM_SUM_L] | 
                             ((uint16_t)paramTable.data[PARAM_SUM_H] << 8);
    
    /* ��CУ�� */
    if (calculatedChecksum != storedChecksum) {
        printf("������У���e�`: Ӌ��ֵ=%04X, �惦ֵ=%04X\r\n", 
               calculatedChecksum, storedChecksum);
        return 0;
    }
    
    /* ��CCRC16 */
    uint16_t calculatedCRC = PARAM_Calculate_CRC16(paramTable.data, PARAM_CRC16);
    uint8_t storedCRC = paramTable.data[PARAM_CRC16];
    
    if ((calculatedCRC & 0xFF) != storedCRC) {
        printf("������CRCУ��e�`: Ӌ��ֵ=%02X, �惦ֵ=%02X\r\n", 
               calculatedCRC & 0xFF, storedCRC);
        return 0;
    }
    
    return 1;
}

/**
 * @brief Ӌ��CRC16У�a
 * @param data: ����ָ�
 * @param size: ������С
 * @return CRC16У�a
 */
static uint16_t PARAM_Calculate_CRC16(const uint8_t *data, uint16_t size)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < size; i++) {
        crc ^= (uint16_t)data[i];
        
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief ���慢����Flash��EEPROM
 * @note �˺�����������HӲ�w���F
 */
void PARAM_Save(void)
{
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    /* ���浽EEPROM��Flash���������HӲ�w���F */
    printf("�������ѱ���\r\n");
}

/**
 * @brief ��Flash��EEPROM�d�녢����
 * @return 1:�d��ɹ� 0:�d��ʧ��
 * @note �˺�����������HӲ�w���F
 */
uint8_t PARAM_Load(void)
{
    /* ��EEPROM��Flash���d���������HӲ�w���F */
    /* ����ʾ�����@�eֱ��ʹ��Ĭ�Jֵ */
    
    /* ��C��������Ч�� */
    if (!PARAM_VerifyChecksum()) {
        printf("�������d��ʧ����ʹ��Ĭ�Jֵ\r\n");
        return 0;
    }
    
    printf("�������d��ɹ�\r\n");
    return 1;
} 