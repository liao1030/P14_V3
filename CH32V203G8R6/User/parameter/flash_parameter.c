/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_parameter.c
 * Author             : WCH & HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化儀Flash參數儲存方案實現
*********************************************************************************
* Copyright (c) 2025 HMD Co., Ltd.
* All rights reserved.
*******************************************************************************/

#include "flash_parameter.h"

/* 全局變數 */
static uint32_t g_activeBlockAddr = 0;     // 當前活動參數塊地址
static uint16_t g_paramCRC = 0;            // 參數CRC校驗值

/* 內部函數聲明 */
static bool IsParamBlockValid(uint32_t blockAddr);
static uint16_t CalculateChecksum(const ParameterBlock *block);
static void UpdateChecksum(ParameterBlock *block);
static bool WriteParameterBlock(uint32_t addr, const ParameterBlock *block);
static bool FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size);
static bool GetActiveParameterBlock(ParameterBlock *block);
static uint32_t GetActiveBlockAddress(void);
static bool FindParameterBlock(uint32_t *blockAddr);
static void GetBlockOffset(uint8_t blockType, uint32_t *offset, uint16_t *size);

/**
 * @brief  初始化參數管理模塊
 * @param  無
 * @retval 無
 */
void PARAM_Init(void)
{
    ParameterBlock block;
    
    // 查找活動參數塊
    if (!FindParameterBlock(&g_activeBlockAddr))
    {
        // 如果沒有找到有效的參數塊，恢復出廠設置
        PARAM_ResetToDefault();
    }
    
    // 讀取參數塊並計算校驗和
    if (GetActiveParameterBlock(&block))
    {
        g_paramCRC = CalculateChecksum(&block);
    }
}

/**
 * @brief  讀取指定類型的參數塊
 * @param  blockType: 參數塊類型
 * @param  data: 數據緩衝區
 * @param  size: 數據大小
 * @retval true: 成功, false: 失敗
 */
bool PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size)
{
    ParameterBlock block;
    uint32_t offset = 0;
    uint16_t blockSize = 0;
    
    if (data == NULL)
    {
        return false;
    }
    
    // 獲取指定參數塊的偏移和大小
    GetBlockOffset(blockType, &offset, &blockSize);
    
    // 檢查參數大小是否匹配
    if (size != blockSize)
    {
        return false;
    }
    
    // 讀取整個參數塊
    if (!GetActiveParameterBlock(&block))
    {
        return false;
    }
    
    // 複製指定的參數塊數據
    uint8_t *src = (uint8_t *)&block.params + offset;
    memcpy(data, src, size);
    
    return true;
}

/**
 * @brief  更新指定類型的參數塊
 * @param  blockType: 參數塊類型
 * @param  data: 數據緩衝區
 * @param  size: 數據大小
 * @retval true: 成功, false: 失敗
 */
bool PARAM_UpdateBlock(uint8_t blockType, void *data, uint16_t size)
{
    ParameterBlock block;
    uint32_t offset = 0;
    uint16_t blockSize = 0;
    uint32_t backupAddr;
    
    if (data == NULL)
    {
        return false;
    }
    
    // 獲取指定參數塊的偏移和大小
    GetBlockOffset(blockType, &offset, &blockSize);
    
    // 檢查參數大小是否匹配
    if (size != blockSize)
    {
        return false;
    }
    
    // 讀取當前參數塊
    if (!GetActiveParameterBlock(&block))
    {
        return false;
    }
    
    // 複製新的參數數據
    uint8_t *dst = (uint8_t *)&block.params + offset;
    memcpy(dst, data, size);
    
    // 更新參數塊頭
    block.header.writeCounter++;
    block.header.timestamp = (uint32_t)((block.params.basicSystem.year << 26) | 
                                        (block.params.basicSystem.month << 22) | 
                                        (block.params.basicSystem.date << 17) | 
                                        (block.params.basicSystem.hour << 12) | 
                                        (block.params.basicSystem.minute << 6) | 
                                        block.params.basicSystem.second);
    
    // 計算新的校驗和
    UpdateChecksum(&block);
    
    // 決定要寫入的參數塊位置 (備份位置)
    backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    // 寫入參數到備份位置
    if (!WriteParameterBlock(backupAddr, &block))
    {
        return false;
    }
    
    // 更新活動參數塊地址
    g_activeBlockAddr = backupAddr;
    
    // 更新參數CRC
    g_paramCRC = CalculateChecksum(&block);
    
    return true;
}

