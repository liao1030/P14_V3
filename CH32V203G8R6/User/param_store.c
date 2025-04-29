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

/* ȫ��׃�� */
static uint32_t g_activeBlockAddr = 0;               /* ��ǰ��Ӆ����^�ĵ�ַ */
static ParamBlockHeader_TypeDef g_blockHeaders[BLOCK_COUNT]; /* �����^�K���^ */
static uint16_t g_blockOffsets[BLOCK_COUNT+1];       /* ���^�K��Flash�е�ƫ�� */

/* ���غ���ԭ�� */
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
 * @brief   ��ʼ����������ģ�M
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_Init(void)
{
    /* ��ʼ���^�Kƫ�Ʊ� */
    PARAM_InitBlockOffsets();
    
    printf("���������ʼ��...\r\n");
    
    /* �Lԇ�ą^�KA�xȡ���� */
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    if (PARAM_LoadActiveBlock() == 0) {
        printf("�яą����^�KA�d�녢��\r\n");
        return 0;
    }
    
    /* �Lԇ�ą^�KB�xȡ���� */
    g_activeBlockAddr = PARAM_BLOCK_B_ADDR;
    if (PARAM_LoadActiveBlock() == 0) {
        printf("�яą����^�KB�d�녢��\r\n");
        return 0;
    }
    
    /* ����ɂ��^�K���o���xȡ���t�d���A�O���� */
    printf("�o���xȡ�������d���A�Oֵ\r\n");
    if (PARAM_LoadDefaultParams() == 0) {
        /* ����^�KA */
        g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
        return 0;
    }
    
    printf("������ʼ��ʧ��\r\n");
    return 1;
}

