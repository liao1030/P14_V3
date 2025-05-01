/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Parameter_Table.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : 多功能生化測試儀參數代碼表實現
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Parameter_Table.h"
#include "debug.h"
#include <string.h>

/* 參數表全局變量 */
static uint8_t g_paramTable[PARAM_TABLE_SIZE];

/* 默認參數值 */
static const uint8_t g_defaultParamTable[PARAM_TABLE_SIZE] = {
    /* 1. 系統基本參數 */
    58,     // PARAM_LBT = 2.58V (低電池閾值)
    55,     // PARAM_OBT = 2.55V (電池耗盡閾值)
    0,      // PARAM_FACTORY = 0 (使用者模式)
    1,      // PARAM_MODEL_NO = 1 (產品型號)
    1,      // PARAM_FW_NO = 1 (韌體版本號)
    0, 0,   // PARAM_NOT = 0 (測量次數，2 bytes)
    1, 0,   // PARAM_CODE_TABLE_V = 1 (代號表版本編號，2 bytes)
    
    /* 2. 時間設定參數 */
    24,     // PARAM_YEAR = 24 (2024年)
    8,      // PARAM_MONTH = 8 (8月)
    12,     // PARAM_DATE = 12 (12日)
    0,      // PARAM_HOUR = 0 (0時)
    0,      // PARAM_MINUTE = 0 (0分)
    0,      // PARAM_SECOND = 0 (0秒)
    
    /* 3. 測量環境參數 */
    1,      // PARAM_TLL = 1.0℃ (操作溫度範圍下限)
    40,     // PARAM_TLH = 40.0℃ (操作溫度範圍上限)
    0,      // PARAM_MGDL = 0 (mmol/L)
    0,      // PARAM_EVENT = 0 (QC)
    0,      // PARAM_STRIP_TYPE = 0 (血糖)
};

/*********************************************************************
 * @fn      P14_ParamTable_Init
 *
 * @brief   初始化參數表
 *
 * @return  none
 */
void P14_ParamTable_Init(void)
{
    printf("初始化參數代碼表...\r\n");
    
    /* 初始化參數表為0 */
    memset(g_paramTable, 0, PARAM_TABLE_SIZE);
    
    /* 嘗試從存儲器載入參數表 */
    P14_ParamTable_Load();
    
    /* 檢查數據有效性，如果無效則載入默認值 */
    if (!P14_ParamTable_VerifyChecksum()) {
        printf("參數表校驗失敗，載入默認值\r\n");
        P14_ParamTable_LoadDefault();
    } else {
        printf("參數表校驗成功\r\n");
    }
    
    /* 顯示基本參數信息 */
    printf("產品型號: %d\r\n", P14_ParamTable_Read(PARAM_MODEL_NO));
    printf("韌體版本: %d\r\n", P14_ParamTable_Read(PARAM_FW_NO));
    printf("代碼表版本: %d\r\n", P14_ParamTable_Read16(PARAM_CODE_TABLE_V));
}

/*********************************************************************
 * @fn      P14_ParamTable_LoadDefault
 *
 * @brief   載入默認參數值
 *
 * @return  none
 */
void P14_ParamTable_LoadDefault(void)
{
    /* 拷貝默認參數到參數表 */
    memcpy(g_paramTable, g_defaultParamTable, sizeof(g_defaultParamTable));
    
    /* 更新校驗和 */
    P14_ParamTable_UpdateChecksum();
    
    /* 保存到存儲器 */
    P14_ParamTable_Save();
}

/*********************************************************************
 * @fn      P14_ParamTable_Read
 *
 * @brief   讀取單字節參數
 *
 * @param   address - 參數地址
 *
 * @return  參數值
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
 * @brief   寫入單字節參數
 *
 * @param   address - 參數地址
 * @param   value - 參數值
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
 * @brief   讀取雙字節參數
 *
 * @param   address - 參數地址
 *
 * @return  參數值
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
 * @brief   寫入雙字節參數
 *
 * @param   address - 參數地址
 * @param   value - 參數值
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
 * @brief   讀取浮點數參數 (4字節)
 *
 * @param   address - 參數地址
 *
 * @return  參數值
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
 * @brief   寫入浮點數參數 (4字節)
 *
 * @param   address - 參數地址
 * @param   value - 參數值
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
 * @brief   讀取參數塊
 *
 * @param   address - 起始地址
 * @param   buffer - 目標緩衝區
 * @param   length - 數據長度
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
 * @brief   寫入參數塊
 *
 * @param   address - 起始地址
 * @param   buffer - 源數據緩衝區
 * @param   length - 數據長度
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
 * @brief   驗證參數表校驗和
 *
 * @return  0: 校驗失敗, 1: 校驗成功
 */
