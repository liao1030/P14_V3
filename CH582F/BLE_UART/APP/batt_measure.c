/********************************** (C) COPYRIGHT *******************************
 * File Name          : batt_measure.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2023/09/05
 * Description        : 늳�늉��y��ģ�M
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "batt_measure.h"
#include "CH58x_common.h"
#include "CONFIG.h"

/*********************************************************************
 * MACROS
 */
#define ADC_SAMPLES_NUM       8      // ��ӴΔ���ȡƽ��ֵ����߾��_��

// 늳�늉��D�Qϵ�����������H�·�{����
// CH582F����ADC���̞�0-2.0V�����ʹ�÷։�������늳أ���Ҫ�������D�Q�S��
// ���磺���ʹ��2:1�։����y��늉��錍�H늉���1/2���t�S����2.0
#define BATT_VOLTAGE_FACTOR   2.0f   // 늉��D�Q�S��

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint16_t current_battery_voltage = 3000; // Ĭ�J3000mV

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      ADC_Calibration
 *
 * @brief   ADCУ�ʺ���
 *
 * @return  У��ƫ��
 */
static signed short ADC_Calibration(void)
{
    return ADC_DataCalib_Rough();
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Batt_MeasureInit
 *
 * @brief   ��ʼ��늳�늉��y��
 *
 * @return  none
 */
void Batt_MeasureInit(void)
{
    // ��ʼ��ADC���늳�늉��y��
    ADC_InterBATSampInit();
    
    // ADCУ��
    ADC_Calibration();
    
    PRINT("Battery Measurement Module Initialized\n");
}

/*********************************************************************
 * @fn      Batt_GetVoltage
 *
 * @brief   �y����ǰ늳�늉�
 *
 * @return  늳�늉� (mV)
 */
uint16_t Batt_GetVoltage(void)
{
    uint32_t adc_sum = 0;
    uint16_t adc_value, voltage;
    
    // ��ʼ��ADC�M��늳؜y��
    ADC_InterBATSampInit();
    
    // �M�ж�Β��ȡƽ��ֵ
    for(uint8_t i = 0; i < ADC_SAMPLES_NUM; i++)
    {
        adc_value = ADC_ExcutSingleConver();
        adc_sum += adc_value;
    }
    
    // Ӌ��ƽ��ADCֵ
    adc_value = adc_sum / ADC_SAMPLES_NUM;
    
    // �D�Q��늉�ֵ (mV)
    // ��PGA_1_4ģʽ�£���Ч���̞�0-4V
    // Ӌ�㹫ʽ: (ADC/512-3)*Vref*5��Vref�s��1.05V
    voltage = (uint16_t)((adc_value / 512.0f - 3.0f) * 1050.0f * 5);
    
    // �惦��ǰ늳�늉�
    current_battery_voltage = voltage;
    
    return voltage;
}

/*********************************************************************
 * @fn      Batt_GetStatus
 *
 * @brief   �@ȡ늳ؠ�B
 *
 * @return  늳ؠ�B (BATT_STATUS_LOW/BATT_STATUS_NORMAL/BATT_STATUS_CHARGING)
 */
uint8_t Batt_GetStatus(void)
{
    uint16_t voltage = Batt_GetVoltage();
    
    // �Д�늳ؠ�B
    if(voltage < BATT_VOLTAGE_LOW)
    {
        return BATT_STATUS_LOW;
    }
    else
    {
        return BATT_STATUS_NORMAL;
    }
    
    // ע����늠�B��Ҫ���ճ�늙z�y���_����̎δ���F
}

/*********************************************************************
*********************************************************************/
