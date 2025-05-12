/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol_stubs.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/12
 * Description        : �๦�������yԇ�xUARTͨӍ�f�h�������
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "uart_protocol.h"
#include "debug.h"
#include "param_table.h"
#include <string.h>

/*********************************************************************
 * @fn      Get_Battery_Voltage
 *
 * @brief   ȡ��늳�늉�(mV)
 *
 * @return  늳�늉�(mV)
 */
uint16_t Get_Battery_Voltage(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫͨ�^ADC�xȡ늳�늉�
    // ���O늳�늉���3600mV
    return 3600;
}

/*********************************************************************
 * @fn      Check_Battery_Status
 *
 * @brief   �z��늳ؠ�B
 *
 * @return  0: ����, ��0: �e�`���a
 */
uint8_t Check_Battery_Status(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫ�z��늳����
    // ���O늳�늉��ֵ��3000mV
    uint16_t batteryVoltage = Get_Battery_Voltage();
    
    if(batteryVoltage < 3000)
        return ERR_BATTERY_LOW;
    
    return 0;
}

/*********************************************************************
 * @fn      Get_Temperature
 *
 * @brief   ȡ�íh���ض�(�Ŵ�10��)
 *
 * @return  �h���ض�(�Ŵ�10��)
 */
uint16_t Get_Temperature(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫͨ�^�ضȸМy���xȡ�ض�
    // ���O�h���ضȞ�25.6��C������256
    return 256;
}

/*********************************************************************
 * @fn      Check_Strip_Status
 *
 * @brief   �z��ԇƬ��B
 *
 * @return  0: ����, ��0: �e�`���a
 */
uint8_t Check_Strip_Status(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫ�z��ԇƬ��B
    // ���OԇƬ��B����
    return 0;
}

/*********************************************************************
 * @fn      Check_Blood_Status
 *
 * @brief   �z��ѪҺ�ӱ���B
 *
 * @return  0: δ�z�y��ѪҺ, 1: �љz�y��ѪҺ
 */
uint8_t Check_Blood_Status(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫ�z���Ƿ��љz�y��ѪҺ�ӱ�
    // ���Oδ�z�y��ѪҺ
    static uint8_t counter = 0;
    
    // ģ�MѪҺ�z�y��ÿ10�κ��з���һ���љz�y��ѪҺ
    counter++;
    if(counter >= 10)
    {
        counter = 0;
        return 1;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      Perform_Test
 *
 * @brief   ���Мyԇ
 *
 * @return  0: �ɹ�, ��0: �e�`���a
 */
uint8_t Perform_Test(void)
{
    // �@�Ǵ�����������Hʹ�Õr��Ҫ���Мyԇ߉݋
    // ���O�yԇ�ɹ�
    return 0;
}

/*********************************************************************
 * @fn      Get_Test_Data
 *
 * @brief   ȡ�Üyԇ�Y��
 *
 * @param   result - �yԇ�Y���Y��ָ��
 *
 * @return  0: ʧ��, 1: �ɹ�
 */
uint8_t Get_Test_Data(TestResult_TypeDef *result)
{
    if(result == NULL)
        return 0;
    
    // �@�Ǵ�����������Hʹ�Õr��Ҫ����挍�Ĝyԇ�Y��
    result->resultStatus = 0; // �ɹ�
    result->testValue = 123;  // �yԇֵ(����123 mg/dL)
    result->stripType = PARAM_GetByte(PARAM_STRIP_TYPE);
    result->eventType = PARAM_GetByte(PARAM_EVENT);
    result->stripCode = PARAM_GetWord(PARAM_CODE_TABLE_V);
    
    // �@ȡ��ǰ�r�g
    uint8_t year, month, date, hour, minute, second;
    PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second);
    
    result->year = 2000 + year;
    result->month = month;
    result->date = date;
    result->hour = hour;
    result->minute = minute;
    result->second = second;
    
    result->batteryVoltage = Get_Battery_Voltage();
    result->temperature = Get_Temperature();
    
    return 1;
}

/*********************************************************************
 * @fn      Get_Raw_Test_Data
 *
 * @brief   ȡ��ԭʼ�yԇ�Y��
 *
 * @param   rawData - ԭʼ�Y�ϽY��ָ��
 *
 * @return  0: ʧ��, 1: �ɹ�
 */
uint8_t Get_Raw_Test_Data(RawData_TypeDef *rawData)
{
    if(rawData == NULL)
        return 0;
    
    // �@�Ǵ�����������Hʹ�Õr��Ҫ����挍��ԭʼ�yԇ�Y��
    TestResult_TypeDef result;
    Get_Test_Data(&result);
    
    rawData->resultStatus = result.resultStatus;
    rawData->testValue = result.testValue;
    rawData->year = result.year;
    rawData->month = result.month;
    rawData->date = result.date;
    rawData->hour = result.hour;
    rawData->minute = result.minute;
    rawData->second = result.second;
    rawData->stripType = result.stripType;
    rawData->eventType = result.eventType;
    rawData->batteryVoltage = result.batteryVoltage;
    rawData->temperature = result.temperature;
    
    // �������RAW DATA���Ù�λ
    rawData->w1Adv = 1000;         // W1ͨ��ADCֵ
    rawData->w2Adv = 1000;         // W2ͨ��ADCֵ
    rawData->t1Ampl = 1000;        // T1�ضȸМy���
    rawData->t1AcMax = 2000;       // T1 AC���ֵ
    rawData->t1AcMin = 1000;       // T1 AC��Сֵ
    rawData->t3Adc = 800;          // T3ͨ��ADCֵ
    rawData->testCount = PARAM_GetWord(PARAM_NOT); // �yԇ�Δ�
    rawData->stripCode = result.stripCode;
    
    // ���ASCII���a�Ĳ�����ID�͈D��̖
    memcpy(rawData->operatorId, "A123456789", 10);
    memcpy(rawData->chartNo, "A123456789", 10);
    
    return 1;
}
