/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table_example.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x������ʹ�ù���
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_table.h"
#include "ch32v20x.h"
#include <stdio.h>

/*********************************************************************
 * @fn      DisplaySystemParameters
 *
 * @brief   �@ʾϵ�y����
 *
 * @return  none
 */
void DisplaySystemParameters(void)
{
    /* �z�酢�����Ƿ��ѳ�ʼ�� */
    if (!PARAM_IsInitialized()) {
        printf("��������δ��ʼ��!\r\n");
        return;
    }
    
    /* �@ʾ��������^��Ϣ */
    printf("������汾: %u\r\n", g_pParamTable->header.version);
    printf("�����팑��Δ�: %lu\r\n", g_pParamTable->header.writeCounter);
    
    /* �@ʾ����ϵ�y���� */
    printf("\r\n����ϵ�y����:\r\n");
    printf("��늳��ֵ: %.2fV\r\n", g_pParamTable->basic.lbt / 100.0f);
    printf("늳غıM�ֵ: %.2fV\r\n", g_pParamTable->basic.obt / 100.0f);
    printf("����ģʽ: %s\r\n", g_pParamTable->basic.factory == 0 ? "ʹ����" : "���S");
    printf("�aƷ��̖: %u\r\n", g_pParamTable->basic.modelNo);
    printf("�g�w�汾: %u\r\n", g_pParamTable->basic.fwNo);
    printf("�yԇ�Δ�: %u\r\n", g_pParamTable->basic.testCount);
    printf("���a��汾: %u\r\n", g_pParamTable->basic.codeTableVer);
    
    /* �@ʾ�r�g�O�� */
    printf("\r\n�r�g�O��:\r\n");
    printf("����: 20%02u/%02u/%02u\r\n", 
           g_pParamTable->basic.year,
           g_pParamTable->basic.month,
           g_pParamTable->basic.date);
    printf("�r�g: %02u:%02u:%02u\r\n", 
           g_pParamTable->basic.hour,
           g_pParamTable->basic.minute,
           g_pParamTable->basic.second);
           
    /* �@ʾ�yԇ�h������ */
    printf("\r\n�yԇ�h������:\r\n");
    printf("�����ضȹ���: %u.0��C ~ %u.0��C\r\n", 
           g_pParamTable->basic.tempLowLimit,
           g_pParamTable->basic.tempHighLimit);
    
    /* �@ʾ�y����λ�O�� */
    printf("�y����λ: ");
    switch (g_pParamTable->basic.measureUnit) {
        case 0:
            printf("mmol/L\r\n");
            break;
        case 1:
            printf("mg/dL\r\n");
            break;
        case 2:
            printf("gm/dL\r\n");
            break;
        default:
            printf("δ֪\r\n");
    }
    
    /* �@ʾ�A�O�¼� */
    printf("�A�O�¼�: ");
    switch (g_pParamTable->basic.defaultEvent) {
        case 0:
            printf("QC (Ʒ��Һ)\r\n");
            break;
        case 1:
            printf("AC (��ǰ)\r\n");
            break;
        case 2:
            printf("PC (����)\r\n");
            break;
        default:
            printf("δ֪\r\n");
    }
    
    /* �@ʾ��ǰ�yԇ�Ŀ��� */
    printf("�yԇ�Ŀ���: ");
    switch (g_pParamTable->basic.stripType) {
        case STRIP_TYPE_GLV:
            printf("GLV (Ѫ��)\r\n");
            break;
        case STRIP_TYPE_U:
            printf("U (����)\r\n");
            break;
        case STRIP_TYPE_C:
            printf("C (��đ�̴�)\r\n");
            break;
        case STRIP_TYPE_TG:
            printf("TG (�������֬)\r\n");
            break;
        case STRIP_TYPE_GAV:
            printf("GAV (Ѫ��AC)\r\n");
            break;
        default:
            printf("δ֪\r\n");
    }
}

