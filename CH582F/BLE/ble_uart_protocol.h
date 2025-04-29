/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_protocol.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : BLE UARTͨӍ�f�h�^�ļ�
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#ifndef BLE_UART_PROTOCOL_H
#define BLE_UART_PROTOCOL_H

#include "CH58x_common.h"

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
#define CMD_START_T1_MEASUREMENT        0x10
#define CMD_T1_MEASUREMENT_RESULT       0x90
#define CMD_STRIP_DETECTED              0x11
#define CMD_STRIP_TYPE_CONFIRM          0x91
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
#define ERR_STRIP_TYPE_UNKNOWN          0x10

/* �y����Ͷ��x */
#define MEASURE_TYPE_GLV                0x0000
#define MEASURE_TYPE_U                  0x0001
#define MEASURE_TYPE_C                  0x0002
#define MEASURE_TYPE_TG                 0x0003
#define MEASURE_TYPE_GAV                0x0004
#define MEASURE_TYPE_UNKNOWN            0xFFFF

/* ԇƬ��Ͷ��x - �f�h�� */
#define PROTOCOL_STRIP_UNKNOWN          0x00
#define PROTOCOL_STRIP_GLV              0x01
#define PROTOCOL_STRIP_U                0x02
#define PROTOCOL_STRIP_C                0x03
#define PROTOCOL_STRIP_TG               0x04
#define PROTOCOL_STRIP_GAV              0x05

/* �¼���Ͷ��x */
#define EVENT_NONE                      0x0000
#define EVENT_AC                        0x0001
#define EVENT_PC                        0x0002
#define EVENT_QC                        0x0003

/* �f�h��������L�� */
#define MAX_FRAME_LENGTH                64
#define MAX_DATA_LENGTH                 50

/* BLE����UUID */
#define BLE_SERVICE_UUID                0xFFE0
#define BLE_CHAR_RX_UUID                0xFFE1
#define BLE_CHAR_TX_UUID                0xFFE2

/* �������Y�����x (�f�hՈ��/푑�) */
typedef struct {
    uint8_t cmd_id;             // ָ��ID
    uint8_t length;             // �����^�L��
    uint8_t data[MAX_FRAME_LENGTH]; // �����^
    uint8_t checksum;           // У��
} Protocol_Frame_t;

/* �O���B�Y�����x */
typedef struct {
    uint16_t measure_type;      // �y���Ŀ
    uint16_t strip_status;      // ԇƬ��B
    uint16_t battery_voltage;   // 늳�늉�(mV)
    uint16_t temperature;       // �h���ض�(x10)
} Device_Status_t;

/* �y���Y���Y�����x */
typedef struct {
    uint16_t result_status;     // �Y����B
    uint16_t measure_value;     // �y���Y��ֵ
    uint16_t measure_type;      // �y���Ŀ
    uint16_t event_type;        // �¼����
    uint8_t  code;              // ԇƬУ�a
    uint16_t year;              // ���
    uint16_t month;             // �·�
    uint16_t day;               // ����
    uint16_t hour;              // С�r
    uint16_t minute;            // ���
    uint16_t second;            // ��
    uint16_t battery_voltage;   // 늳�늉�
    uint16_t temperature;       // �h���ض�
} Measure_Result_t;

/* ������ */
void BLE_Protocol_Init(void);
uint8_t BLE_Protocol_Parse(uint8_t *data, uint16_t length);
uint16_t BLE_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer);
uint8_t BLE_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length);
void BLE_Protocol_SendResponse(uint8_t cmd_id, uint8_t *data, uint8_t length);
void BLE_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code);
void BLE_UART_SendData(uint8_t *data, uint16_t length);

/* UART��BLE�����D�l���� */
void BLE_To_UART_Forward(uint8_t *data, uint16_t length);
void UART_To_BLE_Forward(uint8_t *data, uint16_t length);

/* �f�h������ */
void BLE_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* ̎��ָ��Ļ��{���� */
void BLE_Protocol_HandleSyncTime(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestStatus(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleSetCodeEvent(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleBloodSampleCheck(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestResult(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleRequestRawData(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleT1MeasurementResult(uint8_t *data, uint8_t length);
void BLE_Protocol_HandleStripTypeConfirm(uint8_t *data, uint8_t length);

#endif /* BLE_UART_PROTOCOL_H */ 