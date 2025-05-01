/**
 * @file P14_BLE_Protocol.h
 * @brief �๦�������z�y�xCH582F�{��ͨӍ�f�h���x
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __P14_BLE_PROTOCOL_H
#define __P14_BLE_PROTOCOL_H

#include <stdint.h>
#include "ble_uart_service.h"  // ���������ȡ�� ble_uart_evt_t ���

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

/* �f�h����Y�� */
typedef struct {
    uint8_t start;          // ��ʼ��ӛ 0xAA
    uint8_t command;        // ָ��ID
    uint8_t length;         // �Y���L��
    uint8_t data[MAX_DATA_SIZE]; // �Y�υ^
    uint8_t checksum;       // У��
    uint8_t end;            // �Y����ӛ 0x55
} BLEPacket_TypeDef;

/* BLE�O����Ϣ */
#define BLE_DEVICE_NAME        "P14-"       // �O����ǰ�Y�����MAC��ַ��6λ
#define BLE_SERVICE_UUID       0xFFE0       // ����UUID
#define BLE_CHAR_UUID_RX       0xFFE1       // ����UUID (����֪ͨ)
#define BLE_CHAR_UUID_TX       0xFFE2       // ����UUID (�l��֪ͨ)

/* �������� */
/**
 * @brief ��ʼ���{���f�h
 */
void P14_BLE_ProtocolInit(void);

/**
 * @brief ̎�����APP������
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_ProcessAppCommand(uint8_t *data, uint16_t length);

/**
 * @brief ̎�����MCU�Ĕ���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_ProcessMcuData(uint8_t *data, uint16_t length);

/**
 * @brief ��APP�l�͔���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_SendToApp(uint8_t *data, uint16_t length);

/**
 * @brief ��MCU�l�͔���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_SendToMCU(uint8_t *data, uint16_t length);

/**
 * @brief �{��UART���ջ��{̎��
 * 
 * @param connection_handle �B�Ӿ��
 * @param p_evt �¼�ָ�
 */
void P14_BLE_UartCallback(uint16_t connection_handle, ble_uart_evt_t *p_evt);

/* �f�h̎���� */
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length);
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen);
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet);

#endif /* __P14_BLE_PROTOCOL_H */ 