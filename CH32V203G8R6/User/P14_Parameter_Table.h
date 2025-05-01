/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Parameter_Table.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : �๦�������yԇ�x�������a���x
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_PARAMETER_TABLE_H__
#define __P14_PARAMETER_TABLE_H__

#include "ch32v20x.h"

/* ��������С */
#define PARAM_TABLE_SIZE 678

/* 1. ϵ�y����������ַ���x */
#define PARAM_LBT            0    // ��늳��ֵ
#define PARAM_OBT            1    // 늳غıM�ֵ
#define PARAM_FACTORY        2    // �x������ģʽ
#define PARAM_MODEL_NO       3    // �aƷ��̖
#define PARAM_FW_NO          4    // �g�w�汾̖
#define PARAM_NOT            5    // �y���Δ� (2 bytes)
#define PARAM_CODE_TABLE_V   7    // ��̖��汾��̖ (2 bytes)

/* 2. �r�g�O��������ַ���x */
#define PARAM_YEAR           9    // ����O��
#define PARAM_MONTH          10   // �·��O��
#define PARAM_DATE           11   // �����O��
#define PARAM_HOUR           12   // С�r�O��
#define PARAM_MINUTE         13   // ����O��
#define PARAM_SECOND         14   // �딵�O��

/* 3. �y���h��������ַ���x */
#define PARAM_TLL            15   // �����ضȹ�������
#define PARAM_TLH            16   // �����ضȹ�������
#define PARAM_MGDL           17   // �y����λ�O��
#define PARAM_EVENT          18   // �{���¼����
#define PARAM_STRIP_TYPE     19   // �yԇ�Ŀ

/* 4. Ӳ�wУ��������ַ���x */
#define PARAM_EV_T3_TRG      20   // EV_T3�|�l늉� (2 bytes)
#define PARAM_EV_WORKING     22   // �y������늘O늉�
#define PARAM_EV_T3          23   // ѪҺ����늘O늉�
#define PARAM_DACO           24   // DACƫ������
#define PARAM_DACDO          25   // DACУ�����텢��
#define PARAM_CC211NO_DONE   26   // CC211δ������
#define PARAM_CAL_TOL        27   // OPS/OPIУ���ݲ�
#define PARAM_OPS            28   // OPAУ��б�� (4 bytes)
#define PARAM_OPI            32   // OPAУ���ؾ� (4 bytes)
#define PARAM_QCT            36   // QCTУ���y����λԪ�M
#define PARAM_TOFFSET        37   // �ض�У��ƫ��
#define PARAM_BOFFSET        38   // 늳�У��ƫ��

/* 5. Ѫ��(GLV/GAV)���Å�����ַ���x */
/* 5.1 ԇƬ���� */
#define PARAM_BG_CSU_TOL     39   // ԇƬ�z���ݲ�
#define PARAM_BG_NDL         40   // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_BG_UDL         42   // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_BG_BLOOD_IN    44   // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_BG_STRIP_LOT   46   // ԇƬ��̖ (16�ֹ�)

/* 5.2 �y���������� */
#define PARAM_BG_L           62   // Ѫ��ֵ����
#define PARAM_BG_H           63   // Ѫ��ֵ����
#define PARAM_BG_T3_E37      64   // T3 ADV��37�ֵ (2 bytes)

/* 5.3 �yԇ�r�򅢔� */
#define PARAM_BG_TPL_1       66   // �r�g�}�n����(��һ�M) (2 bytes)
#define PARAM_BG_TRD_1       68   // ԭʼ�����r�g(��һ�M) (2 bytes)
#define PARAM_BG_EV_WIDTH_1  70   // �۟��r�g(��һ�M) (2 bytes)
#define PARAM_BG_TPL_2       72   // �r�g�}�n����(�ڶ��M) (2 bytes)
#define PARAM_BG_TRD_2       74   // ԭʼ�����r�g(�ڶ��M) (2 bytes)
#define PARAM_BG_EV_WIDTH_2  76   // �۟��r�g(�ڶ��M) (2 bytes)

