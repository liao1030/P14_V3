/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/08
 * Description        : 多功能生化測試儀參數代碼表
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "param_table.h"
#include "debug.h"
#include "string.h"
#include "ch32v20x_flash.h"

/* 參數表儲存在RAM中 */
static uint8_t param_table[PARAM_TABLE_SIZE];

/* 參數表在Flash中的儲存地址 */
#define FLASH_PARAM_ADDR      0x08010000   // 使用適合的Flash地址

/* 私有函數聲明 */
static uint16_t CalculateChecksum(void);
static uint8_t CalculateCRC16(void);

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   初始化參數表，從Flash讀取或設定預設值。
 *
 * @return  None
 */
void PARAM_Init(void)
{
    /* 先從Flash中載入參數表 */
    PARAM_LoadFromFlash();
    
    /* 檢查參數表校驗和是否正確 */
    if(!PARAM_VerifyChecksum())
    {
        printf("Parameter table checksum error, loading defaults.\r\n");
        
        /* 校驗失敗，載入預設參數 */
        PARAM_SetDefault();
        
        /* 更新校驗和 */
        PARAM_UpdateChecksum();
        
        /* 保存到Flash中 */
        PARAM_SaveToFlash();
    }
    else
    {
        printf("Parameter table loaded successfully.\r\n");
    }
}

/*********************************************************************
 * @fn      PARAM_GetByte
 *
 * @brief   讀取一個位元組參數。
 *
 * @param   addr - 參數地址
 *
 * @return  參數值
 */
uint8_t PARAM_GetByte(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        return param_table[addr];
    }
    return 0;
}

/*********************************************************************
 * @fn      PARAM_SetByte
 *
 * @brief   設置一個位元組參數。
 *
 * @param   addr - 參數地址
 *          value - 參數值
 *
 * @return  None
 */
void PARAM_SetByte(uint16_t addr, uint8_t value)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        param_table[addr] = value;
    }
}

/*********************************************************************
 * @fn      PARAM_GetWord
 *
 * @brief   讀取一個字（2位元組）參數。
 *
 * @param   addr - 參數起始地址
 *
 * @return  參數值
 */
uint16_t PARAM_GetWord(uint16_t addr)
{
    uint16_t value = 0;
    
    if(addr + 1 < PARAM_TABLE_SIZE)
    {
        value = (uint16_t)param_table[addr] | ((uint16_t)param_table[addr + 1] << 8);
    }
    
    return value;
}

/*********************************************************************
 * @fn      PARAM_SetWord
 *
 * @brief   設置一個字（2位元組）參數。
 *
 * @param   addr - 參數起始地址
 *          value - 參數值
 *
 * @return  None
 */
