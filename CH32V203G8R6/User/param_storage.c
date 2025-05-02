/**
 * @file param_storage.c
 * @brief CH32V203G8R6多功能參數Flash儲存方案實現文件
 * @version 1.0
 * @date 2024-09-25
 * 
 * @copyright Copyright (c) 2024 HMD Biomedical Inc.
 */

#include "param_storage.h"
#include "string.h"
#include "ch32v20x_flash.h"
#include <stdbool.h>

/* 全局變量 */
static uint32_t g_activeBlockAddr = 0;   // 當前活動參數區地址
static bool g_isInitialized = false;     // 參數存儲是否已初始化

/* Flash操作輔助函數 */

/**
 * @brief 查找有效的參數區塊地址
 * 
 * @return uint32_t 有效的參數區地址，若沒有則返回0
 */
static uint32_t FindActiveParameterBlock(void)
{
    ParameterBlock blockA, blockB;
    bool validA = false, validB = false;
    uint8_t counterA = 0, counterB = 0;
    
    // 讀取參數區A
    memcpy(&blockA, (void *)PARAM_BLOCK_A_ADDR, sizeof(ParameterBlock));
    if (blockA.header.magic == 0xA55A) {
        validA = true;
        counterA = blockA.header.writeCounter;
    }
    
    // 讀取參數區B
    memcpy(&blockB, (void *)PARAM_BLOCK_B_ADDR, sizeof(ParameterBlock));
    if (blockB.header.magic == 0xA55A) {
        validB = true;
        counterB = blockB.header.writeCounter;
    }
    
    // 判斷哪個參數區有效且是最新的
    if (validA && validB) {
        if ((uint8_t)(counterA - counterB) < 128) {
            return PARAM_BLOCK_A_ADDR;
        } else {
            return PARAM_BLOCK_B_ADDR;
        }
    } else if (validA) {
        return PARAM_BLOCK_A_ADDR;
    } else if (validB) {
        return PARAM_BLOCK_B_ADDR;
    }
    
    return 0; // 沒有有效的參數區
}

/**
 * @brief 計算區塊校驗和
 * 
 * @param data 數據指針
 * @param size 數據大小
 * @return uint16_t 校驗和
 */
