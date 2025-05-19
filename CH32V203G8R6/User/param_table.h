/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_table.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x�������^�ļ�
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_TABLE_H
#define __PARAM_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include <stdint.h>

/* Flash�����^���x */
#define PARAM_AREA_A_ADDR          ((uint32_t)0x0800D000)   // ��������Ҫ����^���ַ
#define PARAM_AREA_B_ADDR          ((uint32_t)0x0800E000)   // �������݅^���ַ
#define TEST_RECORD_AREA_ADDR      ((uint32_t)0x0800F000)   // �yԇӛ䛅^���ַ

#define PARAM_AREA_SIZE            ((uint32_t)0x1000)       // �����^��С (4KB)
#define FLASH_PAGE_SIZE            ((uint32_t)0x400)        // Flash퓴�С (1KB)

/* ħ�g���ֺͰ汾���x */
#define PARAM_MAGIC                ((uint16_t)0xA55A)       // �����^ħ�g����
#define PARAM_VERSION              ((uint16_t)0x0001)       // ������汾̖

/* �������e�`�a */
typedef enum {
    PARAM_OK = 0,                  // �����ɹ�
    PARAM_ERR_INVALID_AREA,        // �oЧ�ą����^
    PARAM_ERR_FLASH_WRITE,         // Flash�����e�`
    PARAM_ERR_CRC,                 // CRCУ��e�`
    PARAM_ERR_INVALID_PARAM,       // �oЧ����
    PARAM_ERR_NOT_INITIALIZED      // ����δ��ʼ��
} ParamError_TypeDef;

/* �����^����R */
typedef enum {
    PARAM_AREA_MAIN = 0,           // �������^
    PARAM_AREA_BACKUP              // ��݅����^
} ParamArea_TypeDef;

/* �yԇ�Ŀ��� */
typedef enum {
    STRIP_TYPE_GLV = 0,            // Ѫ�� (GLV)
    STRIP_TYPE_U = 1,              // ���� (U)
    STRIP_TYPE_C = 2,              // ��đ�̴� (C)
    STRIP_TYPE_TG = 3,             // �������֬ (TG) 
    STRIP_TYPE_GAV = 4,            // Ѫ�� (GAV)
    STRIP_TYPE_MAX                 // ���ԇƬ��͔����oЧ��ͣ�
} StripType_TypeDef;

#pragma pack(1)  // �_���Y���w�o�ܷ��b

/* �������^���Y�� */
typedef struct {
    uint16_t magic;                // ħ�g���� (0xA55A)����ʾ��Ч�ą����^
    uint16_t version;              // ������汾̖������ Code_Table_V
    uint32_t writeCounter;         // ����Δ�Ӌ����
    uint32_t timestamp;            // ������r�g��ӛ
} ParameterHeader;

/* ����ϵ�y�����^�K */
typedef struct {
    // ϵ�y�������� (9������)
    uint8_t lbt;                   // ��늳��ֵ
    uint8_t obt;                   // 늳غıM�ֵ
    uint8_t factory;               // �x������ģʽ
    uint8_t modelNo;               // �aƷ��̖
    uint8_t fwNo;                  // �g�w�汾̖
    uint16_t testCount;            // �yԇ�Δ�
    uint16_t codeTableVer;         // ���a��汾
    
    // �r�g�O������ (6������)
    uint8_t year;                  // ����O�� (0-99 for 2000-2099)
    uint8_t month;                 // �·��O�� (1-12)
    uint8_t date;                  // �����O�� (1-31)
    uint8_t hour;                  // С�r�O�� (0-23)
    uint8_t minute;                // ����O�� (0-59)
    uint8_t second;                // �딵�O�� (0-59)
    
    // �yԇ�h������ (5������)
    uint8_t tempLowLimit;          // �����ض�����
    uint8_t tempHighLimit;         // �����ض�����
    uint8_t measureUnit;           // �y����λ�O��
    uint8_t defaultEvent;          // �A�O�¼�
    uint8_t stripType;             // �yԇ�Ŀ���
} BasicSystemBlock;

/* Ӳ�wУ�ʅ����^�K */
typedef struct {
    uint16_t evT3Trigger;          // EV_T3�|�l늉�
    uint8_t evWorking;             // �y������늘O늉�
    uint8_t evT3;                  // ѪҺ�z�y늘O늉�
    uint8_t calTolerance;          // OPS/OPIУ���ݲ�
    float ops;                     // OPAУ��б��
    float opi;                     // OPAУ�ʽؾ�
    uint8_t qct;                   // QCTУ�ʜyԇ��λԪ�M
    int8_t tempOffset;             // �ض�У��ƫ��
    int8_t batteryOffset;          // 늳�У��ƫ��
} HardwareCalibBlock;