uint8_t P14_ParamTable_VerifyChecksum(void)
{
    uint16_t calcSum = 0;
    uint16_t storedSum;
    
    /* 計算0-674地址的總和 */
    for (uint16_t i = 0; i < PARAM_SUM_L; i++) {
        calcSum += g_paramTable[i];
    }
    
    /* 獲取存儲的校驗和 */
    storedSum = g_paramTable[PARAM_SUM_L] | ((uint16_t)g_paramTable[PARAM_SUM_H] << 8);
    
    /* 比較計算的校驗和與存儲的校驗和 */
    return (calcSum == storedSum) ? 1 : 0;
}

/*********************************************************************
 * @fn      P14_ParamTable_UpdateChecksum
 *
 * @brief   更新參數表校驗和
 *
 * @return  none
 */
void P14_ParamTable_UpdateChecksum(void)
{
    uint16_t sum = 0;
    
    /* 計算0-674地址的總和 */
    for (uint16_t i = 0; i < PARAM_SUM_L; i++) {
        sum += g_paramTable[i];
    }
    
    /* 存儲校驗和 */
    g_paramTable[PARAM_SUM_L] = (uint8_t)(sum & 0xFF);
    g_paramTable[PARAM_SUM_H] = (uint8_t)((sum >> 8) & 0xFF);
    
    /* 計算CRC16校驗值 (簡化實現，實際項目可能需要標準CRC16算法) */
    uint8_t crc = 0;
    for (uint16_t i = 0; i < PARAM_CRC16; i++) {
        crc ^= g_paramTable[i];
    }
    g_paramTable[PARAM_CRC16] = crc;
}

/*********************************************************************
 * @fn      P14_ParamTable_Save
 *
 * @brief   保存參數表到存儲器
 *
 * @return  none
 */
void P14_ParamTable_Save(void)
{
    /* 此處應實現將參數表保存到非揮發性存儲器的邏輯 */
    /* 根據硬體配置，可能是EEPROM、Flash等 */
    printf("參數表保存\r\n");
    
    /* 示例: 如果使用內部Flash，需要首先擦除相應區域，然後寫入數據 */
    /* 實際實現需要根據硬體配置修改 */
}

/*********************************************************************
 * @fn      P14_ParamTable_Load
 *
 * @brief   從存儲器載入參數表
 *
 * @return  none
 */
void P14_ParamTable_Load(void)
{
    /* 此處應實現從非揮發性存儲器讀取參數表的邏輯 */
    /* 根據硬體配置，可能是EEPROM、Flash等 */
    printf("參數表載入\r\n");
    
    /* 示例: 如果使用內部Flash，需要從指定地址讀取數據 */
    /* 實際實現需要根據硬體配置修改 */
}

/*********************************************************************
 * @fn      P14_ParamTable_GetStripTypeName
 *
 * @brief   獲取試片類型名稱
 *
 * @param   type - 試片類型
 *
 * @return  類型名稱字符串
 */
const char* P14_ParamTable_GetStripTypeName(StripType_TypeDef type)
{
    switch (type) {
        case STRIP_TYPE_GLV: return "血糖";
        case STRIP_TYPE_U:   return "尿酸";
        case STRIP_TYPE_C:   return "總膽固醇";
        case STRIP_TYPE_TG:  return "三酸甘油脂";
        case STRIP_TYPE_GAV: return "血糖(GAV)";
        default:             return "未知";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_GetEventName
 *
 * @brief   獲取事件類型名稱
 *
 * @param   event - 事件類型
 *
 * @return  事件名稱字符串
 */
const char* P14_ParamTable_GetEventName(Event_TypeDef event)
{
    switch (event) {
        case EVENT_QC: return "品管液測試";
        case EVENT_AC: return "餐前測試";
        case EVENT_PC: return "餐後測試";
        default:       return "未知";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_GetUnitName
 *
 * @brief   獲取測量單位名稱
 *
 * @param   unit - 測量單位
 *
 * @return  單位名稱字符串
 */
const char* P14_ParamTable_GetUnitName(Unit_TypeDef unit)
{
    switch (unit) {
        case UNIT_MMOL_L: return "mmol/L";
        case UNIT_MG_DL:  return "mg/dL";
        case UNIT_GM_DL:  return "gm/dl";
        default:          return "未知";
    }
}

/*********************************************************************
 * @fn      P14_ParamTable_IncrementTestCount
 *
 * @brief   增加測試次數計數器
 *
 * @return  none
 */
void P14_ParamTable_IncrementTestCount(void)
{
    uint16_t count = P14_ParamTable_Read16(PARAM_NOT);
    count++;
    P14_ParamTable_Write16(PARAM_NOT, count);
    
    /* 自動保存更新後的參數表 */
    P14_ParamTable_UpdateChecksum();
    P14_ParamTable_Save();
}

/*********************************************************************
 * @fn      P14_ParamTable_ResetTestCount
 *
 * @brief   重置測試次數計數器
 *
 * @return  none
 */
void P14_ParamTable_ResetTestCount(void)
{
    P14_ParamTable_Write16(PARAM_NOT, 0);
    
    /* 自動保存更新後的參數表 */
    P14_ParamTable_UpdateChecksum();
    P14_ParamTable_Save();
} 