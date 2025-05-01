/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch582f_init.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/07/23
 * Description        : CH582F��ʼ�������^�ļ�
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH582F_INIT_H__
#define __CH582F_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   CH582F GPIO��ʼ������
 */
void CH582F_GPIO_Init(void);

/**
 * @brief   CH582F UART��ʼ������
 */
void CH582F_UART_Init(void);

/**
 * @brief   CH582F������ʼ��
 */
void CH582F_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __CH582F_INIT_H__ 