/* Ѫ��(GLV)���Å����^�K */
typedef struct {
    // ԇƬ���� (12������)
    uint8_t glvCsuTolerance;       // ԇƬ�z���ݲ�
    uint16_t glvNdl;               // ��ԇƬ�z�yˮƽ
    uint16_t glvUdl;               // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t glvBloodIn;           // ѪҺ�z�yˮƽ
    uint16_t glvEvWBackground;     // W늘O����ֵ
    uint16_t glvEvWPwmDuty;        // W늘OPWMռ�ձ�
    uint8_t glvCountDownTime;      // �����r�g
    
    // �y���������� (2������)
    uint8_t glvL;                  // Ѫ��ֵ����
    uint8_t glvH;                  // Ѫ��ֵ����
    
    // �yԇ�r�򅢔� (12������)
    uint16_t glvTPL1;              // �r�g�}�n�� (��һ�M)
    uint16_t glvTRD1;              // ԭʼ�����r�g (��һ�M)
    uint16_t glvEVWidth1;          // ȼ���r�g (��һ�M)
    uint16_t glvTPL2;              // �r�g�}�n�� (�ڶ��M)
    uint16_t glvTRD2;              // ԭʼ�����r�g (�ڶ��M)
    uint16_t glvEVWidth2;          // ȼ���r�g (�ڶ��M)
    
    // Ӌ��ʽ�a������ (110������)
    uint8_t glvS2;                 // ԇƬ�a������
    uint8_t glvI2;                 // ԇƬ�a������
    uint8_t glvSq;                 // Ʒ��ҺQC�a������Sq
    uint8_t glvIq;                 // Ʒ��ҺQC�a������Iq
    float glvSr;                   // OPAӲ�wޒ·�a��(б��)
    float glvIr;                   // OPAӲ�wޒ·�a��(�ؾ�)
    uint16_t glvGLU[7];            // ���У������(���y�ֶ��c)
    uint16_t glvGOAL[7];           // ���У������(Ŀ��ֵ�ֶ��c)
    uint8_t glvS3[30];             // ԇƬ�a������S3(1-30)
    uint8_t glvI3[30];             // ԇƬ�a������I3(1-30)
    
    // �ض��a������ (28������)
    uint8_t glvTF[7];              // AC/PC�ض��a��б��(10-40��C)
    uint8_t glvCTF[7];             // QC�ض��a��б��(10-40��C)
    uint8_t glvTO[7];              // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t glvCTO[7];             // QC�ض��a���ؾ�(10-40��C)
    
    // Ӌ���პ���a������ (9������)
    uint8_t glvCVq;                // CV Level of QC
    uint8_t glvAq;                 // A of QC Compensation
    uint8_t glvBq;                 // B of QC Compensation
    uint8_t glvtCV;                // Time of BG Optimize
    uint8_t glvCVbg;               // CV Level of BG
    uint8_t glvAbg;                // A of BG Compensation
    uint8_t glvBbg;                // B of BG Compensation
    uint8_t glvAq5;                // A of QC Compensation Level 5
    uint8_t glvBq5;                // B of QC Compensation Level 5
} GLVParameterBlock;

