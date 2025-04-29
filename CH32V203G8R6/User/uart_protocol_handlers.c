/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol_handlers.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UART通訊協議命令處理實現
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "uart_protocol.h"
#include <string.h>
#include "debug.h"

/* 外部引用的全局變量 */
extern RTC_TimeStruct_t rtc_time;
extern Device_Status_t device_status;
extern Measure_Result_t measure_result;
extern Measure_Param_t measure_param;
extern uint8_t raw_data[];
extern uint16_t raw_data_length;
extern uint8_t blood_sampling_active;
extern uint8_t blood_countdown_seconds;

/* 測量模擬數據 */
static uint16_t simulate_measure_value = 100;

/*********************************************************************
 * @fn      UART_Protocol_HandleSyncTime
 *
 * @brief   處理同步時間指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleSyncTime(uint8_t *data, uint8_t length)
{
    uint8_t response_data = 0x00; // 成功
    
    /* 檢查數據長度 */
    if (length != 7) {
        UART_Protocol_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* 解析時間數據 */
    rtc_time.year = (data[0] << 8) | data[1];
    rtc_time.month = data[2];
    rtc_time.day = data[3];
    rtc_time.hour = data[4];
    rtc_time.minute = data[5];
    rtc_time.second = data[6];
    
    printf("Time synced: %04d-%02d-%02d %02d:%02d:%02d\r\n", 
           rtc_time.year, rtc_time.month, rtc_time.day, 
           rtc_time.hour, rtc_time.minute, rtc_time.second);
    
    /* 更新RTC時間 (實際應用中需要調用RTC設置函數) */
    // TODO: 設置系統RTC時間
    
    /* 發送成功響應 */
    UART_Protocol_SendFrame(CMD_SYNC_TIME_ACK, &response_data, 1);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestStatus
 *
 * @brief   處理請求設備狀態指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length)
{
    uint8_t response_data[8];
    
    /* 更新設備狀態 (實際應用中需要從硬件獲取) */
    device_status.battery_voltage = 3000; // 示例: 3.0V
    device_status.temperature = 250;      // 示例: 25.0°C
    
    /* 組裝狀態數據 */
    response_data[0] = (device_status.measure_type >> 8) & 0xFF;
    response_data[1] = device_status.measure_type & 0xFF;
    response_data[2] = (device_status.strip_status >> 8) & 0xFF;
    response_data[3] = device_status.strip_status & 0xFF;
    response_data[4] = (device_status.battery_voltage >> 8) & 0xFF;
    response_data[5] = device_status.battery_voltage & 0xFF;
    response_data[6] = (device_status.temperature >> 8) & 0xFF;
    response_data[7] = device_status.temperature & 0xFF;
    
    printf("Status requested - Type: %04X, Strip: %04X, Battery: %dmV, Temp: %.1f°C\r\n", 
           device_status.measure_type, device_status.strip_status, 
           device_status.battery_voltage, device_status.temperature / 10.0);
    
    /* 發送響應 */
    UART_Protocol_SendFrame(CMD_STATUS_RESPONSE, response_data, 8);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleSetCodeEvent
 *
 * @brief   處理設置CODE和EVENT指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length)
{
    uint8_t response_data = 0x00; // 成功
    
    /* 檢查數據長度 */
    if (length != 3) {
        UART_Protocol_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    /* 解析CODE和EVENT */
    measure_param.code = data[0];
    measure_param.event = (data[1] << 8) | data[2];
    
    printf("Set CODE=%d, EVENT=%04X\r\n", measure_param.code, measure_param.event);
    
    /* 發送成功響應 */
    UART_Protocol_SendFrame(CMD_CODE_EVENT_ACK, &response_data, 1);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleBloodSampleCheck
 *
 * @brief   處理檢查血樣狀態指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length)
{
    /* 檢查是否有試片插入 (實際應用需要檢測硬件狀態) */
    if (device_status.strip_status != 0) {
        /* 試片插入錯誤 */
        UART_Protocol_SendErrorResponse(CMD_BLOOD_SAMPLE_CHECK, ERR_STRIP_INSERT);
        return;
    }
    
    /* 模擬檢測到血液樣本 */
    blood_sampling_active = 1;
    blood_countdown_seconds = 5; // 5秒倒計時
    
    printf("Blood sample check, countdown: %d seconds\r\n", blood_countdown_seconds);
    
    /* 發送血液通知 */
    UART_Protocol_SendBloodSampleNotify(blood_countdown_seconds);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestResult
 *
 * @brief   處理請求測量結果指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleRequestResult(uint8_t *data, uint8_t length)
{
    /* 如果還在採血或測量過程中 */
    if (blood_sampling_active) {
        UART_Protocol_SendErrorResponse(CMD_REQUEST_RESULT, ERR_MEASURE_TIMEOUT);
        return;
    }
    
    /* 模擬生成測量結果 */
    measure_result.result_status = 0x0000; // 成功
    measure_result.measure_value = simulate_measure_value; // 模擬值
    measure_result.measure_type = device_status.measure_type;
    measure_result.event_type = measure_param.event;
    measure_result.code = measure_param.code;
    
    /* 複製當前時間 */
    measure_result.time = rtc_time;
    
    /* 複製其他參數 */
    measure_result.battery_voltage = device_status.battery_voltage;
    measure_result.temperature = device_status.temperature;
    
    /* 隨機增加測量值以模擬變化 */
    simulate_measure_value += 10;
    if (simulate_measure_value > 200) {
        simulate_measure_value = 100;
    }
    
    printf("Measurement result: Value=%d, Type=%04X, Event=%04X\r\n", 
           measure_result.measure_value, measure_result.measure_type, measure_result.event_type);
    
    /* 發送結果 */
    UART_Protocol_SendResult(&measure_result);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestRawData
 *
 * @brief   處理請求RAW DATA指令
 *
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length)
{
    uint16_t i;
    
    /* 模擬生成RAW數據 */
    raw_data_length = 30; // 示例長度
    
    /* 生成一些示例數據 */
    for (i = 0; i < raw_data_length; i++) {
        raw_data[i] = i + 0x10;
    }
    
    printf("Raw data requested, length=%d\r\n", raw_data_length);
    
    /* 發送RAW數據 */
    UART_Protocol_SendRawData(raw_data, raw_data_length);
}

/*********************************************************************
 * @fn      UART_Protocol_SendBloodSampleNotify
 *
 * @brief   發送血液樣本通知
 *
 * @param   countdown_seconds - 倒計時秒數
 *
 * @return  none
 */
void UART_Protocol_SendBloodSampleNotify(uint8_t countdown_seconds)
{
    UART_Protocol_SendFrame(CMD_BLOOD_SAMPLE_NOTIFY, &countdown_seconds, 1);
}

/*********************************************************************
 * @fn      UART_Protocol_SendStatus
 *
 * @brief   發送設備狀態
 *
 * @param   status - 設備狀態結構指針
 *
 * @return  none
 */
void UART_Protocol_SendStatus(Device_Status_t *status)
{
    uint8_t response_data[8];
    
    /* 組裝狀態數據 */
    response_data[0] = (status->measure_type >> 8) & 0xFF;
    response_data[1] = status->measure_type & 0xFF;
    response_data[2] = (status->strip_status >> 8) & 0xFF;
    response_data[3] = status->strip_status & 0xFF;
    response_data[4] = (status->battery_voltage >> 8) & 0xFF;
    response_data[5] = status->battery_voltage & 0xFF;
    response_data[6] = (status->temperature >> 8) & 0xFF;
    response_data[7] = status->temperature & 0xFF;
    
    /* 發送狀態 */
    UART_Protocol_SendFrame(CMD_STATUS_RESPONSE, response_data, 8);
}

/*********************************************************************
 * @fn      UART_Protocol_SendResult
 *
 * @brief   發送測量結果
 *
 * @param   result - 測量結果結構指針
 *
 * @return  none
 */
void UART_Protocol_SendResult(Measure_Result_t *result)
{
    uint8_t response_data[25];
    uint8_t index = 0;
    
    /* 組裝結果數據 */
    response_data[index++] = (result->result_status >> 8) & 0xFF;
    response_data[index++] = result->result_status & 0xFF;
    response_data[index++] = (result->measure_value >> 8) & 0xFF;
    response_data[index++] = result->measure_value & 0xFF;
    response_data[index++] = (result->measure_type >> 8) & 0xFF;
    response_data[index++] = result->measure_type & 0xFF;
    response_data[index++] = (result->event_type >> 8) & 0xFF;
    response_data[index++] = result->event_type & 0xFF;
    response_data[index++] = result->code;
    
    /* 時間數據 */
    response_data[index++] = (result->time.year >> 8) & 0xFF;
    response_data[index++] = result->time.year & 0xFF;
    response_data[index++] = (result->time.month >> 8) & 0xFF;
    response_data[index++] = result->time.month & 0xFF;
    response_data[index++] = (result->time.day >> 8) & 0xFF;
    response_data[index++] = result->time.day & 0xFF;
    response_data[index++] = (result->time.hour >> 8) & 0xFF;
    response_data[index++] = result->time.hour & 0xFF;
    response_data[index++] = (result->time.minute >> 8) & 0xFF;
    response_data[index++] = result->time.minute & 0xFF;
    response_data[index++] = (result->time.second >> 8) & 0xFF;
    response_data[index++] = result->time.second & 0xFF;
    
    /* 電池電壓和溫度 */
    response_data[index++] = (result->battery_voltage >> 8) & 0xFF;
    response_data[index++] = result->battery_voltage & 0xFF;
    response_data[index++] = (result->temperature >> 8) & 0xFF;
    response_data[index++] = result->temperature & 0xFF;
    
    /* 發送結果 */
    UART_Protocol_SendFrame(CMD_RESULT_RESPONSE, response_data, index);
}

/*********************************************************************
 * @fn      UART_Protocol_SendRawData
 *
 * @brief   發送RAW DATA
 *
 * @param   data - 數據指針
 * @param   length - 數據長度
 *
 * @return  none
 */
void UART_Protocol_SendRawData(uint8_t *data, uint16_t length)
{
    uint8_t response_data[MAX_DATA_LENGTH + 2];
    
    /* 檢查數據長度 */
    if (length > MAX_DATA_LENGTH) {
        length = MAX_DATA_LENGTH;
    }
    
    /* 組裝數據長度 */
    response_data[0] = (length >> 8) & 0xFF;
    response_data[1] = length & 0xFF;
    
    /* 複製RAW數據 */
    memcpy(&response_data[2], data, length);
    
    /* 發送RAW數據 */
    UART_Protocol_SendFrame(CMD_RAW_DATA_RESPONSE, response_data, length + 2);
} 