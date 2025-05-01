/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : CH32V203 Flash參數儲存方案實現
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "P14_Flash_Storage.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

/* 活動參數區塊地址 */
static uint32_t g_activeBlockAddr = 0;

/* 參數表實例 */
static ParameterBlock g_paramBlock;

/* 默認參數表值 */
static const ParameterBlock g_defaultParams = {
    /* 參數表頭部 */
    .header = {
        .magic = 0xA55A,
        .version = 1,
        .writeCounter = 1,
        .timestamp = 0,
        .checksum = 0
    },
    
    /* 參數區 */
    .params = {
        /* 基本系統參數 */
        .basicSystem = {
            .lbt = 58,              // 低電池閾值 (2.58V)
            .obt = 55,              // 電池耗盡閾值 (2.55V)
            .factory = 0,           // 使用者模式
            .modelNo = 1,           // 產品型號
            .fwNo = 10,             // 韌體版本號 1.0
            .testCount = 0,         // 測試次數初始為0
            .codeTableVer = 1,      // 代碼表版本號
            .year = 24,             // 年份設定 (2024年)
            .month = 7,             // 月份設定 (7月)
            .date = 25,             // 日期設定 (25日)
            .hour = 12,             // 小時設定 (12時)
            .minute = 0,            // 分鐘設定 (0分)
            .second = 0,            // 秒數設定 (0秒)
            .tempLowLimit = 10,     // 操作溫度範圍下限 (10°C)
            .tempHighLimit = 40,    // 操作溫度範圍上限 (40°C)
            .measureUnit = 1,       // 濃度單位設定 (mg/dL)
            .defaultEvent = 1,      // 餐飲事件類型 (AC)
            .stripType = 0,         // 測試項目 (血糖)
            .reserved = {0, 0}      // 預留位置
        },
        
        /* 硬體校正參數 */
        .hardwareCalib = {
            .evT3Trigger = 800,     // EV_T3觸發電壓 (800mV)
            .evWorking = 164,       // 濃度工作電流電壓
            .evT3 = 164,            // 血液消耗電流電壓
            .dacoOffset = 0,        // DAC偏移補償
            .dacdo = 0,             // DAC校正管理參數
            .cc211Status = 0,       // CC211未完成旗標
            .calTolerance = 8,      // OPS/OPI校正容差 (0.8%)
            .ops = 1.0f,            // OPA校正斜率
            .opi = 0.0f,            // OPA校正截距
            .qct = 0,               // QCT校正測試低位元組
            .tempOffset = 0,        // 溫度校正偏移
            .batteryOffset = 0,     // 電池校正偏移
            .reserved = {0, 0, 0, 0, 0}  // 預留位置
        },
        
        /* 血糖參數 */
        .bgParams = {
            .bgCsuTolerance = 10,   // 試片檢查容差
            .bgNdl = 100,           // 新試片濃度水平
            .bgUdl = 50,            // 已使用試片濃度水平
            .bgBloodIn = 150,       // 血液濃度水平
            .bgStripLot = {0},      // 試片批號，初始為全0
            .bgL = 20,              // 血糖值下限 (20 mg/dL)
            .bgH = 600,             // 血糖值上限 (600 mg/dL)
            .bgT3E37 = 2000,        // T3 ADV超E37閾值
            .bgTPL1 = 10,           // 第一組時序參數TPL (1.0秒)
            .bgTRD1 = 10,           // 第一組時序參數TRD (1.0秒)
            .bgEVWidth1 = 5,        // 第一組時序參數EV寬度 (0.5秒)
            .bgTPL2 = 20,           // 第二組時序參數TPL (2.0秒)
            .bgTRD2 = 20,           // 第二組時序參數TRD (2.0秒)
            .bgEVWidth2 = 10,       // 第二組時序參數EV寬度 (1.0秒)
            .bgCompA = 0,           // 補償參數A
            .bgCompB = 0,           // 補償參數B
            .bgCompC = 0,           // 補償參數C
            .bgCompD = 0,           // 補償參數D
            .reserved = {0}         // 預留位置
        },
        
        /* 尿酸參數 */
        .uaParams = {
            .uCsuTolerance = 10,    // 試片檢查容差
            .uNdl = 100,            // 新試片濃度水平
            .uUdl = 50,             // 已使用試片濃度水平
            .uBloodIn = 150,        // 血液濃度水平
            .uStripLot = {0},       // 試片批號，初始為全0
            .uL = 3,                // 尿酸值下限 (3 mg/dL)
            .uH = 20,               // 尿酸值上限 (20 mg/dL)
            .uT3E37 = 2000,         // T3 ADV超E37閾值
            .uTPL1 = 12,            // 第一組時序參數TPL (1.2秒)
            .uTRD1 = 12,            // 第一組時序參數TRD (1.2秒)
            .uEVWidth1 = 6,         // 第一組時序參數EV寬度 (0.6秒)
            .uTPL2 = 25,            // 第二組時序參數TPL (2.5秒)
            .uTRD2 = 25,            // 第二組時序參數TRD (2.5秒)
            .uEVWidth2 = 12,        // 第二組時序參數EV寬度 (1.2秒)
            .uCompA = 0,            // 補償參數A
            .uCompB = 0,            // 補償參數B
            .uCompC = 0,            // 補償參數C
            .uCompD = 0,            // 補償參數D
            .reserved = {0}         // 預留位置
        },
        
        /* 總膽固醇參數 */
        .cholParams = {
            .cCsuTolerance = 10,    // 試片檢查容差
            .cNdl = 100,            // 新試片濃度水平
            .cUdl = 50,             // 已使用試片濃度水平
            .cBloodIn = 150,        // 血液濃度水平
            .cStripLot = {0},       // 試片批號，初始為全0
            .cL = 100,              // 膽固醇值下限 (100 mg/dL)
            .cH = 400,              // 膽固醇值上限 (400 mg/dL)
            .cT3E37 = 2000,         // T3 ADV超E37閾值
            .cTPL1 = 15,            // 第一組時序參數TPL (1.5秒)
            .cTRD1 = 15,            // 第一組時序參數TRD (1.5秒)
            .cEVWidth1 = 7,         // 第一組時序參數EV寬度 (0.7秒)
            .cTPL2 = 30,            // 第二組時序參數TPL (3.0秒)
            .cTRD2 = 30,            // 第二組時序參數TRD (3.0秒)
            .cEVWidth2 = 15,        // 第二組時序參數EV寬度 (1.5秒)
            .cCompA = 0,            // 補償參數A
            .cCompB = 0,            // 補償參數B
            .cCompC = 0,            // 補償參數C
            .cCompD = 0,            // 補償參數D
            .reserved = {0}         // 預留位置
        },
        
        /* 三酸甘油脂參數 */
        .tgParams = {
            .tgCsuTolerance = 10,   // 試片檢查容差
            .tgNdl = 100,           // 新試片濃度水平
            .tgUdl = 50,            // 已使用試片濃度水平
            .tgBloodIn = 150,       // 血液濃度水平
            .tgStripLot = {0},      // 試片批號，初始為全0
            .tgL = 50,              // 三酸甘油脂值下限 (50 mg/dL)
            .tgH = 500,             // 三酸甘油脂值上限 (500 mg/dL)
            .tgT3E37 = 2000,        // T3 ADV超E37閾值
            .tgTPL1 = 18,           // 第一組時序參數TPL (1.8秒)
            .tgTRD1 = 18,           // 第一組時序參數TRD (1.8秒)
            .tgEVWidth1 = 9,        // 第一組時序參數EV寬度 (0.9秒)
            .tgTPL2 = 35,           // 第二組時序參數TPL (3.5秒)
            .tgTRD2 = 35,           // 第二組時序參數TRD (3.5秒)
            .tgEVWidth2 = 17,       // 第二組時序參數EV寬度 (1.7秒)
            .tgCompA = 0,           // 補償參數A
            .tgCompB = 0,           // 補償參數B
            .tgCompC = 0,           // 補償參數C
            .tgCompD = 0,           // 補償參數D
            .reserved = {0}         // 預留位置
        },
        
        /* 保留區塊 */
        .reserved = {
            .reserved = {0},        // 預留位置
            .calibChecksum = 0      // 校驗區校驗和
        }
    }
};

