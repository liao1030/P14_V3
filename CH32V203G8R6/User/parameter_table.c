/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_table.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : 多功能生化測試儀參數代碼表實現
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "parameter_table.h"
#include "debug.h"
#include <string.h>

/* 參數表實例 */
static ParameterTable_TypeDef paramTable;

/* 默認參數值 */
static const uint8_t defaultParamTable[PARAM_TABLE_SIZE] = {
    /* 系統基本參數 */
    58,     // PARAM_LBT: 低電池閾值 (2.58V)
    55,     // PARAM_OBT: 電池耗盡閾值 (2.55V)
    0,      // PARAM_FACTORY: 使用者模式
    1,      // PARAM_MODEL_NO: 產品型號 (由廠商修改)
    1,      // PARAM_FW_NO: 韌體版本號 (由廠商修改)
    0, 0,   // PARAM_NOT: 測試次數初始為0
    1, 0,   // PARAM_CODE_TABLE_V: 代碼表版本號 (1.0)
    
    /* 時間設定參數 */
    24,     // PARAM_YEAR: 年份設定 (2024年)
    7,      // PARAM_MONTH: 月份設定 (7月)
    23,     // PARAM_DATE: 日期設定 (23日)
    12,     // PARAM_HOUR: 小時設定 (12時)
    0,      // PARAM_MINUTE: 分鐘設定 (0分)
    0,      // PARAM_SECOND: 秒數設定 (0秒)
    
    /* 測試環境參數 */
    1,      // PARAM_TLL: 操作溫度範圍下限 (1.0°C)
    40,     // PARAM_TLH: 操作溫度範圍上限 (40.0°C)
    0,      // PARAM_MGDL: 濃度單位設定 (mmol/L)
    0,      // PARAM_EVENT: 餐飲事件類型 (QC)
    0,      // PARAM_STRIP_TYPE: 測試項目 (血糖)
    
    /* 硬體校正參數 */
    40, 1,  // PARAM_EV_T3_TRG: EV_T3觸發電壓 (296 = 1.2V)
    164,    // PARAM_EV_WORKING: 濃度工作電流電壓
    164,    // PARAM_EV_T3: 血液消耗電流電壓
    0,      // PARAM_DACO: DAC偏移補償
    0,      // PARAM_DACDO: DAC校正管理參數
    0,      // PARAM_CC211NoDone: CC211未完成旗標
    8,      // PARAM_CAL_TOL: OPS/OPI校正容差 (0.8%)
    /* PARAM_OPS: OPA校正斜率 (4 bytes) */
    0, 0, 0, 0,
    /* PARAM_OPI: OPA校正截距 (4 bytes) */
    0, 0, 0, 0,
    0,      // PARAM_QCT: QCT校正測試低位元組
    0,      // PARAM_TOFFSET: 溫度校正偏移
    0,      // PARAM_BOFFSET: 電池校正偏移
};

/* 靜態函數聲明 */
static uint16_t PARAM_CalculateChecksum(void);
static uint16_t PARAM_Calculate_CRC16(const uint8_t *data, uint16_t size);

/**
 * @brief 初始化參數表
 */
void PARAM_Init(void)
{
    /* 初始化為默認值 */
    memcpy(paramTable.data, defaultParamTable, PARAM_TABLE_SIZE);
    
    /* 從Flash或EEPROM載入參數，載入失敗則使用默認參數 */
    if (!PARAM_Load()) {
        PARAM_Reset();
        PARAM_Save();
    }
    
    printf("參數代碼表初始化完成，版本: %d.%d\r\n", 
           paramTable.data[PARAM_CODE_TABLE_V], 
           paramTable.data[PARAM_CODE_TABLE_V + 1]);
}

/**
 * @brief 讀取8位元參數
 * @param addr: 參數地址
 * @return 參數值
 */
uint8_t PARAM_Read(uint16_t addr)
{
    if (addr < PARAM_TABLE_SIZE) {
        return paramTable.data[addr];
    }
    return 0;
}

/**
 * @brief 寫入8位元參數
 * @param addr: 參數地址
 * @param value: 參數值
 */
void PARAM_Write(uint16_t addr, uint8_t value)
{
    if (addr < PARAM_TABLE_SIZE) {
        paramTable.data[addr] = value;
    }
}

