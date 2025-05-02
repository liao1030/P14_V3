/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_code.h
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x�������a���x
 *********************************************************************************
 * Copyright (c) 2025 HMD.
 *******************************************************************************/

#ifndef __PARAMETER_CODE_H
#define __PARAMETER_CODE_H

#include "ch32v20x.h"

/* ������Ͷ��x */
// 1. ϵ�y��������
#define PARAM_LBT          0   // ��늳��ֵ
#define PARAM_OBT          1   // 늳غıM�ֵ
#define PARAM_FACTORY      2   // �x������ģʽ
#define PARAM_MODEL_NO     3   // �aƷ��̖
#define PARAM_FW_NO        4   // �g�w�汾̖
#define PARAM_NOT          5   // �y���Δ� (2 bytes)
#define PARAM_CODE_TABLE_V 7   // ���a��汾̖ (2 bytes)

// 2. �r�g�O������
#define PARAM_YEAR         9   // ����O��
#define PARAM_MONTH        10  // �·��O��
#define PARAM_DATE         11  // �����O��
#define PARAM_HOUR         12  // С�r�O��
#define PARAM_MINUTE       13  // ����O��
#define PARAM_SECOND       14  // �딵�O��

// 3. �y���@ʾ����
#define PARAM_TLL          15  // �����ضȹ�������
#define PARAM_TLH          16  // �����ضȹ�������
#define PARAM_MGDL         17  // �y����λ�O��
#define PARAM_EVENT        18  // �{���¼����
#define PARAM_STRIP_TYPE   19  // �y���Ŀ

// 4. Ӳ�wУ�ʅ���
#define PARAM_EV_T3_TRG    20  // EV_T3�|�l�λ (2 bytes)
#define PARAM_EV_WORKING   22  // �y�������λ늉�
#define PARAM_EV_T3        23  // ѪҺ�����λ늉�
#define PARAM_DACO         24  // DACƫ���{��
#define PARAM_DACDO        25  // DACУ�ʹ��텢��
#define PARAM_CC211NoDone  26  // CC211δ������
#define PARAM_CAL_TOL      27  // OPS/OPIУ���ݲ�
#define PARAM_OPS          28  // OPAУ��б�� (4 bytes)
#define PARAM_OPI          32  // OPAУ�ʽؾ� (4 bytes)
#define PARAM_QCT          36  // QCTУ�ʜyԇ��λԪ�M
#define PARAM_TOFFSET      37  // �ض�У��ƫ��
#define PARAM_BOFFSET      38  // 늳�У��ƫ��

/* ԇƬ��Ͷ��x */
#define STRIP_TYPE_GLV     0   // Ѫ��
#define STRIP_TYPE_U       1   // ����
#define STRIP_TYPE_C       2   // ��đ�̴�
#define STRIP_TYPE_TG      3   // �������֬
#define STRIP_TYPE_GAV     4   // Ѫ��

/* ȫ�򅢔���Y�����x */
#define PARAM_TABLE_SIZE   678  // ��������С

/* �������� */
void Parameter_Init(void);
uint8_t Parameter_Read(uint16_t addr);
void Parameter_Write(uint16_t addr, uint8_t value);
uint16_t Parameter_Read16(uint16_t addr);
void Parameter_Write16(uint16_t addr, uint16_t value);
uint32_t Parameter_Read32(uint16_t addr);
void Parameter_Write32(uint16_t addr, uint32_t value);
void Parameter_Save(void);
void Parameter_Load(void);
uint8_t Parameter_CheckSum(void);
void Parameter_Reset(void);
void Parameter_SetDefaultValues(void);
const char* Parameter_GetStripTypeStr(uint8_t type);
void Parameter_PrintInfo(void);

#endif /* __PARAMETER_CODE_H */