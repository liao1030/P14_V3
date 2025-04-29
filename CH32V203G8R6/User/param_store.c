/************************************* (C) COPYRIGHT *******************************
 * File Name          : param_store.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter storage implementation for CH32V203G8R6 Flash.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_store.h"
#include "debug.h"
#include <string.h>

/* 全域變數 */
static uint32_t g_activeBlockAddr = 0;               /* 當前活動參數區的地址 */
static ParamBlockHeader_TypeDef g_blockHeaders[BLOCK_COUNT]; /* 參數區塊標頭 */
static uint16_t g_blockOffsets[BLOCK_COUNT+1];       /* 各區塊在Flash中的偏移 */

/* 本地函數原型 */
static uint8_t PARAM_LoadActiveBlock(void);
static uint8_t PARAM_InitBlockOffsets(void);
static uint8_t PARAM_VerifyBlockHeader(ParamBlockHeader_TypeDef *header);
static uint8_t PARAM_CalculateChecksum(const void *data, uint16_t size);
static uint8_t PARAM_WriteToFlash(uint32_t address, const void *data, uint16_t size);
static uint8_t PARAM_EraseFlashPage(uint32_t address);
static uint8_t PARAM_LoadDefaultParams(void);
static uint8_t __attribute__((unused)) PARAM_SaveActiveBlock(void);

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   初始化參數儲存模組
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_Init(void)
{
    /* 初始化區塊偏移表 */
    PARAM_InitBlockOffsets();
    
    printf("參數儲存初始化...\r\n");
    
    /* 嘗試從區塊A讀取參數 */
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    if (PARAM_LoadActiveBlock() == 0) {
        printf("已從參數區塊A載入參數\r\n");
        return 0;
    }
    
    /* 嘗試從區塊B讀取參數 */
    g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
    if (PARAM_LoadActiveBlock() == 0) {
        printf("已從參數區塊B載入參數\r\n");
        return 0;
    }
    
    /* 如果兩個區塊都無法讀取，則載入預設參數 */
    printf("無法讀取參數，載入預設值\r\n");
    if (PARAM_LoadDefaultParams() == 0) {
        /* 寫入區塊A */
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        return 0;
    }
    
    printf("參數初始化失敗\r\n");
    return 1;
}

