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
#define PARAM_EV_WORKING      22     // ��ȹ���늉�
#define PARAM_EV_T3           23     // ѪҺ�y��늘O늉�
#define PARAM_DACO            24     // DACƫ���a��
#define PARAM_DACDO           25     // DACУ�����텢��
#define PARAM_CC211NODONE     26     // CC211�������
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
#define PARAM_BG_STRIP_LOT    46     // ԇƬ��̖ (16 bytes)
#define PARAM_BG_L            62     // Ѫ��ֵ����
#define PARAM_BG_H            63     // Ѫ��ֵ����
#define PARAM_BG_T3_E37       64     // T3 ADV��E37�ֵ (2 bytes)
/* Ѫ�ǜyԇ�r�򅢔� */
#define PARAM_BG_TPL1         66     // �yԇ�g��1 (2 bytes)
#define PARAM_BG_TRD1         68     // �xȡ���t1 (2 bytes)
#define PARAM_BG_EVWIDTH1     70     // ����늉�����1 (2 bytes)
#define PARAM_BG_TPL2         72     // �yԇ�g��2 (2 bytes)
#define PARAM_BG_TRD2         74     // �xȡ���t2 (2 bytes)
#define PARAM_BG_EVWIDTH2     76     // ����늉�����2 (2 bytes)
/* Ѫ�ǘ˜�Ʒ�������� */
#define PARAM_BG_CAL_A1       78     // �˜�Ʒ�����S��A1 (4 bytes)
#define PARAM_BG_CAL_B1       82     // �˜�Ʒ�����S��B1 (4 bytes)
#define PARAM_BG_CAL_C1       86     // �˜�Ʒ�����S��C1 (4 bytes)
/* Ѫ�ǜض��a������ */
#define PARAM_BG_TEMP_A1      90     // �ض��a���S��A1 (4 bytes)
#define PARAM_BG_TEMP_B1      94     // �ض��a���S��B1 (4 bytes)
#define PARAM_BG_TEMP_C1      98     // �ض��a���S��C1 (4 bytes)
/* Ѫ��Ѫ���a������ */
#define PARAM_BG_BLOOD_A1     102    // Ѫ���a���S��A1 (4 bytes)
#define PARAM_BG_BLOOD_B1     106    // Ѫ���a���S��B1 (4 bytes)
#define PARAM_BG_BLOOD_C1     110    // Ѫ���a���S��C1 (4 bytes)
/* Ѫ�ǜyԇ��������^ */
#define PARAM_BG_RESERVED     114    // ����^ (99 bytes)

/* ����(U)���Å�����ַ���x */
#define PARAM_U_CSU_TOL       213    // ԇƬ�z���ݲ�
#define PARAM_U_NDL           214    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_UDL           216    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_U_BLOOD_IN      218    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_U_STRIP_LOT     220    // ԇƬ��̖ (16 bytes)
#define PARAM_U_L             236    // ����ֵ����
#define PARAM_U_H             237    // ����ֵ����
#define PARAM_U_T3_E37        238    // T3 ADV��E37�ֵ (2 bytes)
/* ����yԇ�r�򅢔� */
#define PARAM_U_TPL1          240    // �yԇ�g��1 (2 bytes)
#define PARAM_U_TRD1          242    // �xȡ���t1 (2 bytes)
#define PARAM_U_EVWIDTH1      244    // ����늉�����1 (2 bytes)
#define PARAM_U_TPL2          246    // �yԇ�g��2 (2 bytes)
#define PARAM_U_TRD2          248    // �xȡ���t2 (2 bytes)
#define PARAM_U_EVWIDTH2      250    // ����늉�����2 (2 bytes)
/* ����˜�Ʒ�������� */
#define PARAM_U_CAL_A1        252    // �˜�Ʒ�����S��A1 (4 bytes)
#define PARAM_U_CAL_B1        256    // �˜�Ʒ�����S��B1 (4 bytes)
#define PARAM_U_CAL_C1        260    // �˜�Ʒ�����S��C1 (4 bytes)
/* ����ض��a������ */
#define PARAM_U_TEMP_A1       264    // �ض��a���S��A1 (4 bytes)
#define PARAM_U_TEMP_B1       268    // �ض��a���S��B1 (4 bytes)
#define PARAM_U_TEMP_C1       272    // �ض��a���S��C1 (4 bytes)
/* ����Ѫ���a������ */
#define PARAM_U_BLOOD_A1      276    // Ѫ���a���S��A1 (4 bytes)
#define PARAM_U_BLOOD_B1      280    // Ѫ���a���S��B1 (4 bytes)
#define PARAM_U_BLOOD_C1      284    // Ѫ���a���S��C1 (4 bytes)
/* ����yԇ��������^ */
#define PARAM_U_RESERVED      288    // ����^ (67 bytes)

