/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_test.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : Flash�������淽���yԇ��ʽ
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_test.h"
#include "P14_Flash_Storage.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

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

/**
 * @brief ��ӡ�����^�K�YӍ
 */
static void PrintParameterInfo(void)
{
    BasicSystemBlock basicParams;
    HardwareCalibBlock calibParams;
    
    /* �xȡ����ϵ�y���� */
    if (PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("=== ����ϵ�y���� ===\r\n");
        printf("�aƷ��̖: %d\r\n", basicParams.modelNo);
        printf("�g�w�汾: %d.%d\r\n", basicParams.fwNo/10, basicParams.fwNo%10);
        printf("�yԇ�Δ�: %d\r\n", basicParams.testCount);
        printf("���ڕr�g: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
               basicParams.year, basicParams.month, basicParams.date,
               basicParams.hour, basicParams.minute, basicParams.second);
        printf("�ضȹ���: %d��C ~ %d��C\r\n", basicParams.tempLowLimit, basicParams.tempHighLimit);
        printf("�yԇ�Ŀ: %d\r\n", basicParams.stripType);
        printf("��Ȇ�λ: %s\r\n", basicParams.measureUnit ? "mg/dL" : "mmol/L");
    } else {
        printf("�xȡ����ϵ�y����ʧ��\r\n");
    }
    
    /* �xȡӲ�wУ������ */
    if (PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("\r\n=== Ӳ�wУ������ ===\r\n");
        printf("EV_T3�|�l늉�: %d mV\r\n", calibParams.evT3Trigger);
        printf("�ض��a��: %d (0.1��C)\r\n", calibParams.tempOffset);
        printf("늳��a��: %d (10mV)\r\n", calibParams.batteryOffset);
        printf("OPAб��: %.4f\r\n", calibParams.ops);
        printf("OPA�ؾ�: %.4f\r\n", calibParams.opi);
    } else {
        printf("�xȡӲ�wУ������ʧ��\r\n");
    }
    
    printf("\r\n");
}

/**
 * @brief �yԇϵ�y��������
 */
static void TestSystemParamUpdate(void)
{
    BasicSystemBlock basicParams;
    
    /* �xȡ��ǰ���� */
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("�xȡϵ�y����ʧ��\r\n");
        return;
    }
    
    /* �@ʾԭʼ���� */
    printf("ԭʼ�yԇ�Δ�: %d\r\n", basicParams.testCount);
    
    /* ���yԇ�Δ� */
    if (PARAM_IncreaseTestCount()) {
        printf("�yԇ�Δ�+1�ɹ�\r\n");
    } else {
        printf("�yԇ�Δ�+1ʧ��\r\n");
        return;
    }
    
    /* �����xȡ���� */
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("�xȡϵ�y����ʧ��\r\n");
        return;
    }
    
    /* �@ʾ�����ᅢ�� */
    printf("������yԇ�Δ�: %d\r\n", basicParams.testCount);
    printf("\r\n");
}

/**
 * @brief �yԇ���ڕr�g����
 */
static void TestDateTimeUpdate(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* �xȡ��ǰ�r�g */
    if (!PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second)) {
        printf("�xȡ���ڕr�gʧ��\r\n");
        return;
    }
    
    /* �@ʾԭʼ�r�g */
    printf("ԭʼ�r�g: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
           year, month, date, hour, minute, second);
    
    /* ���r�g */
    if (PARAM_UpdateDateTime(24, 7, 25, 15, 30, 0)) {
        printf("�������ڕr�g�ɹ�\r\n");
    } else {
        printf("�������ڕr�gʧ��\r\n");
        return;
    }
    
    /* �����xȡ�r�g */
    if (!PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second)) {
        printf("�xȡ���ڕr�gʧ��\r\n");
        return;
    }
    
    /* �@ʾ������r�g */
    printf("������r�g: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
           year, month, date, hour, minute, second);
    printf("\r\n");
}

/**
 * @brief �yԇ����ץȡ
 */
