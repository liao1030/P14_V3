/**
 * @file P14_BLE_Protocol.h
 * @brief �๦�������z�y�xUARTͨӍ�f�h���x
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>
#include "P14_Parameter_Table.h" // �����������^�ļ���ȡ��StripType_TypeDef���x

/* �f�h��ֹ��ӛ */
#define PROTOCOL_START_MARKER      0xAA
#define PROTOCOL_END_MARKER        0x55

/* ���n�L�� */
#define MAX_PACKET_SIZE            64
#define MAX_DATA_SIZE              60

/* ����ID���x */
/* APP �� Meter���� */
#define CMD_SYNC_TIME              0x01  // ͬ���r�g
#define CMD_GET_DEVICE_STATUS      0x02  // Ո���O���B
#define CMD_SET_CODE_EVENT         0x03  // �O��CODE��EVENT����
#define CMD_BLOOD_SAMPLE_STATUS    0x04  // �z�yѪҺ��B
#define CMD_GET_TEST_RESULT        0x05  // Ո��yԇ�Y��
#define CMD_GET_RAW_DATA           0x06  // Ո��RAW DATA

/* Meter �� APP�ؑ� */
#define CMD_SYNC_TIME_ACK          0x81  // �r�gͬ���_�J
#define CMD_DEVICE_STATUS_ACK      0x82  // �O���B�ؑ�
#define CMD_CODE_EVENT_ACK         0x83  // CODE/EVENT�O�ô_�J
#define CMD_BLOOD_SAMPLE_ACK       0x84  // ѪҺ�z�y֪ͨ
#define CMD_TEST_RESULT_ACK        0x85  // �yԇ�Y���ؑ�
#define CMD_RAW_DATA_ACK           0x86  // RAW DATA�ؑ�
#define CMD_ERROR_ACK              0xFF  // �e�`�ؑ�

/* �O���B�Y�����x */
typedef struct {
    uint16_t testType;      // �yԇ�Ŀ: 0x0000: GLV(Ѫ��), 0x0001: U(����), 0x0002: C(��đ�̴�), 0x0003: TG(�������֬), 0x0004: GAV(Ѫ��)
    uint16_t stripStatus;   // ԇƬ��B: 0x0000: ����, ��0: �e�`���a
    uint16_t batteryVoltage;// 늳�늉� (mV), ��λ��ǰ (�� 0x0B01 ��ʾ 2817mV = 2.817V)
    uint16_t temperature;   // �h���ض� (�Ŵ�10��), ��λ��ǰ (�� 0x0119 ��ʾ 281 = 28.1��)
} DeviceStatus_TypeDef;

/* �yԇ�Y���Y�����x */
typedef struct {
    uint16_t resultStatus;  // �Y����B: 0x0000: �ɹ�, ��0: �e�`���a
    uint16_t testValue;     // �yԇ�Y��ֵ
    uint16_t testType;      // �yԇ�Ŀ���
    uint16_t event;         // �¼����: 0x0000: �o, 0x0001: AC(��ǰ), 0x0002: PC(����), 0x0003: QC
    uint8_t  code;          // ԇƬУ���a
    uint16_t year;          // ���: 2000+��ֵ (�� 0x0019 ��ʾ 2025��)
    uint16_t month;         // �·�: 1-12
    uint16_t date;          // ����: 1-31
    uint16_t hour;          // С�r: 0-23
    uint16_t minute;        // ���: 0-59
    uint16_t second;        // ��: 0-59
    uint16_t batteryVoltage;// 늳�늉� (mV)
    uint16_t temperature;   // �h���ض� (�Ŵ�10��)
} TestResult_TypeDef;

