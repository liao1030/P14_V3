/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_code.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀參數代碼表實現
 *********************************************************************************
 * Copyright (c) 2025 HMD.
 *******************************************************************************/

#include "parameter_code.h"
#include "debug.h"
#include "string.h"

/* 全域變數 */
static uint8_t g_ParameterTable[PARAM_TABLE_SIZE]; // 參數表陣列
static uint8_t g_ParameterInitialized = 0;         // 參數表初始化標誌

/* 預設值定義 */
#define DEFAULT_LBT          58  // 默認低電池閾值 (2.58V)
#define DEFAULT_OBT          55  // 默認電池耗盡閾值 (2.55V)
#define DEFAULT_FACTORY      0   // 默認為使用者模式
#define DEFAULT_MODEL_NO     1   // 默認產品型號
#define DEFAULT_FW_NO        1   // 默認韌體版本
#define DEFAULT_NOT          0   // 默認測量次數
#define DEFAULT_CODE_TABLE_V 1   // 默認代碼表版本

#define DEFAULT_YEAR         25  // 默認年份 (2025)
#define DEFAULT_MONTH        5   // 默認月份 (5月)
#define DEFAULT_DATE         2   // 默認日期 (2日)
#define DEFAULT_HOUR         12  // 默認小時
#define DEFAULT_MINUTE       0   // 默認分鐘
#define DEFAULT_SECOND       0   // 默認秒數

#define DEFAULT_TLL          1   // 默認操作溫度下限 (1.0°C)
#define DEFAULT_TLH          40  // 默認操作溫度上限 (40.0°C)
#define DEFAULT_MGDL         0   // 默認測量單位 (mmol/L)
#define DEFAULT_EVENT        0   // 默認事件類型 (QC)
#define DEFAULT_STRIP_TYPE   0   // 默認測量項目 (血糖)

#define DEFAULT_EV_T3_TRG    150 // 默認EV_T3觸發電位 (1500mV)
#define DEFAULT_EV_WORKING   164 // 默認測量工作電位電壓
#define DEFAULT_EV_T3        164 // 默認血液導入電位電壓
#define DEFAULT_DACO         0   // 默認DAC偏移
#define DEFAULT_DACDO        0   // 默認DAC校準管理參數
#define DEFAULT_CC211NoDone  0   // 默認CC211正常
#define DEFAULT_CAL_TOL      8   // 默認校準容差 (0.8%)

/*********************************************************************
 * @fn      Parameter_Init
 *
 * @brief   初始化參數表
 *
 * @return  none
 */
void Parameter_Init(void)
{
    if(g_ParameterInitialized == 0)
    {
        // 嘗試從Flash或EEPROM載入參數
        // 如果失敗則設置默認值
        Parameter_Load();
        
        // 檢查校驗和
        if(Parameter_CheckSum() != 0)
        {
            printf("Parameter checksum error, reset to default values\r\n");
            Parameter_SetDefaultValues();
            Parameter_Save();
        }
        
        g_ParameterInitialized = 1;
    }
}

/*********************************************************************
 * @fn      Parameter_Read
 *
 * @brief   讀取指定地址的參數(8位)
 *
 * @param   addr - 參數地址
 *
 * @return  參數值
 */
uint8_t Parameter_Read(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        return g_ParameterTable[addr];
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write
 *
 * @brief   寫入指定地址的參數(8位)
 *
 * @param   addr - 參數地址
 * @param   value - 參數值
 *
 * @return  none
 */
void Parameter_Write(uint16_t addr, uint8_t value)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        g_ParameterTable[addr] = value;
    }
}

/*********************************************************************
 * @fn      Parameter_Read16
 *
 * @brief   讀取指定地址的參數(16位)
 *
 * @param   addr - 參數地址
 *
 * @return  參數值
 */
