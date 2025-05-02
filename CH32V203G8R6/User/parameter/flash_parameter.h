/********************************** (C) COPYRIGHT *******************************
 * File Name          : flash_parameter.h
 * Author             : WCH & HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������xFlash�������淽��
*********************************************************************************
* Copyright (c) 2025 HMD Co., Ltd.
* All rights reserved.
*******************************************************************************/

#ifndef __FLASH_PARAMETER_H
#define __FLASH_PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include "ch32v20x_flash.h"
#include <stdbool.h>
#include <string.h>

/* �����^����� */
#define PARAM_BLOCK_A_ADDR       ((uint32_t)0x0800D800)  // �����^A����ʼ��ַ
#define PARAM_BLOCK_B_ADDR       ((uint32_t)0x0800E000)  // �����^B����ʼ��ַ
#define RECORD_AREA_ADDR         ((uint32_t)0x0800E800)  // �yԇӛ䛅^����ʼ��ַ
#define PARAM_BLOCK_SIZE         ((uint32_t)0x0800)      // �����^��С (2KB)
#define RECORD_AREA_SIZE         ((uint32_t)0x1800)      // �yԇӛ䛅^��С (6KB)

/* �����K���R�� */
#define PARAM_MAGIC_NUMBER       ((uint16_t)0xA55A)      // �����Kħ��
#define PARAM_VERSION            ((uint8_t)0x01)         // �����汾

/* �����^�K��Ͷ��x */
#define BLOCK_BASIC_SYSTEM       ((uint8_t)0x00)         // ����ϵ�y����
#define BLOCK_HARDWARE_CALIB     ((uint8_t)0x01)         // Ӳ�wУ�ʅ���
#define BLOCK_GLUCOSE            ((uint8_t)0x02)         // Ѫ��(GLV/GAV)����
#define BLOCK_URIC_ACID          ((uint8_t)0x03)         // ����(U)����
#define BLOCK_CHOLESTEROL        ((uint8_t)0x04)         // ��đ�̴�(C)����
#define BLOCK_TRIGLYCERIDE       ((uint8_t)0x05)         // �������֬(TG)����
#define BLOCK_RESERVED           ((uint8_t)0x06)         // ���􅢔�
#define BLOCK_CHECKSUM           ((uint8_t)0x07)         // У򞅢��

/* �����K�^���Y�� */
typedef struct {
    uint16_t magic;              // ħ�� (0xA55A)
    uint8_t  version;            // �汾̖
    uint16_t writeCounter;       // ����Δ�Ӌ����
    uint32_t timestamp;          // �r�g��
    uint16_t checksum;           // У��
} ParamBlockHeader;

/* ����ϵ�y�����Y�� */
typedef struct {
    uint8_t  lbt;                // ��늳��ֵ (��λ: 0.01V)
    uint8_t  obt;                // ����늳��ֵ (��λ: 0.01V)
    uint8_t  factory;            // ���Sģʽ���I (0=ʹ����ģʽ, 1=���Sģʽ)
    uint8_t  modelNo;            // �aƷ��̖
    uint8_t  fwNo;               // �g�w�汾̖
    uint32_t testCount;          // �yԇ�Δ���Ӌ
    uint8_t  codeTableVer;       // ������汾̖
    uint8_t  year;               // �� (0-99)
    uint8_t  month;              // �� (1-12)
    uint8_t  date;               // �� (1-31)
    uint8_t  hour;               // �r (0-23)
    uint8_t  minute;             // �� (0-59)
    uint8_t  second;             // �� (0-59)
    uint8_t  tempLowLimit;       // �ض����� (��λ: ��C)
    uint8_t  tempHighLimit;      // �ض����� (��λ: ��C)
    uint8_t  measureUnit;        // �y����λ (0=mmol/L, 1=mg/dL)
    uint8_t  defaultEvent;       // Ĭ�J�¼� (0=GP, 1=AC, 2=PC)
    uint8_t  stripType;          // ԇƬ��� (0=GLV, 1=U, 2=C, 3=TG, 4=GAV)
    uint8_t  reserved[2];        // ����
} BasicSystemBlock;

/* Ӳ�wУ�ʅ����Y�� */
typedef struct {
    uint16_t evT3Trigger;        // EV늉��ֵ
    uint16_t evWorking;          // EV����늉�
    uint16_t evT3;               // EV T3늉�
    int8_t   dacoOffset;         // DACOƫ��
    int8_t   dacdo;              // DACDOƫ��
    uint8_t  cc211Status;        // CC211��B
    uint8_t  calTolerance;       // У���ݲ� (��λ: 0.1%)
    float    ops;                // �\��Ŵ���У��б��
    float    opi;                // �\��Ŵ���У�ʽؾ�
    uint8_t  qct;                // Ʒ�ܜyԇ�Y��
    int8_t   tempOffset;         // �ض�ƫ���a�� (��λ: 0.1��C)
    int8_t   batteryOffset;      // 늳�ƫ���a�� (��λ: 10mV)
    uint8_t  reserved[3];        // ����
} HardwareCalibBlock;

