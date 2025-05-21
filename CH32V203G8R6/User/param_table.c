/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/08
 * Description        : �๦�������yԇ�x�������a��
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "param_table.h"
#include "debug.h"
#include "string.h"
#include "ch32v20x_flash.h"

/* �����탦����RAM�� */
static uint8_t param_table[PARAM_TABLE_SIZE];

/* ��������Flash�еă����ַ */
#define FLASH_PARAM_ADDR      0x08010000   // ʹ���m�ϵ�Flash��ַ

/* ˽�к����� */
static uint16_t CalculateChecksum(void);
static uint8_t CalculateCRC16(void);

/*********************************************************************
 * @fn      PARAM_Init
 *
 * @brief   ��ʼ����������Flash�xȡ���O���A�Oֵ��
 *
 * @return  None
 */
void PARAM_Init(void)
{
    /* �ȏ�Flash���d�녢���� */
    PARAM_LoadFromFlash();
    
    /* �z�酢����У���Ƿ����_ */
    if(!PARAM_VerifyChecksum())
    {
        printf("Parameter table checksum error, loading defaults.\r\n");
        
        /* У�ʧ�����d���A�O���� */
        PARAM_SetDefault();
        
        /* ����У�� */
        PARAM_UpdateChecksum();
        
        /* ���浽Flash�� */
        PARAM_SaveToFlash();
    }
    else
    {
        printf("Parameter table loaded successfully.\r\n");
    }
}

/*********************************************************************
 * @fn      PARAM_GetByte
 *
 * @brief   �xȡһ��λԪ�M������
 *
 * @param   addr - ������ַ
 *
 * @return  ����ֵ
 */
uint8_t PARAM_GetByte(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        return param_table[addr];
    }
    return 0;
}

/*********************************************************************
 * @fn      PARAM_SetByte
 *
 * @brief   �O��һ��λԪ�M������
 *
 * @param   addr - ������ַ
 *          value - ����ֵ
 *
 * @return  None
 */
void PARAM_SetByte(uint16_t addr, uint8_t value)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        param_table[addr] = value;
    }
}

/*********************************************************************
 * @fn      PARAM_GetWord
 *
 * @brief   �xȡһ���֣�2λԪ�M��������
 *
 * @param   addr - ������ʼ��ַ
 *
 * @return  ����ֵ
 */
uint16_t PARAM_GetWord(uint16_t addr)
{
    uint16_t value = 0;
    
    if(addr + 1 < PARAM_TABLE_SIZE)
    {
        value = (uint16_t)param_table[addr] | ((uint16_t)param_table[addr + 1] << 8);
    }
    
    return value;
}

/*********************************************************************
 * @fn      PARAM_SetWord
 *
 * @brief   �O��һ���֣�2λԪ�M��������
 *
 * @param   addr - ������ʼ��ַ
 *          value - ����ֵ
 *
 * @return  None
 */