/*********************************************************************
 * @fn      UpdateSystemTime
 *
 * @brief   ����ϵ�y�r�g
 *
 * @param   year - ���(0-99)
 * @param   month - �·�(1-12)
 * @param   date - ����(1-31)
 * @param   hour - С�r(0-23)
 * @param   minute - ���(0-59)
 * @param   second - ��(0-59)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef UpdateSystemTime(uint8_t year, uint8_t month, uint8_t date, 
                                 uint8_t hour, uint8_t minute, uint8_t second)
{
    ParamError_TypeDef result;
    
    /* ��C�r�g���� */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* ���������еĕr�g�O�� */
    result = PARAM_SetValue(9, &year, 1);   // YEAR
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(10, &month, 1);  // MONTH
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(11, &date, 1);   // DATE
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(12, &hour, 1);   // HOUR
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(13, &minute, 1); // MINUTE
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(14, &second, 1); // SECOND
    if (result != PARAM_OK) return result;
    
    /* �����ı��浽Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* ͬ�r����݅^ */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      SetMeasurementUnit
 *
 * @brief   �O�Üy����λ
 *
 * @param   unit - �y����λ(0:mmol/L, 1:mg/dL, 2:gm/dL)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef SetMeasurementUnit(uint8_t unit)
{
    ParamError_TypeDef result;
    
    /* ��C��λ���� */
    if (unit > 2) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* ���������еĜy����λ */
    result = PARAM_SetValue(17, &unit, 1);   // MGDL
    if (result != PARAM_OK) return result;
    
    /* �����ı��浽Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* ͬ�r����݅^ */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      SetStripType
 *
 * @brief   �O�Üyԇ�Ŀ���
 *
 * @param   type - �yԇ�Ŀ���
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef SetStripType(StripType_TypeDef type)
{
    ParamError_TypeDef result;
    uint8_t typeValue = (uint8_t)type;
    
    /* ��C�yԇ�Ŀ��� */
    if (typeValue > STRIP_TYPE_GAV) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* ���������еĜyԇ�Ŀ��� */
    result = PARAM_SetValue(19, &typeValue, 1);   // Strip_Type
    if (result != PARAM_OK) return result;
    
    /* �����ı��浽Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* ͬ�r����݅^ */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      IncrementTestCount
 *
 * @brief   ���Ӝyԇ�Δ�
 *
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef IncrementTestCount(void)
{
    ParamError_TypeDef result;
    uint16_t testCount;
    
    /* �xȡ��ǰ�yԇ�Δ� */
    result = PARAM_GetValue(5, &testCount, 2);   // NOT
    if (result != PARAM_OK) return result;
    
    /* ���Ӝyԇ�Δ� */
    testCount++;
    
    /* ���yԇ�Δ� */
    result = PARAM_SetValue(5, &testCount, 2);   // NOT
    if (result != PARAM_OK) return result;
    
    /* �����ı��浽Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* ͬ�r����݅^ */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      ParamTableExample
 *
 * @brief   ������ʹ�ù���
 *
 * @return  none
 */
void ParamTableExample(void)
{
    ParamError_TypeDef result;
    
    /* ��ʼ�������� */
    result = PARAM_Init();
    if (result != PARAM_OK) {
        printf("�������ʼ��ʧ��! �e�`���a: %d\r\n", result);
        return;
    }
    
    printf("�������ʼ���ɹ�!\r\n\r\n");
    
    /* �@ʾϵ�y���� */
    DisplaySystemParameters();
    
    printf("\r\n�_ʼ���й�������:\r\n");
    
    /* ����ϵ�y�r�g */
    printf("����ϵ�y�r�g...\r\n");
    result = UpdateSystemTime(25, 5, 19, 14, 30, 0);
    if (result != PARAM_OK) {
        printf("���r�gʧ��! �e�`���a: %d\r\n", result);
    }
    
    /* �O�Üy����λ */
    printf("�O�Üy����λ��mg/dL...\r\n");
    result = SetMeasurementUnit(1);
    if (result != PARAM_OK) {
        printf("�O�Üy����λʧ��! �e�`���a: %d\r\n", result);
    }
    
    /* �O�Üyԇ�Ŀ��� */
    printf("�O�Üyԇ�Ŀ������(U)...\r\n");
    result = SetStripType(STRIP_TYPE_U);
    if (result != PARAM_OK) {
        printf("�O�Üyԇ�Ŀʧ��! �e�`���a: %d\r\n", result);
    }
    
    /* ���Ӝyԇ�Δ� */
    printf("���Ӝyԇ�Δ�...\r\n");
    result = IncrementTestCount();
    if (result != PARAM_OK) {
        printf("���Ӝyԇ�Δ�ʧ��! �e�`���a: %d\r\n", result);
    }
    
    printf("\r\n������ą���:\r\n");
    DisplaySystemParameters();
    
    printf("\r\n������ʹ�ù������!\r\n");
}
