/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_param.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/06
 * Description        : CH32V203 Flash 參數儲存方案定義
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

/* 參數區地址定義 */
#define FLASH_PARAM_BLOCK_A_ADDR        0x0800D800  /* 參數區塊A起始地址 */
#define FLASH_PARAM_BLOCK_B_ADDR        0x0800E000  /* 參數區塊B起始地址 */
#define FLASH_RECORD_BLOCK_ADDR         0x0800E800  /* 記錄區起始地址 */
#define FLASH_PARAM_MAX_SIZE            2048        /* 參數區最大尺寸 */
#define FLASH_PAGE_SIZE                 1024        /* Flash頁大小 */

/* 參數塊類型定義 */
#define BLOCK_BASIC_SYSTEM              0x01        /* 基本系統參數區 */
#define BLOCK_HARDWARE_CALIB            0x02        /* 硬體校準參數區 */
#define BLOCK_BG_PARAMS                 0x03        /* 血糖測試參數區 */
#define BLOCK_U_PARAMS                  0x04        /* 尿酸測試參數區 */
#define BLOCK_C_PARAMS                  0x05        /* 總膽固醇測試參數區 */
#define BLOCK_TG_PARAMS                 0x06        /* 三酸甘油脂測試參數區 */
#define BLOCK_RESERVED                  0x07        /* 保留參數區 */

/* 參數操作結果 */
typedef enum {
    PARAM_SUCCESS = 0,           /* 成功 */
    PARAM_ERROR_INVALID,         /* 無效的參數區 */
    PARAM_ERROR_EMPTY,           /* 參數區為空 */
    PARAM_ERROR_CHECKSUM,        /* 校驗和錯誤 */
    PARAM_ERROR_SIZE,            /* 參數尺寸錯誤 */
    PARAM_ERROR_FLASH,           /* Flash操作錯誤 */
    PARAM_ERROR_RANGE            /* 參數範圍錯誤 */
} FLASH_ParamResult_TypeDef;

/* 參數區塊標頭結構 */
typedef struct {
    uint16_t magic;              /* 參數區塊標頭魔數 (0xA55A) */
    uint8_t version;             /* 參數區塊版本號 */
    uint8_t blockType;           /* 區塊類型 */
    uint32_t writeCounter;       /* 寫入計數器 */
    uint32_t timestamp;          /* 時間戳 */
    uint16_t dataSize;           /* 數據大小 */
    uint16_t checksum;           /* 校驗和 */
} FLASH_ParamBlockHeader_TypeDef;

/* 參數區塊結構 */
typedef struct {
    FLASH_ParamBlockHeader_TypeDef header;
    uint8_t data[FLASH_PARAM_MAX_SIZE - sizeof(FLASH_ParamBlockHeader_TypeDef)];
} FLASH_ParamBlock_TypeDef;

/* 參數區操作函數 */
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

/* Flash基本操作函數 */
FLASH_ParamResult_TypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint16_t size);
FLASH_ParamResult_TypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_PARAM_H */