/* Ѫ�ǅ����Y�� */
typedef struct {
    uint8_t  bgCsuTolerance;     // Ѫ��CSU�ݲ� (��λ: 0.1%)
    uint16_t bgNdl;              // ��ԇƬ�z�yˮƽ
    uint16_t bgUdl;              // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t bgBloodIn;          // ѪҺ�z�yˮƽ
    char     bgStripLot[16];     // ԇƬ��̖
    uint8_t  bgL;                // ��Ѫ���@ʾ�ֵ (��λ: 10 mg/dL)
    uint8_t  bgH;                // ��Ѫ���@ʾ�ֵ (��λ: 10 mg/dL)
    uint16_t bgT3E37;            // T3 ADV�e�`�ֵ

    // �yԇ�r�򅢔� (�M1)
    uint16_t bgTPL1;             // �r��TPL1 (��λ: 0.1��)
    uint16_t bgTRD1;             // �r��TRD1 (��λ: 0.1��)
    uint16_t bgEVWidth1;         // �r��EVWidth1 (��λ: 0.1��)

    // �yԇ�r�򅢔� (�M2)
    uint16_t bgTPL2;             // �r��TPL2 (��λ: 0.1��)
    uint16_t bgTRD2;             // �r��TRD2 (��λ: 0.1��)
    uint16_t bgEVWidth2;         // �r��EVWidth2 (��λ: 0.1��)

    // �ض��a���S��
    float    bgTempA;            // �ض��a���S��A
    float    bgTempB;            // �ض��a���S��B
    float    bgTempC;            // �ض��a���S��C

    // �����a���S��
    float    bgAltA;             // �����a���S��A
    float    bgAltB;             // �����a���S��B
    float    bgAltC;             // �����a���S��C

    // Ѫ�����a���S��
    float    bgHctA;             // Ѫ�����a���S��A
    float    bgHctB;             // Ѫ�����a���S��B
    float    bgHctC;             // Ѫ�����a���S��C

    // �a���ؾ�ͱ����S��
    float    bgCompS;            // �a�������S��
    float    bgCompI;            // �a���ؾ��S��

    // ����
    uint8_t  reserved[10];       // ����
} GlucoseParamBlock;

/* ���ᅢ���Y�� */
typedef struct {
    uint8_t  uaCsuTolerance;     // ����CSU�ݲ� (��λ: 0.1%)
    uint16_t uaNdl;              // ��ԇƬ�z�yˮƽ
    uint16_t uaUdl;              // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t uaBloodIn;          // ѪҺ�z�yˮƽ
    char     uaStripLot[16];     // ԇƬ��̖
    uint8_t  uaL;                // �������@ʾ�ֵ (��λ: 0.1 mg/dL)
    uint8_t  uaH;                // �������@ʾ�ֵ (��λ: 0.1 mg/dL)
    uint16_t uaT3E37;            // T3 ADV�e�`�ֵ

    // �yԇ�r�򅢔� (�M1)
    uint16_t uaTPL1;             // �r��TPL1 (��λ: 0.1��)
    uint16_t uaTRD1;             // �r��TRD1 (��λ: 0.1��)
    uint16_t uaEVWidth1;         // �r��EVWidth1 (��λ: 0.1��)

    // �yԇ�r�򅢔� (�M2)
    uint16_t uaTPL2;             // �r��TPL2 (��λ: 0.1��)
    uint16_t uaTRD2;             // �r��TRD2 (��λ: 0.1��)
    uint16_t uaEVWidth2;         // �r��EVWidth2 (��λ: 0.1��)

    // �ض��a���S��
    float    uaTempA;            // �ض��a���S��A
    float    uaTempB;            // �ض��a���S��B
    float    uaTempC;            // �ض��a���S��C

    // �����a���S��
    float    uaAltA;             // �����a���S��A
    float    uaAltB;             // �����a���S��B
    float    uaAltC;             // �����a���S��C

    // �a���ؾ�ͱ����S��
    float    uaCompS;            // �a�������S��
    float    uaCompI;            // �a���ؾ��S��

    // ����
    uint8_t  reserved[10];       // ����
} UricAcidParamBlock;