void PARAM_SetWord(uint16_t addr, uint16_t value)
{
    if(addr + 1 < PARAM_TABLE_SIZE)
    {
        param_table[addr] = (uint8_t)(value & 0xFF);
        param_table[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/*********************************************************************
 * @fn      PARAM_GetFloat
 *
 * @brief   �xȡһ�����c����4λԪ�M��������
 *
 * @param   addr - ������ʼ��ַ
 *
 * @return  ����ֵ
 */
float PARAM_GetFloat(uint16_t addr)
{
    float value = 0.0f;
    uint8_t *p = (uint8_t *)&value;
    
    if(addr + 3 < PARAM_TABLE_SIZE)
    {
        p[0] = param_table[addr];
        p[1] = param_table[addr + 1];
        p[2] = param_table[addr + 2];
        p[3] = param_table[addr + 3];
    }
    
    return value;
}

/*********************************************************************
 * @fn      PARAM_SetFloat
 *
 * @brief   �O��һ�����c����4λԪ�M��������
 *
 * @param   addr - ������ʼ��ַ
 *          value - ����ֵ
 *
 * @return  None
 */
void PARAM_SetFloat(uint16_t addr, float value)
{
    uint8_t *p = (uint8_t *)&value;
    
    if(addr + 3 < PARAM_TABLE_SIZE)
    {
        param_table[addr] = p[0];
        param_table[addr + 1] = p[1];
        param_table[addr + 2] = p[2];
        param_table[addr + 3] = p[3];
    }
}

/*********************************************************************
 * @fn      PARAM_GetString
 *
 * @brief   �xȡ�ִ�������
 *
 * @param   addr - ������ʼ��ַ
 *          buffer - ���վ��n�^
 *          length - �ִ��L��
 *
 * @return  None
 */
void PARAM_GetString(uint16_t addr, uint8_t *buffer, uint8_t length)
{
    if(addr + length <= PARAM_TABLE_SIZE && buffer != NULL)
    {
        memcpy(buffer, &param_table[addr], length);
    }
}

/*********************************************************************
 * @fn      PARAM_SetString
 *
 * @brief   �O���ִ�������
 *
 * @param   addr - ������ʼ��ַ
 *          buffer - �ִ����n�^
 *          length - �ִ��L��
 *
 * @return  None
 */
void PARAM_SetString(uint16_t addr, uint8_t *buffer, uint8_t length)
{
    if(addr + length <= PARAM_TABLE_SIZE && buffer != NULL)
    {
        memcpy(&param_table[addr], buffer, length);
    }
}

/*********************************************************************
 * @fn      PARAM_VerifyChecksum
 *
 * @brief   ��C������У�͡�
 *
 * @return  1: У�ͨ�^��0: У�ʧ��
 */
uint8_t PARAM_VerifyChecksum(void)
{
    uint16_t checksum = CalculateChecksum();
    uint16_t stored_checksum = (uint16_t)param_table[PARAM_SUM_L] | ((uint16_t)param_table[PARAM_SUM_H] << 8);
    
    return (checksum == stored_checksum) ? 1 : 0;
}

/*********************************************************************
 * @fn      PARAM_UpdateChecksum
 *
 * @brief   ��������У�͡�
 *
 * @return  None
 */
void PARAM_UpdateChecksum(void)
{
    uint16_t checksum = CalculateChecksum();
    
    param_table[PARAM_SUM_L] = (uint8_t)(checksum & 0xFF);
    param_table[PARAM_SUM_H] = (uint8_t)((checksum >> 8) & 0xFF);
    
    /* Ӌ��CRC16У� */
    param_table[PARAM_CRC16] = CalculateCRC16();
}

/*********************************************************************
 * @fn      CalculateChecksum
 *
 * @brief   Ӌ�ㅢ����У�ͣ�0-674λַ�ă��ݿ��ͣ���
 *
 * @return  У��
 */
static uint16_t CalculateChecksum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    for(i = 0; i < PARAM_SUM_L; i++)
    {
        sum += param_table[i];
    }
    
    return sum;
}

/*********************************************************************
 * @fn      CalculateCRC16
 *
 * @brief   Ӌ�ㅢ����CRC16У򞣨����У���ڃȵ������Y�ϣ���
 *          ���Ό��F���ɸ�����Ҫ���Ğ���}�s��CRC�㷨��
 *
 * @return  CRC16У�ֵ
 */
static uint8_t CalculateCRC16(void)
{
    uint8_t crc = 0;
    uint16_t i;
    
    for(i = 0; i < PARAM_CRC16; i++)
    {
        crc ^= param_table[i];
    }
    
    return crc;
}

/*********************************************************************
 * @fn      PARAM_SaveToFlash
 *
 * @brief   ���������浽Flash�С�
 *
 * @return  None
 */
void PARAM_SaveToFlash(void)
{
    uint16_t i;
    uint32_t Address = FLASH_PARAM_ADDR;
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ������������^ */
    FLASH_ErasePage(FLASH_PARAM_ADDR);
    
    /* ���녢���� */
    for(i = 0; i < PARAM_TABLE_SIZE; i += 2)
    {
        uint16_t data;
        
        if(i + 1 < PARAM_TABLE_SIZE)
        {
            data = ((uint16_t)param_table[i + 1] << 8) | param_table[i];
        }
        else
        {
            data = param_table[i];
        }
        
        FLASH_ProgramHalfWord(Address, data);
        Address += 2;
    }
    
    /* �i��Flash */
    FLASH_Lock();
}

/*********************************************************************
 * @fn      PARAM_LoadFromFlash
 *
 * @brief   ��Flash���d�녢����
 *
 * @return  None
 */
void PARAM_LoadFromFlash(void)
{
    uint16_t i;
    uint32_t Address = FLASH_PARAM_ADDR;
    
    for(i = 0; i < PARAM_TABLE_SIZE; i += 2)
    {
        uint16_t data = *(volatile uint16_t*)Address;
        
        param_table[i] = (uint8_t)(data & 0xFF);
        
        if(i + 1 < PARAM_TABLE_SIZE)
        {
            param_table[i + 1] = (uint8_t)((data >> 8) & 0xFF);
        }
        
        Address += 2;
    }
}

/*********************************************************************
 * @fn      PARAM_SetDefault
 *
 * @brief   �O�Å������A�Oֵ��
 *
 * @return  None
 */
void PARAM_SetDefault(void)
{
    /* ��Յ����� */
    memset(param_table, 0, PARAM_TABLE_SIZE);
    
    /* ϵ�y���������A�Oֵ */
    param_table[PARAM_LBT] = 58;        // ��늳��ֵ: 2.58V
    param_table[PARAM_OBT] = 55;        // 늳غıM�ֵ: 2.55V
    param_table[PARAM_FACTORY] = 0;     // ʹ����ģʽ
    param_table[PARAM_MODEL_NO] = 1;    // �aƷ��̖
    param_table[PARAM_FW_NO] = 10;      // �g�w�汾 1.0
    PARAM_SetWord(PARAM_NOT, 0);        // �yԇ�Δ�
    PARAM_SetWord(PARAM_CODE_TABLE_V, 1); // ���a��汾 1
    
    /* �r�g�O�������A�Oֵ���O�Þ鮔ǰ���g�r�g�� */
    param_table[PARAM_YEAR] = 25;       // 2025�� (25)
    param_table[PARAM_MONTH] = 5;       // 5��
    param_table[PARAM_DATE] = 21;        // 21��
    param_table[PARAM_HOUR] = 12;       // 12�r
    param_table[PARAM_MINUTE] = 0;      // 0��
    param_table[PARAM_SECOND] = 0;      // 0��
    
    /* �yԇ�l�������A�Oֵ */
    param_table[PARAM_TLL] = 10;        // �����ض�����: 10��C
    param_table[PARAM_TLH] = 40;        // �����ض�����: 40��C
    param_table[PARAM_MGDL] = UNIT_MG_DL; // �A�O��λ: mg/dL
    param_table[PARAM_EVENT] = EVENT_AC;   // �A�O�¼�: �ǰ
    param_table[PARAM_STRIP_TYPE] = STRIP_TYPE_GLV; // �A�OԇƬ: Ѫ��
    
    /* Ӳ�wУ�ʅ����A�Oֵ */
    PARAM_SetWord(PARAM_EV_T3_TRG, 120);   // EV_T3�|�l늉�
    param_table[PARAM_EV_WORKING] = 164;   // ��ȹ���늉�
    param_table[PARAM_EV_T3] = 164;        // ѪҺ�y��늘O늉�
    param_table[PARAM_DACO] = 0;           // DACƫ���a��
    
    /* ����yԇ�Ŀ�O��W늘OPWMռ�ձ��A�Oֵ */
    PARAM_SetWord(PARAM_BG_W_PWM_DUTY, 1000); // Ѫ��W늘OPWMռ�ձ��A�Oֵ: 100.0%
    PARAM_SetWord(PARAM_U_W_PWM_DUTY, 750);  // ����W늘OPWMռ�ձ��A�Oֵ: 75.0%
    PARAM_SetWord(PARAM_C_W_PWM_DUTY, 250);  // ��đ�̴�W늘OPWMռ�ձ��A�Oֵ: 25.0%
    PARAM_SetWord(PARAM_TG_W_PWM_DUTY, 250); // �������֬W늘OPWMռ�ձ��A�Oֵ: 25.0%

    param_table[PARAM_DACDO] = 0;          // DACУ�����텢��
    param_table[PARAM_CC211NODONE] = 0;    // CC211�������
    param_table[PARAM_CAL_TOL] = 8;        // OPS/OPIУ���ݲ�: 0.8%
    PARAM_SetFloat(PARAM_OPS, 1.0);        // OPAУ��б��
    PARAM_SetFloat(PARAM_OPI, 0.0);        // OPAУ�ʽؾ�
    param_table[PARAM_QCT] = 0;            // QCTУ�ʜyԇ��λԪ�M
    param_table[PARAM_TOFFSET] = 0;        // �ض�У��ƫ��
    param_table[PARAM_BOFFSET] = 0;        // 늳�У��ƫ��
    
    /* Ѫ��(GLV/GAV)���Å����A�Oֵ */
    param_table[PARAM_BG_CSU_TOL] = 10;    // ԇƬ�z���ݲ�: 3%
    PARAM_SetWord(PARAM_BG_NDL, 800);      // ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_BG_UDL, 850);      // ��ʹ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_BG_BLOOD_IN, 900); // ѪҺ�y��ˮƽ
    /* ԇƬ��̖�A�O��հ� */
    param_table[PARAM_BG_L] = 30;          // Ѫ��ֵ����: 3.0 mmol/L
    param_table[PARAM_BG_H] = 240;         // Ѫ��ֵ����: 24.0 mmol/L
    PARAM_SetWord(PARAM_BG_T3_E37, 1800);  // T3 ADV��E37�ֵ
    
    /* ����(U)���Å����A�Oֵ */
    param_table[PARAM_U_CSU_TOL] = 10;     // ԇƬ�z���ݲ�: 3%
    PARAM_SetWord(PARAM_U_NDL, 800);       // ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_U_UDL, 850);       // ��ʹ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_U_BLOOD_IN, 900);  // ѪҺ�y��ˮƽ
    /* ԇƬ��̖�A�O��հ� */
    param_table[PARAM_U_L] = 20;           // ����ֵ����
    param_table[PARAM_U_H] = 130;          // ����ֵ����
    PARAM_SetWord(PARAM_U_T3_E37, 1800);   // T3 ADV��E37�ֵ
    
    /* ��đ�̴�(C)���Å����A�Oֵ */
    param_table[PARAM_C_CSU_TOL] = 10;     // ԇƬ�z���ݲ�: 3%
    PARAM_SetWord(PARAM_C_NDL, 800);       // ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_C_UDL, 850);       // ��ʹ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_C_BLOOD_IN, 900);  // ѪҺ�y��ˮƽ
    /* ԇƬ��̖�A�O��հ� */
    param_table[PARAM_C_L] = 25;           // ��đ�̴�ֵ����
    /* ʹ��PARAM_SetWord�惦���^uint8_t������ֵ */
    PARAM_SetWord(PARAM_C_H, 400);         // ��đ�̴�ֵ�����O��400��ʹ��2��λԪ�M
    PARAM_SetWord(PARAM_C_T3_E37, 1800);   // T3 ADV��E37�ֵ
    
    /* �������֬(TG)���Å����A�Oֵ */
    param_table[PARAM_TG_CSU_TOL] = 10;    // ԇƬ�z���ݲ�: 3%
    PARAM_SetWord(PARAM_TG_NDL, 800);      // ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_TG_UDL, 850);      // ��ʹ��ԇƬ�y��ˮƽ
    PARAM_SetWord(PARAM_TG_BLOOD_IN, 900); // ѪҺ�y��ˮƽ
    /* ԇƬ��̖�A�O��հ� */
    param_table[PARAM_TG_L] = 30;          // �������ֵ֬����
    /* ʹ��PARAM_SetWord�惦���^uint8_t������ֵ */
    PARAM_SetWord(PARAM_TG_H, 500);        // �������ֵ֬�����O��500��ʹ��2��λԪ�M
    PARAM_SetWord(PARAM_TG_T3_E37, 1800);  // T3 ADV��E37�ֵ
    
    /* ����^�򱣳֞�0 */
    
    /* У�^���� PARAM_UpdateChecksum() �Ԅ�Ӌ�� */
}

/*********************************************************************
 * @fn      StripType_GetName
 *
 * @brief   ȡ��ԇƬ������Q��
 *
 * @param   type - ԇƬ���
 *
 * @return  ԇƬ������Q
 */
const char* StripType_GetName(StripType_TypeDef type)
{
    switch(type)
    {
        case STRIP_TYPE_GLV:
            return "Blood Glucose";
        case STRIP_TYPE_U:
            return "Uric Acid";
        case STRIP_TYPE_C:
            return "Cholesterol";
        case STRIP_TYPE_TG:
            return "Triglycerides";
        case STRIP_TYPE_GAV:
            return "Blood Glucose (GAV)";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Unit_GetSymbol
 *
 * @brief   ȡ�Ý�Ȇ�λ��̖��
 *
 * @param   unit - ��Ȇ�λ
 *
 * @return  ��Ȇ�λ��̖
 */
const char* Unit_GetSymbol(Unit_TypeDef unit)
{
    switch(unit)
    {
        case UNIT_MMOL_L:
            return "mmol/L";
        case UNIT_MG_DL:
            return "mg/dL";
        case UNIT_GM_DL:
            return "gm/dl";
        default:
            return "?";
    }
}

/*********************************************************************
 * @fn      PARAM_GetDateTime
 *
 * @brief   �@ȡ�������Ѓ�������ڕr�g��
 *
 * @param   year, month, date, hour, minute, second - ���ڕr�g׃����ָ�
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                         uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    if(year == NULL || month == NULL || date == NULL || 
       hour == NULL || minute == NULL || second == NULL)
    {
        return 0;
    }
    
    *year = param_table[PARAM_YEAR];
    *month = param_table[PARAM_MONTH];
    *date = param_table[PARAM_DATE];
    *hour = param_table[PARAM_HOUR];
    *minute = param_table[PARAM_MINUTE];
    *second = param_table[PARAM_SECOND];
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_SetDateTime
 *
 * @brief   �O�Å������е����ڕr�g��
 *
 * @param   year, month, date, hour, minute, second - ���ڕr�gֵ
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date, 
                         uint8_t hour, uint8_t minute, uint8_t second)
{
    /* �z�����ڕr�g����Ч�� */
    if(year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
       hour > 23 || minute > 59 || second > 59)
    {
        return 0;
    }
    
    param_table[PARAM_YEAR] = year;
    param_table[PARAM_MONTH] = month;
    param_table[PARAM_DATE] = date;
    param_table[PARAM_HOUR] = hour;
    param_table[PARAM_MINUTE] = minute;
    param_table[PARAM_SECOND] = second;
    
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    /* ��������ą������浽Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_IncreaseTestCount
 *
 * @brief   ���ӜyԇӋ������
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_IncreaseTestCount(void)
{
    uint16_t count = PARAM_GetWord(PARAM_NOT);
    count++;
    PARAM_SetWord(PARAM_NOT, count);
    
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    /* ��������ą������浽Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetTestCount
 *
 * @brief   �@ȡ�yԇӋ����ֵ��
 *
 * @return  �yԇӋ����ֵ
 */
uint16_t PARAM_GetTestCount(void)
{
    return PARAM_GetWord(PARAM_NOT);
}

/*********************************************************************
 * @fn      PARAM_ReadBlock
 *
 * @brief   �xȡһ�������^�K��
 *
 * @param   block - �^�K���
 *          data - �������n�^
 *          size - ������С
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_ReadBlock(BlockType_TypeDef block, void *data, uint16_t size)
{
    uint16_t start_addr = 0;
    uint16_t block_size = 0;
    
    /* �����z�� */
    if(data == NULL || block >= BLOCK_MAX)
    {
        return 0;
    }
    
    /* �_���^�K����ʼ��ַ�ʹ�С */
    switch(block)
    {
        case BLOCK_BASIC_SYSTEM:
            start_addr = 0;
            block_size = 20;
            break;
        case BLOCK_HARDWARE_CALIB:
            start_addr = 20;
            block_size = 19;
            break;
        case BLOCK_BG_PARAMS:
            start_addr = 39;
            block_size = 174;
            break;
        case BLOCK_U_PARAMS:
            start_addr = 213;
            block_size = 142;
            break;
        case BLOCK_C_PARAMS:
            start_addr = 355;
            block_size = 142;
            break;
        case BLOCK_TG_PARAMS:
            start_addr = 497;
            block_size = 142;
            break;
        case BLOCK_RESERVED:
            start_addr = 639;
            block_size = 39;
            break;
        default:
            return 0;
    }
    
    /* �z�锵����С�Ƿ��^�^�K��С */
    if(size > block_size)
    {
        size = block_size;
    }
    
    /* �}�u�������� */
    memcpy(data, &param_table[start_addr], size);
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_UpdateBlock
 *
 * @brief   ����һ�������^�K��
 *
 * @param   block - �^�K���
 *          data - �������n�^
 *          size - ������С
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_UpdateBlock(BlockType_TypeDef block, void *data, uint16_t size)
{
    uint16_t start_addr = 0;
    uint16_t block_size = 0;
    
    /* �����z�� */
    if(data == NULL || block >= BLOCK_MAX)
    {
        return 0;
    }
    
    /* �_���^�K����ʼ��ַ�ʹ�С */
    switch(block)
    {
        case BLOCK_BASIC_SYSTEM:
            start_addr = 0;
            block_size = 20;
            break;
        case BLOCK_HARDWARE_CALIB:
            start_addr = 20;
            block_size = 19;
            break;
        case BLOCK_BG_PARAMS:
            start_addr = 39;
            block_size = 174;
            break;
        case BLOCK_U_PARAMS:
            start_addr = 213;
            block_size = 142;
            break;
        case BLOCK_C_PARAMS:
            start_addr = 355;
            block_size = 142;
            break;
        case BLOCK_TG_PARAMS:
            start_addr = 497;
            block_size = 142;
            break;
        case BLOCK_RESERVED:
            start_addr = 639;
            block_size = 39;
            break;
        default:
            return 0;
    }
    
    /* �z�锵����С�Ƿ��^�^�K��С */
    if(size > block_size)
    {
        size = block_size;
    }
    
    /* ���������� */
    memcpy(&param_table[start_addr], data, size);
    
    /* ����У�� */
    PARAM_UpdateChecksum();
    
    /* ��������ą������浽Flash */
    PARAM_SaveToFlash();
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetStripParameters
 *
 * @brief   ����ԇƬ��ͫ@ȡԇƬ������
 *
 * @param   type - ԇƬ���
 *          ndl - ��ԇƬ�z�yˮƽָ�
 *          udl - ��ʹ��ԇƬ�z�yˮƽָ�
 *          bloodIn - ѪҺ�z�yˮƽָ�
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_GetStripParameters(StripType_TypeDef type, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn)
{
    uint16_t ndl_addr = 0;
    uint16_t udl_addr = 0;
    uint16_t bloodIn_addr = 0;
    
    /* �����z�� */
    if(ndl == NULL || udl == NULL || bloodIn == NULL || type >= STRIP_TYPE_MAX)
    {
        return 0;
    }
    
    /* ����ԇƬ��ʹ_��������ַ */
    switch(type)
    {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            ndl_addr = PARAM_BG_NDL;
            udl_addr = PARAM_BG_UDL;
            bloodIn_addr = PARAM_BG_BLOOD_IN;
            break;
        case STRIP_TYPE_U:
            ndl_addr = PARAM_U_NDL;
            udl_addr = PARAM_U_UDL;
            bloodIn_addr = PARAM_U_BLOOD_IN;
            break;
        case STRIP_TYPE_C:
            ndl_addr = PARAM_C_NDL;
            udl_addr = PARAM_C_UDL;
            bloodIn_addr = PARAM_C_BLOOD_IN;
            break;
        case STRIP_TYPE_TG:
            ndl_addr = PARAM_TG_NDL;
            udl_addr = PARAM_TG_UDL;
            bloodIn_addr = PARAM_TG_BLOOD_IN;
            break;
        default:
            return 0;
    }
    
    /* �xȡ����ֵ */
    *ndl = PARAM_GetWord(ndl_addr);
    *udl = PARAM_GetWord(udl_addr);
    *bloodIn = PARAM_GetWord(bloodIn_addr);
    
    return 1;
}

/*********************************************************************
 * @fn      PARAM_GetTimingParameters
 *
 * @brief   ����ԇƬ��ͫ@ȡ�yԇ�r�򅢔���
 *
 * @param   type - ԇƬ���
 *          tpl - �yԇ�g��ָ�
 *          trd - �xȡ���tָ�
 *          evWidth - ����늉�����ָ�
 *          phase - �r���A��(1��2)
 *
 * @return  �ɹ�����1��ʧ������0
 */
uint8_t PARAM_GetTimingParameters(StripType_TypeDef type, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t phase)
{
    uint16_t tpl_addr = 0;
    uint16_t trd_addr = 0;
    uint16_t evWidth_addr = 0;
    
    /* �����z�� */
    if(tpl == NULL || trd == NULL || evWidth == NULL || 
       type >= STRIP_TYPE_MAX || (phase != 1 && phase != 2))
    {
        return 0;
    }
    
    /* ����ԇƬ��ͺ͕r���A�δ_��������ַ */
    switch(type)
    {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            if(phase == 1)
            {
                tpl_addr = PARAM_BG_TPL1;
                trd_addr = PARAM_BG_TRD1;
                evWidth_addr = PARAM_BG_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_BG_TPL2;
                trd_addr = PARAM_BG_TRD2;
                evWidth_addr = PARAM_BG_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_U:
            if(phase == 1)
            {
                tpl_addr = PARAM_U_TPL1;
                trd_addr = PARAM_U_TRD1;
                evWidth_addr = PARAM_U_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_U_TPL2;
                trd_addr = PARAM_U_TRD2;
                evWidth_addr = PARAM_U_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_C:
            if(phase == 1)
            {
                tpl_addr = PARAM_C_TPL1;
                trd_addr = PARAM_C_TRD1;
                evWidth_addr = PARAM_C_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_C_TPL2;
                trd_addr = PARAM_C_TRD2;
                evWidth_addr = PARAM_C_EVWIDTH2;
            }
            break;
        case STRIP_TYPE_TG:
            if(phase == 1)
            {
                tpl_addr = PARAM_TG_TPL1;
                trd_addr = PARAM_TG_TRD1;
                evWidth_addr = PARAM_TG_EVWIDTH1;
            }
            else
            {
                tpl_addr = PARAM_TG_TPL2;
                trd_addr = PARAM_TG_TRD2;
                evWidth_addr = PARAM_TG_EVWIDTH2;
            }
            break;
        default:
            return 0;
    }
    
    /* �xȡ����ֵ */
    *tpl = PARAM_GetWord(tpl_addr);
    *trd = PARAM_GetWord(trd_addr);
    *evWidth = PARAM_GetWord(evWidth_addr);
    
    return 1;
}