/*********************************************************************
 * @fn      PARAM_InitBlockOffsets
 *
 * @brief   初始化各參數區塊的偏移量
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t PARAM_InitBlockOffsets(void)
{
    uint16_t offset = 0;
    
    /* 標頭區 */
    g_blockOffsets[0] = offset;
    offset += sizeof(ParamBlockHeader_TypeDef) * BLOCK_COUNT;
    
    /* 基本系統參數區 */
    g_blockOffsets[1] = offset;
    offset += sizeof(BasicSystemBlock);
    
    /* 硬體校準參數區 */
    g_blockOffsets[2] = offset;
    offset += sizeof(HardwareCalibBlock);
    
    /* 血糖參數區 */
    g_blockOffsets[3] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* 尿酸參數區 */
    g_blockOffsets[4] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* 膽固醇參數區 */
    g_blockOffsets[5] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* 三酸甘油脂參數區 */
    g_blockOffsets[6] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* 保留區與校驗區 */
    g_blockOffsets[7] = offset;
    
    /* 總長度偏移 */
    g_blockOffsets[BLOCK_COUNT] = offset + 32; // 為校驗區和保留區預留32字節
    
    /* 檢查參數區長度不超過Flash頁大小 */
    if (g_blockOffsets[BLOCK_COUNT] > FLASH_PAGE_SIZE) {
        printf("參數區大小超過Flash頁大小！\r\n");
        return 1;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_LoadActiveBlock
 *
 * @brief   載入當前活動參數區
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t PARAM_LoadActiveBlock(void)
{
    /* 讀取所有標頭 */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        uint32_t headerAddr = g_activeBlockAddr + g_blockOffsets[i];
        memcpy(&g_blockHeaders[i], (void *)headerAddr, sizeof(ParamBlockHeader_TypeDef));
        
        /* 驗證標頭 */
        if (PARAM_VerifyBlockHeader(&g_blockHeaders[i]) != 0) {
            return 1;
        }
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_VerifyBlockHeader
 *
 * @brief   驗證參數區塊標頭
 *
 * @param   header - 待驗證的標頭結構
 *
 * @return  狀態 (0: 有效, 非0: 無效)
 */
static uint8_t PARAM_VerifyBlockHeader(ParamBlockHeader_TypeDef *header)
{
    /* 檢查魔術碼 */
    if (header->magic != 0xA55A) {
        return 1;
    }
    
    /* 版本號必須大於等於1 */
    if (header->version < 1) {
        return 2;
    }
    
    /* 寫入計數器必須大於等於1 */
    if (header->writeCounter < 1) {
        return 3;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   讀取指定參數區塊
 *
 * @param   blockType - 要讀取的區塊類型
 * @param   buffer - 接收數據的緩衝區
 * @param   size - 緩衝區大小
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_ReadBlock(ParamBlockType blockType, void *buffer, uint16_t size)
{
    /* 檢查參數 */
    if (blockType >= BLOCK_COUNT || buffer == NULL || g_activeBlockAddr == 0) {
        return 1;
    }
    
    /* 檢查緩衝區大小 */
    uint16_t blockSize = g_blockOffsets[blockType + 1] - g_blockOffsets[blockType] - sizeof(ParamBlockHeader_TypeDef);
    if (size < blockSize) {
        return 2;
    }
    
    /* 計算數據地址 */
    uint32_t dataAddr = g_activeBlockAddr + g_blockOffsets[blockType] + sizeof(ParamBlockHeader_TypeDef);
    
    /* 複製數據到緩衝區 */
    memcpy(buffer, (void *)dataAddr, blockSize);
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateBlock
 *
 * @brief   更新指定參數區塊
 *
 * @param   blockType - 要更新的區塊類型
 * @param   buffer - 新參數數據
 * @param   size - 數據大小
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_UpdateBlock(ParamBlockType blockType, const void *buffer, uint16_t size)
{
    /* 檢查參數 */
    if (blockType >= BLOCK_COUNT || buffer == NULL || g_activeBlockAddr == 0) {
        return 1;
    }
    
    /* 檢查緩衝區大小 */
    uint16_t blockSize = g_blockOffsets[blockType + 1] - g_blockOffsets[blockType] - sizeof(ParamBlockHeader_TypeDef);
    if (size > blockSize) {
        return 2;
    }
    
    /* 更新標頭資訊 */
    g_blockHeaders[blockType].writeCounter++;
    
    /* 取當前系統時間作為時間戳 */
    /* 在實際應用中可以使用RTC時間或系統時間作為時間戳 */
    g_blockHeaders[blockType].timestamp = (uint32_t)((g_blockHeaders[blockType].writeCounter << 16) | blockType);
    
    /* 保存到備份區塊 */
    uint32_t backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    /* 擦除備份區塊 */
    if (PARAM_EraseFlashPage(backupAddr) != 0) {
        return 3;
    }
    
    /* 複製所有參數區塊到臨時緩衝區 */
    uint8_t tempBuf[FLASH_PAGE_SIZE];
    memset(tempBuf, 0xFF, FLASH_PAGE_SIZE);
    
    /* 複製所有標頭 */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        memcpy(&tempBuf[g_blockOffsets[i]], &g_blockHeaders[i], sizeof(ParamBlockHeader_TypeDef));
    }
    
    /* 複製所有數據區塊 */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        uint32_t srcAddr = g_activeBlockAddr + g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef);
        if (i == blockType) {
            /* 使用新數據 */
            memcpy(&tempBuf[g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef)], buffer, size);
        } else {
            /* 使用原有數據 */
            uint16_t dataSize = g_blockOffsets[i + 1] - g_blockOffsets[i] - sizeof(ParamBlockHeader_TypeDef);
            memcpy(&tempBuf[g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef)], (void *)srcAddr, dataSize);
        }
    }
    
    /* 計算校驗和 */
    uint8_t checksum = PARAM_CalculateChecksum(tempBuf, g_blockOffsets[BLOCK_COUNT] - 1);
    tempBuf[g_blockOffsets[BLOCK_COUNT] - 1] = checksum;
    
    /* 寫入到備份區塊 */
    if (PARAM_WriteToFlash(backupAddr, tempBuf, g_blockOffsets[BLOCK_COUNT]) != 0) {
        return 4;
    }
    
    /* 切換活動區塊 */
    g_activeBlockAddr = backupAddr;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateDateTime
 *
 * @brief   更新日期時間參數
 *
 * @param   year - 年份(0-99)
 * @param   month - 月份(1-12)
 * @param   date - 日期(1-31)
 * @param   hour - 小時(0-23)
 * @param   minute - 分鐘(0-59)
 * @param   second - 秒(0-59)
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    /* 檢查參數有效性 */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return 1;
    }
    
    /* 讀取當前基本系統參數 */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 2;
    }
    
    /* 更新日期時間 */
    sysBlock.year = year;
    sysBlock.month = month;
    sysBlock.date = date;
    sysBlock.hour = hour;
    sysBlock.minute = minute;
    sysBlock.second = second;
    
    /* 寫回參數區 */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   獲取日期時間參數
 *
 * @param   year - 年份指標
 * @param   month - 月份指標
 * @param   date - 日期指標
 * @param   hour - 小時指標
 * @param   minute - 分鐘指標
 * @param   second - 秒指標
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* 檢查指標有效性 */
    if (!year || !month || !date || !hour || !minute || !second) {
        return 1;
    }
    
    /* 讀取基本系統參數 */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 2;
    }
    
    /* 填充數據 */
    *year = sysBlock.year;
    *month = sysBlock.month;
    *date = sysBlock.date;
    *hour = sysBlock.hour;
    *minute = sysBlock.minute;
    *second = sysBlock.second;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_IncreaseTestCount
 *
 * @brief   增加測試計數器
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_IncreaseTestCount(void)
{
    /* 讀取當前基本系統參數 */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 1;
    }
    
    /* 增加計數器 */
    sysBlock.testCount++;
    
    /* 寫回參數區 */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_GetActiveBlockAddr
 *
 * @brief   取得當前活動參數區地址
 *
 * @return  活動參數區地址
 */