/* ��đ�̴������Y�� */
typedef struct {
    uint8_t  chCsuTolerance;     // đ�̴�CSU�ݲ� (��λ: 0.1%)
    uint16_t chNdl;              // ��ԇƬ�z�yˮƽ
    uint16_t chUdl;              // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t chBloodIn;          // ѪҺ�z�yˮƽ
    char     chStripLot[16];     // ԇƬ��̖
    uint8_t  chL;                // ��đ�̴��@ʾ�ֵ (��λ: mg/dL)
    uint16_t chH;                // ��đ�̴��@ʾ�ֵ (��λ: mg/dL)
    uint16_t chT3E37;            // T3 ADV�e�`�ֵ

    // �yԇ�r�򅢔� (�M1)
    uint16_t chTPL1;             // �r��TPL1 (��λ: 0.1��)
    uint16_t chTRD1;             // �r��TRD1 (��λ: 0.1��)
    uint16_t chEVWidth1;         // �r��EVWidth1 (��λ: 0.1��)

    // �yԇ�r�򅢔� (�M2)
    uint16_t chTPL2;             // �r��TPL2 (��λ: 0.1��)
    uint16_t chTRD2;             // �r��TRD2 (��λ: 0.1��)
    uint16_t chEVWidth2;         // �r��EVWidth2 (��λ: 0.1��)

    // �ض��a���S��
    float    chTempA;            // �ض��a���S��A
    float    chTempB;            // �ض��a���S��B
    float    chTempC;            // �ض��a���S��C

    // �a���ؾ�ͱ����S��
    float    chCompS;            // �a�������S��
    float    chCompI;            // �a���ؾ��S��

    // ����
    uint8_t  reserved[20];       // ����
} CholesterolParamBlock;

/* �������֬�����Y�� */
typedef struct {
    uint8_t  tgCsuTolerance;     // �������֬CSU�ݲ� (��λ: 0.1%)
    uint16_t tgNdl;              // ��ԇƬ�z�yˮƽ
    uint16_t tgUdl;              // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t tgBloodIn;          // ѪҺ�z�yˮƽ
    char     tgStripLot[16];     // ԇƬ��̖
    uint8_t  tgL;                // ���������֬�@ʾ�ֵ (��λ: mg/dL)
    uint16_t tgH;                // ���������֬�@ʾ�ֵ (��λ: mg/dL)
    uint16_t tgT3E37;            // T3 ADV�e�`�ֵ

    // �yԇ�r�򅢔� (�M1)
    uint16_t tgTPL1;             // �r��TPL1 (��λ: 0.1��)
    uint16_t tgTRD1;             // �r��TRD1 (��λ: 0.1��)
    uint16_t tgEVWidth1;         // �r��EVWidth1 (��λ: 0.1��)

    // �yԇ�r�򅢔� (�M2)
    uint16_t tgTPL2;             // �r��TPL2 (��λ: 0.1��)
    uint16_t tgTRD2;             // �r��TRD2 (��λ: 0.1��)
    uint16_t tgEVWidth2;         // �r��EVWidth2 (��λ: 0.1��)

    // �ض��a���S��
    float    tgTempA;            // �ض��a���S��A
    float    tgTempB;            // �ض��a���S��B
    float    tgTempC;            // �ض��a���S��C

    // �a���ؾ�ͱ����S��
    float    tgCompS;            // �a�������S��
    float    tgCompI;            // �a���ؾ��S��

    // ����
    uint8_t  reserved[20];       // ����
} TriglycerideParamBlock;

/* ������У򞅢���Y�� */
typedef struct {
    uint8_t  reserved[39];       // ����
} ReservedBlock;

/* �������������Y�� */
typedef struct {
    BasicSystemBlock     basicSystem;    // ����ϵ�y����
    HardwareCalibBlock   hardwareCalib;  // Ӳ�wУ�ʅ���
    GlucoseParamBlock    bgParams;       // Ѫ�ǅ���
    UricAcidParamBlock   uaParams;       // ���ᅢ��
    CholesterolParamBlock chParams;      // ��đ�̴�����
    TriglycerideParamBlock tgParams;     // �������֬����
    ReservedBlock         reserved;      // ���􅢔�
} ParameterData;

/* ���������K�Y�� */
typedef struct {
    ParamBlockHeader header;      // �����K�^��
    ParameterData    params;      // ��������
} ParameterBlock;

/* �yԇӛ䛽Y�� */
typedef struct {
    uint16_t resultStatus;        // �yԇ�Y����B (0=�ɹ�������=�e�`)
    uint16_t resultValue;         // �yԇ�Y��ֵ
    uint16_t testType;            // �yԇ�Ŀ���
    uint16_t event;               // �¼����
    uint8_t  code;                // ԇƬ���δa
    uint16_t year;                // ��
    uint16_t month;               // ��
    uint16_t date;                // ��
    uint16_t hour;                // �r
    uint16_t minute;              // ��
    uint16_t second;              // ��
    uint16_t batteryVoltage;      // 늳�늉�
    uint16_t temperature;         // �h���ض�
} TestRecord;

/* ������ */
void PARAM_Init(void);
bool PARAM_ReadBlock(uint8_t blockType, void *data, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, void *data, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
uint32_t PARAM_GetTestCount(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_ResetToDefault(void);
bool PARAM_SaveTestRecord(TestRecord *record);
bool PARAM_GetTestRecord(uint32_t index, TestRecord *record);
uint32_t PARAM_GetTestRecordCount(void);
bool PARAM_EraseAllTestRecords(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_PARAMETER_H */