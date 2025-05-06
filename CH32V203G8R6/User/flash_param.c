/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_param.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : CH32V203 Flash 參數儲存方案實現
 *********************************************************************************
 * Copyright (c) 2025 HMD Biomedical Inc.
 *******************************************************************************/

#include "flash_param.h"
#include "param_code_table.h"
#include <string.h>
#include "debug.h"

/* 定義常量值 */
#define FLASH_PARAM_MAGIC      0xA55A  /* 參數區塊標頭魔數 */
#define FLASH_PARAM_VERSION    0x01    /* 參數區塊版本號 */

/* 當前活動參數區指示器，默認使用區塊A */
static uint32_t activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;

/* 靜態函數聲明 */
static FLASH_ParamResult_TypeDef PARAM_CheckBlockValidity(uint32_t blockAddr, uint8_t blockType);
static FLASH_ParamResult_TypeDef PARAM_SelectActiveBlock(void);
static uint16_t PARAM_CalculateChecksum(uint8_t *data, uint16_t size);

/**
 * @brief 計算參數區塊校驗和
 * @param data: 資料指針
 * @param size: 資料大小
 * @return 校驗和
 */
static uint16_t PARAM_CalculateChecksum(uint8_t *data, uint16_t size)
{
    uint16_t sum = 0;
    uint16_t i;
    
    for (i = 0; i < size; i++)
    {
        sum += data[i];
    }
    
    return sum;
}

/**
 * @brief 檢查參數區塊有效性
 * @param blockAddr: 參數區塊地址
 * @param blockType: 區塊類型
 * @return 參數操作結果
 */
static FLASH_ParamResult_TypeDef PARAM_CheckBlockValidity(uint32_t blockAddr, uint8_t blockType)
{
    FLASH_ParamBlockHeader_TypeDef header;
    
    /* 從Flash讀取區塊標頭 */
    FLASH_ReadBuffer(blockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* 檢查魔數 */
    if (header.magic != FLASH_PARAM_MAGIC)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 檢查區塊類型 */
    if (blockType != 0xFF && header.blockType != blockType)
    {
        return PARAM_ERROR_INVALID;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 選擇活動參數區塊
 * @return 參數操作結果
 */
static FLASH_ParamResult_TypeDef PARAM_SelectActiveBlock(void)
{
    FLASH_ParamBlockHeader_TypeDef headerA, headerB;
    FLASH_ParamResult_TypeDef validA, validB;
    
    /* 檢查參數區塊A的有效性 */
    validA = PARAM_CheckBlockValidity(FLASH_PARAM_BLOCK_A_ADDR, BLOCK_BASIC_SYSTEM);
    
    /* 檢查參數區塊B的有效性 */
    validB = PARAM_CheckBlockValidity(FLASH_PARAM_BLOCK_B_ADDR, BLOCK_BASIC_SYSTEM);
    
    /* 根據有效性決定使用哪個區塊 */
    if (validA == PARAM_SUCCESS && validB == PARAM_SUCCESS)
    {
        /* 兩個區塊都有效，讀取標頭比較寫入計數器 */
        FLASH_ReadBuffer(FLASH_PARAM_BLOCK_A_ADDR, (uint8_t*)&headerA, sizeof(FLASH_ParamBlockHeader_TypeDef));
        FLASH_ReadBuffer(FLASH_PARAM_BLOCK_B_ADDR, (uint8_t*)&headerB, sizeof(FLASH_ParamBlockHeader_TypeDef));
        
        /* 選擇寫入計數較大的區塊 */
        if (headerA.writeCounter >= headerB.writeCounter)
        {
            activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;
        }
        else
        {
            activeParamBlockAddr = FLASH_PARAM_BLOCK_B_ADDR;
        }
        
        return PARAM_SUCCESS;
    }
    else if (validA == PARAM_SUCCESS)
    {
        /* 只有區塊A有效 */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;
        return PARAM_SUCCESS;
    }
    else if (validB == PARAM_SUCCESS)
    {
        /* 只有區塊B有效 */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_B_ADDR;
        return PARAM_SUCCESS;
    }
    else
    {
        /* 兩個區塊都無效 */
        return PARAM_ERROR_EMPTY;
    }
}

/**
 * @brief 初始化參數區
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_Init(void)
{
    FLASH_ParamResult_TypeDef result;
    FLASH_ParamBlock_TypeDef paramBlock;
    
    /* 選擇活動參數區塊 */
    result = PARAM_SelectActiveBlock();
    
    if (result == PARAM_ERROR_EMPTY)
    {
        /* 如果參數區是空的，創建默認參數區 */
        printf("參數區為空，初始化中...\r\n");
        
        /* 重置參數代碼表為默認值 */
        ParamCodeTable_Reset();
        
        /* 構建參數區塊標頭 */
        paramBlock.header.magic = FLASH_PARAM_MAGIC;
        paramBlock.header.version = FLASH_PARAM_VERSION;
        paramBlock.header.blockType = BLOCK_BASIC_SYSTEM;
        paramBlock.header.writeCounter = 1;
        paramBlock.header.timestamp = 0;
        paramBlock.header.dataSize = sizeof(ParamCodeTable_TypeDef);
        
        /* 複製參數數據 */
        memcpy(paramBlock.data, (uint8_t*)&ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
        
        /* 計算校驗和 */
        paramBlock.header.checksum = PARAM_CalculateChecksum(paramBlock.data, paramBlock.header.dataSize);
        
        /* 擦除參數區塊A和B */
        FLASH_ErasePage(FLASH_PARAM_BLOCK_A_ADDR);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_A_ADDR + FLASH_PAGE_SIZE);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_B_ADDR);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_B_ADDR + FLASH_PAGE_SIZE);
        
        /* 寫入到參數區塊A */
        if (FLASH_WriteBuffer(FLASH_PARAM_BLOCK_A_ADDR, (uint8_t*)&paramBlock, 
                             sizeof(FLASH_ParamBlockHeader_TypeDef) + paramBlock.header.dataSize) != PARAM_SUCCESS)
        {
            return PARAM_ERROR_FLASH;
        }
        
        /* 寫入到參數區塊B作為備份 */
        if (FLASH_WriteBuffer(FLASH_PARAM_BLOCK_B_ADDR, (uint8_t*)&paramBlock, 
                             sizeof(FLASH_ParamBlockHeader_TypeDef) + paramBlock.header.dataSize) != PARAM_SUCCESS)
        {
            return PARAM_ERROR_FLASH;
        }
        
        /* 設置活動參數區塊地址為A */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;
        
        return PARAM_SUCCESS;
    }
    else if (result == PARAM_SUCCESS)
    {
        /* 從活動參數區讀取到RAM */
        return PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, (void*)&ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
    }
    
    return result;
}