/**
 * @brief  獲取系統日期時間
 * @param  year, month, date, hour, minute, second: 日期時間參數
 * @retval true: 成功, false: 失敗
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // 複製日期時間
    if (year != NULL) *year = basicParams.year;
    if (month != NULL) *month = basicParams.month;
    if (date != NULL) *date = basicParams.date;
    if (hour != NULL) *hour = basicParams.hour;
    if (minute != NULL) *minute = basicParams.minute;
    if (second != NULL) *second = basicParams.second;
    
    return true;
}

/**
 * @brief  更新系統日期時間
 * @param  year, month, date, hour, minute, second: 日期時間參數
 * @retval true: 成功, false: 失敗
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    BasicSystemBlock basicParams;
    
    // 檢查日期時間參數有效性
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59)
    {
        return false;
    }
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // 更新日期時間
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    // 更新參數
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief  增加測試計數
 * @param  無
 * @retval true: 成功, false: 失敗
 */
bool PARAM_IncreaseTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // 增加測試計數
    basicParams.testCount++;
    
    // 更新參數
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief  獲取測試計數
 * @param  無
 * @retval 測試計數
 */
uint32_t PARAM_GetTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // 讀取基本系統參數
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return 0;
    }
    
    return basicParams.testCount;
}

/**
 * @brief  獲取指定試片類型的試片參數
 * @param  stripType: 試片類型
 * @param  ndl: 新試片檢測水平
 * @param  udl: 已使用試片檢測水平
 * @param  bloodIn: 血液檢測水平
 * @retval true: 成功, false: 失敗
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    // 基於試片類型獲取相應的參數
    switch (stripType)
    {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            GlucoseParamBlock bgParams;
            if (!PARAM_ReadBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock)))
            {
                return false;
            }
            if (ndl != NULL) *ndl = bgParams.bgNdl;
            if (udl != NULL) *udl = bgParams.bgUdl;
            if (bloodIn != NULL) *bloodIn = bgParams.bgBloodIn;
            break;
        }
        case 1: // U(尿酸)
        {
            UricAcidParamBlock uaParams;
            if (!PARAM_ReadBlock(BLOCK_URIC_ACID, &uaParams, sizeof(UricAcidParamBlock)))
            {
                return false;
            }
            if (ndl != NULL) *ndl = uaParams.uaNdl;
            if (udl != NULL) *udl = uaParams.uaUdl;
            if (bloodIn != NULL) *bloodIn = uaParams.uaBloodIn;
            break;
        }
        case 2: // C(總膽固醇)
        {
            CholesterolParamBlock chParams;
            if (!PARAM_ReadBlock(BLOCK_CHOLESTEROL, &chParams, sizeof(CholesterolParamBlock)))
            {
                return false;
            }
            if (ndl != NULL) *ndl = chParams.chNdl;
            if (udl != NULL) *udl = chParams.chUdl;
            if (bloodIn != NULL) *bloodIn = chParams.chBloodIn;
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TriglycerideParamBlock tgParams;
            if (!PARAM_ReadBlock(BLOCK_TRIGLYCERIDE, &tgParams, sizeof(TriglycerideParamBlock)))
            {
                return false;
            }
            if (ndl != NULL) *ndl = tgParams.tgNdl;
            if (udl != NULL) *udl = tgParams.tgUdl;
            if (bloodIn != NULL) *bloodIn = tgParams.tgBloodIn;
            break;
        }
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief  獲取指定試片類型的測量範圍
 * @param  stripType: 試片類型
 * @param  lowLimit: 低限閾值
 * @param  highLimit: 高限閾值
 * @retval true: 成功, false: 失敗
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit)
{
    // 基於試片類型獲取相應的參數
    switch (stripType)
    {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            GlucoseParamBlock bgParams;
            if (!PARAM_ReadBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock)))
            {
                return false;
            }
            if (lowLimit != NULL) *lowLimit = bgParams.bgL;
            if (highLimit != NULL) *highLimit = bgParams.bgH;
            break;
        }
        case 1: // U(尿酸)
        {
            UricAcidParamBlock uaParams;
            if (!PARAM_ReadBlock(BLOCK_URIC_ACID, &uaParams, sizeof(UricAcidParamBlock)))
            {
                return false;
            }
            if (lowLimit != NULL) *lowLimit = uaParams.uaL;
            if (highLimit != NULL) *highLimit = uaParams.uaH;
            break;
        }
        case 2: // C(總膽固醇)
        {
            CholesterolParamBlock chParams;
            if (!PARAM_ReadBlock(BLOCK_CHOLESTEROL, &chParams, sizeof(CholesterolParamBlock)))
            {
                return false;
            }
            if (lowLimit != NULL) *lowLimit = chParams.chL;
            if (highLimit != NULL) *highLimit = chParams.chH;
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TriglycerideParamBlock tgParams;
            if (!PARAM_ReadBlock(BLOCK_TRIGLYCERIDE, &tgParams, sizeof(TriglycerideParamBlock)))
            {
                return false;
            }
            if (lowLimit != NULL) *lowLimit = tgParams.tgL;
            if (highLimit != NULL) *highLimit = tgParams.tgH;
            break;
        }
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief  獲取指定試片類型的時序參數
 * @param  stripType: 試片類型
 * @param  tpl: TPL時序參數
 * @param  trd: TRD時序參數
 * @param  evWidth: EV寬度時序參數
 * @param  group: 參數組 (1 或 2)
 * @retval true: 成功, false: 失敗
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    // 檢查參數組有效性
    if (group != 1 && group != 2)
    {
        return false;
    }
    
    // 基於試片類型獲取相應的參數
    switch (stripType)
    {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            GlucoseParamBlock bgParams;
            if (!PARAM_ReadBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock)))
            {
                return false;
            }
            
            if (group == 1)
            {
                if (tpl != NULL) *tpl = bgParams.bgTPL1;
                if (trd != NULL) *trd = bgParams.bgTRD1;
                if (evWidth != NULL) *evWidth = bgParams.bgEVWidth1;
            }
            else
            {
                if (tpl != NULL) *tpl = bgParams.bgTPL2;
                if (trd != NULL) *trd = bgParams.bgTRD2;
                if (evWidth != NULL) *evWidth = bgParams.bgEVWidth2;
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UricAcidParamBlock uaParams;
            if (!PARAM_ReadBlock(BLOCK_URIC_ACID, &uaParams, sizeof(UricAcidParamBlock)))
            {
                return false;
            }
            
            if (group == 1)
            {
                if (tpl != NULL) *tpl = uaParams.uaTPL1;
                if (trd != NULL) *trd = uaParams.uaTRD1;
                if (evWidth != NULL) *evWidth = uaParams.uaEVWidth1;
            }
            else
            {
                if (tpl != NULL) *tpl = uaParams.uaTPL2;
                if (trd != NULL) *trd = uaParams.uaTRD2;
                if (evWidth != NULL) *evWidth = uaParams.uaEVWidth2;
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CholesterolParamBlock chParams;
            if (!PARAM_ReadBlock(BLOCK_CHOLESTEROL, &chParams, sizeof(CholesterolParamBlock)))
            {
                return false;
            }
            
            if (group == 1)
            {
                if (tpl != NULL) *tpl = chParams.chTPL1;
                if (trd != NULL) *trd = chParams.chTRD1;
                if (evWidth != NULL) *evWidth = chParams.chEVWidth1;
            }
            else
            {
                if (tpl != NULL) *tpl = chParams.chTPL2;
                if (trd != NULL) *trd = chParams.chTRD2;
                if (evWidth != NULL) *evWidth = chParams.chEVWidth2;
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TriglycerideParamBlock tgParams;
            if (!PARAM_ReadBlock(BLOCK_TRIGLYCERIDE, &tgParams, sizeof(TriglycerideParamBlock)))
            {
                return false;
            }
            
            if (group == 1)
            {
                if (tpl != NULL) *tpl = tgParams.tgTPL1;
                if (trd != NULL) *trd = tgParams.tgTRD1;
                if (evWidth != NULL) *evWidth = tgParams.tgEVWidth1;
            }
            else
            {
                if (tpl != NULL) *tpl = tgParams.tgTPL2;
                if (trd != NULL) *trd = tgParams.tgTRD2;
                if (evWidth != NULL) *evWidth = tgParams.tgEVWidth2;
            }
            break;
        }
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief  重設參數為出廠預設值
 * @param  無
 * @retval true: 成功, false: 失敗
 */
