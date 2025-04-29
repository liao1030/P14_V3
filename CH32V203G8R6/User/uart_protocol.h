/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UARTͨӍ�f�h�^�ļ�
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "debug.h"

/* ͨӍ�f�h�������x */
#define FRAME_START_BYTE                0xAA
#define FRAME_END_BYTE                  0x55

/* ָ��ID���x */
#define CMD_SYNC_TIME                   0x01
#define CMD_SYNC_TIME_ACK               0x81
#define CMD_REQUEST_STATUS              0x02
#define CMD_STATUS_RESPONSE             0x82
#define CMD_SET_CODE_EVENT              0x03
#define CMD_CODE_EVENT_ACK              0x83
#define CMD_BLOOD_SAMPLE_CHECK          0x04
#define CMD_BLOOD_SAMPLE_NOTIFY         0x84
#define CMD_REQUEST_RESULT              0x05
#define CMD_RESULT_RESPONSE             0x85
#define CMD_REQUEST_RAW_DATA            0x06
#define CMD_RAW_DATA_RESPONSE           0x86
#define CMD_ERROR_RESPONSE              0xFF

/* �e�`���a���x */
#define ERR_BATTERY_LOW                 0x01
#define ERR_TEMPERATURE_HIGH            0x02
#define ERR_TEMPERATURE_LOW             0x03
#define ERR_STRIP_EXPIRED               0x04
#define ERR_STRIP_USED                  0x05
#define ERR_STRIP_INSERT                0x06
#define ERR_BLOOD_INSUFFICIENT          0x07
#define ERR_MEASURE_TIMEOUT             0x08
#define ERR_CALIBRATION                 0x09
#define ERR_HARDWARE                    0x0A
#define ERR_COMMUNICATION               0x0B
#define ERR_DATA_FORMAT                 0x0C
#define ERR_CHECKSUM                    0x0D
#define ERR_COMMAND_NOTSUPPORT          0x0E
#define ERR_RESULT_OUTOFRANGE           0x0F

/* �y����Ͷ��x */
#define MEASURE_TYPE_GLV                0x0000
#define MEASURE_TYPE_U                  0x0001
#define MEASURE_TYPE_C                  0x0002
#define MEASURE_TYPE_TG                 0x0003
#define MEASURE_TYPE_GAV                0x0004

/* �¼���Ͷ��x */
#define UART_EVENT_NONE                 0x0000
#define UART_EVENT_AC                   0x0001
#define UART_EVENT_PC                   0x0002
#define UART_EVENT_QC                   0x0003

/* �f�h��������L�� */
#define MAX_FRAME_LENGTH                64
#define MAX_DATA_LENGTH                 50

/* �؂��C�����P���� */
#define MAX_RETRY_COUNT                 3
#define RETRY_TIMEOUT_MS                500

/* RTC�r�g�Y�� */
typedef struct {
    uint16_t year;                      // ���
    uint8_t month;                      // �·�
    uint8_t day;                        // ����
    uint8_t hour;                       // С�r
    uint8_t minute;                     // ���
    uint8_t second;                     // ��
} RTC_TimeStruct_t;

/* �O���B�Y�����x */
typedef struct {
    uint16_t measure_type;              // �y���Ŀ
    uint16_t strip_status;              // ԇƬ��B
    uint16_t battery_voltage;           // 늳�늉�(mV)
    uint16_t temperature;               // �h���ض�(x10)
} Device_Status_t;

/* �y���Y���Y�����x */
typedef struct {
    uint16_t result_status;             // �Y����B
    uint16_t measure_value;             // �y���Y��ֵ
    uint16_t measure_type;              // �y���Ŀ
    uint16_t event_type;                // �¼����
    uint8_t  code;                      // ԇƬУ�a
    RTC_TimeStruct_t time;              // �y���r�g
    uint16_t battery_voltage;           // 늳�늉�
    uint16_t temperature;               // �h���ض�
} Measure_Result_t;

/* �y�������Y�����x */
typedef struct {
    uint8_t code;                       // ԇƬУ�a
    uint16_t event;                     // �¼����
} Measure_Param_t;

/* �f�h������ */
void UART_Protocol_Init(void);
uint8_t UART_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length);
uint16_t UART_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer);
void UART_Protocol_SendFrame(uint8_t cmd_id, uint8_t *data, uint8_t length);
void UART_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code);
uint8_t UART_Protocol_Parse(uint8_t *data, uint16_t length);
void UART_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* ̎��ָ��Ļ��{���� */
void UART_Protocol_HandleSyncTime(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length);
void UART_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length);
void UART_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestResult(uint8_t *data, uint8_t length);
void UART_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length);

/* ���Ӱl�ͺ��� */
void UART_Protocol_SendBloodSampleNotify(uint8_t countdown_seconds);
void UART_Protocol_SendStatus(Device_Status_t *status);
void UART_Protocol_SendResult(Measure_Result_t *result);
void UART_Protocol_SendRawData(uint8_t *data, uint16_t length);

#endif /* UART_PROTOCOL_H */ 