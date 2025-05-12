/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol_stubs.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/12
 * Description        : 多功能生化測試儀UART通訊協議存根函數
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
 * @brief   取得電池電壓(mV)
 *
 * @return  電池電壓(mV)
 */
uint16_t Get_Battery_Voltage(void)
{
    // 這是存根函數，實際使用時需要通過ADC讀取電池電壓
    // 假設電池電壓為3600mV
    return 3600;
}

/*********************************************************************
 * @fn      Check_Battery_Status
 *
 * @brief   檢查電池狀態
 *
 * @return  0: 正常, 非0: 錯誤代碼
 */
uint8_t Check_Battery_Status(void)
{
    // 這是存根函數，實際使用時需要檢查電池電量
    // 假設電池電壓閾值為3000mV
    uint16_t batteryVoltage = Get_Battery_Voltage();
    
    if(batteryVoltage < 3000)
        return ERR_BATTERY_LOW;
    
    return 0;
}

/*********************************************************************
 * @fn      Get_Temperature
 *
 * @brief   取得環境溫度(放大10倍)
 *
 * @return  環境溫度(放大10倍)
 */
uint16_t Get_Temperature(void)
{
    // 這是存根函數，實際使用時需要通過溫度感測器讀取溫度
    // 假設環境溫度為25.6度C，返回256
    return 256;
}

/*********************************************************************
 * @fn      Check_Strip_Status
 *
 * @brief   檢查試片狀態
 *
 * @return  0: 正常, 非0: 錯誤代碼
 */
uint8_t Check_Strip_Status(void)
{
    // 這是存根函數，實際使用時需要檢查試片狀態
    // 假設試片狀態正常
    return 0;
}

/*********************************************************************
 * @fn      Check_Blood_Status
 *
 * @brief   檢查血液樣本狀態
 *
 * @return  0: 未檢測到血液, 1: 已檢測到血液
 */
uint8_t Check_Blood_Status(void)
{
    // 這是存根函數，實際使用時需要檢查是否已檢測到血液樣本
    // 假設未檢測到血液
    static uint8_t counter = 0;
    
    // 模擬血液檢測，每10次呼叫返回一次已檢測到血液
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
 * @brief   執行測試
 *
 * @return  0: 成功, 非0: 錯誤代碼
 */
uint8_t Perform_Test(void)
{
    // 這是存根函數，實際使用時需要執行測試邏輯
    // 假設測試成功
    return 0;
}

/*********************************************************************
 * @fn      Get_Test_Data
 *
 * @brief   取得測試資料
 *
 * @param   result - 測試結果結構指標
 *
 * @return  0: 失敗, 1: 成功
 */
uint8_t Get_Test_Data(TestResult_TypeDef *result)
{
    if(result == NULL)
        return 0;
    
    // 這是存根函數，實際使用時需要填充真實的測試資料
    result->resultStatus = 0; // 成功
    result->testValue = 123;  // 測試值(例如123 mg/dL)
    result->stripType = PARAM_GetByte(PARAM_STRIP_TYPE);
    result->eventType = PARAM_GetByte(PARAM_EVENT);
    result->stripCode = PARAM_GetWord(PARAM_CODE_TABLE_V);
    
    // 獲取當前時間
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
 * @brief   取得原始測試資料
 *
 * @param   rawData - 原始資料結構指標
 *
 * @return  0: 失敗, 1: 成功
 */
uint8_t Get_Raw_Test_Data(RawData_TypeDef *rawData)
{
    if(rawData == NULL)
        return 0;
    
    // 這是存根函數，實際使用時需要填充真實的原始測試資料
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
    
    // 填充其他RAW DATA專用欄位
    rawData->w1Adv = 1000;         // W1通道ADC值
    rawData->w2Adv = 1000;         // W2通道ADC值
    rawData->t1Ampl = 1000;        // T1溫度感測振幅
    rawData->t1AcMax = 2000;       // T1 AC最大值
    rawData->t1AcMin = 1000;       // T1 AC最小值
    rawData->t3Adc = 800;          // T3通道ADC值
    rawData->testCount = PARAM_GetWord(PARAM_NOT); // 測試次數
    rawData->stripCode = result.stripCode;
    
    // 填充ASCII編碼的操作者ID和圖表編號
    memcpy(rawData->operatorId, "A123456789", 10);
    memcpy(rawData->chartNo, "A123456789", 10);
    
    return 1;
}
