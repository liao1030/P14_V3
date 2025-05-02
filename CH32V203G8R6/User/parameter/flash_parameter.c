/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_parameter.c
 * Author             : WCH & HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������xFlash�������淽�����F
*********************************************************************************
* Copyright (c) 2025 HMD Co., Ltd.
* All rights reserved.
*******************************************************************************/

#include "flash_parameter.h"

/* ȫ��׃�� */
static uint32_t g_activeBlockAddr = 0;     // ��ǰ��Ӆ����K��ַ
static uint16_t g_paramCRC = 0;            // ����CRCУ�ֵ

/* �Ȳ������� */
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
 * @brief  ��ʼ����������ģ�K
 * @param  �o
 * @retval �o
 */
void PARAM_Init(void)
{
    ParameterBlock block;
    
    // ���һ�Ӆ����K
    if (!FindParameterBlock(&g_activeBlockAddr))
    {
        // ����]���ҵ���Ч�ą����K���֏ͳ��S�O��
        PARAM_ResetToDefault();
    }
    
    // �xȡ�����K�KӋ��У��
    if (GetActiveParameterBlock(&block))
    {
        g_paramCRC = CalculateChecksum(&block);
    }
}

/**
 * @brief  �xȡָ����͵ą����K
 * @param  blockType: �����K���
 * @param  data: �������n�^
 * @param  size: ������С
 * @retval true: �ɹ�, false: ʧ��
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
    
    // �@ȡָ�������K��ƫ�ƺʹ�С
    GetBlockOffset(blockType, &offset, &blockSize);
    
    // �z�酢����С�Ƿ�ƥ��
    if (size != blockSize)
    {
        return false;
    }
    
    // �xȡ���������K
    if (!GetActiveParameterBlock(&block))
    {
        return false;
    }
    
    // �}�uָ���ą����K����
    uint8_t *src = (uint8_t *)&block.params + offset;
    memcpy(data, src, size);
    
    return true;
}

/**
 * @brief  ����ָ����͵ą����K
 * @param  blockType: �����K���
 * @param  data: �������n�^
 * @param  size: ������С
 * @retval true: �ɹ�, false: ʧ��
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
    
    // �@ȡָ�������K��ƫ�ƺʹ�С
    GetBlockOffset(blockType, &offset, &blockSize);
    
    // �z�酢����С�Ƿ�ƥ��
    if (size != blockSize)
    {
        return false;
    }
    
    // �xȡ��ǰ�����K
    if (!GetActiveParameterBlock(&block))
    {
        return false;
    }
    
    // �}�u�µą�������
    uint8_t *dst = (uint8_t *)&block.params + offset;
    memcpy(dst, data, size);
    
    // �������K�^
    block.header.writeCounter++;
    block.header.timestamp = (uint32_t)((block.params.basicSystem.year << 26) | 
                                        (block.params.basicSystem.month << 22) | 
                                        (block.params.basicSystem.date << 17) | 
                                        (block.params.basicSystem.hour << 12) | 
                                        (block.params.basicSystem.minute << 6) | 
                                        block.params.basicSystem.second);
    
    // Ӌ���µ�У��
    UpdateChecksum(&block);
    
    // �Q��Ҫ����ą����Kλ�� (���λ��)
    backupAddr = (g_activeBlockAddr == PARAM_BLOCK_A_ADDR) ? PARAM_BLOCK_B_ADDR : PARAM_BLOCK_A_ADDR;
    
    // ���녢�������λ��
    if (!WriteParameterBlock(backupAddr, &block))
    {
        return false;
    }
    
    // ���»�Ӆ����K��ַ
    g_activeBlockAddr = backupAddr;
    
    // ������CRC
    g_paramCRC = CalculateChecksum(&block);
    
    return true;
}

/**
 * @brief  �@ȡϵ�y���ڕr�g
 * @param  year, month, date, hour, minute, second: ���ڕr�g����
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // �}�u���ڕr�g
    if (year != NULL) *year = basicParams.year;
    if (month != NULL) *month = basicParams.month;
    if (date != NULL) *date = basicParams.date;
    if (hour != NULL) *hour = basicParams.hour;
    if (minute != NULL) *minute = basicParams.minute;
    if (second != NULL) *second = basicParams.second;
    
    return true;
}

/**
 * @brief  ����ϵ�y���ڕr�g
 * @param  year, month, date, hour, minute, second: ���ڕr�g����
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    BasicSystemBlock basicParams;
    
    // �z�����ڕr�g������Ч��
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59)
    {
        return false;
    }
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // �������ڕr�g
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    // ������
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief  ���ӜyԇӋ��
 * @param  �o
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_IncreaseTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return false;
    }
    
    // ���ӜyԇӋ��
    basicParams.testCount++;
    
    // ������
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

/**
 * @brief  �@ȡ�yԇӋ��
 * @param  �o
 * @retval �yԇӋ��
 */
