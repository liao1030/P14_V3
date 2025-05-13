/* ͨӍ�f�h��ӛ */
#define PROTOCOL_START_MARK   0xAA    // ��ʼ��ӛ
#define PROTOCOL_END_MARK     0x55    // �Y����ӛ

/* ָ��a���x */
#define CMD_SYNC_TIME         0x01    // ͬ���r�g
#define CMD_SYNC_TIME_ACK     0x81    // �r�gͬ���_�J
#define CMD_GET_STATUS        0x02    // Ո���b�à�B
#define CMD_STATUS_ACK        0x82    // �b�à�B�ؑ�
#define CMD_SET_PARAM         0x03    // �O��CODE��EVENT
#define CMD_SET_PARAM_ACK     0x83    // �O��CODE��EVENT�ؑ�
#define CMD_CHECK_BLOOD       0x04    // �z�yѪҺ
#define CMD_BLOOD_NOTIFY      0x84    // ѪҺ�z�y֪ͨ
#define CMD_GET_RESULT        0x05    // Ո��yԇ�Y��
#define CMD_RESULT_ACK        0x85    // �yԇ�Y���ؑ�
#define CMD_GET_RAW_DATA      0x06    // Ո��RAW DATA
#define CMD_RAW_DATA_ACK      0x86    // RAW DATA�ؑ�
#define CMD_ERROR_ACK         0x8F    // �e�`�ؑ�

/* ԇƬ���Pָ�� */
#define CMD_STRIP_INSERTED    0x20    // ԇƬ����֪ͨ
#define CMD_STRIP_TYPE_ACK    0xA0    // ԇƬ��ͻؑ�

//-------------------------------------------------
//  * File Name          : uart_protocol.h
//  * Author             : HMD Team
//  * Version            : V1.0.0
//  * Date               : 2025/05/12
//  * Description        : �๦�������yԇ�xUARTͨӍ�f�h
//-------------------------------------------------

#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#include "ch32v20x.h"
#include "param_table.h"

/* ͨӍ�f�h��ӛ */
#define PROTOCOL_START_MARK   0xAA    // ��ʼ��ӛ
#define PROTOCOL_END_MARK     0x55    // �Y����ӛ

/* ָ��a���x */
#define CMD_SYNC_TIME         0x01    // ͬ���r�g
#define CMD_SYNC_TIME_ACK     0x81    // �r�gͬ���_�J
#define CMD_GET_STATUS        0x02    // Ո���b�à�B
#define CMD_STATUS_ACK        0x82    // �b�à�B�ؑ�
#define CMD_SET_PARAM         0x03    // �O��CODE��EVENT
#define CMD_SET_PARAM_ACK     0x83    // �O��CODE��EVENT�ؑ�
#define CMD_CHECK_BLOOD       0x04    // �z�yѪҺ
#define CMD_BLOOD_NOTIFY      0x84    // ѪҺ�z�y֪ͨ
#define CMD_GET_RESULT        0x05    // Ո��yԇ�Y��
#define CMD_RESULT_ACK        0x85    // �yԇ�Y���ؑ�
#define CMD_GET_RAW_DATA      0x06    // Ո��RAW DATA
#define CMD_RAW_DATA_ACK      0x86    // RAW DATA�ؑ�
#define CMD_ERROR_ACK         0x8F    // �e�`�ؑ�

/* ԇƬ���Pָ�� */
#define CMD_STRIP_INSERTED    0x20    // ԇƬ����֪ͨ
#define CMD_STRIP_TYPE_ACK    0xA0    // ԇƬ��ͻؑ�

/* �e�`���a���x */
#define ERR_BATTERY_LOW       0x01    // 늳�����^��
#define ERR_TEMP_HIGH         0x02    // �ض��^��
#define ERR_TEMP_LOW          0x03    // �ض��^��
#define ERR_STRIP_EXPIRED     0x04    // ԇƬ�^�ڻ�p��
#define ERR_STRIP_USED        0x05    // ԇƬ��ʹ��
#define ERR_STRIP_ERROR       0x06    // ԇƬ�����e�`
#define ERR_BLOOD_NOT_ENOUGH  0x07    // ѪҺ�ӱ�����
#define ERR_TEST_TIMEOUT      0x08    // �y�����r
#define ERR_CAL_ERROR         0x09    // У���e�`
#define ERR_HARDWARE_ERROR    0x0A    // Ӳ�w�e�`
#define ERR_COMM_ERROR        0x0B    // ͨӍ�e�`
#define ERR_DATA_FORMAT       0x0C    // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM_ERROR    0x0D    // У���e�`
#define ERR_COMMAND_NOTSUPP   0x0E    // ָ�֧Ԯ
#define ERR_RESULT_OUTRANGE   0x0F    // �y���Y����������

