#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#include "ch32v20x.h"
#include "ch32v20x_usart.h"
#include "param_code_table.h"
#include "flash_param.h"  /* ��� flash_param.h �ԫ@ȡ FLASH_ParamResult_TypeDef ��Ͷ��x */

/* ͨӍ�f�h��ӛ�c�L�ȶ��x */
#define PROTOCOL_START_MARK      0xAA
#define PROTOCOL_END_MARK        0x55
#define PROTOCOL_HEADER_LEN      3     // ��ʼ��ӛ + ָ��ID + �L��
#define PROTOCOL_FOOTER_LEN      2     // У�� + �Y����ӛ
#define PROTOCOL_MAX_DATA_LEN    128   // �Y�ϙ�λ����L��
#define PROTOCOL_MAX_PACKET_LEN  (PROTOCOL_HEADER_LEN + PROTOCOL_MAX_DATA_LEN + PROTOCOL_FOOTER_LEN)

/* ͨӍָ��ID���x */
/* ����ָ�� (��APP��Meter) */
#define CMD_SYNC_TIME            0x01  // ͬ���r�g
#define CMD_REQ_STATUS           0x02  // Ո���b�à�B
#define CMD_SET_CODE_EVENT       0x03  // �O��CODE��EVENT
#define CMD_CHECK_BLOOD          0x04  // �z�y��BՈ��
#define CMD_REQ_RESULT           0x05  // Ո��yԇ�Y��
#define CMD_REQ_RAW_DATA         0x06  // Ո��RAW DATA

/* �ؑ�ָ�� (��Meter��APP) */
#define CMD_SYNC_TIME_ACK        0x81  // �r�gͬ���_�J
#define CMD_STATUS_ACK           0x82  // �b�à�B�ؑ�
#define CMD_CODE_EVENT_ACK       0x83  // CODE/EVENT�O���_�J
#define CMD_BLOOD_DETECTED_ACK   0x84  // ѪҺ�z�y֪ͨ
#define CMD_RESULT_ACK           0x85  // �yԇ�Y���ؑ�
#define CMD_RAW_DATA_ACK         0x86  // RAW DATA�ؑ�
#define CMD_ERROR_ACK            0xFF  // �e�`�ؑ�

/* �e�`���a���x */
#define ERR_LOW_BATTERY          0x01  // 늳�����^��
#define ERR_TEMP_HIGH            0x02  // �ض��^��
#define ERR_TEMP_LOW             0x03  // �ض��^��
#define ERR_STRIP_EXPIRED        0x04  // ԇƬ�^�ڻ�p��
#define ERR_STRIP_USED           0x05  // ԇƬ��ʹ��
#define ERR_STRIP_ERROR          0x06  // ԇƬ�����e�`
#define ERR_BLOOD_INSUFFICIENT   0x07  // ѪҺ�ӱ�����
#define ERR_TIMEOUT              0x08  // �y�����r
#define ERR_CALIBRATION          0x09  // У���e�`
#define ERR_HARDWARE             0x0A  // Ӳ�w�e�`
#define ERR_COMMUNICATION        0x0B  // ͨӍ�e�`
#define ERR_DATA_FORMAT          0x0C  // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM             0x0D  // У���e�`
#define ERR_COMMAND_UNSUPPORTED  0x0E  // ָ�֧Ԯ
#define ERR_RESULT_OUT_OF_RANGE  0x0F  // �y���Y����������

/* �¼���Ͷ��x */
#define EVENT_NONE               0x0000  // �o
#define EVENT_AC                 0x0001  // �ǰ
#define EVENT_PC                 0x0002  // ���
#define EVENT_QC                 0x0003  // QC

/* �yԇ�Ŀ��Ͷ��x */
#define STRIP_TYPE_GLV           0x0000  // Ѫ��
#define STRIP_TYPE_U             0x0001  // ����
#define STRIP_TYPE_C             0x0002  // ��đ�̴�
#define STRIP_TYPE_TG            0x0003  // �������֬
#define STRIP_TYPE_GAV           0x0004  // Ѫ��(��һ�N)

/* �yԇӛ䛽Y�����x */
typedef struct {
    uint16_t EVENT;            // �¼����
    uint8_t Strip_Code;        // ԇƬ��̖�a
} TestRecord_TypeDef;

/* ͨӍ�f�h�Y���w���x */
typedef struct {
    uint8_t startMark;           // ��ʼ��ӛ (0xAA)
    uint8_t cmdId;               // ָ��ID
    uint8_t dataLen;             // �Y���L��
    uint8_t data[PROTOCOL_MAX_DATA_LEN]; // �Y��
    uint8_t checksum;            // У��
    uint8_t endMark;             // �Y����ӛ (0x55)
} UartProtocol_TypeDef;

/* UART ���ՠ�B�C���x */
typedef enum {
    UART_RX_WAIT_START,          // �ȴ���ʼ��ӛ
    UART_RX_WAIT_CMD,            // �ȴ�ָ��ID
    UART_RX_WAIT_LEN,            // �ȴ��Y���L��
    UART_RX_WAIT_DATA,           // �ȴ��Y��
    UART_RX_WAIT_CHECKSUM,       // �ȴ�У��
    UART_RX_WAIT_END             // �ȴ��Y����ӛ
} UartRxState_TypeDef;

/* ���ܺ������� */
void UART_Protocol_Init(void);
void UART_Protocol_Process(uint8_t rxByte);
void UART_Send_Packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
void UART_Send_Error(uint8_t originalCmd, uint8_t errorCode);
uint8_t UART_Calculate_Checksum(uint8_t cmdId, uint8_t dataLen, uint8_t *data);

/* ָ��̎�������� */
void UART_Handle_SyncTime(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqStatus(uint8_t *data, uint8_t dataLen);
void UART_Handle_SetCodeEvent(uint8_t *data, uint8_t dataLen);
void UART_Handle_CheckBlood(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqResult(uint8_t *data, uint8_t dataLen);
void UART_Handle_ReqRawData(uint8_t *data, uint8_t dataLen);

/* Flash�������溯������ */
FLASH_ParamResult_TypeDef PARAM_SaveParameters(void);
FLASH_ParamResult_TypeDef PARAM_SaveTime(void);

#endif /* __UART_PROTOCOL_H */