/* �e�`���a���x */
#define ERR_BATTERY_LOW            0x01  // 늳�����^��
#define ERR_TEMP_TOO_HIGH          0x02  // �ض��^��
#define ERR_TEMP_TOO_LOW           0x03  // �ض��^��
#define ERR_STRIP_EXPIRED          0x04  // ԇƬ�^�ڻ��ӻ�
#define ERR_STRIP_USED             0x05  // ԇƬ��ʹ��
#define ERR_STRIP_INSERT           0x06  // ԇƬ�����e�`
#define ERR_BLOOD_INSUFFICIENT     0x07  // ѪҺ�ӱ�����
#define ERR_TEST_TIMEOUT           0x08  // �y�����r
#define ERR_CALIBRATION            0x09  // У���e�`
#define ERR_HARDWARE               0x0A  // Ӳ�w�e�`
#define ERR_COMMUNICATION          0x0B  // ͨӍ�e�`
#define ERR_DATA_FORMAT            0x0C  // ������ʽ�e�`
#define ERR_CHECKSUM               0x0D  // У��e�`
#define ERR_UNSUPPORTED_CMD        0x0E  // ָ�֧Ԯ
#define ERR_RESULT_OUT_OF_RANGE    0x0F  // �y���Y����������

/* RTC�r�g���ڽY���w���x */
typedef struct {
    uint16_t Year;      // ��� (0-99)
    uint8_t Month;      // �·� (1-12)
    uint8_t Day;        // ���� (1-31)
    uint8_t Hour;       // С�r (0-23)
    uint8_t Min;        // ��� (0-59)
    uint8_t Sec;        // �� (0-59)
} RTC_DateTimeTypeDef;

/* �f�h����Y�� */
typedef struct {
    uint8_t start;          // �_ʼ��ӛ 0xAA
    uint8_t command;        // ָ��ID
    uint8_t length;         // �����L��
    uint8_t data[MAX_DATA_SIZE]; // ������
    uint8_t checksum;       // У�
    uint8_t end;            // �Y����ӛ 0x55
} BLEPacket_TypeDef;

/* ���ܺ��� */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_SendPacket(BLEPacket_TypeDef *packet);
void BLE_ProtocolInit(void);

/* CH582F-CH32V203ͨӍ���x */
#define CMD_START_MARKER        0xAA    // �����_ʼ��ӛ
#define CMD_END_MARKER          0xBB    // ����Y����ӛ

/* ������� */
#define CMD_MEASURE_T1_OUT      0x01    // �y��T1_OUT늉�
#define CMD_STRIP_INSERTED      0x02    // ԇƬ����֪ͨ
#define CMD_STRIP_TYPE_RESULT   0x03    // ԇƬ����Д�Y��
#define CMD_START_MEASUREMENT   0x04    // �_ʼ�y��

/* T1늘O�y���Y�����x */
#define T1_OUT_NEAR_0V          0       // T1_OUT�ӽ�0V
#define T1_OUT_NEAR_2_5V        1       // T1_OUT�ӽ�2.5V

/* ������ՠ�B�C */
typedef enum {
    PROTOCOL_STATE_IDLE,            // �ȴ������_ʼ
    PROTOCOL_STATE_CMD_TYPE,        // �����������
    PROTOCOL_STATE_DATA,            // ���Ք���
    PROTOCOL_STATE_END              // �ȴ�����Y��
} ProtocolState_TypeDef;

/* �f�h̎���� */
void BLE_ProtocolInit(void);
void BLE_ProtocolRxHandler(uint8_t rx_data);
void BLE_SendADCValue(uint16_t adc_value);
void BLE_NotifyStripType(StripType_TypeDef type);
void BLE_ProcessCommand(void);

/* ԇƬ�ɜy�c�Єe���P���� */
void BLE_MeasureT1Out(void);
void BLE_ProcessStripInsertion(StripType_TypeDef type);
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v);

/* ������ */
void BLE_ProtocolInit(void);
void BLE_ProtocolRxHandler(uint8_t rx_data);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_MeasureT1Out(void);
void BLE_SendADCValue(uint16_t adc_value);
void BLE_ProcessStripInsertion(StripType_TypeDef type);
void BLE_NotifyStripType(StripType_TypeDef type);
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v);
float Get_Chip_Temperature(void);

#endif /* __P14_BLE_PROTOCOL_H */ 