/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : �๦�������yԇ�x�������a���x�^�ļ�
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ��������С��λԪ�M�� */
#define PARAM_TABLE_SIZE            678

/* �����^��ƫ�� */
#define PARAM_SYSTEM_OFFSET         0    // ϵ�y�������� (0-8)
#define PARAM_TIME_OFFSET           9    // �r�g�O������ (9-14)
#define PARAM_ENV_OFFSET            15   // �yԇ�h������ (15-19)
#define PARAM_CAL_OFFSET            20   // Ӳ�wУ�ʅ��� (20-38)
#define PARAM_BG_OFFSET             39   // Ѫ�ǌ��Å��� (39-212)
#define PARAM_UA_OFFSET             213  // ���ጣ�Å��� (213-354)
#define PARAM_CHOL_OFFSET           355  // ��đ�̴����Å��� (355-496)
#define PARAM_TG_OFFSET             497  // �������֬���Å��� (497-638)
#define PARAM_RESERVED_OFFSET       639  // ����^�� (639-674)
#define PARAM_CHECKSUM_OFFSET       675  // У�^ (675-677)

/* �yԇ�Ŀ��� */
#define STRIP_TYPE_GLV              0    // Ѫ��(GLV)
#define STRIP_TYPE_UA               1    // ����(U)
#define STRIP_TYPE_CHOL             2    // ��đ�̴�(C)
#define STRIP_TYPE_TG               3    // �������֬(TG)
#define STRIP_TYPE_GAV              4    // Ѫ��(GAV)

/* �y����λ�O�� */
#define UNIT_MMOL_L                 0    // mmol/L
#define UNIT_MG_DL                  1    // mg/dL
#define UNIT_GM_DL                  2    // gm/dl

/* �¼���� */
#define EVENT_QC                    0    // Ʒ��Һ�yԇ
#define EVENT_AC                    1    // ��ǰ�yԇ
#define EVENT_PC                    2    // ����yԇ

/* �x������ģʽ */
#define MODE_USER                   0    // ʹ����ģʽ
#define MODE_FACTORY                210  // ���Sģʽ