static uint16_t CalculateChecksum(const void *data, uint16_t size)
{
    uint16_t checksum = 0;
    const uint8_t *ptr = (const uint8_t *)data;
    
    for (uint16_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    
    return checksum;
}

/**
 * @brief 計算區塊CRC8
 * 
 * @param data 數據指針
 * @param size 數據大小
 * @return uint8_t CRC8校驗值
 */
static uint8_t CalculateCRC8(const void *data, uint16_t size)
{
    uint8_t crc = 0;
    const uint8_t *ptr = (const uint8_t *)data;
    
    for (uint16_t i = 0; i < size; i++) {
        crc ^= ptr[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31; // CRC-8多項式
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief 寫入Flash緩衝區
 * 
 * @param address 目標Flash地址
 * @param buffer 數據緩衝區
 * @param size 數據大小
 * @return true 寫入成功
 * @return false 寫入失敗
 */
static bool FlashWrite(uint32_t address, const uint8_t *buffer, uint32_t size)
{
    uint32_t i;
    uint32_t *dst = (uint32_t*)address;
    const uint32_t *src = (const uint32_t*)buffer;
    uint32_t wordCount = (size + 3) / 4; // 向上取整至字大小
    
    FLASH_Unlock();
    
    for (i = 0; i < wordCount; i++) {
        FLASH_ProgramWord((uint32_t)dst, *src);
        if (*(uint32_t*)dst != *src) {
            FLASH_Lock();
            return false;
        }
        dst++;
        src++;
    }
    
    FLASH_Lock();
    return true;
}

/**
 * @brief 擦除Flash頁
 * 
 * @param address 頁起始地址
 * @return true 擦除成功
 * @return false 擦除失敗
 */
static bool FlashErasePage(uint32_t address)
{
    uint32_t pageAddr = address & 0xFFFFF800; // 對齊到頁邊界(2KB)
    FLASH_Unlock();
    FLASH_Status status = FLASH_ErasePage(pageAddr);
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE);
}

/**
 * @brief 獲取參數區塊偏移地址
 * 
 * @param blockId 區塊ID
 * @return uint32_t 區塊在參數區中的偏移地址，若無效則返回0
 */
static uint32_t GetBlockOffset(uint8_t blockId)
{
    uint32_t offset = 0;
    
    switch (blockId) {
        case BLOCK_BASIC_SYSTEM:
            offset = offsetof(ParameterBlock, params.basicSystem);
            break;
        case BLOCK_HARDWARE_CALIB:
            offset = offsetof(ParameterBlock, params.hardwareCalib);
            break;
        case BLOCK_BG_PARAMS:
            offset = offsetof(ParameterBlock, params.bgParams);
            break;
        case BLOCK_UA_PARAMS:
            offset = offsetof(ParameterBlock, params.uaParams);
            break;
        case BLOCK_CHOL_PARAMS:
            offset = offsetof(ParameterBlock, params.cholParams);
            break;
        case BLOCK_TG_PARAMS:
            offset = offsetof(ParameterBlock, params.tgParams);
            break;
        case BLOCK_RESERVED:
            offset = offsetof(ParameterBlock, params.reserved);
            break;
        default:
            offset = 0;
            break;
    }
    
    return offset;
}

/**
 * @brief 獲取參數區塊大小
 * 
 * @param blockId 區塊ID
 * @return uint16_t 區塊大小，若無效則返回0
 */
static uint16_t GetBlockSize(uint8_t blockId)
{
    uint16_t size = 0;
    
    switch (blockId) {
        case BLOCK_BASIC_SYSTEM:
            size = sizeof(BasicSystemBlock);
            break;
        case BLOCK_HARDWARE_CALIB:
            size = sizeof(HardwareCalibBlock);
            break;
        case BLOCK_BG_PARAMS:
            size = sizeof(BloodGlucoseBlock);
            break;
        case BLOCK_UA_PARAMS:
            size = sizeof(UricAcidBlock);
            break;
        case BLOCK_CHOL_PARAMS:
            size = sizeof(CholesterolBlock);
            break;
        case BLOCK_TG_PARAMS:
            size = sizeof(TriglycerideBlock);
            break;
        case BLOCK_RESERVED:
            size = sizeof(ReservedBlock);
            break;
        default:
            size = 0;
            break;
    }
    
    return size;
}

/**
 * @brief 更新區塊校驗和
 * 
 * @param block 參數區塊指針
 */
static void UpdateBlockChecksum(ParameterBlock *block)
{
    uint16_t checksum;
    uint8_t crc;
    
    // 計算校驗和
    checksum = CalculateChecksum(block, offsetof(ParameterBlock, params.reserved.checksum));
    block->params.reserved.checksum = checksum;
    
    // 計算CRC
    crc = CalculateCRC8(block, sizeof(ParameterBlock) - 1);
    block->params.reserved.crc = crc;
}

/**
 * @brief 設定參數區塊默認值
 * 
 * @param block 參數區塊指針
 */
static void SetDefaultParameters(ParameterBlock *block)
{
    memset(block, 0, sizeof(ParameterBlock));
    
    // 設定區塊頭部
    block->header.magic = 0xA55A;
    block->header.version = 1;
    block->header.writeCounter = 1;
    block->header.timestamp = 0;
    
    // 設定基本系統參數
    block->params.basicSystem.lbt = 58;                 // 2.58V
    block->params.basicSystem.obt = 55;                 // 2.55V
    block->params.basicSystem.factory = 0;              // 用戶模式
    block->params.basicSystem.modelNo = 1;              // 產品型號
    block->params.basicSystem.fwNo = 10;                // 韌體版本 V1.0
    block->params.basicSystem.testCount = 0;            // 測試次數
    block->params.basicSystem.codeTableVer = 1;         // 代碼表版本
    
    // 設定默認時間 (2024-09-25 00:00:00)
    block->params.basicSystem.year = 24;
    block->params.basicSystem.month = 9;
    block->params.basicSystem.date = 25;
    block->params.basicSystem.hour = 0;
    block->params.basicSystem.minute = 0;
    block->params.basicSystem.second = 0;
    
    // 設定測試環境參數
    block->params.basicSystem.tempLowLimit = 10;        // 10°C
    block->params.basicSystem.tempHighLimit = 40;       // 40°C
    block->params.basicSystem.measureUnit = 1;          // mg/dL
    block->params.basicSystem.defaultEvent = 1;         // AC
    block->params.basicSystem.stripType = 0;            // GLV(血糖)
    
    // 設定硬體校準參數
    block->params.hardwareCalib.evT3Trigger = 800;      // 800mV
    block->params.hardwareCalib.evWorking = 164;
    block->params.hardwareCalib.evT3 = 164;
    block->params.hardwareCalib.dacoOffset = 0;
    block->params.hardwareCalib.dacdo = 0;
    block->params.hardwareCalib.cc211Status = 0;
    block->params.hardwareCalib.calTolerance = 8;       // 0.8%
    block->params.hardwareCalib.ops = 1.0f;
    block->params.hardwareCalib.opi = 0.0f;
    block->params.hardwareCalib.qct = 0;
    block->params.hardwareCalib.tempOffset = 0;
    block->params.hardwareCalib.batteryOffset = 0;
    
    // 設定各測試項目參數
    // 血糖參數
    block->params.bgParams.bgCsuTolerance = 10;
    block->params.bgParams.bgNdl = 100;
    block->params.bgParams.bgUdl = 50;
    block->params.bgParams.bgBloodIn = 150;
    strcpy((char *)block->params.bgParams.bgStripLot, "GLV24092500");
    block->params.bgParams.bgL = 2;                     // 20 mg/dL
    block->params.bgParams.bgH = 60;                    // 600 mg/dL
    block->params.bgParams.bgT3E37 = 2000;
    
    // 設定測試時序參數
    block->params.bgParams.bgTPL1 = 10;                 // 1秒
    block->params.bgParams.bgTRD1 = 10;                 // 1秒
    block->params.bgParams.bgEVWidth1 = 5;              // 0.5秒
    block->params.bgParams.bgTPL2 = 20;                 // 2秒
    block->params.bgParams.bgTRD2 = 20;                 // 2秒
    block->params.bgParams.bgEVWidth2 = 10;             // 1秒
    
    // 設定其他測試項目的默認參數 (尿酸、總膽固醇、三酸甘油脂)
    // 尿酸參數
    block->params.uaParams.uCsuTolerance = 10;
    block->params.uaParams.uNdl = 100;
    block->params.uaParams.uUdl = 50;
    block->params.uaParams.uBloodIn = 150;
    strcpy((char *)block->params.uaParams.uStripLot, "UA24092500");
    block->params.uaParams.uL = 3;                      // 3 mg/dL
    block->params.uaParams.uH = 20;                     // 20 mg/dL
    block->params.uaParams.uT3E37 = 2000;
    
    // 計算校驗和和CRC
    UpdateBlockChecksum(block);
}

/**
 * @brief 驗證參數區塊有效性
 * 
 * @param block 參數區塊指針
 * @return true 參數區塊有效
 * @return false 參數區塊無效
 */
static bool ValidateBlock(const ParameterBlock *block)
{
    uint16_t checksum;
    uint8_t crc;
    
    // 驗證魔數
    if (block->header.magic != 0xA55A) {
        return false;
    }
    
    // 驗證校驗和
    checksum = CalculateChecksum(block, offsetof(ParameterBlock, params.reserved.checksum));
    if (checksum != block->params.reserved.checksum) {
        return false;
    }
    
    // 驗證CRC
    crc = CalculateCRC8(block, sizeof(ParameterBlock) - 1);
    if (crc != block->params.reserved.crc) {
        return false;
    }
    
    return true;
}

/* 公開API函數 */

/**
 * @brief 初始化參數存儲
 */
void PARAM_Init(void)
{
    ParameterBlock block;
    
    // 查找活動參數區塊
    g_activeBlockAddr = FindActiveParameterBlock();
    
    // 如果沒有有效的參數區，則初始化參數並寫入兩個參數區
    if (g_activeBlockAddr == 0) {
        // 設定默認參數
        SetDefaultParameters(&block);
        
        // 擦除並寫入參數區A
        FlashErasePage(PARAM_BLOCK_A_ADDR);
        FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
        
        // 擦除並寫入參數區B
        FlashErasePage(PARAM_BLOCK_B_ADDR);
        FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
        
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    }
    // 驗證參數區塊的有效性
    else {
        ParameterBlock currentBlock;
        memcpy(&currentBlock, (void *)g_activeBlockAddr, sizeof(ParameterBlock));
        
        if (!ValidateBlock(&currentBlock)) {
            // 如果當前活動區塊無效，嘗試使用另一個區塊
            uint32_t backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
            memcpy(&currentBlock, (void *)backupAddr, sizeof(ParameterBlock));
            
            if (ValidateBlock(&currentBlock)) {
                g_activeBlockAddr = backupAddr;
            } else {
                // 兩個區塊都無效，重新初始化
                SetDefaultParameters(&block);
                
                // 擦除並寫入參數區A
                FlashErasePage(PARAM_BLOCK_A_ADDR);
                FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
                
                // 擦除並寫入參數區B
                FlashErasePage(PARAM_BLOCK_B_ADDR);
                FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
                
                g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
            }
        }
    }
    
    g_isInitialized = true;
}

/**
 * @brief 讀取參數區塊
 * 
 * @param blockId 區塊ID
 * @param data 數據緩衝區
 * @param size 數據大小
 * @return true 讀取成功
 * @return false 讀取失敗
 */
bool PARAM_ReadBlock(uint8_t blockId, void *data, uint16_t size)
{
    uint32_t offset;
    uint16_t blockSize;
    
    // 檢查參數存儲是否已初始化
    if (!g_isInitialized) {
        PARAM_Init();
    }
    
    // 檢查區塊ID有效性
    if (blockId >= BLOCK_MAX) {
        return false;
    }
    
    // 檢查緩衝區指針
    if (data == NULL) {
        return false;
    }
    
    // 獲取區塊偏移和大小
    offset = GetBlockOffset(blockId);
    blockSize = GetBlockSize(blockId);
    
    // 檢查偏移和大小有效性
    if (offset == 0 || blockSize == 0) {
        return false;
    }
    
    // 檢查緩衝區大小
    if (size < blockSize) {
        return false;
    }
    
    // 讀取區塊數據
    memcpy(data, (void *)(g_activeBlockAddr + offset), blockSize);
    
    return true;
}

/**
 * @brief 更新參數區塊
 * 
 * @param blockId 區塊ID
 * @param data 數據緩衝區
 * @param size 數據大小
 * @return true 更新成功
 * @return false 更新失敗
 */
bool PARAM_UpdateBlock(uint8_t blockId, const void *data, uint16_t size)
{
    ParameterBlock block;
    uint32_t offset;
    uint16_t blockSize;
    uint32_t backupAddr;
    
    // 檢查參數存儲是否已初始化
    if (!g_isInitialized) {
        PARAM_Init();
    }
    
    // 檢查區塊ID有效性
    if (blockId >= BLOCK_MAX) {
        return false;
    }
    
    // 檢查數據指針
    if (data == NULL) {
        return false;
    }
    
    // 獲取區塊偏移和大小
    offset = GetBlockOffset(blockId);
    blockSize = GetBlockSize(blockId);
    
    // 檢查偏移和大小有效性
    if (offset == 0 || blockSize == 0) {
        return false;
    }
    
    // 檢查數據大小
    if (size > blockSize) {
        return false;
    }
    
    // 讀取當前參數區
    memcpy(&block, (void *)g_activeBlockAddr, sizeof(ParameterBlock));
    
    // 更新區塊數據
    memcpy((uint8_t *)&block + offset, data, size);
    
    // 更新區塊頭部
    block.header.writeCounter++;
    block.header.timestamp++; // 簡單的時間戳，可替換為RTC時間
    
    // 更新校驗和和CRC
    UpdateBlockChecksum(&block);
    
    // 確定備份區地址
    backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    // 擦除備份區
    if (!FlashErasePage(backupAddr)) {
        return false;
    }
    
    // 寫入備份區
    if (!FlashWrite(backupAddr, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // 更新活動參數區地址
    g_activeBlockAddr = backupAddr;
    
    return true;
}

/**
 * @brief 獲取系統時間
 * 
 * @param year 年指針(0-99)
 * @param month 月指針(1-12)
 * @param date 日指針(1-31)
 * @param hour 時指針(0-23)
 * @param minute 分指針(0-59)
 * @param second 秒指針(0-59)
 * @return true 獲取成功
 * @return false 獲取失敗
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 設定返回值
    if (year) *year = basicParams.year;
    if (month) *month = basicParams.month;
    if (date) *date = basicParams.date;
    if (hour) *hour = basicParams.hour;
    if (minute) *minute = basicParams.minute;
    if (second) *second = basicParams.second;
    
    return true;
}

/**
 * @brief 更新系統時間
 * 
 * @param year 年(0-99)
 * @param month 月(1-12)
 * @param date 日(1-31)
 * @param hour 時(0-23)
 * @param minute 分(0-59)
 * @param second 秒(0-59)
 * @return true 更新成功
 * @return false 更新失敗
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 更新時間參數
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    // 更新參數區
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief 增加測試次數
 * 
 * @return true 增加成功
 * @return false 增加失敗
 */
bool PARAM_IncreaseTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 增加測試次數
    basicParams.testCount++;
    
    // 更新參數區
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief 獲取測試次數
 * 
 * @param count 測試次數指針
 * @return true 獲取成功
 * @return false 獲取失敗
 */
bool PARAM_GetTestCount(uint16_t *count)
{
    BasicSystemBlock basicParams;
    
    // 檢查指針有效性
    if (count == NULL) {
        return false;
    }
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 返回測試次數
    *count = basicParams.testCount;
    
    return true;
}

/**
 * @brief 恢復出廠設置
 * 
 * @return true 恢復成功
 * @return false 恢復失敗
 */
bool PARAM_ResetToFactory(void)
{
    ParameterBlock block;
    
    // 設定默認參數
    SetDefaultParameters(&block);
    
    // 擦除並寫入參數區A
    if (!FlashErasePage(PARAM_BLOCK_A_ADDR)) {
        return false;
    }
    
    if (!FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // 擦除並寫入參數區B
    if (!FlashErasePage(PARAM_BLOCK_B_ADDR)) {
        return false;
    }
    
    if (!FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // 更新活動參數區地址
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    
    return true;
}

/**
 * @brief 根據試片類型獲取試片參數
 * 
 * @param stripType 試片類型
 * @param ndl 新試片檢測水平指針
 * @param udl 已使用試片檢測水平指針
 * @param bloodIn 血液檢測水平指針
 * @return true 獲取成功
 * @return false 獲取失敗
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    uint8_t blockId;
    uint16_t ndlVal = 0, udlVal = 0, bloodInVal = 0;
    bool result = false;
    
    // 根據試片類型選擇參數區塊
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                BloodGlucoseBlock bgParams;
                blockId = BLOCK_BG_PARAMS;
                if (PARAM_ReadBlock(blockId, &bgParams, sizeof(BloodGlucoseBlock))) {
                    ndlVal = bgParams.bgNdl;
                    udlVal = bgParams.bgUdl;
                    bloodInVal = bgParams.bgBloodIn;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_UA:
            {
                UricAcidBlock uaParams;
                blockId = BLOCK_UA_PARAMS;
                if (PARAM_ReadBlock(blockId, &uaParams, sizeof(UricAcidBlock))) {
                    ndlVal = uaParams.uNdl;
                    udlVal = uaParams.uUdl;
                    bloodInVal = uaParams.uBloodIn;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_CHOL:
            {
                CholesterolBlock cholParams;
                blockId = BLOCK_CHOL_PARAMS;
                if (PARAM_ReadBlock(blockId, &cholParams, sizeof(CholesterolBlock))) {
                    ndlVal = cholParams.cNdl;
                    udlVal = cholParams.cUdl;
                    bloodInVal = cholParams.cBloodIn;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_TG:
            {
                TriglycerideBlock tgParams;
                blockId = BLOCK_TG_PARAMS;
                if (PARAM_ReadBlock(blockId, &tgParams, sizeof(TriglycerideBlock))) {
                    ndlVal = tgParams.tgNdl;
                    udlVal = tgParams.tgUdl;
                    bloodInVal = tgParams.tgBloodIn;
                    result = true;
                }
            }
            break;
        default:
            result = false;
            break;
    }
    
    // 設定返回值
    if (result) {
        if (ndl) *ndl = ndlVal;
        if (udl) *udl = udlVal;
        if (bloodIn) *bloodIn = bloodInVal;
    }
    
    return result;
}

/**
 * @brief 根據試片類型獲取測量範圍
 * 
 * @param stripType 試片類型
 * @param lowLimit 下限指針
 * @param highLimit 上限指針
 * @return true 獲取成功
 * @return false 獲取失敗
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    uint8_t blockId;
    uint8_t lowVal = 0;
    uint16_t highVal = 0;
    bool result = false;
    
    // 根據試片類型選擇參數區塊
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                BloodGlucoseBlock bgParams;
                blockId = BLOCK_BG_PARAMS;
                if (PARAM_ReadBlock(blockId, &bgParams, sizeof(BloodGlucoseBlock))) {
                    lowVal = bgParams.bgL;
                    highVal = bgParams.bgH;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_UA:
            {
                UricAcidBlock uaParams;
                blockId = BLOCK_UA_PARAMS;
                if (PARAM_ReadBlock(blockId, &uaParams, sizeof(UricAcidBlock))) {
                    lowVal = uaParams.uL;
                    highVal = uaParams.uH;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_CHOL:
            {
                CholesterolBlock cholParams;
                blockId = BLOCK_CHOL_PARAMS;
                if (PARAM_ReadBlock(blockId, &cholParams, sizeof(CholesterolBlock))) {
                    lowVal = cholParams.cL;
                    highVal = cholParams.cH;
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_TG:
            {
                TriglycerideBlock tgParams;
                blockId = BLOCK_TG_PARAMS;
                if (PARAM_ReadBlock(blockId, &tgParams, sizeof(TriglycerideBlock))) {
                    lowVal = tgParams.tgL;
                    highVal = tgParams.tgH;
                    result = true;
                }
            }
            break;
        default:
            result = false;
            break;
    }
    
    // 設定返回值
    if (result) {
        if (lowLimit) *lowLimit = lowVal;
        if (highLimit) *highLimit = highVal;
    }
    
    return result;
}

/**
 * @brief 根據試片類型獲取測試時序參數
 * 
 * @param stripType 試片類型
 * @param tpl 時間脈衝低指針
 * @param trd 原始數據時間指針
 * @param evWidth 燃燒時間指針
 * @param group 參數組別(1或2)
 * @return true 獲取成功
 * @return false 獲取失敗
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    uint8_t blockId;
    uint16_t tplVal = 0, trdVal = 0, evWidthVal = 0;
    bool result = false;
    
    // 檢查參數組別
    if (group != 1 && group != 2) {
        return false;
    }
    
    // 根據試片類型選擇參數區塊
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                BloodGlucoseBlock bgParams;
                blockId = BLOCK_BG_PARAMS;
                if (PARAM_ReadBlock(blockId, &bgParams, sizeof(BloodGlucoseBlock))) {
                    if (group == 1) {
                        tplVal = bgParams.bgTPL1;
                        trdVal = bgParams.bgTRD1;
                        evWidthVal = bgParams.bgEVWidth1;
                    } else {
                        tplVal = bgParams.bgTPL2;
                        trdVal = bgParams.bgTRD2;
                        evWidthVal = bgParams.bgEVWidth2;
                    }
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_UA:
            {
                UricAcidBlock uaParams;
                blockId = BLOCK_UA_PARAMS;
                if (PARAM_ReadBlock(blockId, &uaParams, sizeof(UricAcidBlock))) {
                    if (group == 1) {
                        tplVal = uaParams.uTPL1;
                        trdVal = uaParams.uTRD1;
                        evWidthVal = uaParams.uEVWidth1;
                    } else {
                        tplVal = uaParams.uTPL2;
                        trdVal = uaParams.uTRD2;
                        evWidthVal = uaParams.uEVWidth2;
                    }
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_CHOL:
            {
                CholesterolBlock cholParams;
                blockId = BLOCK_CHOL_PARAMS;
                if (PARAM_ReadBlock(blockId, &cholParams, sizeof(CholesterolBlock))) {
                    if (group == 1) {
                        tplVal = cholParams.cTPL1;
                        trdVal = cholParams.cTRD1;
                        evWidthVal = cholParams.cEVWidth1;
                    } else {
                        tplVal = cholParams.cTPL2;
                        trdVal = cholParams.cTRD2;
                        evWidthVal = cholParams.cEVWidth2;
                    }
                    result = true;
                }
            }
            break;
        case STRIP_TYPE_TG:
            {
                TriglycerideBlock tgParams;
                blockId = BLOCK_TG_PARAMS;
                if (PARAM_ReadBlock(blockId, &tgParams, sizeof(TriglycerideBlock))) {
                    if (group == 1) {
                        tplVal = tgParams.tgTPL1;
                        trdVal = tgParams.tgTRD1;
                        evWidthVal = tgParams.tgEVWidth1;
                    } else {
                        tplVal = tgParams.tgTPL2;
                        trdVal = tgParams.tgTRD2;
                        evWidthVal = tgParams.tgEVWidth2;
                    }
                    result = true;
                }
            }
            break;
        default:
            result = false;
            break;
    }
    
    // 設定返回值
    if (result) {
        if (tpl) *tpl = tplVal;
        if (trd) *trd = trdVal;
        if (evWidth) *evWidth = evWidthVal;
    }
    
    return result;
}

/**
 * @brief 保存測試記錄
 * 
 * @param record 測試記錄指針
 * @return true 保存成功
 * @return false 保存失敗
 */
bool PARAM_SaveTestRecord(const TestRecord *record)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // 測試記錄區起始地址
    uint32_t recordSize = sizeof(TestRecord);     // 記錄大小
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB 記錄區可存儲的記錄數
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkValue;
    
    // 檢查記錄指針
    if (record == NULL) {
        return false;
    }
    
    // 查找下一個可用的記錄位置
    while (addr < endAddr) {
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        if (checkValue == 0xFFFF) { // 假設空記錄的檢驗為0xFFFF
            break;
        }
        addr += recordSize;
    }
    
    // 如果記錄區已滿，則擦除整個記錄區並從頭開始
    if (addr >= endAddr) {
        FLASH_Unlock();
        
        // 擦除整個記錄區(6KB, 6頁)
        for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += 1024) {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        addr = recordBase; // 回到記錄區起始地址
    }
    
    // 寫入新記錄
    return FlashWrite(addr, (const uint8_t *)record, recordSize);
}

/**
 * @brief 讀取測試記錄
 * 
 * @param records 記錄緩衝區
 * @param startIdx 起始索引
 * @param count 請求記錄數
 * @param actualCount 實際讀取的記錄數指針
 * @return true 讀取成功
 * @return false 讀取失敗
 */
bool PARAM_ReadTestRecords(TestRecord *records, uint16_t startIdx, uint16_t count, uint16_t *actualCount)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // 測試記錄區起始地址
    uint32_t recordSize = sizeof(TestRecord);     // 記錄大小
    uint16_t totalCount = 0;
    uint16_t readCount = 0;
    uint16_t checkValue;
    
    // 檢查參數
    if (records == NULL || count == 0) {
        return false;
    }
    
    // 計算總記錄數
    totalCount = PARAM_GetTestRecordCount();
    
    // 檢查起始索引
    if (startIdx >= totalCount) {
        if (actualCount) *actualCount = 0;
        return true;
    }
    
    // 限制讀取數量
    if (startIdx + count > totalCount) {
        count = totalCount - startIdx;
    }
    
    // 讀取記錄
    for (uint16_t i = 0; i < count; i++) {
        uint32_t addr = recordBase + (startIdx + i) * recordSize;
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        
        // 檢查記錄是否有效
        if (checkValue != 0xFFFF) {
            memcpy(&records[readCount], (void *)addr, recordSize);
            readCount++;
        }
    }
    
    // 設定實際讀取數量
    if (actualCount) *actualCount = readCount;
    
    return true;
}

/**
 * @brief 獲取測試記錄總數
 * 
 * @return uint16_t 測試記錄總數
 */
uint16_t PARAM_GetTestRecordCount(void)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // 測試記錄區起始地址
    uint32_t recordSize = sizeof(TestRecord);     // 記錄大小
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB 記錄區可存儲的記錄數
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t count = 0;
    uint16_t checkValue;
    
    // 查找已使用的記錄數
    while (addr < endAddr) {
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        if (checkValue == 0xFFFF) { // 假設空記錄的檢驗為0xFFFF
            break;
        }
        count++;
        addr += recordSize;
    }
    
    return count;
} 