/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_param.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : CH32V203 Flash �������淽�����F
 *********************************************************************************
 * Copyright (c) 2025 HMD Biomedical Inc.
 *******************************************************************************/

#include "flash_param.h"
#include "param_code_table.h"
#include <string.h>
#include "debug.h"

/* ���x����ֵ */
#define FLASH_PARAM_MAGIC      0xA55A  /* �����^�K���^ħ�� */
#define FLASH_PARAM_VERSION    0x01    /* �����^�K�汾̖ */

/* ��ǰ��Ӆ����^ָʾ����Ĭ�Jʹ�Å^�KA */
static uint32_t activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;

/* �o�B������ */
static FLASH_ParamResult_TypeDef PARAM_CheckBlockValidity(uint32_t blockAddr, uint8_t blockType);
static FLASH_ParamResult_TypeDef PARAM_SelectActiveBlock(void);
static uint16_t PARAM_CalculateChecksum(uint8_t *data, uint16_t size);

/**
 * @brief Ӌ�ㅢ���^�KУ��
 * @param data: �Y��ָ�
 * @param size: �Y�ϴ�С
 * @return У��
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
 * @brief �z�酢���^�K��Ч��
 * @param blockAddr: �����^�K��ַ
 * @param blockType: �^�K���
 * @return ���������Y��
 */