uint32_t PARAM_GetTestCount(void)
{
    BasicSystemBlock basicParams;
    
    // �xȡ����ϵ�y����
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock)))
    {
        return 0;
    }
    
    return basicParams.testCount;
}

/**
 * @brief  �@ȡָ��ԇƬ��͵�ԇƬ����
 * @param  stripType: ԇƬ���
 * @param  ndl: ��ԇƬ�z�yˮƽ
 * @param  udl: ��ʹ��ԇƬ�z�yˮƽ
 * @param  bloodIn: ѪҺ�z�yˮƽ
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    // ���ԇƬ��ͫ@ȡ�����ą���
    switch (stripType)
    {
        case 0: // GLV(Ѫ��)
        case 4: // GAV(Ѫ��)
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
        case 1: // U(����)
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
        case 2: // C(��đ�̴�)
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
        case 3: // TG(�������֬)
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
 * @brief  �@ȡָ��ԇƬ��͵Ĝy������
 * @param  stripType: ԇƬ���
 * @param  lowLimit: �����ֵ
 * @param  highLimit: �����ֵ
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit)
{
    // ���ԇƬ��ͫ@ȡ�����ą���
    switch (stripType)
    {
        case 0: // GLV(Ѫ��)
        case 4: // GAV(Ѫ��)
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
        case 1: // U(����)
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
        case 2: // C(��đ�̴�)
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
        case 3: // TG(�������֬)
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
 * @brief  �@ȡָ��ԇƬ��͵ĕr�򅢔�
 * @param  stripType: ԇƬ���
 * @param  tpl: TPL�r�򅢔�
 * @param  trd: TRD�r�򅢔�
 * @param  evWidth: EV���ȕr�򅢔�
 * @param  group: �����M (1 �� 2)
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group)
{
    // �z�酢���M��Ч��
    if (group != 1 && group != 2)
    {
        return false;
    }
    
    // ���ԇƬ��ͫ@ȡ�����ą���
    switch (stripType)
    {
        case 0: // GLV(Ѫ��)
        case 4: // GAV(Ѫ��)
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
        case 1: // U(����)
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
        case 2: // C(��đ�̴�)
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
        case 3: // TG(�������֬)
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
 * @brief  ���O��������S�A�Oֵ
 * @param  �o
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_ResetToDefault(void)
{
    // �����K
    ParameterBlock defaultBlock;
    memset(&defaultBlock, 0, sizeof(ParameterBlock));
    
    // ��ʼ���^��
    defaultBlock.header.magic = PARAM_MAGIC_NUMBER;
    defaultBlock.header.version = PARAM_VERSION;
    defaultBlock.header.writeCounter = 1;
    defaultBlock.header.timestamp = 0;
    
    // ����ϵ�y����
    defaultBlock.params.basicSystem.lbt = 58;  // 2.58V
    defaultBlock.params.basicSystem.obt = 55;  // 2.55V
    defaultBlock.params.basicSystem.factory = 0; // ʹ����ģʽ
    defaultBlock.params.basicSystem.modelNo = 1;
    defaultBlock.params.basicSystem.fwNo = 10; // �汾1.0
    defaultBlock.params.basicSystem.testCount = 0;
    defaultBlock.params.basicSystem.codeTableVer = 1;
    
    // �O���A�O�r�g (2025-05-01 00:00:00)
    defaultBlock.params.basicSystem.year = 25;
    defaultBlock.params.basicSystem.month = 5;
    defaultBlock.params.basicSystem.date = 1;
    defaultBlock.params.basicSystem.hour = 0;
    defaultBlock.params.basicSystem.minute = 0;
    defaultBlock.params.basicSystem.second = 0;
    
    // �O���h������
    defaultBlock.params.basicSystem.tempLowLimit = 10;  // 10��C
    defaultBlock.params.basicSystem.tempHighLimit = 40; // 40��C
    defaultBlock.params.basicSystem.measureUnit = 1;    // mg/dL
    defaultBlock.params.basicSystem.defaultEvent = 1;   // AC
    defaultBlock.params.basicSystem.stripType = 0;      // GLV(Ѫ��)
    
    // Ӳ�wУ�ʅ���
    defaultBlock.params.hardwareCalib.evT3Trigger = 800;  // 800mV
    defaultBlock.params.hardwareCalib.evWorking = 164;
    defaultBlock.params.hardwareCalib.evT3 = 164;
    defaultBlock.params.hardwareCalib.dacoOffset = 0;
    defaultBlock.params.hardwareCalib.dacdo = 0;
    defaultBlock.params.hardwareCalib.cc211Status = 0;
    defaultBlock.params.hardwareCalib.calTolerance = 8;  // 0.8%
    defaultBlock.params.hardwareCalib.ops = 1.0f;       // �\��У��б�ʞ�1.0
    defaultBlock.params.hardwareCalib.opi = 0.0f;       // �\��У�ʽؾ���0.0
    defaultBlock.params.hardwareCalib.qct = 0;
    defaultBlock.params.hardwareCalib.tempOffset = 0;   // �ض�ƫ���a��
    defaultBlock.params.hardwareCalib.batteryOffset = 0; // 늳�ƫ���a��
    
    // Ѫ�ǅ���
    defaultBlock.params.bgParams.bgCsuTolerance = 10;  // 1.0%�ݲ�
    defaultBlock.params.bgParams.bgNdl = 100;  // ��ԇƬ�z�yˮƽ
    defaultBlock.params.bgParams.bgUdl = 50;   // ��ʹ��ԇƬ�z�yˮƽ
    defaultBlock.params.bgParams.bgBloodIn = 150; // ѪҺ�z�yˮƽ
    memset(defaultBlock.params.bgParams.bgStripLot, 0, 16); // ���ԇƬ��̖
    
    defaultBlock.params.bgParams.bgL = 2;    // ���20 mg/dL�@ʾ"LO"
    defaultBlock.params.bgParams.bgH = 60;   // ���600 mg/dL�@ʾ"HI"
    defaultBlock.params.bgParams.bgT3E37 = 2000;  // T3 ADV�e�`�ֵ
    
    // Ѫ�ǜyԇ�r�򅢔�
    defaultBlock.params.bgParams.bgTPL1 = 10;        // 1��
    defaultBlock.params.bgParams.bgTRD1 = 10;        // 1��
    defaultBlock.params.bgParams.bgEVWidth1 = 5;     // 0.5��
    defaultBlock.params.bgParams.bgTPL2 = 20;        // 2��
    defaultBlock.params.bgParams.bgTRD2 = 20;        // 2��
    defaultBlock.params.bgParams.bgEVWidth2 = 10;    // 1��
    
    // Ѫ�ǜض��a���S��
    defaultBlock.params.bgParams.bgTempA = 0.0f;
    defaultBlock.params.bgParams.bgTempB = 0.0f;
    defaultBlock.params.bgParams.bgTempC = 0.0f;
    
    // Ѫ�Ǻ����a���S��
    defaultBlock.params.bgParams.bgAltA = 0.0f;
    defaultBlock.params.bgParams.bgAltB = 0.0f;
    defaultBlock.params.bgParams.bgAltC = 0.0f;
    
    // Ѫ��Ѫ�����a���S��
    defaultBlock.params.bgParams.bgHctA = 0.0f;
    defaultBlock.params.bgParams.bgHctB = 0.0f;
    defaultBlock.params.bgParams.bgHctC = 0.0f;
    
    // Ѫ���a���ؾ�ͱ����S��
    defaultBlock.params.bgParams.bgCompS = 1.0f;
    defaultBlock.params.bgParams.bgCompI = 0.0f;
    
    // ���ᅢ��
    defaultBlock.params.uaParams.uaCsuTolerance = 10;  // 1.0%�ݲ�
    defaultBlock.params.uaParams.uaNdl = 100;  // ��ԇƬ�z�yˮƽ
    defaultBlock.params.uaParams.uaUdl = 50;   // ��ʹ��ԇƬ�z�yˮƽ
    defaultBlock.params.uaParams.uaBloodIn = 150; // ѪҺ�z�yˮƽ
    memset(defaultBlock.params.uaParams.uaStripLot, 0, 16); // ���ԇƬ��̖
    
    defaultBlock.params.uaParams.uaL = 20;   // ���2.0 mg/dL�@ʾ"LO"
    defaultBlock.params.uaParams.uaH = 200;  // ���20.0 mg/dL�@ʾ"HI"
    defaultBlock.params.uaParams.uaT3E37 = 2000;  // T3 ADV�e�`�ֵ
    
    // ����yԇ�r�򅢔�
    defaultBlock.params.uaParams.uaTPL1 = 40;        // 4��
    defaultBlock.params.uaParams.uaTRD1 = 40;        // 4��
    defaultBlock.params.uaParams.uaEVWidth1 = 5;     // 0.5��
    defaultBlock.params.uaParams.uaTPL2 = 80;        // 8��
    defaultBlock.params.uaParams.uaTRD2 = 80;        // 8��
    defaultBlock.params.uaParams.uaEVWidth2 = 10;    // 1��
    
    // ����ض��a���S��
    defaultBlock.params.uaParams.uaTempA = 0.0f;
    defaultBlock.params.uaParams.uaTempB = 0.0f;
    defaultBlock.params.uaParams.uaTempC = 0.0f;
    
    // ���ả���a���S��
    defaultBlock.params.uaParams.uaAltA = 0.0f;
    defaultBlock.params.uaParams.uaAltB = 0.0f;
    defaultBlock.params.uaParams.uaAltC = 0.0f;
    
    // �����a���ؾ�ͱ����S��
    defaultBlock.params.uaParams.uaCompS = 1.0f;
    defaultBlock.params.uaParams.uaCompI = 0.0f;
    
    // ��đ�̴�����
    defaultBlock.params.chParams.chCsuTolerance = 10;  // 1.0%�ݲ�
    defaultBlock.params.chParams.chNdl = 100;  // ��ԇƬ�z�yˮƽ
    defaultBlock.params.chParams.chUdl = 50;   // ��ʹ��ԇƬ�z�yˮƽ
    defaultBlock.params.chParams.chBloodIn = 150; // ѪҺ�z�yˮƽ
    memset(defaultBlock.params.chParams.chStripLot, 0, 16); // ���ԇƬ��̖
    
    defaultBlock.params.chParams.chL = 50;   // ���50 mg/dL�@ʾ"LO"
    defaultBlock.params.chParams.chH = 500;  // ���500 mg/dL�@ʾ"HI"
    defaultBlock.params.chParams.chT3E37 = 2000;  // T3 ADV�e�`�ֵ
    
    // ��đ�̴��yԇ�r�򅢔�
    defaultBlock.params.chParams.chTPL1 = 120;       // 12��
    defaultBlock.params.chParams.chTRD1 = 150;       // 15��
    defaultBlock.params.chParams.chEVWidth1 = 5;     // 0.5��
    defaultBlock.params.chParams.chTPL2 = 150;       // 15��
    defaultBlock.params.chParams.chTRD2 = 170;       // 17��
    defaultBlock.params.chParams.chEVWidth2 = 10;    // 1��
    
    // ��đ�̴��ض��a���S��
    defaultBlock.params.chParams.chTempA = 0.0f;
    defaultBlock.params.chParams.chTempB = 0.0f;
    defaultBlock.params.chParams.chTempC = 0.0f;
    
    // ��đ�̴��a���ؾ�ͱ����S��
    defaultBlock.params.chParams.chCompS = 1.0f;
    defaultBlock.params.chParams.chCompI = 0.0f;
    
    // �������֬����
    defaultBlock.params.tgParams.tgCsuTolerance = 10;  // 1.0%�ݲ�
    defaultBlock.params.tgParams.tgNdl = 100;  // ��ԇƬ�z�yˮƽ
    defaultBlock.params.tgParams.tgUdl = 50;   // ��ʹ��ԇƬ�z�yˮƽ
    defaultBlock.params.tgParams.tgBloodIn = 150; // ѪҺ�z�yˮƽ
    memset(defaultBlock.params.tgParams.tgStripLot, 0, 16); // ���ԇƬ��̖
    
    defaultBlock.params.tgParams.tgL = 50;   // ���50 mg/dL�@ʾ"LO"
    defaultBlock.params.tgParams.tgH = 1000; // ���1000 mg/dL�@ʾ"HI"
    defaultBlock.params.tgParams.tgT3E37 = 2000;  // T3 ADV�e�`�ֵ
    
    // �������֬�yԇ�r�򅢔�
    defaultBlock.params.tgParams.tgTPL1 = 120;       // 12��
    defaultBlock.params.tgParams.tgTRD1 = 150;       // 15��
    defaultBlock.params.tgParams.tgEVWidth1 = 5;     // 0.5��
    defaultBlock.params.tgParams.tgTPL2 = 150;       // 15��
    defaultBlock.params.tgParams.tgTRD2 = 170;       // 17��
    defaultBlock.params.tgParams.tgEVWidth2 = 10;    // 1��
    
    // �������֬�ض��a���S��
    defaultBlock.params.tgParams.tgTempA = 0.0f;
    defaultBlock.params.tgParams.tgTempB = 0.0f;
    defaultBlock.params.tgParams.tgTempC = 0.0f;
    
    // �������֬�a���ؾ�ͱ����S��
    defaultBlock.params.tgParams.tgCompS = 1.0f;
    defaultBlock.params.tgParams.tgCompI = 0.0f;
    
    // Ӌ��У��
    UpdateChecksum(&defaultBlock);
    
    // ���������^
    FLASH_Unlock();
    FLASH_ErasePage(PARAM_BLOCK_A_ADDR);
    FLASH_ErasePage(PARAM_BLOCK_B_ADDR);
    FLASH_Lock();
    
    // ���녢���������^A
    bool resultA = WriteParameterBlock(PARAM_BLOCK_A_ADDR, &defaultBlock);
    
    // ���녢���������^B
    bool resultB = WriteParameterBlock(PARAM_BLOCK_B_ADDR, &defaultBlock);
    
    // ���»�Ӆ����K��ַ
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
    
    // ������CRC
    g_paramCRC = CalculateChecksum(&defaultBlock);
    
    return (resultA || resultB);
}

/**
 * @brief  ����yԇӛ�
 * @param  record: �yԇӛ�
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_SaveTestRecord(TestRecord *record)
{
    if (record == NULL)
    {
        return false;
    }
    
    // �yԇӛ䛴�С
    uint32_t recordSize = sizeof(TestRecord);
    
    // ���Ҝyԇӛ䛅^����һ������λ��
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    
    while (addr < endAddr)
    {
        // �xȡһ����Ԫ�������0xFFFF��ʾ��λ��δʹ��
        checkFlag = *((uint16_t *)addr);
        if (checkFlag == 0xFFFF)
        {
            break;
        }
        addr += recordSize;
    }
    
    // ���ӛ䛅^�ѝM����������ӛ䛅^�K���^�_ʼ
    if (addr >= endAddr)
    {
        FLASH_Unlock();
        
        // ����ӛ䛅^ (ÿ�1KB����Ҫ���������)
        for (uint32_t pageAddr = RECORD_AREA_ADDR; pageAddr < endAddr; pageAddr += 0x400)
        {
            FLASH_ErasePage(pageAddr);
        }
        
        FLASH_Lock();
        
        // ���O��ַ��ӛ䛅^��ʼλ��
        addr = RECORD_AREA_ADDR;
    }
    
    // ������ӛ�
    FLASH_Unlock();
    bool result = FlashWriteBuffer(addr, (uint8_t *)record, recordSize);
    FLASH_Lock();
    
    // ���Ӝyԇ�Δ�Ӌ��
    if (result)
    {
        PARAM_IncreaseTestCount();
    }
    
    return result;
}

/**
 * @brief  �@ȡ�yԇӛ�
 * @param  index: ӛ�������0��ʾ���£�
 * @param  record: �yԇӛ䛾��n�^
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_GetTestRecord(uint32_t index, TestRecord *record)
{
    if (record == NULL)
    {
        return false;
    }
    
    // �yԇӛ䛴�С
    uint32_t recordSize = sizeof(TestRecord);
    
    // Ӌ��ӛ䛅^���Ա�������ӛ䛔�
    uint32_t maxRecords = RECORD_AREA_SIZE / recordSize;
    
    // �z�������Ƿ񳬳�����
    if (index >= maxRecords)
    {
        return false;
    }
    
    // ��������ӛ�λ��
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    uint32_t lastRecordAddr = 0;
    
    while (addr < endAddr)
    {
        // �xȡһ����Ԫ�������0xFFFF��ʾ��λ��δʹ��
        checkFlag = *((uint16_t *)addr);
        if (checkFlag == 0xFFFF)
        {
            break;
        }
        lastRecordAddr = addr;
        addr += recordSize;
    }
    
    // ����]���ҵ��κ�ӛ�
    if (lastRecordAddr == 0)
    {
        return false;
    }
    
    // ������ӛ��_ʼ����
    uint32_t targetAddr;
    if (addr == RECORD_AREA_ADDR)
    {
        // �����һ��λ�þ���0xFFFF����ʾӛ䛅^���
        return false;
    }
    else if (addr == RECORD_AREA_ADDR + recordSize)
    {
        // ���ֻ��һ�lӛ�
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
        // Ӌ��Ŀ��ӛ䛵�ַ
        if (index == 0)
        {
            // ����ӛ�
            targetAddr = lastRecordAddr;
        }
        else
        {
            // ����ָ��������ӛ�
            targetAddr = lastRecordAddr - (index * recordSize);
            if (targetAddr < RECORD_AREA_ADDR)
            {
                // ̎����@
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
    
    // �xȡӛ�
    memcpy(record, (void *)targetAddr, recordSize);
    
    return true;
}

/**
 * @brief  �@ȡ�yԇӛ䛿���
 * @param  �o
 * @retval �yԇӛ䛔���
 */
