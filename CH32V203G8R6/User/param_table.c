/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數表實現
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_table.h"
#include "ch32v20x_flash.h"
#include <string.h>

/* 全局參數表 */
static ParameterTable g_paramTable;
ParameterTable *g_pParamTable = &g_paramTable;

/* 參數表初始化狀態 */
static uint8_t g_paramInitialized = 0;

/* 內部函數聲明 */
static ParamError_TypeDef PARAM_VerifyChecksum(void);
static ParamError_TypeDef PARAM_VerifyCRC16(void);
static void PARAM_UpdateChecksum(void);
static void PARAM_UpdateCRC16(void);
static ParamError_TypeDef PARAM_EraseArea(uint32_t startAddress);

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   初始化參數表
 *
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_Init(void)
{
    ParamError_TypeDef result;
    
    /* 首先嘗試從主參數區讀取 */
    result = PARAM_LoadFromFlash(PARAM_AREA_MAIN);
    
    /* 如果主參數區讀取失敗，嘗試從備份參數區讀取 */
    if (result != PARAM_OK) {
        result = PARAM_LoadFromFlash(PARAM_AREA_BACKUP);
        
        /* 如果備份區也讀取失敗，載入預設參數 */
        if (result != PARAM_OK) {
            result = PARAM_LoadDefaults();
            
            /* 將預設參數寫入兩個參數區 */
            if (result == PARAM_OK) {
                PARAM_SaveToFlash(PARAM_AREA_MAIN);
                PARAM_SaveToFlash(PARAM_AREA_BACKUP);
            }
        } else {
            /* 備份區讀取成功，恢復到主參數區 */
            PARAM_SaveToFlash(PARAM_AREA_MAIN);
        }
    } else {
        /* 主參數區讀取成功，更新備份區 */
        PARAM_SaveToFlash(PARAM_AREA_BACKUP);
    }
    
    /* 標記初始化完成 */
    g_paramInitialized = (result == PARAM_OK) ? 1 : 0;
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_LoadFromFlash
 *
 * @brief   從指定參數區域載入參數表
 *
 * @param   area - 參數區域標識 (主區或備份區)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_LoadFromFlash(ParamArea_TypeDef area)
{
    uint32_t areaAddress;
    
    /* 根據區域參數選擇地址 */
    if (area == PARAM_AREA_MAIN) {
        areaAddress = PARAM_AREA_A_ADDR;
    } else if (area == PARAM_AREA_BACKUP) {
        areaAddress = PARAM_AREA_B_ADDR;
    } else {
        return PARAM_ERR_INVALID_AREA;
    }
    
    /* 從Flash讀取整個參數表 */
    memcpy(&g_paramTable, (void*)areaAddress, sizeof(ParameterTable));
    
    /* 檢查魔術數字是否正確 */
    if (g_paramTable.header.magic != PARAM_MAGIC) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 檢查校驗和 */
    if (PARAM_VerifyChecksum() != PARAM_OK) {
        return PARAM_ERR_CRC;
    }
    
    /* 檢查CRC16 */
    if (PARAM_VerifyCRC16() != PARAM_OK) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_SaveToFlash
 *
 * @brief   將參數表保存到指定區域
 *
 * @param   area - 參數區域標識 (主區或備份區)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SaveToFlash(ParamArea_TypeDef area)
{
    uint32_t areaAddress;
    FLASH_Status status;
    uint32_t i;
    uint16_t *pData;
    
    /* 根據區域參數選擇地址 */
    if (area == PARAM_AREA_MAIN) {
        areaAddress = PARAM_AREA_A_ADDR;
    } else if (area == PARAM_AREA_BACKUP) {
        areaAddress = PARAM_AREA_B_ADDR;
    } else {
        return PARAM_ERR_INVALID_AREA;
    }
    
    /* 保存前更新參數表頭部信息 */
    g_paramTable.header.writeCounter++;
    g_paramTable.header.timestamp = ((uint32_t)g_paramTable.basic.year << 24) | 
                                    ((uint32_t)g_paramTable.basic.month << 16) | 
                                    ((uint32_t)g_paramTable.basic.date << 8) | 
                                    g_paramTable.basic.hour;
    
    /* 更新校驗和 */
    PARAM_UpdateChecksum();
    
    /* 更新CRC16 */
    PARAM_UpdateCRC16();
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除目標區域 */
    if (PARAM_EraseArea(areaAddress) != PARAM_OK) {
        FLASH_Lock();
        return PARAM_ERR_FLASH_WRITE;
    }
    
    /* 按半字(2字節)寫入參數表數據 */
    pData = (uint16_t*)&g_paramTable;
    for (i = 0; i < sizeof(ParameterTable) / 2; i++) {
        status = FLASH_ProgramHalfWord(areaAddress + i * 2, pData[i]);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    /* 如果大小是奇數，處理最後一個字節 */
    if (sizeof(ParameterTable) % 2 != 0) {
        uint16_t lastWord = 0xFF00 | ((uint8_t*)&g_paramTable)[sizeof(ParameterTable) - 1];
        status = FLASH_ProgramHalfWord(areaAddress + i * 2, lastWord);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_LoadDefaults
 *
 * @brief   載入參數表預設值
 *
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_LoadDefaults(void)
{
    /* 清空整個參數表結構 */
    memset(&g_paramTable, 0, sizeof(ParameterTable));
    
    /* 設置參數表頭部 */
    g_paramTable.header.magic = PARAM_MAGIC;
    g_paramTable.header.version = PARAM_VERSION;
    g_paramTable.header.writeCounter = 0;
    g_paramTable.header.timestamp = 0;
    
    /* 設置基本系統參數預設值 */
    g_paramTable.basic.lbt = 58;              // 2.58V 低電池閾值
    g_paramTable.basic.obt = 55;              // 2.55V 電池耗盡閾值
    g_paramTable.basic.factory = 0;           // 使用者模式
    g_paramTable.basic.modelNo = 1;           // 產品型號
    g_paramTable.basic.fwNo = 1;              // 韌體版本號
    g_paramTable.basic.testCount = 0;         // 測試次數
    g_paramTable.basic.codeTableVer = 1;      // 代碼表版本
    
    /* 時間設定預設值 */
    g_paramTable.basic.year = 25;             // 2025年
    g_paramTable.basic.month = 5;             // 5月
    g_paramTable.basic.date = 19;             // 19日
    g_paramTable.basic.hour = 12;             // 12時
    g_paramTable.basic.minute = 0;            // 0分
    g_paramTable.basic.second = 0;            // 0秒
    
    /* 測試環境預設值 */
    g_paramTable.basic.tempLowLimit = 10;     // 10°C
    g_paramTable.basic.tempHighLimit = 40;    // 40°C
    g_paramTable.basic.measureUnit = 0;       // mmol/L
    g_paramTable.basic.defaultEvent = 1;      // AC (餐前)
    g_paramTable.basic.stripType = 0;         // GLV血糖
    
    /* 硬體校準參數預設值 */
    g_paramTable.hardware.evT3Trigger = 80;   // 觸發電壓
    g_paramTable.hardware.evWorking = 164;    // 工作電壓
    g_paramTable.hardware.evT3 = 164;         // 血液檢測電壓
    g_paramTable.hardware.calTolerance = 8;   // 校準容差
    g_paramTable.hardware.ops = 1.0f;         // OPA校準斜率
    g_paramTable.hardware.opi = 0.0f;         // OPA校準截距
    g_paramTable.hardware.qct = 0;            // QCT校準測試
    g_paramTable.hardware.tempOffset = 0;     // 溫度校準偏移
    g_paramTable.hardware.batteryOffset = 0;  // 電池校準偏移
    
    /* 各種測試項目的預設值（簡化，實際應用中需更詳細設定） */
    /* 血糖(GLV)參數預設值 */
    g_paramTable.glv.glvCsuTolerance = 10;    // 試片檢查容差
    g_paramTable.glv.glvL = 20;               // 下限 2.0 mmol/L
    g_paramTable.glv.glvH = 74;               // 上限 33.0 mmol/L (限制為uint8_t最大值)
    g_paramTable.glv.glvCountDownTime = 5;    // 5秒倒數
    
    /* 尿酸(U)參數預設值 */
    g_paramTable.u.uCsuTolerance = 10;        // 試片檢查容差
    g_paramTable.u.uL = 90;                   // 下限
    g_paramTable.u.uH = 200;                  // 上限 (限制為uint8_t最大值)
    g_paramTable.u.uCountDownTime = 20;       // 20秒倒數
    
    /* 總膽固醇(C)參數預設值 */
    g_paramTable.c.cCsuTolerance = 10;        // 試片檢查容差
    g_paramTable.c.cL = 100;                  // 下限
    g_paramTable.c.cH = 200;                  // 上限 (限制為uint8_t最大值)
    g_paramTable.c.cCountDownTime = 150;      // 150秒倒數
    
    /* 三酸甘油脂(TG)參數預設值 */
    g_paramTable.tg.tgCsuTolerance = 10;      // 試片檢查容差
    g_paramTable.tg.tgL = 70;                 // 下限
    g_paramTable.tg.tgH = 600;                // 上限
    g_paramTable.tg.tgCountDownTime = 180;    // 180秒倒數
    
    /* 血糖(GAV)參數預設值 */
    g_paramTable.gav.gavCsuTolerance = 10;    // 試片檢查容差
    g_paramTable.gav.gavL = 20;               // 下限 2.0 mmol/L
    g_paramTable.gav.gavH = 330;              // 上限 33.0 mmol/L
    g_paramTable.gav.gavT3E37 = 200;          // T3 ADV錯誤37閾值
    g_paramTable.gav.gavCountDownTime = 5;    // 5秒倒數
    
    /* 更新校驗和和CRC */
    PARAM_UpdateChecksum();
    PARAM_UpdateCRC16();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_GetValue
 *
 * @brief   獲取指定地址的參數值
 *
 * @param   paramAddress - 參數地址 (對應於參數代碼表中的地址)
 * @param   pValue - 參數值的指標
 * @param   size - 參數大小 (字節)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_GetValue(uint16_t paramAddress, void *pValue, uint8_t size)
{
    uint8_t *pParamData;
    
    /* 檢查參數表是否已初始化 */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* 檢查參數地址是否有效 */
    if (paramAddress >= 848) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 指向參數地址在g_paramTable中的位置 */
    pParamData = (uint8_t*)&g_paramTable;
    pParamData += sizeof(ParameterHeader); // 跳過頭部
    pParamData += paramAddress;
    
    /* 複製參數值 */
    memcpy(pValue, pParamData, size);
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_SetValue
 *
 * @brief   設置指定地址的參數值
 *
 * @param   paramAddress - 參數地址 (對應於參數代碼表中的地址)
 * @param   pValue - 參數值的指標
 * @param   size - 參數大小 (字節)
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_SetValue(uint16_t paramAddress, void *pValue, uint8_t size)
{
    uint8_t *pParamData;
    
    /* 檢查參數表是否已初始化 */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* 檢查參數地址是否有效 */
    if (paramAddress >= 848) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* 指向參數地址在g_paramTable中的位置 */
    pParamData = (uint8_t*)&g_paramTable;
    pParamData += sizeof(ParameterHeader); // 跳過頭部
    pParamData += paramAddress;
    
    /* 複製參數值 */
    memcpy(pParamData, pValue, size);
    
    /* 更新校驗和和CRC */
    PARAM_UpdateChecksum();
    PARAM_UpdateCRC16();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_Backup
 *
 * @brief   備份參數表到備份區域
 *
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_Backup(void)
{
    /* 檢查參數表是否已初始化 */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* 將當前參數表保存到備份區 */
    return PARAM_SaveToFlash(PARAM_AREA_BACKUP);
}

/*********************************************************************
 * @fn      PARAM_Restore
 *
 * @brief   從備份區域恢復參數表
 *
 * @return  ParamError_TypeDef - 操作結果
 */
ParamError_TypeDef PARAM_Restore(void)
{
    ParamError_TypeDef result;
    
    /* 從備份區載入參數 */
    result = PARAM_LoadFromFlash(PARAM_AREA_BACKUP);
    if (result != PARAM_OK) {
        return result;
    }
    
    /* 將恢復的參數保存到主區域 */
    return PARAM_SaveToFlash(PARAM_AREA_MAIN);
}

/*********************************************************************
 * @fn      PARAM_IsInitialized
 *
 * @brief   檢查參數表是否已初始化
 *
 * @return  uint8_t - 0: 未初始化, 1: 已初始化
 */
uint8_t PARAM_IsInitialized(void)
{
    return g_paramInitialized;
}

/*********************************************************************
 * @fn      PARAM_CalculateChecksum
 *
 * @brief   計算參數表校驗和
 *
 * @return  uint16_t - 計算出的校驗和
 */
uint16_t PARAM_CalculateChecksum(void)
{
    uint32_t sum = 0;
    uint16_t i;
    uint8_t *pData = (uint8_t*)&g_paramTable;
    
    /* 計算地址0~844的總和 (不包括校驗和和CRC) */
    for (i = 0; i < sizeof(ParameterTable) - 3; i++) {
        sum += pData[i];
    }
    
    return (uint16_t)(sum & 0xFFFF);
}

/*********************************************************************
 * @fn      PARAM_VerifyChecksum
 *
 * @brief   驗證參數表校驗和
 *
 * @return  ParamError_TypeDef - 驗證結果
 */
static ParamError_TypeDef PARAM_VerifyChecksum(void)
{
    uint16_t calculatedChecksum = PARAM_CalculateChecksum();
    
    if (calculatedChecksum != g_paramTable.checksum) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_UpdateChecksum
 *
 * @brief   更新參數表校驗和
 *
 * @return  none
 */
static void PARAM_UpdateChecksum(void)
{
    g_paramTable.checksum = PARAM_CalculateChecksum();
}

/*********************************************************************
 * @fn      PARAM_CalculateCRC16
 *
 * @brief   計算參數表CRC16
 *
 * @return  uint8_t - 計算出的CRC16
 */
uint8_t PARAM_CalculateCRC16(void)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    uint8_t *pData = (uint8_t*)&g_paramTable;
    
    /* 計算整個參數表的CRC16 (不包括CRC16自身) */
    for (i = 0; i < sizeof(ParameterTable) - 1; i++) {
        crc ^= pData[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 0xA001是Modbus CRC多項式的反轉值
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return (uint8_t)(crc & 0xFF);
}

/*********************************************************************
 * @fn      PARAM_VerifyCRC16
 *
 * @brief   驗證參數表CRC16
 *
 * @return  ParamError_TypeDef - 驗證結果
 */
static ParamError_TypeDef PARAM_VerifyCRC16(void)
{
    uint8_t calculatedCRC = PARAM_CalculateCRC16();
    
    if (calculatedCRC != g_paramTable.crc16) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_UpdateCRC16
 *
 * @brief   更新參數表CRC16
 *
 * @return  none
 */
static void PARAM_UpdateCRC16(void)
{
    g_paramTable.crc16 = PARAM_CalculateCRC16();
}

/*********************************************************************
 * @fn      PARAM_EraseArea
 *
 * @brief   擦除參數區域
 *
 * @param   startAddress - 起始地址
 * @return  ParamError_TypeDef - 操作結果
 */
static ParamError_TypeDef PARAM_EraseArea(uint32_t startAddress)
{
    uint32_t pageAddress;
    FLASH_Status status;
    uint8_t pageCount = (sizeof(ParameterTable) + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    uint8_t i;
    
    /* 按頁擦除 */
    for (i = 0; i < pageCount; i++) {
        pageAddress = startAddress + i * FLASH_PAGE_SIZE;
        status = FLASH_ErasePage(pageAddress);
        if (status != FLASH_COMPLETE) {
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    return PARAM_OK;
}
