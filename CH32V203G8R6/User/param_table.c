/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table.c
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x�����팍�F
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#include "param_table.h"
#include "ch32v20x_flash.h"
#include <string.h>

/* ȫ�օ����� */
static ParameterTable g_paramTable;
ParameterTable *g_pParamTable = &g_paramTable;

/* �������ʼ����B */
static uint8_t g_paramInitialized = 0;

/* �Ȳ������� */
static ParamError_TypeDef PARAM_VerifyChecksum(void);
static ParamError_TypeDef PARAM_VerifyCRC16(void);
static void PARAM_UpdateChecksum(void);
static void PARAM_UpdateCRC16(void);
static ParamError_TypeDef PARAM_EraseArea(uint32_t startAddress);

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   ��ʼ��������
 *
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_Init(void)
{
    ParamError_TypeDef result;
    
    /* ���ȇLԇ���������^�xȡ */
    result = PARAM_LoadFromFlash(PARAM_AREA_MAIN);
    
    /* ����������^�xȡʧ�����Lԇ�Ă�݅����^�xȡ */
    if (result != PARAM_OK) {
        result = PARAM_LoadFromFlash(PARAM_AREA_BACKUP);
        
        /* �����݅^Ҳ�xȡʧ�����d���A�O���� */
        if (result != PARAM_OK) {
            result = PARAM_LoadDefaults();
            
            /* ���A�O��������ɂ������^ */
            if (result == PARAM_OK) {
                PARAM_SaveToFlash(PARAM_AREA_MAIN);
                PARAM_SaveToFlash(PARAM_AREA_BACKUP);
            }
        } else {
            /* ��݅^�xȡ�ɹ����֏͵��������^ */
            PARAM_SaveToFlash(PARAM_AREA_MAIN);
        }
    } else {
        /* �������^�xȡ�ɹ�������݅^ */
        PARAM_SaveToFlash(PARAM_AREA_BACKUP);
    }
    
    /* ��ӛ��ʼ����� */
    g_paramInitialized = (result == PARAM_OK) ? 1 : 0;
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_LoadFromFlash
 *
 * @brief   ��ָ�������^���d�녢����
 *
 * @param   area - �����^����R (���^���݅^)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_LoadFromFlash(ParamArea_TypeDef area)
{
    uint32_t areaAddress;
    
    /* �����^�򅢔��x���ַ */
    if (area == PARAM_AREA_MAIN) {
        areaAddress = PARAM_AREA_A_ADDR;
    } else if (area == PARAM_AREA_BACKUP) {
        areaAddress = PARAM_AREA_B_ADDR;
    } else {
        return PARAM_ERR_INVALID_AREA;
    }
    
    /* ��Flash�xȡ���������� */
    memcpy(&g_paramTable, (void*)areaAddress, sizeof(ParameterTable));
    
    /* �z��ħ�g�����Ƿ����_ */
    if (g_paramTable.header.magic != PARAM_MAGIC) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* �z��У�� */
    if (PARAM_VerifyChecksum() != PARAM_OK) {
        return PARAM_ERR_CRC;
    }
    
    /* �z��CRC16 */
    if (PARAM_VerifyCRC16() != PARAM_OK) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_SaveToFlash
 *
 * @brief   ���������浽ָ���^��
 *
 * @param   area - �����^����R (���^���݅^)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SaveToFlash(ParamArea_TypeDef area)
{
    uint32_t areaAddress;
    FLASH_Status status;
    uint32_t i;
    uint16_t *pData;
    
    /* �����^�򅢔��x���ַ */
    if (area == PARAM_AREA_MAIN) {
        areaAddress = PARAM_AREA_A_ADDR;
    } else if (area == PARAM_AREA_BACKUP) {
        areaAddress = PARAM_AREA_B_ADDR;
    } else {
        return PARAM_ERR_INVALID_AREA;
    }
    
    /* ����ǰ���������^����Ϣ */
    g_paramTable.header.writeCounter++;
    g_paramTable.header.timestamp = ((uint32_t)g_paramTable.basic.year << 24) | 
                                    ((uint32_t)g_paramTable.basic.month << 16) | 
                                    ((uint32_t)g_paramTable.basic.date << 8) | 
                                    g_paramTable.basic.hour;
    
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    /* ����CRC16 */
    PARAM_UpdateCRC16();
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ����Ŀ�˅^�� */
    if (PARAM_EraseArea(areaAddress) != PARAM_OK) {
        FLASH_Lock();
        return PARAM_ERR_FLASH_WRITE;
    }
    
    /* ������(2�ֹ�)���녢���픵�� */
    pData = (uint16_t*)&g_paramTable;
    for (i = 0; i < sizeof(ParameterTable) / 2; i++) {
        status = FLASH_ProgramHalfWord(areaAddress + i * 2, pData[i]);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    /* �����С���攵��̎������һ���ֹ� */
    if (sizeof(ParameterTable) % 2 != 0) {
        uint16_t lastWord = 0xFF00 | ((uint8_t*)&g_paramTable)[sizeof(ParameterTable) - 1];
        status = FLASH_ProgramHalfWord(areaAddress + i * 2, lastWord);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    /* �i��Flash */
    FLASH_Lock();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_LoadDefaults
 *
 * @brief   �d�녢�����A�Oֵ
 *
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_LoadDefaults(void)
{
    /* �������������Y�� */
    memset(&g_paramTable, 0, sizeof(ParameterTable));
    
    /* �O�Å������^�� */
    g_paramTable.header.magic = PARAM_MAGIC;
    g_paramTable.header.version = PARAM_VERSION;
    g_paramTable.header.writeCounter = 0;
    g_paramTable.header.timestamp = 0;
    
    /* �O�û���ϵ�y�����A�Oֵ */
    g_paramTable.basic.lbt = 58;              // 2.58V ��늳��ֵ
    g_paramTable.basic.obt = 55;              // 2.55V 늳غıM�ֵ
    g_paramTable.basic.factory = 0;           // ʹ����ģʽ
    g_paramTable.basic.modelNo = 1;           // �aƷ��̖
    g_paramTable.basic.fwNo = 1;              // �g�w�汾̖
    g_paramTable.basic.testCount = 0;         // �yԇ�Δ�
    g_paramTable.basic.codeTableVer = 1;      // ���a��汾
    
    /* �r�g�O���A�Oֵ */
    g_paramTable.basic.year = 25;             // 2025��
    g_paramTable.basic.month = 5;             // 5��
    g_paramTable.basic.date = 19;             // 19��
    g_paramTable.basic.hour = 12;             // 12�r
    g_paramTable.basic.minute = 0;            // 0��
    g_paramTable.basic.second = 0;            // 0��
    
    /* �yԇ�h���A�Oֵ */
    g_paramTable.basic.tempLowLimit = 10;     // 10��C
    g_paramTable.basic.tempHighLimit = 40;    // 40��C
    g_paramTable.basic.measureUnit = 0;       // mmol/L
    g_paramTable.basic.defaultEvent = 1;      // AC (��ǰ)
    g_paramTable.basic.stripType = 0;         // GLVѪ��
    
    /* Ӳ�wУ�ʅ����A�Oֵ */
    g_paramTable.hardware.evT3Trigger = 80;   // �|�l늉�
    g_paramTable.hardware.evWorking = 164;    // ����늉�
    g_paramTable.hardware.evT3 = 164;         // ѪҺ�z�y늉�
    g_paramTable.hardware.calTolerance = 8;   // У���ݲ�
    g_paramTable.hardware.ops = 1.0f;         // OPAУ��б��
    g_paramTable.hardware.opi = 0.0f;         // OPAУ�ʽؾ�
    g_paramTable.hardware.qct = 0;            // QCTУ�ʜyԇ
    g_paramTable.hardware.tempOffset = 0;     // �ض�У��ƫ��
    g_paramTable.hardware.batteryOffset = 0;  // 늳�У��ƫ��
    
    /* ���N�yԇ�Ŀ���A�Oֵ�����������H���������Ԕ���O���� */
    /* Ѫ��(GLV)�����A�Oֵ */
    g_paramTable.glv.glvCsuTolerance = 10;    // ԇƬ�z���ݲ�
    g_paramTable.glv.glvL = 20;               // ���� 2.0 mmol/L
    g_paramTable.glv.glvH = 74;               // ���� 33.0 mmol/L (���ƞ�uint8_t���ֵ)
    g_paramTable.glv.glvCountDownTime = 5;    // 5�뵹��
    
    /* ����(U)�����A�Oֵ */
    g_paramTable.u.uCsuTolerance = 10;        // ԇƬ�z���ݲ�
    g_paramTable.u.uL = 90;                   // ����
    g_paramTable.u.uH = 200;                  // ���� (���ƞ�uint8_t���ֵ)
    g_paramTable.u.uCountDownTime = 20;       // 20�뵹��
    
    /* ��đ�̴�(C)�����A�Oֵ */
    g_paramTable.c.cCsuTolerance = 10;        // ԇƬ�z���ݲ�
    g_paramTable.c.cL = 100;                  // ����
    g_paramTable.c.cH = 200;                  // ���� (���ƞ�uint8_t���ֵ)
    g_paramTable.c.cCountDownTime = 150;      // 150�뵹��
    
    /* �������֬(TG)�����A�Oֵ */
    g_paramTable.tg.tgCsuTolerance = 10;      // ԇƬ�z���ݲ�
    g_paramTable.tg.tgL = 70;                 // ����
    g_paramTable.tg.tgH = 600;                // ����
    g_paramTable.tg.tgCountDownTime = 180;    // 180�뵹��
    
    /* Ѫ��(GAV)�����A�Oֵ */
    g_paramTable.gav.gavCsuTolerance = 10;    // ԇƬ�z���ݲ�
    g_paramTable.gav.gavL = 20;               // ���� 2.0 mmol/L
    g_paramTable.gav.gavH = 330;              // ���� 33.0 mmol/L
    g_paramTable.gav.gavT3E37 = 200;          // T3 ADV�e�`37�ֵ
    g_paramTable.gav.gavCountDownTime = 5;    // 5�뵹��
    
    /* ����У�ͺ�CRC */
    PARAM_UpdateChecksum();
    PARAM_UpdateCRC16();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_GetValue
 *
 * @brief   �@ȡָ����ַ�ą���ֵ
 *
 * @param   paramAddress - ������ַ (����춅������a���еĵ�ַ)
 * @param   pValue - ����ֵ��ָ��
 * @param   size - ������С (�ֹ�)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_GetValue(uint16_t paramAddress, void *pValue, uint8_t size)
{
    uint8_t *pParamData;
    
    /* �z�酢�����Ƿ��ѳ�ʼ�� */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* �z�酢����ַ�Ƿ���Ч */
    if (paramAddress >= 848) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* ָ�򅢔���ַ��g_paramTable�е�λ�� */
    pParamData = (uint8_t*)&g_paramTable;
    pParamData += sizeof(ParameterHeader); // ���^�^��
    pParamData += paramAddress;
    
    /* �}�u����ֵ */
    memcpy(pValue, pParamData, size);
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_SetValue
 *
 * @brief   �O��ָ����ַ�ą���ֵ
 *
 * @param   paramAddress - ������ַ (����춅������a���еĵ�ַ)
 * @param   pValue - ����ֵ��ָ��
 * @param   size - ������С (�ֹ�)
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_SetValue(uint16_t paramAddress, void *pValue, uint8_t size)
{
    uint8_t *pParamData;
    
    /* �z�酢�����Ƿ��ѳ�ʼ�� */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* �z�酢����ַ�Ƿ���Ч */
    if (paramAddress >= 848) {
        return PARAM_ERR_INVALID_PARAM;
    }
    
    /* ָ�򅢔���ַ��g_paramTable�е�λ�� */
    pParamData = (uint8_t*)&g_paramTable;
    pParamData += sizeof(ParameterHeader); // ���^�^��
    pParamData += paramAddress;
    
    /* �}�u����ֵ */
    memcpy(pParamData, pValue, size);
    
    /* ����У�ͺ�CRC */
    PARAM_UpdateChecksum();
    PARAM_UpdateCRC16();
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_Backup
 *
 * @brief   ��݅�������݅^��
 *
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_Backup(void)
{
    /* �z�酢�����Ƿ��ѳ�ʼ�� */
    if (!g_paramInitialized) {
        return PARAM_ERR_NOT_INITIALIZED;
    }
    
    /* ����ǰ�������浽��݅^ */
    return PARAM_SaveToFlash(PARAM_AREA_BACKUP);
}

/*********************************************************************
 * @fn      PARAM_Restore
 *
 * @brief   �Ă�݅^��֏ͅ�����
 *
 * @return  ParamError_TypeDef - �����Y��
 */
ParamError_TypeDef PARAM_Restore(void)
{
    ParamError_TypeDef result;
    
    /* �Ă�݅^�d�녢�� */
    result = PARAM_LoadFromFlash(PARAM_AREA_BACKUP);
    if (result != PARAM_OK) {
        return result;
    }
    
    /* ���֏͵ą������浽���^�� */
    return PARAM_SaveToFlash(PARAM_AREA_MAIN);
}

/*********************************************************************
 * @fn      PARAM_IsInitialized
 *
 * @brief   �z�酢�����Ƿ��ѳ�ʼ��
 *
 * @return  uint8_t - 0: δ��ʼ��, 1: �ѳ�ʼ��
 */
uint8_t PARAM_IsInitialized(void)
{
    return g_paramInitialized;
}

/*********************************************************************
 * @fn      PARAM_CalculateChecksum
 *
 * @brief   Ӌ�ㅢ����У��
 *
 * @return  uint16_t - Ӌ�����У��
 */
uint16_t PARAM_CalculateChecksum(void)
{
    uint32_t sum = 0;
    uint16_t i;
    uint8_t *pData = (uint8_t*)&g_paramTable;
    
    /* Ӌ���ַ0~844�Ŀ��� (������У�ͺ�CRC) */
    for (i = 0; i < sizeof(ParameterTable) - 3; i++) {
        sum += pData[i];
    }
    
    return (uint16_t)(sum & 0xFFFF);
}

/*********************************************************************
 * @fn      PARAM_VerifyChecksum
 *
 * @brief   ��C������У��
 *
 * @return  ParamError_TypeDef - ��C�Y��
 */
static ParamError_TypeDef PARAM_VerifyChecksum(void)
{
    uint16_t calculatedChecksum = PARAM_CalculateChecksum();
    
    if (calculatedChecksum != g_paramTable.checksum) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_UpdateChecksum
 *
 * @brief   ��������У��
 *
 * @return  none
 */
static void PARAM_UpdateChecksum(void)
{
    g_paramTable.checksum = PARAM_CalculateChecksum();
}

/*********************************************************************
 * @fn      PARAM_CalculateCRC16
 *
 * @brief   Ӌ�ㅢ����CRC16
 *
 * @return  uint8_t - Ӌ�����CRC16
 */
uint8_t PARAM_CalculateCRC16(void)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    uint8_t *pData = (uint8_t*)&g_paramTable;
    
    /* Ӌ�������������CRC16 (������CRC16����) */
    for (i = 0; i < sizeof(ParameterTable) - 1; i++) {
        crc ^= pData[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 0xA001��Modbus CRC���ʽ�ķ��Dֵ
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return (uint8_t)(crc & 0xFF);
}

/*********************************************************************
 * @fn      PARAM_VerifyCRC16
 *
 * @brief   ��C������CRC16
 *
 * @return  ParamError_TypeDef - ��C�Y��
 */
static ParamError_TypeDef PARAM_VerifyCRC16(void)
{
    uint8_t calculatedCRC = PARAM_CalculateCRC16();
    
    if (calculatedCRC != g_paramTable.crc16) {
        return PARAM_ERR_CRC;
    }
    
    return PARAM_OK;
}

/*********************************************************************
 * @fn      PARAM_UpdateCRC16
 *
 * @brief   ��������CRC16
 *
 * @return  none
 */
static void PARAM_UpdateCRC16(void)
{
    g_paramTable.crc16 = PARAM_CalculateCRC16();
}

/*********************************************************************
 * @fn      PARAM_EraseArea
 *
 * @brief   ���������^��
 *
 * @param   startAddress - ��ʼ��ַ
 * @return  ParamError_TypeDef - �����Y��
 */
static ParamError_TypeDef PARAM_EraseArea(uint32_t startAddress)
{
    uint32_t pageAddress;
    FLASH_Status status;
    uint8_t pageCount = (sizeof(ParameterTable) + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    uint8_t i;
    
    /* ��퓲��� */
    for (i = 0; i < pageCount; i++) {
        pageAddress = startAddress + i * FLASH_PAGE_SIZE;
        status = FLASH_ErasePage(pageAddress);
        if (status != FLASH_COMPLETE) {
            return PARAM_ERR_FLASH_WRITE;
        }
    }
    
    return PARAM_OK;
}
