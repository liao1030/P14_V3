/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_code.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x�������a�팍�F
 *********************************************************************************
 * Copyright (c) 2025 HMD.
 *******************************************************************************/

#include "parameter_code.h"
#include "debug.h"
#include "string.h"

/* ȫ��׃�� */
static uint8_t g_ParameterTable[PARAM_TABLE_SIZE]; // ���������
static uint8_t g_ParameterInitialized = 0;         // �������ʼ�����I

/* �A�Oֵ���x */
#define DEFAULT_LBT          58  // Ĭ�J��늳��ֵ (2.58V)
#define DEFAULT_OBT          55  // Ĭ�J늳غıM�ֵ (2.55V)
#define DEFAULT_FACTORY      0   // Ĭ�J��ʹ����ģʽ
#define DEFAULT_MODEL_NO     1   // Ĭ�J�aƷ��̖
#define DEFAULT_FW_NO        1   // Ĭ�J�g�w�汾
#define DEFAULT_NOT          0   // Ĭ�J�y���Δ�
#define DEFAULT_CODE_TABLE_V 1   // Ĭ�J���a��汾

#define DEFAULT_YEAR         25  // Ĭ�J��� (2025)
#define DEFAULT_MONTH        5   // Ĭ�J�·� (5��)
#define DEFAULT_DATE         2   // Ĭ�J���� (2��)
#define DEFAULT_HOUR         12  // Ĭ�JС�r
#define DEFAULT_MINUTE       0   // Ĭ�J���
#define DEFAULT_SECOND       0   // Ĭ�J�딵

#define DEFAULT_TLL          1   // Ĭ�J�����ض����� (1.0��C)
#define DEFAULT_TLH          40  // Ĭ�J�����ض����� (40.0��C)
#define DEFAULT_MGDL         0   // Ĭ�J�y����λ (mmol/L)
#define DEFAULT_EVENT        0   // Ĭ�J�¼���� (QC)
#define DEFAULT_STRIP_TYPE   0   // Ĭ�J�y���Ŀ (Ѫ��)

#define DEFAULT_EV_T3_TRG    150 // Ĭ�JEV_T3�|�l�λ (1500mV)
#define DEFAULT_EV_WORKING   164 // Ĭ�J�y�������λ늉�
#define DEFAULT_EV_T3        164 // Ĭ�JѪҺ�����λ늉�
#define DEFAULT_DACO         0   // Ĭ�JDACƫ��
#define DEFAULT_DACDO        0   // Ĭ�JDACУ�ʹ��텢��
#define DEFAULT_CC211NoDone  0   // Ĭ�JCC211����
#define DEFAULT_CAL_TOL      8   // Ĭ�JУ���ݲ� (0.8%)

/*********************************************************************
 * @fn      Parameter_Init
 *
 * @brief   ��ʼ��������
 *
 * @return  none
 */
void Parameter_Init(void)
{
    if(g_ParameterInitialized == 0)
    {
        // �Lԇ��Flash��EEPROM�d�녢��
        // ���ʧ���t�O��Ĭ�Jֵ
        Parameter_Load();
        
        // �z��У��
        if(Parameter_CheckSum() != 0)
        {
            printf("Parameter checksum error, reset to default values\r\n");
            Parameter_SetDefaultValues();
            Parameter_Save();
        }
        
        g_ParameterInitialized = 1;
    }
}

/*********************************************************************
 * @fn      Parameter_Read
 *
 * @brief   �xȡָ����ַ�ą���(8λ)
 *
 * @param   addr - ������ַ
 *
 * @return  ����ֵ
 */