uint32_t PARAM_GetActiveBlockAddr(void)
{
    return g_activeBlockAddr;
}

/*********************************************************************
 * @fn      PARAM_CalculateChecksum
 *
 * @brief   計算校驗和
 *
 * @param   data - 數據指標
 * @param   size - 數據大小
 *
 * @return  校驗和
 */
static uint8_t PARAM_CalculateChecksum(const void *data, uint16_t size)
{
    uint8_t *ptr = (uint8_t *)data;
    uint8_t checksum = 0;
    
    for (uint16_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    
    return checksum;
}

/*********************************************************************
 * @fn      PARAM_EraseFlashPage
 *
 * @brief   擦除Flash頁
 *
 * @param   address - 頁的起始地址
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t PARAM_EraseFlashPage(uint32_t address)
{
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除頁 */
    FLASH_Status status = FLASH_ErasePage(address);
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE) ? 0 : 1;
}

/*********************************************************************
 * @fn      PARAM_WriteToFlash
 *
 * @brief   寫入數據到Flash
 *
 * @param   address - 寫入地址
 * @param   data - 數據指標
 * @param   size - 數據大小
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t PARAM_WriteToFlash(uint32_t address, const void *data, uint16_t size)
{
    /* 半字對齊檢查 */
    if (size % 2 != 0) {
        return 1;
    }
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    uint16_t *ptr = (uint16_t *)data;
    FLASH_Status status = FLASH_COMPLETE;
    
    /* 按半字寫入 */
    for (uint16_t i = 0; i < size / 2; i++) {
        status = FLASH_ProgramHalfWord(address + i * 2, ptr[i]);
        if (status != FLASH_COMPLETE) {
            break;
        }
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE) ? 0 : 2;
}

