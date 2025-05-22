/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_state.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/22
 * Description        : ϵ�y��B�C���P���x
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ϵ�y��B���x */
typedef enum {
    STATE_IDLE,                // ���f��B
    STATE_STRIP_DETECTED,      // �z�y��ԇƬ
    STATE_STRIP_VALIDATION,    // ԇƬ��C�A��
    STATE_PARAMETER_SETUP,     // �����O���A��
    STATE_WAIT_FOR_BLOOD,      // �ȴ�ѪҺ����
    STATE_MEASURING,           // �y����
    STATE_RESULT_READY,        // �Y���ʂ��
    STATE_ERROR                // �e�`��B
} SystemState_TypeDef;

/* ������ */
void System_SetState(SystemState_TypeDef newState);
SystemState_TypeDef System_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_STATE_H */