/* ������Y���w���x */
typedef struct {
    /* 1. ϵ�y�������� */
    uint8_t  LBT;                       // ��늳��ֵ
    uint8_t  OBT;                       // 늳غıM�ֵ
    uint8_t  FACTORY;                   // �x������ģʽ
    uint8_t  MODEL_NO;                  // �aƷ��̖
    uint8_t  FW_NO;                     // �g�w�汾̖
    uint16_t NOT;                       // �yԇ�Δ�
    uint16_t Code_Table_V;              // ���a��汾��̖

    /* 2. �r�g�O������ */
    uint8_t  YEAR;                      // ����O�� (0-99, 2000-2099��)
    uint8_t  MONTH;                     // �·��O�� (1-12)
    uint8_t  DATE;                      // �����O�� (1-31)
    uint8_t  HOUR;                      // С�r�O�� (0-23)
    uint8_t  MINUTE;                    // ����O�� (0-59)
    uint8_t  SECOND;                    // �딵�O�� (0-59)

    /* 3. �yԇ�h������ */
    uint8_t  TLL;                       // �����ضȹ�������
    uint8_t  TLH;                       // �����ضȹ�������
    uint8_t  MGDL;                      // �y����λ�O��
    uint8_t  EVENT;                     // �A�O�¼����
    uint8_t  Strip_Type;                // �yԇ�Ŀ

    /* 4. Ӳ�wУ�ʅ��� */
    uint16_t EV_T3_TRG;                 // EV_T3�|�l늉�
    uint8_t  EV_WORKING;                // �y������늘O늉�
    uint8_t  EV_T3;                     // ѪҺ�z�y늘O늉�
    uint8_t  DACO;                      // DACƫ���a��
    uint8_t  DACDO;                     // DACУ�����텢��
    uint8_t  CC211NoDone;               // CC211δ������
    uint8_t  CAL_TOL;                   // OPS/OPIУ���ݲ�
    float    OPS;                       // OPAУ��б��
    float    OPI;                       // OPAУ�ʽؾ�
    uint8_t  QCT;                       // QCTУ�ʜyԇ��λԪ�M
    uint8_t  TOFFSET;                   // �ض�У��ƫ��
    uint8_t  BOFFSET;                   // 늳�У��ƫ��

    /* 5. Ѫ��(GLV/GAV)���Å��� */
    /* 5.1 ԇƬ���� */
    uint8_t  BG_CSU_TOL;                // ԇƬ�z���ݲ�
    uint16_t BG_NDL;                    // ��ԇƬ�z�yˮƽ
    uint16_t BG_UDL;                    // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t BG_BLOOD_IN;               // ѪҺ�z�yˮƽ
    uint8_t  BG_Strip_Lot[16];          // ԇƬ��̖
    
    /* 5.2 �y���������� */
    uint8_t  BG_L;                      // Ѫ��ֵ����
    uint16_t  BG_H;                      // Ѫ��ֵ����
    uint16_t BG_T3_E37;                 // T3 ADV�e�`37�ֵ

    /* 5.3 �yԇ�r�򅢔� */
    uint16_t BG_TPL_1;                  // �r�g�}�n��(��һ�M)
    uint16_t BG_TRD_1;                  // ԭʼ�����r�g(��һ�M)
    uint16_t BG_EV_Width_1;             // ȼ���r�g(��һ�M)
    uint16_t BG_TPL_2;                  // �r�g�}�n��(�ڶ��M)
    uint16_t BG_TRD_2;                  // ԭʼ�����r�g(�ڶ��M)
    uint16_t BG_EV_Width_2;             // ȼ���r�g(�ڶ��M)

    /* 5.4 Ʒ��Һ(QC)���� */
    uint8_t  BG_Sq;                     // QC�a������Sq
    uint8_t  BG_Iq;                     // QC�a������Iq
    uint8_t  BG_CVq;                    // QCˮƽ��CV
    uint8_t  BG_Aq;                     // QC�a������A
    uint8_t  BG_Bq;                     // QC�a������B
    uint8_t  BG_Aq5;                    // QC�a������A(�ȼ�5)
    uint8_t  BG_Bq5;                    // QC�a������B(�ȼ�5)
    uint8_t  BG_QC[30];                 // QC�˜ʹ�������

    /* 5.5 Ӌ��ʽ�a������ */
    uint8_t  BG_S2;                     // ԇƬ�a������
    uint8_t  BG_I2;                     // ԇƬ�a������
    float    BG_Sr;                     // OPAӲ�wޒ·�a��(б��)
    float    BG_Ir;                     // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  BG_S3[30];                 // ԇƬ�a������S3(1-30)
    uint8_t  BG_I3[30];                 // ԇƬ�a������I3(1-30)

    /* 5.6 �ض��a������ */
    uint8_t  BG_TF[7];                  // AC/PC�ض��a��б��(10-40��C)
    uint8_t  BG_CTF[7];                 // QC�ض��a��б��(10-40��C)
    uint8_t  BG_TO[7];                  // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  BG_CTO[7];                 // QC�ض��a���ؾ�(10-40��C)

    /* 6. ����(U)���Å��� */
    /* 6.1 ԇƬ���� */
    uint8_t  U_CSU_TOL;                 // ԇƬ�z���ݲ�
    uint16_t U_NDL;                     // ��ԇƬ�z�yˮƽ
    uint16_t U_UDL;                     // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t U_BLOOD_IN;                // ѪҺ�z�yˮƽ
    uint8_t  U_Strip_Lot[16];           // ԇƬ��̖
    
    /* 6.2 �y���������� */
    uint8_t  U_L;                       // ����ֵ����
    uint16_t  U_H;                       // ����ֵ����
    uint16_t U_T3_E37;                  // T3 ADV�e�`37�ֵ

    /* 6.3 �yԇ�r�򅢔� */
    uint16_t U_TPL_1;                   // �r�g�}�n��(��һ�M)
    uint16_t U_TRD_1;                   // ԭʼ�����r�g(��һ�M)
    uint16_t U_EV_Width_1;              // ȼ���r�g(��һ�M)
    uint16_t U_TPL_2;                   // �r�g�}�n��(�ڶ��M)
    uint16_t U_TRD_2;                   // ԭʼ�����r�g(�ڶ��M)
    uint16_t U_EV_Width_2;              // ȼ���r�g(�ڶ��M)

    /* 6.4 Ʒ��Һ(QC)���� */
    uint8_t  U_Sq;                      // QC�a������Sq
    uint8_t  U_Iq;                      // QC�a������Iq
    uint8_t  U_CVq;                     // QCˮƽ��CV
    uint8_t  U_Aq;                      // QC�a������A
    uint8_t  U_Bq;                      // QC�a������B
    uint8_t  U_QC[30];                  // QC�˜ʹ�������

    /* 6.5 Ӌ��ʽ�a������ */
    uint8_t  U_S2;                      // ԇƬ�a������
    uint8_t  U_I2;                      // ԇƬ�a������
    float    U_Sr;                      // OPAӲ�wޒ·�a��(б��)
    float    U_Ir;                      // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  U_S3[15];                  // ԇƬ�a������S3(1-15)
    uint8_t  U_I3[15];                  // ԇƬ�a������I3(1-15)

    /* 6.6 �ض��a������ */
    uint8_t  U_TF[7];                   // AC/PC�ض��a��б��(10-40��C)
    uint8_t  U_CTF[7];                  // QC�ض��a��б��(10-40��C)
    uint8_t  U_TO[7];                   // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  U_CTO[7];                  // QC�ض��a���ؾ�(10-40��C)

    /* 7. ��đ�̴�(C)���Å��� */
    /* 7.1 ԇƬ���� */
    uint8_t  C_CSU_TOL;                 // ԇƬ�z���ݲ�
    uint16_t C_NDL;                     // ��ԇƬ�z�yˮƽ
    uint16_t C_UDL;                     // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t C_BLOOD_IN;                // ѪҺ�z�yˮƽ
    uint8_t  C_Strip_Lot[16];           // ԇƬ��̖
    
    /* 7.2 �y���������� */
    uint8_t  C_L;                       // đ�̴�ֵ����
    uint16_t  C_H;                       // đ�̴�ֵ����
    uint16_t C_T3_E37;                  // T3 ADV�e�`37�ֵ

    /* 7.3 �yԇ�r�򅢔� */
    uint16_t C_TPL_1;                   // �r�g�}�n��(��һ�M)
    uint16_t C_TRD_1;                   // ԭʼ�����r�g(��һ�M)
    uint16_t C_EV_Width_1;              // ȼ���r�g(��һ�M)
    uint16_t C_TPL_2;                   // �r�g�}�n��(�ڶ��M)
    uint16_t C_TRD_2;                   // ԭʼ�����r�g(�ڶ��M)
    uint16_t C_EV_Width_2;              // ȼ���r�g(�ڶ��M)

    /* 7.4 Ʒ��Һ(QC)���� */
    uint8_t  C_Sq;                      // QC�a������Sq
    uint8_t  C_Iq;                      // QC�a������Iq
    uint8_t  C_CVq;                     // QCˮƽ��CV
    uint8_t  C_Aq;                      // QC�a������A
    uint8_t  C_Bq;                      // QC�a������B
    uint8_t  C_QC[30];                  // QC�˜ʹ�������

    /* 7.5 Ӌ��ʽ�a������ */
    uint8_t  C_S2;                      // ԇƬ�a������
    uint8_t  C_I2;                      // ԇƬ�a������
    float    C_Sr;                      // OPAӲ�wޒ·�a��(б��)
    float    C_Ir;                      // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  C_S3[15];                  // ԇƬ�a������S3(1-15)
    uint8_t  C_I3[15];                  // ԇƬ�a������I3(1-15)

    /* 7.6 �ض��a������ */
    uint8_t  C_TF[7];                   // AC/PC�ض��a��б��(10-40��C)
    uint8_t  C_CTF[7];                  // QC�ض��a��б��(10-40��C)
    uint8_t  C_TO[7];                   // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  C_CTO[7];                  // QC�ض��a���ؾ�(10-40��C)

    /* 8. �������֬(TG)���Å��� */
    /* 8.1 ԇƬ���� */
    uint8_t  TG_CSU_TOL;                // ԇƬ�z���ݲ�
    uint16_t TG_NDL;                    // ��ԇƬ�z�yˮƽ
    uint16_t TG_UDL;                    // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t TG_BLOOD_IN;               // ѪҺ�z�yˮƽ
    uint8_t  TG_Strip_Lot[16];          // ԇƬ��̖
    
    /* 8.2 �y���������� */
    uint8_t  TG_L;                      // �������ֵ֬����
    uint16_t  TG_H;                      // �������ֵ֬����
    uint16_t TG_T3_E37;                 // T3 ADV�e�`37�ֵ

    /* 8.3 �yԇ�r�򅢔� */
    uint16_t TG_TPL_1;                  // �r�g�}�n��(��һ�M)
    uint16_t TG_TRD_1;                  // ԭʼ�����r�g(��һ�M)
    uint16_t TG_EV_Width_1;             // ȼ���r�g(��һ�M)
    uint16_t TG_TPL_2;                  // �r�g�}�n��(�ڶ��M)
    uint16_t TG_TRD_2;                  // ԭʼ�����r�g(�ڶ��M)
    uint16_t TG_EV_Width_2;             // ȼ���r�g(�ڶ��M)

    /* 8.4 Ʒ��Һ(QC)���� */
    uint8_t  TG_Sq;                     // QC�a������Sq
    uint8_t  TG_Iq;                     // QC�a������Iq
    uint8_t  TG_CVq;                    // QCˮƽ��CV
    uint8_t  TG_Aq;                     // QC�a������A
    uint8_t  TG_Bq;                     // QC�a������B
    uint8_t  TG_QC[30];                 // QC�˜ʹ�������

    /* 8.5 Ӌ��ʽ�a������ */
    uint8_t  TG_S2;                     // ԇƬ�a������
    uint8_t  TG_I2;                     // ԇƬ�a������
    float    TG_Sr;                     // OPAӲ�wޒ·�a��(б��)
    float    TG_Ir;                     // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  TG_S3[15];                 // ԇƬ�a������S3(1-15)
    uint8_t  TG_I3[15];                 // ԇƬ�a������I3(1-15)

    /* 8.6 �ض��a������ */
    uint8_t  TG_TF[7];                  // AC/PC�ض��a��б��(10-40��C)
    uint8_t  TG_CTF[7];                 // QC�ض��a��б��(10-40��C)
    uint8_t  TG_TO[7];                  // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  TG_CTO[7];                 // QC�ض��a���ؾ�(10-40��C)

    /* 9. �����cУ�^ */
    uint8_t  Reserved[36];              // ����^��δ��Uչ
    uint8_t  SUM_L;                     // ��ַ0~674У��(��)
    uint8_t  SUM_H;                     // ��ַ0~674У��(��)
    uint8_t  CRC16;                     // ����������CRCУ�
} ParamTable_TypeDef;

/* �������� */
void ParamTable_Init(ParamTable_TypeDef *param_table);
uint16_t ParamTable_CalculateChecksum(ParamTable_TypeDef *param_table);
uint8_t ParamTable_CalculateCRC16(ParamTable_TypeDef *param_table);
uint8_t ParamTable_Verify(ParamTable_TypeDef *param_table);
void ParamTable_SaveToFlash(ParamTable_TypeDef *param_table);
uint8_t ParamTable_LoadFromFlash(ParamTable_TypeDef *param_table);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */ 