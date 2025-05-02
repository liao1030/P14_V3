/**
 * @file param_storage.h
 * @brief CH32V203G8R6�๦�܅���Flash���淽���^�ļ�
 * @version 1.0
 * @date 2024-09-25
 * 
 * @copyright Copyright (c) 2024 HMD Biomedical Inc.
 */

#ifndef __PARAM_STORAGE_H__
#define __PARAM_STORAGE_H__

#include "ch32v20x.h"
#include "param_table.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* �����^�����P���x */
#define PARAM_BLOCK_A_ADDR      0x0800D800  // �����^A��ʼ��ַ
#define PARAM_BLOCK_B_ADDR      0x0800E000  // �����^B��ʼ��ַ
#define PARAM_RECORD_ADDR       0x0800E800  // �yԇӛ䛅^��ʼ��ַ
#define PARAM_BLOCK_SIZE        2048        // �����^��С(2KB)

/* �����^�K�R�e�a */
#define BLOCK_BASIC_SYSTEM      0x00        // ����ϵ�y�^
#define BLOCK_HARDWARE_CALIB    0x01        // Ӳ�wУ�ʅ^
#define BLOCK_BG_PARAMS         0x02        // Ѫ��(GLV/GAV)�����^
#define BLOCK_UA_PARAMS         0x03        // ����(U)�����^
#define BLOCK_CHOL_PARAMS       0x04        // ��đ�̴�(C)�����^
#define BLOCK_TG_PARAMS         0x05        // �������֬(TG)�����^
#define BLOCK_RESERVED          0x06        // ����^��
#define BLOCK_MAX               0x07        // ���^�K�R�e�a

/* �����^�KHeader���x */
typedef struct {
    uint16_t magic;               // ħ��(0xA55A)
    uint8_t  version;             // �汾̖
    uint8_t  writeCounter;        // ����Ӌ����
    uint32_t timestamp;           // ����r�g��
} BlockHeader_TypeDef;

/* ����ϵ�y�����^�K */
typedef struct {
    uint8_t  lbt;                 // ��늳��ֵ
    uint8_t  obt;                 // 늳غıM�ֵ
    uint8_t  factory;             // �x������ģʽ
    uint8_t  modelNo;             // �aƷ��̖
    uint8_t  fwNo;                // �g�w�汾̖
    uint16_t testCount;           // �yԇ�Δ�
    uint16_t codeTableVer;        // ���a��汾��̖
    uint8_t  year;                // ����O��
    uint8_t  month;               // �·��O��
    uint8_t  date;                // �����O��
    uint8_t  hour;                // С�r�O��
    uint8_t  minute;              // ����O��
    uint8_t  second;              // �딵�O��
    uint8_t  tempLowLimit;        // �����ضȹ�������
    uint8_t  tempHighLimit;       // �����ضȹ�������
    uint8_t  measureUnit;         // �y����λ�O��
    uint8_t  defaultEvent;        // �A�O�¼����
    uint8_t  stripType;           // �yԇ�Ŀ
} BasicSystemBlock;

/* Ӳ�wУ�ʅ����^�K */
typedef struct {
    uint16_t evT3Trigger;         // EV_T3�|�l늉�
    uint8_t  evWorking;           // �y������늘O늉�
    uint8_t  evT3;                // ѪҺ�z�y늘O늉�
    uint8_t  dacoOffset;          // DACƫ���a��
    uint8_t  dacdo;               // DACУ�����텢��
    uint8_t  cc211Status;         // CC211��B���
    uint8_t  calTolerance;        // OPS/OPIУ���ݲ�
    float    ops;                 // OPAУ��б��
    float    opi;                 // OPAУ�ʽؾ�
    uint8_t  qct;                 // QCTУ�ʜyԇ��λԪ�M
    int8_t   tempOffset;          // �ض�У��ƫ��
    int8_t   batteryOffset;       // 늳�У��ƫ��
} HardwareCalibBlock;

