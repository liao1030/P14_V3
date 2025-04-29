/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_test.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter storage test module for CH32V203G8R6.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_test.h"
#include "param_store.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>  /* ��� abs ���� */

/* ��ģ�K����ԭ�� */
static void PARAM_TestBasicParams(void);
static void PARAM_TestCalibParams(void);
static void PARAM_TestStripParams(void);
static void PARAM_TestDateTime(void);
static void PARAM_TestBackupRecovery(void);
static void PARAM_TestGetFunctions(void);

/*********************************************************************
 * @fn      PARAM_TestAll
 *
 * @brief   �������Ѕ�������ģ�K�yԇ
 *
 * @return  �o
 */
void PARAM_TestAll(void)
{
    printf("�_ʼ��������ģ�K�yԇ...\r\n");
    
    /* ��ʼ����������ģ�K */
    if (PARAM_Init() != 0) {
        printf("��������ģ�K��ʼ��ʧ����\r\n");
        return;
    }
    
    /* ���и�헜yԇ */
    PARAM_TestBasicParams();
    PARAM_TestCalibParams();
    PARAM_TestStripParams();
    PARAM_TestDateTime();
    PARAM_TestBackupRecovery();
    PARAM_TestGetFunctions();
    
    printf("��������ģ�K�yԇ��ɣ�\r\n");
}

/*********************************************************************
 * @fn      PARAM_TestBasicParams
 *
 * @brief   �yԇ����ϵ�y�������x��
 *
 * @return  �o
 */
static void PARAM_TestBasicParams(void)
{
    printf("\r\n�yԇ����ϵ�y����...\r\n");
    
    /* �xȡ��ǰ���� */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("�xȡ����ϵ�y����ʧ����\r\n");
        return;
    }
    
    /* �@ʾ��ǰ���� */
    printf("��ǰ����������\r\n");
    printf("늳��ֵ: ��=%d.%02dV, �ıM=%d.%02dV\r\n", 
        sysBlock.lbt / 100, sysBlock.lbt % 100, 
        sysBlock.obt / 100, sysBlock.obt % 100);
    printf("��̖: %d, �g�w�汾: %d.%d\r\n", 
        sysBlock.modelNo, sysBlock.fwNo / 10, sysBlock.fwNo % 10);
    printf("�yԇ�Δ�: %d, �A�O�¼�: %d\r\n", 
        sysBlock.testCount, sysBlock.defaultEvent);
    
    /* �޸ą��� */
    uint16_t oldTestCount = sysBlock.testCount;
    uint8_t oldEvent = sysBlock.defaultEvent;
    
    sysBlock.defaultEvent = (sysBlock.defaultEvent + 1) % 3; /* �ГQ�A�O�¼� */
    sysBlock.testCount += 10; /* ���Ӝyԇ�Δ� */
    
    printf("�޸ą������¼�=%d, �yԇ�Δ�=%d\r\n", 
        sysBlock.defaultEvent, sysBlock.testCount);
    
    /* ���녢�� */
    if (PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("���»���ϵ�y����ʧ����\r\n");
        return;
    }
    
    /* �ٴ��xȡ�_�J */
    memset(&sysBlock, 0, sizeof(BasicSystemBlock));
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("�xȡ����ϵ�y����ʧ����\r\n");
        return;
    }
    
    /* �z�酢���Ƿ��Ѹ��� */
    if (sysBlock.defaultEvent == oldEvent || sysBlock.testCount == oldTestCount) {
        printf("����������Cʧ����\r\n");
    } else {
        printf("����������C�ɹ���\r\n");
    }
    
    /* �֏�ԭ��ą��� */
    sysBlock.defaultEvent = oldEvent;
    sysBlock.testCount = oldTestCount;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_TestCalibParams
 *
 * @brief   �yԇӲ�wУ�ʅ������x��
 *
 * @return  �o
 */
static void PARAM_TestCalibParams(void)
{
    printf("\r\n�yԇӲ�wУ�ʅ���...\r\n");
    
    /* �xȡ��ǰ���� */
    HardwareCalibBlock calibBlock;
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("�xȡӲ�wУ�ʅ���ʧ����\r\n");
        return;
    }
    
    /* �@ʾ��ǰ���� */
    printf("��ǰУ�ʅ�����\r\n");
    printf("EV_T3�|�l: %dmV, ����늉�: %d, T3늉�: %d\r\n", 
        calibBlock.evT3Trigger, calibBlock.evWorking, calibBlock.evT3);
    printf("�ض�ƫ��: %d.%01d��C, 늳�ƫ��: %dmV\r\n", 
        calibBlock.tempOffset / 10, abs(calibBlock.tempOffset) % 10, 
        calibBlock.batteryOffset * 10);
    
    /* �޸ą��� */
    int8_t oldTempOffset = calibBlock.tempOffset;
    int8_t oldBattOffset = calibBlock.batteryOffset;
    
    calibBlock.tempOffset = 5;  /* �O�Üض�ƫ�ƞ�0.5��C */
    calibBlock.batteryOffset = 3; /* �O��늳�ƫ�ƞ�30mV */
    
    printf("�޸ą������ض�ƫ��=%d.%01d��C, 늳�ƫ��=%dmV\r\n", 
        calibBlock.tempOffset / 10, abs(calibBlock.tempOffset) % 10, 
        calibBlock.batteryOffset * 10);
    
    /* ���녢�� */
    if (PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("����Ӳ�wУ�ʅ���ʧ����\r\n");
        return;
    }
    
    /* �ٴ��xȡ�_�J */
    memset(&calibBlock, 0, sizeof(HardwareCalibBlock));
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock)) != 0) {
        printf("�xȡӲ�wУ�ʅ���ʧ����\r\n");
        return;
    }
    
    /* �z�酢���Ƿ��Ѹ��� */
    if (calibBlock.tempOffset != 5 || calibBlock.batteryOffset != 3) {
        printf("����������Cʧ����\r\n");
    } else {
        printf("����������C�ɹ���\r\n");
    }
    
    /* �֏�ԭ��ą��� */
    calibBlock.tempOffset = oldTempOffset;
    calibBlock.batteryOffset = oldBattOffset;
    PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibBlock, sizeof(HardwareCalibBlock));
}

/*********************************************************************
 * @fn      PARAM_TestStripParams
 *
 * @brief   �yԇԇƬ�������x��
 *
 * @return  �o
 */
static void PARAM_TestStripParams(void)
{
    printf("\r\n�yԇԇƬ����...\r\n");
    
    /* �xȡѪ��ԇƬ���� */
    TestParamBaseBlock bgBlock;
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("�xȡѪ�ǅ���ʧ����\r\n");
        return;
    }
    
    /* �@ʾ��ǰ���� */
    printf("��ǰѪ��ԇƬ������\r\n");
    printf("�z�y�ֵ: ��ԇƬ=%d, ����=%d, ѪҺ=%d\r\n", 
        bgBlock.ndl, bgBlock.udl, bgBlock.bloodIn);
    printf("�y������: %d-%dmg/dL\r\n", bgBlock.lowLimit, bgBlock.highLimit);
    printf("�yԇ�r��1: TPL=%dms, TRD=%dms, EV=%dms\r\n", 
        bgBlock.tpl1 * 100, bgBlock.trd1 * 100, bgBlock.evWidth1 * 100);
    
    /* �޸ą��� */
    uint16_t oldNdl = bgBlock.ndl;
    uint16_t oldTpl1 = bgBlock.tpl1;
    
    bgBlock.ndl = 120;  /* �޸���ԇƬ�ֵ */
    bgBlock.tpl1 = 15;  /* �޸Ĝyԇ�r�� */
    
    printf("�޸ą�������ԇƬ�ֵ=%d, TPL1=%dms\r\n", 
        bgBlock.ndl, bgBlock.tpl1 * 100);
    
    /* ���녢�� */
    if (PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("����Ѫ�ǅ���ʧ����\r\n");
        return;
    }
    
    /* �ٴ��xȡ�_�J */
    memset(&bgBlock, 0, sizeof(TestParamBaseBlock));
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock)) != 0) {
        printf("�xȡѪ�ǅ���ʧ����\r\n");
        return;
    }
    
    /* �z�酢���Ƿ��Ѹ��� */
    if (bgBlock.ndl != 120 || bgBlock.tpl1 != 15) {
        printf("����������Cʧ����\r\n");
    } else {
        printf("����������C�ɹ���\r\n");
    }
    
    /* �֏�ԭ��ą��� */
    bgBlock.ndl = oldNdl;
    bgBlock.tpl1 = oldTpl1;
    PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgBlock, sizeof(TestParamBaseBlock));
}

/*********************************************************************
 * @fn      PARAM_TestDateTime
 *
 * @brief   �yԇ���ڕr�g�������x��
 *
 * @return  �o
 */
