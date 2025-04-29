/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol_handlers.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UARTͨӍ�f�h����̎�팍�F
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "uart_protocol.h"
#include <string.h>
#include "debug.h"

/* �ⲿ���õ�ȫ��׃�� */
extern RTC_TimeStruct_t rtc_time;
extern Device_Status_t device_status;
extern Measure_Result_t measure_result;
extern Measure_Param_t measure_param;
extern uint8_t raw_data[];
extern uint16_t raw_data_length;
extern uint8_t blood_sampling_active;
extern uint8_t blood_countdown_seconds;

/* �y��ģ�M���� */
static uint16_t simulate_measure_value = 100;

/*********************************************************************
 * @fn      UART_Protocol_HandleSyncTime
 *
 * @brief   ̎��ͬ���r�gָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleSyncTime(uint8_t *data, uint8_t length)
{
    uint8_t response_data = 0x00; // �ɹ�
    
    /* �z�锵���L�� */
    if (length != 7) {
        UART_Protocol_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* �����r�g���� */
    rtc_time.year = (data[0] << 8) | data[1];
    rtc_time.month = data[2];
    rtc_time.day = data[3];
    rtc_time.hour = data[4];
    rtc_time.minute = data[5];
    rtc_time.second = data[6];
    
    printf("Time synced: %04d-%02d-%02d %02d:%02d:%02d\r\n", 
           rtc_time.year, rtc_time.month, rtc_time.day, 
           rtc_time.hour, rtc_time.minute, rtc_time.second);
    
    /* ����RTC�r�g (���H��������Ҫ�{��RTC�O�ú���) */
    // TODO: �O��ϵ�yRTC�r�g
    
    /* �l�ͳɹ�푑� */
    UART_Protocol_SendFrame(CMD_SYNC_TIME_ACK, &response_data, 1);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestStatus
 *
 * @brief   ̎��Ո���O���Bָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length)
{
    uint8_t response_data[8];
    
    /* �����O���B (���H��������Ҫ��Ӳ���@ȡ) */
    device_status.battery_voltage = 3000; // ʾ��: 3.0V
    device_status.temperature = 250;      // ʾ��: 25.0��C
    
    /* �M�b��B���� */
    response_data[0] = (device_status.measure_type >> 8) & 0xFF;
    response_data[1] = device_status.measure_type & 0xFF;
    response_data[2] = (device_status.strip_status >> 8) & 0xFF;
    response_data[3] = device_status.strip_status & 0xFF;
    response_data[4] = (device_status.battery_voltage >> 8) & 0xFF;
    response_data[5] = device_status.battery_voltage & 0xFF;
    response_data[6] = (device_status.temperature >> 8) & 0xFF;
    response_data[7] = device_status.temperature & 0xFF;
    
    printf("Status requested - Type: %04X, Strip: %04X, Battery: %dmV, Temp: %.1f��C\r\n", 
           device_status.measure_type, device_status.strip_status, 
           device_status.battery_voltage, device_status.temperature / 10.0);
    
    /* �l��푑� */
    UART_Protocol_SendFrame(CMD_STATUS_RESPONSE, response_data, 8);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleSetCodeEvent
 *
 * @brief   ̎���O��CODE��EVENTָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length)
{
    uint8_t response_data = 0x00; // �ɹ�
    
    /* �z�锵���L�� */
    if (length != 3) {
        UART_Protocol_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    /* ����CODE��EVENT */
    measure_param.code = data[0];
    measure_param.event = (data[1] << 8) | data[2];
    
    printf("Set CODE=%d, EVENT=%04X\r\n", measure_param.code, measure_param.event);
    
    /* �l�ͳɹ�푑� */
    UART_Protocol_SendFrame(CMD_CODE_EVENT_ACK, &response_data, 1);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleBloodSampleCheck
 *
 * @brief   ̎��z��Ѫ�Ӡ�Bָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length)
{
    /* �z���Ƿ���ԇƬ���� (���H������Ҫ�z�yӲ����B) */
    if (device_status.strip_status != 0) {
        /* ԇƬ�����e�` */
        UART_Protocol_SendErrorResponse(CMD_BLOOD_SAMPLE_CHECK, ERR_STRIP_INSERT);
        return;
    }
    
    /* ģ�M�z�y��ѪҺ�ӱ� */
    blood_sampling_active = 1;
    blood_countdown_seconds = 5; // 5�뵹Ӌ�r
    
    printf("Blood sample check, countdown: %d seconds\r\n", blood_countdown_seconds);
    
    /* �l��ѪҺ֪ͨ */
    UART_Protocol_SendBloodSampleNotify(blood_countdown_seconds);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestResult
 *
 * @brief   ̎��Ո��y���Y��ָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleRequestResult(uint8_t *data, uint8_t length)
{
    /* ���߀�ڒ�Ѫ��y���^���� */
    if (blood_sampling_active) {
        UART_Protocol_SendErrorResponse(CMD_REQUEST_RESULT, ERR_MEASURE_TIMEOUT);
        return;
    }
    
    /* ģ�M���ɜy���Y�� */
    measure_result.result_status = 0x0000; // �ɹ�
    measure_result.measure_value = simulate_measure_value; // ģ�Mֵ
    measure_result.measure_type = device_status.measure_type;
    measure_result.event_type = measure_param.event;
    measure_result.code = measure_param.code;
    
    /* �}�u��ǰ�r�g */
    measure_result.time = rtc_time;
    
    /* �}�u�������� */
    measure_result.battery_voltage = device_status.battery_voltage;
    measure_result.temperature = device_status.temperature;
    
    /* �S�C���Ӝy��ֵ��ģ�M׃�� */
    simulate_measure_value += 10;
    if (simulate_measure_value > 200) {
        simulate_measure_value = 100;
    }
    
    printf("Measurement result: Value=%d, Type=%04X, Event=%04X\r\n", 
           measure_result.measure_value, measure_result.measure_type, measure_result.event_type);
    
    /* �l�ͽY�� */
    UART_Protocol_SendResult(&measure_result);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleRequestRawData
 *
 * @brief   ̎��Ո��RAW DATAָ��
 *
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length)
{
    uint16_t i;
    
    /* ģ�M����RAW���� */
    raw_data_length = 30; // ʾ���L��
    
    /* ����һЩʾ������ */
    for (i = 0; i < raw_data_length; i++) {
        raw_data[i] = i + 0x10;
    }
    
    printf("Raw data requested, length=%d\r\n", raw_data_length);
    
    /* �l��RAW���� */
    UART_Protocol_SendRawData(raw_data, raw_data_length);
}

/*********************************************************************
 * @fn      UART_Protocol_SendBloodSampleNotify
 *
 * @brief   �l��ѪҺ�ӱ�֪ͨ
 *
 * @param   countdown_seconds - ��Ӌ�r�딵
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
 * @brief   �l���O���B
 *
 * @param   status - �O���B�Y��ָ�
 *
 * @return  none
 */
void UART_Protocol_SendStatus(Device_Status_t *status)
{
    uint8_t response_data[8];
    
    /* �M�b��B���� */
    response_data[0] = (status->measure_type >> 8) & 0xFF;
    response_data[1] = status->measure_type & 0xFF;
    response_data[2] = (status->strip_status >> 8) & 0xFF;
    response_data[3] = status->strip_status & 0xFF;
    response_data[4] = (status->battery_voltage >> 8) & 0xFF;
    response_data[5] = status->battery_voltage & 0xFF;
    response_data[6] = (status->temperature >> 8) & 0xFF;
    response_data[7] = status->temperature & 0xFF;
    
    /* �l�͠�B */
    UART_Protocol_SendFrame(CMD_STATUS_RESPONSE, response_data, 8);
}

/*********************************************************************
 * @fn      UART_Protocol_SendResult
 *
 * @brief   �l�͜y���Y��
 *
 * @param   result - �y���Y���Y��ָ�
 *
 * @return  none
 */
void UART_Protocol_SendResult(Measure_Result_t *result)
{
    uint8_t response_data[25];
    uint8_t index = 0;
    
    /* �M�b�Y������ */
    response_data[index++] = (result->result_status >> 8) & 0xFF;
    response_data[index++] = result->result_status & 0xFF;
    response_data[index++] = (result->measure_value >> 8) & 0xFF;
    response_data[index++] = result->measure_value & 0xFF;
    response_data[index++] = (result->measure_type >> 8) & 0xFF;
    response_data[index++] = result->measure_type & 0xFF;
    response_data[index++] = (result->event_type >> 8) & 0xFF;
    response_data[index++] = result->event_type & 0xFF;
    response_data[index++] = result->code;
    
    /* �r�g���� */
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
    
    /* 늳�늉��͜ض� */
    response_data[index++] = (result->battery_voltage >> 8) & 0xFF;
    response_data[index++] = result->battery_voltage & 0xFF;
    response_data[index++] = (result->temperature >> 8) & 0xFF;
    response_data[index++] = result->temperature & 0xFF;
    
    /* �l�ͽY�� */
    UART_Protocol_SendFrame(CMD_RESULT_RESPONSE, response_data, index);
}

/*********************************************************************
 * @fn      UART_Protocol_SendRawData
 *
 * @brief   �l��RAW DATA
 *
 * @param   data - ����ָ�
 * @param   length - �����L��
 *
 * @return  none
 */
void UART_Protocol_SendRawData(uint8_t *data, uint16_t length)
{
    uint8_t response_data[MAX_DATA_LENGTH + 2];
    
    /* �z�锵���L�� */
    if (length > MAX_DATA_LENGTH) {
        length = MAX_DATA_LENGTH;
    }
    
    /* �M�b�����L�� */
    response_data[0] = (length >> 8) & 0xFF;
    response_data[1] = length & 0xFF;
    
    /* �}�uRAW���� */
    memcpy(&response_data[2], data, length);
    
    /* �l��RAW���� */
    UART_Protocol_SendFrame(CMD_RAW_DATA_RESPONSE, response_data, length + 2);
} 