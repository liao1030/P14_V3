/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_Flash_Storage.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/12
 * Description        : �๦�������yԇ�xFlash�������淽�����x
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_FLASH_STORAGE_H__
#define __P14_FLASH_STORAGE_H__

#include "ch32v20x.h"
#include "P14_Parameter_Table.h"
#include <stdbool.h>

/* ������^���x */
#define PARAM_BLOCK_A_ADDR       0x0800D800  // ������A�^��ʼ��ַ
#define PARAM_BLOCK_B_ADDR       0x0800E000  // ������B�^��ʼ��ַ
#define RECORD_ZONE_ADDR         0x0800E800  // �y��ӛ䛅^��ʼ��ַ
#define PARAM_PAGE_SIZE          1024        // Flash퓴�С(1KB)
#define PARAM_BLOCK_SIZE         2048        // ������^��С(2KB)

/* ������^�K���^���x */
typedef struct {
    uint16_t magic;         // ħ��,�̶���0xA55A
    uint16_t version;       // ������汾̖
    uint32_t writeCounter;  // ����Ӌ����
    uint32_t timestamp;     // �ϴθ��r�g��
    uint16_t checksum;      // ��������У��
    uint16_t crc16;         // �����^�KCRCУ�
} ParamBlockHeader;

/* ����ϵ�y�����^ */
typedef struct {
    uint8_t lbt;            // ��늳��ֵ
    uint8_t obt;            // 늳غıM�ֵ
    uint8_t factory;        // �x������ģʽ
    uint8_t modelNo;        // �aƷ��̖
    uint8_t fwNo;           // �g�w�汾̖
    uint16_t testCount;     // �y���Δ�
    uint16_t codeTableVer;  // ��̖��汾��̖
    
    uint8_t year;           // ��
    uint8_t month;          // ��
    uint8_t date;           // ��
    uint8_t hour;           // �r
    uint8_t minute;         // ��
    uint8_t second;         // ��
    
    uint8_t tempLowLimit;   // �����ض�����
    uint8_t tempHighLimit;  // �����ض�����
    uint8_t measureUnit;    // �y����λ�O��
    uint8_t defaultEvent;   // �A�O�¼����
    uint8_t stripType;      // �yԇ�Ŀ���
} BasicSystemBlock;

/* Ӳ�wУ�ʅ����^ */
typedef struct {
    uint16_t evT3Trigger;   // EV_T3�|�l늉�
    uint8_t evWorking;      // �y������늘O늉�
    uint8_t evT3;           // ѪҺ����늘O늉�
    uint8_t dacoOffset;     // DACƫ������
    uint8_t dacdo;          // DACУ�����텢��
    uint8_t cc211Status;    // CC211δ������
    uint8_t calTolerance;   // OPS/OPIУ���ݲ�
    float ops;              // OPAУ��б��
    float opi;              // OPAУ���ؾ�
    uint8_t qct;            // QCTУ���y��
    int8_t tempOffset;      // �ض�У��ƫ��
    int8_t batteryOffset;   // 늳�У��ƫ��
} HardwareCalibBlock;