uint32_t PARAM_GetTestRecordCount(void)
{
    // �yԇӛ䛴�С
    uint32_t recordSize = sizeof(TestRecord);
    
    // ���Ҝyԇӛ䛅^����ʹ�ÿ��g
    uint32_t addr = RECORD_AREA_ADDR;
    uint32_t endAddr = RECORD_AREA_ADDR + RECORD_AREA_SIZE;
    uint16_t checkFlag;
    uint32_t count = 0;
    
    while (addr < endAddr)
    {
        // �xȡһ����Ԫ�������0xFFFF��ʾ��λ��δʹ��
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
 * @brief  �������Мyԇӛ�
 * @param  �o
 * @retval true: �ɹ�, false: ʧ��
 */
bool PARAM_EraseAllTestRecords(void)
{
    FLASH_Unlock();
    
    // ����ӛ䛅^ (ÿ�1KB����Ҫ���������)
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
 * @brief  �z�酢���K��Ч��
 * @param  blockAddr: �����K��ַ
 * @retval true: ��Ч, false: �oЧ
 */
static bool IsParamBlockValid(uint32_t blockAddr)
{
    ParamBlockHeader *header = (ParamBlockHeader *)blockAddr;
    
    // �z��ħ��
    if (header->magic != PARAM_MAGIC_NUMBER)
    {
        return false;
    }
    
    // �z��汾
    if (header->version != PARAM_VERSION)
    {
        return false;
    }
    
    // Ӌ��У��
    ParameterBlock *block = (ParameterBlock *)blockAddr;
    uint16_t calcChecksum = CalculateChecksum(block);
    
    // �z��У��
    return (calcChecksum == header->checksum);
}

/**
 * @brief  Ӌ�ㅢ���KУ��
 * @param  block: �����K
 * @retval У��
 */
static uint16_t CalculateChecksum(const ParameterBlock *block)
{
    uint16_t checksum = 0;
    uint8_t *data = (uint8_t *)block;
    uint32_t size = sizeof(ParameterBlock) - sizeof(uint16_t);  // �ų�У���ֶ�
    
    // Ӌ�㺆�ε�У��
    for (uint32_t i = 0; i < size; i++)
    {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief  �������KУ��
 * @param  block: �����K
 * @retval �o
 */
static void UpdateChecksum(ParameterBlock *block)
{
    block->header.checksum = CalculateChecksum(block);
}

/**
 * @brief  ���녢���K��Flash
 * @param  addr: Ŀ�˵�ַ
 * @param  block: �����K
 * @retval true: �ɹ�, false: ʧ��
 */
static bool WriteParameterBlock(uint32_t addr, const ParameterBlock *block)
{
    bool result = false;
    
    // ���Ȳ����
    FLASH_Unlock();
    FLASH_Status status = FLASH_ErasePage(addr);
    
    if (status == FLASH_COMPLETE)
    {
        // ���녢������
        result = FlashWriteBuffer(addr, (const uint8_t *)block, sizeof(ParameterBlock));
    }
    
    FLASH_Lock();
    
    return result;
}

/**
 * @brief  �����n�^����Flash
 * @param  addr: Ŀ�˵�ַ
 * @param  buffer: �������n�^
 * @param  size: ������С
 * @retval true: �ɹ�, false: ʧ��
 */
static bool FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    // �z�酢��
    if (buffer == NULL || size == 0)
    {
        return false;
    }
    
    // �z���ַ�Ƿ��R4�ֹ�
    if (addr % 4 != 0)
    {
        return false;
    }
    
    // ���֌��� (4�ֹ���һ����)
    for (uint32_t i = 0; i < size; i += 4)
    {
        uint32_t data;
        if (i + 4 <= size)
        {
            // ������
            data = ((uint32_t)buffer[i]) |
                   ((uint32_t)buffer[i+1] << 8) |
                   ((uint32_t)buffer[i+2] << 16) |
                   ((uint32_t)buffer[i+3] << 24);
        }
        else
        {
            // ���᲻��4�ֹ��Ĳ���
            data = 0xFFFFFFFF;  // �����ȫ1
            for (uint32_t j = 0; j < (size - i); j++)
            {
                data &= ~(0xFF << (j * 8));
                data |= ((uint32_t)buffer[i+j] << (j * 8));
            }
        }
        
        // ����һ���� (4�ֹ�)
        FLASH_Status status = FLASH_ProgramWord(addr + i, data);
        if (status != FLASH_COMPLETE)
        {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief  �@ȡ��ǰ��Ӆ����K
 * @param  block: �����K���n�^
 * @retval true: �ɹ�, false: ʧ��
 */
static bool GetActiveParameterBlock(ParameterBlock *block)
{
    if (block == NULL)
    {
        return false;
    }
    
    // �@ȡ��Ӆ����K��ַ
    uint32_t addr = GetActiveBlockAddress();
    if (addr == 0)
    {
        return false;
    }
    
    // �xȡ���������K
    memcpy(block, (void *)addr, sizeof(ParameterBlock));
    
    return true;
}

/**
 * @brief  �@ȡ��Ӆ����K��ַ
 * @param  �o
 * @retval �����K��ַ (0��ʾʧ��)
 */
static uint32_t GetActiveBlockAddress(void)
{
    // ���ȫ��׃���ѽ������˻�ӉK��ַ��ֱ�ӷ���
    if (g_activeBlockAddr != 0 && IsParamBlockValid(g_activeBlockAddr))
    {
        return g_activeBlockAddr;
    }
    
    // ��t���²��һ�Ӆ����K
    FindParameterBlock(&g_activeBlockAddr);
    
    return g_activeBlockAddr;
}

/**
 * @brief  ������Ч�ą����K
 * @param  blockAddr: �����ҵ��ą����K��ַ
 * @retval true: �ɹ�, false: ʧ��
 */
static bool FindParameterBlock(uint32_t *blockAddr)
{
    if (blockAddr == NULL)
    {
        return false;
    }
    
    // �z�酢���KA�Ƿ���Ч
    bool validA = IsParamBlockValid(PARAM_BLOCK_A_ADDR);
    
    // �z�酢���KB�Ƿ���Ч
    bool validB = IsParamBlockValid(PARAM_BLOCK_B_ADDR);
    
    // �O���A�O��
    *blockAddr = 0;
    
    // ����ɂ�����Ч���x����Ӌ����������ǂ�
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
    // ���ֻ��A��Ч
    else if (validA)
    {
        *blockAddr = PARAM_BLOCK_A_ADDR;
        return true;
    }
    // ���ֻ��B��Ч
    else if (validB)
    {
        *blockAddr = PARAM_BLOCK_B_ADDR;
        return true;
    }
    
    // ���oЧ
    return false;
}

/**
 * @brief  �@ȡ�����K��͌�����ƫ�ƺʹ�С
 * @param  blockType: �����K���
 * @param  offset: ����ƫ��
 * @param  size: ���ش�С
 * @retval �o
 */
static void GetBlockOffset(uint8_t blockType, uint32_t *offset, uint16_t *size)
{
    // �O���A�Oֵ
    *offset = 0;
    *size = 0;
    
    // ���������K����O��ƫ�ƺʹ�С
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