/* 靜態函數宣告 */
static void UpdateChecksum(ParameterBlock *block);
static bool VerifyChecksum(const ParameterBlock *block);
static void ValidateActiveBlock(void);
static bool SaveParameterBlock(void);
static uint16_t CalculateChecksum(const uint8_t *data, uint16_t size);
static uint32_t GetBlockOffsetByType(uint8_t blockType);
static uint16_t GetBlockSizeByType(uint8_t blockType);

/**
 * @brief 初始化Flash儲存系統
 */
void FLASH_Storage_Init(void)
{
    /* 驗證活動參數區塊 */
    ValidateActiveBlock();
    
    printf("Flash參數儲存系統初始化完成\r\n");
    printf("活動參數區塊地址: 0x%08lX\r\n", g_activeBlockAddr);
    printf("參數表版本: %d, 寫入計數: %d\r\n", 
           g_paramBlock.header.version, 
           g_paramBlock.header.writeCounter);
}

/**
 * @brief 將數據寫入Flash
 * @param address: 目標地址
 * @param data: 數據指針
 * @param size: 數據大小
 * @return 操作是否成功
 */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size)
{
    /* 參數檢查 */
    if (data == NULL || size == 0) {
        return false;
    }
    
    /* 檢查地址是否在Flash範圍內 */
    if (address < PARAM_FLASH_BASE || 
        address + size > PARAM_FLASH_BASE + 64 * 1024) { // 64KB總大小
        return false;
    }
    
    /* 每次寫入2字節 */
    uint32_t i;
    uint32_t aligned_size = (size + 1) & ~1; // 向上對齊到偶數
    FLASH_Status status;
    
    for (i = 0; i < aligned_size; i += 2) {
        uint16_t halfword;
        
        if (i + 1 < size) {
            halfword = (uint16_t)data[i] | ((uint16_t)data[i + 1] << 8);
        } else if (i < size) {
            /* 如果只剩最後一個字節，高字節用0xFF填充 */
            halfword = (uint16_t)data[i] | 0xFF00;
        } else {
            /* 對齊填充 */
            halfword = 0xFFFF;
        }
        
        status = FLASH_ProgramHalfWord(address + i, halfword);
        if (status != FLASH_COMPLETE) {
            printf("Flash寫入錯誤: 地址0x%08lX, 狀態%d\r\n", address + i, status);
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 從Flash讀取數據
 * @param address: 源地址
 * @param data: 目標緩衝區
 * @param size: 數據大小
 * @return 操作是否成功
 */
bool FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t size)
{
    /* 參數檢查 */
    if (data == NULL || size == 0) {
        return false;
    }
    
    /* 檢查地址是否在Flash範圍內 */
    if (address < PARAM_FLASH_BASE || 
        address + size > PARAM_FLASH_BASE + 64 * 1024) { // 64KB總大小
        return false;
    }
    
    /* 直接從Flash讀取 */
    memcpy(data, (const void *)address, size);
    
    return true;
}

/**
 * @brief 驗證活動參數區塊
 */
static void ValidateActiveBlock(void)
{
    ParameterBlock blockA, blockB;
    bool validA = false, validB = false;
    
    /* 讀取參數區A */
    if (FLASH_ReadBuffer(PARAM_BLOCK_A_ADDR, (uint8_t *)&blockA, sizeof(ParameterBlock))) {
        validA = (blockA.header.magic == 0xA55A) && VerifyChecksum(&blockA);
    }
    
    /* 讀取參數區B */
    if (FLASH_ReadBuffer(PARAM_BLOCK_B_ADDR, (uint8_t *)&blockB, sizeof(ParameterBlock))) {
        validB = (blockB.header.magic == 0xA55A) && VerifyChecksum(&blockB);
    }
    
    /* 確定活動參數區 */
    if (validA && validB) {
        /* 兩個區塊都有效，比較寫入計數器 */
        if (blockA.header.writeCounter >= blockB.header.writeCounter) {
            g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
            memcpy(&g_paramBlock, &blockA, sizeof(ParameterBlock));
        } else {
            g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
            memcpy(&g_paramBlock, &blockB, sizeof(ParameterBlock));
        }
    } else if (validA) {
        /* 只有A區塊有效 */
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        memcpy(&g_paramBlock, &blockA, sizeof(ParameterBlock));
    } else if (validB) {
        /* 只有B區塊有效 */
        g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
        memcpy(&g_paramBlock, &blockB, sizeof(ParameterBlock));
    } else {
        /* 兩個區塊都無效，使用默認參數 */
        memcpy(&g_paramBlock, &g_defaultParams, sizeof(ParameterBlock));
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        
        /* 更新校驗和 */
        UpdateChecksum(&g_paramBlock);
        
        /* 保存到Flash */
        SaveParameterBlock();
    }
}

/**
 * @brief 保存參數區塊到Flash
 * @return 操作是否成功
 */
static bool SaveParameterBlock(void)
{
    uint32_t targetAddr;
    
    /* 參數區交替使用，避免同一區域反覆寫入 */
    if (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) {
        targetAddr = PARAM_BLOCK_B_ADDR;
    } else {
        targetAddr = PARAM_BLOCK_A_ADDR;
    }
    
    /* 增加寫入計數器 */
    g_paramBlock.header.writeCounter++;
    
    /* 更新校驗和 */
    UpdateChecksum(&g_paramBlock);
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除目標扇區 */
    FLASH_Status status1 = FLASH_ErasePage(targetAddr);
    FLASH_Status status2 = FLASH_COMPLETE;
    
    /* 如果參數區大於1KB，需要擦除第二個扇區 */
    if (sizeof(ParameterBlock) > PARAM_SECTOR_SIZE) {
        status2 = FLASH_ErasePage(targetAddr + PARAM_SECTOR_SIZE);
    }
    
    if (status1 != FLASH_COMPLETE || status2 != FLASH_COMPLETE) {
        FLASH_Lock();
        printf("擦除Flash失敗: 狀態1=%d, 狀態2=%d\r\n", status1, status2);
        return false;
    }
    
    /* 寫入參數區 */
    bool result = FLASH_WriteBuffer(targetAddr, (const uint8_t *)&g_paramBlock, sizeof(ParameterBlock));
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    if (result) {
        /* 更新活動參數區地址 */
        g_activeBlockAddr = targetAddr;
        return true;
    }
    
    return false;
}

/**
 * @brief 計算數據校驗和
 * @param data: 數據指針
 * @param size: 數據大小
 * @return 校驗和
 */
static uint16_t CalculateChecksum(const uint8_t *data, uint16_t size)
{
    uint16_t checksum = 0;
    
    for (uint16_t i = 0; i < size; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief 更新參數區塊校驗和
 * @param block: 參數區塊指針
 */
static void UpdateChecksum(ParameterBlock *block)
{
    if (block != NULL) {
        /* 先將校驗和清零 */
        block->header.checksum = 0;
        
        /* 計算校驗和 */
        block->header.checksum = CalculateChecksum(
            (const uint8_t *)block, 
            sizeof(ParameterBlock)
        );
    }
}

/**
 * @brief 驗證參數區塊校驗和
 * @param block: 參數區塊指針
 * @return 校驗是否通過
 */
static bool VerifyChecksum(const ParameterBlock *block)
{
    if (block != NULL) {
        /* 保存原校驗和 */
        uint16_t storedChecksum = block->header.checksum;
        
        /* 創建臨時副本用於計算 */
        ParameterBlock tempBlock;
        memcpy(&tempBlock, block, sizeof(ParameterBlock));
        
        /* 清零校驗和字段 */
        tempBlock.header.checksum = 0;
        
        /* 計算校驗和 */
        uint16_t calculatedChecksum = CalculateChecksum(
            (const uint8_t *)&tempBlock, 
            sizeof(ParameterBlock)
        );
        
        /* 比較校驗和 */
        return (calculatedChecksum == storedChecksum);
    }
    
    return false;
}

/**
 * @brief 獲取區塊類型對應的偏移地址
 * @param blockType: 區塊類型
 * @return 偏移地址
 */
static uint32_t GetBlockOffsetByType(uint8_t blockType)
{
    uint32_t offset = 0;
    
    switch (blockType) {
        case BLOCK_BASIC_SYSTEM:
            offset = offsetof(ParameterBlocks, basicSystem);
            break;
        case BLOCK_HARDWARE_CALIB:
            offset = offsetof(ParameterBlocks, hardwareCalib);
            break;
        case BLOCK_GLUCOSE_PARAMS:
            offset = offsetof(ParameterBlocks, bgParams);
            break;
        case BLOCK_URIC_ACID_PARAMS:
            offset = offsetof(ParameterBlocks, uaParams);
            break;
        case BLOCK_CHOLESTEROL_PARAMS:
            offset = offsetof(ParameterBlocks, cholParams);
            break;
        case BLOCK_TRIGLYCERIDE_PARAMS:
            offset = offsetof(ParameterBlocks, tgParams);
            break;
        case BLOCK_RESERVED:
            offset = offsetof(ParameterBlocks, reserved);
            break;
        default:
            offset = 0xFFFFFFFF; // 無效偏移
            break;
    }
    
    /* 參數區在參數表中的偏移量 */
    if (offset != 0xFFFFFFFF) {
        offset += offsetof(ParameterBlock, params);
    }
    
    return offset;
}

/**
 * @brief 獲取區塊類型對應的大小
 * @param blockType: 區塊類型
 * @return 區塊大小
 */
static uint16_t GetBlockSizeByType(uint8_t blockType)
{
    uint16_t size = 0;
    
    switch (blockType) {
        case BLOCK_BASIC_SYSTEM:
            size = sizeof(BasicSystemBlock);
            break;
        case BLOCK_HARDWARE_CALIB:
            size = sizeof(HardwareCalibBlock);
            break;
        case BLOCK_GLUCOSE_PARAMS:
            size = sizeof(GlucoseParamBlock);
            break;
        case BLOCK_URIC_ACID_PARAMS:
            size = sizeof(UricAcidParamBlock);
            break;
        case BLOCK_CHOLESTEROL_PARAMS:
            size = sizeof(CholesterolParamBlock);
            break;
        case BLOCK_TRIGLYCERIDE_PARAMS:
            size = sizeof(TriglycerideParamBlock);
            break;
        case BLOCK_RESERVED:
            size = sizeof(ReservedBlock);
            break;
        default:
            size = 0; // 無效大小
            break;
    }
    
    return size;
}

/**
 * @brief 讀取指定類型的參數區塊
 * @param blockType: 區塊類型
 * @param buffer: 緩衝區指針
 * @param size: 緩衝區大小
 * @return 操作是否成功
 */
bool PARAM_ReadParameterBlock(uint8_t blockType, void *buffer, uint16_t size)
{
    if (buffer == NULL) {
        return false;
    }
    
    /* 獲取區塊偏移和大小 */
    uint32_t offset = GetBlockOffsetByType(blockType);
    uint16_t blockSize = GetBlockSizeByType(blockType);
    
    /* 檢查參數是否有效 */
    if (offset == 0xFFFFFFFF || blockSize == 0 || size > blockSize) {
        return false;
    }
    
    /* 計算區塊在緩存中的地址 */
    uint8_t *srcAddr = (uint8_t *)&g_paramBlock + offset;
    
    /* 複製數據 */
    memcpy(buffer, srcAddr, size);
    
    return true;
}

/**
 * @brief 更新指定類型的參數區塊
 * @param blockType: 區塊類型
 * @param buffer: 數據緩衝區
 * @param size: 緩衝區大小
 * @return 操作是否成功
 */
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size)
{
    if (buffer == NULL) {
        return false;
    }
    
    /* 獲取區塊偏移和大小 */
    uint32_t offset = GetBlockOffsetByType(blockType);
    uint16_t blockSize = GetBlockSizeByType(blockType);
    
    /* 檢查參數是否有效 */
    if (offset == 0xFFFFFFFF || blockSize == 0 || size > blockSize) {
        return false;
    }
    
    /* 計算區塊在內存中的地址 */
    uint8_t *destAddr = (uint8_t *)&g_paramBlock + offset;
    
    /* 複製數據 */
    memcpy(destAddr, buffer, size);
    
    /* 保存到Flash */
    return SaveParameterBlock();
}

/**
 * @brief 獲取系統時間
 * @param year: 年份指針
 * @param month: 月份指針
 * @param date: 日期指針
 * @param hour: 小時指針
 * @param minute: 分鐘指針
 * @param second: 秒數指針
 * @return 操作是否成功
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* 讀取系統參數塊 */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* 填時間數據 */
    if (year != NULL) *year = basicParams.year;
    if (month != NULL) *month = basicParams.month;
    if (date != NULL) *date = basicParams.date;
    if (hour != NULL) *hour = basicParams.hour;
    if (minute != NULL) *minute = basicParams.minute;
    if (second != NULL) *second = basicParams.second;
    
    return true;
}

/**
 * @brief 更新系統時間
 * @param year: 年份
 * @param month: 月份
 * @param date: 日期
 * @param hour: 小時
 * @param minute: 分鐘
 * @param second: 秒數
 * @return 操作是否成功
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    /* 檢查時間數據有效性 */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    
    /* 讀取系統參數塊 */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* 填時間數據 */
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    /* 更新到參數塊中 */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief 增加測試次數
 * @return 操作是否成功
 */
bool PARAM_IncreaseTestCount(void)
{
    /* 讀取系統參數塊 */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* 增加測試次數 */
    basicParams.testCount++;
    
    /* 更新到參數塊中 */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief 根據試片類型獲取試片參數
 * @param stripType: 試片類型
 * @param ndl: 新試片濃度水平指針
 * @param udl: 已使用試片濃度水平指針
 * @param bloodIn: 血液濃度水平指針
 * @return 操作是否成功
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* 血糖參數 */
                GlucoseParamBlock bgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_GLUCOSE_PARAMS, &bgParams, sizeof(GlucoseParamBlock))) {
                    return false;
                }
                if (ndl) *ndl = bgParams.bgNdl;
                if (udl) *udl = bgParams.bgUdl;
                if (bloodIn) *bloodIn = bgParams.bgBloodIn;
            }
            break;
            
        case STRIP_TYPE_U:
            {
                /* 尿酸參數 */
                UricAcidParamBlock uaParams;
                if (!PARAM_ReadParameterBlock(BLOCK_URIC_ACID_PARAMS, &uaParams, sizeof(UricAcidParamBlock))) {
                    return false;
                }
                if (ndl) *ndl = uaParams.uNdl;
                if (udl) *udl = uaParams.uUdl;
                if (bloodIn) *bloodIn = uaParams.uBloodIn;
            }
            break;
            
        case STRIP_TYPE_C:
            {
                /* 總膽固醇參數 */
                CholesterolParamBlock cholParams;
                if (!PARAM_ReadParameterBlock(BLOCK_CHOLESTEROL_PARAMS, &cholParams, sizeof(CholesterolParamBlock))) {
                    return false;
                }
                if (ndl) *ndl = cholParams.cNdl;
                if (udl) *udl = cholParams.cUdl;
                if (bloodIn) *bloodIn = cholParams.cBloodIn;
            }
            break;
            
        case STRIP_TYPE_TG:
            {
                /* 三酸甘油脂參數 */
                TriglycerideParamBlock tgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_TRIGLYCERIDE_PARAMS, &tgParams, sizeof(TriglycerideParamBlock))) {
                    return false;
                }
                if (ndl) *ndl = tgParams.tgNdl;
                if (udl) *udl = tgParams.tgUdl;
                if (bloodIn) *bloodIn = tgParams.tgBloodIn;
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief 根據試片類型獲取測量範圍
 * @param stripType: 試片類型
 * @param lowLimit: 下限指針
 * @param highLimit: 上限指針
 * @return 操作是否成功
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* 血糖參數 */
                GlucoseParamBlock bgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_GLUCOSE_PARAMS, &bgParams, sizeof(GlucoseParamBlock))) {
                    return false;
                }
                if (lowLimit) *lowLimit = bgParams.bgL;
                if (highLimit) *highLimit = bgParams.bgH;
            }
            break;
            
        case STRIP_TYPE_U:
            {
                /* 尿酸參數 */
                UricAcidParamBlock uaParams;
                if (!PARAM_ReadParameterBlock(BLOCK_URIC_ACID_PARAMS, &uaParams, sizeof(UricAcidParamBlock))) {
                    return false;
                }
                if (lowLimit) *lowLimit = uaParams.uL;
                if (highLimit) *highLimit = uaParams.uH;
            }
            break;
            
        case STRIP_TYPE_C:
            {
                /* 總膽固醇參數 */
                CholesterolParamBlock cholParams;
                if (!PARAM_ReadParameterBlock(BLOCK_CHOLESTEROL_PARAMS, &cholParams, sizeof(CholesterolParamBlock))) {
                    return false;
                }
                if (lowLimit) *lowLimit = cholParams.cL;
                if (highLimit) *highLimit = cholParams.cH;
            }
            break;
            
        case STRIP_TYPE_TG:
            {
                /* 三酸甘油脂參數 */
                TriglycerideParamBlock tgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_TRIGLYCERIDE_PARAMS, &tgParams, sizeof(TriglycerideParamBlock))) {
                    return false;
                }
                if (lowLimit) *lowLimit = tgParams.tgL;
                if (highLimit) *highLimit = tgParams.tgH;
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief 根據試片類型獲取時序參數
 * @param stripType: 試片類型
 * @param tpl: TPL時間指針
 * @param trd: TRD時間指針
 * @param evWidth: EV寬度指針
 * @param group: 時序組別 (1或2)
 * @return 操作是否成功
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    /* 檢查時序組別有效性 */
    if (group != 1 && group != 2) {
        return false;
    }
    
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* 血糖參數 */
                GlucoseParamBlock bgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_GLUCOSE_PARAMS, &bgParams, sizeof(GlucoseParamBlock))) {
                    return false;
                }
                
                if (group == 1) {
                    if (tpl) *tpl = bgParams.bgTPL1;
                    if (trd) *trd = bgParams.bgTRD1;
                    if (evWidth) *evWidth = bgParams.bgEVWidth1;
                } else {
                    if (tpl) *tpl = bgParams.bgTPL2;
                    if (trd) *trd = bgParams.bgTRD2;
                    if (evWidth) *evWidth = bgParams.bgEVWidth2;
                }
            }
            break;
            
        case STRIP_TYPE_U:
            {
                /* 尿酸參數 */
                UricAcidParamBlock uaParams;
                if (!PARAM_ReadParameterBlock(BLOCK_URIC_ACID_PARAMS, &uaParams, sizeof(UricAcidParamBlock))) {
                    return false;
                }
                
                if (group == 1) {
                    if (tpl) *tpl = uaParams.uTPL1;
                    if (trd) *trd = uaParams.uTRD1;
                    if (evWidth) *evWidth = uaParams.uEVWidth1;
                } else {
                    if (tpl) *tpl = uaParams.uTPL2;
                    if (trd) *trd = uaParams.uTRD2;
                    if (evWidth) *evWidth = uaParams.uEVWidth2;
                }
            }
            break;
            
        case STRIP_TYPE_C:
            {
                /* 總膽固醇參數 */
                CholesterolParamBlock cholParams;
                if (!PARAM_ReadParameterBlock(BLOCK_CHOLESTEROL_PARAMS, &cholParams, sizeof(CholesterolParamBlock))) {
                    return false;
                }
                
                if (group == 1) {
                    if (tpl) *tpl = cholParams.cTPL1;
                    if (trd) *trd = cholParams.cTRD1;
                    if (evWidth) *evWidth = cholParams.cEVWidth1;
                } else {
                    if (tpl) *tpl = cholParams.cTPL2;
                    if (trd) *trd = cholParams.cTRD2;
                    if (evWidth) *evWidth = cholParams.cEVWidth2;
                }
            }
            break;
            
        case STRIP_TYPE_TG:
            {
                /* 三酸甘油脂參數 */
                TriglycerideParamBlock tgParams;
                if (!PARAM_ReadParameterBlock(BLOCK_TRIGLYCERIDE_PARAMS, &tgParams, sizeof(TriglycerideParamBlock))) {
                    return false;
                }
                
                if (group == 1) {
                    if (tpl) *tpl = tgParams.tgTPL1;
                    if (trd) *trd = tgParams.tgTRD1;
                    if (evWidth) *evWidth = tgParams.tgEVWidth1;
                } else {
                    if (tpl) *tpl = tgParams.tgTPL2;
                    if (trd) *trd = tgParams.tgTRD2;
                    if (evWidth) *evWidth = tgParams.tgEVWidth2;
                }
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief 重置參數為默認值
 */
void PARAM_ResetToDefault(void)
{
    /* 複製默認參數 */
    memcpy(&g_paramBlock, &g_defaultParams, sizeof(ParameterBlock));
    
    /* 更新校驗和 */
    UpdateChecksum(&g_paramBlock);
    
    /* 保存到Flash */
    SaveParameterBlock();
    
    printf("參數已重置為默認值\r\n");
} 