static void TestParametersFetch(void)
{
    uint16_t ndl, udl, bloodIn;
    uint8_t lowLimit;
    uint16_t highLimit;
    uint16_t tpl1, trd1, evWidth1;
    uint16_t tpl2, trd2, evWidth2;
    
    /* �yԇѪ�ǅ��� */
    printf("=== Ѫ�ǅ����yԇ ===\r\n");
    
    /* �xȡԇƬ���� */
    if (PARAM_GetStripParametersByStripType(STRIP_TYPE_GLV, &ndl, &udl, &bloodIn)) {
        printf("Ѫ��ԇƬ����: NDL=%d, UDL=%d, BloodIn=%d\r\n", ndl, udl, bloodIn);
    } else {
        printf("�xȡѪ��ԇƬ����ʧ��\r\n");
    }
    
    /* �xȡ�y������ */
    if (PARAM_GetMeasurementRangeByStripType(STRIP_TYPE_GLV, &lowLimit, &highLimit)) {
        printf("Ѫ�ǜy������: %d ~ %d mg/dL\r\n", lowLimit, highLimit);
    } else {
        printf("�xȡѪ�ǜy������ʧ��\r\n");
    }
    
    /* �xȡ�r�򅢔�(�M1) */
    if (PARAM_GetTimingParametersByStripType(STRIP_TYPE_GLV, &tpl1, &trd1, &evWidth1, 1)) {
        printf("Ѫ�Ǖr�򅢔�(�M1): TPL=%d, TRD=%d, EVWidth=%d\r\n", tpl1, trd1, evWidth1);
    } else {
        printf("�xȡѪ�Ǖr�򅢔�(�M1)ʧ��\r\n");
    }
    
    /* �xȡ�r�򅢔�(�M2) */
    if (PARAM_GetTimingParametersByStripType(STRIP_TYPE_GLV, &tpl2, &trd2, &evWidth2, 2)) {
        printf("Ѫ�Ǖr�򅢔�(�M2): TPL=%d, TRD=%d, EVWidth=%d\r\n", tpl2, trd2, evWidth2);
    } else {
        printf("�xȡѪ�Ǖr�򅢔�(�M2)ʧ��\r\n");
    }
    
    /* �yԇ���ᅢ�� */
    printf("\r\n=== ���ᅢ���yԇ ===\r\n");
    
    /* �xȡԇƬ���� */
    if (PARAM_GetStripParametersByStripType(STRIP_TYPE_U, &ndl, &udl, &bloodIn)) {
        printf("����ԇƬ����: NDL=%d, UDL=%d, BloodIn=%d\r\n", ndl, udl, bloodIn);
    } else {
        printf("�xȡ����ԇƬ����ʧ��\r\n");
    }
    
    /* �xȡ�y������ */
    if (PARAM_GetMeasurementRangeByStripType(STRIP_TYPE_U, &lowLimit, &highLimit)) {
        printf("����y������: %d ~ %d mg/dL\r\n", lowLimit, highLimit);
    } else {
        printf("�xȡ����y������ʧ��\r\n");
    }
    
    printf("\r\n");
}

/**
 * @brief �yԇӲ�wУ����������
 */
static void TestHardwareCalibUpdate(void)
{
    HardwareCalibBlock calibParams;
    
    /* �xȡ��ǰ���� */
    if (!PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("�xȡӲ�wУ������ʧ��\r\n");
        return;
    }
    
    /* �@ʾԭʼ���� */
    printf("ԭʼ�ض��a��: %d (0.1��C)\r\n", calibParams.tempOffset);
    printf("ԭʼ늳��a��: %d (10mV)\r\n", calibParams.batteryOffset);
    
    /* ����У������ */
    calibParams.tempOffset = 5;     // ����0.5��C
    calibParams.batteryOffset = -10; // �p��100mV
    
    if (PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("����Ӳ�wУ�������ɹ�\r\n");
    } else {
        printf("����Ӳ�wУ������ʧ��\r\n");
        return;
    }
    
    /* �����xȡ���� */
    if (!PARAM_ReadParameterBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        printf("�xȡӲ�wУ������ʧ��\r\n");
        return;
    }
    
    /* �@ʾ�����ᅢ�� */
    printf("������ض��a��: %d (0.1��C)\r\n", calibParams.tempOffset);
    printf("������늳��a��: %d (10mV)\r\n", calibParams.batteryOffset);
    printf("\r\n");
}

/**
 * @brief ���Å����yԇ
 */
static void TestParameterReset(void)
{
    printf("�������Å����yԇ...\r\n");
    
    /* �@ʾ����ǰ���� */
    printf("=== ����ǰ���� ===\r\n");
    PrintParameterInfo();
    
    /* ���Å��� */
    PARAM_ResetToDefault();
    
    /* �@ʾ�����ᅢ�� */
    printf("=== �����ᅢ�� ===\r\n");
    PrintParameterInfo();
}

/**
 * @brief ���Ѕ�������yԇ
 */
void RunParameterTest(void)
{
    printf("\r\n===== �_ʼFlash�������淽���yԇ =====\r\n\r\n");
    
    /* ��ʼ����������ϵ�y */
    FLASH_Storage_Init();
    
    /* ��ӡ�����YӍ */
    PrintParameterInfo();
    
    /* �yԇϵ�y�������� */
    printf("�yԇϵ�y��������...\r\n");
    TestSystemParamUpdate();
    
    /* �yԇ���ڕr�g���� */
    printf("�yԇ���ڕr�g����...\r\n");
    TestDateTimeUpdate();
    
    /* �yԇ����ץȡ */
    printf("�yԇ����ץȡ...\r\n");
    TestParametersFetch();
    
    /* �yԇӲ�wУ���������� */
    printf("�yԇӲ�wУ����������...\r\n");
    TestHardwareCalibUpdate();
    
    /* ���Å����yԇ */
    printf("�yԇ��������...\r\n");
    TestParameterReset();
    
    printf("\r\n===== Flash�������淽���yԇ��� =====\r\n");
} 