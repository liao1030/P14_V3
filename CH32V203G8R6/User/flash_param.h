/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_param.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : CH32V203 Flash �������淽�����x
 *********************************************************************************
 * Copyright (c) 2025 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __FLASH_PARAM_H
#define __FLASH_PARAM_H

#include "ch32v20x.h"
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* �����^��ַ���x */
#define FLASH_PARAM_BLOCK_A_ADDR        0x0800D800  /* �����^�KA��ʼ��ַ */
#define FLASH_PARAM_BLOCK_B_ADDR        0x0800E000  /* �����^�KB��ʼ��ַ */
#define FLASH_RECORD_BLOCK_ADDR         0x0800E800  /* ӛ䛅^��ʼ��ַ */
#define FLASH_PARAM_MAX_SIZE            2048        /* �����^���ߴ� */
#define FLASH_PAGE_SIZE                 1024        /* Flash퓴�С */

/* �����K��Ͷ��x */
#define BLOCK_BASIC_SYSTEM              0x01        /* ����ϵ�y�����^ */
#define BLOCK_HARDWARE_CALIB            0x02        /* Ӳ�wУ�ʅ����^ */
#define BLOCK_BG_PARAMS                 0x03        /* Ѫ�ǜyԇ�����^ */
#define BLOCK_U_PARAMS                  0x04        /* ����yԇ�����^ */
#define BLOCK_C_PARAMS                  0x05        /* ��đ�̴��yԇ�����^ */
#define BLOCK_TG_PARAMS                 0x06        /* �������֬�yԇ�����^ */
#define BLOCK_RESERVED                  0x07        /* ���􅢔��^ */

/* ���������Y�� */
typedef enum {
    PARAM_SUCCESS = 0,           /* �ɹ� */
    PARAM_ERROR_INVALID,         /* �oЧ�ą����^ */
    PARAM_ERROR_EMPTY,           /* �����^��� */
    PARAM_ERROR_CHECKSUM,        /* У���e�` */
    PARAM_ERROR_SIZE,            /* �����ߴ��e�` */
    PARAM_ERROR_FLASH,           /* Flash�����e�` */
    PARAM_ERROR_RANGE            /* ���������e�` */
} FLASH_ParamResult_TypeDef;

/* �����^�K���^�Y�� */
typedef struct {
    uint16_t magic;              /* �����^�K���^ħ�� (0xA55A) */
    uint8_t version;             /* �����^�K�汾̖ */
    uint8_t blockType;           /* �^�K��� */
    uint32_t writeCounter;       /* ����Ӌ���� */
    uint32_t timestamp;          /* �r�g�� */
    uint16_t dataSize;           /* ������С */
    uint16_t checksum;           /* У�� */
} FLASH_ParamBlockHeader_TypeDef;

/* �����^�K�Y�� */
typedef struct {
    FLASH_ParamBlockHeader_TypeDef header;
    uint8_t data[FLASH_PARAM_MAX_SIZE - sizeof(FLASH_ParamBlockHeader_TypeDef)];
} FLASH_ParamBlock_TypeDef;

/* �����^�������� */
FLASH_ParamResult_TypeDef PARAM_Init(void);
FLASH_ParamResult_TypeDef PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size);
FLASH_ParamResult_TypeDef PARAM_UpdateBlock(uint8_t blockType, const void *data, uint16_t size);
FLASH_ParamResult_TypeDef PARAM_IncreaseTestCount(void);
FLASH_ParamResult_TypeDef PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, 
                                              uint8_t hour, uint8_t minute, uint8_t second);
FLASH_ParamResult_TypeDef PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                                           uint8_t *hour, uint8_t *minute, uint8_t *second);
FLASH_ParamResult_TypeDef PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, 
                                                             uint16_t *udl, uint16_t *bloodIn);
FLASH_ParamResult_TypeDef PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, 
                                                              uint8_t *highLimit);
FLASH_ParamResult_TypeDef PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, 
                                                              uint16_t *trd, uint16_t *evWidth, 
                                                              uint8_t groupIndex);
FLASH_ParamResult_TypeDef PARAM_ResetToDefault(void);

/* Flash������������ */
FLASH_ParamResult_TypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint16_t size);
FLASH_ParamResult_TypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_PARAM_H */