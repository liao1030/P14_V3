/**
 * @file strip_detection.h
 * @brief ԇƬ����Єe���P���x�ͺ�����
 */

#ifndef __STRIP_DETECTION_H
#define __STRIP_DETECTION_H

#include "ch32v20x.h"

// ԇƬ��Ͷ��x
typedef enum {
    STRIP_TYPE_DET_UNKNOWN = 0,  // δ֪���
    STRIP_TYPE_DET_GLV,          // Ѫ��(GLVԇƬ)
    STRIP_TYPE_DET_U,            // ����
    STRIP_TYPE_DET_C,            // ��đ�̴�
    STRIP_TYPE_DET_TG,           // �������֬
    STRIP_TYPE_DET_GAV           // Ѫ��(GAVԇƬ)
} StripType_t;

// ԇƬ�����B���x
typedef enum {
    STRIP_STATUS_NONE = 0,   // �oԇƬ
    STRIP_STATUS_INSERTED,   // �Ѳ���
    STRIP_STATUS_IDENTIFIED, // ���R�e���
    STRIP_STATUS_TESTING,    // �yԇ��
    STRIP_STATUS_COMPLETE    // �yԇ���
} StripStatus_t;

// ͨӍ����x
#define CMD_WAKEUP_MCU              0xA1    // ����CH32V203������
#define CMD_STRIP_INSERTED          0xA2    // ֪ͨԇƬ�Ѳ��������
#define CMD_REQUEST_STRIP_TYPE      0xA3    // Ո��ԇƬ����Єe������
#define CMD_STRIP_TYPE_RESULT       0xB1    // ԇƬ����Єe�Y��������

// ��ʼ��ԇƬ����Єe����
void Strip_Detection_Init(void);

// �_ʼԇƬ����Єe
void Strip_StartTypeDetection(void);

// �@ȡԇƬ��B
StripStatus_t Strip_GetStatus(void);

// �@ȡԇƬ���
StripType_t Strip_GetType(void);

// UART����̎����
void Strip_UART_Process(uint8_t receivedByte);

#endif /* __STRIP_DETECTION_H */