/**
 * @brief 讀取16位元參數 (小端格式)
 * @param addr: 參數地址
 * @return 參數值
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
 * @brief 寫入16位元參數 (小端格式)
 * @param addr: 參數地址
 * @param value: 參數值
 */
void PARAM_Write16(uint16_t addr, uint16_t value)
{
    if (addr < PARAM_TABLE_SIZE - 1) {
        paramTable.data[addr] = (uint8_t)(value & 0xFF);
        paramTable.data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/**
 * @brief 讀取參數塊
 * @param addr: 起始地址
 * @param buffer: 緩衝區
 * @param size: 大小
 */
void PARAM_ReadBlock(uint16_t addr, uint8_t *buffer, uint16_t size)
{
    if (addr + size <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(buffer, &paramTable.data[addr], size);
    }
}

/**
 * @brief 寫入參數塊
 * @param addr: 起始地址
 * @param buffer: 緩衝區
 * @param size: 大小
 */
void PARAM_WriteBlock(uint16_t addr, const uint8_t *buffer, uint16_t size)
{
    if (addr + size <= PARAM_TABLE_SIZE && buffer != NULL) {
        memcpy(&paramTable.data[addr], buffer, size);
    }
}

/**
 * @brief 重置參數表為默認值
 */
void PARAM_Reset(void)
{
    /* 複製默認參數值 */
    memcpy(paramTable.data, defaultParamTable, PARAM_TABLE_SIZE);
    
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    printf("參數表已重置為默認值\r\n");
}

/**
 * @brief 計算參數表校驗和
 * @return 校驗和
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
 * @brief 更新參數表校驗和
 */
void PARAM_UpdateChecksum(void)
{
    uint16_t checksum = PARAM_CalculateChecksum();
    paramTable.data[PARAM_SUM_L] = checksum & 0xFF;
    paramTable.data[PARAM_SUM_H] = (checksum >> 8) & 0xFF;
    
    /* 計算CRC16 */
    uint16_t crc = PARAM_Calculate_CRC16(paramTable.data, PARAM_CRC16);
    paramTable.data[PARAM_CRC16] = crc & 0xFF;
}

/**
 * @brief 驗證參數表校驗和
 * @return 1:校驗成功 0:校驗失敗
 */
uint8_t PARAM_VerifyChecksum(void)
{
    uint16_t calculatedChecksum = PARAM_CalculateChecksum();
    uint16_t storedChecksum = (uint16_t)paramTable.data[PARAM_SUM_L] | 
                             ((uint16_t)paramTable.data[PARAM_SUM_H] << 8);
    
    /* 驗證校驗和 */
    if (calculatedChecksum != storedChecksum) {
        printf("參數表校驗和錯誤: 計算值=%04X, 存儲值=%04X\r\n", 
               calculatedChecksum, storedChecksum);
        return 0;
    }
    
    /* 驗證CRC16 */
    uint16_t calculatedCRC = PARAM_Calculate_CRC16(paramTable.data, PARAM_CRC16);
    uint8_t storedCRC = paramTable.data[PARAM_CRC16];
    
    if ((calculatedCRC & 0xFF) != storedCRC) {
        printf("參數表CRC校驗錯誤: 計算值=%02X, 存儲值=%02X\r\n", 
               calculatedCRC & 0xFF, storedCRC);
        return 0;
    }
    
    return 1;
}

/**
 * @brief 計算CRC16校驗碼
 * @param data: 數據指針
 * @param size: 數據大小
 * @return CRC16校驗碼
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
 * @brief 保存參數表到Flash或EEPROM
 * @note 此函數需根據實際硬體實現
 */
void PARAM_Save(void)
{
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    /* 保存到EEPROM或Flash，根據實際硬體實現 */
    printf("參數表已保存\r\n");
}

/**
 * @brief 從Flash或EEPROM載入參數表
 * @return 1:載入成功 0:載入失敗
 * @note 此函數需根據實際硬體實現
 */
uint8_t PARAM_Load(void)
{
    /* 從EEPROM或Flash加載，根據實際硬體實現 */
    /* 為了示例，這裡直接使用默認值 */
    
    /* 驗證參數表有效性 */
    if (!PARAM_VerifyChecksum()) {
        printf("參數表載入失敗，使用默認值\r\n");
        return 0;
    }
    
    printf("參數表載入成功\r\n");
    return 1;
} 