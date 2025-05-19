/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_utils.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x�������ߺ����^�ļ�
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_UTILS_H
#define __PARAM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "param_table.h"
#include "param_code_table.h"

/* �y����λö�e */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL = 1,       // mg/dL
    UNIT_GM_DL = 2,       // gm/dL
    UNIT_MAX
} Unit_TypeDef;

/* �¼����ö�e */
typedef enum {
    EVENT_NONE = 0,               // �o�¼�
    EVENT_BEFORE_MEAL = 1,        // �ǰ
    EVENT_AFTER_MEAL = 2,         // ���
    EVENT_FASTING = 3,            // �ո�
    EVENT_BEDTIME = 4,            // ˯ǰ
    EVENT_AC = 5,                 // AC
    EVENT_PC = 6,                 // PC
    EVENT_QC = 7,                 // QC (Ʒ��Һ)
    EVENT_MAX
} Event_TypeDef;

/* �o������ԭ�� */
uint8_t PARAM_GetByte(uint16_t paramAddress);
uint16_t PARAM_GetWord(uint16_t paramAddress);
float PARAM_GetFloat(uint16_t paramAddress);
ParamError_TypeDef PARAM_SetByte(uint16_t paramAddress, uint8_t value);
ParamError_TypeDef PARAM_SetWord(uint16_t paramAddress, uint16_t value);
ParamError_TypeDef PARAM_SetFloat(uint16_t paramAddress, float value);
ParamError_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date,
                                    uint8_t *hour, uint8_t *minute, uint8_t *second);
ParamError_TypeDef PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date,
                                    uint8_t hour, uint8_t minute, uint8_t second);
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);
const char* Event_GetName(Event_TypeDef event);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_UTILS_H */
