/********************************** (C) COPYRIGHT *******************************
 * File Name          : strip_detect.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : ԇƬ����ɜy�c�Ԅ��ЄeԇƬ������P����
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef __STRIP_DETECT_H
#define __STRIP_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"
#include "ch32v20x.h"
#include "param_table.h"
#include "ch32v20x_tim.h"  // ���TIM������
#include "system_state.h"   // ϵ�y��B���x

/* ԇƬ��B���x */
typedef enum {
    STRIP_STATE_NONE = 0,      // δ����
    STRIP_STATE_INSERTED = 1,  // �Ѳ��룬���Єe
    STRIP_STATE_DETECTED = 2,  // ���Єe���
    STRIP_STATE_ERROR = 3      // �e�`��B
} StripState_TypeDef;

/* SystemState_TypeDef ���x�� system_state.h �� */

/* ԇƬ�����B�Y�� */
typedef struct {
    StripState_TypeDef state;         // ԇƬ��B
    StripType_TypeDef type;           // ԇƬ���
    uint8_t pin3Status;               // ��3�_��B
    uint8_t pin5Status;               // ��5�_��B
    float t1Voltage;                  // T1_OUT늉�
    uint32_t insertTime;              // ����r�g��
    uint8_t detectionComplete;        // �ɜy��ɘ��I
    uint8_t ackSent;                  // �ؑ��Ѱl��
} StripDetectInfo_TypeDef;

/* �������� */
void STRIP_DETECT_Init(void);
void STRIP_DETECT_Process(void);
uint8_t STRIP_DETECT_IsInserted(void);
StripType_TypeDef STRIP_DETECT_GetStripType(void);
void STRIP_DETECT_HandleInsertedEvent(void);
void STRIP_DETECT_SetStripType(StripType_TypeDef type);
void STRIP_DETECT_SetPinStatus(uint8_t pin3, uint8_t pin5);

/* �ⲿ������ */
extern void Delay_Ms(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* __STRIP_DETECT_H */