/* �yԇ�Ŀ�����^ (��Ѫ�Ǟ���) */
typedef struct {
    uint8_t bgCsuTolerance;     // ԇƬ�z���ݲ�
    uint16_t bgNdl;             // ��ԇƬ�y��ˮƽ
    uint16_t bgUdl;             // ��ʹ��ԇƬ�y��ˮƽ
    uint16_t bgBloodIn;         // ѪҺ�y��ˮƽ
    char bgStripLot[16];        // ԇƬ��̖
    
    uint8_t bgL;                // Ѫ��ֵ����
    uint8_t bgH;                // Ѫ��ֵ����
    uint16_t bgT3E37;           // T3 ADV��37�ֵ
    
    uint16_t bgTPL1;            // �r�g�}�n����(��һ�M)
    uint16_t bgTRD1;            // ԭʼ�����r�g(��һ�M)
    uint16_t bgEVWidth1;        // �۟��r�g(��һ�M)
    uint16_t bgTPL2;            // �r�g�}�n����(�ڶ��M)
    uint16_t bgTRD2;            // ԭʼ�����r�g(�ڶ��M)
    uint16_t bgEVWidth2;        // �۟��r�g(�ڶ��M)
    
    // QC�����^
    uint8_t bgSq;               // QC���Ѕ���Sq
    uint8_t bgIq;               // QC���Ѕ���Iq
    uint8_t bgCvq;              // QCˮƽ��CV
    uint8_t bgAq;               // QC���Ѕ���A
    uint8_t bgBq;               // QC���Ѕ���B
    uint8_t bgAq5;              // QC���Ѕ���A(�ȼ�5)
    uint8_t bgBq5;              // QC���Ѕ���B(�ȼ�5)
    uint8_t bgQc[30];           // QCģ�幠������
    
    // �`���ȅ����^
    uint8_t bgS2;               // ԇƬ���Ѕ���
    uint8_t bgI2;               // ԇƬ���Ѕ���
    float bgSr;                 // OPAӲ�wޒ·����(б��)
    float bgIr;                 // OPAӲ�wޒ·����(�ؾ�)
    uint8_t bgS3[30];           // ԇƬ���Ѕ���S3
    uint8_t bgI3[30];           // ԇƬ���Ѕ���I3
    
    // �ض����Ѕ����^
    uint8_t bgTf[7];            // AC/PC�ض�����б��
    uint8_t bgCtf[7];           // QC�ض�����б��
    uint8_t bgTo[7];            // AC/PC�ض����нؾ�
    uint8_t bgCto[7];           // QC�ض����нؾ�
} BGParameterBlock;

typedef struct {
    uint8_t uCsuTolerance;      // ԇƬ�z���ݲ�
    uint16_t uNdl;              // ��ԇƬ�y��ˮƽ
    uint16_t uUdl;              // ��ʹ��ԇƬ�y��ˮƽ
    uint16_t uBloodIn;          // ѪҺ�y��ˮƽ
    char uStripLot[16];         // ԇƬ��̖
    
    uint8_t uL;                 // ����ֵ����
    uint8_t uH;                 // ����ֵ����
    uint16_t uT3E37;            // T3 ADV��37�ֵ
    
    uint16_t uTPL1;             // �r�g�}�n����(��һ�M)
    uint16_t uTRD1;             // ԭʼ�����r�g(��һ�M)
    uint16_t uEVWidth1;         // �۟��r�g(��һ�M)
    uint16_t uTPL2;             // �r�g�}�n����(�ڶ��M)
    uint16_t uTRD2;             // ԭʼ�����r�g(�ڶ��M)
    uint16_t uEVWidth2;         // �۟��r�g(�ڶ��M)
    
    // QC�����^
    uint8_t uSq;                // QC���Ѕ���Sq
    uint8_t uIq;                // QC���Ѕ���Iq
    uint8_t uCvq;               // QCˮƽ��CV
    uint8_t uAq;                // QC���Ѕ���A
    uint8_t uBq;                // QC���Ѕ���B
    uint8_t uQc[30];            // QCģ�幠������
    
    // �`���ȅ����^
    uint8_t uS2;                // ԇƬ���Ѕ���
    uint8_t uI2;                // ԇƬ���Ѕ���
    float uSr;                  // OPAӲ�wޒ·����(б��)
    float uIr;                  // OPAӲ�wޒ·����(�ؾ�)
    uint8_t uS3[15];            // ԇƬ���Ѕ���S3
    uint8_t uI3[15];            // ԇƬ���Ѕ���I3
    
    // �ض����Ѕ����^
    uint8_t uTf[7];             // AC/PC�ض�����б��
    uint8_t uCtf[7];            // QC�ض�����б��
    uint8_t uTo[7];             // AC/PC�ض����нؾ�
    uint8_t uCto[7];            // QC�ض����нؾ�
} UricAcidParameterBlock;

