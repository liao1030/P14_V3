/********************************** (C) COPYRIGHT *******************************
 * File Name          : batt_measure.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2023/09/05
 * Description        : 電池電壓測量模組
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
#define ADC_SAMPLES_NUM       8      // 採樣次數，取平均值以提高精確度

// 電池電壓轉換系數（根據實際電路調整）
// CH582F內置ADC量程為0-2.0V，如果使用分壓電阻外接電池，需要對應的轉換係數
// 例如：如果使用2:1分壓，測量電壓為實際電壓的1/2，則係數為2.0
#define BATT_VOLTAGE_FACTOR   2.0f   // 電壓轉換係數

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint16_t current_battery_voltage = 3000; // 默認3000mV

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      ADC_Calibration
 *
 * @brief   ADC校準函數
 *
 * @return  校準偏差
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
 * @brief   初始化電池電壓測量
 *
 * @return  none
 */
void Batt_MeasureInit(void)
{
    // 初始化ADC用於電池電壓測量
    ADC_InterBATSampInit();
    
    // ADC校準
    ADC_Calibration();
    
    PRINT("Battery Measurement Module Initialized\n");
}

/*********************************************************************
 * @fn      Batt_GetVoltage
 *
 * @brief   測量當前電池電壓
 *
 * @return  電池電壓 (mV)
 */
uint16_t Batt_GetVoltage(void)
{
    uint32_t adc_sum = 0;
    uint16_t adc_value, voltage;
    
    // 初始化ADC進行電池測量
    ADC_InterBATSampInit();
    
    // 進行多次採樣取平均值
    for(uint8_t i = 0; i < ADC_SAMPLES_NUM; i++)
    {
        adc_value = ADC_ExcutSingleConver();
        adc_sum += adc_value;
    }
    
    // 計算平均ADC值
    adc_value = adc_sum / ADC_SAMPLES_NUM;
    
    // 轉換為電壓值 (mV)
    // 在PGA_1_4模式下，有效量程為0-4V
    // 計算公式: (ADC/512-3)*Vref*5，Vref約為1.05V
    voltage = (uint16_t)((adc_value / 512.0f - 3.0f) * 1050.0f * 5);
    
    // 存儲當前電池電壓
    current_battery_voltage = voltage;
    
    return voltage;
}

/*********************************************************************
 * @fn      Batt_GetStatus
 *
 * @brief   獲取電池狀態
 *
 * @return  電池狀態 (BATT_STATUS_LOW/BATT_STATUS_NORMAL/BATT_STATUS_CHARGING)
 */
uint8_t Batt_GetStatus(void)
{
    uint16_t voltage = Batt_GetVoltage();
    
    // 判斷電池狀態
    if(voltage < BATT_VOLTAGE_LOW)
    {
        return BATT_STATUS_LOW;
    }
    else
    {
        return BATT_STATUS_NORMAL;
    }
    
    // 注：充電狀態需要接收充電檢測引腳，此處未實現
}

/*********************************************************************
*********************************************************************/