/* �f�h����Y�����x (����С) */
#define MAX_PACKET_SIZE       64      // �������С
#define MAX_DATA_SIZE         60      // ����Y�υ^��С

/* �yԇ�Y�ϽY�� */
typedef struct {
    uint16_t resultStatus;    // �Y����B (0=�ɹ�, ��0=�e�`���a)
    uint16_t testValue;       // �yԇ�Y��ֵ
    uint16_t stripType;       // �yԇ�Ŀ
    uint16_t eventType;       // �¼����
    uint16_t stripCode;       // ԇƬУ���a
    uint16_t year;            // ���
    uint16_t month;           // �·�
    uint16_t date;            // ����
    uint16_t hour;            // С�r
    uint16_t minute;          // ���
    uint16_t second;          // �딵
    uint16_t batteryVoltage;  // 늳�늉� (mV)
    uint16_t temperature;     // �h���ض� (�Ŵ�10��)
} TestResult_TypeDef;

/* �b�à�B�Y�� */
typedef struct {
    uint16_t stripType;       // �yԇ�Ŀ
    uint16_t stripStatus;     // ԇƬ��B (0=����, ��0=�e�`���a)
    uint16_t batteryVoltage;  // 늳�늉� (mV)
    uint16_t temperature;     // �h���ض� (�Ŵ�10��)
} DeviceStatus_TypeDef;

/* RAW DATA�Y�� */
typedef struct {
    uint16_t resultStatus;    // �Y����B
    uint16_t testValue;       // �yԇֵ
    uint16_t year;            // ���
    uint16_t month;           // �·�
    uint16_t date;            // ����
    uint16_t hour;            // С�r
    uint16_t minute;          // ���
    uint16_t second;          // �딵
    uint16_t stripType;       // �yԇ�Ŀ
    uint16_t eventType;       // �¼����
    uint16_t batteryVoltage;  // 늳�늉�
    uint16_t temperature;     // �h���ض�
    uint16_t w1Adv;           // W1ͨ��ADCֵ
    uint16_t w2Adv;           // W2ͨ��ADCֵ
    uint16_t t1Ampl;          // T1�ضȸМy���
    uint16_t t1AcMax;         // T1 AC���ֵ
    uint16_t t1AcMin;         // T1 AC��Сֵ
    uint16_t t3Adc;           // T3ͨ��ADCֵ
    uint16_t testCount;       // �yԇ�Δ�
    uint16_t stripCode;       // ԇƬУ���a
    char operatorId[10];      // ������ID
    char chartNo[10];         // �D��̖
} RawData_TypeDef;

/* ������������ */
void UART_Protocol_Init(void);
void UART_Protocol_Process(void);

/* ���̎���� */
uint8_t UART_ProcessPacket(uint8_t *packet, uint16_t length);

/* �ض�ָ��̎���� */
uint8_t UART_ProcessSyncTime(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetStatus(uint8_t *data, uint8_t length);
uint8_t UART_ProcessSetParam(uint8_t *data, uint8_t length);
uint8_t UART_ProcessCheckBlood(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetResult(uint8_t *data, uint8_t length);
uint8_t UART_ProcessGetRawData(uint8_t *data, uint8_t length);
uint8_t UART_ProcessStripInsertedCmd(uint8_t *data, uint8_t length);

/* �ؑ�����a������ */
uint8_t UART_SendSyncTimeAck(uint8_t status);
uint8_t UART_SendStatusAck(DeviceStatus_TypeDef *status);
uint8_t UART_SendParamAck(uint8_t status);
uint8_t UART_SendBloodNotify(uint8_t countdown);
uint8_t UART_SendResultAck(TestResult_TypeDef *result);
uint8_t UART_SendRawDataAck(RawData_TypeDef *rawData);
uint8_t UART_SendErrorAck(uint8_t cmdId, uint8_t errorCode);
uint8_t UART_SendStripTypeAck(StripType_TypeDef stripType);

/* �o������ */
uint8_t UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t length);
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t length, uint8_t *data);
uint8_t UART_VerifyChecksum(uint8_t *packet, uint16_t length);

/* �����Д�̎���� */
void UART2_Receive_Byte_ISR(uint8_t byte);

#endif /* __UART_PROTOCOL_H */