/* ����(U)���Å����^�K */
typedef struct {
    // ԇƬ���� (12������)
    uint8_t uCsuTolerance;         // ԇƬ�z���ݲ�
    uint16_t uNdl;                 // ��ԇƬ�z�yˮƽ
    uint16_t uUdl;                 // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t uBloodIn;             // ѪҺ�z�yˮƽ
    uint16_t uEvWBackground;       // W늘O����ֵ
    uint16_t uEvWPwmDuty;          // W늘OPWMռ�ձ�
    uint8_t uCountDownTime;        // �����r�g
    
    // �y���������� (2������)
    uint8_t uL;                    // ����ֵ����
    uint8_t uH;                    // ����ֵ����
    
    // �yԇ�r�򅢔� (12������)
    uint16_t uTPL1;                // �r�g�}�n�� (��һ�M)
    uint16_t uTRD1;                // ԭʼ�����r�g (��һ�M)
    uint16_t uEVWidth1;            // ȼ���r�g (��һ�M)
    uint16_t uTPL2;                // �r�g�}�n�� (�ڶ��M)
    uint16_t uTRD2;                // ԭʼ�����r�g (�ڶ��M)
    uint16_t uEVWidth2;            // ȼ���r�g (�ڶ��M)
    
    // Ӌ��ʽ�a������ (80������)
    uint8_t uS2;                   // ԇƬ�a������
    uint8_t uI2;                   // ԇƬ�a������
    uint8_t uSq;                   // Ʒ��ҺQC�a������Sq
    uint8_t uIq;                   // Ʒ��ҺQC�a������Iq
    float uSr;                     // OPAӲ�wޒ·�a��(б��)
    float uIr;                     // OPAӲ�wޒ·�a��(�ؾ�)
    uint16_t uGLU[7];              // ���У������(���y�ֶ��c)
    uint16_t uGOAL[7];             // ���У������(Ŀ��ֵ�ֶ��c)
    uint8_t uS3[15];               // ԇƬ�a������S3(1-15)
    uint8_t uI3[15];               // ԇƬ�a������I3(1-15)
    
    // �ض��a������ (28������)
    uint8_t uTF[7];                // AC/PC�ض��a��б��(10-40��C)
    uint8_t uCTF[7];               // QC�ض��a��б��(10-40��C)
    uint8_t uTO[7];                // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t uCTO[7];               // QC�ض��a���ؾ�(10-40��C)
    
    // Ӌ���პ���a������ (9������)
    uint8_t uCVq;                  // CV Level of QC
    uint8_t uAq;                   // A of QC Compensation
    uint8_t uBq;                   // B of QC Compensation
    uint8_t utCV;                  // Time of BG Optimize
    uint8_t uCVbg;                 // CV Level of BG
    uint8_t uAbg;                  // A of BG Compensation
    uint8_t uBbg;                  // B of BG Compensation
    uint8_t uAq5;                  // A of QC Compensation Level 5
    uint8_t uBq5;                  // B of QC Compensation Level 5
} UParameterBlock;

/* ��đ�̴�(C)���Å����^�K */
typedef struct {
    // ԇƬ���� (12������)
    uint8_t cCsuTolerance;         // ԇƬ�z���ݲ�
    uint16_t cNdl;                 // ��ԇƬ�z�yˮƽ
    uint16_t cUdl;                 // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t cBloodIn;             // ѪҺ�z�yˮƽ
    uint16_t cEvWBackground;       // W늘O����ֵ
    uint16_t cEvWPwmDuty;          // W늘OPWMռ�ձ�
    uint8_t cCountDownTime;        // �����r�g
    
    // �y���������� (2������)
    uint8_t cL;                    // đ�̴�ֵ����
    uint8_t cH;                    // đ�̴�ֵ����
    
    // �yԇ�r�򅢔� (12������)
    uint16_t cTPL1;                // �r�g�}�n�� (��һ�M)
    uint16_t cTRD1;                // ԭʼ�����r�g (��һ�M)
    uint16_t cEVWidth1;            // ȼ���r�g (��һ�M)
    uint16_t cTPL2;                // �r�g�}�n�� (�ڶ��M)
    uint16_t cTRD2;                // ԭʼ�����r�g (�ڶ��M)
    uint16_t cEVWidth2;            // ȼ���r�g (�ڶ��M)
    
    // Ӌ��ʽ�a������ (80������)
    uint8_t cS2;                   // ԇƬ�a������
    uint8_t cI2;                   // ԇƬ�a������
    uint8_t cSq;                   // Ʒ��ҺQC�a������Sq
    uint8_t cIq;                   // Ʒ��ҺQC�a������Iq
    float cSr;                     // OPAӲ�wޒ·�a��(б��)
    float cIr;                     // OPAӲ�wޒ·�a��(�ؾ�)
    uint16_t cGLU[7];              // ���У������(���y�ֶ��c)
    uint16_t cGOAL[7];             // ���У������(Ŀ��ֵ�ֶ��c)
    uint8_t cS3[15];               // ԇƬ�a������S3(1-15)
    uint8_t cI3[15];               // ԇƬ�a������I3(1-15)
    
    // �ض��a������ (28������)
    uint8_t cTF[7];                // AC/PC�ض��a��б��(10-40��C)
    uint8_t cCTF[7];               // QC�ض��a��б��(10-40��C)
    uint8_t cTO[7];                // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t cCTO[7];               // QC�ض��a���ؾ�(10-40��C)
    
    // Ӌ���პ���a������ (9������)
    uint8_t cCVq;                  // CV Level of QC
    uint8_t cAq;                   // A of QC Compensation
    uint8_t cBq;                   // B of QC Compensation
    uint8_t ctCV;                  // Time of BG Optimize
    uint8_t cCVbg;                 // CV Level of BG
    uint8_t cAbg;                  // A of BG Compensation
    uint8_t cBbg;                  // B of BG Compensation
    uint8_t cAq5;                  // A of QC Compensation Level 5
    uint8_t cBq5;                  // B of QC Compensation Level 5
} CParameterBlock;

