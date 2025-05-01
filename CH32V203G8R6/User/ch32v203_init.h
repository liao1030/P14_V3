/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v203_init.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH32V203G8R6初始化配置頭文件
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH32V203_INIT_H__
#define __CH32V203_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v20x.h"

/**
 * @brief   CH32V203 系統時鐘初始化
 */
void CH32V203_RCC_Init(void);

/**
 * @brief   CH32V203G8R6 GPIO初始化配置
 */
void CH32V203_GPIO_Init(void);

/**
 * @brief   CH32V203G8R6 USART初始化配置
 */
void CH32V203_USART_Init(void);

/**
 * @brief   CH32V203G8R6 完整初始化
 */
void CH32V203_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __CH32V203_INIT_H__ 