/*********************************************************************
 * @fn      PARAM_InitBlockOffsets
 *
 * @brief   ��ʼ���������^�K��ƫ����
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t PARAM_InitBlockOffsets(void)
{
    uint16_t offset = 0;
    
    /* ���^�^ */
    g_blockOffsets[0] = offset;
    offset += sizeof(ParamBlockHeader_TypeDef) * BLOCK_COUNT;
    
    /* ����ϵ�y�����^ */
    g_blockOffsets[1] = offset;
    offset += sizeof(BasicSystemBlock);
    
    /* Ӳ�wУ�ʅ����^ */
    g_blockOffsets[2] = offset;
    offset += sizeof(HardwareCalibBlock);
    
    /* Ѫ�ǅ����^ */
    g_blockOffsets[3] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* ���ᅢ���^ */
    g_blockOffsets[4] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* đ�̴������^ */
    g_blockOffsets[5] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* �������֬�����^ */
    g_blockOffsets[6] = offset;
    offset += sizeof(TestParamBaseBlock);
    
    /* ����^�cУ�^ */
    g_blockOffsets[7] = offset;
    
    /* ���L��ƫ�� */
    g_blockOffsets[BLOCK_COUNT] = offset + 32; // ��У�^�ͱ���^�A��32�ֹ�
    
    /* �z�酢���^�L�Ȳ����^Flash퓴�С */
    if (g_blockOffsets[BLOCK_COUNT] > FLASH_PAGE_SIZE) {
        printf("�����^��С���^Flash퓴�С��\r\n");
        return 1;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_LoadActiveBlock
 *
 * @brief   �d�뮔ǰ��Ӆ����^
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t PARAM_LoadActiveBlock(void)
{
    /* �xȡ���И��^ */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        uint32_t headerAddr = g_activeBlockAddr + g_blockOffsets[i];
        memcpy(&g_blockHeaders[i], (void *)headerAddr, sizeof(ParamBlockHeader_TypeDef));
        
        /* ��C���^ */
        if (PARAM_VerifyBlockHeader(&g_blockHeaders[i]) != 0) {
            return 1;
        }
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_VerifyBlockHeader
 *
 * @brief   ��C�����^�K���^
 *
 * @param   header - ����C�Ę��^�Y��
 *
 * @return  ��B (0: ��Ч, ��0: �oЧ)
 */
static uint8_t PARAM_VerifyBlockHeader(ParamBlockHeader_TypeDef *header)
{
    /* �z��ħ�g�a */
    if (header->magic != 0xA55A) {
        return 1;
    }
    
    /* �汾̖��횴�춵��1 */
    if (header->version < 1) {
        return 2;
    }
    
    /* ����Ӌ������횴�춵��1 */
    if (header->writeCounter < 1) {
        return 3;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   �xȡָ�������^�K
 *
 * @param   blockType - Ҫ�xȡ�ą^�K���
 * @param   buffer - ���Ք����ľ��n�^
 * @param   size - ���n�^��С
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_ReadBlock(ParamBlockType blockType, void *buffer, uint16_t size)
{
    /* �z�酢�� */
    if (blockType >= BLOCK_COUNT || buffer == NULL || g_activeBlockAddr == 0) {
        return 1;
    }
    
    /* �z�龏�n�^��С */
    uint16_t blockSize = g_blockOffsets[blockType + 1] - g_blockOffsets[blockType] - sizeof(ParamBlockHeader_TypeDef);
    if (size < blockSize) {
        return 2;
    }
    
    /* Ӌ�㔵����ַ */
    uint32_t dataAddr = g_activeBlockAddr + g_blockOffsets[blockType] + sizeof(ParamBlockHeader_TypeDef);
    
    /* �}�u���������n�^ */
    memcpy(buffer, (void *)dataAddr, blockSize);
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateBlock
 *
 * @brief   ����ָ�������^�K
 *
 * @param   blockType - Ҫ���µą^�K���
 * @param   buffer - ��������
 * @param   size - ������С
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_UpdateBlock(ParamBlockType blockType, const void *buffer, uint16_t size)
{
    /* �z�酢�� */
    if (blockType >= BLOCK_COUNT || buffer == NULL || g_activeBlockAddr == 0) {
        return 1;
    }
    
    /* �z�龏�n�^��С */
    uint16_t blockSize = g_blockOffsets[blockType + 1] - g_blockOffsets[blockType] - sizeof(ParamBlockHeader_TypeDef);
    if (size > blockSize) {
        return 2;
    }
    
    /* �����^�YӍ */
    g_blockHeaders[blockType].writeCounter++;
    
    /* ȡ��ǰϵ�y�r�g����r�g�� */
    /* �ڌ��H�����п���ʹ��RTC�r�g��ϵ�y�r�g����r�g�� */
    g_blockHeaders[blockType].timestamp = (uint32_t)((g_blockHeaders[blockType].writeCounter << 16) | blockType);
    
    /* ���浽��݅^�K */
    uint32_t backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    /* ������݅^�K */
    if (PARAM_EraseFlashPage(backupAddr) != 0) {
        return 3;
    }
    
    /* �}�u���Ѕ����^�K���R�r���n�^ */
    uint8_t tempBuf[FLASH_PAGE_SIZE];
    memset(tempBuf, 0xFF, FLASH_PAGE_SIZE);
    
    /* �}�u���И��^ */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        memcpy(&tempBuf[g_blockOffsets[i]], &g_blockHeaders[i], sizeof(ParamBlockHeader_TypeDef));
    }
    
    /* �}�u���Д����^�K */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        uint32_t srcAddr = g_activeBlockAddr + g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef);
        if (i == blockType) {
            /* ʹ������ */
            memcpy(&tempBuf[g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef)], buffer, size);
        } else {
            /* ʹ��ԭ�Д��� */
            uint16_t dataSize = g_blockOffsets[i + 1] - g_blockOffsets[i] - sizeof(ParamBlockHeader_TypeDef);
            memcpy(&tempBuf[g_blockOffsets[i] + sizeof(ParamBlockHeader_TypeDef)], (void *)srcAddr, dataSize);
        }
    }
    
    /* Ӌ��У�� */
    uint8_t checksum = PARAM_CalculateChecksum(tempBuf, g_blockOffsets[BLOCK_COUNT] - 1);
    tempBuf[g_blockOffsets[BLOCK_COUNT] - 1] = checksum;
    
    /* ���뵽��݅^�K */
    if (PARAM_WriteToFlash(backupAddr, tempBuf, g_blockOffsets[BLOCK_COUNT]) != 0) {
        return 4;
    }
    
    /* �ГQ��Ӆ^�K */
    g_activeBlockAddr = backupAddr;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateDateTime
 *
 * @brief   �������ڕr�g����
 *
 * @param   year - ���(0-99)
 * @param   month - �·�(1-12)
 * @param   date - ����(1-31)
 * @param   hour - С�r(0-23)
 * @param   minute - ���(0-59)
 * @param   second - ��(0-59)
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    /* �z�酢����Ч�� */
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return 1;
    }
    
    /* �xȡ��ǰ����ϵ�y���� */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 2;
    }
    
    /* �������ڕr�g */
    sysBlock.year = year;
    sysBlock.month = month;
    sysBlock.date = date;
    sysBlock.hour = hour;
    sysBlock.minute = minute;
    sysBlock.second = second;
    
    /* ���؅����^ */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   �@ȡ���ڕr�g����
 *
 * @param   year - ���ָ��
 * @param   month - �·�ָ��
 * @param   date - ����ָ��
 * @param   hour - С�rָ��
 * @param   minute - ���ָ��
 * @param   second - ��ָ��
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* �z��ָ����Ч�� */
    if (!year || !month || !date || !hour || !minute || !second) {
        return 1;
    }
    
    /* �xȡ����ϵ�y���� */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 2;
    }
    
    /* ��䔵�� */
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
 * @brief   ���ӜyԇӋ����
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_IncreaseTestCount(void)
{
    /* �xȡ��ǰ����ϵ�y���� */
    BasicSystemBlock sysBlock;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock)) != 0) {
        return 1;
    }
    
    /* ����Ӌ���� */
    sysBlock.testCount++;
    
    /* ���؅����^ */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &sysBlock, sizeof(BasicSystemBlock));
}

