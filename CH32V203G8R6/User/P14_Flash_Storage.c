/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : �๦�������yԇ�xFlash�������淽�����F
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "P14_Flash_Storage.h"
#include "ch32v20x_flash.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

/* ȫ��׃�����x */
static uint32_t g_activeBlockAddr = 0;      // ��ǰʹ�õą����K��ַ
static ParameterBlock g_paramBlock;         // �����K����

/*********************************************************************
 * @fn      FLASH_WriteBuffer
 *
 * @brief   ���딵����Flash
 *
 * @param   address - Ŀ�˵�ַ
 * @param   buffer - �������n�^
 * @param   size - ������С
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *buffer, uint32_t size)
{
    if (!buffer || (size % 2) != 0) {
        return false;  // ��С�����2�ı���
    }
    
    uint32_t i;
    uint32_t *alignedBuffer = (uint32_t *)buffer;
    FLASH_Status status = FLASH_COMPLETE;
    
    // ������攵�ֹ����a��һ���ֹ�
    for (i = 0; i < size / 4; i++) {
        status = FLASH_ProgramWord(address + i * 4, alignedBuffer[i]);
        if (status != FLASH_COMPLETE) {
            return false;
        }
    }
    
    // �����ʣ�N�İ��֣��Ϊ�̎��
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
 * @brief   Ӌ�㔵����У��
 *
 * @param   data - �������n�^
 * @param   size - ������С
 *
 * @return  16λУ��
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
 * @brief   Ӌ�㔵����CRC16У�
 *
 * @param   data - �������n�^
 * @param   size - ������С
 *
 * @return  16λCRCֵ
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
 * @brief   �������K��У�ͺ�CRC
 *
 * @param   block - �����Kָ�
 *
 * @return  none
 */
void UpdateChecksum(ParameterBlock *block)
{
    if (!block) {
        return;
    }
    
    // Ӌ�ㅢ��������У��
    block->header.checksum = CalcChecksum((uint8_t *)&block->params, sizeof(ParameterData) - sizeof(ReservedBlock));
    
    // Ӌ�㱣��^֮ǰ�����Д�����CRC16У�
    block->header.crc16 = CalcCRC16((uint8_t *)block, 
                                    sizeof(ParamBlockHeader) + sizeof(ParameterData) - sizeof(ReservedBlock));
    
    // ���±���^�е�У�ֵ
    block->params.reserved.checksum = block->header.checksum;
    block->params.reserved.crc16 = block->header.crc16;
}

/*********************************************************************
 * @fn      FindActiveParameterBlock
 *
 * @brief   ���Ү�ǰ��ӵą����K
 *
 * @return  ��Ӆ����K��ַ������Ҳ����t����0
 */
static uint32_t FindActiveParameterBlock(void)
{
    ParamBlockHeader headerA, headerB;
    
    // �xȡA�^��B�^���^����Ϣ
    memcpy(&headerA, (void *)PARAM_BLOCK_A_ADDR, sizeof(ParamBlockHeader));
    memcpy(&headerB, (void *)PARAM_BLOCK_B_ADDR, sizeof(ParamBlockHeader));
    
    // �z��ħ��
    if (headerA.magic != 0xA55A && headerB.magic != 0xA55A) {
        // �ɂ��^�򶼟oЧ������0
        return 0;
    }
    
    // ���ֻ��һ���^����Ч������ԓ�^��
    if (headerA.magic != 0xA55A) {
        return PARAM_BLOCK_B_ADDR;
    }
    
    if (headerB.magic != 0xA55A) {
        return PARAM_BLOCK_A_ADDR;
    }
    
    // �ɂ��^����Ч�����^����Ӌ����
    if (headerA.writeCounter > headerB.writeCounter) {
        return PARAM_BLOCK_A_ADDR;
    } else {
        return PARAM_BLOCK_B_ADDR;
    }
}

/*********************************************************************
 * @fn      LoadParameterBlock
 *
 * @brief   �d�녢���K������
 *
 * @param   blockAddr - �����K��ַ
 *
 * @return  true: �ɹ�, false: ʧ��
 */
