/**
 * @file P14_UART.c
 * @brief �๦�������z�y�xUARTͨӍ���挍�F
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

/* UART���n�^ */
static uint8_t g_uart1RxBuffer[UART_RX_BUFFER_SIZE];
static uint8_t g_uart1TxBuffer[UART_TX_BUFFER_SIZE];
static volatile uint16_t g_uart1RxHead = 0;
static volatile uint16_t g_uart1RxTail = 0;
static volatile uint16_t g_uart1TxHead = 0;
static volatile uint16_t g_uart1TxTail = 0;
static volatile uint8_t g_uart1TxBusy = 0;

/**
 * @brief ��ʼ��UART1
 * 
 * @param baudRate ������
 */
void UART1_Init(uint32_t baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* ����GPIOA��USART1�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    
    /* ����USART1 Tx (PA.09) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ����USART1 Rx (PA.10) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* USART1 ���� */
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    /* ����USART1�����Д� */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    /* ����USART1 */
    USART_Cmd(USART1, ENABLE);
    
    /* ����USART1�Д� */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* ��ս��վ��n�^ */
    g_uart1RxHead = 0;
    g_uart1RxTail = 0;
}

/**
 * @brief USART1�Д�̎����
 */
void USART1_IRQHandler(void)
{
    uint8_t data;
    
    /* ̎������Д� */
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        /* �xȡ���յ��Ĕ��� */
        data = USART_ReceiveData(USART1);
        
        /* �������惦���h�ξ��n�^ */
        uint16_t nextHead = (g_uart1RxHead + 1) % UART_RX_BUFFER_SIZE;
        if (nextHead != g_uart1RxTail) {
            g_uart1RxBuffer[g_uart1RxHead] = data;
            g_uart1RxHead = nextHead;
        }
        
        /* ����Д����I */
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    
    /* ̎��l������Д� */
    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET) {
        /* �z��l�;��n�^�Ƿ�߀�Д��� */
        if (g_uart1TxHead != g_uart1TxTail) {
            /* �l����һ���ֹ� */
            USART_SendData(USART1, g_uart1TxBuffer[g_uart1TxTail]);
            g_uart1TxTail = (g_uart1TxTail + 1) % UART_TX_BUFFER_SIZE;
        } else {
            /* ���Д����Ѱl���ꮅ */
            g_uart1TxBusy = 0;
            
            /* ���ðl������Д� */
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        }
        
        /* ����Д����I */
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
}

/**
 * @brief �l�͔���
 * 
 * @param data �Y��ָ�
 * @param len �Y���L��
 */
void UART1_SendData(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        /* Ӌ����һ���^��λ�� */
        uint16_t nextHead = (g_uart1TxHead + 1) % UART_TX_BUFFER_SIZE;
        
        /* �z�龏�n�^�Ƿ��������g */
        if (nextHead == g_uart1TxTail) {
            /* ���n�^�ѝM���G��ʣ�N�Y�� */
            break;
        }
        
        /* ���������뾏�n�^ */
        g_uart1TxBuffer[g_uart1TxHead] = data[i];
        g_uart1TxHead = nextHead;
    }
    
    /* ���UART�l������æ�����ӂ�ݔ */
    if (g_uart1TxBusy == 0) {
        g_uart1TxBusy = 1;
        
        /* ���ðl������Д� */
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        
        /* �l�͵�һ���ֹ� */
        USART_SendData(USART1, g_uart1TxBuffer[g_uart1TxTail]);
        g_uart1TxTail = (g_uart1TxTail + 1) % UART_TX_BUFFER_SIZE;
    }
}

/**
 * @brief ̎����յ��Ĕ���
 * 
 * �˺���������ѭ�h�ж����{��
 */
void UART1_ProcessReceivedData(void)
{
    static uint8_t tempBuffer[UART_RX_BUFFER_SIZE];
    uint16_t dataCount = 0;
    
    /* �����Д��Ա��o�xȡ���� */
    NVIC_DisableIRQ(USART1_IRQn);
    
    /* Ӌ���ж��ٔ������� */
    while (g_uart1RxHead != g_uart1RxTail) {
        tempBuffer[dataCount++] = g_uart1RxBuffer[g_uart1RxTail];
        g_uart1RxTail = (g_uart1RxTail + 1) % UART_RX_BUFFER_SIZE;
        
        /* ��ֹ���n�^��� */
        if (dataCount >= UART_RX_BUFFER_SIZE) {
            break;
        }
    }
    
    /* �������Д� */
    NVIC_EnableIRQ(USART1_IRQn);
    
    /* ����Д������t�{�û��{���� */
    if (dataCount > 0) {
        UART1_RxCallback(tempBuffer, dataCount);
    }
}

/**
 * @brief UART1���Ք������{����
 * 
 * @param data ���յ��Ĕ���ָ�
 * @param length �����L��
 */
void UART1_RxCallback(uint8_t *data, uint16_t length)
{
    /* ��̎̎����յ��Ĕ������D�l�o�{���f�h̎��ģ�M */
    BLE_ProcessReceivedData(data, length);
} 