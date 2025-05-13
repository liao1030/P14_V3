/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : ԇƬ����ɜyģ�M�^�ļ�
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef STRIP_DETECT_H
#define STRIP_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"

/*********************************************************************
 * CONSTANTS
 */
// �΄��¼����x
#define STRIP_DETECT_EVT                 0x0020

// ԇƬ��Ͷ��x
#define STRIP_TYPE_GLV                   0x00
#define STRIP_TYPE_U                     0x01
#define STRIP_TYPE_C                     0x02
#define STRIP_TYPE_TG                    0x03
#define STRIP_TYPE_GAV                   0x04
#define STRIP_TYPE_UNKNOWN               0xFF

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * ��ʼ��ԇƬ�ɜyģ�M
 */
extern void StripDetect_Init(tmosTaskID task_id);

/*
 * ԇƬ�ɜy�¼�̎��
 */
extern uint16_t StripDetect_ProcessEvent(tmosTaskID task_id, uint16_t events);

/*
 * �O��ԇƬ��� (��MCU����ԇƬ��͕r�{��)
 */
extern void StripDetect_SetStripType(uint8_t type);

/*
 * �z��ԇƬ�Ƿ����
 */
extern bool StripDetect_IsStripInserted(void);

/*
 * �@ȡԇƬ���
 */
extern uint8_t StripDetect_GetStripType(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* STRIP_DETECT_H */