/* ��đ�̴�(C)���Å�����ַ���x */
#define PARAM_C_CSU_TOL       355    // ԇƬ�z���ݲ�
#define PARAM_C_NDL           356    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_UDL           358    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_C_BLOOD_IN      360    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_C_STRIP_LOT     362    // ԇƬ��̖ (16 bytes)
#define PARAM_C_L             378    // ��đ�̴�ֵ����
#define PARAM_C_H             379    // ��đ�̴�ֵ���� (2 bytes)
#define PARAM_C_T3_E37        381    // T3 ADV��E37�ֵ (2 bytes)
/* ��đ�̴��yԇ�r�򅢔� */
#define PARAM_C_TPL1          383    // �yԇ�g��1 (2 bytes)
#define PARAM_C_TRD1          385    // �xȡ���t1 (2 bytes)
#define PARAM_C_EVWIDTH1      387    // ����늉�����1 (2 bytes)
#define PARAM_C_TPL2          389    // �yԇ�g��2 (2 bytes)
#define PARAM_C_TRD2          391    // �xȡ���t2 (2 bytes)
#define PARAM_C_EVWIDTH2      393    // ����늉�����2 (2 bytes)
/* ��đ�̴��˜�Ʒ�������� */
#define PARAM_C_CAL_A1        395    // �˜�Ʒ�����S��A1 (4 bytes)
#define PARAM_C_CAL_B1        399    // �˜�Ʒ�����S��B1 (4 bytes)
#define PARAM_C_CAL_C1        403    // �˜�Ʒ�����S��C1 (4 bytes)
/* ��đ�̴��ض��a������ */
#define PARAM_C_TEMP_A1       407    // �ض��a���S��A1 (4 bytes)
#define PARAM_C_TEMP_B1       411    // �ض��a���S��B1 (4 bytes)
#define PARAM_C_TEMP_C1       415    // �ض��a���S��C1 (4 bytes)
/* ��đ�̴�Ѫ���a������ */
#define PARAM_C_BLOOD_A1      419    // Ѫ���a���S��A1 (4 bytes)
#define PARAM_C_BLOOD_B1      423    // Ѫ���a���S��B1 (4 bytes)
#define PARAM_C_BLOOD_C1      427    // Ѫ���a���S��C1 (4 bytes)
/* ��đ�̴��yԇ��������^ */
#define PARAM_C_RESERVED      431    // ����^ (66 bytes)

/* �������֬(TG)���Å�����ַ���x */
#define PARAM_TG_CSU_TOL      497    // ԇƬ�z���ݲ�
#define PARAM_TG_NDL          498    // ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_UDL          500    // ��ʹ��ԇƬ�y��ˮƽ (2 bytes)
#define PARAM_TG_BLOOD_IN     502    // ѪҺ�y��ˮƽ (2 bytes)
#define PARAM_TG_STRIP_LOT    504    // ԇƬ��̖ (16 bytes)
#define PARAM_TG_L            520    // �������ֵ֬����
#define PARAM_TG_H            521    // �������ֵ֬���� (2 bytes)
#define PARAM_TG_T3_E37       523    // T3 ADV��E37�ֵ (2 bytes)
/* �������֬�yԇ�r�򅢔� */
#define PARAM_TG_TPL1         525    // �yԇ�g��1 (2 bytes)
#define PARAM_TG_TRD1         527    // �xȡ���t1 (2 bytes)
#define PARAM_TG_EVWIDTH1     529    // ����늉�����1 (2 bytes)
#define PARAM_TG_TPL2         531    // �yԇ�g��2 (2 bytes)
#define PARAM_TG_TRD2         533    // �xȡ���t2 (2 bytes)
#define PARAM_TG_EVWIDTH2     535    // ����늉�����2 (2 bytes)
/* �������֬�˜�Ʒ�������� */
#define PARAM_TG_CAL_A1       537    // �˜�Ʒ�����S��A1 (4 bytes)
#define PARAM_TG_CAL_B1       541    // �˜�Ʒ�����S��B1 (4 bytes)
#define PARAM_TG_CAL_C1       545    // �˜�Ʒ�����S��C1 (4 bytes)
/* �������֬�ض��a������ */
#define PARAM_TG_TEMP_A1      549    // �ض��a���S��A1 (4 bytes)
#define PARAM_TG_TEMP_B1      553    // �ض��a���S��B1 (4 bytes)
#define PARAM_TG_TEMP_C1      557    // �ض��a���S��C1 (4 bytes)
/* �������֬Ѫ���a������ */
#define PARAM_TG_BLOOD_A1     561    // Ѫ���a���S��A1 (4 bytes)
#define PARAM_TG_BLOOD_B1     565    // Ѫ���a���S��B1 (4 bytes)
#define PARAM_TG_BLOOD_C1     569    // Ѫ���a���S��C1 (4 bytes)
/* �������֬�yԇ��������^ */
#define PARAM_TG_RESERVED     573    // ����^ (66 bytes)