static FLASH_ParamResult_TypeDef PARAM_CheckBlockValidity(uint32_t blockAddr, uint8_t blockType)
{
    FLASH_ParamBlockHeader_TypeDef header;
    
    /* ��Flash�xȡ�^�K���^ */
    FLASH_ReadBuffer(blockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* �z��ħ�� */
    if (header.magic != FLASH_PARAM_MAGIC)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* �z��^�K��� */
    if (blockType != 0xFF && header.blockType != blockType)
    {
        return PARAM_ERROR_INVALID;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief �x���Ӆ����^�K
 * @return ���������Y��
 */
static FLASH_ParamResult_TypeDef PARAM_SelectActiveBlock(void)
{
    FLASH_ParamBlockHeader_TypeDef headerA, headerB;
    FLASH_ParamResult_TypeDef validA, validB;
    
    /* �z�酢���^�KA����Ч�� */
    validA = PARAM_CheckBlockValidity(FLASH_PARAM_BLOCK_A_ADDR, BLOCK_BASIC_SYSTEM);
    
    /* �z�酢���^�KB����Ч�� */
    validB = PARAM_CheckBlockValidity(FLASH_PARAM_BLOCK_B_ADDR, BLOCK_BASIC_SYSTEM);
    
    /* ������Ч�ԛQ��ʹ���Ă��^�K */
    if (validA == PARAM_SUCCESS && validB == PARAM_SUCCESS)
    {
        /* �ɂ��^�K����Ч���xȡ���^���^����Ӌ���� */
        FLASH_ReadBuffer(FLASH_PARAM_BLOCK_A_ADDR, (uint8_t*)&headerA, sizeof(FLASH_ParamBlockHeader_TypeDef));
        FLASH_ReadBuffer(FLASH_PARAM_BLOCK_B_ADDR, (uint8_t*)&headerB, sizeof(FLASH_ParamBlockHeader_TypeDef));
        
        /* �x����Ӌ���^��ą^�K */
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
        /* ֻ�Ѕ^�KA��Ч */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;
        return PARAM_SUCCESS;
    }
    else if (validB == PARAM_SUCCESS)
    {
        /* ֻ�Ѕ^�KB��Ч */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_B_ADDR;
        return PARAM_SUCCESS;
    }
    else
    {
        /* �ɂ��^�K���oЧ */
        return PARAM_ERROR_EMPTY;
    }
}

/**
 * @brief ��ʼ�������^
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_Init(void)
{
    FLASH_ParamResult_TypeDef result;
    FLASH_ParamBlock_TypeDef paramBlock;
    
    /* �x���Ӆ����^�K */
    result = PARAM_SelectActiveBlock();
    
    if (result == PARAM_ERROR_EMPTY)
    {
        /* ��������^�ǿյģ�����Ĭ�J�����^ */
        printf("�����^��գ���ʼ����...\r\n");
        
        /* ���Å������a���Ĭ�Jֵ */
        ParamCodeTable_Reset();
        
        /* ���������^�K���^ */
        paramBlock.header.magic = FLASH_PARAM_MAGIC;
        paramBlock.header.version = FLASH_PARAM_VERSION;
        paramBlock.header.blockType = BLOCK_BASIC_SYSTEM;
        paramBlock.header.writeCounter = 1;
        paramBlock.header.timestamp = 0;
        paramBlock.header.dataSize = sizeof(ParamCodeTable_TypeDef);
        
        /* �}�u�������� */
        memcpy(paramBlock.data, (uint8_t*)&ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
        
        /* Ӌ��У�� */
        paramBlock.header.checksum = PARAM_CalculateChecksum(paramBlock.data, paramBlock.header.dataSize);
        
        /* ���������^�KA��B */
        FLASH_ErasePage(FLASH_PARAM_BLOCK_A_ADDR);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_A_ADDR + FLASH_PAGE_SIZE);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_B_ADDR);
        FLASH_ErasePage(FLASH_PARAM_BLOCK_B_ADDR + FLASH_PAGE_SIZE);
        
        /* ���뵽�����^�KA */
        if (FLASH_WriteBuffer(FLASH_PARAM_BLOCK_A_ADDR, (uint8_t*)&paramBlock, 
                             sizeof(FLASH_ParamBlockHeader_TypeDef) + paramBlock.header.dataSize) != PARAM_SUCCESS)
        {
            return PARAM_ERROR_FLASH;
        }
        
        /* ���뵽�����^�KB������ */
        if (FLASH_WriteBuffer(FLASH_PARAM_BLOCK_B_ADDR, (uint8_t*)&paramBlock, 
                             sizeof(FLASH_ParamBlockHeader_TypeDef) + paramBlock.header.dataSize) != PARAM_SUCCESS)
        {
            return PARAM_ERROR_FLASH;
        }
        
        /* �O�û�Ӆ����^�K��ַ��A */
        activeParamBlockAddr = FLASH_PARAM_BLOCK_A_ADDR;
        
        return PARAM_SUCCESS;
    }
    else if (result == PARAM_SUCCESS)
    {
        /* �Ļ�Ӆ����^�xȡ��RAM */
        return PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, (void*)&ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
    }
    
    return result;
}

/**
 * @brief �xȡ�����^�K
 * @param blockType: �^�K���
 * @param data: �Y��ָ�
 * @param size: �Y�ϴ�С
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size)
{
    FLASH_ParamBlockHeader_TypeDef header;
    uint16_t checksum;
    
    /* �z�酢����Ч�� */
    if (data == NULL || size == 0)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* ��Flash�xȡ�^�K���^ */
    FLASH_ReadBuffer(activeParamBlockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* �z��ħ�� */
    if (header.magic != FLASH_PARAM_MAGIC)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* �z��^�K��� */
    if (header.blockType != blockType)
    {
        /* ���Ҍ�����͵ą^�K */
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
            blockAddr = (blockAddr + 3) & ~3; /* 4�ֹ����R */
        }
        
        if (blockAddr >= endAddr || header.blockType != blockType)
        {
            return PARAM_ERROR_INVALID;
        }
    }
    
    /* �z���Y�ϴ�С */
    if (header.dataSize > size)
    {
        return PARAM_ERROR_SIZE;
    }
    
    /* �xȡ�������� */
    FLASH_ReadBuffer(activeParamBlockAddr + sizeof(FLASH_ParamBlockHeader_TypeDef), (uint8_t*)data, header.dataSize);
    
    /* ��CУ�� */
    checksum = PARAM_CalculateChecksum((uint8_t*)data, header.dataSize);
    if (checksum != header.checksum)
    {
        return PARAM_ERROR_CHECKSUM;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief �������^�K
 * @param blockType: �^�K���
 * @param data: �Y��ָ�
 * @param size: �Y�ϴ�С
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_UpdateBlock(uint8_t blockType, const void *data, uint16_t size)
{
    FLASH_ParamBlockHeader_TypeDef header;
    FLASH_ParamBlock_TypeDef paramBlock;
    uint32_t destAddr, backupAddr;
    
    /* �z�酢����Ч�� */
    if (data == NULL || size == 0 || size > (FLASH_PARAM_MAX_SIZE - sizeof(FLASH_ParamBlockHeader_TypeDef)))
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* �ʂ䅢���^�K�^�� */
    paramBlock.header.magic = FLASH_PARAM_MAGIC;
    paramBlock.header.version = FLASH_PARAM_VERSION;
    paramBlock.header.blockType = blockType;
    paramBlock.header.dataSize = size;
    
    /* �xȡ��ǰ��Ӆ^�K�^�� */
    FLASH_ReadBuffer(activeParamBlockAddr, (uint8_t*)&header, sizeof(FLASH_ParamBlockHeader_TypeDef));
    
    /* ���ӌ���Ӌ���� */
    paramBlock.header.writeCounter = header.writeCounter + 1;
    
    /* �@ȡ��ǰϵ�y�r�g����r�g�� */
    paramBlock.header.timestamp = (ParamCodeTable.Time.YEAR << 26) | 
                                 (ParamCodeTable.Time.MONTH << 22) | 
                                 (ParamCodeTable.Time.DATE << 17) | 
                                 (ParamCodeTable.Time.HOUR << 12) | 
                                 (ParamCodeTable.Time.MINUTE << 6) | 
                                 ParamCodeTable.Time.SECOND;
    
    /* �}�u�������� */
    memcpy(paramBlock.data, data, size);
    
    /* Ӌ��У�� */
    paramBlock.header.checksum = PARAM_CalculateChecksum(paramBlock.data, size);
    
    /* �_��Ŀ�˵�ַ�͂�ݵ�ַ */
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
    
    /* ������݅^�K */
    FLASH_ErasePage(backupAddr);
    FLASH_ErasePage(backupAddr + FLASH_PAGE_SIZE);
    
    /* �Ȍ��뵽��݅^�K */
    if (FLASH_WriteBuffer(backupAddr, (uint8_t*)&paramBlock, 
                         sizeof(FLASH_ParamBlockHeader_TypeDef) + size) != PARAM_SUCCESS)
    {
        return PARAM_ERROR_FLASH;
    }
    
    /* ����Ŀ�˅^�K */
    FLASH_ErasePage(destAddr);
    FLASH_ErasePage(destAddr + FLASH_PAGE_SIZE);
    
    /* ���뵽Ŀ�˅^�K */
    if (FLASH_WriteBuffer(destAddr, (uint8_t*)&paramBlock, 
                         sizeof(FLASH_ParamBlockHeader_TypeDef) + size) != PARAM_SUCCESS)
    {
        /* �������Ŀ�˅^�Kʧ�����ГQ����݅^�K */
        activeParamBlockAddr = backupAddr;
        return PARAM_ERROR_FLASH;
    }
    
    return PARAM_SUCCESS;
}

/**
 * @brief ���ӜyԇӋ��
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_IncreaseTestCount(void)
{
    /* ���ӜyԇӋ�� */
    ParamCodeTable.System.NOT++;
    
    /* ��������Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief �������ڕr�g
 * @param year: �� (0-99)
 * @param month: �� (1-12)
 * @param date: �� (1-31)
 * @param hour: �r (0-23)
 * @param minute: �� (0-59)
 * @param second: �� (0-59)
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, 
                                              uint8_t hour, uint8_t minute, uint8_t second)
{
    /* �z�酢����Ч�� */
    if (month < 1 || month > 12 || date < 1 || date > 31 || 
        hour > 23 || minute > 59 || second > 59)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* ���r�g�O�� */
    ParamCodeTable.Time.YEAR = year;
    ParamCodeTable.Time.MONTH = month;
    ParamCodeTable.Time.DATE = date;
    ParamCodeTable.Time.HOUR = hour;
    ParamCodeTable.Time.MINUTE = minute;
    ParamCodeTable.Time.SECOND = second;
    
    /* ��������Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief �@ȡ���ڕr�g
 * @param year: ��ָ�
 * @param month: ��ָ�
 * @param date: ��ָ�
 * @param hour: �rָ�
 * @param minute: ��ָ�
 * @param second: ��ָ�
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                                           uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* �z��ָ���Ч�� */
    if (year == NULL || month == NULL || date == NULL || 
        hour == NULL || minute == NULL || second == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* �@ȡ�r�g�O�� */
    *year = ParamCodeTable.Time.YEAR;
    *month = ParamCodeTable.Time.MONTH;
    *date = ParamCodeTable.Time.DATE;
    *hour = ParamCodeTable.Time.HOUR;
    *minute = ParamCodeTable.Time.MINUTE;
    *second = ParamCodeTable.Time.SECOND;
    
    return PARAM_SUCCESS;
}

/**
 * @brief �@ȡԇƬ����
 * @param stripType: ԇƬ���
 * @param ndl: ��ԇƬ���ˮƽ
 * @param udl: ��ʹ��ԇƬ���ˮƽ
 * @param bloodIn: ѪҺ���ˮƽ
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, 
                                                             uint16_t *udl, uint16_t *bloodIn)
{
    /* �z�酢����Ч�� */
    if (ndl == NULL || udl == NULL || bloodIn == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* ����ԇƬ��ͫ@ȡ���� */
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
 * @brief �@ȡ�y������
 * @param stripType: ԇƬ���
 * @param lowLimit: �y������ָ�
 * @param highLimit: �y������ָ�
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, 
                                                              uint8_t *highLimit)
{
    /* �z�酢����Ч�� */
    if (lowLimit == NULL || highLimit == NULL)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* ����ԇƬ��ͫ@ȡ���� */
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
 * @brief �@ȡ�r�򅢔�
 * @param stripType: ԇƬ���
 * @param tpl: �r�g�}�n��ָ�
 * @param trd: ԭʼ�����r�gָ�
 * @param evWidth: ���ԕr�gָ�
 * @param groupIndex: �����M���� (0:��һ�M, 1:�ڶ��M)
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, 
                                                              uint16_t *trd, uint16_t *evWidth, 
                                                              uint8_t groupIndex)
{
    /* �z�酢����Ч�� */
    if (tpl == NULL || trd == NULL || evWidth == NULL || groupIndex > 1)
    {
        return PARAM_ERROR_INVALID;
    }
    
    /* ����ԇƬ��ͺͅ����M�����@ȡ���� */
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
 * @brief �����������Þ�Ĭ�Jֵ
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef PARAM_ResetToDefault(void)
{
    /* ���Å������a���Ĭ�Jֵ */
    ParamCodeTable_Reset();
    
    /* ��������Flash */
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &ParamCodeTable, sizeof(ParamCodeTable_TypeDef));
}

/**
 * @brief ���딵����Flash
 * @param address: Flash��ַ
 * @param data: ����ָ�
 * @param size: ������С
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint16_t size)
{
    uint32_t i;
    uint32_t word;
    FLASH_Status status;
    
    /* �z���ַ�ʹ�С�Ƿ���Ч */
    if (address < FLASH_PARAM_BLOCK_A_ADDR || 
        address + size > FLASH_RECORD_BLOCK_ADDR + FLASH_PAGE_SIZE*6)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ���֣�4�ֹ������딵�� */
    for (i = 0; i < size; i += 4)
    {
        /* ���R̎�� */
        if (i + 4 <= size)
        {
            /* ֱ���xȡ4�ֹ� */
            word = *(uint32_t*)(data + i);
        }
        else
        {
            /* ̎��β������4�ֹ�����r */
            word = 0xFFFFFFFF;
            memcpy(&word, data + i, size - i);
        }
        
        /* ����һ���֣�4�ֹ��� */
        status = FLASH_ProgramWord(address + i, word);
        
        if (status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return PARAM_ERROR_FLASH;
        }
    }
    
    /* �i��Flash */
    FLASH_Lock();
    
    return PARAM_SUCCESS;
}

/**
 * @brief ��Flash�xȡ����
 * @param address: Flash��ַ
 * @param data: �������n�^ָ�
 * @param size: ������С
 * @return ���������Y��
 */
FLASH_ParamResult_TypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint16_t size)
{
    /* �z���ַ�ʹ�С�Ƿ���Ч */
    if (address < FLASH_PARAM_BLOCK_A_ADDR || 
        address + size > FLASH_RECORD_BLOCK_ADDR + FLASH_PAGE_SIZE*6)
    {
        return PARAM_ERROR_RANGE;
    }
    
    /* ֱ�ӏ�Flash���}�u���� */
    memcpy(data, (uint8_t*)address, size);
    
    return PARAM_SUCCESS;
}