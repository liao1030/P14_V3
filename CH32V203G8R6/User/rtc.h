/********************************** (C) COPYRIGHT *******************************
 * File Name          : rtc.h
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/21
 * Description        : 多功能生化測試儀RTC功能
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#ifndef __RTC_H
#define __RTC_H

#include "ch32v20x.h"
#include "param_table.h"

/* 函數宣告 */
void RTC_Init(void);
void RTC_Config(void);
void RTC_SetTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
void RTC_GetTime(uint8_t* year, uint8_t* month, uint8_t* date, uint8_t* hour, uint8_t* minute, uint8_t* second);
void RTC_SyncWithParam(void);
void RTC_UpdateParam(void);

#endif /* __RTC_H */