/* ����^�� */
#define PARAM_RESERVED        639    // ϵ�y���� (37 bytes)

/* У�^�� */
#define PARAM_SUM_L           676    // У�͵�λԪ�M
#define PARAM_SUM_H           677    // У�͸�λԪ�M
#define PARAM_CRC16           675    // CRC16У�λԪ�M

/* ԇƬ��Ͷ��x */
typedef enum {
    STRIP_TYPE_GLV = 0,   // Ѫ��ԇƬ(����)
    STRIP_TYPE_U,         // ����ԇƬ
    STRIP_TYPE_C,         // ��đ�̴�ԇƬ
    STRIP_TYPE_TG,        // �������֬ԇƬ
    STRIP_TYPE_GAV,       // Ѫ��ԇƬ(�߾���)
    STRIP_TYPE_MAX
} StripType_TypeDef;

/* ��λ���x */
typedef enum {
    UNIT_MMOL_L = 0,      // mmol/L
    UNIT_MG_DL,           // mg/dL
    UNIT_GM_DL,           // g/dL
    UNIT_MAX
} Unit_TypeDef;

/* �¼���Ͷ��x */
typedef enum {
    EVENT_GEN = 0,        // һ��yԇ
    EVENT_AC,             // ��ǰ�yԇ
    EVENT_PC,             // ����yԇ
    EVENT_QC1,            // Ʒ��Һ1
    EVENT_QC2,            // Ʒ��Һ2
    EVENT_QC3,            // Ʒ��Һ3
    EVENT_MAX
} Event_TypeDef;

/* ���܅^�K���x */
typedef enum {
    BLOCK_BASIC_SYSTEM = 0,    // ����ϵ�y����
    BLOCK_HARDWARE_CALIB,      // Ӳ�wУ�ʅ���
    BLOCK_BG_PARAMS,           // Ѫ�ǅ���
    BLOCK_U_PARAMS,            // ���ᅢ��
    BLOCK_C_PARAMS,            // ��đ�̴�����
    BLOCK_TG_PARAMS,           // �������֬����
    BLOCK_RESERVED,            // ����^
    BLOCK_MAX
} BlockType_TypeDef;

/* ���ܺ����� */
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
const char* StripType_GetName(StripType_TypeDef type);
const char* Unit_GetSymbol(Unit_TypeDef unit);

/* �߼����ܺ����� */
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                         uint8_t *hour, uint8_t *minute, uint8_t *second);
uint8_t PARAM_SetDateTime(uint8_t year, uint8_t month, uint8_t date, 
                         uint8_t hour, uint8_t minute, uint8_t second);
uint8_t PARAM_IncreaseTestCount(void);
uint16_t PARAM_GetTestCount(void);
uint8_t PARAM_ReadBlock(BlockType_TypeDef block, void *data, uint16_t size);
uint8_t PARAM_UpdateBlock(BlockType_TypeDef block, void *data, uint16_t size);
uint8_t PARAM_GetStripParameters(StripType_TypeDef type, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
uint8_t PARAM_GetTimingParameters(StripType_TypeDef type, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t phase);

#endif /* __PARAM_TABLE_H */