bool PARAM_ResetToDefault(void)
{
    // 參數塊
    ParameterBlock defaultBlock;
    memset(&defaultBlock, 0, sizeof(ParameterBlock));
    
    // 初始化頭部
    defaultBlock.header.magic = PARAM_MAGIC_NUMBER;
    defaultBlock.header.version = PARAM_VERSION;
    defaultBlock.header.writeCounter = 1;
    defaultBlock.header.timestamp = 0;
    
    // 基本系統參數
    defaultBlock.params.basicSystem.lbt = 58;  // 2.58V
    defaultBlock.params.basicSystem.obt = 55;  // 2.55V
    defaultBlock.params.basicSystem.factory = 0; // 使用者模式
    defaultBlock.params.basicSystem.modelNo = 1;
    defaultBlock.params.basicSystem.fwNo = 10; // 版本1.0
    defaultBlock.params.basicSystem.testCount = 0;
    defaultBlock.params.basicSystem.codeTableVer = 1;
    
    // 設定預設時間 (2025-05-01 00:00:00)
    defaultBlock.params.basicSystem.year = 25;
    defaultBlock.params.basicSystem.month = 5;
    defaultBlock.params.basicSystem.date = 1;
    defaultBlock.params.basicSystem.hour = 0;
    defaultBlock.params.basicSystem.minute = 0;
    defaultBlock.params.basicSystem.second = 0;
    
    // 設定環境參數
    defaultBlock.params.basicSystem.tempLowLimit = 10;  // 10°C
    defaultBlock.params.basicSystem.tempHighLimit = 40; // 40°C
    defaultBlock.params.basicSystem.measureUnit = 1;    // mg/dL
    defaultBlock.params.basicSystem.defaultEvent = 1;   // AC
    defaultBlock.params.basicSystem.stripType = 0;      // GLV(血糖)
    
    // 硬體校準參數
    defaultBlock.params.hardwareCalib.evT3Trigger = 800;  // 800mV
    defaultBlock.params.hardwareCalib.evWorking = 164;
    defaultBlock.params.hardwareCalib.evT3 = 164;
    defaultBlock.params.hardwareCalib.dacoOffset = 0;
    defaultBlock.params.hardwareCalib.dacdo = 0;
    defaultBlock.params.hardwareCalib.cc211Status = 0;
    defaultBlock.params.hardwareCalib.calTolerance = 8;  // 0.8%
    defaultBlock.params.hardwareCalib.ops = 1.0f;       // 運算校準斜率為1.0
    defaultBlock.params.hardwareCalib.opi = 0.0f;       // 運算校準截距為0.0
    defaultBlock.params.hardwareCalib.qct = 0;
    defaultBlock.params.hardwareCalib.tempOffset = 0;   // 溫度偏移補償
    defaultBlock.params.hardwareCalib.batteryOffset = 0; // 電池偏移補償
    
    // 血糖參數
    defaultBlock.params.bgParams.bgCsuTolerance = 10;  // 1.0%容差
    defaultBlock.params.bgParams.bgNdl = 100;  // 新試片檢測水平
    defaultBlock.params.bgParams.bgUdl = 50;   // 已使用試片檢測水平
    defaultBlock.params.bgParams.bgBloodIn = 150; // 血液檢測水平
    memset(defaultBlock.params.bgParams.bgStripLot, 0, 16); // 清空試片批號
    
    defaultBlock.params.bgParams.bgL = 2;    // 低於20 mg/dL顯示"LO"
    defaultBlock.params.bgParams.bgH = 60;   // 高於600 mg/dL顯示"HI"
    defaultBlock.params.bgParams.bgT3E37 = 2000;  // T3 ADV錯誤閾值
    
    // 血糖測試時序參數
    defaultBlock.params.bgParams.bgTPL1 = 10;        // 1秒
    defaultBlock.params.bgParams.bgTRD1 = 10;        // 1秒
    defaultBlock.params.bgParams.bgEVWidth1 = 5;     // 0.5秒
    defaultBlock.params.bgParams.bgTPL2 = 20;        // 2秒
    defaultBlock.params.bgParams.bgTRD2 = 20;        // 2秒
    defaultBlock.params.bgParams.bgEVWidth2 = 10;    // 1秒
    
    // 血糖溫度補償係數
    defaultBlock.params.bgParams.bgTempA = 0.0f;
    defaultBlock.params.bgParams.bgTempB = 0.0f;
    defaultBlock.params.bgParams.bgTempC = 0.0f;
    
    // 血糖海拔補償係數
    defaultBlock.params.bgParams.bgAltA = 0.0f;
    defaultBlock.params.bgParams.bgAltB = 0.0f;
    defaultBlock.params.bgParams.bgAltC = 0.0f;
    
    // 血糖血比容補償係數
    defaultBlock.params.bgParams.bgHctA = 0.0f;
    defaultBlock.params.bgParams.bgHctB = 0.0f;
    defaultBlock.params.bgParams.bgHctC = 0.0f;
    
    // 血糖補償截距和比例係數
    defaultBlock.params.bgParams.bgCompS = 1.0f;
    defaultBlock.params.bgParams.bgCompI = 0.0f;
    
    // 尿酸參數
    defaultBlock.params.uaParams.uaCsuTolerance = 10;  // 1.0%容差
    defaultBlock.params.uaParams.uaNdl = 100;  // 新試片檢測水平
    defaultBlock.params.uaParams.uaUdl = 50;   // 已使用試片檢測水平
    defaultBlock.params.uaParams.uaBloodIn = 150; // 血液檢測水平
    memset(defaultBlock.params.uaParams.uaStripLot, 0, 16); // 清空試片批號
    
    defaultBlock.params.uaParams.uaL = 20;   // 低於2.0 mg/dL顯示"LO"
    defaultBlock.params.uaParams.uaH = 200;  // 高於20.0 mg/dL顯示"HI"
    defaultBlock.params.uaParams.uaT3E37 = 2000;  // T3 ADV錯誤閾值
    
    // 尿酸測試時序參數
    defaultBlock.params.uaParams.uaTPL1 = 40;        // 4秒
    defaultBlock.params.uaParams.uaTRD1 = 40;        // 4秒
    defaultBlock.params.uaParams.uaEVWidth1 = 5;     // 0.5秒
    defaultBlock.params.uaParams.uaTPL2 = 80;        // 8秒
    defaultBlock.params.uaParams.uaTRD2 = 80;        // 8秒
    defaultBlock.params.uaParams.uaEVWidth2 = 10;    // 1秒
    
    // 尿酸溫度補償係數
    defaultBlock.params.uaParams.uaTempA = 0.0f;
    defaultBlock.params.uaParams.uaTempB = 0.0f;
    defaultBlock.params.uaParams.uaTempC = 0.0f;
    
    // 尿酸海拔補償係數
    defaultBlock.params.uaParams.uaAltA = 0.0f;
    defaultBlock.params.uaParams.uaAltB = 0.0f;
    defaultBlock.params.uaParams.uaAltC = 0.0f;
    
    // 尿酸補償截距和比例係數
    defaultBlock.params.uaParams.uaCompS = 1.0f;
    defaultBlock.params.uaParams.uaCompI = 0.0f;
    
    // 總膽固醇參數
    defaultBlock.params.chParams.chCsuTolerance = 10;  // 1.0%容差
    defaultBlock.params.chParams.chNdl = 100;  // 新試片檢測水平
    defaultBlock.params.chParams.chUdl = 50;   // 已使用試片檢測水平
    defaultBlock.params.chParams.chBloodIn = 150; // 血液檢測水平
    memset(defaultBlock.params.chParams.chStripLot, 0, 16); // 清空試片批號
    
    defaultBlock.params.chParams.chL = 50;   // 低於50 mg/dL顯示"LO"
    defaultBlock.params.chParams.chH = 500;  // 高於500 mg/dL顯示"HI"
    defaultBlock.params.chParams.chT3E37 = 2000;  // T3 ADV錯誤閾值
    
    // 總膽固醇測試時序參數
    defaultBlock.params.chParams.chTPL1 = 120;       // 12秒
    defaultBlock.params.chParams.chTRD1 = 150;       // 15秒
    defaultBlock.params.chParams.chEVWidth1 = 5;     // 0.5秒
    defaultBlock.params.chParams.chTPL2 = 150;       // 15秒
    defaultBlock.params.chParams.chTRD2 = 170;       // 17秒
    defaultBlock.params.chParams.chEVWidth2 = 10;    // 1秒
    
    // 總膽固醇溫度補償係數
    defaultBlock.params.chParams.chTempA = 0.0f;
    defaultBlock.params.chParams.chTempB = 0.0f;
    defaultBlock.params.chParams.chTempC = 0.0f;
    
    // 總膽固醇補償截距和比例係數
    defaultBlock.params.chParams.chCompS = 1.0f;
    defaultBlock.params.chParams.chCompI = 0.0f;
    
    // 三酸甘油脂參數
    defaultBlock.params.tgParams.tgCsuTolerance = 10;  // 1.0%容差
    defaultBlock.params.tgParams.tgNdl = 100;  // 新試片檢測水平
    defaultBlock.params.tgParams.tgUdl = 50;   // 已使用試片檢測水平
    defaultBlock.params.tgParams.tgBloodIn = 150; // 血液檢測水平
    memset(defaultBlock.params.tgParams.tgStripLot, 0, 16); // 清空試片批號
    
    defaultBlock.params.tgParams.tgL = 50;   // 低於50 mg/dL顯示"LO"
    defaultBlock.params.tgParams.tgH = 1000; // 高於1000 mg/dL顯示"HI"
    defaultBlock.params.tgParams.tgT3E37 = 2000;  // T3 ADV錯誤閾值
    
    // 三酸甘油脂測試時序參數
    defaultBlock.params.tgParams.tgTPL1 = 120;       // 12秒
    defaultBlock.params.tgParams.tgTRD1 = 150;       // 15秒
    defaultBlock.params.tgParams.tgEVWidth1 = 5;     // 0.5秒
    defaultBlock.params.tgParams.tgTPL2 = 150;       // 15秒
    defaultBlock.params.tgParams.tgTRD2 = 170;       // 17秒
    defaultBlock.params.tgParams.tgEVWidth2 = 10;    // 1秒
    
    // 三酸甘油脂溫度補償係數
    defaultBlock.params.tgParams.tgTempA = 0.0f;
    defaultBlock.params.tgParams.tgTempB = 0.0f;
    defaultBlock.params.tgParams.tgTempC = 0.0f;
    
    // 三酸甘油脂補償截距和比例係數
    defaultBlock.params.tgParams.tgCompS = 1.0f;
    defaultBlock.params.tgParams.tgCompI = 0.0f;
    
    // 計算校驗和
    UpdateChecksum(&defaultBlock);
    
    // 擦除參數區
    FLASH_Unlock();
    FLASH_ErasePage(PARAM_BLOCK_A_ADDR);
    FLASH_ErasePage(PARAM_BLOCK_B_ADDR);
    FLASH_Lock();
    
    // 寫入參數到參數區A
    bool resultA = WriteParameterBlock(PARAM_BLOCK_A_ADDR, &defaultBlock);
    
    // 寫入參數到參數區B
    bool resultB = WriteParameterBlock(PARAM_BLOCK_B_ADDR, &defaultBlock);
    
    // 更新活動參數塊地址
    if (resultA)
    {
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    }
    else if (resultB)
    {
        g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
    }
    else
    {
        return false;
    }
    
    // 更新參數CRC
    g_paramCRC = CalculateChecksum(&defaultBlock);
    
    return (resultA || resultB);
}