/* 5.4 Ʒ��Һ(QC)���� */
#define PARAM_BG_SQ          78   // QC���Ѕ���Sq
#define PARAM_BG_IQ          79   // QC���Ѕ���Iq
#define PARAM_BG_CVQ         80   // QCˮƽ��CV
#define PARAM_BG_AQ          81   // QC���Ѕ���A
#define PARAM_BG_BQ          82   // QC���Ѕ���B
#define PARAM_BG_AQ5         83   // QC���Ѕ���A(�ȼ�5)
#define PARAM_BG_BQ5         84   // QC���Ѕ���B(�ȼ�5)
#define PARAM_BG_QC          85   // QCģ�幠������ (30 bytes)

/* 5.5 Ӌ��ʽ�`���ȅ��� */
#define PARAM_BG_S2          115  // ԇƬ���Ѕ���
#define PARAM_BG_I2          116  // ԇƬ���Ѕ���
#define PARAM_BG_SR          117  // OPAӲ�wޒ·����(б��) (4 bytes)
#define PARAM_BG_IR          121  // OPAӲ�wޒ·����(�ؾ�) (4 bytes)
#define PARAM_BG_S3          125  // ԇƬ���Ѕ���S3 (30 bytes)
#define PARAM_BG_I3          155  // ԇƬ���Ѕ���I3 (30 bytes)

/* 5.6 �ض����Ѕ��� */
#define PARAM_BG_TF          185  // AC/PC�ض�����б�� (7 bytes)
#define PARAM_BG_CTF         192  // QC�ض�����б�� (7 bytes)
#define PARAM_BG_TO          199  // AC/PC�ض����нؾ� (7 bytes)
#define PARAM_BG_CTO         206  // QC�ض����нؾ� (7 bytes)

/* 6. ����(U)���Å�����ַ���x */
/* 6.1 ԇƬ���� */
#define PARAM_U_CSU_TOL      213  // ԇƬ�z���ݲ�
#define PARAM_U_NDL          214  // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_UDL          216  // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_BLOOD_IN     218  // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_U_STRIP_LOT    220  // ԇƬ��̖ (16�ֹ�)

/* 6.2 �y���������� */
#define PARAM_U_L            236  // ����ֵ����
#define PARAM_U_H            237  // ����ֵ����
#define PARAM_U_T3_E37       238  // T3 ADV��37�ֵ (2 bytes)

/* 6.3 �yԇ�r�򅢔� */
#define PARAM_U_TPL_1        240  // �r�g�}�n����(��һ�M) (2 bytes)
#define PARAM_U_TRD_1        242  // ԭʼ�����r�g(��һ�M) (2 bytes)
#define PARAM_U_EV_WIDTH_1   244  // �۟��r�g(��һ�M) (2 bytes)
#define PARAM_U_TPL_2        246  // �r�g�}�n����(�ڶ��M) (2 bytes)
#define PARAM_U_TRD_2        248  // ԭʼ�����r�g(�ڶ��M) (2 bytes)
#define PARAM_U_EV_WIDTH_2   250  // �۟��r�g(�ڶ��M) (2 bytes)

/* 6.4 Ʒ��Һ(QC)���� */
#define PARAM_U_SQ           252  // QC���Ѕ���Sq
#define PARAM_U_IQ           253  // QC���Ѕ���Iq
#define PARAM_U_CVQ          254  // QCˮƽ��CV
#define PARAM_U_AQ           255  // QC���Ѕ���A
#define PARAM_U_BQ           256  // QC���Ѕ���B
#define PARAM_U_QC           257  // QCģ�幠������ (30 bytes)

/* 6.5 Ӌ��ʽ�`���ȅ��� */
#define PARAM_U_S2           287  // ԇƬ���Ѕ���
#define PARAM_U_I2           288  // ԇƬ���Ѕ���
#define PARAM_U_SR           289  // OPAӲ�wޒ·����(б��) (4 bytes)
#define PARAM_U_IR           293  // OPAӲ�wޒ·����(�ؾ�) (4 bytes)
#define PARAM_U_S3           297  // ԇƬ���Ѕ���S3 (15 bytes)
#define PARAM_U_I3           312  // ԇƬ���Ѕ���I3 (15 bytes)

/* 6.6 �ض����Ѕ��� */
#define PARAM_U_TF           327  // AC/PC�ض�����б�� (7 bytes)
#define PARAM_U_CTF          334  // QC�ض�����б�� (7 bytes)
#define PARAM_U_TO           341  // AC/PC�ض����нؾ� (7 bytes)
#define PARAM_U_CTO          348  // QC�ض����нؾ� (7 bytes)

