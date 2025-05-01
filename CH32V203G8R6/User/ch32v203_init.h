/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v203_init.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH32V203G8R6��ʼ�������^�ļ�
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
 * @brief   CH32V203 ϵ�y�r犳�ʼ��
 */
void CH32V203_RCC_Init(void);

/**
 * @brief   CH32V203G8R6 GPIO��ʼ������
 */
void CH32V203_GPIO_Init(void);

/**
 * @brief   CH32V203G8R6 USART��ʼ������
 */
void CH32V203_USART_Init(void);

/**
 * @brief   CH32V203G8R6 ������ʼ��
 */
void CH32V203_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __CH32V203_INIT_H__ 