uint8_t Parameter_Read(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        return g_ParameterTable[addr];
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write
 *
 * @brief   ����ָ����ַ�ą���(8λ)
 *
 * @param   addr - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void Parameter_Write(uint16_t addr, uint8_t value)
{
    if(addr < PARAM_TABLE_SIZE)
    {
        g_ParameterTable[addr] = value;
    }
}

/*********************************************************************
 * @fn      Parameter_Read16
 *
 * @brief   �xȡָ����ַ�ą���(16λ)
 *
 * @param   addr - ������ַ
 *
 * @return  ����ֵ
 */
uint16_t Parameter_Read16(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE - 1)
    {
        return (uint16_t)g_ParameterTable[addr] | 
               ((uint16_t)g_ParameterTable[addr + 1] << 8);
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write16
 *
 * @brief   ����ָ����ַ�ą���(16λ)
 *
 * @param   addr - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void Parameter_Write16(uint16_t addr, uint16_t value)
{
    if(addr < PARAM_TABLE_SIZE - 1)
    {
        g_ParameterTable[addr] = (uint8_t)(value & 0xFF);
        g_ParameterTable[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
    }
}

/*********************************************************************
 * @fn      Parameter_Read32
 *
 * @brief   �xȡָ����ַ�ą���(32λ)
 *
 * @param   addr - ������ַ
 *
 * @return  ����ֵ
 */
uint32_t Parameter_Read32(uint16_t addr)
{
    if(addr < PARAM_TABLE_SIZE - 3)
    {
        return (uint32_t)g_ParameterTable[addr] | 
               ((uint32_t)g_ParameterTable[addr + 1] << 8) |
               ((uint32_t)g_ParameterTable[addr + 2] << 16) |
               ((uint32_t)g_ParameterTable[addr + 3] << 24);
    }
    return 0;
}

/*********************************************************************
 * @fn      Parameter_Write32
 *
 * @brief   ����ָ����ַ�ą���(32λ)
 *
 * @param   addr - ������ַ
 * @param   value - ����ֵ
 *
 * @return  none
 */
void Parameter_Write32(uint16_t addr, uint32_t value)
{
    if(addr < PARAM_TABLE_SIZE - 3)
    {
        g_ParameterTable[addr] = (uint8_t)(value & 0xFF);
        g_ParameterTable[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
        g_ParameterTable[addr + 2] = (uint8_t)((value >> 16) & 0xFF);
        g_ParameterTable[addr + 3] = (uint8_t)((value >> 24) & 0xFF);
    }
}

/*********************************************************************
 * @fn      Parameter_CheckSum
 *
 * @brief   Ӌ�ㅢ��У��
 *
 * @return  У�Y�� (0:���_, ��0:�e�`)
 */
uint8_t Parameter_CheckSum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    // Ӌ���ַ0~674�Ŀ���
    for(i = 0; i < 675; i++)
    {
        sum += g_ParameterTable[i];
    }
    
    // �z��У��
    if((uint8_t)(sum & 0xFF) == g_ParameterTable[675] &&
       (uint8_t)((sum >> 8) & 0xFF) == g_ParameterTable[676])
    {
        return 0;  // У����_
    }
    
    return 1;  // У��e�`
}

/*********************************************************************
 * @fn      Parameter_UpdateCheckSum
 *
 * @brief   ��������У��
 *
 * @return  none
 */
static void Parameter_UpdateCheckSum(void)
{
    uint16_t sum = 0;
    uint16_t i;
    
    // Ӌ���ַ0~674�Ŀ���
    for(i = 0; i < 675; i++)
    {
        sum += g_ParameterTable[i];
    }
    
    // ����У��
    g_ParameterTable[675] = (uint8_t)(sum & 0xFF);         // SUM_L
    g_ParameterTable[676] = (uint8_t)((sum >> 8) & 0xFF);  // SUM_H
    
    // ����CRC16 (���H���F��������w�㷨Ӌ��CRC16)
    g_ParameterTable[677] = 0;  // ���r�Þ�0�����m���F
}

/*********************************************************************
 * @fn      Parameter_SetDefaultValues
 *
 * @brief   �O�Å�����Ĭ�Jֵ
 *
 * @return  none
 */
void Parameter_SetDefaultValues(void)
{
    // ��Յ�����
    memset(g_ParameterTable, 0, PARAM_TABLE_SIZE);
    
    // 1. ϵ�y��������
    Parameter_Write(PARAM_LBT, DEFAULT_LBT);
    Parameter_Write(PARAM_OBT, DEFAULT_OBT);
    Parameter_Write(PARAM_FACTORY, DEFAULT_FACTORY);
    Parameter_Write(PARAM_MODEL_NO, DEFAULT_MODEL_NO);
    Parameter_Write(PARAM_FW_NO, DEFAULT_FW_NO);
    Parameter_Write16(PARAM_NOT, DEFAULT_NOT);
    Parameter_Write16(PARAM_CODE_TABLE_V, DEFAULT_CODE_TABLE_V);
    
    // 2. �r�g�O������
    Parameter_Write(PARAM_YEAR, DEFAULT_YEAR);
    Parameter_Write(PARAM_MONTH, DEFAULT_MONTH);
    Parameter_Write(PARAM_DATE, DEFAULT_DATE);
    Parameter_Write(PARAM_HOUR, DEFAULT_HOUR);
    Parameter_Write(PARAM_MINUTE, DEFAULT_MINUTE);
    Parameter_Write(PARAM_SECOND, DEFAULT_SECOND);
    
    // 3. �y���@ʾ����
    Parameter_Write(PARAM_TLL, DEFAULT_TLL);
    Parameter_Write(PARAM_TLH, DEFAULT_TLH);
    Parameter_Write(PARAM_MGDL, DEFAULT_MGDL);
    Parameter_Write(PARAM_EVENT, DEFAULT_EVENT);
    Parameter_Write(PARAM_STRIP_TYPE, DEFAULT_STRIP_TYPE);
    
    // 4. Ӳ�wУ�ʅ���
    Parameter_Write16(PARAM_EV_T3_TRG, DEFAULT_EV_T3_TRG);
    Parameter_Write(PARAM_EV_WORKING, DEFAULT_EV_WORKING);
    Parameter_Write(PARAM_EV_T3, DEFAULT_EV_T3);
    Parameter_Write(PARAM_DACO, DEFAULT_DACO);
    Parameter_Write(PARAM_DACDO, DEFAULT_DACDO);
    Parameter_Write(PARAM_CC211NoDone, DEFAULT_CC211NoDone);
    Parameter_Write(PARAM_CAL_TOL, DEFAULT_CAL_TOL);
    
    // ����У��
    Parameter_UpdateCheckSum();
}

/*********************************************************************
 * @fn      Parameter_Save
 *
 * @brief   ���慢����Flash��EEPROM
 *
 * @return  none
 */
void Parameter_Save(void)
{
    // ����У��
    Parameter_UpdateCheckSum();
    
    // TODO: ���FFlash��EEPROM����
    // ��̎��Ҫ�������HӲ�w��ӱ��慢����Ĵ��a
    printf("Parameter saved\r\n");
}

/*********************************************************************
 * @fn      Parameter_Load
 *
 * @brief   ��Flash��EEPROM�d�녢��
 *
 * @return  none
 */
void Parameter_Load(void)
{
    // TODO: ���FFlash��EEPROM�xȡ
    // ��̎��Ҫ�������HӲ�w����xȡ������Ĵ��a
    
    // ����o���xȡ���t�O��Ĭ�Jֵ
    Parameter_SetDefaultValues();
    printf("Parameter loaded\r\n");
}

/*********************************************************************
 * @fn      Parameter_Reset
 *
 * @brief   ���Å������Ĭ�Jֵ
 *
 * @return  none
 */
void Parameter_Reset(void)
{
    Parameter_SetDefaultValues();
    Parameter_Save();
    printf("Parameter reset to default values\r\n");
}

/*********************************************************************
 * @fn      Parameter_GetStripTypeStr
 *
 * @brief   �@ȡԇƬ��͵��ַ�������
 *
 * @param   type - ԇƬ���
 *
 * @return  �ַ�������
 */
const char* Parameter_GetStripTypeStr(uint8_t type)
{
    switch(type)
    {
        case STRIP_TYPE_GLV:
            return "GLV";
        case STRIP_TYPE_U:
            return "U";
        case STRIP_TYPE_C:
            return "C";
        case STRIP_TYPE_TG:
            return "TG";
        case STRIP_TYPE_GAV:
            return "GAV";
        default:
            return "Unknown";
    }
}

/*********************************************************************
 * @fn      Parameter_PrintInfo
 *
 * @brief   ��ӡ��������Ϣ
 *
 * @return  none
 */
void Parameter_PrintInfo(void)
{
    printf("==== Parameter Table Info ====\r\n");
    
    // 1. ϵ�y��������
    printf("LBT: %d (%.2fV)\r\n", Parameter_Read(PARAM_LBT), Parameter_Read(PARAM_LBT) * 0.05);
    printf("OBT: %d (%.2fV)\r\n", Parameter_Read(PARAM_OBT), Parameter_Read(PARAM_OBT) * 0.05);
    printf("Factory Mode: %d\r\n", Parameter_Read(PARAM_FACTORY));
    printf("Model NO: %d\r\n", Parameter_Read(PARAM_MODEL_NO));
    printf("FW Version: %d\r\n", Parameter_Read(PARAM_FW_NO));
    printf("Test Count: %d\r\n", Parameter_Read16(PARAM_NOT));
    printf("Code Table Version: %d\r\n", Parameter_Read16(PARAM_CODE_TABLE_V));
    
    // 2. �r�g�O������
    printf("Date: 20%02d-%02d-%02d\r\n", 
           Parameter_Read(PARAM_YEAR),
           Parameter_Read(PARAM_MONTH),
           Parameter_Read(PARAM_DATE));
    printf("Time: %02d:%02d:%02d\r\n", 
           Parameter_Read(PARAM_HOUR),
           Parameter_Read(PARAM_MINUTE),
           Parameter_Read(PARAM_SECOND));
    
    // 3. �y���@ʾ����
    printf("Temperature Range: %d.0��C ~ %d.0��C\r\n", 
           Parameter_Read(PARAM_TLL),
           Parameter_Read(PARAM_TLH));
    
    const char* unit_str[3] = {"mmol/L", "mg/dL", "gm/dl"};
    uint8_t unit = Parameter_Read(PARAM_MGDL);
    if(unit > 2) unit = 0;
    
    printf("Measurement Unit: %s\r\n", unit_str[unit]);
    
    const char* event_str[3] = {"QC", "AC", "PC"};
    uint8_t event = Parameter_Read(PARAM_EVENT);
    if(event > 2) event = 0;
    
    printf("Event Type: %s\r\n", event_str[event]);
    printf("Strip Type: %s\r\n", Parameter_GetStripTypeStr(Parameter_Read(PARAM_STRIP_TYPE)));
    
    printf("============================\r\n");
}