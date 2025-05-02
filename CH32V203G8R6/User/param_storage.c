/**
 * @file param_storage.c
 * @brief CH32V203G8R6�๦�܅���Flash���淽�����F�ļ�
 * @version 1.0
 * @date 2024-09-25
 * 
 * @copyright Copyright (c) 2024 HMD Biomedical Inc.
 */

#include "param_storage.h"
#include "string.h"
#include "ch32v20x_flash.h"
#include <stdbool.h>

/* ȫ��׃�� */
static uint32_t g_activeBlockAddr = 0;   // ��ǰ��Ӆ����^��ַ
static bool g_isInitialized = false;     // �����惦�Ƿ��ѳ�ʼ��

/* Flash�����o������ */

/**
 * @brief ������Ч�ą����^�K��ַ
 * 
 * @return uint32_t ��Ч�ą����^��ַ�����]�Єt����0
 */
static uint32_t FindActiveParameterBlock(void)
{
    ParameterBlock blockA, blockB;
    bool validA = false, validB = false;
    uint8_t counterA = 0, counterB = 0;
    
    // �xȡ�����^A
    memcpy(&blockA, (void *)PARAM_BLOCK_A_ADDR, sizeof(ParameterBlock));
    if (blockA.header.magic == 0xA55A) {
        validA = true;
        counterA = blockA.header.writeCounter;
    }
    
    // �xȡ�����^B
    memcpy(&blockB, (void *)PARAM_BLOCK_B_ADDR, sizeof(ParameterBlock));
    if (blockB.header.magic == 0xA55A) {
        validB = true;
        counterB = blockB.header.writeCounter;
    }
    
    // �Д��Ă������^��Ч�������µ�
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
    
    return 0; // �]����Ч�ą����^
}

/**
 * @brief Ӌ��^�KУ��
 * 
 * @param data ����ָ�
 * @param size ������С
 * @return uint16_t У��
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
 * @brief Ӌ��^�KCRC8
 * 
 * @param data ����ָ�
 * @param size ������С
 * @return uint8_t CRC8У�ֵ
 */
