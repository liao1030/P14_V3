/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : CH32V203 Flash�������淽�����F
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "P14_Flash_Storage.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

/* ��Ӆ����^�K��ַ */
static uint32_t g_activeBlockAddr = 0;

/* �����팍�� */
static ParameterBlock g_paramBlock;

/* Ĭ�J������ֵ */
static const ParameterBlock g_defaultParams = {
    /* �������^�� */
    .header = {
        .magic = 0xA55A,
        .version = 1,
        .writeCounter = 1,
        .timestamp = 0,
        .checksum = 0
    },
    
    /* �����^ */
    .params = {
        /* ����ϵ�y���� */
        .basicSystem = {
            .lbt = 58,              // ��늳��ֵ (2.58V)
            .obt = 55,              // 늳غıM�ֵ (2.55V)
            .factory = 0,           // ʹ����ģʽ
            .modelNo = 1,           // �aƷ��̖
            .fwNo = 10,             // �g�w�汾̖ 1.0
            .testCount = 0,         // �yԇ�Δ���ʼ��0
            .codeTableVer = 1,      // ���a��汾̖
            .year = 24,             // ����O�� (2024��)
            .month = 7,             // �·��O�� (7��)
            .date = 25,             // �����O�� (25��)
            .hour = 12,             // С�r�O�� (12�r)
            .minute = 0,            // ����O�� (0��)
            .second = 0,            // �딵�O�� (0��)
            .tempLowLimit = 10,     // �����ضȹ������� (10��C)
            .tempHighLimit = 40,    // �����ضȹ������� (40��C)
            .measureUnit = 1,       // ��Ȇ�λ�O�� (mg/dL)
            .defaultEvent = 1,      // ����¼���� (AC)
            .stripType = 0,         // �yԇ�Ŀ (Ѫ��)
            .reserved = {0, 0}      // �A��λ��
        },
        
        /* Ӳ�wУ������ */
        .hardwareCalib = {
            .evT3Trigger = 800,     // EV_T3�|�l늉� (800mV)
            .evWorking = 164,       // ��ȹ������늉�
            .evT3 = 164,            // ѪҺ�������늉�
            .dacoOffset = 0,        // DACƫ���a��
            .dacdo = 0,             // DACУ�����텢��
            .cc211Status = 0,       // CC211δ������
            .calTolerance = 8,      // OPS/OPIУ���ݲ� (0.8%)
            .ops = 1.0f,            // OPAУ��б��
            .opi = 0.0f,            // OPAУ���ؾ�
            .qct = 0,               // QCTУ���yԇ��λԪ�M
            .tempOffset = 0,        // �ض�У��ƫ��
            .batteryOffset = 0,     // 늳�У��ƫ��
            .reserved = {0, 0, 0, 0, 0}  // �A��λ��
        },
        
        /* Ѫ�ǅ��� */
        .bgParams = {
            .bgCsuTolerance = 10,   // ԇƬ�z���ݲ�
            .bgNdl = 100,           // ��ԇƬ���ˮƽ
            .bgUdl = 50,            // ��ʹ��ԇƬ���ˮƽ
            .bgBloodIn = 150,       // ѪҺ���ˮƽ
            .bgStripLot = {0},      // ԇƬ��̖����ʼ��ȫ0
            .bgL = 20,              // Ѫ��ֵ���� (20 mg/dL)
            .bgH = 600,             // Ѫ��ֵ���� (600 mg/dL)
            .bgT3E37 = 2000,        // T3 ADV��E37�ֵ
            .bgTPL1 = 10,           // ��һ�M�r�򅢔�TPL (1.0��)
            .bgTRD1 = 10,           // ��һ�M�r�򅢔�TRD (1.0��)
            .bgEVWidth1 = 5,        // ��һ�M�r�򅢔�EV���� (0.5��)
            .bgTPL2 = 20,           // �ڶ��M�r�򅢔�TPL (2.0��)
            .bgTRD2 = 20,           // �ڶ��M�r�򅢔�TRD (2.0��)
            .bgEVWidth2 = 10,       // �ڶ��M�r�򅢔�EV���� (1.0��)
            .bgCompA = 0,           // �a������A
            .bgCompB = 0,           // �a������B
            .bgCompC = 0,           // �a������C
            .bgCompD = 0,           // �a������D
            .reserved = {0}         // �A��λ��
        },
        
        /* ���ᅢ�� */
        .uaParams = {
            .uCsuTolerance = 10,    // ԇƬ�z���ݲ�
            .uNdl = 100,            // ��ԇƬ���ˮƽ
            .uUdl = 50,             // ��ʹ��ԇƬ���ˮƽ
            .uBloodIn = 150,        // ѪҺ���ˮƽ
            .uStripLot = {0},       // ԇƬ��̖����ʼ��ȫ0
            .uL = 3,                // ����ֵ���� (3 mg/dL)
            .uH = 20,               // ����ֵ���� (20 mg/dL)
            .uT3E37 = 2000,         // T3 ADV��E37�ֵ
            .uTPL1 = 12,            // ��һ�M�r�򅢔�TPL (1.2��)
            .uTRD1 = 12,            // ��һ�M�r�򅢔�TRD (1.2��)
            .uEVWidth1 = 6,         // ��һ�M�r�򅢔�EV���� (0.6��)
            .uTPL2 = 25,            // �ڶ��M�r�򅢔�TPL (2.5��)
            .uTRD2 = 25,            // �ڶ��M�r�򅢔�TRD (2.5��)
            .uEVWidth2 = 12,        // �ڶ��M�r�򅢔�EV���� (1.2��)
            .uCompA = 0,            // �a������A
            .uCompB = 0,            // �a������B
            .uCompC = 0,            // �a������C
            .uCompD = 0,            // �a������D
            .reserved = {0}         // �A��λ��
        },
        
        /* ��đ�̴����� */
        .cholParams = {
            .cCsuTolerance = 10,    // ԇƬ�z���ݲ�
            .cNdl = 100,            // ��ԇƬ���ˮƽ
            .cUdl = 50,             // ��ʹ��ԇƬ���ˮƽ
            .cBloodIn = 150,        // ѪҺ���ˮƽ
            .cStripLot = {0},       // ԇƬ��̖����ʼ��ȫ0
            .cL = 100,              // đ�̴�ֵ���� (100 mg/dL)
            .cH = 400,              // đ�̴�ֵ���� (400 mg/dL)
            .cT3E37 = 2000,         // T3 ADV��E37�ֵ
            .cTPL1 = 15,            // ��һ�M�r�򅢔�TPL (1.5��)
            .cTRD1 = 15,            // ��һ�M�r�򅢔�TRD (1.5��)
            .cEVWidth1 = 7,         // ��һ�M�r�򅢔�EV���� (0.7��)
            .cTPL2 = 30,            // �ڶ��M�r�򅢔�TPL (3.0��)
            .cTRD2 = 30,            // �ڶ��M�r�򅢔�TRD (3.0��)
            .cEVWidth2 = 15,        // �ڶ��M�r�򅢔�EV���� (1.5��)
            .cCompA = 0,            // �a������A
            .cCompB = 0,            // �a������B
            .cCompC = 0,            // �a������C
            .cCompD = 0,            // �a������D
            .reserved = {0}         // �A��λ��
        },
        
        /* �������֬���� */
        .tgParams = {
            .tgCsuTolerance = 10,   // ԇƬ�z���ݲ�
            .tgNdl = 100,           // ��ԇƬ���ˮƽ
            .tgUdl = 50,            // ��ʹ��ԇƬ���ˮƽ
            .tgBloodIn = 150,       // ѪҺ���ˮƽ
            .tgStripLot = {0},      // ԇƬ��̖����ʼ��ȫ0
            .tgL = 50,              // �������ֵ֬���� (50 mg/dL)
            .tgH = 500,             // �������ֵ֬���� (500 mg/dL)
            .tgT3E37 = 2000,        // T3 ADV��E37�ֵ
            .tgTPL1 = 18,           // ��һ�M�r�򅢔�TPL (1.8��)
            .tgTRD1 = 18,           // ��һ�M�r�򅢔�TRD (1.8��)
            .tgEVWidth1 = 9,        // ��һ�M�r�򅢔�EV���� (0.9��)
            .tgTPL2 = 35,           // �ڶ��M�r�򅢔�TPL (3.5��)
            .tgTRD2 = 35,           // �ڶ��M�r�򅢔�TRD (3.5��)
            .tgEVWidth2 = 17,       // �ڶ��M�r�򅢔�EV���� (1.7��)
            .tgCompA = 0,           // �a������A
            .tgCompB = 0,           // �a������B
            .tgCompC = 0,           // �a������C
            .tgCompD = 0,           // �a������D
            .reserved = {0}         // �A��λ��
        },
        
        /* ����^�K */
        .reserved = {
            .reserved = {0},        // �A��λ��
            .calibChecksum = 0      // У�^У��
        }
    }
};

