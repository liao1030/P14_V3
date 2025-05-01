/********************************** (C) COPYRIGHT *******************************
 * File Name          : parameter_test.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : 多功能生化測試儀參數代碼表測試函數頭文件
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#ifndef __PARAMETER_TEST_H__
#define __PARAMETER_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"
#include "parameter_table.h"

/* 測試函數宣告 */
void PARAM_Test_PrintMainParameters(void);
void PARAM_Test_SetStripLot(uint8_t stripType, const char* lotNumber);
void PARAM_Test_UpdateTestCount(void);
void PARAM_Test_SetTemperatureRange(uint8_t lower, uint8_t upper);
void PARAM_Test_SetConcentrationUnit(uint8_t unit);
void PARAM_Test_RunIntegrityTest(void);
void PARAM_Test_RunPerformanceTest(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAMETER_TEST_H__ */ 