/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : 多功能生化測試儀Flash參數儲存方案實現
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Flash_Storage.h"
#include "ch32v20x_flash.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

/* 全局變數定義 */
static uint32_t g_activeBlockAddr = 0;      // 當前使用的參數塊地址
static ParameterBlock g_paramBlock;         // 參數塊緩存

/*********************************************************************
 * @fn      FLASH_WriteBuffer
 *
 * @brief   寫入數據到Flash
 *
 * @param   address - 目標地址
 * @param   buffer - 數據緩衝區
 * @param   size - 數據大小
 *
 * @return  true: 成功, false: 失敗
 */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *buffer, uint32_t size)
{
    if (!buffer || (size % 2) != 0) {
        return false;  // 大小必須是2的倍數
    }
    
    uint32_t i;
    uint32_t *alignedBuffer = (uint32_t *)buffer;
    FLASH_Status status = FLASH_COMPLETE;
    
    // 如果是奇數字節，補充一個字節
    for (i = 0; i < size / 4; i++) {
        status = FLASH_ProgramWord(address + i * 4, alignedBuffer[i]);
        if (status != FLASH_COMPLETE) {
            return false;
        }
    }
    
    // 如果有剩餘的半字，單獨處理
    if (size % 4 != 0) {
        status = FLASH_ProgramHalfWord(address + i * 4, *((uint16_t *)&buffer[i * 4]));
        if (status != FLASH_COMPLETE) {
            return false;
        }
    }
    
    return true;
}

/*********************************************************************
 * @fn      CalcChecksum
 *
 * @brief   計算數據的校驗和
 *
 * @param   data - 數據緩衝區
 * @param   size - 數據大小
 *
 * @return  16位校驗和
 */
uint16_t CalcChecksum(const uint8_t *data, uint32_t size)
{
    uint16_t sum = 0;
    uint32_t i;
    
    for (i = 0; i < size; i++) {
        sum += data[i];
    }
    
    return sum;
}

/*********************************************************************
 * @fn      CalcCRC16
 *
 * @brief   計算數據的CRC16校驗
 *
 * @param   data - 數據緩衝區
 * @param   size - 數據大小
 *
 * @return  16位CRC值
 */