static bool LoadParameterBlock(uint32_t blockAddr)
{
    if (!blockAddr) {
        return false;
    }
    
    // ��Flash���xȡ�����K
    memcpy(&g_paramBlock, (void *)blockAddr, sizeof(ParameterBlock));
    
    // �z��ħ��
    if (g_paramBlock.header.magic != 0xA55A) {
        return false;
    }
    
    // �z��CRC��У��
    uint16_t calcChecksum = CalcChecksum((uint8_t *)&g_paramBlock.params, 
                                          sizeof(ParameterData) - sizeof(ReservedBlock));
    
    uint16_t calcCRC = CalcCRC16((uint8_t *)&g_paramBlock, 
                                 sizeof(ParamBlockHeader) + sizeof(ParameterData) - sizeof(ReservedBlock));
    
    if (calcChecksum != g_paramBlock.header.checksum || calcCRC != g_paramBlock.header.crc16) {
        printf("�����^У�ʧ��: Checksum=%04X(Ӌ��ֵ=%04X), CRC=%04X(Ӌ��ֵ=%04X)\r\n",
                g_paramBlock.header.checksum, calcChecksum, g_paramBlock.header.crc16, calcCRC);
        return false;
    }
    
    // ���»�Ӆ^���ַ
    g_activeBlockAddr = blockAddr;
    
    return true;
}

/*********************************************************************
 * @fn      SaveParameterBlock
 *
 * @brief   ���澏���еą����K��Flash
 *
 * @return  true: �ɹ�, false: ʧ��
 */
