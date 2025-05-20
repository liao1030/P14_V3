/********************************** (C) COPYRIGHT *******************************
 * File Name          : batt_measure.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2023/09/05
 * Description        : 電池電壓測量模組頭文件
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
// 電池電壓測量週期 (毫秒)
#define BATT_MEASURE_PERIOD   30000  // 每30秒測量一次

// 電池電壓狀態定義
#define BATT_STATUS_LOW       0x00   // 電池電壓低
#define BATT_STATUS_NORMAL    0x01   // 電池電壓正常
#define BATT_STATUS_CHARGING  0x02   // 電池正在充電

// 電池電壓閥值定義 (mV)
#define BATT_VOLTAGE_LOW      2500   // 低於2.5V視為電量低
#define BATT_VOLTAGE_FULL     3100   // 3.1V以上視為電量滿

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   初始化電池電壓測量
 *
 * @param   none
 *
 * @return  none
 */
void Batt_MeasureInit(void);

/**
 * @brief   測量當前電池電壓
 *
 * @param   none
 *
 * @return  電池電壓 (mV)
 */
uint16_t Batt_GetVoltage(void);

/**
 * @brief   獲取電池狀態
 *
 * @param   none
 *
 * @return  電池狀態 (BATT_STATUS_LOW/BATT_STATUS_NORMAL/BATT_STATUS_CHARGING)
 */
uint8_t Batt_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __BATT_MEASURE_H */