uint16_t CalcCRC16(const uint8_t *data, uint32_t size)
{
    uint16_t crc = 0xFFFF;
    uint32_t i, j;
    
    for (i = 0; i < size; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

/*********************************************************************
 * @fn      UpdateChecksum
 *
 * @brief   更新參數塊的校驗和和CRC
 *
 * @param   block - 參數塊指針
 *
 * @return  none
 */
void UpdateChecksum(ParameterBlock *block)
{
    if (!block) {
        return;
    }
    
    // 計算參數數據的校驗和
    block->header.checksum = CalcChecksum((uint8_t *)&block->params, sizeof(ParameterData) - sizeof(ReservedBlock));
    
    // 計算保留區之前的所有數據的CRC16校驗
    block->header.crc16 = CalcCRC16((uint8_t *)block, 
                                    sizeof(ParamBlockHeader) + sizeof(ParameterData) - sizeof(ReservedBlock));
    
    // 更新保留區中的校驗值
    block->params.reserved.checksum = block->header.checksum;
    block->params.reserved.crc16 = block->header.crc16;
}

/*********************************************************************
 * @fn      FindActiveParameterBlock
 *
 * @brief   尋找當前活動的參數塊
 *
 * @return  活動參數塊地址，如果找不到則返回0
 */
static uint32_t FindActiveParameterBlock(void)
{
    ParamBlockHeader headerA, headerB;
    
    // 讀取A區和B區的頭部信息
    memcpy(&headerA, (void *)PARAM_BLOCK_A_ADDR, sizeof(ParamBlockHeader));
    memcpy(&headerB, (void *)PARAM_BLOCK_B_ADDR, sizeof(ParamBlockHeader));
    
    // 檢查魔數
    if (headerA.magic != 0xA55A && headerB.magic != 0xA55A) {
        // 兩個區域都無效，返回0
        return 0;
    }
    
    // 如果只有一個區域有效，返回該區域
    if (headerA.magic != 0xA55A) {
        return PARAM_BLOCK_B_ADDR;
    }
    
    if (headerB.magic != 0xA55A) {
        return PARAM_BLOCK_A_ADDR;
    }
    
    // 兩個區域都有效，比較寫入計數器
    if (headerA.writeCounter > headerB.writeCounter) {
        return PARAM_BLOCK_A_ADDR;
    } else {
        return PARAM_BLOCK_B_ADDR;
    }
}

/*********************************************************************
 * @fn      LoadParameterBlock
 *
 * @brief   載入參數塊到緩存
 *
 * @param   blockAddr - 參數塊地址
 *
 * @return  true: 成功, false: 失敗
 */
static bool LoadParameterBlock(uint32_t blockAddr)
{
    if (!blockAddr) {
        return false;
    }
    
    // 從Flash中讀取參數塊
    memcpy(&g_paramBlock, (void *)blockAddr, sizeof(ParameterBlock));
    
    // 檢查魔數
    if (g_paramBlock.header.magic != 0xA55A) {
        return false;
    }
    
    // 檢查CRC和校驗和
    uint16_t calcChecksum = CalcChecksum((uint8_t *)&g_paramBlock.params, 
                                          sizeof(ParameterData) - sizeof(ReservedBlock));
    
    uint16_t calcCRC = CalcCRC16((uint8_t *)&g_paramBlock, 
                                 sizeof(ParamBlockHeader) + sizeof(ParameterData) - sizeof(ReservedBlock));
    
    if (calcChecksum != g_paramBlock.header.checksum || calcCRC != g_paramBlock.header.crc16) {
        printf("參數區校驗失敗: Checksum=%04X(計算值=%04X), CRC=%04X(計算值=%04X)\r\n",
                g_paramBlock.header.checksum, calcChecksum, g_paramBlock.header.crc16, calcCRC);
        return false;
    }
    
    // 更新活動區域地址
    g_activeBlockAddr = blockAddr;
    
    return true;
}

/*********************************************************************
 * @fn      SaveParameterBlock
 *
 * @brief   保存緩存中的參數塊到Flash
 *
 * @return  true: 成功, false: 失敗
 */
static bool SaveParameterBlock(void)
{
    uint32_t targetAddr;
    
    // 增加寫入計數器
    g_paramBlock.header.writeCounter++;
    
    // 更新校驗和
    UpdateChecksum(&g_paramBlock);
    
    // 選擇目標地址（與當前活動區域相反的區域）
    if (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) {
        targetAddr = PARAM_BLOCK_B_ADDR;
    } else {
        targetAddr = PARAM_BLOCK_A_ADDR;
    }
    
    // 解鎖Flash
    FLASH_Unlock();
    
    // 擦除目標頁
    FLASH_Status status = FLASH_ErasePage(targetAddr);
    if (status != FLASH_COMPLETE) {
        FLASH_Lock();
        return false;
    }
    
    // 寫入整個參數塊
    bool result = FLASH_WriteBuffer(targetAddr, (uint8_t *)&g_paramBlock, sizeof(ParameterBlock));
    
    // 鎖定Flash
    FLASH_Lock();
    
    if (result) {
        // 更新活動區域地址
        g_activeBlockAddr = targetAddr;
    }
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   初始化參數存儲系統
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_Init(void)
{
    printf("初始化參數儲存系統...\r\n");
    
    // 尋找活動參數塊
    uint32_t activeAddr = FindActiveParameterBlock();
    
    // 嘗試載入參數塊
    if (activeAddr && LoadParameterBlock(activeAddr)) {
        printf("成功載入參數塊，地址: 0x%08X\r\n", activeAddr);
        return true;
    }
    
    // 找不到有效參數塊，載入默認值
    printf("未找到有效參數塊，載入默認值\r\n");
    return PARAM_LoadDefault();
}

/*********************************************************************
 * @fn      PARAM_LoadDefault
 *
 * @brief   載入默認參數
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_LoadDefault(void)
{
    // 清除參數塊
    memset(&g_paramBlock, 0, sizeof(ParameterBlock));
    
    // 設置標頭信息
    g_paramBlock.header.magic = 0xA55A;
    g_paramBlock.header.version = 1;
    g_paramBlock.header.writeCounter = 1;
    g_paramBlock.header.timestamp = 0;
    
    // 設置基本系統參數
    g_paramBlock.params.basicSystem.lbt = 58;  // 2.58V
    g_paramBlock.params.basicSystem.obt = 55;  // 2.55V
    g_paramBlock.params.basicSystem.factory = 0; // 使用者模式
    g_paramBlock.params.basicSystem.modelNo = 1;
    g_paramBlock.params.basicSystem.fwNo = 10; // 版本1.0
    g_paramBlock.params.basicSystem.testCount = 0;
    g_paramBlock.params.basicSystem.codeTableVer = 1;
    
    // 設置默認時間 (2024-08-12 00:00:00)
    g_paramBlock.params.basicSystem.year = 24;
    g_paramBlock.params.basicSystem.month = 8;
    g_paramBlock.params.basicSystem.date = 12;
    g_paramBlock.params.basicSystem.hour = 0;
    g_paramBlock.params.basicSystem.minute = 0;
    g_paramBlock.params.basicSystem.second = 0;
    
    // 設置環境參數
    g_paramBlock.params.basicSystem.tempLowLimit = 10;  // 10℃
    g_paramBlock.params.basicSystem.tempHighLimit = 40; // 40℃
    g_paramBlock.params.basicSystem.measureUnit = 1;    // mg/dL
    g_paramBlock.params.basicSystem.defaultEvent = 1;   // AC
    g_paramBlock.params.basicSystem.stripType = 0;      // GLV(血糖)
    
    // 設置硬體校準參數
    g_paramBlock.params.hardwareCalib.evT3Trigger = 800;  // 800mV
    g_paramBlock.params.hardwareCalib.evWorking = 164;
    g_paramBlock.params.hardwareCalib.evT3 = 164;
    g_paramBlock.params.hardwareCalib.dacoOffset = 0;
    g_paramBlock.params.hardwareCalib.dacdo = 0;
    g_paramBlock.params.hardwareCalib.cc211Status = 0;
    g_paramBlock.params.hardwareCalib.calTolerance = 8;  // 0.8%
    g_paramBlock.params.hardwareCalib.ops = 1.0f;       // 校準斜率為1.0
    g_paramBlock.params.hardwareCalib.opi = 0.0f;       // 校準截距為0.0
    g_paramBlock.params.hardwareCalib.qct = 0;
    g_paramBlock.params.hardwareCalib.tempOffset = 0;   // 溫度偏移校準
    g_paramBlock.params.hardwareCalib.batteryOffset = 0; // 電池偏移校準
    
    // 設置血糖參數
    g_paramBlock.params.bgParams.bgCsuTolerance = 10;  // 3%容差
    g_paramBlock.params.bgParams.bgNdl = 100;  // 新試片檢測水平
    g_paramBlock.params.bgParams.bgUdl = 50;   // 已使用試片檢測水平
    g_paramBlock.params.bgParams.bgBloodIn = 150; // 血液檢測水平
    memset(g_paramBlock.params.bgParams.bgStripLot, 0, 16); // 清空試片批號
    
    g_paramBlock.params.bgParams.bgL = 2;    // 低於20 mg/dL顯示"LO"
    g_paramBlock.params.bgParams.bgH = 60;   // 高於600 mg/dL顯示"HI"
    g_paramBlock.params.bgParams.bgT3E37 = 2000;  // T3 ADV閾值
    
    // 設置血糖測試時序參數
    g_paramBlock.params.bgParams.bgTPL1 = 10;        // 1秒
    g_paramBlock.params.bgParams.bgTRD1 = 10;        // 1秒
    g_paramBlock.params.bgParams.bgEVWidth1 = 5;     // 0.5秒
    g_paramBlock.params.bgParams.bgTPL2 = 20;        // 2秒
    g_paramBlock.params.bgParams.bgTRD2 = 20;        // 2秒
    g_paramBlock.params.bgParams.bgEVWidth2 = 10;    // 1秒
    
    // 設置尿酸參數
    g_paramBlock.params.uParams.uCsuTolerance = 10;
    g_paramBlock.params.uParams.uNdl = 100;
    g_paramBlock.params.uParams.uUdl = 50;
    g_paramBlock.params.uParams.uBloodIn = 150;
    memset(g_paramBlock.params.uParams.uStripLot, 0, 16);
    
    g_paramBlock.params.uParams.uL = 2;
    g_paramBlock.params.uParams.uH = 60;
    g_paramBlock.params.uParams.uT3E37 = 2000;
    
    g_paramBlock.params.uParams.uTPL1 = 15;
    g_paramBlock.params.uParams.uTRD1 = 15;
    g_paramBlock.params.uParams.uEVWidth1 = 5;
    g_paramBlock.params.uParams.uTPL2 = 25;
    g_paramBlock.params.uParams.uTRD2 = 25;
    g_paramBlock.params.uParams.uEVWidth2 = 10;
    
    // 設置膽固醇參數
    g_paramBlock.params.cParams.cCsuTolerance = 10;
    g_paramBlock.params.cParams.cNdl = 100;
    g_paramBlock.params.cParams.cUdl = 50;
    g_paramBlock.params.cParams.cBloodIn = 150;
    memset(g_paramBlock.params.cParams.cStripLot, 0, 16);
    
    g_paramBlock.params.cParams.cL = 2;
    g_paramBlock.params.cParams.cH = 60;
    g_paramBlock.params.cParams.cT3E37 = 2000;
    
    g_paramBlock.params.cParams.cTPL1 = 20;
    g_paramBlock.params.cParams.cTRD1 = 20;
    g_paramBlock.params.cParams.cEVWidth1 = 5;
    g_paramBlock.params.cParams.cTPL2 = 30;
    g_paramBlock.params.cParams.cTRD2 = 30;
    g_paramBlock.params.cParams.cEVWidth2 = 10;
    
    // 設置三酸甘油脂參數
    g_paramBlock.params.tgParams.tgCsuTolerance = 10;
    g_paramBlock.params.tgParams.tgNdl = 100;
    g_paramBlock.params.tgParams.tgUdl = 50;
    g_paramBlock.params.tgParams.tgBloodIn = 150;
    memset(g_paramBlock.params.tgParams.tgStripLot, 0, 16);
    
    g_paramBlock.params.tgParams.tgL = 2;
    g_paramBlock.params.tgParams.tgH = 60;
    g_paramBlock.params.tgParams.tgT3E37 = 2000;
    
    g_paramBlock.params.tgParams.tgTPL1 = 25;
    g_paramBlock.params.tgParams.tgTRD1 = 25;
    g_paramBlock.params.tgParams.tgEVWidth1 = 5;
    g_paramBlock.params.tgParams.tgTPL2 = 35;
    g_paramBlock.params.tgParams.tgTRD2 = 35;
    g_paramBlock.params.tgParams.tgEVWidth2 = 10;
    
    // 保存參數塊
    if (!SaveParameterBlock()) {
        printf("保存默認參數失敗\r\n");
        return false;
    }
    
    printf("成功載入並保存默認參數\r\n");
    return true;
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   讀取參數區塊
 *
 * @param   blockType - 區塊類型
 * @param   buffer - 目標緩衝區
 * @param   size - 數據大小
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_ReadBlock(uint8_t blockType, void *buffer, uint16_t size)
{
    if (!buffer || !g_activeBlockAddr) {
        return false;
    }
    
    switch (blockType) {
        case BLOCK_BASIC_SYSTEM:
            if (size > sizeof(BasicSystemBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.basicSystem, size);
            break;
            
        case BLOCK_HARDWARE_CALIB:
            if (size > sizeof(HardwareCalibBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.hardwareCalib, size);
            break;
            
        case BLOCK_BG_PARAMS:
            if (size > sizeof(BGParameterBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.bgParams, size);
            break;
            
        case BLOCK_U_PARAMS:
            if (size > sizeof(UricAcidParameterBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.uParams, size);
            break;
            
        case BLOCK_C_PARAMS:
            if (size > sizeof(CholesterolParameterBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.cParams, size);
            break;
            
        case BLOCK_TG_PARAMS:
            if (size > sizeof(TriglycerideParameterBlock)) {
                return false;
            }
            memcpy(buffer, &g_paramBlock.params.tgParams, size);
            break;
            
        default:
            return false;
    }
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_UpdateBlock
 *
 * @brief   更新參數區塊
 *
 * @param   blockType - 區塊類型
 * @param   buffer - 源數據緩衝區
 * @param   size - 數據大小
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size)
{
    if (!buffer || !g_activeBlockAddr) {
        return false;
    }
    
    switch (blockType) {
        case BLOCK_BASIC_SYSTEM:
            if (size > sizeof(BasicSystemBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.basicSystem, buffer, size);
            break;
            
        case BLOCK_HARDWARE_CALIB:
            if (size > sizeof(HardwareCalibBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.hardwareCalib, buffer, size);
            break;
            
        case BLOCK_BG_PARAMS:
            if (size > sizeof(BGParameterBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.bgParams, buffer, size);
            break;
            
        case BLOCK_U_PARAMS:
            if (size > sizeof(UricAcidParameterBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.uParams, buffer, size);
            break;
            
        case BLOCK_C_PARAMS:
            if (size > sizeof(CholesterolParameterBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.cParams, buffer, size);
            break;
            
        case BLOCK_TG_PARAMS:
            if (size > sizeof(TriglycerideParameterBlock)) {
                return false;
            }
            memcpy(&g_paramBlock.params.tgParams, buffer, size);
            break;
            
        default:
            return false;
    }
    
    // 保存更新後的參數
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   獲取系統時間
 *
 * @param   year, month, date, hour, minute, second - 時間參數指針
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    if (year) {
        *year = g_paramBlock.params.basicSystem.year;
    }
    
    if (month) {
        *month = g_paramBlock.params.basicSystem.month;
    }
    
    if (date) {
        *date = g_paramBlock.params.basicSystem.date;
    }
    
    if (hour) {
        *hour = g_paramBlock.params.basicSystem.hour;
    }
    
    if (minute) {
        *minute = g_paramBlock.params.basicSystem.minute;
    }
    
    if (second) {
        *second = g_paramBlock.params.basicSystem.second;
    }
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_UpdateDateTime
 *
 * @brief   更新系統時間
 *
 * @param   year, month, date, hour, minute, second - 時間參數
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    // 檢查參數有效性
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    
    g_paramBlock.params.basicSystem.year = year;
    g_paramBlock.params.basicSystem.month = month;
    g_paramBlock.params.basicSystem.date = date;
    g_paramBlock.params.basicSystem.hour = hour;
    g_paramBlock.params.basicSystem.minute = minute;
    g_paramBlock.params.basicSystem.second = second;
    
    // 保存更新後的參數
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetStripParametersByStripType
 *
 * @brief   根據試片類型獲取測量水平參數
 *
 * @param   stripType - 試片類型
 * @param   ndl - 新試片測量水平指針
 * @param   udl - 已使用試片測量水平指針
 * @param   bloodIn - 血液測量水平指針
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(血糖)
        case 4:  // GAV(血糖)
            if (ndl) {
                *ndl = g_paramBlock.params.bgParams.bgNdl;
            }
            if (udl) {
                *udl = g_paramBlock.params.bgParams.bgUdl;
            }
            if (bloodIn) {
                *bloodIn = g_paramBlock.params.bgParams.bgBloodIn;
            }
            break;
            
        case 1:  // U(尿酸)
            if (ndl) {
                *ndl = g_paramBlock.params.uParams.uNdl;
            }
            if (udl) {
                *udl = g_paramBlock.params.uParams.uUdl;
            }
            if (bloodIn) {
                *bloodIn = g_paramBlock.params.uParams.uBloodIn;
            }
            break;
            
        case 2:  // C(總膽固醇)
            if (ndl) {
                *ndl = g_paramBlock.params.cParams.cNdl;
            }
            if (udl) {
                *udl = g_paramBlock.params.cParams.cUdl;
            }
            if (bloodIn) {
                *bloodIn = g_paramBlock.params.cParams.cBloodIn;
            }
            break;
            
        case 3:  // TG(三酸甘油脂)
            if (ndl) {
                *ndl = g_paramBlock.params.tgParams.tgNdl;
            }
            if (udl) {
                *udl = g_paramBlock.params.tgParams.tgUdl;
            }
            if (bloodIn) {
                *bloodIn = g_paramBlock.params.tgParams.tgBloodIn;
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_GetMeasurementRangeByStripType
 *
 * @brief   根據試片類型獲取測量範圍參數
 *
 * @param   stripType - 試片類型
 * @param   lowLimit - 下限指針
 * @param   highLimit - 上限指針
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(血糖)
        case 4:  // GAV(血糖)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.bgParams.bgL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.bgParams.bgH;
            }
            break;
            
        case 1:  // U(尿酸)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.uParams.uL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.uParams.uH;
            }
            break;
            
        case 2:  // C(總膽固醇)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.cParams.cL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.cParams.cH;
            }
            break;
            
        case 3:  // TG(三酸甘油脂)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.tgParams.tgL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.tgParams.tgH;
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_GetTimingParametersByStripType
 *
 * @brief   根據試片類型獲取測試時序參數
 *
 * @param   stripType - 試片類型
 * @param   tpl - 時間脈衝寬度指針
 * @param   trd - 原始數據時間指針
 * @param   evWidth - 熔燒時間指針
 * @param   group - 參數組(1或2)
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    if (!g_activeBlockAddr || (group != 1 && group != 2)) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(血糖)
        case 4:  // GAV(血糖)
            if (group == 1) {
                if (tpl) {
                    *tpl = g_paramBlock.params.bgParams.bgTPL1;
                }
                if (trd) {
                    *trd = g_paramBlock.params.bgParams.bgTRD1;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.bgParams.bgEVWidth1;
                }
            } else {
                if (tpl) {
                    *tpl = g_paramBlock.params.bgParams.bgTPL2;
                }
                if (trd) {
                    *trd = g_paramBlock.params.bgParams.bgTRD2;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.bgParams.bgEVWidth2;
                }
            }
            break;
            
        case 1:  // U(尿酸)
            if (group == 1) {
                if (tpl) {
                    *tpl = g_paramBlock.params.uParams.uTPL1;
                }
                if (trd) {
                    *trd = g_paramBlock.params.uParams.uTRD1;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.uParams.uEVWidth1;
                }
            } else {
                if (tpl) {
                    *tpl = g_paramBlock.params.uParams.uTPL2;
                }
                if (trd) {
                    *trd = g_paramBlock.params.uParams.uTRD2;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.uParams.uEVWidth2;
                }
            }
            break;
            
        case 2:  // C(總膽固醇)
            if (group == 1) {
                if (tpl) {
                    *tpl = g_paramBlock.params.cParams.cTPL1;
                }
                if (trd) {
                    *trd = g_paramBlock.params.cParams.cTRD1;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.cParams.cEVWidth1;
                }
            } else {
                if (tpl) {
                    *tpl = g_paramBlock.params.cParams.cTPL2;
                }
                if (trd) {
                    *trd = g_paramBlock.params.cParams.cTRD2;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.cParams.cEVWidth2;
                }
            }
            break;
            
        case 3:  // TG(三酸甘油脂)
            if (group == 1) {
                if (tpl) {
                    *tpl = g_paramBlock.params.tgParams.tgTPL1;
                }
                if (trd) {
                    *trd = g_paramBlock.params.tgParams.tgTRD1;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.tgParams.tgEVWidth1;
                }
            } else {
                if (tpl) {
                    *tpl = g_paramBlock.params.tgParams.tgTPL2;
                }
                if (trd) {
                    *trd = g_paramBlock.params.tgParams.tgTRD2;
                }
                if (evWidth) {
                    *evWidth = g_paramBlock.params.tgParams.tgEVWidth2;
                }
            }
            break;
            
        default:
            return false;
    }
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_IncreaseTestCount
 *
 * @brief   增加測試計數
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_IncreaseTestCount(void)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    g_paramBlock.params.basicSystem.testCount++;
    
    // 保存更新後的參數
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetTestCount
 *
 * @brief   獲取測試計數
 *
 * @return  測試計數
 */
uint16_t PARAM_GetTestCount(void)
{
    if (!g_activeBlockAddr) {
        return 0;
    }
    
    return g_paramBlock.params.basicSystem.testCount;
}

/*********************************************************************
 * @fn      PARAM_SaveTestRecord
 *
 * @brief   儲存測試記錄
 *
 * @param   resultValue - 測試結果值
 * @param   resultStatus - 測試結果狀態
 * @param   event - 事件類型
 * @param   code - 試片校正碼
 * @param   batteryVoltage - 電池電壓
 * @param   temperature - 環境溫度
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_SaveTestRecord(uint16_t resultValue, uint16_t resultStatus, uint8_t event, uint8_t code, uint16_t batteryVoltage, uint16_t temperature)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    // 填充測試記錄
    TestRecord record;
    record.flag = 0xA55A;  // 記錄有效標記
    record.resultStatus = resultStatus;
    record.resultValue = resultValue;
    record.testType = g_paramBlock.params.basicSystem.stripType;
    record.event = event;
    record.stripCode = code;
    record.year = g_paramBlock.params.basicSystem.year;
    record.month = g_paramBlock.params.basicSystem.month;
    record.date = g_paramBlock.params.basicSystem.date;
    record.hour = g_paramBlock.params.basicSystem.hour;
    record.minute = g_paramBlock.params.basicSystem.minute;
    record.second = g_paramBlock.params.basicSystem.second;
    record.batteryVoltage = batteryVoltage;
    record.temperature = temperature;
    
    // 檢查記錄區
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB 記錄區能存儲的記錄數
    
    // 尋找下一個可用的記錄位置
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag != 0xA55A) { // 如果記錄無效或未使用
            break;
        }
        addr += recordSize;
    }
    
    // 如果記錄區已滿，則擦除整個記錄區並從頭開始
    if (addr >= endAddr) {
        FLASH_Unlock();
        
        // 擦除記錄區的所有頁
        for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += PARAM_PAGE_SIZE) {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        addr = recordBase; // 回到記錄區起始地址
    }
    
    // 寫入新記錄
    FLASH_Unlock();
    bool result = FLASH_WriteBuffer(addr, (uint8_t *)&record, recordSize);
    FLASH_Lock();
    
    // 增加測試次數
    if (result) {
        PARAM_IncreaseTestCount();
    }
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_GetTestRecord
 *
 * @brief   獲取測試記錄
 *
 * @param   index - 記錄索引(0為最新)
 * @param   record - 記錄緩衝區
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_GetTestRecord(uint16_t index, TestRecord *record)
{
    if (!record || !g_activeBlockAddr) {
        return false;
    }
    
    // 檢查記錄區
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB 記錄區能存儲的記錄數
    
    // 尋找有效記錄
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    uint16_t validCount = 0;
    uint32_t validRecords[maxRecords];
    
    // 第一遍掃描記錄所有有效記錄地址
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag == 0xA55A) { // 有效記錄
            validRecords[validCount++] = addr;
        }
        addr += recordSize;
    }
    
    // 檢查索引是否有效
    if (index >= validCount) {
        return false;
    }
    
    // 按照倒序讀取記錄
    addr = validRecords[validCount - 1 - index];
    memcpy(record, (void *)addr, recordSize);
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_GetTestRecordCount
 *
 * @brief   獲取有效測試記錄數量
 *
 * @return  記錄數量
 */
uint16_t PARAM_GetTestRecordCount(void)
{
    if (!g_activeBlockAddr) {
        return 0;
    }
    
    // 檢查記錄區
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB 記錄區能存儲的記錄數
    
    // 尋找有效記錄
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    uint16_t validCount = 0;
    
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag == 0xA55A) { // 有效記錄
            validCount++;
        }
        addr += recordSize;
    }
    
    return validCount;
}

/*********************************************************************
 * @fn      PARAM_EraseAllTestRecords
 *
 * @brief   擦除所有測試記錄
 *
 * @return  true: 成功, false: 失敗
 */
bool PARAM_EraseAllTestRecords(void)
{
    // 檢查記錄區
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t endAddr = recordBase + 6 * 1024;  // 6KB 記錄區
    
    FLASH_Unlock();
    
    // 擦除記錄區的所有頁
    for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += PARAM_PAGE_SIZE) {
        if (FLASH_ErasePage(pageAddr) != FLASH_COMPLETE) {
            FLASH_Lock();
            return false;
        }
    }
    
    FLASH_Lock();
    return true;
} 