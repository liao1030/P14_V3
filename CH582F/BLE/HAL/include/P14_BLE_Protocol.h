/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_BLE_Protocol.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14�ĿCH582F�{��ͨӍ�f�h���x
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_BLE_PROTOCOL_H__
#define __P14_BLE_PROTOCOL_H__

#include "CH58x_common.h"

/* CH582F-CH32V203ͨӍ����x */
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

/* �f�h̎���� */
void P14_BLE_ProtocolInit(void);
void P14_BLE_UartCallback(uint8_t *data, uint16_t length);

#endif /* __P14_BLE_PROTOCOL_H__ */ 