/* �������֬(TG)���Å����^�K */
typedef struct {
    // ԇƬ���� (12������)
    uint8_t tgCsuTolerance;        // ԇƬ�z���ݲ�
    uint16_t tgNdl;                // ��ԇƬ�z�yˮƽ
    uint16_t tgUdl;                // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t tgBloodIn;            // ѪҺ�z�yˮƽ
    uint16_t tgEvWBackground;      // W늘O����ֵ
    uint16_t tgEvWPwmDuty;         // W늘OPWMռ�ձ�
    uint8_t tgCountDownTime;       // �����r�g
    
    // �y���������� (2������)
    uint8_t tgL;                   // �������ֵ֬����
    uint16_t tgH;                  // �������ֵ֬����
    
    // �yԇ�r�򅢔� (12������)
    uint16_t tgTPL1;               // �r�g�}�n�� (��һ�M)
    uint16_t tgTRD1;               // ԭʼ�����r�g (��һ�M)
    uint16_t tgEVWidth1;           // ȼ���r�g (��һ�M)
    uint16_t tgTPL2;               // �r�g�}�n�� (�ڶ��M)
    uint16_t tgTRD2;               // ԭʼ�����r�g (�ڶ��M)
    uint16_t tgEVWidth2;           // ȼ���r�g (�ڶ��M)
    
    // Ӌ��ʽ�a������ (80������)
    uint8_t tgS2;                  // ԇƬ�a������
    uint8_t tgI2;                  // ԇƬ�a������
    uint8_t tgSq;                  // Ʒ��ҺQC�a������Sq
    uint8_t tgIq;                  // Ʒ��ҺQC�a������Iq
    float tgSr;                    // OPAӲ�wޒ·�a��(б��)
    float tgIr;                    // OPAӲ�wޒ·�a��(�ؾ�)
    uint16_t tgGLU[7];             // ���У������(���y�ֶ��c)
    uint16_t tgGOAL[7];            // ���У������(Ŀ��ֵ�ֶ��c)
    uint8_t tgS3[15];              // ԇƬ�a������S3(1-15)
    uint8_t tgI3[15];              // ԇƬ�a������I3(1-15)
    
    // �ض��a������ (28������)
    uint8_t tgTF[7];               // AC/PC�ض��a��б��(10-40��C)
    uint8_t tgCTF[7];              // QC�ض��a��б��(10-40��C)
    uint8_t tgTO[7];               // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t tgCTO[7];              // QC�ض��a���ؾ�(10-40��C)
    
    // Ӌ���პ���a������ (9������)
    uint8_t tgCVq;                 // CV Level of QC
    uint8_t tgAq;                  // A of QC Compensation
    uint8_t tgBq;                  // B of QC Compensation
    uint8_t tgtCV;                 // Time of BG Optimize
    uint8_t tgCVbg;                // CV Level of BG
    uint8_t tgAbg;                 // A of BG Compensation
    uint8_t tgBbg;                 // B of BG Compensation
    uint8_t tgAq5;                 // A of QC Compensation Level 5
    uint8_t tgBq5;                 // B of QC Compensation Level 5
} TGParameterBlock;