/*********************************************************************
 * @fn      PARAM_LoadDefaultParams
 *
 * @brief   載入預設參數
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t PARAM_LoadDefaultParams(void)
{
    /* 設置所有參數區塊標頭的預設值 */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        g_blockHeaders[i].magic = 0xA55A;
        g_blockHeaders[i].version = 1;
        g_blockHeaders[i].writeCounter = 1;
        g_blockHeaders[i].timestamp = 0;
    }
    
    /* 擦除參數區塊A */
    if (PARAM_EraseFlashPage(PARAM_BLOCK_A_ADDR) != 0) {
        return 1;
    }
    
    /* 準備參數區臨時緩衝區 */
    uint8_t tempBuf[FLASH_PAGE_SIZE];
    memset(tempBuf, 0xFF, FLASH_PAGE_SIZE);
    
    /* 填充標頭 */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        memcpy(&tempBuf[g_blockOffsets[i]], &g_blockHeaders[i], sizeof(ParamBlockHeader_TypeDef));
    }
    
    /* 初始化基本系統參數 */
    BasicSystemBlock sysBlock;
    memset(&sysBlock, 0, sizeof(BasicSystemBlock));
    sysBlock.lbt = 58;              /* 電池低電量閾值 2.58V */
    sysBlock.obt = 55;              /* 電池耗盡閾值 2.55V */
    sysBlock.factory = 0;           /* 使用者模式 */
    sysBlock.modelNo = 1;           /* 產品型號 */
    sysBlock.fwNo = 10;             /* 韌體版本 1.0 */
    sysBlock.testCount = 0;         /* 測試次數 */
    sysBlock.codeTableVer = 1;      /* 代碼表版本 */
    sysBlock.year = 24;             /* 2024年 */
    sysBlock.month = 6;             /* 6月 */
    sysBlock.date = 12;             /* 12日 */
    sysBlock.hour = 12;             /* 12時 */
    sysBlock.minute = 0;            /* 0分 */
    sysBlock.second = 0;            /* 0秒 */
    sysBlock.tempLowLimit = 10;     /* 溫度下限10°C */
    sysBlock.tempHighLimit = 40;    /* 溫度上限40°C */
    sysBlock.measureUnit = 1;       /* 測量單位 mg/dL */
    sysBlock.defaultEvent = 1;      /* 預設事件AC */
    sysBlock.stripType = 0;         /* 預設測試類型GLV(血糖) */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_BASIC_SYSTEM] + sizeof(ParamBlockHeader_TypeDef)], 
           &sysBlock, sizeof(BasicSystemBlock));
    
    /* 初始化硬體校準參數 */
    HardwareCalibBlock calibBlock;
    memset(&calibBlock, 0, sizeof(HardwareCalibBlock));
    calibBlock.evT3Trigger = 800;   /* EV_T3觸發電壓 800mV */
    calibBlock.evWorking = 164;     /* 工作電壓 */
    calibBlock.evT3 = 164;          /* T3電壓 */
    calibBlock.dacoOffset = 0;      /* DAC偏移 */
    calibBlock.dacdo = 0;           /* DAC調整 */
    calibBlock.cc211Status = 0;     /* CC211狀態 */
    calibBlock.calTolerance = 8;    /* 校準容差 0.8% */
    calibBlock.ops = 1.0f;          /* 校準斜率 1.0 */
    calibBlock.opi = 0.0f;          /* 校準截距 0.0 */
    calibBlock.qct = 0;             /* QCT測試 */
    calibBlock.tempOffset = 0;      /* 溫度偏移 */
    calibBlock.batteryOffset = 0;   /* 電池偏移 */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_HARDWARE_CALIB] + sizeof(ParamBlockHeader_TypeDef)], 
           &calibBlock, sizeof(HardwareCalibBlock));
    
    /* 初始化血糖參數 */
    TestParamBaseBlock bgBlock;
    memset(&bgBlock, 0, sizeof(TestParamBaseBlock));
    bgBlock.csuTolerance = 10;      /* 試片檢查容差 */
    bgBlock.ndl = 100;              /* 新試片電壓水平 */
    bgBlock.udl = 50;               /* 已使用試片電壓水平 */
    bgBlock.bloodIn = 150;          /* 血液電壓水平 */
    bgBlock.lowLimit = 20;          /* 測量下限 20 mg/dL */
    bgBlock.highLimit = 600;        /* 測量上限 600 mg/dL */
    bgBlock.t3E37 = 2000;           /* T3 ADV錯誤37閾值 */
    bgBlock.tpl1 = 10;              /* 時間脈衝寬度 1秒(第一組) */
    bgBlock.trd1 = 10;              /* 原始數據採集時間 1秒(第一組) */
    bgBlock.evWidth1 = 5;           /* 激勵時間 0.5秒(第一組) */
    bgBlock.tpl2 = 20;              /* 時間脈衝寬度 2秒(第二組) */
    bgBlock.trd2 = 20;              /* 原始數據採集時間 2秒(第二組) */
    bgBlock.evWidth2 = 10;          /* 激勵時間 1秒(第二組) */
    bgBlock.sq = 0;                 /* QC參數Sq */
    bgBlock.iq = 0;                 /* QC參數Iq */
    bgBlock.cvq = 0;                /* QC水平CV */
    bgBlock.aq = 0;                 /* QC參數A */
    bgBlock.bq = 0;                 /* QC參數B */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_BG_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &bgBlock, sizeof(TestParamBaseBlock));
    
    /* 初始化尿酸參數 */
    TestParamBaseBlock uaBlock;
    memset(&uaBlock, 0, sizeof(TestParamBaseBlock));
    uaBlock.csuTolerance = 10;      /* 試片檢查容差 */
    uaBlock.ndl = 100;              /* 新試片電壓水平 */
    uaBlock.udl = 50;               /* 已使用試片電壓水平 */
    uaBlock.bloodIn = 150;          /* 血液電壓水平 */
    uaBlock.lowLimit = 3;           /* 測量下限 3 mg/dL */
    uaBlock.highLimit = 200;        /* 測量上限 20 mg/dL */
    uaBlock.t3E37 = 2000;           /* T3 ADV錯誤37閾值 */
    uaBlock.tpl1 = 60;              /* 時間脈衝寬度 6秒(第一組) */
    uaBlock.trd1 = 50;              /* 原始數據採集時間 5秒(第一組) */
    uaBlock.evWidth1 = 40;          /* 激勵時間 4秒(第一組) */
    uaBlock.tpl2 = 70;              /* 時間脈衝寬度 7秒(第二組) */
    uaBlock.trd2 = 60;              /* 原始數據採集時間 6秒(第二組) */
    uaBlock.evWidth2 = 50;          /* 激勵時間 5秒(第二組) */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_UA_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &uaBlock, sizeof(TestParamBaseBlock));
    
    /* 初始化總膽固醇參數 */
    TestParamBaseBlock cholBlock;
    memset(&cholBlock, 0, sizeof(TestParamBaseBlock));
    cholBlock.csuTolerance = 10;    /* 試片檢查容差 */
    cholBlock.ndl = 100;            /* 新試片電壓水平 */
    cholBlock.udl = 50;             /* 已使用試片電壓水平 */
    cholBlock.bloodIn = 150;        /* 血液電壓水平 */
    cholBlock.lowLimit = 100;       /* 測量下限 100 mg/dL */
    cholBlock.highLimit = 400;      /* 測量上限 400 mg/dL */
    cholBlock.t3E37 = 2000;         /* T3 ADV錯誤37閾值 */
    cholBlock.tpl1 = 150;           /* 時間脈衝寬度 15秒(第一組) */
    cholBlock.trd1 = 140;           /* 原始數據採集時間 14秒(第一組) */
    cholBlock.evWidth1 = 130;       /* 激勵時間 13秒(第一組) */
    cholBlock.tpl2 = 160;           /* 時間脈衝寬度 16秒(第二組) */
    cholBlock.trd2 = 150;           /* 原始數據採集時間 15秒(第二組) */
    cholBlock.evWidth2 = 140;       /* 激勵時間 14秒(第二組) */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_CHOL_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &cholBlock, sizeof(TestParamBaseBlock));
    
    /* 初始化三酸甘油脂參數 */
    TestParamBaseBlock tgBlock;
    memset(&tgBlock, 0, sizeof(TestParamBaseBlock));
    tgBlock.csuTolerance = 10;      /* 試片檢查容差 */
    tgBlock.ndl = 100;              /* 新試片電壓水平 */
    tgBlock.udl = 50;               /* 已使用試片電壓水平 */
    tgBlock.bloodIn = 150;          /* 血液電壓水平 */
    tgBlock.lowLimit = 50;          /* 測量下限 50 mg/dL */
    tgBlock.highLimit = 500;        /* 測量上限 500 mg/dL */
    tgBlock.t3E37 = 2000;           /* T3 ADV錯誤37閾值 */
    tgBlock.tpl1 = 150;             /* 時間脈衝寬度 15秒(第一組) */
    tgBlock.trd1 = 140;             /* 原始數據採集時間 14秒(第一組) */
    tgBlock.evWidth1 = 130;         /* 激勵時間 13秒(第一組) */
    tgBlock.tpl2 = 160;             /* 時間脈衝寬度 16秒(第二組) */
    tgBlock.trd2 = 150;             /* 原始數據採集時間 15秒(第二組) */
    tgBlock.evWidth2 = 140;         /* 激勵時間 14秒(第二組) */
    
    /* 複製到臨時緩衝區 */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_TG_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &tgBlock, sizeof(TestParamBaseBlock));
    
    /* 計算校驗和 */
    uint8_t checksum = PARAM_CalculateChecksum(tempBuf, g_blockOffsets[BLOCK_COUNT] - 1);
    tempBuf[g_blockOffsets[BLOCK_COUNT] - 1] = checksum;
    
    /* 寫入到參數區塊A */
    if (PARAM_WriteToFlash(PARAM_BLOCK_A_ADDR, tempBuf, g_blockOffsets[BLOCK_COUNT]) != 0) {
        return 2;
    }
    
    /* 設置當前活動區塊為A */
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_ResetToDefault
 *
 * @brief   重置參數為默認值
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_ResetToDefault(void)
{
    /* 載入預設參數 */
    return PARAM_LoadDefaultParams();
}