typedef struct {
    uint8_t cCsuTolerance;      // ԇƬ�z���ݲ�
    uint16_t cNdl;              // ��ԇƬ�y��ˮƽ
    uint16_t cUdl;              // ��ʹ��ԇƬ�y��ˮƽ
    uint16_t cBloodIn;          // ѪҺ�y��ˮƽ
    char cStripLot[16];         // ԇƬ��̖
    
    uint8_t cL;                 // đ�̴�ֵ����
    uint8_t cH;                 // đ�̴�ֵ����
    uint16_t cT3E37;            // T3 ADV��37�ֵ
    
    uint16_t cTPL1;             // �r�g�}�n����(��һ�M)
    uint16_t cTRD1;             // ԭʼ�����r�g(��һ�M)
    uint16_t cEVWidth1;         // �۟��r�g(��һ�M)
    uint16_t cTPL2;             // �r�g�}�n����(�ڶ��M)
    uint16_t cTRD2;             // ԭʼ�����r�g(�ڶ��M)
    uint16_t cEVWidth2;         // �۟��r�g(�ڶ��M)
    
    // QC�����^
    uint8_t cSq;                // QC���Ѕ���Sq
    uint8_t cIq;                // QC���Ѕ���Iq
    uint8_t cCvq;               // QCˮƽ��CV
    uint8_t cAq;                // QC���Ѕ���A
    uint8_t cBq;                // QC���Ѕ���B
    uint8_t cQc[30];            // QCģ�幠������
    
    // �`���ȅ����^
    uint8_t cS2;                // ԇƬ���Ѕ���
    uint8_t cI2;                // ԇƬ���Ѕ���
    float cSr;                  // OPAӲ�wޒ·����(б��)
    float cIr;                  // OPAӲ�wޒ·����(�ؾ�)
    uint8_t cS3[15];            // ԇƬ���Ѕ���S3
    uint8_t cI3[15];            // ԇƬ���Ѕ���I3
    
    // �ض����Ѕ����^
    uint8_t cTf[7];             // AC/PC�ض�����б��
    uint8_t cCtf[7];            // QC�ض�����б��
    uint8_t cTo[7];             // AC/PC�ض����нؾ�
    uint8_t cCto[7];            // QC�ض����нؾ�
} CholesterolParameterBlock;

typedef struct {
    uint8_t tgCsuTolerance;     // ԇƬ�z���ݲ�
    uint16_t tgNdl;             // ��ԇƬ�y��ˮƽ
    uint16_t tgUdl;             // ��ʹ��ԇƬ�y��ˮƽ
    uint16_t tgBloodIn;         // ѪҺ�y��ˮƽ
    char tgStripLot[16];        // ԇƬ��̖
    
    uint8_t tgL;                // �������ֵ֬����
    uint8_t tgH;                // �������ֵ֬����
    uint16_t tgT3E37;           // T3 ADV��37�ֵ
    
    uint16_t tgTPL1;            // �r�g�}�n����(��һ�M)
    uint16_t tgTRD1;            // ԭʼ�����r�g(��һ�M)
    uint16_t tgEVWidth1;        // �۟��r�g(��һ�M)
    uint16_t tgTPL2;            // �r�g�}�n����(�ڶ��M)
    uint16_t tgTRD2;            // ԭʼ�����r�g(�ڶ��M)
    uint16_t tgEVWidth2;        // �۟��r�g(�ڶ��M)
    
    // QC�����^
    uint8_t tgSq;               // QC���Ѕ���Sq
    uint8_t tgIq;               // QC���Ѕ���Iq
    uint8_t tgCvq;              // QCˮƽ��CV
    uint8_t tgAq;               // QC���Ѕ���A
    uint8_t tgBq;               // QC���Ѕ���B
    uint8_t tgQc[30];           // QCģ�幠������
    
    // �`���ȅ����^
    uint8_t tgS2;               // ԇƬ���Ѕ���
    uint8_t tgI2;               // ԇƬ���Ѕ���
    float tgSr;                 // OPAӲ�wޒ·����(б��)
    float tgIr;                 // OPAӲ�wޒ·����(�ؾ�)
    uint8_t tgS3[15];           // ԇƬ���Ѕ���S3
    uint8_t tgI3[15];           // ԇƬ���Ѕ���I3
    
    // �ض����Ѕ����^
    uint8_t tgTf[7];            // AC/PC�ض�����б��
    uint8_t tgCtf[7];           // QC�ض�����б��
    uint8_t tgTo[7];            // AC/PC�ض����нؾ�
    uint8_t tgCto[7];           // QC�ض����нؾ�
} TriglycerideParameterBlock;