/*********************************************************************
 * @fn      PARAM_GetActiveBlockAddr
 *
 * @brief   ȡ�î�ǰ��Ӆ����^��ַ
 *
 * @return  ��Ӆ����^��ַ
 */
uint32_t PARAM_GetActiveBlockAddr(void)
{
    return g_activeBlockAddr;
}

/*********************************************************************
 * @fn      PARAM_CalculateChecksum
 *
 * @brief   Ӌ��У��
 *
 * @param   data - ����ָ��
 * @param   size - ������С
 *
 * @return  У��
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
 * @brief   ����Flash�
 *
 * @param   address - 퓵���ʼ��ַ
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t PARAM_EraseFlashPage(uint32_t address)
{
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ����� */
    FLASH_Status status = FLASH_ErasePage(address);
    
    /* �i��Flash */
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE) ? 0 : 1;
}

/*********************************************************************
 * @fn      PARAM_WriteToFlash
 *
 * @brief   ���딵����Flash
 *
 * @param   address - �����ַ
 * @param   data - ����ָ��
 * @param   size - ������С
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t PARAM_WriteToFlash(uint32_t address, const void *data, uint16_t size)
{
    /* ���֌��R�z�� */
    if (size % 2 != 0) {
        return 1;
    }
    
    /* ���iFlash */
    FLASH_Unlock();
    
    uint16_t *ptr = (uint16_t *)data;
    FLASH_Status status = FLASH_COMPLETE;
    
    /* �����֌��� */
    for (uint16_t i = 0; i < size / 2; i++) {
        status = FLASH_ProgramHalfWord(address + i * 2, ptr[i]);
        if (status != FLASH_COMPLETE) {
            break;
        }
    }
    
    /* �i��Flash */
    FLASH_Lock();
    
    return (status == FLASH_COMPLETE) ? 0 : 2;
}

