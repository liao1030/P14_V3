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
#include "Calculation.h"  // ����Ѫ��Ӌ�㺯�����^�n
#include "strip_detect.h"  // ����ԇƬ�z�y���P����
#include "system_state.h"  // ����ϵ�y��B���P����

/* �ⲿ׃������ */
extern uint16_t W_ADC;  // ����늘OADCֵ

/*********************************************************************
 * @fn      Get_Battery_Voltage
 *
 * @brief   ȡ��늳�늉�(mV)
 *
 * @return  늳�늉�(mV)
 */
uint16_t Get_Battery_Voltage(void)
{
    // ʹ��ȫ��늳�늉�ֵ
    extern uint16_t g_batteryVoltage;
    
    // ���늳�늉���0������߀δ�յ�CH582F�Ĕ�����������һ��Ĭ�Jֵ
    if (g_batteryVoltage == 0) {
        return 3600; // Ĭ�Jֵ
    }
    
    return g_batteryVoltage;
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
    // �z��ϵ�y��B�����ϵ�y��B�� STATE_MEASURING ��֮��Ġ�B����ʾ�љz�y��ѪҺ
    SystemState_TypeDef currentState = System_GetState();
    
    // ��ϵ�y��B�� STATE_MEASURING �� STATE_RESULT_READY �r�������љz�y��ѪҺ
    if (currentState == STATE_MEASURING || currentState == STATE_RESULT_READY) {
        return 1;  // �љz�y��ѪҺ
    }
    
    return 0;  // δ�z�y��ѪҺ
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
    result->testValue = wGlucose;  // Ѫ�ǜy��ֵ
    // ����ʹ��ԇƬ�z�yģ�M�Į�ǰ��ͣ�����oЧ�tʹ�Å������е�ֵ
    StripType_TypeDef stripType = STRIP_DETECT_GetStripType();
    if (stripType >= STRIP_TYPE_MAX) {
        stripType = (StripType_TypeDef)PARAM_GetByte(PARAM_STRIP_TYPE);
    }
    result->stripType = stripType;
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
    rawData->w1Adv = W_ADC;        // W1ͨ��ADCֵ (���H�y��ֵ)
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