void PARAM_SetWord(uint16_t addr, uint16_t value)
{
    if(addr + 1 < PARAM_TABLE_SIZE)
    {
        param_table[addr] = (uint8_t)(value & 0xFF);
        param_table[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/*********************************************************************
 * @fn      PARAM_GetFloat
 *
 * @brief   讀取一個浮點數（4位元組）參數。
 *
 * @param   addr - 參數起始地址
 *
 * @return  參數值
 */
float PARAM_GetFloat(uint16_t addr)
{
    float value = 0.0f;
    uint8_t *p = (uint8_t *)&value;
    
    if(addr + 3 < PARAM_TABLE_SIZE)
    {
        p[0] = param_table[addr];
        p[1] = param_table[addr + 1];
        p[2] = param_table[addr + 2];
        p[3] = param_table[addr + 3];
    }
    
    return value;
}

/*********************************************************************
 * @fn      PARAM_SetFloat
 *
 * @brief   設置一個浮點數（4位元組）參數。
 *
 * @param   addr - 參數起始地址
 *          value - 參數值
 *
 * @return  None
 */
void PARAM_SetFloat(uint16_t addr, float value)
{
    uint8_t *p = (uint8_t *)&value;
    
    if(addr + 3 < PARAM_TABLE_SIZE)
    {
        param_table[addr] = p[0];
        param_table[addr + 1] = p[1];
        param_table[addr + 2] = p[2];
        param_table[addr + 3] = p[3];
    }
}

/*********************************************************************
 * @fn      PARAM_GetString
 *
 * @brief   讀取字串參數。
 *
 * @param   addr - 參數起始地址
 *          buffer - 接收緩衝區
 *          length - 字串長度
 *
 * @return  None
 */
void PARAM_GetString(uint16_t addr, uint8_t *buffer, uint8_t length)
{
    if(addr + length <= PARAM_TABLE_SIZE && buffer != NULL)
    {
        memcpy(buffer, &param_table[addr], length);
    }
}

/*********************************************************************
 * @fn      PARAM_SetString
 *
 * @brief   設置字串參數。
 *
 * @param   addr - 參數起始地址
 *          buffer - 字串緩衝區
 *          length - 字串長度
 *
 * @return  None
 */
void PARAM_SetString(uint16_t addr, uint8_t *buffer, uint8_t length)
{
    if(addr + length <= PARAM_TABLE_SIZE && buffer != NULL)
    {
        memcpy(&param_table[addr], buffer, length);
    }
}

/*********************************************************************
 * @fn      PARAM_VerifyChecksum
 *
 * @brief   驗證參數表校驗和。
 *
 * @return  1: 校驗通過，0: 校驗失敗
 */
uint8_t PARAM_VerifyChecksum(void)
{
    uint16_t checksum = CalculateChecksum();
    uint16_t stored_checksum = (uint16_t)param_table[PARAM_SUM_L] | ((uint16_t)param_table[PARAM_SUM_H] << 8);
    
    return (checksum == stored_checksum) ? 1 : 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateChecksum
 *
 * @brief   更新參數表校驗和。
 *
 * @return  None
 */
void PARAM_UpdateChecksum(void)
{
    uint16_t checksum = CalculateChecksum();
    
    param_table[PARAM_SUM_L] = (uint8_t)(checksum & 0xFF);
    param_table[PARAM_SUM_H] = (uint8_t)((checksum >> 8) & 0xFF);
    
    /* 計算CRC16校驗 */
    param_table[PARAM_CRC16] = CalculateCRC16();
}

/*********************************************************************
 * @fn      CalculateChecksum
 *
 * @brief   計算參數表校驗和（0-674位址的內容總和）。
 *
 * @return  校驗和
 */
static uint16_t CalculateChecksum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    for(i = 0; i < PARAM_SUM_L; i++)
    {
        sum += param_table[i];
    }
    
    return sum;
}

/*********************************************************************
 * @fn      CalculateCRC16
 *
 * @brief   計算參數表CRC16校驗（包括校驗和在內的所有資料）。
 *          簡單實現，可根據需要更改為更複雜的CRC算法。
 *
 * @return  CRC16校驗值
 */
static uint8_t CalculateCRC16(void)
{
    uint8_t crc = 0;
    uint16_t i;
    
    for(i = 0; i < PARAM_CRC16; i++)
    {
        crc ^= param_table[i];
    }
    
    return crc;
}

/*********************************************************************
 * @fn      PARAM_SaveToFlash
 *
 * @brief   將參數表保存到Flash中。
 *
 * @return  None
 */
void PARAM_SaveToFlash(void)
{
    uint16_t i;
    uint32_t Address = FLASH_PARAM_ADDR;
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除參數儲存區 */
    FLASH_ErasePage(FLASH_PARAM_ADDR);
    
    /* 寫入參數表 */
    for(i = 0; i < PARAM_TABLE_SIZE; i += 2)
    {
        uint16_t data;
        
        if(i + 1 < PARAM_TABLE_SIZE)
        {
            data = ((uint16_t)param_table[i + 1] << 8) | param_table[i];
        }
        else
        {
            data = param_table[i];
        }
        
        FLASH_ProgramHalfWord(Address, data);
        Address += 2;
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
}

/*********************************************************************
 * @fn      PARAM_LoadFromFlash
 *
 * @brief   從Flash中載入參數表。
 *
 * @return  None
 */
void PARAM_LoadFromFlash(void)
{
    uint16_t i;
    uint32_t Address = FLASH_PARAM_ADDR;
    
    for(i = 0; i < PARAM_TABLE_SIZE; i += 2)
    {
        uint16_t data = *(volatile uint16_t*)Address;
        
        param_table[i] = (uint8_t)(data & 0xFF);
        
        if(i + 1 < PARAM_TABLE_SIZE)
        {
            param_table[i + 1] = (uint8_t)((data >> 8) & 0xFF);
        }
        
        Address += 2;
    }
}

/*********************************************************************
 * @fn      PARAM_SetDefault
 *
 * @brief   設置參數表預設值。
 *
 * @return  None
 */
void PARAM_SetDefault(void)
{
    /* 清空參數表 */
    memset(param_table, 0, PARAM_TABLE_SIZE);
    
    /* 系統基本參數預設值 */
    param_table[PARAM_LBT] = 58;        // 低電池閾值: 2.58V
    param_table[PARAM_OBT] = 55;        // 電池耗盡閾值: 2.55V
    param_table[PARAM_FACTORY] = 0;     // 使用者模式
    param_table[PARAM_MODEL_NO] = 1;    // 產品型號
    param_table[PARAM_FW_NO] = 10;      // 韌體版本 1.0
    PARAM_SetWord(PARAM_NOT, 0);        // 測試次數
    PARAM_SetWord(PARAM_CODE_TABLE_V, 1); // 代碼表版本 1
    
    /* 時間設定參數預設值（設置為當前編譯時間） */
    param_table[PARAM_YEAR] = 25;       // 2025年 (25)
    param_table[PARAM_MONTH] = 5;       // 5月
    param_table[PARAM_DATE] = 21;        // 21日
    param_table[PARAM_HOUR] = 12;       // 12時
    param_table[PARAM_MINUTE] = 0;      // 0分
    param_table[PARAM_SECOND] = 0;      // 0秒
    
    /* 測試條件參數預設值 */
    param_table[PARAM_TLL] = 10;        // 操作溫度下限: 10°C
    param_table[PARAM_TLH] = 40;        // 操作溫度上限: 40°C
    param_table[PARAM_MGDL] = UNIT_MG_DL; // 預設單位: mg/dL
    param_table[PARAM_EVENT] = EVENT_AC;   // 預設事件: 飯前
    param_table[PARAM_STRIP_TYPE] = STRIP_TYPE_GLV; // 預設試片: 血糖
    
    /* 硬體校準參數預設值 */
    PARAM_SetWord(PARAM_EV_T3_TRG, 120);   // EV_T3觸發電壓
    param_table[PARAM_EV_WORKING] = 164;   // 濃度工作電壓
    param_table[PARAM_EV_T3] = 164;        // 血液測濃電極電壓
    param_table[PARAM_DACO] = 0;           // DAC偏移補償
    
    /* 為各測試項目設置W電極PWM占空比預設值 */
    PARAM_SetWord(PARAM_BG_W_PWM_DUTY, 1000); // 血糖W電極PWM占空比預設值: 100.0%
    PARAM_SetWord(PARAM_U_W_PWM_DUTY, 750);  // 尿酸W電極PWM占空比預設值: 75.0%
    PARAM_SetWord(PARAM_C_W_PWM_DUTY, 250);  // 總膽固醇W電極PWM占空比預設值: 25.0%
    PARAM_SetWord(PARAM_TG_W_PWM_DUTY, 250); // 三酸甘油脂W電極PWM占空比預設值: 25.0%

    param_table[PARAM_DACDO] = 0;          // DAC校正管理參數
    param_table[PARAM_CC211NODONE] = 0;    // CC211正常完成
    param_table[PARAM_CAL_TOL] = 8;        // OPS/OPI校準容差: 0.8%
    PARAM_SetFloat(PARAM_OPS, 1.0);        // OPA校準斜率
    PARAM_SetFloat(PARAM_OPI, 0.0);        // OPA校準截距
    param_table[PARAM_QCT] = 0;            // QCT校準測試低位元組
    param_table[PARAM_TOFFSET] = 0;        // 溫度校準偏移
    param_table[PARAM_BOFFSET] = 0;        // 電池校準偏移
    
    /* 血糖(GLV/GAV)專用參數預設值 */
    param_table[PARAM_BG_CSU_TOL] = 10;    // 試片檢查容差: 3%
    PARAM_SetWord(PARAM_BG_NDL, 800);      // 新試片測濃水平
    PARAM_SetWord(PARAM_BG_UDL, 850);      // 已使用試片測濃水平
    PARAM_SetWord(PARAM_BG_BLOOD_IN, 900); // 血液測濃水平
    /* 試片批號預設為空白 */
    param_table[PARAM_BG_L] = 30;          // 血糖值下限: 3.0 mmol/L
    param_table[PARAM_BG_H] = 240;         // 血糖值上限: 24.0 mmol/L
    PARAM_SetWord(PARAM_BG_T3_E37, 1800);  // T3 ADV超E37閾值
    
    /* 尿酸(U)專用參數預設值 */
    param_table[PARAM_U_CSU_TOL] = 10;     // 試片檢查容差: 3%
    PARAM_SetWord(PARAM_U_NDL, 800);       // 新試片測濃水平
    PARAM_SetWord(PARAM_U_UDL, 850);       // 已使用試片測濃水平
    PARAM_SetWord(PARAM_U_BLOOD_IN, 900);  // 血液測濃水平
    /* 試片批號預設為空白 */
    param_table[PARAM_U_L] = 20;           // 尿酸值下限
    param_table[PARAM_U_H] = 130;          // 尿酸值上限
    PARAM_SetWord(PARAM_U_T3_E37, 1800);   // T3 ADV超E37閾值
    
    /* 總膽固醇(C)專用參數預設值 */
    param_table[PARAM_C_CSU_TOL] = 10;     // 試片檢查容差: 3%
    PARAM_SetWord(PARAM_C_NDL, 800);       // 新試片測濃水平
    PARAM_SetWord(PARAM_C_UDL, 850);       // 已使用試片測濃水平
    PARAM_SetWord(PARAM_C_BLOOD_IN, 900);  // 血液測濃水平
    /* 試片批號預設為空白 */
    param_table[PARAM_C_L] = 25;           // 總膽固醇值下限
    /* 使用PARAM_SetWord存儲超過uint8_t範圍的值 */
    PARAM_SetWord(PARAM_C_H, 400);         // 總膽固醇值上限設為400，使用2個位元組
    PARAM_SetWord(PARAM_C_T3_E37, 1800);   // T3 ADV超E37閾值
    
    /* 三酸甘油脂(TG)專用參數預設值 */
    param_table[PARAM_TG_CSU_TOL] = 10;    // 試片檢查容差: 3%
    PARAM_SetWord(PARAM_TG_NDL, 800);      // 新試片測濃水平
    PARAM_SetWord(PARAM_TG_UDL, 850);      // 已使用試片測濃水平
    PARAM_SetWord(PARAM_TG_BLOOD_IN, 900); // 血液測濃水平
    /* 試片批號預設為空白 */
    param_table[PARAM_TG_L] = 30;          // 三酸甘油脂值下限
    /* 使用PARAM_SetWord存儲超過uint8_t範圍的值 */
    PARAM_SetWord(PARAM_TG_H, 500);        // 三酸甘油脂值上限設為500，使用2個位元組
    PARAM_SetWord(PARAM_TG_T3_E37, 1800);  // T3 ADV超E37閾值
    
    /* 保留區域保持為0 */
    
    /* 校驗區將由 PARAM_UpdateChecksum() 自動計算 */
}

/*********************************************************************
 * @fn      StripType_GetName
 *
 * @brief   取得試片類型名稱。
 *
 * @param   type - 試片類型
 *
 * @return  試片類型名稱
 */
const char* StripType_GetName(StripType_TypeDef type)
{
    switch(type)
    {
        case STRIP_TYPE_GLV:
            return "Blood Glucose";
        case STRIP_TYPE_U:
            return "Uric Acid";
        case STRIP_TYPE_C:
            return "Cholesterol";
        case STRIP_TYPE_TG:
            return "Triglycerides";
        case STRIP_TYPE_GAV:
            return "Blood Glucose (GAV)";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Unit_GetSymbol
 *
 * @brief   取得濃度單位符號。
 *
 * @param   unit - 濃度單位
 *
 * @return  濃度單位符號
 */
const char* Unit_GetSymbol(Unit_TypeDef unit)
{
    switch(unit)
    {
        case UNIT_MMOL_L:
            return "mmol/L";
        case UNIT_MG_DL:
            return "mg/dL";
        case UNIT_GM_DL:
            return "gm/dl";
        default:
            return "?";
    }
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   獲取參數表中儲存的日期時間。
 *
 * @param   year, month, date, hour, minute, second - 日期時間變數的指針
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                         uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    if(year == NULL || month == NULL || date == NULL || 
       hour == NULL || minute == NULL || second == NULL)
    {
        return 0;
    }
    
    *year = param_table[PARAM_YEAR];
    *month = param_table[PARAM_MONTH];
    *date = param_table[PARAM_DATE];
    *hour = param_table[PARAM_HOUR];
    *minute = param_table[PARAM_MINUTE];
    *second = param_table[PARAM_SECOND];
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_SetDateTime
 *
 * @brief   設置參數表中的日期時間。
 *
 * @param   year, month, date, hour, minute, second - 日期時間值
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date, 
                         uint8_t hour, uint8_t minute, uint8_t second)
{
    /* 檢查日期時間的有效性 */
    if(year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
       hour > 23 || minute > 59 || second > 59)
    {
        return 0;
    }
    
    param_table[PARAM_YEAR] = year;
    param_table[PARAM_MONTH] = month;
    param_table[PARAM_DATE] = date;
    param_table[PARAM_HOUR] = hour;
    param_table[PARAM_MINUTE] = minute;
    param_table[PARAM_SECOND] = second;
    
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    /* 將更新後的參數表保存到Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_IncreaseTestCount
 *
 * @brief   增加測試計數器。
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_IncreaseTestCount(void)
{
    uint16_t count = PARAM_GetWord(PARAM_NOT);
    count++;
    PARAM_SetWord(PARAM_NOT, count);
    
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    /* 將更新後的參數表保存到Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetTestCount
 *
 * @brief   獲取測試計數器值。
 *
 * @return  測試計數器值
 */
uint16_t PARAM_GetTestCount(void)
{
    return PARAM_GetWord(PARAM_NOT);
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   讀取一個參數區塊。
 *
 * @param   block - 區塊類型
 *          data - 數據緩衝區
 *          size - 數據大小
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_ReadBlock(BlockType_TypeDef block, void *data, uint16_t size)
{
    uint16_t start_addr = 0;
    uint16_t block_size = 0;
    
    /* 參數檢查 */
    if(data == NULL || block >= BLOCK_MAX)
    {
        return 0;
    }
    
    /* 確定區塊的起始地址和大小 */
    switch(block)
    {
        case BLOCK_BASIC_SYSTEM:
            start_addr = 0;
            block_size = 20;
            break;
        case BLOCK_HARDWARE_CALIB:
            start_addr = 20;
            block_size = 19;
            break;
        case BLOCK_BG_PARAMS:
            start_addr = 39;
            block_size = 174;
            break;
        case BLOCK_U_PARAMS:
            start_addr = 213;
            block_size = 142;
            break;
        case BLOCK_C_PARAMS:
            start_addr = 355;
            block_size = 142;
            break;
        case BLOCK_TG_PARAMS:
            start_addr = 497;
            block_size = 142;
            break;
        case BLOCK_RESERVED:
            start_addr = 639;
            block_size = 39;
            break;
        default:
            return 0;
    }
    
    /* 檢查數據大小是否超過區塊大小 */
    if(size > block_size)
    {
        size = block_size;
    }
    
    /* 複製參數數據 */
    memcpy(data, &param_table[start_addr], size);
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_UpdateBlock
 *
 * @brief   更新一個參數區塊。
 *
 * @param   block - 區塊類型
 *          data - 數據緩衝區
 *          size - 數據大小
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_UpdateBlock(BlockType_TypeDef block, void *data, uint16_t size)
{
    uint16_t start_addr = 0;
    uint16_t block_size = 0;
    
    /* 參數檢查 */
    if(data == NULL || block >= BLOCK_MAX)
    {
        return 0;
    }
    
    /* 確定區塊的起始地址和大小 */
    switch(block)
    {
        case BLOCK_BASIC_SYSTEM:
            start_addr = 0;
            block_size = 20;
            break;
        case BLOCK_HARDWARE_CALIB:
            start_addr = 20;
            block_size = 19;
            break;
        case BLOCK_BG_PARAMS:
            start_addr = 39;
            block_size = 174;
            break;
        case BLOCK_U_PARAMS:
            start_addr = 213;
            block_size = 142;
            break;
        case BLOCK_C_PARAMS:
            start_addr = 355;
            block_size = 142;
            break;
        case BLOCK_TG_PARAMS:
            start_addr = 497;
            block_size = 142;
            break;
        case BLOCK_RESERVED:
            start_addr = 639;
            block_size = 39;
            break;
        default:
            return 0;
    }
    
    /* 檢查數據大小是否超過區塊大小 */
    if(size > block_size)
    {
        size = block_size;
    }
    
    /* 更新參數數據 */
    memcpy(&param_table[start_addr], data, size);
    
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    /* 將更新後的參數表保存到Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetStripParameters
 *
 * @brief   依據試片類型獲取試片參數。
 *
 * @param   type - 試片類型
 *          ndl - 新試片檢測水平指針
 *          udl - 已使用試片檢測水平指針
 *          bloodIn - 血液檢測水平指針
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_GetStripParameters(StripType_TypeDef type, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    uint16_t ndl_addr = 0;
    uint16_t udl_addr = 0;
    uint16_t bloodIn_addr = 0;
    
    /* 參數檢查 */
    if(ndl == NULL || udl == NULL || bloodIn == NULL || type >= STRIP_TYPE_MAX)
    {
        return 0;
    }
    
    /* 根據試片類型確定參數地址 */
    switch(type)
    {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            ndl_addr = PARAM_BG_NDL;
            udl_addr = PARAM_BG_UDL;
            bloodIn_addr = PARAM_BG_BLOOD_IN;
            break;
        case STRIP_TYPE_U:
            ndl_addr = PARAM_U_NDL;
            udl_addr = PARAM_U_UDL;
            bloodIn_addr = PARAM_U_BLOOD_IN;
            break;
        case STRIP_TYPE_C:
            ndl_addr = PARAM_C_NDL;
            udl_addr = PARAM_C_UDL;
            bloodIn_addr = PARAM_C_BLOOD_IN;
            break;
        case STRIP_TYPE_TG:
            ndl_addr = PARAM_TG_NDL;
            udl_addr = PARAM_TG_UDL;
            bloodIn_addr = PARAM_TG_BLOOD_IN;
            break;
        default:
            return 0;
    }
    
    /* 讀取參數值 */
    *ndl = PARAM_GetWord(ndl_addr);
    *udl = PARAM_GetWord(udl_addr);
    *bloodIn = PARAM_GetWord(bloodIn_addr);
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetTimingParameters
 *
 * @brief   依據試片類型獲取測試時序參數。
 *
 * @param   type - 試片類型
 *          tpl - 測試間格指針
 *          trd - 讀取延遲指針
 *          evWidth - 工作電壓寬度指針
 *          phase - 時序階段(1或2)
 *
 * @return  成功返回1，失敗返回0
 */
uint8_t PARAM_GetTimingParameters(StripType_TypeDef type, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t phase)
{
    uint16_t tpl_addr = 0;
    uint16_t trd_addr = 0;
    uint16_t evWidth_addr = 0;
    
    /* 參數檢查 */
    if(tpl == NULL || trd == NULL || evWidth == NULL || 
       type >= STRIP_TYPE_MAX || (phase != 1 && phase != 2))
    {
        return 0;
    }
    
    /* 根據試片類型和時序階段確定參數地址 */
    switch(type)
    {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            if(phase == 1)
            {
                tpl_addr = PARAM_BG_TPL1;
                trd_addr = PARAM_BG_TRD1;
                evWidth_addr = PARAM_BG_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_BG_TPL2;
                trd_addr = PARAM_BG_TRD2;
                evWidth_addr = PARAM_BG_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_U:
            if(phase == 1)
            {
                tpl_addr = PARAM_U_TPL1;
                trd_addr = PARAM_U_TRD1;
                evWidth_addr = PARAM_U_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_U_TPL2;
                trd_addr = PARAM_U_TRD2;
                evWidth_addr = PARAM_U_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_C:
            if(phase == 1)
            {
                tpl_addr = PARAM_C_TPL1;
                trd_addr = PARAM_C_TRD1;
                evWidth_addr = PARAM_C_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_C_TPL2;
                trd_addr = PARAM_C_TRD2;
                evWidth_addr = PARAM_C_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_TG:
            if(phase == 1)
            {
                tpl_addr = PARAM_TG_TPL1;
                trd_addr = PARAM_TG_TRD1;
                evWidth_addr = PARAM_TG_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_TG_TPL2;
                trd_addr = PARAM_TG_TRD2;
                evWidth_addr = PARAM_TG_EVWIDTH2;
            }
            break;
        default:
            return 0;
    }
    
    /* 讀取參數值 */
    *tpl = PARAM_GetWord(tpl_addr);
    *trd = PARAM_GetWord(trd_addr);
    *evWidth = PARAM_GetWord(evWidth_addr);
    
    return 1;
}