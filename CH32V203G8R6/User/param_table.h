/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/08
 * Description        : �๦�������yԇ�x�������a��
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

/* �������С���x */
#define PARAM_TABLE_SIZE      678    // ��������С��0~677��

/* ϵ�y����������ַ���x */
#define PARAM_LBT             0      // ��늳��ֵ
#define PARAM_OBT             1      // 늳غıM�ֵ
#define PARAM_FACTORY         2      // �x������ģʽ
#define PARAM_MODEL_NO        3      // �aƷ��̖
#define PARAM_FW_NO           4      // �g�w�汾̖
#define PARAM_NOT             5      // �yԇ�Δ� (2 bytes)
#define PARAM_CODE_TABLE_V    7      // ���a��汾̖ (2 bytes)

/* �r�g�O��������ַ���x */
#define PARAM_YEAR            9      // ����O��
#define PARAM_MONTH           10     // �·��O��
#define PARAM_DATE            11     // �����O��
#define PARAM_HOUR            12     // С�r�O��
#define PARAM_MINUTE          13     // ����O��
#define PARAM_SECOND          14     // �딵�O��

/* �yԇ�l��������ַ���x */
#define PARAM_TLL             15     // �����ضȹ�������
#define PARAM_TLH             16     // �����ضȹ�������
#define PARAM_MGDL            17     // ��Ȇ�λ�O��
#define PARAM_EVENT           18     // ���y�¼����
#define PARAM_STRIP_TYPE      19     // �yԇ�Ŀ

/* Ӳ�wУ�ʅ�����ַ���x */
#define PARAM_EV_T3_TRG       20     // EV_T3�|�l늉� (2 bytes)
#define PARAM_EV_WORKING      22     // ��ȹ���늉�늉�
#define PARAM_EV_T3           23     // ѪҺ�y��늘O늉�
#define PARAM_DACO            24     // DACƫ���a��
#define PARAM_DACDO           25     // DACУ�����텢��
#define PARAM_CC211NODONE     26     // CC211δ������
#define PARAM_CAL_TOL         27     // OPS/OPIУ���ݲ�
#define PARAM_OPS             28     // OPAУ��б�� (4 bytes)
#define PARAM_OPI             32     // OPAУ�ʽؾ� (4 bytes)
#define PARAM_QCT             36     // QCTУ�ʜyԇ��λԪ�M
#define PARAM_TOFFSET         37     // �ض�У��ƫ��
#define PARAM_BOFFSET         38     // 늳�У��ƫ��

/* Ѫ��(GLV/GAV)���Å�����ַ���x */
#define PARAM_BG_CSU_TOL      39     // ԇƬ�z���ݲ�
#define PARAM_BG_NDL          40     // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_BG_UDL          42     // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_BG_BLOOD_IN     44     // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_BG_STRIP_LOT    46     // ԇƬ��̖(16��Ԫ) (16 bytes)
#define PARAM_BG_L            62     // Ѫ��ֵ����
#define PARAM_BG_H            63     // Ѫ��ֵ����
#define PARAM_BG_T3_E37       64     // T3 ADV��E37�ֵ (2 bytes)
/* ... ����Ѫ�ǅ������x ... */

/* ����(U)���Å�����ַ���x */
#define PARAM_U_CSU_TOL       213    // ԇƬ�z���ݲ�
#define PARAM_U_NDL           214    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_UDL           216    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_BLOOD_IN      218    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_U_STRIP_LOT     220    // ԇƬ��̖(16��Ԫ) (16 bytes)
#define PARAM_U_L             236    // ����ֵ����
#define PARAM_U_H             237    // ����ֵ����
#define PARAM_U_T3_E37        238    // T3 ADV��E37�ֵ (2 bytes)
/* ... �������ᅢ�����x ... */

/* ��đ�̴�(C)���Å�����ַ���x */
#define PARAM_C_CSU_TOL       355    // ԇƬ�z���ݲ�
#define PARAM_C_NDL           356    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_UDL           358    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_BLOOD_IN      360    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_C_STRIP_LOT     362    // ԇƬ��̖(16��Ԫ) (16 bytes)
#define PARAM_C_L             378    // ��đ�̴�ֵ����
#define PARAM_C_H             379    // ��đ�̴�ֵ����
#define PARAM_C_T3_E37        380    // T3 ADV��E37�ֵ (2 bytes)
/* ... ���࿂đ�̴��������x ... */

/* �������֬(TG)���Å�����ַ���x */
#define PARAM_TG_CSU_TOL      497    // ԇƬ�z���ݲ�
#define PARAM_TG_NDL          498    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_UDL          500    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_BLOOD_IN     502    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_TG_STRIP_LOT    504    // ԇƬ��̖(16��Ԫ) (16 bytes)
#define PARAM_TG_L            520    // �������ֵ֬����
#define PARAM_TG_H            521    // �������ֵ֬����
#define PARAM_TG_T3_E37       522    // T3 ADV��E37�ֵ (2 bytes)
/* ... �����������֬�������x ... */

/* �����cУ�^ */
#define PARAM_RESERVED        639    // ����^��δ��Uչ (36 bytes)
#define PARAM_SUM_L           675    // ��ַ0~674У��(��)
#define PARAM_SUM_H           676    // ��ַ0~674У��(��)
#define PARAM_CRC16           677    // ����������CRCУ�

/* ԇƬ��Ͷ��x */
typedef enum {
    STRIP_TYPE_GLV = 0,   // Ѫ��
    STRIP_TYPE_U = 1,     // ����
    STRIP_TYPE_C = 2,     // ��đ�̴�
    STRIP_TYPE_TG = 3,    // �������֬
    STRIP_TYPE_GAV = 4,   // Ѫ��(��һ�N)
    STRIP_TYPE_MAX = 5
} StripType_TypeDef;

/* ��Ȇ�λ���x */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL = 1,       // mg/dL
    UNIT_GM_DL = 2,       // gm/dl
    UNIT_MAX = 3
} Unit_TypeDef;

/* �¼���Ͷ��x */
typedef enum {
    EVENT_QC = 0,         // Ʒ��Һ
    EVENT_AC = 1,         // �ǰ 
    EVENT_PC = 2,         // ���
    EVENT_MAX = 3
} Event_TypeDef;

/* �����x�������� */
void PARAM_Init(void);
uint8_t PARAM_GetByte(uint16_t addr);
void PARAM_SetByte(uint16_t addr, uint8_t value);
uint16_t PARAM_GetWord(uint16_t addr);
void PARAM_SetWord(uint16_t addr, uint16_t value);
float PARAM_GetFloat(uint16_t addr);
void PARAM_SetFloat(uint16_t addr, float value);
void PARAM_GetString(uint16_t addr, uint8_t *buffer, uint8_t length);
void PARAM_SetString(uint16_t addr, uint8_t *buffer, uint8_t length);
uint8_t PARAM_VerifyChecksum(void);
void PARAM_UpdateChecksum(void);
void PARAM_SaveToFlash(void);
void PARAM_LoadFromFlash(void);
void PARAM_SetDefault(void);

/* ���ߺ����� */
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);

#endif /* __PARAM_TABLE_H */