/*********************************************************************
 * @fn      PARAM_LoadDefaultParams
 *
 * @brief   �d���A�O����
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t PARAM_LoadDefaultParams(void)
{
    /* �O�����Ѕ����^�K���^���A�Oֵ */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        g_blockHeaders[i].magic = 0xA55A;
        g_blockHeaders[i].version = 1;
        g_blockHeaders[i].writeCounter = 1;
        g_blockHeaders[i].timestamp = 0;
    }
    
    /* ���������^�KA */
    if (PARAM_EraseFlashPage(PARAM_BLOCK_A_ADDR) != 0) {
        return 1;
    }
    
    /* �ʂ䅢���^�R�r���n�^ */
    uint8_t tempBuf[FLASH_PAGE_SIZE];
    memset(tempBuf, 0xFF, FLASH_PAGE_SIZE);
    
    /* �����^ */
    for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
        memcpy(&tempBuf[g_blockOffsets[i]], &g_blockHeaders[i], sizeof(ParamBlockHeader_TypeDef));
    }
    
    /* ��ʼ������ϵ�y���� */
    BasicSystemBlock sysBlock;
    memset(&sysBlock, 0, sizeof(BasicSystemBlock));
    sysBlock.lbt = 58;              /* 늳ص�����ֵ 2.58V */
    sysBlock.obt = 55;              /* 늳غıM�ֵ 2.55V */
    sysBlock.factory = 0;           /* ʹ����ģʽ */
    sysBlock.modelNo = 1;           /* �aƷ��̖ */
    sysBlock.fwNo = 10;             /* �g�w�汾 1.0 */
    sysBlock.testCount = 0;         /* �yԇ�Δ� */
    sysBlock.codeTableVer = 1;      /* ���a��汾 */
    sysBlock.year = 24;             /* 2024�� */
    sysBlock.month = 6;             /* 6�� */
    sysBlock.date = 12;             /* 12�� */
    sysBlock.hour = 12;             /* 12�r */
    sysBlock.minute = 0;            /* 0�� */
    sysBlock.second = 0;            /* 0�� */
    sysBlock.tempLowLimit = 10;     /* �ض�����10��C */
    sysBlock.tempHighLimit = 40;    /* �ض�����40��C */
    sysBlock.measureUnit = 1;       /* �y����λ mg/dL */
    sysBlock.defaultEvent = 1;      /* �A�O�¼�AC */
    sysBlock.stripType = 0;         /* �A�O�yԇ���GLV(Ѫ��) */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_BASIC_SYSTEM] + sizeof(ParamBlockHeader_TypeDef)], 
           &sysBlock, sizeof(BasicSystemBlock));
    
    /* ��ʼ��Ӳ�wУ�ʅ��� */
    HardwareCalibBlock calibBlock;
    memset(&calibBlock, 0, sizeof(HardwareCalibBlock));
    calibBlock.evT3Trigger = 800;   /* EV_T3�|�l늉� 800mV */
    calibBlock.evWorking = 164;     /* ����늉� */
    calibBlock.evT3 = 164;          /* T3늉� */
    calibBlock.dacoOffset = 0;      /* DACƫ�� */
    calibBlock.dacdo = 0;           /* DAC�{�� */
    calibBlock.cc211Status = 0;     /* CC211��B */
    calibBlock.calTolerance = 8;    /* У���ݲ� 0.8% */
    calibBlock.ops = 1.0f;          /* У��б�� 1.0 */
    calibBlock.opi = 0.0f;          /* У�ʽؾ� 0.0 */
    calibBlock.qct = 0;             /* QCT�yԇ */
    calibBlock.tempOffset = 0;      /* �ض�ƫ�� */
    calibBlock.batteryOffset = 0;   /* 늳�ƫ�� */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_HARDWARE_CALIB] + sizeof(ParamBlockHeader_TypeDef)], 
           &calibBlock, sizeof(HardwareCalibBlock));
    
    /* ��ʼ��Ѫ�ǅ��� */
    TestParamBaseBlock bgBlock;
    memset(&bgBlock, 0, sizeof(TestParamBaseBlock));
    bgBlock.csuTolerance = 10;      /* ԇƬ�z���ݲ� */
    bgBlock.ndl = 100;              /* ��ԇƬ늉�ˮƽ */
    bgBlock.udl = 50;               /* ��ʹ��ԇƬ늉�ˮƽ */
    bgBlock.bloodIn = 150;          /* ѪҺ늉�ˮƽ */
    bgBlock.lowLimit = 20;          /* �y������ 20 mg/dL */
    bgBlock.highLimit = 600;        /* �y������ 600 mg/dL */
    bgBlock.t3E37 = 2000;           /* T3 ADV�e�`37�ֵ */
    bgBlock.tpl1 = 10;              /* �r�g�}�n���� 1��(��һ�M) */
    bgBlock.trd1 = 10;              /* ԭʼ�����񼯕r�g 1��(��һ�M) */
    bgBlock.evWidth1 = 5;           /* ����r�g 0.5��(��һ�M) */
    bgBlock.tpl2 = 20;              /* �r�g�}�n���� 2��(�ڶ��M) */
    bgBlock.trd2 = 20;              /* ԭʼ�����񼯕r�g 2��(�ڶ��M) */
    bgBlock.evWidth2 = 10;          /* ����r�g 1��(�ڶ��M) */
    bgBlock.sq = 0;                 /* QC����Sq */
    bgBlock.iq = 0;                 /* QC����Iq */
    bgBlock.cvq = 0;                /* QCˮƽCV */
    bgBlock.aq = 0;                 /* QC����A */
    bgBlock.bq = 0;                 /* QC����B */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_BG_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &bgBlock, sizeof(TestParamBaseBlock));
    
    /* ��ʼ�����ᅢ�� */
    TestParamBaseBlock uaBlock;
    memset(&uaBlock, 0, sizeof(TestParamBaseBlock));
    uaBlock.csuTolerance = 10;      /* ԇƬ�z���ݲ� */
    uaBlock.ndl = 100;              /* ��ԇƬ늉�ˮƽ */
    uaBlock.udl = 50;               /* ��ʹ��ԇƬ늉�ˮƽ */
    uaBlock.bloodIn = 150;          /* ѪҺ늉�ˮƽ */
    uaBlock.lowLimit = 3;           /* �y������ 3 mg/dL */
    uaBlock.highLimit = 200;        /* �y������ 20 mg/dL */
    uaBlock.t3E37 = 2000;           /* T3 ADV�e�`37�ֵ */
    uaBlock.tpl1 = 60;              /* �r�g�}�n���� 6��(��һ�M) */
    uaBlock.trd1 = 50;              /* ԭʼ�����񼯕r�g 5��(��һ�M) */
    uaBlock.evWidth1 = 40;          /* ����r�g 4��(��һ�M) */
    uaBlock.tpl2 = 70;              /* �r�g�}�n���� 7��(�ڶ��M) */
    uaBlock.trd2 = 60;              /* ԭʼ�����񼯕r�g 6��(�ڶ��M) */
    uaBlock.evWidth2 = 50;          /* ����r�g 5��(�ڶ��M) */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_UA_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &uaBlock, sizeof(TestParamBaseBlock));
    
    /* ��ʼ����đ�̴����� */
    TestParamBaseBlock cholBlock;
    memset(&cholBlock, 0, sizeof(TestParamBaseBlock));
    cholBlock.csuTolerance = 10;    /* ԇƬ�z���ݲ� */
    cholBlock.ndl = 100;            /* ��ԇƬ늉�ˮƽ */
    cholBlock.udl = 50;             /* ��ʹ��ԇƬ늉�ˮƽ */
    cholBlock.bloodIn = 150;        /* ѪҺ늉�ˮƽ */
    cholBlock.lowLimit = 100;       /* �y������ 100 mg/dL */
    cholBlock.highLimit = 400;      /* �y������ 400 mg/dL */
    cholBlock.t3E37 = 2000;         /* T3 ADV�e�`37�ֵ */
    cholBlock.tpl1 = 150;           /* �r�g�}�n���� 15��(��һ�M) */
    cholBlock.trd1 = 140;           /* ԭʼ�����񼯕r�g 14��(��һ�M) */
    cholBlock.evWidth1 = 130;       /* ����r�g 13��(��һ�M) */
    cholBlock.tpl2 = 160;           /* �r�g�}�n���� 16��(�ڶ��M) */
    cholBlock.trd2 = 150;           /* ԭʼ�����񼯕r�g 15��(�ڶ��M) */
    cholBlock.evWidth2 = 140;       /* ����r�g 14��(�ڶ��M) */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_CHOL_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &cholBlock, sizeof(TestParamBaseBlock));
    
    /* ��ʼ���������֬���� */
    TestParamBaseBlock tgBlock;
    memset(&tgBlock, 0, sizeof(TestParamBaseBlock));
    tgBlock.csuTolerance = 10;      /* ԇƬ�z���ݲ� */
    tgBlock.ndl = 100;              /* ��ԇƬ늉�ˮƽ */
    tgBlock.udl = 50;               /* ��ʹ��ԇƬ늉�ˮƽ */
    tgBlock.bloodIn = 150;          /* ѪҺ늉�ˮƽ */
    tgBlock.lowLimit = 50;          /* �y������ 50 mg/dL */
    tgBlock.highLimit = 500;        /* �y������ 500 mg/dL */
    tgBlock.t3E37 = 2000;           /* T3 ADV�e�`37�ֵ */
    tgBlock.tpl1 = 150;             /* �r�g�}�n���� 15��(��һ�M) */
    tgBlock.trd1 = 140;             /* ԭʼ�����񼯕r�g 14��(��һ�M) */
    tgBlock.evWidth1 = 130;         /* ����r�g 13��(��һ�M) */
    tgBlock.tpl2 = 160;             /* �r�g�}�n���� 16��(�ڶ��M) */
    tgBlock.trd2 = 150;             /* ԭʼ�����񼯕r�g 15��(�ڶ��M) */
    tgBlock.evWidth2 = 140;         /* ����r�g 14��(�ڶ��M) */
    
    /* �}�u���R�r���n�^ */
    memcpy(&tempBuf[g_blockOffsets[BLOCK_TG_PARAMS] + sizeof(ParamBlockHeader_TypeDef)], 
           &tgBlock, sizeof(TestParamBaseBlock));
    
    /* Ӌ��У�� */
    uint8_t checksum = PARAM_CalculateChecksum(tempBuf, g_blockOffsets[BLOCK_COUNT] - 1);
    tempBuf[g_blockOffsets[BLOCK_COUNT] - 1] = checksum;
    
    /* ���뵽�����^�KA */
    if (PARAM_WriteToFlash(PARAM_BLOCK_A_ADDR, tempBuf, g_blockOffsets[BLOCK_COUNT]) != 0) {
        return 2;
    }
    
    /* �O�î�ǰ��Ӆ^�K��A */
    g_activeBlockAddr = PARAM_BLOCK_A_ADDR;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_ResetToDefault
 *
 * @brief   ���Å�����Ĭ�Jֵ
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_ResetToDefault(void)
{
    /* �d���A�O���� */
    return PARAM_LoadDefaultParams();
}

