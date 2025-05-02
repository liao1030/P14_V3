/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x�{���cUARTͨӍ�f�h���^�n
 ********************************************************************************/

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* �f�h���P�������x */
#define PROTOCOL_START_MARKER     0xAA
#define PROTOCOL_END_MARKER       0x55

/* ָ��ID���x */
/* APP��Meter����ָ�� */
#define CMD_SYNC_TIME             0x01    // ͬ���r�g
#define CMD_GET_DEVICE_STATUS     0x02    // Ո���b�à�B
#define CMD_SET_CODE_EVENT        0x03    // �O��CODE��EVENT
#define CMD_BLOOD_STATUS_REQ      0x04    // �z�y��BՈ��
#define CMD_REQUEST_RESULT        0x05    // Ո��y���Y��
#define CMD_REQUEST_RAW_DATA      0x06    // Ո��RAW DATA

/* Meter��APP����ָ�� */
#define CMD_SYNC_TIME_ACK         0x81    // �r�gͬ���_�J
#define CMD_DEVICE_STATUS_ACK     0x82    // �b�à�B�ؑ�
#define CMD_CODE_EVENT_ACK        0x83    // CODE/EVENT�O���_�J
#define CMD_BLOOD_STATUS_NOTIFY   0x84    // ѪҺ�z�y֪ͨ
#define CMD_RESULT_RESPONSE       0x85    // �y���Y���ؑ�
#define CMD_RAW_DATA_RESPONSE     0x86    // RAW DATA�ؑ�
#define CMD_ERROR_RESPONSE        0xFF    // �e�`�ؑ�

/* �e�`���a */
#define ERR_BATTERY_LOW           0x01    // 늳�����^��
#define ERR_TEMP_HIGH             0x02    // �ض��^��
#define ERR_TEMP_LOW              0x03    // �ض��^��
#define ERR_STRIP_EXPIRED         0x04    // ԇƬ�^�ڻ�p��
#define ERR_STRIP_USED            0x05    // ԇƬ��ʹ��
#define ERR_STRIP_INSERT_ERROR    0x06    // ԇƬ�����e�`
#define ERR_BLOOD_SAMPLE_SHORT    0x07    // ѪҺ�ӱ�����
#define ERR_MEASURE_TIMEOUT       0x08    // �y�����r
#define ERR_CAL_ERROR             0x09    // У��e�`
#define ERR_HARDWARE_ERROR        0x0A    // Ӳ�w�e�`
#define ERR_COMM_ERROR            0x0B    // ͨӍ�e�`
#define ERR_DATA_FORMAT           0x0C    // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM              0x0D    // У���e�`
#define ERR_CMD_NOT_SUPPORTED     0x0E    // ָ�֧Ԯ
#define ERR_RESULT_OUT_OF_RANGE   0x0F    // �y���Y����������

/* �y���Ŀ���x */
#define ITEM_GLV                  0x0000  // Ѫ��
#define ITEM_U                    0x0001  // ����
#define ITEM_C                    0x0002  // ��đ�̴�
#define ITEM_TG                   0x0003  // �������֬
#define ITEM_GAV                  0x0004  // Ѫ��

/* EVENT���x */
#define EVENT_NONE                0x0000  // �o
#define EVENT_AC                  0x0001  // �ǰ
#define EVENT_PC                  0x0002  // ���
#define EVENT_QC                  0x0003  // QC

/* ����Y�� */
typedef struct {
    uint8_t cmd;             // ָ��ID
    uint8_t length;          // �Y���L��
    uint8_t *data;           // �Y��ָ��
    uint8_t checksum;        // У��
} Protocol_Packet_t;

/* �O���B�Y�� */
typedef struct {
    uint16_t measure_item;    // �y���Ŀ
    uint16_t strip_status;    // ԇƬ��B
    uint16_t battery_voltage; // 늳�늉� (mV)
    uint16_t temperature;     // �ض� (�Ŵ�10������25.7�Ȟ�257)
} Device_Status_t;

/* �y���Y���Y�� */
typedef struct {
    uint16_t result_status;  // �Y����B (0: �ɹ�, ��0: �e�`���a)
    uint16_t measure_value;  // �y���Y��ֵ
    uint16_t measure_item;   // �y���Ŀ
    uint16_t event;          // �¼����
    uint8_t  code;           // ԇƬУ���a
    uint16_t year;           // ��
    uint16_t month;          // ��
    uint16_t day;            // ��
    uint16_t hour;           // �r
    uint16_t minute;         // ��
    uint16_t second;         // ��
    uint16_t battery_voltage;// 늳�늉� (mV)
    uint16_t temperature;    // �ض� (�Ŵ�10��)
} Measure_Result_t;

/* �r�g�Y�� */
typedef struct {
    uint16_t year;           // ��
    uint8_t  month;          // ��
    uint8_t  day;            // ��
    uint8_t  hour;           // �r
    uint8_t  minute;         // ��
    uint8_t  second;         // ��
} Time_t;

/* �f�h����API */
uint8_t Protocol_Init(void);
uint8_t Protocol_ProcessReceivedData(uint8_t *data, uint16_t length);
uint8_t Protocol_SendPacket(uint8_t cmd, uint8_t *data, uint8_t length);
uint8_t Protocol_SendErrorResponse(uint8_t original_cmd, uint8_t error_code);
uint8_t Protocol_SendDeviceStatus(Device_Status_t *status);
uint8_t Protocol_SendMeasureResult(Measure_Result_t *result);
uint8_t Protocol_SendSyncTimeAck(uint8_t status);
uint8_t Protocol_SendCodeEventAck(uint8_t status);
uint8_t Protocol_SendBloodStatusNotify(uint8_t countdown);
uint8_t Protocol_SendRawData(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */