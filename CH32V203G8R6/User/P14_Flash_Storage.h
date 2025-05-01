/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/25
 * Description        : CH32V203 Flash�������淽���^�ļ�
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __P14_FLASH_STORAGE_H__
#define __P14_FLASH_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ch32v20x.h"
#include "parameter_table.h"

/* �����탦��^���� */
#define PARAM_FLASH_BASE          0x08000000
#define PARAM_BLOCK_A_ADDR        0x0800D800  // �����^A��ʼ��ַ (55KB)
#define PARAM_BLOCK_B_ADDR        0x0800E000  // �����^B��ʼ��ַ (56KB)
#define PARAM_BLOCK_SIZE          0x800       // �����^��С (2KB)
#define PARAM_SECTOR_SIZE         0x400       // �����ȅ^��С (1KB)

/* �����^��^�K��� */
#define BLOCK_BASIC_SYSTEM        0  // ����ϵ�y����
#define BLOCK_HARDWARE_CALIB      1  // Ӳ�wУ������
#define BLOCK_GLUCOSE_PARAMS      2  // Ѫ��(GLV/GAV)����
#define BLOCK_URIC_ACID_PARAMS    3  // ����(U)����
#define BLOCK_CHOLESTEROL_PARAMS  4  // ��đ�̴�(C)����
#define BLOCK_TRIGLYCERIDE_PARAMS 5  // �������֬(TG)����
#define BLOCK_RESERVED            6  // ����^�K

/* ������Y�����x */
#pragma pack(1)

/* �������^���Y�� */
typedef struct {
    uint16_t magic;         // ħ�g�a (0xA55A)
    uint8_t version;        // ������汾
    uint16_t writeCounter;  // ����Ӌ����
    uint32_t timestamp;     // �r�g��ӛ
    uint16_t checksum;      // У��
} ParamHeader_TypeDef;

/* ����ϵ�y�����^�K */
typedef struct {
    uint8_t lbt;              // ��늳��ֵ
    uint8_t obt;              // 늳غıM�ֵ
    uint8_t factory;          // �x������ģʽ
    uint8_t modelNo;          // �aƷ��̖
    uint8_t fwNo;             // �g�w�汾̖
    uint16_t testCount;       // �yԇ�Δ�
    uint16_t codeTableVer;    // ���a��汾̖
    uint8_t year;             // ����O��
    uint8_t month;            // �·��O��
    uint8_t date;             // �����O��
    uint8_t hour;             // С�r�O��
    uint8_t minute;           // ����O��
    uint8_t second;           // �딵�O��
    uint8_t tempLowLimit;     // �����ضȹ�������
    uint8_t tempHighLimit;    // �����ضȹ�������
    uint8_t measureUnit;      // ��Ȇ�λ�O��
    uint8_t defaultEvent;     // ����¼����
    uint8_t stripType;        // �yԇ�Ŀ
    uint8_t reserved[2];      // �A��λ��
} BasicSystemBlock;

/* Ӳ�wУ�������^�K */
typedef struct {
    uint16_t evT3Trigger;    // EV_T3�|�l늉�
    uint8_t evWorking;       // ��ȹ������늉�
    uint8_t evT3;            // ѪҺ�������늉�
    uint8_t dacoOffset;      // DACƫ���a��
    uint8_t dacdo;           // DACУ�����텢��
    uint8_t cc211Status;     // CC211δ������
    uint8_t calTolerance;    // OPS/OPIУ���ݲ�
    float ops;               // OPAУ��б��
    float opi;               // OPAУ���ؾ�
    uint8_t qct;             // QCTУ���yԇ��λԪ�M
    int8_t tempOffset;       // �ض�У��ƫ��
    int8_t batteryOffset;    // 늳�У��ƫ��
    uint8_t reserved[5];     // �A��λ��
} HardwareCalibBlock;

/* Ѫ�ǅ����^�K */
typedef struct {
    uint8_t bgCsuTolerance;  // ԇƬ�z���ݲ�
    uint16_t bgNdl;          // ��ԇƬ���ˮƽ
    uint16_t bgUdl;          // ��ʹ��ԇƬ���ˮƽ
    uint16_t bgBloodIn;      // ѪҺ���ˮƽ
    uint8_t bgStripLot[16];  // ԇƬ��̖
    uint8_t bgL;             // Ѫ��ֵ����
    uint16_t bgH;            // Ѫ��ֵ����
    uint16_t bgT3E37;        // T3 ADV��E37�ֵ
    uint16_t bgTPL1;         // ��һ�M�r�򅢔�TPL
    uint16_t bgTRD1;         // ��һ�M�r�򅢔�TRD
    uint16_t bgEVWidth1;     // ��һ�M�r�򅢔�EV����
    uint16_t bgTPL2;         // �ڶ��M�r�򅢔�TPL
    uint16_t bgTRD2;         // �ڶ��M�r�򅢔�TRD
    uint16_t bgEVWidth2;     // �ڶ��M�r�򅢔�EV����
    uint8_t bgCompA;         // �a������A
    uint8_t bgCompB;         // �a������B
    uint8_t bgCompC;         // �a������C
    uint8_t bgCompD;         // �a������D
    uint8_t reserved[128];   // �A��λ��
} GlucoseParamBlock;

/* ���ᅢ���^�K */
typedef struct {
    uint8_t uCsuTolerance;   // ԇƬ�z���ݲ�
    uint16_t uNdl;           // ��ԇƬ���ˮƽ
    uint16_t uUdl;           // ��ʹ��ԇƬ���ˮƽ
    uint16_t uBloodIn;       // ѪҺ���ˮƽ
    uint8_t uStripLot[16];   // ԇƬ��̖
    uint8_t uL;              // ����ֵ����
    uint16_t uH;             // ����ֵ����
    uint16_t uT3E37;         // T3 ADV��E37�ֵ
    uint16_t uTPL1;          // ��һ�M�r�򅢔�TPL
    uint16_t uTRD1;          // ��һ�M�r�򅢔�TRD
    uint16_t uEVWidth1;      // ��һ�M�r�򅢔�EV����
    uint16_t uTPL2;          // �ڶ��M�r�򅢔�TPL
    uint16_t uTRD2;          // �ڶ��M�r�򅢔�TRD
    uint16_t uEVWidth2;      // �ڶ��M�r�򅢔�EV����
    uint8_t uCompA;          // �a������A
    uint8_t uCompB;          // �a������B
    uint8_t uCompC;          // �a������C
    uint8_t uCompD;          // �a������D
    uint8_t reserved[128];   // �A��λ��
} UricAcidParamBlock;

/* ��đ�̴������^�K */
typedef struct {
    uint8_t cCsuTolerance;   // ԇƬ�z���ݲ�
    uint16_t cNdl;           // ��ԇƬ���ˮƽ
    uint16_t cUdl;           // ��ʹ��ԇƬ���ˮƽ
    uint16_t cBloodIn;       // ѪҺ���ˮƽ
    uint8_t cStripLot[16];   // ԇƬ��̖
    uint8_t cL;              // đ�̴�ֵ����
    uint16_t cH;             // đ�̴�ֵ����
    uint16_t cT3E37;         // T3 ADV��E37�ֵ
    uint16_t cTPL1;          // ��һ�M�r�򅢔�TPL
    uint16_t cTRD1;          // ��һ�M�r�򅢔�TRD
    uint16_t cEVWidth1;      // ��һ�M�r�򅢔�EV����
    uint16_t cTPL2;          // �ڶ��M�r�򅢔�TPL
    uint16_t cTRD2;          // �ڶ��M�r�򅢔�TRD
    uint16_t cEVWidth2;      // �ڶ��M�r�򅢔�EV����
    uint8_t cCompA;          // �a������A
    uint8_t cCompB;          // �a������B
    uint8_t cCompC;          // �a������C
    uint8_t cCompD;          // �a������D
    uint8_t reserved[128];   // �A��λ��
} CholesterolParamBlock;

/* �������֬�����^�K */
typedef struct {
    uint8_t tgCsuTolerance;  // ԇƬ�z���ݲ�
    uint16_t tgNdl;          // ��ԇƬ���ˮƽ
    uint16_t tgUdl;          // ��ʹ��ԇƬ���ˮƽ
    uint16_t tgBloodIn;      // ѪҺ���ˮƽ
    uint8_t tgStripLot[16];  // ԇƬ��̖
    uint8_t tgL;             // �������ֵ֬����
    uint16_t tgH;            // �������ֵ֬����
    uint16_t tgT3E37;        // T3 ADV��E37�ֵ
    uint16_t tgTPL1;         // ��һ�M�r�򅢔�TPL
    uint16_t tgTRD1;         // ��һ�M�r�򅢔�TRD
    uint16_t tgEVWidth1;     // ��һ�M�r�򅢔�EV����
    uint16_t tgTPL2;         // �ڶ��M�r�򅢔�TPL
    uint16_t tgTRD2;         // �ڶ��M�r�򅢔�TRD
    uint16_t tgEVWidth2;     // �ڶ��M�r�򅢔�EV����
    uint8_t tgCompA;         // �a������A
    uint8_t tgCompB;         // �a������B
    uint8_t tgCompC;         // �a������C
    uint8_t tgCompD;         // �a������D
    uint8_t reserved[128];   // �A��λ��
} TriglycerideParamBlock;

/* ����^�K */
typedef struct {
    uint8_t reserved[120];   // �A��λ��
    uint16_t calibChecksum;  // У�^У��
} ReservedBlock;

/* �����^���Y�� */
typedef struct {
    BasicSystemBlock basicSystem;           // ����ϵ�y����
    HardwareCalibBlock hardwareCalib;       // Ӳ�wУ������
    GlucoseParamBlock bgParams;             // Ѫ�ǅ���
    UricAcidParamBlock uaParams;            // ���ᅢ��
    CholesterolParamBlock cholParams;       // ��đ�̴�����
    TriglycerideParamBlock tgParams;        // �������֬����
    ReservedBlock reserved;                 // ����^�K
} ParameterBlocks;

/* ����������Y�� */
typedef struct {
    ParamHeader_TypeDef header;             // �������^��
    ParameterBlocks params;                 // �����^
} ParameterBlock;

#pragma pack()

/* �������� */
void FLASH_Storage_Init(void);
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size);
bool FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t size);
bool PARAM_ReadParameterBlock(uint8_t blockType, void *buffer, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
void PARAM_ResetToDefault(void);

#ifdef __cplusplus
}
#endif

#endif /* __P14_FLASH_STORAGE_H__ */ 