/**
 * @brief  儲存測試記錄
 * @param  record: 測試記錄
 * @retval true: 成功, false: 失敗
 */
bool PARAM_SaveTestRecord(TestRecord *record)
{
    if (record == NULL)
    {
        return false;
    }
    
    // 測試記錄大小
    uint32_t recordSize = sizeof(TestRecord);
    
    // 查找測試記錄區的下一個可用位置
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    
    while (addr < endAddr)
    {
        // 讀取一個字元，如果是0xFFFF表示此位置未使用
        checkFlag = *((uint16_t *)addr);
        if (checkFlag == 0xFFFF)
        {
            break;
        }
        addr += recordSize;
    }
    
    // 如果記錄區已滿，擦除整個記錄區並從頭開始
    if (addr >= endAddr)
    {
        FLASH_Unlock();
        
        // 擦除記錄區 (每頁1KB，需要擦除多個頁)
        for (uint32_t pageAddr = RECORD_AREA_ADDR; pageAddr < endAddr; pageAddr += 0x400)
        {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        // 重設地址到記錄區起始位置
        addr = RECORD_AREA_ADDR;
    }
    
    // 寫入新記錄
    FLASH_Unlock();
    bool result = FlashWriteBuffer(addr, (uint8_t *)record, recordSize);
    FLASH_Lock();
    
    // 增加測試次數計數
    if (result)
    {
        PARAM_IncreaseTestCount();
    }
    
    return result;
}

/**
 * @brief  獲取測試記錄
 * @param  index: 記錄索引（0表示最新）
 * @param  record: 測試記錄緩衝區
 * @retval true: 成功, false: 失敗
 */
bool PARAM_GetTestRecord(uint32_t index, TestRecord *record)
{
    if (record == NULL)
    {
        return false;
    }
    
    // 測試記錄大小
    uint32_t recordSize = sizeof(TestRecord);
    
    // 計算記錄區可以保存的最大記錄數
    uint32_t maxRecords = RECORD_AREA_SIZE / recordSize;
    
    // 檢查索引是否超出範圍
    if (index >= maxRecords)
    {
        return false;
    }
    
    // 查找最新記錄位置
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    uint32_t lastRecordAddr = 0;
    
    while (addr < endAddr)
    {
        // 讀取一個字元，如果是0xFFFF表示此位置未使用
        checkFlag = *((uint16_t *)addr);
        if (checkFlag == 0xFFFF)
        {
            break;
        }
        lastRecordAddr = addr;
        addr += recordSize;
    }
    
    // 如果沒有找到任何記錄
    if (lastRecordAddr == 0)
    {
        return false;
    }
    
    // 從最新記錄開始回溯
    uint32_t targetAddr;
    if (addr == RECORD_AREA_ADDR)
    {
        // 如果第一個位置就是0xFFFF，表示記錄區為空
        return false;
    }
    else if (addr == RECORD_AREA_ADDR + recordSize)
    {
        // 如果只有一條記錄
        if (index == 0)
        {
            targetAddr = RECORD_AREA_ADDR;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // 計算目標記錄地址
        if (index == 0)
        {
            // 最新記錄
            targetAddr = lastRecordAddr;
        }
        else
        {
            // 查找指定索引的記錄
            targetAddr = lastRecordAddr - (index * recordSize);
            if (targetAddr < RECORD_AREA_ADDR)
            {
                // 處理回繞
                uint32_t recordCount = (lastRecordAddr - RECORD_AREA_ADDR) / recordSize + 1;
                uint32_t wrapIndex = index % recordCount;
                targetAddr = lastRecordAddr - (wrapIndex * recordSize);
                if (wrapIndex != index)
                {
                    return false;
                }
            }
        }
    }
    
    // 讀取記錄
    memcpy(record, (void *)targetAddr, recordSize);
    
    return true;
}

/**
 * @brief  獲取測試記錄總數
 * @param  無
 * @retval 測試記錄數量
 */
uint32_t PARAM_GetTestRecordCount(void)
{
    // 測試記錄大小
    uint32_t recordSize = sizeof(TestRecord);
    
    // 查找測試記錄區的已使用空間
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    uint32_t count = 0;
    
    while (addr < endAddr)
    {
        // 讀取一個字元，如果是0xFFFF表示此位置未使用
        checkFlag = *((uint16_t *)addr);
        if (checkFlag == 0xFFFF)
        {
            break;
        }
        count++;
        addr += recordSize;
    }
    
    return count;
}

/**
 * @brief  擦除所有測試記錄
 * @param  無
 * @retval true: 成功, false: 失敗
 */
bool PARAM_EraseAllTestRecords(void)
{
    FLASH_Unlock();
    
    // 擦除記錄區 (每頁1KB，需要擦除多個頁)
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    for (uint32_t pageAddr = RECORD_AREA_ADDR; pageAddr < endAddr; pageAddr += 0x400)
    {
        FLASH_Status status = FLASH_ErasePage(pageAddr);
        if (status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return false;
        }
    }
    
    FLASH_Lock();
    return true;
}

/**
 * @brief  檢查參數塊有效性
 * @param  blockAddr: 參數塊地址
 * @retval true: 有效, false: 無效
 */
static bool IsParamBlockValid(uint32_t blockAddr)
{
    ParamBlockHeader *header = (ParamBlockHeader *)blockAddr;
    
    // 檢查魔數
    if (header->magic != PARAM_MAGIC_NUMBER)
    {
        return false;
    }
    
    // 檢查版本
    if (header->version != PARAM_VERSION)
    {
        return false;
    }
    
    // 計算校驗和
    ParameterBlock *block = (ParameterBlock *)blockAddr;
    uint16_t calcChecksum = CalculateChecksum(block);
    
    // 檢查校驗和
    return (calcChecksum == header->checksum);
}

/**
 * @brief  計算參數塊校驗和
 * @param  block: 參數塊
 * @retval 校驗和
 */
static uint16_t CalculateChecksum(const ParameterBlock *block)
{
    uint16_t checksum = 0;
    uint8_t *data = (uint8_t *)block;
    uint32_t size = sizeof(ParameterBlock) - sizeof(uint16_t);  // 排除校驗和字段
    
    // 計算簡單的校驗和
    for (uint32_t i = 0; i < size; i++)
    {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief  更新參數塊校驗和
 * @param  block: 參數塊
 * @retval 無
 */
static void UpdateChecksum(ParameterBlock *block)
{
    block->header.checksum = CalculateChecksum(block);
}

/**
 * @brief  寫入參數塊到Flash
 * @param  addr: 目標地址
 * @param  block: 參數塊
 * @retval true: 成功, false: 失敗
 */
static bool WriteParameterBlock(uint32_t addr, const ParameterBlock *block)
{
    bool result = false;
    
    // 首先擦除頁
    FLASH_Unlock();
    FLASH_Status status = FLASH_ErasePage(addr);
    
    if (status == FLASH_COMPLETE)
    {
        // 寫入參數數據
        result = FlashWriteBuffer(addr, (const uint8_t *)block, sizeof(ParameterBlock));
    }
    
    FLASH_Lock();
    
    return result;
}

/**
 * @brief  將緩衝區寫入Flash
 * @param  addr: 目標地址
 * @param  buffer: 數據緩衝區
 * @param  size: 數據大小
 * @retval true: 成功, false: 失敗
 */
static bool FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    // 檢查參數
    if (buffer == NULL || size == 0)
    {
        return false;
    }
    
    // 檢查地址是否對齊4字節
    if (addr % 4 != 0)
    {
        return false;
    }
    
    // 按字寫入 (4字節為一個字)
    for (uint32_t i = 0; i < size; i += 4)
    {
        uint32_t data;
        if (i + 4 <= size)
        {
            // 完整字
            data = ((uint32_t)buffer[i]) |
                   ((uint32_t)buffer[i+1] << 8) |
                   ((uint32_t)buffer[i+2] << 16) |
                   ((uint32_t)buffer[i+3] << 24);
        }
        else
        {
            // 最後不足4字節的部分
            data = 0xFFFFFFFF;  // 先填充全1
            for (uint32_t j = 0; j < (size - i); j++)
            {
                data &= ~(0xFF << (j * 8));
                data |= ((uint32_t)buffer[i+j] << (j * 8));
            }
        }
        
        // 寫入一個字 (4字節)
        FLASH_Status status = FLASH_ProgramWord(addr + i, data);
        if (status != FLASH_COMPLETE)
        {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief  獲取當前活動參數塊
 * @param  block: 參數塊緩衝區
 * @retval true: 成功, false: 失敗
 */
static bool GetActiveParameterBlock(ParameterBlock *block)
{
    if (block == NULL)
    {
        return false;
    }
    
    // 獲取活動參數塊地址
    uint32_t addr = GetActiveBlockAddress();
    if (addr == 0)
    {
        return false;
    }
    
    // 讀取整個參數塊
    memcpy(block, (void *)addr, sizeof(ParameterBlock));
    
    return true;
}

/**
 * @brief  獲取活動參數塊地址
 * @param  無
 * @retval 參數塊地址 (0表示失敗)
 */
static uint32_t GetActiveBlockAddress(void)
{
    // 如果全局變數已經保存了活動塊地址，直接返回
    if (g_activeBlockAddr != 0 && IsParamBlockValid(g_activeBlockAddr))
    {
        return g_activeBlockAddr;
    }
    
    // 否則重新查找活動參數塊
    FindParameterBlock(&g_activeBlockAddr);
    
    return g_activeBlockAddr;
}

/**
 * @brief  查找有效的參數塊
 * @param  blockAddr: 保存找到的參數塊地址
 * @retval true: 成功, false: 失敗
 */
static bool FindParameterBlock(uint32_t *blockAddr)
{
    if (blockAddr == NULL)
    {
        return false;
    }
    
    // 檢查參數塊A是否有效
    bool validA = IsParamBlockValid(PARAM_BLOCK_A_ADDR);
    
    // 檢查參數塊B是否有效
    bool validB = IsParamBlockValid(PARAM_BLOCK_B_ADDR);
    
    // 設置預設値
    *blockAddr = 0;
    
    // 如果兩個都有效，選擇寫入計數器更大的那個
    if (validA && validB)
    {
        ParamBlockHeader *headerA = (ParamBlockHeader *)PARAM_BLOCK_A_ADDR;
        ParamBlockHeader *headerB = (ParamBlockHeader *)PARAM_BLOCK_B_ADDR;
        
        if (headerA->writeCounter >= headerB->writeCounter)
        {
            *blockAddr = PARAM_BLOCK_A_ADDR;
        }
        else
        {
            *blockAddr = PARAM_BLOCK_B_ADDR;
        }
        
        return true;
    }
    // 如果只有A有效
    else if (validA)
    {
        *blockAddr = PARAM_BLOCK_A_ADDR;
        return true;
    }
    // 如果只有B有效
    else if (validB)
    {
        *blockAddr = PARAM_BLOCK_B_ADDR;
        return true;
    }
    
    // 都無效
    return false;
}

/**
 * @brief  獲取參數塊類型對應的偏移和大小
 * @param  blockType: 參數塊類型
 * @param  offset: 返回偏移
 * @param  size: 返回大小
 * @retval 無
 */
static void GetBlockOffset(uint8_t blockType, uint32_t *offset, uint16_t *size)
{
    // 設置預設值
    *offset = 0;
    *size = 0;
    
    // 根據參數塊類型設置偏移和大小
    switch (blockType)
    {
        case BLOCK_BASIC_SYSTEM:
            *offset = offsetof(ParameterData, basicSystem);
            *size = sizeof(BasicSystemBlock);
            break;
        
        case BLOCK_HARDWARE_CALIB:
            *offset = offsetof(ParameterData, hardwareCalib);
            *size = sizeof(HardwareCalibBlock);
            break;
        
        case BLOCK_GLUCOSE:
            *offset = offsetof(ParameterData, bgParams);
            *size = sizeof(GlucoseParamBlock);
            break;
        
        case BLOCK_URIC_ACID:
            *offset = offsetof(ParameterData, uaParams);
            *size = sizeof(UricAcidParamBlock);
            break;
        
        case BLOCK_CHOLESTEROL:
            *offset = offsetof(ParameterData, chParams);
            *size = sizeof(CholesterolParamBlock);
            break;
        
        case BLOCK_TRIGLYCERIDE:
            *offset = offsetof(ParameterData, tgParams);
            *size = sizeof(TriglycerideParamBlock);
            break;
        
        case BLOCK_RESERVED:
            *offset = offsetof(ParameterData, reserved);
            *size = sizeof(ReservedBlock);
            break;
        
        default:
            break;
    }
}