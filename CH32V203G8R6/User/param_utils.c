/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_utils.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x�������ߺ������F
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_utils.h"

/*********************************************************************
 * @fn      PARAM_GetByte
 *
 * @brief   �@ȡ1λԪ�M����ֵ
 *
 * @param   paramAddress - ������ַ
 * @return  uint8_t - ����ֵ
 */
uint8_t PARAM_GetByte(uint16_t paramAddress)
{
    uint8_t value = 0;
    PARAM_GetValue(paramAddress, &value, 1);
    return value;
}

/*********************************************************************
 * @fn      PARAM_GetWord
 *
 * @brief   �@ȡ2λԪ�M����ֵ
 *
 * @param   paramAddress - ������ַ
 * @return  uint16_t - ����ֵ
 */
uint16_t PARAM_GetWord(uint16_t paramAddress)
{
    uint16_t value = 0;
    PARAM_GetValue(paramAddress, &value, 2);
    return value;
}

/*********************************************************************
 * @fn      PARAM_GetFloat
 *
 * @brief   �@ȡ���c����ֵ
 *
 * @param   paramAddress - ������ַ
 * @return  float - ����ֵ
 */
float PARAM_GetFloat(uint16_t paramAddress)
{
    float value = 0.0f;
    PARAM_GetValue(paramAddress, &value, sizeof(float));
    return value;
}

/*********************************************************************
 * @fn      PARAM_SetByte
 *
 * @brief   �O��1λԪ�M����ֵ
 *
 * @param   paramAddress - ������ַ
 * @param   value - ����ֵ
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SetByte(uint16_t paramAddress, uint8_t value)
{
    return PARAM_SetValue(paramAddress, &value, 1);
}

/*********************************************************************
 * @fn      PARAM_SetWord
 *
 * @brief   �O��2λԪ�M����ֵ
 *
 * @param   paramAddress - ������ַ
 * @param   value - ����ֵ
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SetWord(uint16_t paramAddress, uint16_t value)
{
    return PARAM_SetValue(paramAddress, &value, 2);
}

/*********************************************************************
 * @fn      PARAM_SetFloat
 *
 * @brief   �O�ø��c����ֵ
 *
 * @param   paramAddress - ������ַ
 * @param   value - ����ֵ
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SetFloat(uint16_t paramAddress, float value)
{
    return PARAM_SetValue(paramAddress, &value, sizeof(float));
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   �@ȡ���ڕr�g
 *
 * @param   year - ���ָ� (0-99)
 * @param   month - �·�ָ� (1-12)
 * @param   date - ����ָ� (1-31)
 * @param   hour - С�rָ� (0-23)
 * @param   minute - ���ָ� (0-59)
 * @param   second - �딵ָ� (0-59)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date,
                                   uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    ParamError_TypeDef result;
    
    result = PARAM_GetValue(PARAM_YEAR, year, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_GetValue(PARAM_MONTH, month, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_GetValue(PARAM_DATE, date, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_GetValue(PARAM_HOUR, hour, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_GetValue(PARAM_MINUTE, minute, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_GetValue(PARAM_SECOND, second, 1);
    return result;
}

/*********************************************************************
 * @fn      PARAM_SetDateTime
 *
 * @brief   �O�����ڕr�g
 *
 * @param   year - ��� (0-99)
 * @param   month - �·� (1-12)
 * @param   date - ���� (1-31)
 * @param   hour - С�r (0-23)
 * @param   minute - ��� (0-59)
 * @param   second - �딵 (0-59)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date,
                                   uint8_t hour, uint8_t minute, uint8_t second)
{
    ParamError_TypeDef result;
    
    /* ��C�r�g���� */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    result = PARAM_SetValue(PARAM_YEAR, &year, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(PARAM_MONTH, &month, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(PARAM_DATE, &date, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(PARAM_HOUR, &hour, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(PARAM_MINUTE, &minute, 1);
    if (result != PARAM_OK) return result;
    
    result = PARAM_SetValue(PARAM_SECOND, &second, 1);
    if (result != PARAM_OK) return result;
    
    /* �����ı��浽Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* ͬ�r����݅^ */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      StripType_GetName
 *
 * @brief   �@ȡԇƬ������Q
 *
 * @param   type - ԇƬ���
 * @return  const char* - ԇƬ������Q
 */
const char* StripType_GetName(StripType_TypeDef type)
{
    switch (type)
    {
        case STRIP_TYPE_GLV:
            return "GLV (Ѫ��)";
        case STRIP_TYPE_U:
            return "U (����)";
        case STRIP_TYPE_C:
            return "C (��đ�̴�)";
        case STRIP_TYPE_TG:
            return "TG (�������֬)";
        case STRIP_TYPE_GAV:
            return "GAV (Ѫ��)";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Unit_GetSymbol
 *
 * @brief   �@ȡ�y����λ��̖
 *
 * @param   unit - �y����λ
 * @return  const char* - �y����λ��̖
 */
const char* Unit_GetSymbol(Unit_TypeDef unit)
{
    switch (unit)
    {
        case UNIT_MMOL_L:
            return "mmol/L";
        case UNIT_MG_DL:
            return "mg/dL";
        case UNIT_GM_DL:
            return "gm/dL";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Event_GetName
 *
 * @brief   �@ȡ�¼�������Q
 *
 * @param   event - �¼����
 * @return  const char* - �¼�������Q
 */
const char* Event_GetName(Event_TypeDef event)
{
    switch (event)
    {
        case EVENT_NONE:
            return "�o�¼�";
        case EVENT_BEFORE_MEAL:
            return "�ǰ";
        case EVENT_AFTER_MEAL:
            return "���";
        case EVENT_FASTING:
            return "�ո�";
        case EVENT_BEDTIME:
            return "˯ǰ";
        case EVENT_AC:
            return "AC";
        case EVENT_PC:
            return "PC";
        case EVENT_QC:
            return "QC";
        default:
            return "Unknown";
    }
}
