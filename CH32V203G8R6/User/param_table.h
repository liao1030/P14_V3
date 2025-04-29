/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter code table for biochemical analyzer.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#include "ch32v20x.h"

/* Parameter table size definition */
#define PARAM_TABLE_SIZE          678    /* Total parameter table size in bytes */
#define PARAM_TABLE_BASE_ADDR     0x0800F000    /* Flash storage base address */

/* Parameter type definitions */
#define STRIP_TYPE_GLV    0       /* Ѫ��(GLV) */
#define STRIP_TYPE_U      1       /* ����(U) */
#define STRIP_TYPE_C      2       /* ��đ�̴�(C) */
#define STRIP_TYPE_TG     3       /* �������֬(TG) */
#define STRIP_TYPE_GAV    4       /* Ѫ��(GAV) */

/* Factory mode definition */
#define USER_MODE         0       /* ʹ����ģʽ */
#define FACTORY_MODE      210     /* ���Sģʽ */

/* Event type definition */
#define EVENT_QC          0       /* QC (Quality Control) */
#define EVENT_AC          1       /* AC (Before meal) */
#define EVENT_PC          2       /* PC (After meal) */

/* Measurement unit definition */
#define UNIT_MMOL_L       0       /* mmol/L */
#define UNIT_MG_DL        1       /* mg/dL */
#define UNIT_GM_DL        2       /* gm/dL */

/* Parameter IDs for Param_GetValue/Param_SetValue */
typedef enum {
    /* System basic parameters */
    PARAM_LBT = 0,            /* ��늳��ֵ */
    PARAM_OBT,                /* 늳غıM�ֵ */
    PARAM_FACTORY,            /* �x������ģʽ */
    PARAM_MODEL_NO,           /* �aƷ��̖ */
    PARAM_FW_NO,              /* �g�w�汾̖ */
    PARAM_NOT,                /* �yԇ�Δ� */
    PARAM_CODE_TABLE_V,       /* ���a��汾̖ */
    
    /* Time setting parameters */
    PARAM_YEAR,               /* ����O�� (0-99) */
    PARAM_MONTH,              /* �·��O�� (1-12) */
    PARAM_DATE,               /* �����O�� (1-31) */
    PARAM_HOUR,               /* С�r�O�� (0-23) */
    PARAM_MINUTE,             /* ����O�� (0-59) */
    PARAM_SECOND,             /* �딵�O�� (0-59) */
    
    /* Measurement display parameters */
    PARAM_TLL,                /* �����ضȹ������� */
    PARAM_TLH,                /* �����ضȹ������� */
    PARAM_MGDL,               /* �y����λ�O�� */
    PARAM_EVENT,              /* �S���¼���� */
    PARAM_STRIP_TYPE,         /* �yԇ�Ŀ */
    
    /* Hardware calibration parameters */
    PARAM_EV_T3_TRG,          /* EV_T3�|�l늉� */
    PARAM_EV_WORKING,         /* �y������늉�늉� */
    PARAM_EV_T3,              /* ѪҺע��늉�늉� */
    PARAM_DACO,               /* DACƫ���`�� */
    PARAM_DACDO,              /* DACУ�����텢�� */
    PARAM_CC211_NO_DONE,      /* CC211δ������ */
    PARAM_CAL_TOL,            /* OPS/OPIУ���ݲ� */
    PARAM_TOFFSET,            /* �ض�У��ƫ�� */
    PARAM_BOFFSET,            /* 늳�У��ƫ�� */
    
    /* Blood Glucose specific parameters */
    PARAM_BG_L,               /* Ѫ��ֵ���� */
    PARAM_BG_H,               /* Ѫ��ֵ���� */
    
    /* Uric Acid specific parameters */
    PARAM_U_L,                /* ����ֵ���� */
    PARAM_U_H,                /* ����ֵ���� */
    
    /* Cholesterol specific parameters */
    PARAM_C_L,                /* đ�̴�ֵ���� */
    PARAM_C_H,                /* đ�̴�ֵ���� */
    
    /* Triglycerides specific parameters */
    PARAM_TG_L,               /* �������ֵ֬���� */
    PARAM_TG_H,               /* �������ֵ֬���� */
    
    /* Total number of parameters */
    PARAM_COUNT
} Param_ID_TypeDef;