static uint8_t CalculateCRC8(const void *data, uint16_t size)
{
    uint8_t crc = 0;
    const uint8_t *ptr = (const uint8_t *)data;
    
    for (uint16_t i = 0; i < size; i++) {
        crc ^= ptr[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31; // CRC-8���ʽ
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief ����Flash���n�^
 * 
 * @param address Ŀ��Flash��ַ
 * @param buffer �������n�^
 * @param size ������С
 * @return true ����ɹ�
 * @return false ����ʧ��
 */
static bool FlashWrite(uint32_t address, const uint8_t *buffer, uint32_t size)
{
    uint32_t i;
    uint32_t *dst = (uint32_t*)address;
    const uint32_t *src = (const uint32_t*)buffer;
    uint32_t wordCount = (size + 3) / 4; // ����ȡ�����ִ�С
    
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
 * @brief ����Flash�
 * 
 * @param address ���ʼ��ַ
 * @return true �����ɹ�
 * @return false ����ʧ��
 */
static bool FlashErasePage(uint32_t address)
{
    uint32_t pageAddr = address & 0xFFFFF800; // ���R���߅��(2KB)
    FLASH_Unlock();
    FLASH_Status status = FLASH_ErasePage(pageAddr);
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE);
}

/**
 * @brief �@ȡ�����^�Kƫ�Ƶ�ַ
 * 
 * @param blockId �^�KID
 * @return uint32_t �^�K�څ����^�е�ƫ�Ƶ�ַ�����oЧ�t����0
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
 * @brief �@ȡ�����^�K��С
 * 
 * @param blockId �^�KID
 * @return uint16_t �^�K��С�����oЧ�t����0
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
 * @brief ���^�KУ��
 * 
 * @param block �����^�Kָ�
 */
static void UpdateBlockChecksum(ParameterBlock *block)
{
    uint16_t checksum;
    uint8_t crc;
    
    // Ӌ��У��
    checksum = CalculateChecksum(block, offsetof(ParameterBlock, params.reserved.checksum));
    block->params.reserved.checksum = checksum;
    
    // Ӌ��CRC
    crc = CalculateCRC8(block, sizeof(ParameterBlock) - 1);
    block->params.reserved.crc = crc;
}

/**
 * @brief �O�������^�KĬ�Jֵ
 * 
 * @param block �����^�Kָ�
 */
static void SetDefaultParameters(ParameterBlock *block)
{
    memset(block, 0, sizeof(ParameterBlock));
    
    // �O���^�K�^��
    block->header.magic = 0xA55A;
    block->header.version = 1;
    block->header.writeCounter = 1;
    block->header.timestamp = 0;
    
    // �O������ϵ�y����
    block->params.basicSystem.lbt = 58;                 // 2.58V
    block->params.basicSystem.obt = 55;                 // 2.55V
    block->params.basicSystem.factory = 0;              // �Ñ�ģʽ
    block->params.basicSystem.modelNo = 1;              // �aƷ��̖
    block->params.basicSystem.fwNo = 10;                // �g�w�汾 V1.0
    block->params.basicSystem.testCount = 0;            // �yԇ�Δ�
    block->params.basicSystem.codeTableVer = 1;         // ���a��汾
    
    // �O��Ĭ�J�r�g (2024-09-25 00:00:00)
    block->params.basicSystem.year = 24;
    block->params.basicSystem.month = 9;
    block->params.basicSystem.date = 25;
    block->params.basicSystem.hour = 0;
    block->params.basicSystem.minute = 0;
    block->params.basicSystem.second = 0;
    
    // �O���yԇ�h������
    block->params.basicSystem.tempLowLimit = 10;        // 10��C
    block->params.basicSystem.tempHighLimit = 40;       // 40��C
    block->params.basicSystem.measureUnit = 1;          // mg/dL
    block->params.basicSystem.defaultEvent = 1;         // AC
    block->params.basicSystem.stripType = 0;            // GLV(Ѫ��)
    
    // �O��Ӳ�wУ�ʅ���
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
    
    // �O�����yԇ�Ŀ����
    // Ѫ�ǅ���
    block->params.bgParams.bgCsuTolerance = 10;
    block->params.bgParams.bgNdl = 100;
    block->params.bgParams.bgUdl = 50;
    block->params.bgParams.bgBloodIn = 150;
    strcpy((char *)block->params.bgParams.bgStripLot, "GLV24092500");
    block->params.bgParams.bgL = 2;                     // 20 mg/dL
    block->params.bgParams.bgH = 60;                    // 600 mg/dL
    block->params.bgParams.bgT3E37 = 2000;
    
    // �O���yԇ�r�򅢔�
    block->params.bgParams.bgTPL1 = 10;                 // 1��
    block->params.bgParams.bgTRD1 = 10;                 // 1��
    block->params.bgParams.bgEVWidth1 = 5;              // 0.5��
    block->params.bgParams.bgTPL2 = 20;                 // 2��
    block->params.bgParams.bgTRD2 = 20;                 // 2��
    block->params.bgParams.bgEVWidth2 = 10;             // 1��
    
    // �O�������yԇ�Ŀ��Ĭ�J���� (���ᡢ��đ�̴����������֬)
    // ���ᅢ��
    block->params.uaParams.uCsuTolerance = 10;
    block->params.uaParams.uNdl = 100;
    block->params.uaParams.uUdl = 50;
    block->params.uaParams.uBloodIn = 150;
    strcpy((char *)block->params.uaParams.uStripLot, "UA24092500");
    block->params.uaParams.uL = 3;                      // 3 mg/dL
    block->params.uaParams.uH = 20;                     // 20 mg/dL
    block->params.uaParams.uT3E37 = 2000;
    
    // Ӌ��У�ͺ�CRC
    UpdateBlockChecksum(block);
}

/**
 * @brief ��C�����^�K��Ч��
 * 
 * @param block �����^�Kָ�
 * @return true �����^�K��Ч
 * @return false �����^�K�oЧ
 */
static bool ValidateBlock(const ParameterBlock *block)
{
    uint16_t checksum;
    uint8_t crc;
    
    // ��Cħ��
    if (block->header.magic != 0xA55A) {
        return false;
    }
    
    // ��CУ��
    checksum = CalculateChecksum(block, offsetof(ParameterBlock, params.reserved.checksum));
    if (checksum != block->params.reserved.checksum) {
        return false;
    }
    
    // ��CCRC
    crc = CalculateCRC8(block, sizeof(ParameterBlock) - 1);
    if (crc != block->params.reserved.crc) {
        return false;
    }
    
    return true;
}

/* ���_API���� */

/**
 * @brief ��ʼ�������惦
 */
void PARAM_Init(void)
{
    ParameterBlock block;
    
    // ���һ�Ӆ����^�K
    g_activeBlockAddr = FindActiveParameterBlock();
    
    // ����]����Ч�ą����^���t��ʼ�������K����ɂ������^
    if (g_activeBlockAddr == 0) {
        // �O��Ĭ�J����
        SetDefaultParameters(&block);
        
        // �����K���녢���^A
        FlashErasePage(PARAM_BLOCK_A_ADDR);
        FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
        
        // �����K���녢���^B
        FlashErasePage(PARAM_BLOCK_B_ADDR);
        FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
        
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    }
    // ��C�����^�K����Ч��
    else {
        ParameterBlock currentBlock;
        memcpy(&currentBlock, (void *)g_activeBlockAddr, sizeof(ParameterBlock));
        
        if (!ValidateBlock(&currentBlock)) {
            // �����ǰ��Ӆ^�K�oЧ���Lԇʹ����һ���^�K
            uint32_t backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
            memcpy(&currentBlock, (void *)backupAddr, sizeof(ParameterBlock));
            
            if (ValidateBlock(&currentBlock)) {
                g_activeBlockAddr = backupAddr;
            } else {
                // �ɂ��^�K���oЧ�����³�ʼ��
                SetDefaultParameters(&block);
                
                // �����K���녢���^A
                FlashErasePage(PARAM_BLOCK_A_ADDR);
                FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
                
                // �����K���녢���^B
                FlashErasePage(PARAM_BLOCK_B_ADDR);
                FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock));
                
                g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
            }
        }
    }
    
    g_isInitialized = true;
}

/**
 * @brief �xȡ�����^�K
 * 
 * @param blockId �^�KID
 * @param data �������n�^
 * @param size ������С
 * @return true �xȡ�ɹ�
 * @return false �xȡʧ��
 */
bool PARAM_ReadBlock(uint8_t blockId, void *data, uint16_t size)
{
    uint32_t offset;
    uint16_t blockSize;
    
    // �z�酢���惦�Ƿ��ѳ�ʼ��
    if (!g_isInitialized) {
        PARAM_Init();
    }
    
    // �z��^�KID��Ч��
    if (blockId >= BLOCK_MAX) {
        return false;
    }
    
    // �z�龏�n�^ָ�
    if (data == NULL) {
        return false;
    }
    
    // �@ȡ�^�Kƫ�ƺʹ�С
    offset = GetBlockOffset(blockId);
    blockSize = GetBlockSize(blockId);
    
    // �z��ƫ�ƺʹ�С��Ч��
    if (offset == 0 || blockSize == 0) {
        return false;
    }
    
    // �z�龏�n�^��С
    if (size < blockSize) {
        return false;
    }
    
    // �xȡ�^�K����
    memcpy(data, (void *)(g_activeBlockAddr + offset), blockSize);
    
    return true;
}

/**
 * @brief �������^�K
 * 
 * @param blockId �^�KID
 * @param data �������n�^
 * @param size ������С
 * @return true ���³ɹ�
 * @return false ����ʧ��
 */
bool PARAM_UpdateBlock(uint8_t blockId, const void *data, uint16_t size)
{
    ParameterBlock block;
    uint32_t offset;
    uint16_t blockSize;
    uint32_t backupAddr;
    
    // �z�酢���惦�Ƿ��ѳ�ʼ��
    if (!g_isInitialized) {
        PARAM_Init();
    }
    
    // �z��^�KID��Ч��
    if (blockId >= BLOCK_MAX) {
        return false;
    }
    
    // �z�锵��ָ�
    if (data == NULL) {
        return false;
    }
    
    // �@ȡ�^�Kƫ�ƺʹ�С
    offset = GetBlockOffset(blockId);
    blockSize = GetBlockSize(blockId);
    
    // �z��ƫ�ƺʹ�С��Ч��
    if (offset == 0 || blockSize == 0) {
        return false;
    }
    
    // �z�锵����С
    if (size > blockSize) {
        return false;
    }
    
    // �xȡ��ǰ�����^
    memcpy(&block, (void *)g_activeBlockAddr, sizeof(ParameterBlock));
    
    // ���^�K����
    memcpy((uint8_t *)&block + offset, data, size);
    
    // ���^�K�^��
    block.header.writeCounter++;
    block.header.timestamp++; // ���εĕr�g��������Q��RTC�r�g
    
    // ����У�ͺ�CRC
    UpdateBlockChecksum(&block);
    
    // �_����݅^��ַ
    backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    // ������݅^
    if (!FlashErasePage(backupAddr)) {
        return false;
    }
    
    // �����݅^
    if (!FlashWrite(backupAddr, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // ���»�Ӆ����^��ַ
    g_activeBlockAddr = backupAddr;
    
    return true;
}

/**
 * @brief �@ȡϵ�y�r�g
 * 
 * @param year ��ָ�(0-99)
 * @param month ��ָ�(1-12)
 * @param date ��ָ�(1-31)
 * @param hour �rָ�(0-23)
 * @param minute ��ָ�(0-59)
 * @param second ��ָ�(0-59)
 * @return true �@ȡ�ɹ�
 * @return false �@ȡʧ��
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // �O������ֵ
    if (year) *year = basicParams.year;
    if (month) *month = basicParams.month;
    if (date) *date = basicParams.date;
    if (hour) *hour = basicParams.hour;
    if (minute) *minute = basicParams.minute;
    if (second) *second = basicParams.second;
    
    return true;
}

/**
 * @brief ����ϵ�y�r�g
 * 
 * @param year ��(0-99)
 * @param month ��(1-12)
 * @param date ��(1-31)
 * @param hour �r(0-23)
 * @param minute ��(0-59)
 * @param second ��(0-59)
 * @return true ���³ɹ�
 * @return false ����ʧ��
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // ���r�g����
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    // �������^
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief ���Ӝyԇ�Δ�
 * 
 * @return true ���ӳɹ�
 * @return false ����ʧ��
 */
bool PARAM_IncreaseTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // ���Ӝyԇ�Δ�
    basicParams.testCount++;
    
    // �������^
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief �@ȡ�yԇ�Δ�
 * 
 * @param count �yԇ�Δ�ָ�
 * @return true �@ȡ�ɹ�
 * @return false �@ȡʧ��
 */
bool PARAM_GetTestCount(uint16_t *count)
{
    BasicSystemBlock basicParams;
    
    // �z��ָ���Ч��
    if (count == NULL) {
        return false;
    }
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // ���؜yԇ�Δ�
    *count = basicParams.testCount;
    
    return true;
}

/**
 * @brief �֏ͳ��S�O��
 * 
 * @return true �֏ͳɹ�
 * @return false �֏�ʧ��
 */
bool PARAM_ResetToFactory(void)
{
    ParameterBlock block;
    
    // �O��Ĭ�J����
    SetDefaultParameters(&block);
    
    // �����K���녢���^A
    if (!FlashErasePage(PARAM_BLOCK_A_ADDR)) {
        return false;
    }
    
    if (!FlashWrite(PARAM_BLOCK_A_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // �����K���녢���^B
    if (!FlashErasePage(PARAM_BLOCK_B_ADDR)) {
        return false;
    }
    
    if (!FlashWrite(PARAM_BLOCK_B_ADDR, (const uint8_t *)&block, sizeof(ParameterBlock))) {
        return false;
    }
    
    // ���»�Ӆ����^��ַ
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    
    return true;
}

/**
 * @brief ����ԇƬ��ͫ@ȡԇƬ����
 * 
 * @param stripType ԇƬ���
 * @param ndl ��ԇƬ�z�yˮƽָ�
 * @param udl ��ʹ��ԇƬ�z�yˮƽָ�
 * @param bloodIn ѪҺ�z�yˮƽָ�
 * @return true �@ȡ�ɹ�
 * @return false �@ȡʧ��
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    uint8_t blockId;
    uint16_t ndlVal = 0, udlVal = 0, bloodInVal = 0;
    bool result = false;
    
    // ����ԇƬ����x�񅢔��^�K
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
    
    // �O������ֵ
    if (result) {
        if (ndl) *ndl = ndlVal;
        if (udl) *udl = udlVal;
        if (bloodIn) *bloodIn = bloodInVal;
    }
    
    return result;
}

/**
 * @brief ����ԇƬ��ͫ@ȡ�y������
 * 
 * @param stripType ԇƬ���
 * @param lowLimit ����ָ�
 * @param highLimit ����ָ�
 * @return true �@ȡ�ɹ�
 * @return false �@ȡʧ��
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    uint8_t blockId;
    uint8_t lowVal = 0;
    uint16_t highVal = 0;
    bool result = false;
    
    // ����ԇƬ����x�񅢔��^�K
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
    
    // �O������ֵ
    if (result) {
        if (lowLimit) *lowLimit = lowVal;
        if (highLimit) *highLimit = highVal;
    }
    
    return result;
}

/**
 * @brief ����ԇƬ��ͫ@ȡ�yԇ�r�򅢔�
 * 
 * @param stripType ԇƬ���
 * @param tpl �r�g�}�n��ָ�
 * @param trd ԭʼ�����r�gָ�
 * @param evWidth ȼ���r�gָ�
 * @param group �����M�e(1��2)
 * @return true �@ȡ�ɹ�
 * @return false �@ȡʧ��
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    uint8_t blockId;
    uint16_t tplVal = 0, trdVal = 0, evWidthVal = 0;
    bool result = false;
    
    // �z�酢���M�e
    if (group != 1 && group != 2) {
        return false;
    }
    
    // ����ԇƬ����x�񅢔��^�K
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
    
    // �O������ֵ
    if (result) {
        if (tpl) *tpl = tplVal;
        if (trd) *trd = trdVal;
        if (evWidth) *evWidth = evWidthVal;
    }
    
    return result;
}

/**
 * @brief ����yԇӛ�
 * 
 * @param record �yԇӛ�ָ�
 * @return true ����ɹ�
 * @return false ����ʧ��
 */
bool PARAM_SaveTestRecord(const TestRecord *record)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // �yԇӛ䛅^��ʼ��ַ
    uint32_t recordSize = sizeof(TestRecord);     // ӛ䛴�С
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB ӛ䛅^�ɴ惦��ӛ䛔�
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkValue;
    
    // �z��ӛ�ָ�
    if (record == NULL) {
        return false;
    }
    
    // ������һ�����õ�ӛ�λ��
    while (addr < endAddr) {
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        if (checkValue == 0xFFFF) { // ���O��ӛ䛵ęz��0xFFFF
            break;
        }
        addr += recordSize;
    }
    
    // ���ӛ䛅^�ѝM���t��������ӛ䛅^�K���^�_ʼ
    if (addr >= endAddr) {
        FLASH_Unlock();
        
        // ��������ӛ䛅^(6KB, 6�)
        for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += 1024) {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        addr = recordBase; // �ص�ӛ䛅^��ʼ��ַ
    }
    
    // ������ӛ�
    return FlashWrite(addr, (const uint8_t *)record, recordSize);
}

/**
 * @brief �xȡ�yԇӛ�
 * 
 * @param records ӛ䛾��n�^
 * @param startIdx ��ʼ����
 * @param count Ո��ӛ䛔�
 * @param actualCount ���H�xȡ��ӛ䛔�ָ�
 * @return true �xȡ�ɹ�
 * @return false �xȡʧ��
 */
bool PARAM_ReadTestRecords(TestRecord *records, uint16_t startIdx, uint16_t count, uint16_t *actualCount)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // �yԇӛ䛅^��ʼ��ַ
    uint32_t recordSize = sizeof(TestRecord);     // ӛ䛴�С
    uint16_t totalCount = 0;
    uint16_t readCount = 0;
    uint16_t checkValue;
    
    // �z�酢��
    if (records == NULL || count == 0) {
        return false;
    }
    
    // Ӌ�㿂ӛ䛔�
    totalCount = PARAM_GetTestRecordCount();
    
    // �z����ʼ����
    if (startIdx >= totalCount) {
        if (actualCount) *actualCount = 0;
        return true;
    }
    
    // �����xȡ����
    if (startIdx + count > totalCount) {
        count = totalCount - startIdx;
    }
    
    // �xȡӛ�
    for (uint16_t i = 0; i < count; i++) {
        uint32_t addr = recordBase + (startIdx + i) * recordSize;
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        
        // �z��ӛ��Ƿ���Ч
        if (checkValue != 0xFFFF) {
            memcpy(&records[readCount], (void *)addr, recordSize);
            readCount++;
        }
    }
    
    // �O�����H�xȡ����
    if (actualCount) *actualCount = readCount;
    
    return true;
}

/**
 * @brief �@ȡ�yԇӛ䛿���
 * 
 * @return uint16_t �yԇӛ䛿���
 */
uint16_t PARAM_GetTestRecordCount(void)
{
    uint32_t recordBase = PARAM_RECORD_ADDR;      // �yԇӛ䛅^��ʼ��ַ
    uint32_t recordSize = sizeof(TestRecord);     // ӛ䛴�С
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB ӛ䛅^�ɴ惦��ӛ䛔�
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t count = 0;
    uint16_t checkValue;
    
    // ������ʹ�õ�ӛ䛔�
    while (addr < endAddr) {
        memcpy(&checkValue, (void *)addr, sizeof(uint16_t));
        if (checkValue == 0xFFFF) { // ���O��ӛ䛵ęz��0xFFFF
            break;
        }
        count++;
        addr += recordSize;
    }
    
    return count;
} 