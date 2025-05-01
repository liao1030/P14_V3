/**
 * @file P14_UART.c
 * @brief 多功能生化檢測儀UART通訊介面實現
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "P14_UART.h"
#include "P14_BLE_Protocol.h"
#include "debug.h"
#include "string.h"

/* UART緩衝區 */
static uint8_t g_uart1RxBuffer[UART_RX_BUFFER_SIZE];
static uint8_t g_uart1TxBuffer[UART_TX_BUFFER_SIZE];
static volatile uint16_t g_uart1RxHead = 0;
static volatile uint16_t g_uart1RxTail = 0;
static volatile uint16_t g_uart1TxHead = 0;
static volatile uint16_t g_uart1TxTail = 0;
static volatile uint8_t g_uart1TxBusy = 0;

/**
 * @brief 初始化UART1
 * 
 * @param baudRate 波特率
 */
void UART1_Init(uint32_t baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 啟用GPIOA和USART1時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    
    /* 配置USART1 Tx (PA.09) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置USART1 Rx (PA.10) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* USART1 配置 */
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    /* 啟用USART1接收中斷 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    /* 啟用USART1 */
    USART_Cmd(USART1, ENABLE);
    
    /* 配置USART1中斷 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 清空接收緩衝區 */
    g_uart1RxHead = 0;
    g_uart1RxTail = 0;
}

/**
 * @brief USART1中斷處理函數
 */
void USART1_IRQHandler(void)
{
    uint8_t data;
    
    /* 處理接收中斷 */
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        /* 讀取接收到的數據 */
        data = USART_ReceiveData(USART1);
        
        /* 將數據存儲到環形緩衝區 */
        uint16_t nextHead = (g_uart1RxHead + 1) % UART_RX_BUFFER_SIZE;
        if (nextHead != g_uart1RxTail) {
            g_uart1RxBuffer[g_uart1RxHead] = data;
            g_uart1RxHead = nextHead;
        }
        
        /* 清除中斷標誌 */
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    
    /* 處理發送完成中斷 */
    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET) {
        /* 檢查發送緩衝區是否還有數據 */
        if (g_uart1TxHead != g_uart1TxTail) {
            /* 發送下一個字節 */
            USART_SendData(USART1, g_uart1TxBuffer[g_uart1TxTail]);
            g_uart1TxTail = (g_uart1TxTail + 1) % UART_TX_BUFFER_SIZE;
        } else {
            /* 所有數據已發送完畢 */
            g_uart1TxBusy = 0;
            
            /* 禁用發送完成中斷 */
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        }
        
        /* 清除中斷標誌 */
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
}

/**
 * @brief 發送數據
 * 
 * @param data 資料指針
 * @param len 資料長度
 */
void UART1_SendData(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        /* 計算下一個頭部位置 */
        uint16_t nextHead = (g_uart1TxHead + 1) % UART_TX_BUFFER_SIZE;
        
        /* 檢查緩衝區是否有足夠空間 */
        if (nextHead == g_uart1TxTail) {
            /* 緩衝區已滿，丟棄剩餘資料 */
            break;
        }
        
        /* 將數據存入緩衝區 */
        g_uart1TxBuffer[g_uart1TxHead] = data[i];
        g_uart1TxHead = nextHead;
    }
    
    /* 如果UART發送器不忙，啟動傳輸 */
    if (g_uart1TxBusy == 0) {
        g_uart1TxBusy = 1;
        
        /* 啟用發送完成中斷 */
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        
        /* 發送第一個字節 */
        USART_SendData(USART1, g_uart1TxBuffer[g_uart1TxTail]);
        g_uart1TxTail = (g_uart1TxTail + 1) % UART_TX_BUFFER_SIZE;
    }
}

/**
 * @brief 處理接收到的數據
 * 
 * 此函數應在主循環中定期調用
 */
void UART1_ProcessReceivedData(void)
{
    static uint8_t tempBuffer[UART_RX_BUFFER_SIZE];
    uint16_t dataCount = 0;
    
    /* 禁用中斷以保護讀取操作 */
    NVIC_DisableIRQ(USART1_IRQn);
    
    /* 計算有多少數據可用 */
    while (g_uart1RxHead != g_uart1RxTail) {
        tempBuffer[dataCount++] = g_uart1RxBuffer[g_uart1RxTail];
        g_uart1RxTail = (g_uart1RxTail + 1) % UART_RX_BUFFER_SIZE;
        
        /* 防止緩衝區溢出 */
        if (dataCount >= UART_RX_BUFFER_SIZE) {
            break;
        }
    }
    
    /* 重新啟用中斷 */
    NVIC_EnableIRQ(USART1_IRQn);
    
    /* 如果有數據，則調用回調函數 */
    if (dataCount > 0) {
        UART1_RxCallback(tempBuffer, dataCount);
    }
}

/**
 * @brief UART1接收數據回調函數
 * 
 * @param data 接收到的數據指針
 * @param length 數據長度
 */
void UART1_RxCallback(uint8_t *data, uint16_t length)
{
    /* 此處處理接收到的數據，轉發給藍牙協議處理模組 */
    BLE_ProcessReceivedData(data, length);
} 