/* 7. ��đ�̴�(C)���Å�����ַ���x */
/* 7.1 ԇƬ���� */
#define PARAM_C_CSU_TOL      355  // ԇƬ�z���ݲ�
#define PARAM_C_NDL          356  // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_UDL          358  // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_BLOOD_IN     360  // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_C_STRIP_LOT    362  // ԇƬ��̖ (16�ֹ�)

/* 7.2 �y���������� */
#define PARAM_C_L            378  // đ�̴�ֵ����
#define PARAM_C_H            379  // đ�̴�ֵ����
#define PARAM_C_T3_E37       380  // T3 ADV��37�ֵ (2 bytes)

/* 7.3 �yԇ�r�򅢔� */
#define PARAM_C_TPL_1        382  // �r�g�}�n����(��һ�M) (2 bytes)
#define PARAM_C_TRD_1        384  // ԭʼ�����r�g(��һ�M) (2 bytes)
#define PARAM_C_EV_WIDTH_1   386  // �۟��r�g(��һ�M) (2 bytes)
#define PARAM_C_TPL_2        388  // �r�g�}�n����(�ڶ��M) (2 bytes)
#define PARAM_C_TRD_2        390  // ԭʼ�����r�g(�ڶ��M) (2 bytes)
#define PARAM_C_EV_WIDTH_2   392  // �۟��r�g(�ڶ��M) (2 bytes)

/* 7.4 Ʒ��Һ(QC)���� */
#define PARAM_C_SQ           394  // QC���Ѕ���Sq
#define PARAM_C_IQ           395  // QC���Ѕ���Iq
#define PARAM_C_CVQ          396  // QCˮƽ��CV
#define PARAM_C_AQ           397  // QC���Ѕ���A
#define PARAM_C_BQ           398  // QC���Ѕ���B
#define PARAM_C_QC           399  // QCģ�幠������ (30 bytes)

/* 7.5 Ӌ��ʽ�`���ȅ��� */
#define PARAM_C_S2           429  // ԇƬ���Ѕ���
#define PARAM_C_I2           430  // ԇƬ���Ѕ���
#define PARAM_C_SR           431  // OPAӲ�wޒ·����(б��) (4 bytes)
#define PARAM_C_IR           435  // OPAӲ�wޒ·����(�ؾ�) (4 bytes)
#define PARAM_C_S3           439  // ԇƬ���Ѕ���S3 (15 bytes)
#define PARAM_C_I3           454  // ԇƬ���Ѕ���I3 (15 bytes)

/* 7.6 �ض����Ѕ��� */
#define PARAM_C_TF           469  // AC/PC�ض�����б�� (7 bytes)
#define PARAM_C_CTF          476  // QC�ض�����б�� (7 bytes)
#define PARAM_C_TO           483  // AC/PC�ض����нؾ� (7 bytes)
#define PARAM_C_CTO          490  // QC�ض����нؾ� (7 bytes)

/* 8. �������֬(TG)���Å�����ַ���x */
/* 8.1 ԇƬ���� */
#define PARAM_TG_CSU_TOL     497  // ԇƬ�z���ݲ�
#define PARAM_TG_NDL         498  // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_UDL         500  // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_BLOOD_IN    502  // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_TG_STRIP_LOT   504  // ԇƬ��̖ (16�ֹ�)

/* 8.2 �y���������� */
#define PARAM_TG_L           520  // �������ֵ֬����
#define PARAM_TG_H           521  // �������ֵ֬����
#define PARAM_TG_T3_E37      522  // T3 ADV��37�ֵ (2 bytes)

/* 8.3 �yԇ�r�򅢔� */
#define PARAM_TG_TPL_1       524  // �r�g�}�n����(��һ�M) (2 bytes)
#define PARAM_TG_TRD_1       526  // ԭʼ�����r�g(��һ�M) (2 bytes)
#define PARAM_TG_EV_WIDTH_1  528  // �۟��r�g(��һ�M) (2 bytes)
#define PARAM_TG_TPL_2       530  // �r�g�}�n����(�ڶ��M) (2 bytes)
#define PARAM_TG_TRD_2       532  // ԭʼ�����r�g(�ڶ��M) (2 bytes)
#define PARAM_TG_EV_WIDTH_2  534  // �۟��r�g(�ڶ��M) (2 bytes)