/* Ѫ��(GAV)���Å����^�K */
typedef struct {
    // ԇƬ���� (12������)
    uint8_t gavCsuTolerance;       // ԇƬ�z���ݲ�
    uint16_t gavNdl;               // ��ԇƬ�z�yˮƽ
    uint16_t gavUdl;               // ��ʹ��ԇƬ�z�yˮƽ
    uint16_t gavBloodIn;           // ѪҺ�z�yˮƽ
    uint16_t gavEvWBackground;     // W늘O����ֵ
    uint16_t gavEvWPwmDuty;        // W늘OPWMռ�ձ�
    uint8_t gavCountDownTime;      // �����r�g
    
    // �y���������� (3������)
    uint8_t gavL;                  // Ѫ��ֵ����
    uint16_t gavH;                 // Ѫ��ֵ����
    uint16_t gavT3E37;             // T3 ADV�e�`37�ֵ
    
    // �yԇ�r�򅢔� (12������)
    uint16_t gavTPL1;              // �r�g�}�n�� (��һ�M)
    uint16_t gavTRD1;              // ԭʼ�����r�g (��һ�M)
    uint16_t gavEVWidth1;          // ȼ���r�g (��һ�M)
    uint16_t gavTPL2;              // �r�g�}�n�� (�ڶ��M)
    uint16_t gavTRD2;              // ԭʼ�����r�g (�ڶ��M)
    uint16_t gavEVWidth2;          // ȼ���r�g (�ڶ��M)
    
    // Ӌ��ʽ�a������ (110������)
    uint8_t gavS2;                 // ԇƬ�a������
    uint8_t gavI2;                 // ԇƬ�a������
    uint8_t gavSq;                 // Ʒ��ҺQC�a������Sq
    uint8_t gavIq;                 // Ʒ��ҺQC�a������Iq
    float gavSr;                   // OPAӲ�wޒ·�a��(б��)
    float gavIr;                   // OPAӲ�wޒ·�a��(�ؾ�)
    uint16_t gavGLU[7];            // ���У������(���y�ֶ��c)
    uint16_t gavGOAL[7];           // ���У������(Ŀ��ֵ�ֶ��c)
    uint8_t gavS3[30];             // ԇƬ�a������S3(1-30)
    uint8_t gavI3[30];             // ԇƬ�a������I3(1-30)
    
    // AC�a������ (28������)
    uint16_t gavAcLWaveL;          // AC����Д�(��)
    uint16_t gavAcHWaveL;          // AC����Д�(��)
    uint8_t gavS4[7];              // AC�a������S4(HCT10-HCT70)
    uint8_t gavI4[7];              // AC�a������I4(HCT10-HCT70)
    uint16_t gavAcAdc[7];          // AC�a���ж�ֵ(HCT10-HCT70)
    
    // �ض��a������ (42������)
    uint8_t gavTF[7];              // AC/PC�ض��a��б��(10-40��C)
    uint8_t gavCTF[7];             // QC�ض��a��б��(10-40��C)
    uint8_t gavTO[7];              // AC/PC�ض��a���ؾ�(10-40��C)
    uint8_t gavCTO[7];             // QC�ض��a���ؾ�(10-40��C)
    uint8_t gavAcTF[7];            // AC(����)�ض��a��б��(10-40��C)
    uint8_t gavAcTO[7];            // AC(����)�ض��a���ؾ�(10-40��C)
    
    // Ӌ���პ���a������ (9������)
    uint8_t gavCVq;                // CV Level of QC
    uint8_t gavAq;                 // A of QC Compensation
    uint8_t gavBq;                 // B of QC Compensation
    uint8_t gavtCV;                // Time of BG Optimize
    uint8_t gavCVbg;               // CV Level of BG
    uint8_t gavAbg;                // A of BG Compensation
    uint8_t gavBbg;                // B of BG Compensation
    uint8_t gavAq5;                // A of QC Compensation Level 5
    uint8_t gavBq5;                // B of QC Compensation Level 5
} GAVParameterBlock;

/* ����������Y�� */
typedef struct {
    ParameterHeader header;         // �������^��
    BasicSystemBlock basic;         // ����ϵ�y����
    HardwareCalibBlock hardware;    // Ӳ�wУ�ʅ���
    GLVParameterBlock glv;          // Ѫ��(GLV)����
    UParameterBlock u;              // ����(U)����
    CParameterBlock c;              // đ�̴�(C)����
    TGParameterBlock tg;            // �������֬(TG)����
    GAVParameterBlock gav;          // Ѫ��(GAV)����
    uint8_t reserved[36];           // ����^��δ��Uչ
    uint16_t checksum;              // У�� (0~844λַ�Ŀ���)
    uint8_t crc16;                  // CRC16У�ֵ
} ParameterTable;

#pragma pack()  // �֏�Ĭ�J���R

/* ȫ�օ�����ָ� */
extern ParameterTable *g_pParamTable;

/* ��������ԭ�� */
ParamError_TypeDef PARAM_Init(void);
ParamError_TypeDef PARAM_LoadFromFlash(ParamArea_TypeDef area);
ParamError_TypeDef PARAM_SaveToFlash(ParamArea_TypeDef area);
ParamError_TypeDef PARAM_LoadDefaults(void);
ParamError_TypeDef PARAM_GetValue(uint16_t paramAddress, void *pValue, uint8_t size);
ParamError_TypeDef PARAM_SetValue(uint16_t paramAddress, void *pValue, uint8_t size);
ParamError_TypeDef PARAM_Backup(void);
ParamError_TypeDef PARAM_Restore(void);
uint8_t PARAM_IsInitialized(void);
uint16_t PARAM_CalculateChecksum(void);
uint8_t PARAM_CalculateCRC16(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TABLE_H */