static void PARAM_TestDateTime(void)
{
    printf("\r\n�yԇ���ڕr�g����...\r\n");
    
    /* �xȡ��ǰ���ڕr�g */
    uint8_t year, month, date, hour, minute, second;
    if (PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second) != 0) {
        printf("�xȡ���ڕr�gʧ����\r\n");
        return;
    }
    
    /* �@ʾ��ǰ���ڕr�g */
    printf("��ǰ���ڕr�g��20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
        year, month, date, hour, minute, second);
    
    /* �޸����ڕr�g */
    uint8_t newHour = (hour + 1) % 24;
    uint8_t newMinute = (minute + 30) % 60;
    
    printf("�޸ĕr�g�飺20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
        year, month, date, newHour, newMinute, second);
    
    /* �������ڕr�g */
    if (PARAM_UpdateDateTime(year, month, date, newHour, newMinute, second) != 0) {
        printf("�������ڕr�gʧ����\r\n");
        return;
    }
    
    /* �ٴ��xȡ�_�J */
    if (PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second) != 0) {
        printf("�xȡ���ڕr�gʧ����\r\n");
        return;
    }
    
    /* �z��r�g�Ƿ��Ѹ��� */
    if (hour != newHour || minute != newMinute) {
        printf("�r�g������Cʧ����\r\n");
    } else {
        printf("�r�g������C�ɹ����r�g��%02d:%02d:%02d\r\n", 
            hour, minute, second);
    }
}

/*********************************************************************
 * @fn      PARAM_TestBackupRecovery
 *
 * @brief   �yԇ��݅^����
 *
 * @return  �o
 */
static void PARAM_TestBackupRecovery(void)
{
    printf("\r\n�yԇ��݅^����...\r\n");
    
    /* �@ʾ��ǰ��Ӆ^�� */
    uint32_t activeAddr = PARAM_GetActiveBlockAddr();
    printf("��ǰ��Ӆ^�K��ַ: 0x%08X\r\n", (unsigned int)activeAddr);
    
    /* �xȡ��ǰ���� */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("�xȡ����ϵ�y����ʧ����\r\n");
        return;
    }
    
    /* �޸ą������|�l��݌��� */
    uint16_t oldTestCount = sysBlock.testCount;
    sysBlock.testCount += 1;
    
    /* ������ */
    if (PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("���»���ϵ�y����ʧ����\r\n");
        return;
    }
    
    /* �z���Ӆ^���Ƿ����ГQ */
    uint32_t newActiveAddr = PARAM_GetActiveBlockAddr();
    if (newActiveAddr == activeAddr) {
        printf("��Ӆ^�Kδ�ГQ��\r\n");
        return;
    }
    
    printf("��Ӆ^�K���ГQ��0x%08X -> 0x%08X\r\n", 
        (unsigned int)activeAddr, (unsigned int)newActiveAddr);
    
    /* ��C�yԇӋ�������� */
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        printf("�xȡ����ϵ�y����ʧ����\r\n");
        return;
    }
    
    if (sysBlock.testCount != oldTestCount + 1) {
        printf("��݅^������Cʧ����\r\n");
    } else {
        printf("��݅^������C�ɹ���\r\n");
    }
}

/*********************************************************************
 * @fn      PARAM_TestGetFunctions
 *
 * @brief   �yԇ�����@ȡ����
 *
 * @return  �o
 */
static void PARAM_TestGetFunctions(void)
{
    printf("\r\n�yԇ�����@ȡ����...\r\n");
    
    /* �xȡ�yԇ���� */
    uint16_t ndl, udl, bloodIn;
    if (PARAM_GetStripParametersByStripType(0, &ndl, &udl, &bloodIn) != 0) {
        printf("�@ȡѪ��ԇƬ����ʧ����\r\n");
        return;
    }
    
    printf("Ѫ��ԇƬ늉���������ԇƬ=%d, ����=%d, ѪҺ=%d\r\n", 
        ndl, udl, bloodIn);
    
    /* �xȡ�y������ */
    uint8_t lowLimit;
    uint16_t highLimit;
    if (PARAM_GetMeasurementRangeByStripType(0, &lowLimit, &highLimit) != 0) {
        printf("�@ȡѪ�ǜy������ʧ����\r\n");
        return;
    }
    
    printf("Ѫ�ǜy��������%d-%dmg/dL\r\n", 
        lowLimit, highLimit);
    
    /* �xȡ�r�򅢔� */
    uint16_t tpl, trd, evWidth;
    if (PARAM_GetTimingParametersByStripType(0, &tpl, &trd, &evWidth, 1) != 0) {
        printf("�@ȡѪ�Ǖr�򅢔�ʧ����\r\n");
        return;
    }
    
    printf("Ѫ�Ǖr�򅢔�(�M1)��TPL=%dms, TRD=%dms, EV=%dms\r\n", 
        tpl * 100, trd * 100, evWidth * 100);
    
    /* �yԇ���ӜyԇӋ�� */
    BasicSystemBlock sysBlock;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
    uint16_t oldCount = sysBlock.testCount;
    
    if (PARAM_IncreaseTestCount() != 0) {
        printf("���ӜyԇӋ��ʧ����\r\n");
        return;
    }
    
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
    printf("�yԇӋ����%d -> %d\r\n", oldCount, sysBlock.testCount);
    
    /* �؏�ԭӋ�� */
    sysBlock.testCount = oldCount;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
} 