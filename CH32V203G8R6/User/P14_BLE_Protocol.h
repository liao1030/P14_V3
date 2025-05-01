/**
 * @file P14_BLE_Protocol.h
 * @brief �๦�������z�y�x�{��UARTͨӍ�f�h���x
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>

/* �f�h���^β��ӛ */
#define PROTOCOL_START_MARKER      0xAA
#define PROTOCOL_END_MARKER        0x55

/* �������L�� */
#define MAX_PACKET_SIZE            64
#define MAX_DATA_SIZE              60

/* ����ID���x */
/* APP �� Meter���� */
#define CMD_SYNC_TIME              0x01  // ͬ���r�g
#define CMD_GET_DEVICE_STATUS      0x02  // Ո���b�à�B
#define CMD_SET_CODE_EVENT         0x03  // �O��CODE��EVENT����
#define CMD_BLOOD_SAMPLE_STATUS    0x04  // �z�yѪҺ��B
#define CMD_GET_TEST_RESULT        0x05  // Ո��yԇ�Y��
#define CMD_GET_RAW_DATA           0x06  // Ո��RAW DATA

/* Meter �� APP�ؑ� */
#define CMD_SYNC_TIME_ACK          0x81  // �r�gͬ���_�J
#define CMD_DEVICE_STATUS_ACK      0x82  // �b�à�B�ؑ�
#define CMD_CODE_EVENT_ACK         0x83  // CODE/EVENT�O���_�J
#define CMD_BLOOD_SAMPLE_ACK       0x84  // ѪҺ�z�y֪ͨ
#define CMD_TEST_RESULT_ACK        0x85  // �yԇ�Y���ؑ�
#define CMD_RAW_DATA_ACK           0x86  // RAW DATA�ؑ�
#define CMD_ERROR_ACK              0xFF  // �e�`�ؑ�

/* �O���B�Y�����x */
typedef struct {
    uint16_t testType;      // �yԇ�Ŀ: 0x0000: GLV(Ѫ��), 0x0001: U(����), 0x0002: C(��đ�̴�), 0x0003: TG(�������֬), 0x0004: GAV(Ѫ��)
    uint16_t stripStatus;   // ԇ����B: 0x0000: ����, ��0: �e�`���a
    uint16_t batteryVoltage;// 늳�늉� (mV), ��λ��ǰ (�� 0x0B01 ��ʾ 2817mV = 2.817V)
    uint16_t temperature;   // �h���ض� (�Ŵ�10��), ��λ��ǰ (�� 0x0119 ��ʾ 281 = 28.1��)
} DeviceStatus_TypeDef;

/* �yԇ�Y���Y�����x */
typedef struct {
    uint16_t resultStatus;  // �Y����B: 0x0000: �ɹ�, ��0: �e�`���a
    uint16_t testValue;     // �yԇ�Y��ֵ
    uint16_t testType;      // �yԇ�Ŀ���
    uint16_t event;         // �¼����: 0x0000: �o, 0x0001: AC(�ǰ), 0x0002: PC(���), 0x0003: QC
    uint8_t  code;          // ԇ��У���a
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
#define ERR_STRIP_EXPIRED          0x04  // ԇ���^�ڻ�p��
#define ERR_STRIP_USED             0x05  // ԇ����ʹ��
#define ERR_STRIP_INSERT           0x06  // ԇ�������e�`
#define ERR_BLOOD_INSUFFICIENT     0x07  // ѪҺ�ӱ�����
#define ERR_TEST_TIMEOUT           0x08  // �y�����r
#define ERR_CALIBRATION            0x09  // У���e�`
#define ERR_HARDWARE               0x0A  // Ӳ�w�e�`
#define ERR_COMMUNICATION          0x0B  // ͨӍ�e�`
#define ERR_DATA_FORMAT            0x0C  // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM               0x0D  // У���e�`
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
    uint8_t start;          // ��ʼ��ӛ 0xAA
    uint8_t command;        // ָ��ID
    uint8_t length;         // �Y���L��
    uint8_t data[MAX_DATA_SIZE]; // �Y�υ^
    uint8_t checksum;       // У��
    uint8_t end;            // �Y����ӛ 0x55
} BLEPacket_TypeDef;

/* �������� */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket);
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length);
void BLE_SendPacket(BLEPacket_TypeDef *packet);
void BLE_ProtocolInit(void);

#endif /* __P14_BLE_PROTOCOL_H */ 