/* �o�B�������� */
static void UpdateChecksum(ParameterBlock *block);
static bool VerifyChecksum(const ParameterBlock *block);
static void ValidateActiveBlock(void);
static bool SaveParameterBlock(void);
static uint16_t CalculateChecksum(const uint8_t *data, uint16_t size);
static uint32_t GetBlockOffsetByType(uint8_t blockType);
static uint16_t GetBlockSizeByType(uint8_t blockType);

/**
 * @brief ��ʼ��Flash����ϵ�y
 */
void FLASH_Storage_Init(void)
{
    /* ��C��Ӆ����^�K */
    ValidateActiveBlock();
    
    printf("Flash��������ϵ�y��ʼ�����\r\n");
    printf("��Ӆ����^�K��ַ: 0x%08lX\r\n", g_activeBlockAddr);
    printf("������汾: %d, ����Ӌ��: %d\r\n", 
           g_paramBlock.header.version, 
           g_paramBlock.header.writeCounter);
}

/**
 * @brief ����������Flash
 * @param address: Ŀ�˵�ַ
 * @param data: ����ָ�
 * @param size: ������С
 * @return �����Ƿ�ɹ�
 */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size)
{
    /* �����z�� */
    if (data == NULL || size == 0) {
        return false;
    }
    
    /* �z���ַ�Ƿ���Flash������ */
    if (address < PARAM_FLASH_BASE || 
        address + size > PARAM_FLASH_BASE + 64 * 1024) { // 64KB����С
        return false;
    }
    
    /* ÿ�Ό���2�ֹ� */
    uint32_t i;
    uint32_t aligned_size = (size + 1) & ~1; // ���ό��R��ż��
    FLASH_Status status;
    
    for (i = 0; i < aligned_size; i += 2) {
        uint16_t halfword;
        
        if (i + 1 < size) {
            halfword = (uint16_t)data[i] | ((uint16_t)data[i + 1] << 8);
        } else if (i < size) {
            /* ���ֻʣ����һ���ֹ������ֹ���0xFF��� */
            halfword = (uint16_t)data[i] | 0xFF00;
        } else {
            /* ���R��� */
            halfword = 0xFFFF;
        }
        
        status = FLASH_ProgramHalfWord(address + i, halfword);
        if (status != FLASH_COMPLETE) {
            printf("Flash�����e�`: ��ַ0x%08lX, ��B%d\r\n", address + i, status);
            return false;
        }
    }
    
    return true;
}

