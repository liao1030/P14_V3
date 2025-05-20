/********************************** (C) COPYRIGHT *******************************
 * File Name          : batt_measure.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2023/09/05
 * Description        : 늳�늉��y��ģ�M�^�ļ�
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef __BATT_MEASURE_H
#define __BATT_MEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "CH58x_common.h"

/*********************************************************************
 * CONSTANTS
 */
// 늳�늉��y���L�� (����)
#define BATT_MEASURE_PERIOD   30000  // ÿ30��y��һ��

// 늳�늉���B���x
#define BATT_STATUS_LOW       0x00   // 늳�늉���
#define BATT_STATUS_NORMAL    0x01   // 늳�늉�����
#define BATT_STATUS_CHARGING  0x02   // 늳����ڳ��

// 늳�늉��yֵ���x (mV)
#define BATT_VOLTAGE_LOW      2500   // ���2.5Vҕ�������
#define BATT_VOLTAGE_FULL     3100   // 3.1V����ҕ������M

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   ��ʼ��늳�늉��y��
 *
 * @param   none
 *
 * @return  none
 */
void Batt_MeasureInit(void);

/**
 * @brief   �y����ǰ늳�늉�
 *
 * @param   none
 *
 * @return  늳�늉� (mV)
 */
uint16_t Batt_GetVoltage(void);

/**
 * @brief   �@ȡ늳ؠ�B
 *
 * @param   none
 *
 * @return  늳ؠ�B (BATT_STATUS_LOW/BATT_STATUS_NORMAL/BATT_STATUS_CHARGING)
 */
uint8_t Batt_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __BATT_MEASURE_H */
