/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/08/01
 * Description        : CH32V203 中斷處理程序
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "ch32v20x_it.h"
#include "debug.h"
#include "P14_BLE_Protocol.h" // 添加以支持試片偵測協議

/* 全局變量聲明 */
extern volatile uint32_t g_systicks;

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/* 在main.c中實現的SysTick處理函數 */
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
    while(1)
    {
    }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
    while(1)
    {
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   處理USART2(與CH582F通訊)中斷
 *          包含試片偵測命令的處理
 *
 * @return  none
 */
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* 接收數據 */
        uint8_t rx_data = USART_ReceiveData(USART2);
        
        /* 處理接收到的數據 */
        BLE_ProtocolRxHandler(rx_data);
        
        /* 清除中斷標誌 */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/*********************************************************************
 * @fn      SysTick_Handler
 *
 * @brief   系統滴答計時器中斷處理函數
 *
 * @return  none
 */
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    /* 更新系統時間計數 */
    g_systicks++;
    
    /* 清除中斷標誌 */
    SysTick->SR = 0;
}


