/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : �๦�������yԇ�x�������a���^�ļ�
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* ��������С (bytes) */
#define PARAM_TABLE_SIZE 678
/* ������У򞿂��С (0~674) */
#define PARAM_TABLE_CHECK_SIZE 675

/* ԇƬ��Ͷ��x */
typedef enum {
    STRIP_TYPE_GLV = 0, // Ѫ��(GLV)
    STRIP_TYPE_U = 1,   // ����(U)
    STRIP_TYPE_C = 2,   // ��đ�̴�(C)
    STRIP_TYPE_TG = 3,  // �������֬(TG)
    STRIP_TYPE_GAV = 4  // Ѫ��(GAV)
} StripType_TypeDef;

/* �¼���Ͷ��x */
typedef enum {
    EVENT_QC = 0,  // Ʒ��Һ
    EVENT_AC = 1,  // �ո�
    EVENT_PC = 2   // ����
} Event_TypeDef;

/* ��Ȇ�λ���x */
typedef enum {
    UNIT_MMOL_L = 0,  // mmol/L
    UNIT_MG_DL = 1,   // mg/dL
    UNIT_GM_DL = 2    // gm/dl
} Unit_TypeDef;

/* ���Sģʽ��B */
#define FACTORY_USER_MODE 0     // ʹ����ģʽ
#define FACTORY_FACTORY_MODE 210 // ���Sģʽ