/* 8.4 Ʒ��Һ(QC)���� */
#define PARAM_TG_SQ          536  // QC���Ѕ���Sq
#define PARAM_TG_IQ          537  // QC���Ѕ���Iq
#define PARAM_TG_CVQ         538  // QCˮƽ��CV
#define PARAM_TG_AQ          539  // QC���Ѕ���A
#define PARAM_TG_BQ          540  // QC���Ѕ���B
#define PARAM_TG_QC          541  // QCģ�幠������ (30 bytes)

/* 8.5 Ӌ��ʽ�`���ȅ��� */
#define PARAM_TG_S2          571  // ԇƬ���Ѕ���
#define PARAM_TG_I2          572  // ԇƬ���Ѕ���
#define PARAM_TG_SR          573  // OPAӲ�wޒ·����(б��) (4 bytes)
#define PARAM_TG_IR          577  // OPAӲ�wޒ·����(�ؾ�) (4 bytes)
#define PARAM_TG_S3          581  // ԇƬ���Ѕ���S3 (15 bytes)
#define PARAM_TG_I3          596  // ԇƬ���Ѕ���I3 (15 bytes)

/* 8.6 �ض����Ѕ��� */
#define PARAM_TG_TF          611  // AC/PC�ض�����б�� (7 bytes)
#define PARAM_TG_CTF         618  // QC�ض�����б�� (7 bytes)
#define PARAM_TG_TO          625  // AC/PC�ض����нؾ� (7 bytes)
#define PARAM_TG_CTO         632  // QC�ض����нؾ� (7 bytes)

/* 9. �����cУ�^ */
#define PARAM_RESERVED       639  // ����^��δ��Uչ (36 bytes)
#define PARAM_SUM_L          675  // ��ַ0~674У��(��)
#define PARAM_SUM_H          676  // ��ַ0~674У��(��)
#define PARAM_CRC16          677  // ����������CRCУ�

/* �yԇ�Ŀ��Ͷ��x */
typedef enum {
    STRIP_TYPE_GLV = 0,   // Ѫ��
    STRIP_TYPE_U   = 1,   // ����
    STRIP_TYPE_C   = 2,   // ��đ�̴�
    STRIP_TYPE_TG  = 3,   // �������֬
    STRIP_TYPE_GAV = 4,   // Ѫ��(��һ�N����)
    STRIP_TYPE_UNKNOWN = 255 // δ֪���e�`
} StripType_TypeDef;

/* �¼���Ͷ��x */
typedef enum {
    EVENT_QC = 0,  // Ʒ��Һ�yԇ
    EVENT_AC = 1,  // ��ǰ�yԇ
    EVENT_PC = 2   // ����yԇ
} Event_TypeDef;

/* �y����λ���x */
typedef enum {
    UNIT_MMOL_L = 0,  // mmol/L
    UNIT_MG_DL  = 1,  // mg/dL
    UNIT_GM_DL  = 2   // gm/dl
} Unit_TypeDef;

/* ����ģʽ���x */
typedef enum {
    MODE_USER    = 0,    // ʹ����ģʽ
    MODE_FACTORY = 210   // ���Sģʽ
} Mode_TypeDef;

/* ������ */
void P14_ParamTable_Init(void);
void P14_ParamTable_LoadDefault(void);
uint8_t P14_ParamTable_Read(uint16_t address);
void P14_ParamTable_Write(uint16_t address, uint8_t value);
uint16_t P14_ParamTable_Read16(uint16_t address);
void P14_ParamTable_Write16(uint16_t address, uint16_t value);
float P14_ParamTable_ReadFloat(uint16_t address);
void P14_ParamTable_WriteFloat(uint16_t address, float value);
void P14_ParamTable_ReadBlock(uint16_t address, uint8_t *buffer, uint16_t length);
void P14_ParamTable_WriteBlock(uint16_t address, const uint8_t *buffer, uint16_t length);
uint8_t P14_ParamTable_VerifyChecksum(void);
void P14_ParamTable_UpdateChecksum(void);
void P14_ParamTable_Save(void);
void P14_ParamTable_Load(void);

/* �����ӵĺ����� */
const char* P14_ParamTable_GetStripTypeName(StripType_TypeDef type);
const char* P14_ParamTable_GetEventName(Event_TypeDef event);
const char* P14_ParamTable_GetUnitName(Unit_TypeDef unit);
void P14_ParamTable_IncrementTestCount(void);
void P14_ParamTable_ResetTestCount(void);

#endif /* __P14_PARAMETER_TABLE_H__ */ 