/*********************************************************************
 * @fn      PARAM_GetStripParametersByStripType
 *
 * @brief   根據試片類型獲取試片電壓參數
 *
 * @param   stripType - 試片類型
 * @param   ndl - 新試片電壓水平指標
 * @param   udl - 已使用試片電壓水平指標
 * @param   bloodIn - 血液電壓水平指標
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    /* 檢查參數有效性 */
    if (!ndl || !udl || !bloodIn) {
        return 1;
    }
    
    /* 根據試片類型選擇對應的參數區 */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(血糖) */
        case 4: /* GAV(血糖) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(尿酸) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(總膽固醇) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(三酸甘油脂) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* 讀取試片參數 */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* 填充參數 */
    *ndl = paramBlock.ndl;
    *udl = paramBlock.udl;
    *bloodIn = paramBlock.bloodIn;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_GetMeasurementRangeByStripType
 *
 * @brief   根據試片類型獲取測量範圍
 *
 * @param   stripType - 試片類型
 * @param   lowLimit - 測量下限指標
 * @param   highLimit - 測量上限指標
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    /* 檢查參數有效性 */
    if (!lowLimit || !highLimit) {
        return 1;
    }
    
    /* 根據試片類型選擇對應的參數區 */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(血糖) */
        case 4: /* GAV(血糖) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(尿酸) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(總膽固醇) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(三酸甘油脂) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* 讀取參數區塊 */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* 填充參數 */
    *lowLimit = paramBlock.lowLimit;
    *highLimit = paramBlock.highLimit;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_GetTimingParametersByStripType
 *
 * @brief   根據試片類型獲取時序參數
 *
 * @param   stripType - 試片類型
 * @param   tpl - 時間脈衝寬度指標
 * @param   trd - 原始數據採集時間指標
 * @param   evWidth - 激勵時間指標
 * @param   seqNum - 時序組號(1或2)
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
uint8_t PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t seqNum)
{
    /* 檢查參數有效性 */
    if (!tpl || !trd || !evWidth || (seqNum != 1 && seqNum != 2)) {
        return 1;
    }
    
    /* 根據試片類型選擇對應的參數區 */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(血糖) */
        case 4: /* GAV(血糖) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(尿酸) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(總膽固醇) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(三酸甘油脂) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* 讀取參數區塊 */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* 根據序列組號填充參數 */
    if (seqNum == 1) {
        *tpl = paramBlock.tpl1;
        *trd = paramBlock.trd1;
        *evWidth = paramBlock.evWidth1;
    } else {
        *tpl = paramBlock.tpl2;
        *trd = paramBlock.trd2;
        *evWidth = paramBlock.evWidth2;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_SaveActiveBlock
 *
 * @brief   保存當前活動參數區
 *
 * @return  狀態 (0: 成功, 非0: 錯誤碼)
 */
static uint8_t __attribute__((unused)) PARAM_SaveActiveBlock(void)
{
    /* 實現保存當前活動參數區的邏輯 */
    return 0; // 假設保存成功
} 