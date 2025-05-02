/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.h
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2023/04/28
 * Description        : P14�๦�������x�x���{���cUARTͨӍ�f�h̎��
 *********************************************************************************
 * Copyright (c) 2023 HMD. All rights reserved.
 *******************************************************************************/

#ifndef __PROTOCOL_HANDLER_H
#define __PROTOCOL_HANDLER_H

#include "CH58x_common.h"

// �f�h�������x
#define PROTOCOL_START_MARKER    0xAA    // ��ʼ��ӛ
#define PROTOCOL_END_MARKER      0x55    // �Y����ӛ

// ����ID���x
#define CMD_SYNC_TIME            0x01    // ͬ���r�g
#define CMD_SYNC_TIME_ACK        0x81    // �r�gͬ���_�J
#define CMD_REQ_DEVICE_STATUS    0x02    // Ո���O���B
#define CMD_DEVICE_STATUS_ACK    0x82    // �O���B�ؑ�
#define CMD_SET_CODE_EVENT       0x03    // �O��CODE��EVENT
#define CMD_CODE_EVENT_ACK       0x83    // CODE/EVENT�O���_�J
#define CMD_BLOOD_SAMPLE_STATUS  0x04    // �z�y��BՈ��
#define CMD_BLOOD_SAMPLE_ACK     0x84    // ѪҺ�z�y֪ͨ
#define CMD_REQ_RESULT           0x05    // Ո��z�y�Y��
#define CMD_RESULT_ACK           0x85    // �z�y�Y���ؑ�
#define CMD_REQ_RAW_DATA         0x06    // Ո��RAW DATA
#define CMD_RAW_DATA_ACK         0x86    // RAW DATA�ؑ�
#define CMD_ERROR_ACK            0xFF    // �e�`�ؑ�

// �e�`���a
#define ERR_LOW_BATTERY          0x01    // 늳�����^��
#define ERR_TEMP_HIGH            0x02    // �ض��^��
#define ERR_TEMP_LOW             0x03    // �ض��^��
#define ERR_STRIP_EXPIRED        0x04    // ԇ���^�ڻ�p��
#define ERR_STRIP_USED           0x05    // ԇ����ʹ��
#define ERR_STRIP_ERROR          0x06    // ԇ�������e�`
#define ERR_BLOOD_SAMPLE_INSUF   0x07    // ѪҺ�ӱ�����
#define ERR_MEASURE_TIMEOUT      0x08    // �z�y���r
#define ERR_CALIB_ERROR          0x09    // У���e�`
#define ERR_HARDWARE_ERROR       0x0A    // Ӳ���e�`
#define ERR_COMM_ERROR           0x0B    // ͨӍ�e�`
#define ERR_DATA_FORMAT          0x0C    // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM             0x0D    // У���e�`
#define ERR_COMMAND_UNSUPPORTED  0x0E    // ָ�֧��
#define ERR_RESULT_OUT_OF_RANGE  0x0F    // �z�y�Y����������

// �z�y�Ŀ
#define MEASURE_ITEM_GLV         0x0000  // Ѫ��
#define MEASURE_ITEM_U           0x0001  // ����
#define MEASURE_ITEM_C           0x0002  // ��đ�̴�
#define MEASURE_ITEM_TG          0x0003  // �������֬
#define MEASURE_ITEM_GAV         0x0004  // Ѫ��

// �¼����
#define EVENT_NONE               0x0000  // �o
#define EVENT_AC                 0x0001  // AC(�ǰ)
#define EVENT_PC                 0x0002  // PC(���)
#define EVENT_QC                 0x0003  // QC

// ������L�ȶ��x
#define MAX_PACKET_SIZE          64      // ������L��

// �f�h����Y��
typedef struct {
    uint8_t startMarker;         // ��ʼ��ӛ (0xAA)
    uint8_t cmdId;               // ָ��ID
    uint8_t dataLen;             // �������L��(N)
    uint8_t data[MAX_PACKET_SIZE]; // ����
    uint8_t checksum;            // У��
    uint8_t endMarker;           // �Y����ӛ (0x55)
} protocol_packet_t;

// �O���B�Y��
typedef struct {
    uint16_t measureItem;        // �z�y�Ŀ
    uint16_t stripStatus;        // ԇ����B
    uint16_t batteryVoltage;     // 늳�늉� (mV)
    uint16_t temperature;        // �ض� (�Ŵ�10��)
} device_status_t;

// �r�g�Y��
typedef struct {
    uint16_t year;               // ��
    uint8_t  month;              // ��
    uint8_t  day;                // ��
    uint8_t  hour;               // �r
    uint8_t  minute;             // ��
    uint8_t  second;             // ��
} time_info_t;

// �y���Y���Y��
typedef struct {
    uint16_t resultStatus;       // �Y����B
    uint16_t measureValue;       // �y��ֵ
    uint16_t measureItem;        // �y���Ŀ
    uint16_t event;              // �¼����
    uint8_t  code;               // ԇ��У�a
    time_info_t time;            // �y���r�g
    uint16_t batteryVoltage;     // 늳�늉�
    uint16_t temperature;        // �ض�
} measure_result_t;

// ������
void protocol_init(void);
void protocol_process_uart_data(uint8_t *data, uint16_t len);
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
uint8_t protocol_send_error(uint8_t originalCmdId, uint8_t errorCode);
uint8_t protocol_calculate_checksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
void protocol_handle_ble_data(uint8_t *data, uint16_t len);

#endif /* __PROTOCOL_HANDLER_H */ 