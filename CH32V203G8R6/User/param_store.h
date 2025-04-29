/************************************* (C) COPYRIGHT *******************************
 * File Name          : param_store.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter storage module for CH32V203G8R6 Flash.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAM_STORE_H
#define __PARAM_STORE_H

#include "ch32v20x.h"

/* Flash memory mapping */
#define PARAM_BLOCK_A_ADDR      0x0800D800    /* �����^A��ʼ��ַ */
#define PARAM_BLOCK_B_ADDR      0x0800E000    /* �����^B��ʼ��ַ */
#define PARAM_RECORD_ADDR       0x0800E800    /* �yԇӛ䛅^��ʼ��ַ */
#define FLASH_PAGE_SIZE         0x400         /* 1KBÿ� */

/* �����^�K���^�Y�� */
typedef struct {
    uint16_t magic;         /* �^�Kħ�g�a 0xA55A */
    uint16_t version;       /* ������汾̖ */
    uint16_t writeCounter;  /* ����Δ�Ӌ���� */
    uint32_t timestamp;     /* �r�g��ӛ */
} ParamBlockHeader_TypeDef;

/* �����^�K��Ͷ��x */
typedef enum {
    BLOCK_BASIC_SYSTEM = 0,     /* ����ϵ�y�����^ */
    BLOCK_HARDWARE_CALIB,       /* Ӳ�wУ�ʅ����^ */
    BLOCK_BG_PARAMS,            /* Ѫ��(GLV/GAV)�����^ */
    BLOCK_UA_PARAMS,            /* ����(U)�����^ */
    BLOCK_CHOL_PARAMS,          /* ��đ�̴�(C)�����^ */
    BLOCK_TG_PARAMS,            /* �������֬(TG)�����^ */
    BLOCK_RESERVED,             /* ����^���У�^ */
    BLOCK_COUNT                 /* �^�K���� */
} ParamBlockType;

/* ����ϵ�y�����^�K */
typedef struct {
    uint8_t lbt;                /* ��늳��ֵ */
    uint8_t obt;                /* 늳غıM�ֵ */
    uint8_t factory;            /* �x������ģʽ */
    uint8_t modelNo;            /* �aƷ��̖ */
    uint8_t fwNo;               /* �g�w�汾̖ */
    uint16_t testCount;         /* �yԇ�Δ� */
    uint16_t codeTableVer;      /* ���a��汾̖ */
    uint8_t year;               /* ����O�� (0-99) */
    uint8_t month;              /* �·��O�� (1-12) */
    uint8_t date;               /* �����O�� (1-31) */
    uint8_t hour;               /* С�r�O�� (0-23) */
    uint8_t minute;             /* ����O�� (0-59) */
    uint8_t second;             /* �딵�O�� (0-59) */
    uint8_t tempLowLimit;       /* �����ض����� */
    uint8_t tempHighLimit;      /* �����ض����� */
    uint8_t measureUnit;        /* �y����λ�O�� */
    uint8_t defaultEvent;       /* �A�O�¼���� */
    uint8_t stripType;          /* �yԇ�Ŀ��� */
    uint8_t reserved[2];        /* ���� */
} BasicSystemBlock;

/* Ӳ�wУ�ʅ����^�K */
typedef struct {
    uint16_t evT3Trigger;       /* EV_T3�|�l늉� */
    uint8_t evWorking;          /* �y������늉� */
    uint8_t evT3;               /* ѪҺע��늉� */
    uint8_t dacoOffset;         /* DACƫ���`�� */
    uint8_t dacdo;              /* DACУ������ */
    uint8_t cc211Status;        /* CC211��B */
    uint8_t calTolerance;       /* У���ݲ� */
    float ops;                  /* OPAУ��б�� */
    float opi;                  /* OPAУ���ؾ� */
    uint8_t qct;                /* QCTУ���yԇ */
    int8_t tempOffset;          /* �ض�У��ƫ�� */
    int8_t batteryOffset;       /* 늳�У��ƫ�� */
    uint8_t reserved[3];        /* ���� */
} HardwareCalibBlock;

/* �����yԇ�����^�K�Y�� */
typedef struct {
    uint8_t csuTolerance;       /* ԇƬ�z���ݲ� */
    uint16_t ndl;               /* ��ԇƬ늉�ˮƽ */
    uint16_t udl;               /* ��ʹ��ԇƬ늉�ˮƽ */
    uint16_t bloodIn;           /* ѪҺ늉�ˮƽ */
    uint8_t stripLot[16];       /* ԇƬ��̖ */
    uint8_t lowLimit;           /* �y������ */
    uint16_t highLimit;         /* �y������ */
    uint16_t t3E37;             /* T3�e�`37�ֵ */
    uint16_t tpl1;              /* �r�g�}�n����(��һ�M) */
    uint16_t trd1;              /* ԭʼ�����񼯕r�g(��һ�M) */
    uint16_t evWidth1;          /* ����r�g(��һ�M) */
    uint16_t tpl2;              /* �r�g�}�n����(�ڶ��M) */
    uint16_t trd2;              /* ԭʼ�����񼯕r�g(�ڶ��M) */
    uint16_t evWidth2;          /* ����r�g(�ڶ��M) */
    uint8_t sq;                 /* QC����Sq */
    uint8_t iq;                 /* QC����Iq */
    uint8_t cvq;                /* QCˮƽCV */
    uint8_t aq;                 /* QC����A */
    uint8_t bq;                 /* QC����B */
    uint8_t reserved[2];        /* ���� */
} TestParamBaseBlock;

/* �����ͺ���ԭ�� */
uint8_t PARAM_Init(void);
uint8_t PARAM_ReadBlock(ParamBlockType blockType, void *buffer, uint16_t size);
uint8_t PARAM_UpdateBlock(ParamBlockType blockType, const void *buffer, uint16_t size);
uint8_t PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
uint8_t PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
uint8_t PARAM_IncreaseTestCount(void);
uint8_t PARAM_ResetToDefault(void);
uint8_t PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
uint8_t PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
uint8_t PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t seqNum);
uint32_t PARAM_GetActiveBlockAddr(void);

#endif /* __PARAM_STORE_H */ 