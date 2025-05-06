/********************************** (C) COPYRIGHT  *******************************
* File Name          : param_code_table.h
* Author             : WCH / HMD
* Version            : V1.0.0
* Date               : 2025/05/06
* Description        : �๦�������yԇ�x�������a���x�c����
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#ifndef __PARAM_CODE_TABLE_H
#define __PARAM_CODE_TABLE_H

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* �������a��Y�����x */

/* 1. ϵ�y�������� */
typedef struct {
    uint8_t LBT;                /* ��늳��ֵ */
    uint8_t OBT;                /* 늳غıM�ֵ */
    uint8_t FACTORY;            /* �x������ģʽ */
    uint8_t MODEL_NO;           /* �aƷ��̖ */
    uint8_t FW_NO;              /* �g�w�汾̖ */
    uint16_t NOT;               /* �yԇ�Δ� */
    uint16_t Code_Table_V;      /* ���a��汾̖ */
} SystemBasicParams_TypeDef;

/* 2. �r�g�O������ */
typedef struct {
    uint8_t YEAR;               /* ����O�� (0~99, 2000~2099��) */
    uint8_t MONTH;              /* �·��O�� (1~12) */
    uint8_t DATE;               /* �����O�� (1~31) */
    uint8_t HOUR;               /* С�r�O�� (0~23) */
    uint8_t MINUTE;             /* ����O�� (0~59) */
    uint8_t SECOND;             /* �딵�O�� (0~59) */
} TimeSettingParams_TypeDef;

/* 3. �yԇ�@ʾ���� */
typedef struct {
    uint8_t TLL;                /* �����ضȹ������� */
    uint8_t TLH;                /* �����ضȹ������� */
    uint8_t MGDL;               /* ��Ȇ�λ�O�� (0:mmol/L; 1:mg/dL; 2:gm/dl) */
    uint8_t EVENT;              /* �x��¼���� (0:QC, 1:AC, 2:PC) */
    uint8_t Strip_Type;         /* �yԇ�Ŀ (0:GLV, 1:U, 2:C, 3:TG, 4:GAV) */
} TestDisplayParams_TypeDef;

/* 4. Ӳ�wУ������ */
typedef struct {
    uint16_t EV_T3_TRG;         /* EV_T3�|�l늉� */
    uint8_t EV_WORKING;         /* ��ȹ����·늉� */
    uint8_t EV_T3;              /* ѪҺ������·늉� */
    int8_t DACO;                /* DACƫ���a�� */
    uint8_t DACDO;              /* DACУ�����텢�� */
    uint8_t CC211NoDone;        /* CC211δ������ */
    uint8_t CAL_TOL;            /* OPS/OPIУ���ݲ� */
    float OPS;                  /* OPAУ��б�� */
    float OPI;                  /* OPAУ���ؾ� */
    uint8_t QCT;                /* QCTУ���yԇ��λԪ�M */
    int8_t TOFFSET;             /* �ض�У��ƫ�� */
    int8_t BOFFSET;             /* 늳�У��ƫ�� */
} HardwareCalibParams_TypeDef;