static bool SaveParameterBlock(void)
{
    uint32_t targetAddr;
    
    // ���ӌ���Ӌ����
    g_paramBlock.header.writeCounter++;
    
    // ����У��
    UpdateChecksum(&g_paramBlock);
    
    // �x��Ŀ�˵�ַ���c��ǰ��Ӆ^���෴�ą^��
    if (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) {
        targetAddr = PARAM_BLOCK_B_ADDR;
    } else {
        targetAddr = PARAM_BLOCK_A_ADDR;
    }
    
    // ���iFlash
    FLASH_Unlock();
    
    // ����Ŀ���
    FLASH_Status status = FLASH_ErasePage(targetAddr);
    if (status != FLASH_COMPLETE) {
        FLASH_Lock();
        return false;
    }
    
    // �������������K
    bool result = FLASH_WriteBuffer(targetAddr, (uint8_t *)&g_paramBlock, sizeof(ParameterBlock));
    
    // �i��Flash
    FLASH_Lock();
    
    if (result) {
        // ���»�Ӆ^���ַ
        g_activeBlockAddr = targetAddr;
    }
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   ��ʼ�������惦ϵ�y
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_Init(void)
{
    printf("��ʼ����������ϵ�y...\r\n");
    
    // ���һ�Ӆ����K
    uint32_t activeAddr = FindActiveParameterBlock();
    
    // �Lԇ�d�녢���K
    if (activeAddr && LoadParameterBlock(activeAddr)) {
        printf("�ɹ��d�녢���K����ַ: 0x%08X\r\n", activeAddr);
        return true;
    }
    
    // �Ҳ�����Ч�����K���d��Ĭ�Jֵ
    printf("δ�ҵ���Ч�����K���d��Ĭ�Jֵ\r\n");
    return PARAM_LoadDefault();
}

/*********************************************************************
 * @fn      PARAM_LoadDefault
 *
 * @brief   �d��Ĭ�J����
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_LoadDefault(void)
{
    // ��������K
    memset(&g_paramBlock, 0, sizeof(ParameterBlock));
    
    // �O�Ø��^��Ϣ
    g_paramBlock.header.magic = 0xA55A;
    g_paramBlock.header.version = 1;
    g_paramBlock.header.writeCounter = 1;
    g_paramBlock.header.timestamp = 0;
    
    // �O�û���ϵ�y����
    g_paramBlock.params.basicSystem.lbt = 58;  // 2.58V
    g_paramBlock.params.basicSystem.obt = 55;  // 2.55V
    g_paramBlock.params.basicSystem.factory = 0; // ʹ����ģʽ
    g_paramBlock.params.basicSystem.modelNo = 1;
    g_paramBlock.params.basicSystem.fwNo = 10; // �汾1.0
    g_paramBlock.params.basicSystem.testCount = 0;
    g_paramBlock.params.basicSystem.codeTableVer = 1;
    
    // �O��Ĭ�J�r�g (2024-08-12 00:00:00)
    g_paramBlock.params.basicSystem.year = 24;
    g_paramBlock.params.basicSystem.month = 8;
    g_paramBlock.params.basicSystem.date = 12;
    g_paramBlock.params.basicSystem.hour = 0;
    g_paramBlock.params.basicSystem.minute = 0;
    g_paramBlock.params.basicSystem.second = 0;
    
    // �O�íh������
    g_paramBlock.params.basicSystem.tempLowLimit = 10;  // 10��
    g_paramBlock.params.basicSystem.tempHighLimit = 40; // 40��
    g_paramBlock.params.basicSystem.measureUnit = 1;    // mg/dL
    g_paramBlock.params.basicSystem.defaultEvent = 1;   // AC
    g_paramBlock.params.basicSystem.stripType = 0;      // GLV(Ѫ��)
    
    // �O��Ӳ�wУ�ʅ���
    g_paramBlock.params.hardwareCalib.evT3Trigger = 800;  // 800mV
    g_paramBlock.params.hardwareCalib.evWorking = 164;
    g_paramBlock.params.hardwareCalib.evT3 = 164;
    g_paramBlock.params.hardwareCalib.dacoOffset = 0;
    g_paramBlock.params.hardwareCalib.dacdo = 0;
    g_paramBlock.params.hardwareCalib.cc211Status = 0;
    g_paramBlock.params.hardwareCalib.calTolerance = 8;  // 0.8%
    g_paramBlock.params.hardwareCalib.ops = 1.0f;       // У��б�ʞ�1.0
    g_paramBlock.params.hardwareCalib.opi = 0.0f;       // У�ʽؾ���0.0
    g_paramBlock.params.hardwareCalib.qct = 0;
    g_paramBlock.params.hardwareCalib.tempOffset = 0;   // �ض�ƫ��У��
    g_paramBlock.params.hardwareCalib.batteryOffset = 0; // 늳�ƫ��У��
    
    // �O��Ѫ�ǅ���
    g_paramBlock.params.bgParams.bgCsuTolerance = 10;  // 3%�ݲ�
    g_paramBlock.params.bgParams.bgNdl = 100;  // ��ԇƬ�z�yˮƽ
    g_paramBlock.params.bgParams.bgUdl = 50;   // ��ʹ��ԇƬ�z�yˮƽ
    g_paramBlock.params.bgParams.bgBloodIn = 150; // ѪҺ�z�yˮƽ
    memset(g_paramBlock.params.bgParams.bgStripLot, 0, 16); // ���ԇƬ��̖
    
    g_paramBlock.params.bgParams.bgL = 2;    // ���20 mg/dL�@ʾ"LO"
    g_paramBlock.params.bgParams.bgH = 60;   // ���600 mg/dL�@ʾ"HI"
    g_paramBlock.params.bgParams.bgT3E37 = 2000;  // T3 ADV�ֵ
    
    // �O��Ѫ�ǜyԇ�r�򅢔�
    g_paramBlock.params.bgParams.bgTPL1 = 10;        // 1��
    g_paramBlock.params.bgParams.bgTRD1 = 10;        // 1��
    g_paramBlock.params.bgParams.bgEVWidth1 = 5;     // 0.5��
    g_paramBlock.params.bgParams.bgTPL2 = 20;        // 2��
    g_paramBlock.params.bgParams.bgTRD2 = 20;        // 2��
    g_paramBlock.params.bgParams.bgEVWidth2 = 10;    // 1��
    
    // �O�����ᅢ��
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
    
    // �O��đ�̴�����
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
    
    // �O���������֬����
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
    
    // ���慢���K
    if (!SaveParameterBlock()) {
        printf("����Ĭ�J����ʧ��\r\n");
        return false;
    }
    
    printf("�ɹ��d��K����Ĭ�J����\r\n");
    return true;
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   �xȡ�����^�K
 *
 * @param   blockType - �^�K���
 * @param   buffer - Ŀ�˾��n�^
 * @param   size - ������С
 *
 * @return  true: �ɹ�, false: ʧ��
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
 * @brief   �������^�K
 *
 * @param   blockType - �^�K���
 * @param   buffer - Դ�������n�^
 * @param   size - ������С
 *
 * @return  true: �ɹ�, false: ʧ��
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
    
    // ���������ą���
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   �@ȡϵ�y�r�g
 *
 * @param   year, month, date, hour, minute, second - �r�g����ָ�
 *
 * @return  true: �ɹ�, false: ʧ��
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
 * @brief   ����ϵ�y�r�g
 *
 * @param   year, month, date, hour, minute, second - �r�g����
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    // �z�酢����Ч��
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
    
    // ���������ą���
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetStripParametersByStripType
 *
 * @brief   ����ԇƬ��ͫ@ȡ�y��ˮƽ����
 *
 * @param   stripType - ԇƬ���
 * @param   ndl - ��ԇƬ�y��ˮƽָ�
 * @param   udl - ��ʹ��ԇƬ�y��ˮƽָ�
 * @param   bloodIn - ѪҺ�y��ˮƽָ�
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(Ѫ��)
        case 4:  // GAV(Ѫ��)
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
            
        case 1:  // U(����)
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
            
        case 2:  // C(��đ�̴�)
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
            
        case 3:  // TG(�������֬)
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
 * @brief   ����ԇƬ��ͫ@ȡ�y����������
 *
 * @param   stripType - ԇƬ���
 * @param   lowLimit - ����ָ�
 * @param   highLimit - ����ָ�
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(Ѫ��)
        case 4:  // GAV(Ѫ��)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.bgParams.bgL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.bgParams.bgH;
            }
            break;
            
        case 1:  // U(����)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.uParams.uL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.uParams.uH;
            }
            break;
            
        case 2:  // C(��đ�̴�)
            if (lowLimit) {
                *lowLimit = g_paramBlock.params.cParams.cL;
            }
            if (highLimit) {
                *highLimit = g_paramBlock.params.cParams.cH;
            }
            break;
            
        case 3:  // TG(�������֬)
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
 * @brief   ����ԇƬ��ͫ@ȡ�yԇ�r�򅢔�
 *
 * @param   stripType - ԇƬ���
 * @param   tpl - �r�g�}�n����ָ�
 * @param   trd - ԭʼ�����r�gָ�
 * @param   evWidth - �۟��r�gָ�
 * @param   group - �����M(1��2)
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    if (!g_activeBlockAddr || (group != 1 && group != 2)) {
        return false;
    }
    
    switch (stripType) {
        case 0:  // GLV(Ѫ��)
        case 4:  // GAV(Ѫ��)
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
            
        case 1:  // U(����)
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
            
        case 2:  // C(��đ�̴�)
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
            
        case 3:  // TG(�������֬)
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
 * @brief   ���ӜyԇӋ��
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_IncreaseTestCount(void)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    g_paramBlock.params.basicSystem.testCount++;
    
    // ���������ą���
    return SaveParameterBlock();
}

/*********************************************************************
 * @fn      PARAM_GetTestCount
 *
 * @brief   �@ȡ�yԇӋ��
 *
 * @return  �yԇӋ��
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
 * @brief   ����yԇӛ�
 *
 * @param   resultValue - �yԇ�Y��ֵ
 * @param   resultStatus - �yԇ�Y����B
 * @param   event - �¼����
 * @param   code - ԇƬУ���a
 * @param   batteryVoltage - 늳�늉�
 * @param   temperature - �h���ض�
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_SaveTestRecord(uint16_t resultValue, uint16_t resultStatus, uint8_t event, uint8_t code, uint16_t batteryVoltage, uint16_t temperature)
{
    if (!g_activeBlockAddr) {
        return false;
    }
    
    // ���yԇӛ�
    TestRecord record;
    record.flag = 0xA55A;  // ӛ���Ч��ӛ
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
    
    // �z��ӛ䛅^
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB ӛ䛅^�ܴ惦��ӛ䛔�
    
    // ������һ�����õ�ӛ�λ��
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag != 0xA55A) { // ���ӛ䛟oЧ��δʹ��
            break;
        }
        addr += recordSize;
    }
    
    // ���ӛ䛅^�ѝM���t��������ӛ䛅^�K���^�_ʼ
    if (addr >= endAddr) {
        FLASH_Unlock();
        
        // ����ӛ䛅^�������
        for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += PARAM_PAGE_SIZE) {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        addr = recordBase; // �ص�ӛ䛅^��ʼ��ַ
    }
    
    // ������ӛ�
    FLASH_Unlock();
    bool result = FLASH_WriteBuffer(addr, (uint8_t *)&record, recordSize);
    FLASH_Lock();
    
    // ���Ӝyԇ�Δ�
    if (result) {
        PARAM_IncreaseTestCount();
    }
    
    return result;
}

/*********************************************************************
 * @fn      PARAM_GetTestRecord
 *
 * @brief   �@ȡ�yԇӛ�
 *
 * @param   index - ӛ�����(0������)
 * @param   record - ӛ䛾��n�^
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_GetTestRecord(uint16_t index, TestRecord *record)
{
    if (!record || !g_activeBlockAddr) {
        return false;
    }
    
    // �z��ӛ䛅^
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB ӛ䛅^�ܴ惦��ӛ䛔�
    
    // ������Чӛ�
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    uint16_t validCount = 0;
    uint32_t validRecords[maxRecords];
    
    // ��һ�����ӛ�������Чӛ䛵�ַ
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag == 0xA55A) { // ��Чӛ�
            validRecords[validCount++] = addr;
        }
        addr += recordSize;
    }
    
    // �z�������Ƿ���Ч
    if (index >= validCount) {
        return false;
    }
    
    // ���յ����xȡӛ�
    addr = validRecords[validCount - 1 - index];
    memcpy(record, (void *)addr, recordSize);
    
    return true;
}

/*********************************************************************
 * @fn      PARAM_GetTestRecordCount
 *
 * @brief   �@ȡ��Ч�yԇӛ䛔���
 *
 * @return  ӛ䛔���
 */
uint16_t PARAM_GetTestRecordCount(void)
{
    if (!g_activeBlockAddr) {
        return 0;
    }
    
    // �z��ӛ䛅^
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize;  // 6KB ӛ䛅^�ܴ惦��ӛ䛔�
    
    // ������Чӛ�
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag = 0;
    uint16_t validCount = 0;
    
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag == 0xA55A) { // ��Чӛ�
            validCount++;
        }
        addr += recordSize;
    }
    
    return validCount;
}

/*********************************************************************
 * @fn      PARAM_EraseAllTestRecords
 *
 * @brief   �������Мyԇӛ�
 *
 * @return  true: �ɹ�, false: ʧ��
 */
bool PARAM_EraseAllTestRecords(void)
{
    // �z��ӛ䛅^
    uint32_t recordBase = RECORD_ZONE_ADDR;
    uint32_t endAddr = recordBase + 6 * 1024;  // 6KB ӛ䛅^
    
    FLASH_Unlock();
    
    // ����ӛ䛅^�������
    for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += PARAM_PAGE_SIZE) {
        if (FLASH_ErasePage(pageAddr) != FLASH_COMPLETE) {
            FLASH_Lock();
            return false;
        }
    }
    
    FLASH_Lock();
    return true;
} 