/* ������Y�����x */
typedef struct {
    /* 1. ϵ�y��������: 0-8 (9 bytes) */
    uint8_t LBT;              // ��늳��ֵ
    uint8_t OBT;              // 늳غĽ��ֵ
    uint8_t FACTORY;          // �x������ģʽ
    uint8_t MODEL_NO;         // �aƷ��̖
    uint8_t FW_NO;            // �g�w�汾̖
    uint16_t NOT;             // �yԇ�Δ�
    uint16_t Code_Table_V;    // ���a��汾̖

    /* 2. �r�g�O������: 9-14 (6 bytes) */
    uint8_t YEAR;             // ����O�� (0~99, ��ʾ2000~2099��)
    uint8_t MONTH;            // �·��O�� (1~12)
    uint8_t DATE;             // �����O�� (1~31)
    uint8_t HOUR;             // С�r�O�� (0~23)
    uint8_t MINUTE;           // ����O�� (0~59)
    uint8_t SECOND;           // �딵�O�� (0~59)

    /* 3. �yԇ�h������: 15-19 (5 bytes) */
    uint8_t TLL;              // �����ضȹ�������
    uint8_t TLH;              // �����ضȹ�������
    uint8_t MGDL;             // ��Ȇ�λ�O��
    uint8_t EVENT;            // �O���¼����
    uint8_t Strip_Type;       // �yԇ�Ŀ

    /* 4. Ӳ�wУ������: 20-38 (19 bytes) */
    uint16_t EV_T3_TRG;       // EV_T3�|�l늉�
    uint8_t EV_WORKING;       // �y������늉�늉�
    uint8_t EV_T3;            // ѪҺ����늘O늉�
    uint8_t DACO;             // DACƫ���a��
    uint8_t DACDO;            // DACУ�����텢��
    uint8_t CC211NoDone;      // CC211δ������
    uint8_t CAL_TOL;          // OPS/OPIУ���ݲ�
    float OPS;                // OPAУ��б��
    float OPI;                // OPAУ���ؾ�
    uint8_t QCT;              // QCTУ���yԇ��λԪ�M
    uint8_t TOFFSET;          // �ض�У��ƫ��
    uint8_t BOFFSET;          // 늳�У��ƫ��

    /* 5. Ѫ��(GLV/GAV)���Å���: 39-212 (174 bytes) */
    /* 5.1 ԇƬ���� */
    uint8_t BG_CSU_TOL;       // ԇƬ�z���ݲ�
    uint16_t BG_NDL;          // ��ԇƬˮƽ
    uint16_t BG_UDL;          // ��ʹ��ԇƬˮƽ
    uint16_t BG_BLOOD_IN;     // ѪҺˮƽ
    uint8_t BG_Strip_Lot[16]; // ԇƬ��̖(16�ֹ�)

    /* 5.2 �y���������� */
    uint8_t BG_L;             // Ѫ��ֵ����
    uint8_t BG_H;             // Ѫ��ֵ����
    uint16_t BG_T3_E37;       // T3 ADV�E37�ֵ

    /* 5.3 �yԇ�r�򅢔� */
    uint16_t BG_TPL_1;        // �r�g�}�n���� (��һ�M)
    uint16_t BG_TRD_1;        // ԭʼ�����r�g (��һ�M)
    uint16_t BG_EV_Width_1;   // �}�n�r�g (��һ�M)
    uint16_t BG_TPL_2;        // �r�g�}�n���� (�ڶ��M)
    uint16_t BG_TRD_2;        // ԭʼ�����r�g (�ڶ��M)
    uint16_t BG_EV_Width_2;   // �}�n�r�g (�ڶ��M)

    /* 5.4 Ʒ��Һ(QC)���� */
    uint8_t BG_Sq;            // QC���Ѕ���Sq
    uint8_t BG_Iq;            // QC���Ѕ���Iq
    uint8_t BG_CVq;           // QCˮƽ��CV
    uint8_t BG_Aq;            // QC���Ѕ���A
    uint8_t BG_Bq;            // QC���Ѕ���B
    uint8_t BG_Aq5;           // QC���Ѕ���A(�ȼ�5)
    uint8_t BG_Bq5;           // QC���Ѕ���B(�ȼ�5)
    uint8_t BG_QC[30];        // QC�˜ʹ������� (QC1-QC5)

    /* 5.5 Ӌ��ʽ���ЂS�� */
    uint8_t BG_S2;            // ԇƬ���Ѕ���
    uint8_t BG_I2;            // ԇƬ���Ѕ���
    float BG_Sr;              // OPAӲ�wޒ·����(б��)
    float BG_Ir;              // OPAӲ�wޒ·����(�ؾ�)
    uint8_t BG_S3[30];        // ԇƬ���Ѕ���S3(1-30)
    uint8_t BG_I3[30];        // ԇƬ���Ѕ���I3(1-30)

    /* 5.6 �ض����Ѕ��� */
    uint8_t BG_TF[7];         // AC/PC�ض�����б��(10-40��C)
    uint8_t BG_CTF[7];        // QC�ض�����б��(10-40��C)
    uint8_t BG_TO[7];         // AC/PC�ض����нؾ�(10-40��C)
    uint8_t BG_CTO[7];        // QC�ض����нؾ�(10-40��C)

    /* 6. ����(U)���Å���: 213-354 (142 bytes) */
    /* 6.1 ԇƬ���� */
    uint8_t U_CSU_TOL;        // ԇƬ�z���ݲ�
    uint16_t U_NDL;           // ��ԇƬˮƽ
    uint16_t U_UDL;           // ��ʹ��ԇƬˮƽ
    uint16_t U_BLOOD_IN;      // ѪҺˮƽ
    uint8_t U_Strip_Lot[16];  // ԇƬ��̖(16�ֹ�)

    /* 6.2 �y���������� */
    uint8_t U_L;              // ����ֵ����
    uint8_t U_H;              // ����ֵ����
    uint16_t U_T3_E37;        // T3 ADV�E37�ֵ

    /* 6.3 �yԇ�r�򅢔� */
    uint16_t U_TPL_1;         // �r�g�}�n���� (��һ�M)
    uint16_t U_TRD_1;         // ԭʼ�����r�g (��һ�M)
    uint16_t U_EV_Width_1;    // �}�n�r�g (��һ�M)
    uint16_t U_TPL_2;         // �r�g�}�n���� (�ڶ��M)
    uint16_t U_TRD_2;         // ԭʼ�����r�g (�ڶ��M)
    uint16_t U_EV_Width_2;    // �}�n�r�g (�ڶ��M)

    /* 6.4 Ʒ��Һ(QC)���� */
    uint8_t U_Sq;             // QC���Ѕ���Sq
    uint8_t U_Iq;             // QC���Ѕ���Iq
    uint8_t U_CVq;            // QCˮƽ��CV
    uint8_t U_Aq;             // QC���Ѕ���A
    uint8_t U_Bq;             // QC���Ѕ���B
    uint8_t U_QC[30];         // QC�˜ʹ������� (QC1-QC5)

    /* 6.5 Ӌ��ʽ���ЂS�� */
    uint8_t U_S2;             // ԇƬ���Ѕ���
    uint8_t U_I2;             // ԇƬ���Ѕ���
    float U_Sr;               // OPAӲ�wޒ·����(б��)
    float U_Ir;               // OPAӲ�wޒ·����(�ؾ�)
    uint8_t U_S3[15];         // ԇƬ���Ѕ���S3(1-15)
    uint8_t U_I3[15];         // ԇƬ���Ѕ���I3(1-15)

    /* 6.6 �ض����Ѕ��� */
    uint8_t U_TF[7];          // AC/PC�ض�����б��(10-40��C)
    uint8_t U_CTF[7];         // QC�ض�����б��(10-40��C)
    uint8_t U_TO[7];          // AC/PC�ض����нؾ�(10-40��C)
    uint8_t U_CTO[7];         // QC�ض����нؾ�(10-40��C)

    /* 7. ��đ�̴�(C)���Å���: 355-496 (142 bytes) */
    /* 7.1 ԇƬ���� */
    uint8_t C_CSU_TOL;        // ԇƬ�z���ݲ�
    uint16_t C_NDL;           // ��ԇƬˮƽ
    uint16_t C_UDL;           // ��ʹ��ԇƬˮƽ
    uint16_t C_BLOOD_IN;      // ѪҺˮƽ
    uint8_t C_Strip_Lot[16];  // ԇƬ��̖(16�ֹ�)

    /* 7.2 �y���������� */
    uint8_t C_L;              // đ�̴�ֵ����
    uint8_t C_H;              // đ�̴�ֵ����
    uint16_t C_T3_E37;        // T3 ADV�E37�ֵ

    /* 7.3 �yԇ�r�򅢔� */
    uint16_t C_TPL_1;         // �r�g�}�n���� (��һ�M)
    uint16_t C_TRD_1;         // ԭʼ�����r�g (��һ�M)
    uint16_t C_EV_Width_1;    // �}�n�r�g (��һ�M)
    uint16_t C_TPL_2;         // �r�g�}�n���� (�ڶ��M)
    uint16_t C_TRD_2;         // ԭʼ�����r�g (�ڶ��M)
    uint16_t C_EV_Width_2;    // �}�n�r�g (�ڶ��M)

    /* 7.4 Ʒ��Һ(QC)���� */
    uint8_t C_Sq;             // QC���Ѕ���Sq
    uint8_t C_Iq;             // QC���Ѕ���Iq
    uint8_t C_CVq;            // QCˮƽ��CV
    uint8_t C_Aq;             // QC���Ѕ���A
    uint8_t C_Bq;             // QC���Ѕ���B
    uint8_t C_QC[30];         // QC�˜ʹ������� (QC1-QC5)

    /* 7.5 Ӌ��ʽ���ЂS�� */
    uint8_t C_S2;             // ԇƬ���Ѕ���
    uint8_t C_I2;             // ԇƬ���Ѕ���
    float C_Sr;               // OPAӲ�wޒ·����(б��)
    float C_Ir;               // OPAӲ�wޒ·����(�ؾ�)
    uint8_t C_S3[15];         // ԇƬ���Ѕ���S3(1-15)
    uint8_t C_I3[15];         // ԇƬ���Ѕ���I3(1-15)

    /* 7.6 �ض����Ѕ��� */
    uint8_t C_TF[7];          // AC/PC�ض�����б��(10-40��C)
    uint8_t C_CTF[7];         // QC�ض�����б��(10-40��C)
    uint8_t C_TO[7];          // AC/PC�ض����нؾ�(10-40��C)
    uint8_t C_CTO[7];         // QC�ض����нؾ�(10-40��C)

    /* 8. �������֬(TG)���Å���: 497-638 (142 bytes) */
    /* 8.1 ԇƬ���� */
    uint8_t TG_CSU_TOL;       // ԇƬ�z���ݲ�
    uint16_t TG_NDL;          // ��ԇƬˮƽ
    uint16_t TG_UDL;          // ��ʹ��ԇƬˮƽ
    uint16_t TG_BLOOD_IN;     // ѪҺˮƽ
    uint8_t TG_Strip_Lot[16]; // ԇƬ��̖(16�ֹ�)

    /* 8.2 �y���������� */
    uint8_t TG_L;             // �������ֵ֬����
    uint8_t TG_H;             // �������ֵ֬����
    uint16_t TG_T3_E37;       // T3 ADV�E37�ֵ

    /* 8.3 �yԇ�r�򅢔� */
    uint16_t TG_TPL_1;        // �r�g�}�n���� (��һ�M)
    uint16_t TG_TRD_1;        // ԭʼ�����r�g (��һ�M)
    uint16_t TG_EV_Width_1;   // �}�n�r�g (��һ�M)
    uint16_t TG_TPL_2;        // �r�g�}�n���� (�ڶ��M)
    uint16_t TG_TRD_2;        // ԭʼ�����r�g (�ڶ��M)
    uint16_t TG_EV_Width_2;   // �}�n�r�g (�ڶ��M)

    /* 8.4 Ʒ��Һ(QC)���� */
    uint8_t TG_Sq;            // QC���Ѕ���Sq
    uint8_t TG_Iq;            // QC���Ѕ���Iq
    uint8_t TG_CVq;           // QCˮƽ��CV
    uint8_t TG_Aq;            // QC���Ѕ���A
    uint8_t TG_Bq;            // QC���Ѕ���B
    uint8_t TG_QC[30];        // QC�˜ʹ������� (QC1-QC5)

    /* 8.5 Ӌ��ʽ���ЂS�� */
    uint8_t TG_S2;            // ԇƬ���Ѕ���
    uint8_t TG_I2;            // ԇƬ���Ѕ���
    float TG_Sr;              // OPAӲ�wޒ·����(б��)
    float TG_Ir;              // OPAӲ�wޒ·����(�ؾ�)
    uint8_t TG_S3[15];        // ԇƬ���Ѕ���S3(1-15)
    uint8_t TG_I3[15];        // ԇƬ���Ѕ���I3(1-15)

    /* 8.6 �ض����Ѕ��� */
    uint8_t TG_TF[7];         // AC/PC�ض�����б��(10-40��C)
    uint8_t TG_CTF[7];        // QC�ض�����б��(10-40��C)
    uint8_t TG_TO[7];         // AC/PC�ض����нؾ�(10-40��C)
    uint8_t TG_CTO[7];        // QC�ض����нؾ�(10-40��C)

    /* 9. ������У�^: 639-677 (39 bytes) */
    uint8_t Reserved[36];     // ����^��δ��Uչ
    uint8_t SUM_L;            // ��ַ0~674У��(��)
    uint8_t SUM_H;            // ��ַ0~674У��(��)
    uint8_t CRC16;            // ����������CRCУ�
} Param_Table_TypeDef;

/* ����ԭ���� */
void PARAM_TABLE_Init(void);
void PARAM_TABLE_SetDefaults(void);
void PARAM_TABLE_SaveToFlash(void);
void PARAM_TABLE_LoadFromFlash(void);
uint16_t PARAM_TABLE_CalculateChecksum(void);
uint8_t PARAM_TABLE_Verify(void);
void PARAM_TABLE_PrintInfo(void);

/* ������ȫ��׃�� */
extern Param_Table_TypeDef g_ParamTable;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */