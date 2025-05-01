/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_test.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : �๦�������yԇ�x�������a��yԇ����
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_table.h"
#include "debug.h"
#include <string.h>

/**
 * @brief �@ȡϵ�y�r�gӋ����ֵ (����)
 * @return ϵ�y�r�gӋ��
 */
static uint32_t GET_TICK_COUNT(void)
{
    // ʹ���o�B׃����ӛ䛽��^�ĺ��딵
    static uint32_t tick_count = 0;
    
    // ÿ���{������1���룬���H�����Б�ʹ���m�ϵ�Ӌ�r��
    tick_count++;
    
    return tick_count;
}

/**
 * @brief �@ʾ���N�yԇ�Ŀ����Ҫ����
 */
void PARAM_Test_PrintMainParameters(void)
{
    printf("\r\n===== ���yԇ�Ŀ��Ҫ���� =====\r\n");
    
    /* �@ʾѪ�ǅ��� */
    printf("\r\n[Ѫ�ǅ���]\r\n");
    printf("ԇƬ�z���ݲ�: %d %%\r\n", PARAM_Read(PARAM_BG_CSU_TOL) / 10);
    printf("Ѫ��ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_BG_L) * 10);
    printf("Ѫ��ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_BG_H) * 10);
    
    char stripLot[17] = {0};
    PARAM_ReadBlock(PARAM_BG_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("ԇƬ��̖: %s\r\n", stripLot);
    
    /* �@ʾ���ᅢ�� */
    printf("\r\n[���ᅢ��]\r\n");
    printf("ԇƬ�z���ݲ�: %d %%\r\n", PARAM_Read(PARAM_U_CSU_TOL) / 10);
    printf("����ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_U_L));
    printf("����ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_U_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_U_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("ԇƬ��̖: %s\r\n", stripLot);
    
    /* �@ʾ��đ�̴����� */
    printf("\r\n[��đ�̴�����]\r\n");
    printf("ԇƬ�z���ݲ�: %d %%\r\n", PARAM_Read(PARAM_C_CSU_TOL) / 10);
    printf("đ�̴�ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_C_L));
    printf("đ�̴�ֵ����: %d mg/dL\r\n", PARAM_Read(PARAM_C_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_C_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("ԇƬ��̖: %s\r\n", stripLot);
    
    /* �@ʾ�������֬���� */
    printf("\r\n[�������֬����]\r\n");
    printf("ԇƬ�z���ݲ�: %d %%\r\n", PARAM_Read(PARAM_TG_CSU_TOL) / 10);
    printf("�������ֵ֬����: %d mg/dL\r\n", PARAM_Read(PARAM_TG_L));
    printf("�������ֵ֬����: %d mg/dL\r\n", PARAM_Read(PARAM_TG_H));
    
    memset(stripLot, 0, sizeof(stripLot));
    PARAM_ReadBlock(PARAM_TG_STRIP_LOT, (uint8_t*)stripLot, 16);
    printf("ԇƬ��̖: %s\r\n", stripLot);
}

/**
 * @brief �O��ԇƬ��̖
 * @param stripType: ԇƬ���
 * @param lotNumber: ��̖�ִ�
 */
void PARAM_Test_SetStripLot(uint8_t stripType, const char* lotNumber)
{
    uint16_t lotAddr = 0;
    
    switch(stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            lotAddr = PARAM_BG_STRIP_LOT;
            break;
        case STRIP_TYPE_U:
            lotAddr = PARAM_U_STRIP_LOT;
            break;
        case STRIP_TYPE_C:
            lotAddr = PARAM_C_STRIP_LOT;
            break;
        case STRIP_TYPE_TG:
            lotAddr = PARAM_TG_STRIP_LOT;
            break;
        default:
            printf("δ֪ԇƬ���!\r\n");
            return;
    }
    
    // �����̖
    char paddedLot[16] = {0};
    strncpy(paddedLot, lotNumber, 15);
    
    // ������̖
    PARAM_WriteBlock(lotAddr, (const uint8_t*)paddedLot, 16);
    
    // ����У��
    PARAM_UpdateChecksum();
    
    printf("���O����̖: %s\r\n", lotNumber);
}

/**
 * @brief ���yԇ�Δ�
 */
void PARAM_Test_UpdateTestCount(void)
{
    uint16_t count = PARAM_Read16(PARAM_NOT);
    count++;
    PARAM_Write16(PARAM_NOT, count);
    PARAM_UpdateChecksum();
    printf("�Ѹ��yԇ�Δ�: %d\r\n", count);
}

/**
 * @brief �O�ò����ضȹ���
 * @param lower: ���ޜض� (��C)
 * @param upper: ���ޜض� (��C)
 */
void PARAM_Test_SetTemperatureRange(uint8_t lower, uint8_t upper)
{
    if (lower >= upper) {
        printf("�ضȹ����O���e�`!\r\n");
        return;
    }
    
    PARAM_Write(PARAM_TLL, lower);
    PARAM_Write(PARAM_TLH, upper);
    PARAM_UpdateChecksum();
    
    printf("���O�ò����ضȹ���: %d��C ~ %d��C\r\n", lower, upper);
}

/**
 * @brief �O�Ý�Ȇ�λ
 * @param unit: 0:mmol/L, 1:mg/dL, 2:gm/dl
 */
void PARAM_Test_SetConcentrationUnit(uint8_t unit)
{
    if (unit > 2) {
        printf("��λ�O���e�`!\r\n");
        return;
    }
    
    PARAM_Write(PARAM_MGDL, unit);
    PARAM_UpdateChecksum();
    
    const char* unitStr[] = {"mmol/L", "mg/dL", "gm/dl"};
    printf("���O�Ý�Ȇ�λ: %s\r\n", unitStr[unit]);
}

/**
 * @brief ���Ѕ����������Ԝyԇ
 */
void PARAM_Test_RunIntegrityTest(void)
{
    printf("\r\n===== �����������Ԝyԇ =====\r\n");
    
    // У����_����C
    printf("��ʼУ�Y��: %s\r\n", PARAM_VerifyChecksum() ? "ͨ�^" : "ʧ��");
    
    // �޸�һ��ֵ�K��CУ��Ƿ�ʧ��
    uint8_t originalValue = PARAM_Read(PARAM_MODEL_NO);
    uint8_t newValue = originalValue + 1;
    
    printf("�޸ĮaƷ��̖: %d -> %d\r\n", originalValue, newValue);
    PARAM_Write(PARAM_MODEL_NO, newValue);
    
    printf("δ����У�͕r��C�Y��: %s (����ʧ��)\r\n", PARAM_VerifyChecksum() ? "ͨ�^" : "ʧ��");
    
    // ����У�́K�ٴ���C
    PARAM_UpdateChecksum();
    printf("����У������C�Y��: %s (����ͨ�^)\r\n", PARAM_VerifyChecksum() ? "ͨ�^" : "ʧ��");
    
    // �֏�ԭֵ
    PARAM_Write(PARAM_MODEL_NO, originalValue);
    PARAM_UpdateChecksum();
    
    printf("�֏�ԭֵ����C�Y��: %s\r\n", PARAM_VerifyChecksum() ? "ͨ�^" : "ʧ��");
}

/**
 * @brief ���Ѕ��������ܜyԇ
 */
void PARAM_Test_RunPerformanceTest(void)
{
    printf("\r\n===== ���������ܜyԇ =====\r\n");
    
    uint32_t startTime, endTime, elapsedTime;
    uint16_t testCount = 1000;
    uint8_t dummySum = 0; // ʹ�ô�׃�����⃞��
    
    // �yԇ���ֹ��xȡ����
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < testCount; i++) {
        dummySum += PARAM_Read(PARAM_MODEL_NO);
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("���ֹ��xȡ x %d: %ld ms (%.2f ?s/��)\r\n", 
           testCount, elapsedTime, (float)elapsedTime * 1000 / testCount);
    
    // �yԇ���ֹ���������
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < testCount; i++) {
        PARAM_Write(PARAM_MODEL_NO, i & 0xFF);
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("���ֹ����� x %d: %ld ms (%.2f ?s/��)\r\n", 
           testCount, elapsedTime, (float)elapsedTime * 1000 / testCount);
    
    // ��ֹ���g��������
    if (dummySum == 0xFF) {
        printf("Dummy operation to prevent optimization\r\n");
    }
    
    // �yԇУ��Ӌ������
    startTime = GET_TICK_COUNT();
    for (uint16_t i = 0; i < 100; i++) {
        PARAM_UpdateChecksum();
    }
    endTime = GET_TICK_COUNT();
    elapsedTime = endTime - startTime;
    
    printf("У��Ӌ�� x 100: %ld ms (%.2f ms/��)\r\n", 
           elapsedTime, (float)elapsedTime / 100);
    
    // �֏�ԭʼֵ
    PARAM_Reset();
} 