/*********************************************************************
 * @fn      PARAM_GetStripParametersByStripType
 *
 * @brief   ����ԇƬ��ͫ@ȡԇƬ늉�����
 *
 * @param   stripType - ԇƬ���
 * @param   ndl - ��ԇƬ늉�ˮƽָ��
 * @param   udl - ��ʹ��ԇƬ늉�ˮƽָ��
 * @param   bloodIn - ѪҺ늉�ˮƽָ��
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    /* �z�酢����Ч�� */
    if (!ndl || !udl || !bloodIn) {
        return 1;
    }
    
    /* ����ԇƬ����x�񌦑��ą����^ */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(Ѫ��) */
        case 4: /* GAV(Ѫ��) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(����) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(��đ�̴�) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(�������֬) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* �xȡԇƬ���� */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* ��䅢�� */
    *ndl = paramBlock.ndl;
    *udl = paramBlock.udl;
    *bloodIn = paramBlock.bloodIn;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_GetMeasurementRangeByStripType
 *
 * @brief   ����ԇƬ��ͫ@ȡ�y������
 *
 * @param   stripType - ԇƬ���
 * @param   lowLimit - �y������ָ��
 * @param   highLimit - �y������ָ��
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit)
{
    /* �z�酢����Ч�� */
    if (!lowLimit || !highLimit) {
        return 1;
    }
    
    /* ����ԇƬ����x�񌦑��ą����^ */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(Ѫ��) */
        case 4: /* GAV(Ѫ��) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(����) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(��đ�̴�) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(�������֬) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* �xȡ�����^�K */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* ��䅢�� */
    *lowLimit = paramBlock.lowLimit;
    *highLimit = paramBlock.highLimit;
    
    return 0;
}