/* Parameter table structure */
typedef struct {
    /* 1. System basic parameters */
    uint8_t LBT;                  /* ��늳��ֵ */
    uint8_t OBT;                  /* 늳غıM�ֵ */
    uint8_t FACTORY;              /* �x������ģʽ */
    uint8_t MODEL_NO;             /* �aƷ��̖ */
    uint8_t FW_NO;                /* �g�w�汾̖ */
    uint16_t NOT;                 /* �yԇ�Δ� */
    uint16_t Code_Table_V;        /* ���a��汾̖ */

    /* 2. Time setting parameters */
    uint8_t YEAR;                 /* ����O�� (0-99) */
    uint8_t MONTH;                /* �·��O�� (1-12) */
    uint8_t DATE;                 /* �����O�� (1-31) */
    uint8_t HOUR;                 /* С�r�O�� (0-23) */
    uint8_t MINUTE;               /* ����O�� (0-59) */
    uint8_t SECOND;               /* �딵�O�� (0-59) */

    /* 3. Measurement display parameters */
    uint8_t TLL;                  /* �����ضȹ������� */
    uint8_t TLH;                  /* �����ضȹ������� */
    uint8_t MGDL;                 /* �y����λ�O�� */
    uint8_t EVENT;                /* �S���¼���� */
    uint8_t Strip_Type;           /* �yԇ�Ŀ */

    /* 4. Hardware calibration parameters */
    uint16_t EV_T3_TRG;           /* EV_T3�|�l늉� */
    uint8_t EV_WORKING;           /* �y������늉�늉� */
    uint8_t EV_T3;                /* ѪҺע��늉�늉� */
    uint8_t DACO;                 /* DACƫ���`�� */
    uint8_t DACDO;                /* DACУ�����텢�� */
    uint8_t CC211NoDone;          /* CC211δ������ */
    uint8_t CAL_TOL;              /* OPS/OPIУ���ݲ� */
    float OPS;                    /* OPAУ��б�� */
    float OPI;                    /* OPAУ���ؾ� */
    uint8_t QCT;                  /* QCTУ���yԇ��λԪ�M */
    int8_t TOFFSET;               /* �ض�У��ƫ�� */
    int8_t BOFFSET;               /* 늳�У��ƫ�� */

    /* 5. GLV/GAV specific parameters */
    uint8_t BG_CSU_TOL;           /* ԇƬ�z���ݲ� */
    uint16_t BG_NDL;              /* ��ԇƬ늉�ˮƽ */
    uint16_t BG_UDL;              /* ��ʹ��ԇƬ늉�ˮƽ */
    uint16_t BG_BLOOD_IN;         /* ѪҺ늉�ˮƽ */
    uint8_t BG_Strip_Lot[16];     /* ԇƬ��̖ (16�ֹ�) */
    uint8_t BG_L;                 /* Ѫ��ֵ���� */
    uint16_t BG_H;                /* Ѫ��ֵ���� */
    uint16_t BG_T3_E37;           /* T3 ADV�e�`37�ֵ */
    uint16_t BG_TPL_1;            /* �r�g�}�n���� (��һ�M) */
    uint16_t BG_TRD_1;            /* ԭʼ�����񼯕r�g (��һ�M) */
    uint16_t BG_EV_Width_1;       /* ����r�g (��һ�M) */
    uint16_t BG_TPL_2;            /* �r�g�}�n���� (�ڶ��M) */
    uint16_t BG_TRD_2;            /* ԭʼ�����񼯕r�g (�ڶ��M) */
    uint16_t BG_EV_Width_2;       /* ����r�g (�ڶ��M) */
    uint8_t BG_Sq;                /* QC�`�􅢔�Sq */
    uint8_t BG_Iq;                /* QC�`�􅢔�Iq */
    uint8_t BG_CVq;               /* QCˮƽ��CV */
    uint8_t BG_Aq;                /* QC�`�􅢔�A */
    uint8_t BG_Bq;                /* QC�`�􅢔�B */
    uint8_t BG_Aq5;               /* QC�`�􅢔�A(�ȼ�5) */
    uint8_t BG_Bq5;               /* QC�`�􅢔�B(�ȼ�5) */
    uint8_t BG_QC[30];            /* QCģʽ�������� */
    uint8_t BG_S2;                /* ԇƬ�`�􅢔� */
    uint8_t BG_I2;                /* ԇƬ�`�􅢔� */
    float BG_Sr;                  /* OPAӲ�wޒ·�`��(б��) */
    float BG_Ir;                  /* OPAӲ�wޒ·�`��(�ؾ�) */
    uint8_t BG_S3[30];            /* ԇƬ�`�􅢔�S3(1-30) */
    uint8_t BG_I3[30];            /* ԇƬ�`�􅢔�I3(1-30) */
    uint8_t BG_TF[7];             /* AC/PC�ض��`��б��(10-40��C) */
    uint8_t BG_CTF[7];            /* QC�ض��`��б��(10-40��C) */
    uint8_t BG_TO[7];             /* AC/PC�ض��`���ؾ�(10-40��C) */
    uint8_t BG_CTO[7];            /* QC�ض��`���ؾ�(10-40��C) */

    /* 6. U (����) specific parameters */
    uint8_t U_CSU_TOL;            /* ԇƬ�z���ݲ� */
    uint16_t U_NDL;               /* ��ԇƬ늉�ˮƽ */
    uint16_t U_UDL;               /* ��ʹ��ԇƬ늉�ˮƽ */
    uint16_t U_BLOOD_IN;          /* ѪҺ늉�ˮƽ */
    uint8_t U_Strip_Lot[16];      /* ԇƬ��̖ (16�ֹ�) */
    uint8_t U_L;                  /* ����ֵ���� */
    uint8_t U_H;                  /* ����ֵ���� */
    uint16_t U_T3_E37;            /* T3 ADV�e�`37�ֵ */
    uint16_t U_TPL_1;             /* �r�g�}�n���� (��һ�M) */
    uint16_t U_TRD_1;             /* ԭʼ�����񼯕r�g (��һ�M) */
    uint16_t U_EV_Width_1;        /* ����r�g (��һ�M) */
    uint16_t U_TPL_2;             /* �r�g�}�n���� (�ڶ��M) */
    uint16_t U_TRD_2;             /* ԭʼ�����񼯕r�g (�ڶ��M) */
    uint16_t U_EV_Width_2;        /* ����r�g (�ڶ��M) */
    uint8_t U_Sq;                 /* QC�`�􅢔�Sq */
    uint8_t U_Iq;                 /* QC�`�􅢔�Iq */
    uint8_t U_CVq;                /* QCˮƽ��CV */
    uint8_t U_Aq;                 /* QC�`�􅢔�A */
    uint8_t U_Bq;                 /* QC�`�􅢔�B */
    uint8_t U_QC[30];             /* QCģʽ�������� */
    uint8_t U_S2;                 /* ԇƬ�`�􅢔� */
    uint8_t U_I2;                 /* ԇƬ�`�􅢔� */
    float U_Sr;                   /* OPAӲ�wޒ·�`��(б��) */
    float U_Ir;                   /* OPAӲ�wޒ·�`��(�ؾ�) */
    uint8_t U_S3[15];             /* ԇƬ�`�􅢔�S3(1-15) */
    uint8_t U_I3[15];             /* ԇƬ�`�􅢔�I3(1-15) */
    uint8_t U_TF[7];              /* AC/PC�ض��`��б��(10-40��C) */
    uint8_t U_CTF[7];             /* QC�ض��`��б��(10-40��C) */
    uint8_t U_TO[7];              /* AC/PC�ض��`���ؾ�(10-40��C) */
    uint8_t U_CTO[7];             /* QC�ض��`���ؾ�(10-40��C) */

    /* 7. C (��đ�̴�) specific parameters */
    uint8_t C_CSU_TOL;            /* ԇƬ�z���ݲ� */
    uint16_t C_NDL;               /* ��ԇƬ늉�ˮƽ */
    uint16_t C_UDL;               /* ��ʹ��ԇƬ늉�ˮƽ */
    uint16_t C_BLOOD_IN;          /* ѪҺ늉�ˮƽ */
    uint8_t C_Strip_Lot[16];      /* ԇƬ��̖ (16�ֹ�) */
    uint8_t C_L;                  /* đ�̴�ֵ���� */
    uint16_t C_H;                 /* đ�̴�ֵ���� */
    uint16_t C_T3_E37;            /* T3 ADV�e�`37�ֵ */
    uint16_t C_TPL_1;             /* �r�g�}�n���� (��һ�M) */
    uint16_t C_TRD_1;             /* ԭʼ�����񼯕r�g (��һ�M) */
    uint16_t C_EV_Width_1;        /* ����r�g (��һ�M) */
    uint16_t C_TPL_2;             /* �r�g�}�n���� (�ڶ��M) */
    uint16_t C_TRD_2;             /* ԭʼ�����񼯕r�g (�ڶ��M) */
    uint16_t C_EV_Width_2;        /* ����r�g (�ڶ��M) */
    uint8_t C_Sq;                 /* QC�`�􅢔�Sq */
    uint8_t C_Iq;                 /* QC�`�􅢔�Iq */
    uint8_t C_CVq;                /* QCˮƽ��CV */
    uint8_t C_Aq;                 /* QC�`�􅢔�A */
    uint8_t C_Bq;                 /* QC�`�􅢔�B */
    uint8_t C_QC[30];             /* QCģʽ�������� */
    uint8_t C_S2;                 /* ԇƬ�`�􅢔� */
    uint8_t C_I2;                 /* ԇƬ�`�􅢔� */
    float C_Sr;                   /* OPAӲ�wޒ·�`��(б��) */
    float C_Ir;                   /* OPAӲ�wޒ·�`��(�ؾ�) */
    uint8_t C_S3[15];             /* ԇƬ�`�􅢔�S3(1-15) */
    uint8_t C_I3[15];             /* ԇƬ�`�􅢔�I3(1-15) */
    uint8_t C_TF[7];              /* AC/PC�ض��`��б��(10-40��C) */
    uint8_t C_CTF[7];             /* QC�ض��`��б��(10-40��C) */
    uint8_t C_TO[7];              /* AC/PC�ض��`���ؾ�(10-40��C) */
    uint8_t C_CTO[7];             /* QC�ض��`���ؾ�(10-40��C) */

    /* 8. TG (�������֬) specific parameters */
    uint8_t TG_CSU_TOL;           /* ԇƬ�z���ݲ� */
    uint16_t TG_NDL;              /* ��ԇƬ늉�ˮƽ */
    uint16_t TG_UDL;              /* ��ʹ��ԇƬ늉�ˮƽ */
    uint16_t TG_BLOOD_IN;         /* ѪҺ늉�ˮƽ */
    uint8_t TG_Strip_Lot[16];     /* ԇƬ��̖ (16�ֹ�) */
    uint8_t TG_L;                 /* �������ֵ֬���� */
    uint16_t TG_H;                /* �������ֵ֬���� */
    uint16_t TG_T3_E37;           /* T3 ADV�e�`37�ֵ */
    uint16_t TG_TPL_1;            /* �r�g�}�n���� (��һ�M) */
    uint16_t TG_TRD_1;            /* ԭʼ�����񼯕r�g (��һ�M) */
    uint16_t TG_EV_Width_1;       /* ����r�g (��һ�M) */
    uint16_t TG_TPL_2;            /* �r�g�}�n���� (�ڶ��M) */
    uint16_t TG_TRD_2;            /* ԭʼ�����񼯕r�g (�ڶ��M) */
    uint16_t TG_EV_Width_2;       /* ����r�g (�ڶ��M) */
    uint8_t TG_Sq;                /* QC�`�􅢔�Sq */
    uint8_t TG_Iq;                /* QC�`�􅢔�Iq */
    uint8_t TG_CVq;               /* QCˮƽ��CV */
    uint8_t TG_Aq;                /* QC�`�􅢔�A */
    uint8_t TG_Bq;                /* QC�`�􅢔�B */
    uint8_t TG_QC[30];            /* QCģʽ�������� */
    uint8_t TG_S2;                /* ԇƬ�`�􅢔� */
    uint8_t TG_I2;                /* ԇƬ�`�􅢔� */
    float TG_Sr;                  /* OPAӲ�wޒ·�`��(б��) */
    float TG_Ir;                  /* OPAӲ�wޒ·�`��(�ؾ�) */
    uint8_t TG_S3[15];            /* ԇƬ�`�􅢔�S3(1-15) */
    uint8_t TG_I3[15];            /* ԇƬ�`�􅢔�I3(1-15) */
    uint8_t TG_TF[7];             /* AC/PC�ض��`��б��(10-40��C) */
    uint8_t TG_CTF[7];            /* QC�ض��`��б��(10-40��C) */
    uint8_t TG_TO[7];             /* AC/PC�ض��`���ؾ�(10-40��C) */
    uint8_t TG_CTO[7];            /* QC�ض��`���ؾ�(10-40��C) */

    /* 9. Reserved and checksum */
    uint8_t Reserved[36];         /* ����^��δ��Uչ */
    uint8_t SUM_L;                /* ��ַ0~674У��(��) */
    uint8_t SUM_H;                /* ��ַ0~674У��(��) */
    uint8_t CRC16;                /* ����������CRCУ� */
} ParamTable_TypeDef;

/* Function prototypes */
uint8_t Param_Init(void);
uint8_t Param_LoadDefault(void);
uint8_t Param_Save(void);
uint8_t Param_Verify(void);
uint16_t Param_CalculateChecksum(void);
uint8_t Param_CalculateCRC16(void);

/* Parameter utility functions */
int32_t Param_GetValue(Param_ID_TypeDef id);
uint8_t Param_SetValue(Param_ID_TypeDef id, int32_t value);
const char* Param_GetStripTypeName(uint8_t type);
const char* Param_GetEventTypeName(uint8_t event);
const char* Param_GetUnitName(uint8_t unit);

/* External variables */
extern ParamTable_TypeDef gParamTable;

/*
 * Test function for parameter table operations
 */
void ParamTable_Test(void);

#endif /* __PARAM_TABLE_H */ 