/* ����^�cУ�^ */
typedef struct {
    uint8_t reserved[36];       // ����^��δ��Uչ
    uint16_t checksum;          // У��
    uint16_t crc16;             // CRCУ�
} ReservedBlock;

/* ���Ѕ����^�K�M�� */
typedef struct {
    BasicSystemBlock basicSystem;
    HardwareCalibBlock hardwareCalib;
    BGParameterBlock bgParams;
    UricAcidParameterBlock uParams;
    CholesterolParameterBlock cParams;
    TriglycerideParameterBlock tgParams;
    ReservedBlock reserved;
} ParameterData;

/* ����������^�K */
typedef struct {
    ParamBlockHeader header;
    ParameterData params;
} ParameterBlock;

/* �y��ӛ䛽Y�� */
typedef struct {
    uint16_t flag;              // ӛ���Ч��ӛ (0xA55A=��Ч)
    uint16_t resultStatus;      // �y���Y����B (0=�ɹ�������=�e�`)
    uint16_t resultValue;       // �y���Y��ֵ
    uint8_t testType;           // �y���Ŀ���
    uint8_t event;              // �¼����
    uint8_t stripCode;          // ԇƬУ���a
    uint8_t year;               // ��
    uint8_t month;              // ��
    uint8_t date;               // ��
    uint8_t hour;               // �r
    uint8_t minute;             // ��
    uint8_t second;             // ��
    uint16_t batteryVoltage;    // 늳�늉�
    uint16_t temperature;       // �h���ض�
} TestRecord;

/* ���ܺ������� */
bool PARAM_Init(void);
bool PARAM_LoadDefault(void);
bool PARAM_ReadBlock(uint8_t blockType, void *buffer, uint16_t size);
bool PARAM_UpdateBlock(uint8_t blockType, const void *buffer, uint16_t size);
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn);
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit);
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group);
bool PARAM_IncreaseTestCount(void);
bool PARAM_SaveTestRecord(uint16_t resultValue, uint16_t resultStatus, uint8_t event, uint8_t code, uint16_t batteryVoltage, uint16_t temperature);
uint16_t PARAM_GetTestCount(void);
bool PARAM_GetTestRecord(uint16_t index, TestRecord *record);
uint16_t PARAM_GetTestRecordCount(void);
bool PARAM_EraseAllTestRecords(void);

/* Flash�������P���� */
bool FLASH_WriteBuffer(uint32_t address, const uint8_t *buffer, uint32_t size);
uint16_t CalcChecksum(const uint8_t *data, uint32_t size);
uint16_t CalcCRC16(const uint8_t *data, uint32_t size);
void UpdateChecksum(ParameterBlock *block);

#define BLOCK_BASIC_SYSTEM     0
#define BLOCK_HARDWARE_CALIB   1
#define BLOCK_BG_PARAMS        2
#define BLOCK_U_PARAMS         3
#define BLOCK_C_PARAMS         4
#define BLOCK_TG_PARAMS        5

#endif /* __P14_FLASH_STORAGE_H__ */ 