/* ԇƬͨ�Å����Y�� */
typedef struct {
    uint8_t CSU_TOL;            /* ԇƬ�z���ݲ� */
    uint16_t NDL;               /* ��ԇƬ���ˮƽ */
    uint16_t UDL;               /* ��ʹ��ԇƬ���ˮƽ */
    uint16_t BLOOD_IN;          /* ѪҺ���ˮƽ */
    uint8_t Strip_Lot[16];      /* ԇƬ��̖(16�ֹ�) */
    uint8_t L;                  /* ��ֵ���� */
    uint8_t H;                  /* ��ֵ���� */
    uint16_t T3_E37;            /* T3 ADV��37�ֵ */
    
    /* �yԇ�r�򅢔� */
    uint16_t TPL_1;             /* �r�g�}�n�� (��һ�M) */
    uint16_t TRD_1;             /* ԭʼ�����r�g (��һ�M) */
    uint16_t EV_Width_1;        /* ���ԕr�g (��һ�M) */
    uint16_t TPL_2;             /* �r�g�}�n�� (�ڶ��M) */
    uint16_t TRD_2;             /* ԭʼ�����r�g (�ڶ��M) */
    uint16_t EV_Width_2;        /* ���ԕr�g (�ڶ��M) */
    
    /* Ʒ��Һ(QC)���� */
    uint8_t Sq;                 /* QC�`�􅢔�Sq */
    uint8_t Iq;                 /* QC�`�􅢔�Iq */
    uint8_t CVq;                /* QCˮƽ��CV */
    uint8_t Aq;                 /* QC�`�􅢔�A */
    uint8_t Bq;                 /* QC�`�􅢔�B */
    uint8_t Aq5;                /* QC�`�􅢔�A(�ȼ�5) - �HѪ��ʹ�� */
    uint8_t Bq5;                /* QC�`�􅢔�B(�ȼ�5) - �HѪ��ʹ�� */
    
    /* Ӌ��ʽ��헅��� */
    uint8_t S2;                 /* ԇƬ�`�􅢔� */
    uint8_t I2;                 /* ԇƬ�`�􅢔� */
    float Sr;                   /* OPAӲ�wޒ·�`��(б��) */
    float Ir;                   /* OPAӲ�wޒ·�`��(�ؾ�) */
    
    /* �ض��`�􅢔� */
    uint8_t TF[7];              /* AC/PC�ض��`��б��(10-40��C) */
    uint8_t CTF[7];             /* QC�ض��`��б��(10-40��C) */
    uint8_t TO[7];              /* AC/PC�ض��`���ؾ�(10-40��C) */
    uint8_t CTO[7];             /* QC�ض��`���ؾ�(10-40��C) */
} StripCommonParams_TypeDef;

/* Ѫ�����Ѕ��� */
typedef struct {
    uint8_t QC[5][6];           /* QC�˜ʹ������� */
    uint8_t S3[30];             /* ԇƬ�`�􅢔�S3(1-30) */
    uint8_t I3[30];             /* ԇƬ�`�􅢔�I3(1-30) */
} BloodGlucoseParams_TypeDef;

/* ����/��đ�̴�/�������֬���Ѕ��� */
typedef struct {
    uint8_t QC[5][6];           /* QC�˜ʹ������� */
    uint8_t S3[15];             /* ԇƬ�`�􅢔�S3(1-15) */
    uint8_t I3[15];             /* ԇƬ�`�􅢔�I3(1-15) */
} OtherTestParams_TypeDef;

/* �������a�������Y�� */
typedef struct {
    SystemBasicParams_TypeDef System;
    TimeSettingParams_TypeDef Time;
    TestDisplayParams_TypeDef Display;
    HardwareCalibParams_TypeDef Hardware;
    
    /* Ѫ��(GLV/GAV)���� */
    StripCommonParams_TypeDef BG;
    BloodGlucoseParams_TypeDef BG_Special;
    
    /* ����(U)���� */
    StripCommonParams_TypeDef U;
    OtherTestParams_TypeDef U_Special;
    
    /* ��đ�̴�(C)���� */
    StripCommonParams_TypeDef C;
    OtherTestParams_TypeDef C_Special;
    
    /* �������֬(TG)���� */
    StripCommonParams_TypeDef TG;
    OtherTestParams_TypeDef TG_Special;
    
    /* У�a */
    uint8_t SUM_L;              /* ��ַ0~674У��(��) */
    uint8_t SUM_H;              /* ��ַ0~674У��(��) */
    uint8_t CRC16;              /* ����������CRCУ� */
} ParamCodeTable_TypeDef;

/* �������a��������� */
void ParamCodeTable_Init(void);
ErrorStatus ParamCodeTable_Save(void);
ErrorStatus ParamCodeTable_Load(void);
ErrorStatus ParamCodeTable_Reset(void);
uint16_t ParamCodeTable_CalculateChecksum(void);
ErrorStatus ParamCodeTable_VerifyChecksum(void);

/* �@ȡ�΂��������o������ */
uint8_t GetSystemParam(uint16_t paramAddress);
ErrorStatus SetSystemParam(uint16_t paramAddress, uint8_t value);

/* �z��ԇƬ��� */
uint8_t GetStripType(void);
const char* GetStripTypeName(uint8_t stripType);

extern ParamCodeTable_TypeDef ParamCodeTable;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_CODE_TABLE_H */