/*********************************************************************
 * @fn      PARAM_GetTimingParametersByStripType
 *
 * @brief   ����ԇƬ��ͫ@ȡ�r�򅢔�
 *
 * @param   stripType - ԇƬ���
 * @param   tpl - �r�g�}�n����ָ��
 * @param   trd - ԭʼ�����񼯕r�gָ��
 * @param   evWidth - ����r�gָ��
 * @param   seqNum - �r��M̖(1��2)
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
uint8_t PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t seqNum)
{
    /* �z�酢����Ч�� */
    if (!tpl || !trd || !evWidth || (seqNum != 1 && seqNum != 2)) {
        return 1;
    }
    
    /* ����ԇƬ����x�񌦑��ą����^ */
    ParamBlockType blockType;
    switch(stripType) {
        case 0: /* GLV(Ѫ��) */
        case 4: /* GAV(Ѫ��) */
            blockType = BLOCK_BG_PARAMS;
            break;
        case 1: /* U(����) */
            blockType = BLOCK_UA_PARAMS;
            break;
        case 2: /* C(��đ�̴�) */
            blockType = BLOCK_CHOL_PARAMS;
            break;
        case 3: /* TG(�������֬) */
            blockType = BLOCK_TG_PARAMS;
            break;
        default:
            return 2;
    }
    
    /* �xȡ�����^�K */
    TestParamBaseBlock paramBlock;
    if (PARAM_ReadBlock(blockType, &paramBlock, sizeof(TestParamBaseBlock)) != 0) {
        return 3;
    }
    
    /* �������нM̖��䅢�� */
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
 * @brief   ���殔ǰ��Ӆ����^
 *
 * @return  ��B (0: �ɹ�, ��0: �e�`�a)
 */
static uint8_t __attribute__((unused)) PARAM_SaveActiveBlock(void)
{
    /* ���F���殔ǰ��Ӆ����^��߉݋ */
    return 0; // ���O����ɹ�
} 