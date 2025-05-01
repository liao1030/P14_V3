/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_parser.h
 * Description        : �๦�������x�{��ͨӍ�f�h������
 *******************************************************************************/

#ifndef __PROTOCOL_PARSER_H
#define __PROTOCOL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// �f�h����
#define PACKET_START_MARKER     0xAA
#define PACKET_END_MARKER       0x55

// ָ��ID���x
#define CMD_SYNC_TIME                 0x01
#define CMD_SYNC_TIME_CONFIRM         0x81
#define CMD_REQUEST_DEVICE_STATUS     0x02
#define CMD_DEVICE_STATUS_RESPONSE    0x82
#define CMD_SET_CODE_EVENT            0x03
#define CMD_CODE_EVENT_CONFIRM        0x83
#define CMD_BLOOD_STATUS_REQUEST      0x04
#define CMD_BLOOD_STATUS_NOTIFY       0x84
#define CMD_REQUEST_RESULT            0x05
#define CMD_RESULT_RESPONSE           0x85
#define CMD_REQUEST_RAW_DATA          0x06
#define CMD_RAW_DATA_RESPONSE         0x86
#define CMD_ERROR_RESPONSE            0xFF

// �e�`���a
#define ERR_BATTERY_LOW               0x01
#define ERR_TEMP_TOO_HIGH             0x02
#define ERR_TEMP_TOO_LOW              0x03
#define ERR_STRIP_EXPIRED             0x04
#define ERR_STRIP_USED                0x05
#define ERR_STRIP_INSERT_ERROR        0x06
#define ERR_BLOOD_INSUFFICIENT        0x07
#define ERR_TEST_TIMEOUT              0x08
#define ERR_CALIBRATION_ERROR         0x09
#define ERR_HARDWARE_ERROR            0x0A
#define ERR_COMM_ERROR                0x0B
#define ERR_DATA_FORMAT_ERROR         0x0C
#define ERR_CHECKSUM_ERROR            0x0D
#define ERR_COMMAND_NOT_SUPPORTED     0x0E
#define ERR_RESULT_OUT_OF_RANGE       0x0F

// �yԇ�Ŀ
#define TEST_ITEM_GLU                 0x0000
#define TEST_ITEM_U                   0x0001
#define TEST_ITEM_C                   0x0002
#define TEST_ITEM_TG                  0x0003
#define TEST_ITEM_GAV                 0x0004

// �¼����
#define EVENT_NONE                    0x0000
#define EVENT_AC                      0x0001
#define EVENT_PC                      0x0002
#define EVENT_QC                      0x0003

// �f�h����^�Y��
typedef struct {
    uint8_t start;      // ��ʼ��ӛ 0xAA
    uint8_t cmd;        // ָ��ID
    uint8_t length;     // �Y���L��
    // �Y�ϲ��քӑB�L�ȣ��������ڴ˽Y����
    // uint8_t checksum;   // У��
    // uint8_t end;        // �Y����ӛ 0x55
} __attribute__((packed)) PacketHeader_t;

// �f�h���β�Y��
typedef struct {
    uint8_t checksum;   // У��
    uint8_t end;        // �Y����ӛ 0x55
} __attribute__((packed)) PacketFooter_t;

// ͬ���r�gָ���Y�ϽY��
typedef struct {
    uint16_t year;      // ���
    uint8_t month;      // �·� (1-12)
    uint8_t day;        // ���� (1-31)
    uint8_t hour;       // С�r (0-23)
    uint8_t minute;     // ��� (0-59)
    uint8_t second;     // �� (0-59)
} __attribute__((packed)) SyncTimeData_t;

// �O��CODE��EVENT�Y�ϽY��
typedef struct {
    uint8_t code;       // ԇ��У���a
    uint16_t event;     // �¼����
} __attribute__((packed)) CodeEventData_t;

// �O���B�ؑ��Y�ϽY��
typedef struct {
    uint16_t testItem;      // �yԇ�Ŀ
    uint16_t stripStatus;   // ԇ����B
    uint16_t batteryLevel;  // 늳�늉� (mV)
    uint16_t temperature;   // �h���ض� (x10)
} __attribute__((packed)) DeviceStatusData_t;

// �yԇ�Y���Y�ϽY��
typedef struct {
    uint16_t resultStatus;  // �Y����B
    uint16_t testValue;     // �yԇ�Y��ֵ
    uint16_t testItem;      // �yԇ�Ŀ
    uint16_t event;         // �¼����
    uint8_t code;           // ԇ��У���a
    uint16_t year;          // ���
    uint16_t month;         // �·�
    uint16_t day;           // ����
    uint16_t hour;          // С�r
    uint16_t minute;        // ���
    uint16_t second;        // ��
    uint16_t batteryLevel;  // 늳�늉� (mV)
    uint16_t temperature;   // �h���ض� (x10)
} __attribute__((packed)) TestResultData_t;

// RAW DATA �Y�ϽY��
typedef struct {
    uint16_t dataLength;    // RAW DATA���L��
    // ���m���׃�L�ȵ�RAW DATA
} __attribute__((packed)) RawDataHeader_t;

// �e�`�ؑ��Y�ϽY��
typedef struct {
    uint8_t originalCmd;    // ԭʼָ��ID
    uint8_t errorCode;      // �e�`���a
} __attribute__((packed)) ErrorResponseData_t;

// ѪҺ��B֪ͨ�Y�ϽY��
typedef struct {
    uint8_t countdown;      // �����딵
} __attribute__((packed)) BloodStatusData_t;

// �f�h�������Y��
typedef struct {
    uint8_t rxBuffer[256];       // ���վ��n�^
    uint16_t rxIndex;            // ��������
    uint8_t rxState;             // ���ՠ�B�C
    uint8_t isPacketComplete;    // �����ɘ��I
    uint8_t expectedLength;      // �A���Y���L��
    
    // ����Y�ϙ�λ
    uint8_t cmdId;               // ָ��ID
    uint8_t dataLength;          // �Y���L��
    uint8_t checksum;            // У��
    uint8_t calculatedChecksum;  // Ӌ��õ���У��
} ProtocolParser_t;

// ������
void protocol_parser_init(ProtocolParser_t *parser);
void protocol_parser_reset(ProtocolParser_t *parser);
int protocol_parser_parse(ProtocolParser_t *parser, uint8_t byte);
uint8_t calculate_checksum(uint8_t *data, uint16_t length);
int protocol_build_packet(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t *buffer, uint16_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_PARSER_H */ 