/**
 * @brief ��Flash�xȡ����
 * @param address: Դ��ַ
 * @param data: Ŀ�˾��n�^
 * @param size: ������С
 * @return �����Ƿ�ɹ�
 */
bool FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t size)
{
    /* �����z�� */
    if (data == NULL || size == 0) {
        return false;
    }
    
    /* �z���ַ�Ƿ���Flash������ */
    if (address < PARAM_FLASH_BASE || 
        address + size > PARAM_FLASH_BASE + 64 * 1024) { // 64KB����С
        return false;
    }
    
    /* ֱ�ӏ�Flash�xȡ */
    memcpy(data, (const void *)address, size);
    
    return true;
}

/**
 * @brief ��C��Ӆ����^�K
 */
static void ValidateActiveBlock(void)
{
    ParameterBlock blockA, blockB;
    bool validA = false, validB = false;
    
    /* �xȡ�����^A */
    if (FLASH_ReadBuffer(PARAM_BLOCK_A_ADDR, (uint8_t *)&blockA, sizeof(ParameterBlock))) {
        validA = (blockA.header.magic == 0xA55A) && VerifyChecksum(&blockA);
    }
    
    /* �xȡ�����^B */
    if (FLASH_ReadBuffer(PARAM_BLOCK_B_ADDR, (uint8_t *)&blockB, sizeof(ParameterBlock))) {
        validB = (blockB.header.magic == 0xA55A) && VerifyChecksum(&blockB);
    }
    
    /* �_����Ӆ����^ */
    if (validA && validB) {
        /* �ɂ��^�K����Ч�����^����Ӌ���� */
        if (blockA.header.writeCounter >= blockB.header.writeCounter) {
            g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
            memcpy(&g_paramBlock, &blockA, sizeof(ParameterBlock));
        } else {
            g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
            memcpy(&g_paramBlock, &blockB, sizeof(ParameterBlock));
        }
    } else if (validA) {
        /* ֻ��A�^�K��Ч */
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        memcpy(&g_paramBlock, &blockA, sizeof(ParameterBlock));
    } else if (validB) {
        /* ֻ��B�^�K��Ч */
        g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
        memcpy(&g_paramBlock, &blockB, sizeof(ParameterBlock));
    } else {
        /* �ɂ��^�K���oЧ��ʹ��Ĭ�J���� */
        memcpy(&g_paramBlock, &g_defaultParams, sizeof(ParameterBlock));
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        
        /* ����У�� */
        UpdateChecksum(&g_paramBlock);
        
        /* ���浽Flash */
        SaveParameterBlock();
    }
}