/**
 * @brief 讀取參數區塊
 * @param blockType: 區塊類型
 * @param data: 資料指針
 * @param size: 資料大小
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size)
{
    FLASH_ParamBlockHeader_TypeDef header;
    uint16_t checksum;
    
    /* 檢查參數有效性 */
    if (data == NULL || size == 0)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 從Flash讀取區塊標頭 */
    FLASH_ReadBuffer(activeParamBlockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* 檢查魔數 */
    if (header.magic != FLASH_PARAM_MAGIC)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 檢查區塊類型 */
    if (header.blockType != blockType)
    {
        /* 尋找對應類型的區塊 */
        uint32_t blockAddr = activeParamBlockAddr;
        uint32_t endAddr = activeParamBlockAddr + FLASH_PARAM_MAX_SIZE;
        
        while (blockAddr < endAddr)
        {
            FLASH_ReadBuffer(blockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
            
            if (header.magic != FLASH_PARAM_MAGIC)
            {
                break;
            }
            
            if (header.blockType == blockType)
            {
                break;
            }
            
            blockAddr += sizeof(FLASH_ParamBlockHeader_TypeDef) + header.dataSize;
            blockAddr = (blockAddr + 3) & ~3; /* 4字節對齊 */
        }
        
        if (blockAddr >= endAddr || header.blockType != blockType)
        {
            return PARAM_ERROR_INVALID;
        }
    }
    
    /* 檢查資料大小 */
    if (header.dataSize > size)
    {
        return PARAM_ERROR_SIZE;
    }
    
    /* 讀取參數數據 */
    FLASH_ReadBuffer(activeParamBlockAddr + sizeof(FLASH_ParamBlockHeader_TypeDef), (uint8_t*)data, header.dataSize);
    
    /* 驗證校驗和 */
    checksum = PARAM_CalculateChecksum((uint8_t*)data, header.dataSize);
    if (checksum != header.checksum)
    {
        return PARAM_ERROR_CHECKSUM;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 更新參數區塊
 * @param blockType: 區塊類型
 * @param data: 資料指針
 * @param size: 資料大小
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_UpdateBlock(uint8_t blockType, const void *data, uint16_t size)
{
    FLASH_ParamBlockHeader_TypeDef header;
    FLASH_ParamBlock_TypeDef paramBlock;
    uint32_t destAddr, backupAddr;
    
    /* 檢查參數有效性 */
    if (data == NULL || size == 0 || size > (FLASH_PARAM_MAX_SIZE - sizeof(FLASH_ParamBlockHeader_TypeDef)))
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 準備參數區塊頭部 */
    paramBlock.header.magic = FLASH_PARAM_MAGIC;
    paramBlock.header.version = FLASH_PARAM_VERSION;
    paramBlock.header.blockType = blockType;
    paramBlock.header.dataSize = size;
    
    /* 讀取當前活動區塊頭部 */
    FLASH_ReadBuffer(activeParamBlockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* 增加寫入計數器 */
    paramBlock.header.writeCounter = header.writeCounter + 1;
    
    /* 獲取當前系統時間作為時間戳 */
    paramBlock.header.timestamp = (ParamCodeTable.Time.YEAR << 26) | 
                                 (ParamCodeTable.Time.MONTH << 22) | 
                                 (ParamCodeTable.Time.DATE << 17) | 
                                 (ParamCodeTable.Time.HOUR << 12) | 
                                 (ParamCodeTable.Time.MINUTE << 6) | 
                                 ParamCodeTable.Time.SECOND;
    
    /* 複製參數數據 */
    memcpy(paramBlock.data, data, size);
    
    /* 計算校驗和 */
    paramBlock.header.checksum = PARAM_CalculateChecksum(paramBlock.data, size);
    
    /* 確定目標地址和備份地址 */
    if (activeParamBlockAddr == FLASH_PARAM_BLOCK_A_ADDR)
    {
        destAddr = FLASH_PARAM_BLOCK_A_ADDR;
        backupAddr = FLASH_PARAM_BLOCK_B_ADDR;
    }
    else
    {
        destAddr = FLASH_PARAM_BLOCK_B_ADDR;
        backupAddr = FLASH_PARAM_BLOCK_A_ADDR;
    }
    
    /* 擦除備份區塊 */
    FLASH_ErasePage(backupAddr);
    FLASH_ErasePage(backupAddr + FLASH_PAGE_SIZE);
    
    /* 先寫入到備份區塊 */
    if (FLASH_WriteBuffer(backupAddr, (uint8_t*)&paramBlock, 
                         sizeof(FLASH_ParamBlockHeader_TypeDef) + size) != PARAM_SUCCESS)
    {
        return PARAM_ERROR_FLASH;
    }
    
    /* 擦除目標區塊 */
    FLASH_ErasePage(destAddr);
    FLASH_ErasePage(destAddr + FLASH_PAGE_SIZE);
    
    /* 寫入到目標區塊 */
    if (FLASH_WriteBuffer(destAddr, (uint8_t*)&paramBlock, 
                         sizeof(FLASH_ParamBlockHeader_TypeDef) + size) != PARAM_SUCCESS)
    {
        /* 如果寫入目標區塊失敗，切換到備份區塊 */
        activeParamBlockAddr = backupAddr;
        return PARAM_ERROR_FLASH;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 增加測試計數
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_IncreaseTestCount(void)
{
    /* 增加測試計數 */
    ParamCodeTable.System.NOT++;
    
    /* 更新參數表到Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief 更新日期時間
 * @param year: 年 (0-99)
 * @param month: 月 (1-12)
 * @param date: 日 (1-31)
 * @param hour: 時 (0-23)
 * @param minute: 分 (0-59)
 * @param second: 秒 (0-59)
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, 
                                              uint8_t hour, uint8_t minute, uint8_t second)
{
    /* 檢查參數有效性 */
    if (month < 1 || month > 12 || date < 1 || date > 31 || 
        hour > 23 || minute > 59 || second > 59)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* 更新時間設定 */
    ParamCodeTable.Time.YEAR = year;
    ParamCodeTable.Time.MONTH = month;
    ParamCodeTable.Time.DATE = date;
    ParamCodeTable.Time.HOUR = hour;
    ParamCodeTable.Time.MINUTE = minute;
    ParamCodeTable.Time.SECOND = second;
    
    /* 更新參數表到Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief 獲取日期時間
 * @param year: 年指針
 * @param month: 月指針
 * @param date: 日指針
 * @param hour: 時指針
 * @param minute: 分指針
 * @param second: 秒指針
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                                           uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* 檢查指針有效性 */
    if (year == NULL || month == NULL || date == NULL || 
        hour == NULL || minute == NULL || second == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 獲取時間設定 */
    *year = ParamCodeTable.Time.YEAR;
    *month = ParamCodeTable.Time.MONTH;
    *date = ParamCodeTable.Time.DATE;
    *hour = ParamCodeTable.Time.HOUR;
    *minute = ParamCodeTable.Time.MINUTE;
    *second = ParamCodeTable.Time.SECOND;
    
    return PARAM_SUCCESS;
}

/**
 * @brief 獲取試片參數
 * @param stripType: 試片類型
 * @param ndl: 新試片濃度水平
 * @param udl: 已使用試片濃度水平
 * @param bloodIn: 血液濃度水平
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, 
                                                             uint16_t *udl, uint16_t *bloodIn)
{
    /* 檢查參數有效性 */
    if (ndl == NULL || udl == NULL || bloodIn == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 根據試片類型獲取參數 */
    switch (stripType)
    {
        case 0: /* GLV */
        case 4: /* GAV */
            *ndl = ParamCodeTable.BG.NDL;
            *udl = ParamCodeTable.BG.UDL;
            *bloodIn = ParamCodeTable.BG.BLOOD_IN;
            break;
            
        case 1: /* U */
            *ndl = ParamCodeTable.U.NDL;
            *udl = ParamCodeTable.U.UDL;
            *bloodIn = ParamCodeTable.U.BLOOD_IN;
            break;
            
        case 2: /* C */
            *ndl = ParamCodeTable.C.NDL;
            *udl = ParamCodeTable.C.UDL;
            *bloodIn = ParamCodeTable.C.BLOOD_IN;
            break;
            
        case 3: /* TG */
            *ndl = ParamCodeTable.TG.NDL;
            *udl = ParamCodeTable.TG.UDL;
            *bloodIn = ParamCodeTable.TG.BLOOD_IN;
            break;
            
        default:
            return PARAM_ERROR_RANGE;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 獲取測量範圍
 * @param stripType: 試片類型
 * @param lowLimit: 測量下限指針
 * @param highLimit: 測量上限指針
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, 
                                                              uint8_t *highLimit)
{
    /* 檢查參數有效性 */
    if (lowLimit == NULL || highLimit == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 根據試片類型獲取參數 */
    switch (stripType)
    {
        case 0: /* GLV */
        case 4: /* GAV */
            *lowLimit = ParamCodeTable.BG.L;
            *highLimit = ParamCodeTable.BG.H;
            break;
            
        case 1: /* U */
            *lowLimit = ParamCodeTable.U.L;
            *highLimit = ParamCodeTable.U.H;
            break;
            
        case 2: /* C */
            *lowLimit = ParamCodeTable.C.L;
            *highLimit = ParamCodeTable.C.H;
            break;
            
        case 3: /* TG */
            *lowLimit = ParamCodeTable.TG.L;
            *highLimit = ParamCodeTable.TG.H;
            break;
            
        default:
            return PARAM_ERROR_RANGE;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 獲取時序參數
 * @param stripType: 試片類型
 * @param tpl: 時間脈衝寬指針
 * @param trd: 原始數據時間指針
 * @param evWidth: 烈性時間指針
 * @param groupIndex: 參數組索引 (0:第一組, 1:第二組)
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, 
                                                              uint16_t *trd, uint16_t *evWidth, 
                                                              uint8_t groupIndex)
{
    /* 檢查參數有效性 */
    if (tpl == NULL || trd == NULL || evWidth == NULL || groupIndex > 1)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* 根據試片類型和參數組索引獲取參數 */
    switch (stripType)
    {
        case 0: /* GLV */
        case 4: /* GAV */
            if (groupIndex == 0)
            {
                *tpl = ParamCodeTable.BG.TPL_1;
                *trd = ParamCodeTable.BG.TRD_1;
                *evWidth = ParamCodeTable.BG.EV_Width_1;
            }
            else
            {
                *tpl = ParamCodeTable.BG.TPL_2;
                *trd = ParamCodeTable.BG.TRD_2;
                *evWidth = ParamCodeTable.BG.EV_Width_2;
            }
            break;
            
        case 1: /* U */
            if (groupIndex == 0)
            {
                *tpl = ParamCodeTable.U.TPL_1;
                *trd = ParamCodeTable.U.TRD_1;
                *evWidth = ParamCodeTable.U.EV_Width_1;
            }
            else
            {
                *tpl = ParamCodeTable.U.TPL_2;
                *trd = ParamCodeTable.U.TRD_2;
                *evWidth = ParamCodeTable.U.EV_Width_2;
            }
            break;
            
        case 2: /* C */
            if (groupIndex == 0)
            {
                *tpl = ParamCodeTable.C.TPL_1;
                *trd = ParamCodeTable.C.TRD_1;
                *evWidth = ParamCodeTable.C.EV_Width_1;
            }
            else
            {
                *tpl = ParamCodeTable.C.TPL_2;
                *trd = ParamCodeTable.C.TRD_2;
                *evWidth = ParamCodeTable.C.EV_Width_2;
            }
            break;
            
        case 3: /* TG */
            if (groupIndex == 0)
            {
                *tpl = ParamCodeTable.TG.TPL_1;
                *trd = ParamCodeTable.TG.TRD_1;
                *evWidth = ParamCodeTable.TG.EV_Width_1;
            }
            else
            {
                *tpl = ParamCodeTable.TG.TPL_2;
                *trd = ParamCodeTable.TG.TRD_2;
                *evWidth = ParamCodeTable.TG.EV_Width_2;
            }
            break;
            
        default:
            return PARAM_ERROR_RANGE;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief 將參數表重置為默認值
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef PARAM_ResetToDefault(void)
{
    /* 重置參數代碼表為默認值 */
    ParamCodeTable_Reset();
    
    /* 更新參數表到Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief 寫入數據到Flash
 * @param address: Flash地址
 * @param data: 數據指針
 * @param size: 數據大小
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint16_t size)
{
    uint32_t i;
    uint32_t word;
    FLASH_Status status;
    
    /* 檢查地址和大小是否有效 */
    if (address < FLASH_PARAM_BLOCK_A_ADDR || 
        address + size > FLASH_RECORD_BLOCK_ADDR + FLASH_PAGE_SIZE*6)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 按字（4字節）寫入數據 */
    for (i = 0; i < size; i += 4)
    {
        /* 對齊處理 */
        if (i + 4 <= size)
        {
            /* 直接讀取4字節 */
            word = *(uint32_t*)(data + i);
        }
        else
        {
            /* 處理尾部不足4字節的情況 */
            word = 0xFFFFFFFF;
            memcpy(&word, data + i, size - i);
        }
        
        /* 寫入一個字（4字節） */
        status = FLASH_ProgramWord(address + i, word);
        
        if (status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return PARAM_ERROR_FLASH;
        }
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    return PARAM_SUCCESS;
}

/**
 * @brief 從Flash讀取數據
 * @param address: Flash地址
 * @param data: 數據緩衝區指針
 * @param size: 數據大小
 * @return 參數操作結果
 */
FLASH_ParamResult_TypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint16_t size)
{
    /* 檢查地址和大小是否有效 */
    if (address < FLASH_PARAM_BLOCK_A_ADDR || 
        address + size > FLASH_RECORD_BLOCK_ADDR + FLASH_PAGE_SIZE*6)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* 直接從Flash中複製數據 */
    memcpy(data, (uint8_t*)address, size);
    
    return PARAM_SUCCESS;
}