/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_table.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : �๦�������yԇ�x�������a���^�ļ�
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAMETER_TABLE_H__
#define __PARAMETER_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/* ��������С */
#define PARAM_TABLE_SIZE           678  // ��������С (0-677)

/* �yԇ�Ŀ��Ͷ��x */
#define STRIP_TYPE_GLV             0    // Ѫ��
#define STRIP_TYPE_U               1    // ����
#define STRIP_TYPE_C               2    // ��đ�̴�
#define STRIP_TYPE_TG              3    // �������֬
#define STRIP_TYPE_GAV             4    // Ѫ��(��һ�N)

/* ϵ�y����������ַ */
#define PARAM_LBT                  0    // ��늳��ֵ
#define PARAM_OBT                  1    // 늳غıM�ֵ
#define PARAM_FACTORY              2    // �x������ģʽ
#define PARAM_MODEL_NO             3    // �aƷ��̖
#define PARAM_FW_NO                4    // �g�w�汾̖
#define PARAM_NOT                  5    // �yԇ�Δ� (2 bytes)
#define PARAM_CODE_TABLE_V         7    // ���a��汾̖ (2 bytes)

/* �r�g�O��������ַ */
#define PARAM_YEAR                 9    // ����O��
#define PARAM_MONTH                10   // �·��O��
#define PARAM_DATE                 11   // �����O��
#define PARAM_HOUR                 12   // С�r�O��
#define PARAM_MINUTE               13   // ����O��
#define PARAM_SECOND               14   // �딵�O��

/* �yԇ�h��������ַ */
#define PARAM_TLL                  15   // �����ضȹ�������
#define PARAM_TLH                  16   // �����ضȹ�������
#define PARAM_MGDL                 17   // ��Ȇ�λ�O��
#define PARAM_EVENT                18   // ����¼����
#define PARAM_STRIP_TYPE           19   // �yԇ�Ŀ

/* Ӳ�wУ��������ַ */
#define PARAM_EV_T3_TRG            20   // EV_T3�|�l늉� (2 bytes)
#define PARAM_EV_WORKING           22   // ��ȹ������늉�
#define PARAM_EV_T3                23   // ѪҺ�������늉�
#define PARAM_DACO                 24   // DACƫ���a��
#define PARAM_DACDO                25   // DACУ�����텢��
#define PARAM_CC211NoDone          26   // CC211δ������
#define PARAM_CAL_TOL              27   // OPS/OPIУ���ݲ�
#define PARAM_OPS                  28   // OPAУ��б�� (4 bytes)
#define PARAM_OPI                  32   // OPAУ���ؾ� (4 bytes)
#define PARAM_QCT                  36   // QCTУ���yԇ��λԪ�M
#define PARAM_TOFFSET              37   // �ض�У��ƫ��
#define PARAM_BOFFSET              38   // 늳�У��ƫ��

/* Ѫ��(GLV/GAV)���Å�����ַ */
#define PARAM_BG_CSU_TOL           39   // ԇƬ�z���ݲ�
#define PARAM_BG_NDL               40   // ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_BG_UDL               42   // ��ʹ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_BG_BLOOD_IN          44   // ѪҺ���ˮƽ (2 bytes)
#define PARAM_BG_STRIP_LOT         46   // ԇƬ��̖ (16 bytes)
#define PARAM_BG_L                 62   // Ѫ��ֵ����
#define PARAM_BG_H                 63   // Ѫ��ֵ����
#define PARAM_BG_T3_E37            64   // T3 ADV��E37�ֵ (2 bytes)

/* ����(U)���Å�����ַ */
#define PARAM_U_CSU_TOL            213  // ԇƬ�z���ݲ�
#define PARAM_U_NDL                214  // ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_U_UDL                216  // ��ʹ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_U_BLOOD_IN           218  // ѪҺ���ˮƽ (2 bytes)
#define PARAM_U_STRIP_LOT          220  // ԇƬ��̖ (16 bytes)
#define PARAM_U_L                  236  // ����ֵ����
#define PARAM_U_H                  237  // ����ֵ����
#define PARAM_U_T3_E37             238  // T3 ADV��E37�ֵ (2 bytes)

/* ��đ�̴�(C)���Å�����ַ */
#define PARAM_C_CSU_TOL            355  // ԇƬ�z���ݲ�
#define PARAM_C_NDL                356  // ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_C_UDL                358  // ��ʹ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_C_BLOOD_IN           360  // ѪҺ���ˮƽ (2 bytes)
#define PARAM_C_STRIP_LOT          362  // ԇƬ��̖ (16 bytes)
#define PARAM_C_L                  378  // đ�̴�ֵ����
#define PARAM_C_H                  379  // đ�̴�ֵ����
#define PARAM_C_T3_E37             380  // T3 ADV��E37�ֵ (2 bytes)

/* �������֬(TG)���Å�����ַ */
#define PARAM_TG_CSU_TOL           497  // ԇƬ�z���ݲ�
#define PARAM_TG_NDL               498  // ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_TG_UDL               500  // ��ʹ��ԇƬ���ˮƽ (2 bytes)
#define PARAM_TG_BLOOD_IN          502  // ѪҺ���ˮƽ (2 bytes)
#define PARAM_TG_STRIP_LOT         504  // ԇƬ��̖ (16 bytes)
#define PARAM_TG_L                 520  // �������ֵ֬����
#define PARAM_TG_H                 521  // �������ֵ֬����
#define PARAM_TG_T3_E37            522  // T3 ADV��E37�ֵ (2 bytes)

/* У򞅢����ַ */
#define PARAM_SUM_L                675  // ��ַ0~674У��(��)
#define PARAM_SUM_H                676  // ��ַ0~674У��(��) 
#define PARAM_CRC16                677  // ����������CRCУ�

/* ������Y�����x */
typedef struct {
    uint8_t data[PARAM_TABLE_SIZE];
} ParameterTable_TypeDef;

/* �������� */
void PARAM_Init(void);
uint8_t PARAM_Read(uint16_t addr);
void PARAM_Write(uint16_t addr, uint8_t value);
uint16_t PARAM_Read16(uint16_t addr);
void PARAM_Write16(uint16_t addr, uint16_t value);
void PARAM_ReadBlock(uint16_t addr, uint8_t *buffer, uint16_t size);
void PARAM_WriteBlock(uint16_t addr, const uint8_t *buffer, uint16_t size);
uint8_t PARAM_VerifyChecksum(void);
void PARAM_UpdateChecksum(void);
void PARAM_Reset(void);
void PARAM_Save(void);
uint8_t PARAM_Load(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAMETER_TABLE_H__ */ 