/**
 * @brief ���慢���^�K��Flash
 * @return �����Ƿ�ɹ�
 */
static bool SaveParameterBlock(void)
{
    uint32_t targetAddr;
    
    /* �����^����ʹ�ã�����ͬһ�^�򷴸����� */
    if (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) {
        targetAddr = PARAM_BLOCK_B_ADDR;
    } else {
        targetAddr = PARAM_BLOCK_A_ADDR;
    }
    
    /* ���ӌ���Ӌ���� */
    g_paramBlock.header.writeCounter++;
    
    /* ����У�� */
    UpdateChecksum(&g_paramBlock);
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ����Ŀ���ȅ^ */
    FLASH_Status status1 = FLASH_ErasePage(targetAddr);
    FLASH_Status status2 = FLASH_COMPLETE;
    
    /* ��������^���1KB����Ҫ�����ڶ����ȅ^ */
    if (sizeof(ParameterBlock) > PARAM_SECTOR_SIZE) {
        status2 = FLASH_ErasePage(targetAddr + PARAM_SECTOR_SIZE);
    }
    
    if (status1 != FLASH_COMPLETE || status2 != FLASH_COMPLETE) {
        FLASH_Lock();
        printf("����Flashʧ��: ��B1=%d, ��B2=%d\r\n", status1, status2);
        return false;
    }
    
    /* ���녢���^ */
    bool result = FLASH_WriteBuffer(targetAddr, (const uint8_t *)&g_paramBlock, sizeof(ParameterBlock));
    
    /* �i��Flash */
    FLASH_Lock();
    
    if (result) {
        /* ���»�Ӆ����^��ַ */
        g_activeBlockAddr = targetAddr;
        return true;
    }
    
    return false;
}

/**
 * @brief Ӌ�㔵��У��
 * @param data: ����ָ�
 * @param size: ������С
 * @return У��
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
 * @brief �������^�KУ��
 * @param block: �����^�Kָ�
 */
static void UpdateChecksum(ParameterBlock *block)
{
    if (block != NULL) {
        /* �Ȍ�У������ */
        block->header.checksum = 0;
        
        /* Ӌ��У�� */
        block->header.checksum = CalculateChecksum(
            (const uint8_t *)block, 
            sizeof(ParameterBlock)
        );
    }
}

/**
 * @brief ��C�����^�KУ��
 * @param block: �����^�Kָ�
 * @return У��Ƿ�ͨ�^
 */
static bool VerifyChecksum(const ParameterBlock *block)
{
    if (block != NULL) {
        /* ����ԭУ�� */
        uint16_t storedChecksum = block->header.checksum;
        
        /* �����R�r�������Ӌ�� */
        ParameterBlock tempBlock;
        memcpy(&tempBlock, block, sizeof(ParameterBlock));
        
        /* ����У���ֶ� */
        tempBlock.header.checksum = 0;
        
        /* Ӌ��У�� */
        uint16_t calculatedChecksum = CalculateChecksum(
            (const uint8_t *)&tempBlock, 
            sizeof(ParameterBlock)
        );
        
        /* ���^У�� */
        return (calculatedChecksum == storedChecksum);
    }
    
    return false;
}

/**
 * @brief �@ȡ�^�K��͌�����ƫ�Ƶ�ַ
 * @param blockType: �^�K���
 * @return ƫ�Ƶ�ַ
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
            offset = 0xFFFFFFFF; // �oЧƫ��
            break;
    }
    
    /* �����^�څ������е�ƫ���� */
    if (offset != 0xFFFFFFFF) {
        offset += offsetof(ParameterBlock, params);
    }
    
    return offset;
}

/**
 * @brief �@ȡ�^�K��͌����Ĵ�С
 * @param blockType: �^�K���
 * @return �^�K��С
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
            size = 0; // �oЧ��С
            break;
    }
    
    return size;
}

/**
 * @brief �xȡָ����͵ą����^�K
 * @param blockType: �^�K���
 * @param buffer: ���n�^ָ�
 * @param size: ���n�^��С
 * @return �����Ƿ�ɹ�
 */
bool PARAM_ReadParameterBlock(uint8_t blockType, void *buffer, uint16_t size)
{
    if (buffer == NULL) {
        return false;
    }
    
    /* �@ȡ�^�Kƫ�ƺʹ�С */
    uint32_t offset = GetBlockOffsetByType(blockType);
    uint16_t blockSize = GetBlockSizeByType(blockType);
    
    /* �z�酢���Ƿ���Ч */
    if (offset == 0xFFFFFFFF || blockSize == 0 || size > blockSize) {
        return false;
    }
    
    /* Ӌ��^�K�ھ����еĵ�ַ */
    uint8_t *srcAddr = (uint8_t *)&g_paramBlock + offset;
    
    /* �}�u���� */
    memcpy(buffer, srcAddr, size);
    
    return true;
}

/**
 * @brief ����ָ����͵ą����^�K
 * @param blockType: �^�K���
 * @param buffer: �������n�^
 * @param size: ���n�^��С
 * @return �����Ƿ�ɹ�
 */
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size)
{
    if (buffer == NULL) {
        return false;
    }
    
    /* �@ȡ�^�Kƫ�ƺʹ�С */
    uint32_t offset = GetBlockOffsetByType(blockType);
    uint16_t blockSize = GetBlockSizeByType(blockType);
    
    /* �z�酢���Ƿ���Ч */
    if (offset == 0xFFFFFFFF || blockSize == 0 || size > blockSize) {
        return false;
    }
    
    /* Ӌ��^�K�ڃȴ��еĵ�ַ */
    uint8_t *destAddr = (uint8_t *)&g_paramBlock + offset;
    
    /* �}�u���� */
    memcpy(destAddr, buffer, size);
    
    /* ���浽Flash */
    return SaveParameterBlock();
}

/**
 * @brief �@ȡϵ�y�r�g
 * @param year: ���ָ�
 * @param month: �·�ָ�
 * @param date: ����ָ�
 * @param hour: С�rָ�
 * @param minute: ���ָ�
 * @param second: �딵ָ�
 * @return �����Ƿ�ɹ�
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* �xȡϵ�y�����K */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* ��r�g���� */
    if (year != NULL) *year = basicParams.year;
    if (month != NULL) *month = basicParams.month;
    if (date != NULL) *date = basicParams.date;
    if (hour != NULL) *hour = basicParams.hour;
    if (minute != NULL) *minute = basicParams.minute;
    if (second != NULL) *second = basicParams.second;
    
    return true;
}

/**
 * @brief ����ϵ�y�r�g
 * @param year: ���
 * @param month: �·�
 * @param date: ����
 * @param hour: С�r
 * @param minute: ���
 * @param second: �딵
 * @return �����Ƿ�ɹ�
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    /* �z��r�g������Ч�� */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    
    /* �xȡϵ�y�����K */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* ��r�g���� */
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    /* ���µ������K�� */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief ���Ӝyԇ�Δ�
 * @return �����Ƿ�ɹ�
 */
bool PARAM_IncreaseTestCount(void)
{
    /* �xȡϵ�y�����K */
    BasicSystemBlock basicParams;
    if (!PARAM_ReadParameterBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    /* ���Ӝyԇ�Δ� */
    basicParams.testCount++;
    
    /* ���µ������K�� */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief ����ԇƬ��ͫ@ȡԇƬ����
 * @param stripType: ԇƬ���
 * @param ndl: ��ԇƬ���ˮƽָ�
 * @param udl: ��ʹ��ԇƬ���ˮƽָ�
 * @param bloodIn: ѪҺ���ˮƽָ�
 * @return �����Ƿ�ɹ�
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* Ѫ�ǅ��� */
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
                /* ���ᅢ�� */
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
                /* ��đ�̴����� */
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
                /* �������֬���� */
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
 * @brief ����ԇƬ��ͫ@ȡ�y������
 * @param stripType: ԇƬ���
 * @param lowLimit: ����ָ�
 * @param highLimit: ����ָ�
 * @return �����Ƿ�ɹ�
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* Ѫ�ǅ��� */
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
                /* ���ᅢ�� */
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
                /* ��đ�̴����� */
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
                /* �������֬���� */
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
 * @brief ����ԇƬ��ͫ@ȡ�r�򅢔�
 * @param stripType: ԇƬ���
 * @param tpl: TPL�r�gָ�
 * @param trd: TRD�r�gָ�
 * @param evWidth: EV����ָ�
 * @param group: �r��M�e (1��2)
 * @return �����Ƿ�ɹ�
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    /* �z��r��M�e��Ч�� */
    if (group != 1 && group != 2) {
        return false;
    }
    
    switch (stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            {
                /* Ѫ�ǅ��� */
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
                /* ���ᅢ�� */
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
                /* ��đ�̴����� */
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
                /* �������֬���� */
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
 * @brief ���Å�����Ĭ�Jֵ
 */
void PARAM_ResetToDefault(void)
{
    /* �}�uĬ�J���� */
    memcpy(&g_paramBlock, &g_defaultParams, sizeof(ParameterBlock));
    
    /* ����У�� */
    UpdateChecksum(&g_paramBlock);
    
    /* ���浽Flash */
    SaveParameterBlock();
    
    printf("���������Þ�Ĭ�Jֵ\r\n");
} 