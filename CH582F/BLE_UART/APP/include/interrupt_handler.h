/********************************** (C) COPYRIGHT *******************************
 * File Name          : interrupt_handler.h
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/09/25
 * Description        : CH582F GPIO中斷處理程序頭文件
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * GPIOA中斷處理函數
 */
extern void GPIOA_IRQHandler(void);

/*
 * GPIOB中斷處理函數
 */
extern void GPIOB_IRQHandler(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_HANDLER_H */