/* Ѫ��(GLV/GAV)�����^�K */
typedef struct {
    uint8_t  bgCsuTolerance;      // ԇƬ�z���ݲ�
    uint16_t bgNdl;               // ��ԇƬ�z�yˮƽ
    uint16_t bgUdl;               // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t bgBloodIn;           // ѪҺ�z�yˮƽ
    uint8_t  bgStripLot[16];      // ԇƬ��̖
    uint8_t  bgL;                 // Ѫ��ֵ����
    uint16_t bgH;                 // Ѫ��ֵ����
    uint16_t bgT3E37;             // T3 ADV�e�`37�ֵ
    uint16_t bgTPL1;              // �r�g�}�n��(��һ�M)
    uint16_t bgTRD1;              // ԭʼ�����r�g(��һ�M)
    uint16_t bgEVWidth1;          // ȼ���r�g(��һ�M)
    uint16_t bgTPL2;              // �r�g�}�n��(�ڶ��M)
    uint16_t bgTRD2;              // ԭʼ�����r�g(�ڶ��M)
    uint16_t bgEVWidth2;          // ȼ���r�g(�ڶ��M)
    uint8_t  bgSq;                // QC�a������Sq
    uint8_t  bgIq;                // QC�a������Iq
    uint8_t  bgCVq;               // QCˮƽ��CV
    uint8_t  bgAq;                // QC�a������A
    uint8_t  bgBq;                // QC�a������B
    uint8_t  bgAq5;               // QC�a������A(�ȼ�5)
    uint8_t  bgBq5;               // QC�a������B(�ȼ�5)
    uint8_t  bgQC[30];            // QC�˜ʹ�������
    uint8_t  bgS2;                // ԇƬ�a������
    uint8_t  bgI2;                // ԇƬ�a������
    float    bgSr;                // OPAӲ�wޒ·�a��(б��)
    float    bgIr;                // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  bgS3[30];            // ԇƬ�a������S3(1-30)
    uint8_t  bgI3[30];            // ԇƬ�a������I3(1-30)
    uint8_t  bgTF[7];             // AC/PC�ض��a��б��(10-40��C)
    uint8_t  bgCTF[7];            // QC�ض��a��б��(10-40��C)
    uint8_t  bgTO[7];             // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  bgCTO[7];            // QC�ض��a���ؾ�(10-40��C)
} BloodGlucoseBlock;

/* ����(U)�����^�K */
typedef struct {
    uint8_t  uCsuTolerance;       // ԇƬ�z���ݲ�
    uint16_t uNdl;                // ��ԇƬ�z�yˮƽ
    uint16_t uUdl;                // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t uBloodIn;            // ѪҺ�z�yˮƽ
    uint8_t  uStripLot[16];       // ԇƬ��̖
    uint8_t  uL;                  // ����ֵ����
    uint16_t uH;                  // ����ֵ����
    uint16_t uT3E37;              // T3 ADV�e�`37�ֵ
    uint16_t uTPL1;               // �r�g�}�n��(��һ�M)
    uint16_t uTRD1;               // ԭʼ�����r�g(��һ�M)
    uint16_t uEVWidth1;           // ȼ���r�g(��һ�M)
    uint16_t uTPL2;               // �r�g�}�n��(�ڶ��M)
    uint16_t uTRD2;               // ԭʼ�����r�g(�ڶ��M)
    uint16_t uEVWidth2;           // ȼ���r�g(�ڶ��M)
    uint8_t  uSq;                 // QC�a������Sq
    uint8_t  uIq;                 // QC�a������Iq
    uint8_t  uCVq;                // QCˮƽ��CV
    uint8_t  uAq;                 // QC�a������A
    uint8_t  uBq;                 // QC�a������B
    uint8_t  uQC[30];             // QC�˜ʹ�������
    uint8_t  uS2;                 // ԇƬ�a������
    uint8_t  uI2;                 // ԇƬ�a������
    float    uSr;                 // OPAӲ�wޒ·�a��(б��)
    float    uIr;                 // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  uS3[15];             // ԇƬ�a������S3(1-15)
    uint8_t  uI3[15];             // ԇƬ�a������I3(1-15)
    uint8_t  uTF[7];              // AC/PC�ض��a��б��(10-40��C)
    uint8_t  uCTF[7];             // QC�ض��a��б��(10-40��C)
    uint8_t  uTO[7];              // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  uCTO[7];             // QC�ض��a���ؾ�(10-40��C)
} UricAcidBlock;

/* ��đ�̴�(C)�����^�K */
typedef struct {
    uint8_t  cCsuTolerance;       // ԇƬ�z���ݲ�
    uint16_t cNdl;                // ��ԇƬ�z�yˮƽ
    uint16_t cUdl;                // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t cBloodIn;            // ѪҺ�z�yˮƽ
    uint8_t  cStripLot[16];       // ԇƬ��̖
    uint8_t  cL;                  // đ�̴�ֵ����
    uint16_t cH;                  // đ�̴�ֵ����
    uint16_t cT3E37;              // T3 ADV�e�`37�ֵ
    uint16_t cTPL1;               // �r�g�}�n��(��һ�M)
    uint16_t cTRD1;               // ԭʼ�����r�g(��һ�M)
    uint16_t cEVWidth1;           // ȼ���r�g(��һ�M)
    uint16_t cTPL2;               // �r�g�}�n��(�ڶ��M)
    uint16_t cTRD2;               // ԭʼ�����r�g(�ڶ��M)
    uint16_t cEVWidth2;           // ȼ���r�g(�ڶ��M)
    uint8_t  cSq;                 // QC�a������Sq
    uint8_t  cIq;                 // QC�a������Iq
    uint8_t  cCVq;                // QCˮƽ��CV
    uint8_t  cAq;                 // QC�a������A
    uint8_t  cBq;                 // QC�a������B
    uint8_t  cQC[30];             // QC�˜ʹ�������
    uint8_t  cS2;                 // ԇƬ�a������
    uint8_t  cI2;                 // ԇƬ�a������
    float    cSr;                 // OPAӲ�wޒ·�a��(б��)
    float    cIr;                 // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  cS3[15];             // ԇƬ�a������S3(1-15)
    uint8_t  cI3[15];             // ԇƬ�a������I3(1-15)
    uint8_t  cTF[7];              // AC/PC�ض��a��б��(10-40��C)
    uint8_t  cCTF[7];             // QC�ض��a��б��(10-40��C)
    uint8_t  cTO[7];              // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  cCTO[7];             // QC�ض��a���ؾ�(10-40��C)
} CholesterolBlock;

