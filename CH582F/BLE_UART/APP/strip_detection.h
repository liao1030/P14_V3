/**************************************************************************************************
 * �n�����Q: strip_detection.h
 * �����f��: ԇƬ����ɜy���P���x�ͺ�����
 **************************************************************************************************/

#ifndef __STRIP_DETECTION_H
#define __STRIP_DETECTION_H

#include "CH58x_common.h"

// ԇƬ��Ͷ��x
typedef enum {
    STRIP_TYPE_UNKNOWN = 0,  // δ֪���
    STRIP_TYPE_GLV,          // Ѫ��(GLVԇƬ)
    STRIP_TYPE_U,            // ����
    STRIP_TYPE_C,            // ��đ�̴�
    STRIP_TYPE_TG,           // �������֬
    STRIP_TYPE_GAV           // Ѫ��(GAVԇƬ)
} StripType_t;

// ԇƬ�����B���x
typedef enum {
    STRIP_STATUS_NONE = 0,   // �oԇƬ
    STRIP_STATUS_INSERTED,   // �Ѳ���
    STRIP_STATUS_READY,      // �ʂ�;w
    STRIP_STATUS_TESTING,    // �yԇ��
    STRIP_STATUS_COMPLETE    // �yԇ���
} StripStatus_t;

// ��ʼ��ԇƬ����ɜy
void Strip_Detection_Init(void);

// �z��ԇƬ�����B(̎�������)
void Strip_CheckInsertionStatus(void);

// �@ȡԇƬ��B
StripStatus_t Strip_GetStatus(void);

// �@ȡԇƬ���
StripType_t Strip_GetType(void);

// �O��ԇƬ��� (��CH32V203ͨ�^UART֪ͨ)
void Strip_SetType(StripType_t type);

// �yԇ���̎����
void Strip_TestComplete(void);

#endif /* __STRIP_DETECTION_H */