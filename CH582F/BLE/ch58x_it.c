/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch58x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F中斷處理程式
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* 全局變數 */
uint8_t uart1RxBuffer[256];
volatile uint8_t uart1RxCount = 0;

/******************************************************************************/
/* 中斷處理函數 */

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中斷處理函數 - 處理與CH32V203的通訊
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void UART1_IRQHandler(void)
{
    uint8_t rxData;
    
    /* 接收中斷 */
    if(UART1_GetITFlag(RB_IER_RECV_RDY))
    {
        /* 清除中斷標誌 */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
        
        /* 讀取接收數據 */
        rxData = UART1_RecvByte();
        
        /* 儲存接收數據到緩衝區 */
        if(uart1RxCount < sizeof(uart1RxBuffer))
        {
            uart1RxBuffer[uart1RxCount++] = rxData;
        }
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO中斷處理函數 - 處理試片插入檢測
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void GPIO_IRQHandler(void)
{
    /* 判斷是否為試片插入檢測引腳中斷 */
    if(GPIOB_ReadITFlagBit(GPIO_Pin_11))
    {
        /* 清除中斷標誌 */
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        
        /* 處理試片插入檢測中斷 */
        /* TODO: 在此添加試片插入處理邏輯 */
    }
    
    if(GPIOA_ReadITFlagBit(GPIO_Pin_15))
    {
        /* 清除中斷標誌 */
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        
        /* 處理試片插入檢測中斷 */
        /* TODO: 在此添加試片插入處理邏輯 */
    }
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   定時器0中斷處理函數
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void TMR0_IRQHandler(void)
{
    /* 處理定時器0中斷 */
    /* TODO: 在此添加定時器中斷處理邏輯 */
    
    /* 清除中斷標誌 */
    TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
} 