/* �������֬(TG)�����^�K */
typedef struct {
    uint8_t  tgCsuTolerance;      // ԇƬ�z���ݲ�
    uint16_t tgNdl;               // ��ԇƬ�z�yˮƽ
    uint16_t tgUdl;               // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t tgBloodIn;           // ѪҺ�z�yˮƽ
    uint8_t  tgStripLot[16];      // ԇƬ��̖
    uint8_t  tgL;                 // �������ֵ֬����
    uint16_t tgH;                 // �������ֵ֬����
    uint16_t tgT3E37;             // T3 ADV�e�`37�ֵ
    uint16_t tgTPL1;              // �r�g�}�n��(��һ�M)
    uint16_t tgTRD1;              // ԭʼ�����r�g(��һ�M)
    uint16_t tgEVWidth1;          // ȼ���r�g(��һ�M)
    uint16_t tgTPL2;              // �r�g�}�n��(�ڶ��M)
    uint16_t tgTRD2;              // ԭʼ�����r�g(�ڶ��M)
    uint16_t tgEVWidth2;          // ȼ���r�g(�ڶ��M)
    uint8_t  tgSq;                // QC�a������Sq
    uint8_t  tgIq;                // QC�a������Iq
    uint8_t  tgCVq;               // QCˮƽ��CV
    uint8_t  tgAq;                // QC�a������A
    uint8_t  tgBq;                // QC�a������B
    uint8_t  tgQC[30];            // QC�˜ʹ�������
    uint8_t  tgS2;                // ԇƬ�a������
    uint8_t  tgI2;                // ԇƬ�a������
    float    tgSr;                // OPAӲ�wޒ·�a��(б��)
    float    tgIr;                // OPAӲ�wޒ·�a��(�ؾ�)
    uint8_t  tgS3[15];            // ԇƬ�a������S3(1-15)
    uint8_t  tgI3[15];            // ԇƬ�a������I3(1-15)
    uint8_t  tgTF[7];             // AC/PC�ض��a��б��(10-40��C)
    uint8_t  tgCTF[7];            // QC�ض��a��б��(10-40��C)
    uint8_t  tgTO[7];             // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t  tgCTO[7];            // QC�ض��a���ؾ�(10-40��C)
} TriglycerideBlock;

/* �����cУ�^�K */
typedef struct {
    uint8_t  reserved[36];        // ����^��
    uint16_t checksum;            // У��
    uint8_t  crc;                 // CRCУ�
} ReservedBlock;

/* �����^�K���w */
typedef union {
    BasicSystemBlock  basicSystem;
    HardwareCalibBlock hardwareCalib;
    BloodGlucoseBlock bgParams;
    UricAcidBlock     uaParams;
    CholesterolBlock  cholParams;
    TriglycerideBlock tgParams;
    ReservedBlock     reserved;
} ParameterBlockData;

/* �����^�K�����Y�� */
typedef struct {
    BlockHeader_TypeDef header;
    ParameterBlockData  params;
} ParameterBlock;

/* �yԇӛ䛽Y�� */
typedef struct {
    uint16_t resultStatus;    // �yԇ�Y����B (0=�ɹ�������=�e�`)
    uint16_t resultValue;     // �yԇ�Y��ֵ
    uint16_t testType;        // �yԇ�Ŀ���
    uint16_t eventType;       // �¼����
    uint8_t  stripCode;       // ԇƬУ���a
    uint16_t year;            // ��
    uint16_t month;           // ��
    uint16_t date;            // ��
    uint16_t hour;            // �r
    uint16_t minute;          // ��
    uint16_t second;          // ��
    uint16_t batteryVoltage;  // 늳�늉�
    uint16_t temperature;     // �h���ض�
} TestRecord;

/* ������ */
void PARAM_Init(void);
bool PARAM_ReadBlock(uint8_t blockId, void *data, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockId, const void *data, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_IncreaseTestCount(void);
bool PARAM_GetTestCount(uint16_t *count);
bool PARAM_ResetToFactory(void);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint16_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_SaveTestRecord(const TestRecord *record);
bool PARAM_ReadTestRecords(TestRecord *records, uint16_t startIdx, uint16_t count, uint16_t *actualCount);
uint16_t PARAM_GetTestRecordCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_STORAGE_H__ */ 