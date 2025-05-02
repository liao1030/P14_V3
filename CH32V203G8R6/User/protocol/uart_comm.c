/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_comm.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀 UART 通訊驅動實現
 ********************************************************************************/

#include "uart_comm.h"
#include "ch32v20x_usart.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_gpio.h"
#include "debug.h"
#include <string.h>

/* 接收緩衝區 */
static uint8_t UART_RxBuffer[UART_BUFFER_SIZE];
static uint16_t UART_RxBufferHead = 0;
static uint16_t UART_RxBufferTail = 0;
static volatile uint8_t UART_RxFlag = 0;

/* 發送緩衝區 */
static uint8_t UART_TxBuffer[UART_BUFFER_SIZE];
static uint16_t UART_TxCount = 0;
static uint16_t UART_TxIndex = 0;
static volatile uint8_t UART_TxBusy = 0;

/* 接收回調函數 */
static UART_RxCallback_t UART_RxCallback = NULL;

/*********************************************************************
 * @fn      UART_Comm_Init
 *
 * @brief   初始化UART2通訊
 *
 * @return  none
 */
void UART_Comm_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 啟用USART2和GPIOA時鐘 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* 配置USART2 Tx (PA2) 為推挽輸出模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 配置USART2 Rx (PA3) 為浮動輸入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 配置USART2參數 */
    USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    /* 配置USART2中斷 */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 啟用USART2接收中斷 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* 啟用USART2 */
    USART_Cmd(USART2, ENABLE);
    
    /* 初始化緩衝區 */
    UART_RxBufferHead = 0;
    UART_RxBufferTail = 0;
    UART_RxFlag = 0;
    UART_TxCount = 0;
    UART_TxIndex = 0;
    UART_TxBusy = 0;
}

/*********************************************************************
 * @fn      UART_SendData
 *
 * @brief   通過UART發送數據
 *
 * @param   data - 待發送數據指針
 *          length - 待發送數據長度
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t UART_SendData(uint8_t *data, uint16_t length)
{
    uint16_t i;
    
    if (length > UART_BUFFER_SIZE)
    {
        return 1;  // 數據太長，超出緩衝區
    }
    
    /* 等待之前的發送完成 */
    while (UART_TxBusy)
    {
        ;
    }
    
    /* 拷貝數據到發送緩衝區 */
    for (i = 0; i < length; i++)
    {
        UART_TxBuffer[i] = data[i];
    }
    
    UART_TxCount = length;
    UART_TxIndex = 0;
    UART_TxBusy = 1;
    
    /* 啟動發送 */
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    
    return 0;
}

/*********************************************************************
 * @fn      UART_ProcessRxData
 *
 * @brief   處理接收數據
 *
 * @return  none
 */
void UART_ProcessRxData(void)
{
    uint16_t length;
    uint8_t data[UART_BUFFER_SIZE];
    
    if (UART_RxFlag)
    {
        /* 關閉中斷避免處理期間有新數據進來 */
        USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
        
        if (UART_RxBufferHead >= UART_RxBufferTail)
        {
            length = UART_RxBufferHead - UART_RxBufferTail;
            memcpy(data, &UART_RxBuffer[UART_RxBufferTail], length);
        }
        else
        {
            length = UART_BUFFER_SIZE - UART_RxBufferTail;
            memcpy(data, &UART_RxBuffer[UART_RxBufferTail], length);
            memcpy(&data[length], &UART_RxBuffer[0], UART_RxBufferHead);
            length += UART_RxBufferHead;
        }
        
        UART_RxBufferTail = UART_RxBufferHead;
        UART_RxFlag = 0;
        
        /* 重新啟用中斷 */
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        
        /* 呼叫回調函數處理數據 */
        if (UART_RxCallback != NULL && length > 0)
        {
            UART_RxCallback(data, length);
        }
    }
}

/*********************************************************************
 * @fn      UART_SetRxCallback
 *
 * @brief   設置接收回調函數
 *
 * @param   callback - 回調函數指針
 *
 * @return  none
 */
void UART_SetRxCallback(UART_RxCallback_t callback)
{
    UART_RxCallback = callback;
}

/*********************************************************************
 * @fn      UART_EnableRxInt
 *
 * @brief   啟用/禁用接收中斷
 *
 * @param   state - ENABLE/DISABLE
 *
 * @return  none
 */
void UART_EnableRxInt(FunctionalState state)
{
    USART_ITConfig(USART2, USART_IT_RXNE, state);
}

/*********************************************************************
 * @fn      UART_EnableTxInt
 *
 * @brief   啟用/禁用發送中斷
 *
 * @param   state - ENABLE/DISABLE
 *
 * @return  none
 */
void UART_EnableTxInt(FunctionalState state)
{
    USART_ITConfig(USART2, USART_IT_TXE, state);
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   USART2中斷處理函數
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        /* 讀取接收到的數據 */
        uint8_t data = USART_ReceiveData(USART2);
        
        /* 存入接收緩衝區 */
        UART_RxBuffer[UART_RxBufferHead] = data;
        UART_RxBufferHead = (UART_RxBufferHead + 1) % UART_BUFFER_SIZE;
        
        /* 標記接收到數據 */
        UART_RxFlag = 1;
        
        /* 清除中斷標誌 */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if (UART_TxIndex < UART_TxCount)
        {
            /* 發送一個字節 */
            USART_SendData(USART2, UART_TxBuffer[UART_TxIndex++]);
        }
        else
        {
            /* 發送完成，禁用發送中斷 */
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            
            /* 啟用發送完成中斷 */
            USART_ITConfig(USART2, USART_IT_TC, ENABLE);
        }
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
    {
        /* 傳輸完成，清除標誌 */
        USART_ClearITPendingBit(USART2, USART_IT_TC);
        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        
        /* 標記發送完成 */
        UART_TxBusy = 0;
    }
}