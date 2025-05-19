/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_utils.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數工具函數實現
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_utils.h"

/*********************************************************************
 * @fn      PARAM_GetByte
 *
 * @brief   獲取1位元組參數值
 *
 * @param   paramAddress - 參數地址
 * @return  uint8_t - 參數值
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
 * @brief   獲取2位元組參數值
 *
 * @param   paramAddress - 參數地址
 * @return  uint16_t - 參數值
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
 * @brief   獲取浮點參數值
 *
 * @param   paramAddress - 參數地址
 * @return  float - 參數值
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
 * @brief   設置1位元組參數值
 *
 * @param   paramAddress - 參數地址
 * @param   value - 參數值
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SetByte(uint16_t paramAddress, uint8_t value)
{
    return PARAM_SetValue(paramAddress, &value, 1);
}

/*********************************************************************
 * @fn      PARAM_SetWord
 *
 * @brief   設置2位元組參數值
 *
 * @param   paramAddress - 參數地址
 * @param   value - 參數值
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SetWord(uint16_t paramAddress, uint16_t value)
{
    return PARAM_SetValue(paramAddress, &value, 2);
}

/*********************************************************************
 * @fn      PARAM_SetFloat
 *
 * @brief   設置浮點參數值
 *
 * @param   paramAddress - 參數地址
 * @param   value - 參數值
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SetFloat(uint16_t paramAddress, float value)
{
    return PARAM_SetValue(paramAddress, &value, sizeof(float));
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   獲取日期時間
 *
 * @param   year - 年份指針 (0-99)
 * @param   month - 月份指針 (1-12)
 * @param   date - 日期指針 (1-31)
 * @param   hour - 小時指針 (0-23)
 * @param   minute - 分鐘指針 (0-59)
 * @param   second - 秒數指針 (0-59)
 * @return  ParamError_TypeDef - 操作結果
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
 * @brief   設置日期時間
 *
 * @param   year - 年份 (0-99)
 * @param   month - 月份 (1-12)
 * @param   date - 日期 (1-31)
 * @param   hour - 小時 (0-23)
 * @param   minute - 分鐘 (0-59)
 * @param   second - 秒數 (0-59)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date,
                                   uint8_t hour, uint8_t minute, uint8_t second)
{
    ParamError_TypeDef result;
    
    /* 驗證時間參數 */
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
    
    /* 將更改保存到Flash */
    result = PARAM_SaveToFlash(PARAM_AREA_MAIN);
    if (result != PARAM_OK) return result;
    
    /* 同時更新備份區 */
    return PARAM_Backup();
}

/*********************************************************************
 * @fn      StripType_GetName
 *
 * @brief   獲取試片類型名稱
 *
 * @param   type - 試片類型
 * @return  const char* - 試片類型名稱
 */
const char* StripType_GetName(StripType_TypeDef type)
{
    switch (type)
    {
        case STRIP_TYPE_GLV:
            return "GLV (血糖)";
        case STRIP_TYPE_U:
            return "U (尿酸)";
        case STRIP_TYPE_C:
            return "C (總膽固醇)";
        case STRIP_TYPE_TG:
            return "TG (三酸甘油脂)";
        case STRIP_TYPE_GAV:
            return "GAV (血糖)";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Unit_GetSymbol
 *
 * @brief   獲取測量單位符號
 *
 * @param   unit - 測量單位
 * @return  const char* - 測量單位符號
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
 * @brief   獲取事件類型名稱
 *
 * @param   event - 事件類型
 * @return  const char* - 事件類型名稱
 */
const char* Event_GetName(Event_TypeDef event)
{
    switch (event)
    {
        case EVENT_NONE:
            return "無事件";
        case EVENT_BEFORE_MEAL:
            return "飯前";
        case EVENT_AFTER_MEAL:
            return "飯後";
        case EVENT_FASTING:
            return "空腹";
        case EVENT_BEDTIME:
            return "睡前";
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