uint16_t Parameter_Read16(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE - 1)
    {
        return (uint16_t)g_ParameterTable[addr] | 
               ((uint16_t)g_ParameterTable[addr + 1] << 8);
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write16
 *
 * @brief   寫入指定地址的參數(16位)
 *
 * @param   addr - 參數地址
 * @param   value - 參數值
 *
 * @return  none
 */
void Parameter_Write16(uint16_t addr, uint16_t value)
{
    if(addr < PARAM_TABLE_SIZE - 1)
    {
        g_ParameterTable[addr] = (uint8_t)(value & 0xFF);
        g_ParameterTable[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/*********************************************************************
 * @fn      Parameter_Read32
 *
 * @brief   讀取指定地址的參數(32位)
 *
 * @param   addr - 參數地址
 *
 * @return  參數值
 */
uint32_t Parameter_Read32(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE - 3)
    {
        return (uint32_t)g_ParameterTable[addr] | 
               ((uint32_t)g_ParameterTable[addr + 1] << 8) |
               ((uint32_t)g_ParameterTable[addr + 2] << 16) |
               ((uint32_t)g_ParameterTable[addr + 3] << 24);
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write32
 *
 * @brief   寫入指定地址的參數(32位)
 *
 * @param   addr - 參數地址
 * @param   value - 參數值
 *
 * @return  none
 */
void Parameter_Write32(uint16_t addr, uint32_t value)
{
    if(addr < PARAM_TABLE_SIZE - 3)
    {
        g_ParameterTable[addr] = (uint8_t)(value & 0xFF);
        g_ParameterTable[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
        g_ParameterTable[addr + 2] = (uint8_t)((value >> 16) & 0xFF);
        g_ParameterTable[addr + 3] = (uint8_t)((value >> 24) & 0xFF);
    }
}

/*********************************************************************
 * @fn      Parameter_CheckSum
 *
 * @brief   計算參數校驗和
 *
 * @return  校驗結果 (0:正確, 非0:錯誤)
 */
uint8_t Parameter_CheckSum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    // 計算地址0~674的總和
    for(i = 0; i < 675; i++)
    {
        sum += g_ParameterTable[i];
    }
    
    // 檢查校驗和
    if((uint8_t)(sum & 0xFF) == g_ParameterTable[675] &&
       (uint8_t)((sum >> 8) & 0xFF) == g_ParameterTable[676])
    {
        return 0;  // 校驗正確
    }
    
    return 1;  // 校驗錯誤
}

/*********************************************************************
 * @fn      Parameter_UpdateCheckSum
 *
 * @brief   更新參數表校驗和
 *
 * @return  none
 */
static void Parameter_UpdateCheckSum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    // 計算地址0~674的總和
    for(i = 0; i < 675; i++)
    {
        sum += g_ParameterTable[i];
    }
    
    // 更新校驗和
    g_ParameterTable[675] = (uint8_t)(sum & 0xFF);         // SUM_L
    g_ParameterTable[676] = (uint8_t)((sum >> 8) & 0xFF);  // SUM_H
    
    // 更新CRC16 (實際實現需根據具體算法計算CRC16)
    g_ParameterTable[677] = 0;  // 暫時置為0，後續實現
}

/*********************************************************************
 * @fn      Parameter_SetDefaultValues
 *
 * @brief   設置參數表默認值
 *
 * @return  none
 */
void Parameter_SetDefaultValues(void)
{
    // 清空參數表
    memset(g_ParameterTable, 0, PARAM_TABLE_SIZE);
    
    // 1. 系統基本參數
    Parameter_Write(PARAM_LBT, DEFAULT_LBT);
    Parameter_Write(PARAM_OBT, DEFAULT_OBT);
    Parameter_Write(PARAM_FACTORY, DEFAULT_FACTORY);
    Parameter_Write(PARAM_MODEL_NO, DEFAULT_MODEL_NO);
    Parameter_Write(PARAM_FW_NO, DEFAULT_FW_NO);
    Parameter_Write16(PARAM_NOT, DEFAULT_NOT);
    Parameter_Write16(PARAM_CODE_TABLE_V, DEFAULT_CODE_TABLE_V);
    
    // 2. 時間設定參數
    Parameter_Write(PARAM_YEAR, DEFAULT_YEAR);
    Parameter_Write(PARAM_MONTH, DEFAULT_MONTH);
    Parameter_Write(PARAM_DATE, DEFAULT_DATE);
    Parameter_Write(PARAM_HOUR, DEFAULT_HOUR);
    Parameter_Write(PARAM_MINUTE, DEFAULT_MINUTE);
    Parameter_Write(PARAM_SECOND, DEFAULT_SECOND);
    
    // 3. 測量顯示參數
    Parameter_Write(PARAM_TLL, DEFAULT_TLL);
    Parameter_Write(PARAM_TLH, DEFAULT_TLH);
    Parameter_Write(PARAM_MGDL, DEFAULT_MGDL);
    Parameter_Write(PARAM_EVENT, DEFAULT_EVENT);
    Parameter_Write(PARAM_STRIP_TYPE, DEFAULT_STRIP_TYPE);
    
    // 4. 硬體校準參數
    Parameter_Write16(PARAM_EV_T3_TRG, DEFAULT_EV_T3_TRG);
    Parameter_Write(PARAM_EV_WORKING, DEFAULT_EV_WORKING);
    Parameter_Write(PARAM_EV_T3, DEFAULT_EV_T3);
    Parameter_Write(PARAM_DACO, DEFAULT_DACO);
    Parameter_Write(PARAM_DACDO, DEFAULT_DACDO);
    Parameter_Write(PARAM_CC211NoDone, DEFAULT_CC211NoDone);
    Parameter_Write(PARAM_CAL_TOL, DEFAULT_CAL_TOL);
    
    // 更新校驗和
    Parameter_UpdateCheckSum();
}

/*********************************************************************
 * @fn      Parameter_Save
 *
 * @brief   保存參數到Flash或EEPROM
 *
 * @return  none
 */
void Parameter_Save(void)
{
    // 更新校驗和
    Parameter_UpdateCheckSum();
    
    // TODO: 實現Flash或EEPROM寫入
    // 此處需要根據實際硬體添加保存參數表的代碼
    printf("Parameter saved\r\n");
}

/*********************************************************************
 * @fn      Parameter_Load
 *
 * @brief   從Flash或EEPROM載入參數
 *
 * @return  none
 */
void Parameter_Load(void)
{
    // TODO: 實現Flash或EEPROM讀取
    // 此處需要根據實際硬體添加讀取參數表的代碼
    
    // 如果無法讀取，則設置默認值
    Parameter_SetDefaultValues();
    printf("Parameter loaded\r\n");
}

/*********************************************************************
 * @fn      Parameter_Reset
 *
 * @brief   重置參數表為默認值
 *
 * @return  none
 */
void Parameter_Reset(void)
{
    Parameter_SetDefaultValues();
    Parameter_Save();
    printf("Parameter reset to default values\r\n");
}

/*********************************************************************
 * @fn      Parameter_GetStripTypeStr
 *
 * @brief   獲取試片類型的字符串描述
 *
 * @param   type - 試片類型
 *
 * @return  字符串描述
 */
const char* Parameter_GetStripTypeStr(uint8_t type)
{
    switch(type)
    {
        case STRIP_TYPE_GLV:
            return "GLV";
        case STRIP_TYPE_U:
            return "U";
        case STRIP_TYPE_C:
            return "C";
        case STRIP_TYPE_TG:
            return "TG";
        case STRIP_TYPE_GAV:
            return "GAV";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Parameter_PrintInfo
 *
 * @brief   打印參數表信息
 *
 * @return  none
 */
void Parameter_PrintInfo(void)
{
    printf("==== Parameter Table Info ====\r\n");
    
    // 1. 系統基本參數
    printf("LBT: %d (%.2fV)\r\n", Parameter_Read(PARAM_LBT), Parameter_Read(PARAM_LBT) * 0.05);
    printf("OBT: %d (%.2fV)\r\n", Parameter_Read(PARAM_OBT), Parameter_Read(PARAM_OBT) * 0.05);
    printf("Factory Mode: %d\r\n", Parameter_Read(PARAM_FACTORY));
    printf("Model NO: %d\r\n", Parameter_Read(PARAM_MODEL_NO));
    printf("FW Version: %d\r\n", Parameter_Read(PARAM_FW_NO));
    printf("Test Count: %d\r\n", Parameter_Read16(PARAM_NOT));
    printf("Code Table Version: %d\r\n", Parameter_Read16(PARAM_CODE_TABLE_V));
    
    // 2. 時間設定參數
    printf("Date: 20%02d-%02d-%02d\r\n", 
           Parameter_Read(PARAM_YEAR),
           Parameter_Read(PARAM_MONTH),
           Parameter_Read(PARAM_DATE));
    printf("Time: %02d:%02d:%02d\r\n", 
           Parameter_Read(PARAM_HOUR),
           Parameter_Read(PARAM_MINUTE),
           Parameter_Read(PARAM_SECOND));
    
    // 3. 測量顯示參數
    printf("Temperature Range: %d.0°C ~ %d.0°C\r\n", 
           Parameter_Read(PARAM_TLL),
           Parameter_Read(PARAM_TLH));
    
    const char* unit_str[3] = {"mmol/L", "mg/dL", "gm/dl"};
    uint8_t unit = Parameter_Read(PARAM_MGDL);
    if(unit > 2) unit = 0;
    
    printf("Measurement Unit: %s\r\n", unit_str[unit]);
    
    const char* event_str[3] = {"QC", "AC", "PC"};
    uint8_t event = Parameter_Read(PARAM_EVENT);
    if(event > 2) event = 0;
    
    printf("Event Type: %s\r\n", event_str[event]);
    printf("Strip Type: %s\r\n", Parameter_GetStripTypeStr(Parameter_Read(PARAM_STRIP_TYPE)));
    
    printf("============================\r\n");
}