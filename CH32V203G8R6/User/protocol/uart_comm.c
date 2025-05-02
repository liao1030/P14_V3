/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_comm.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x UART ͨӍ�ӌ��F
 ********************************************************************************/

#include "uart_comm.h"
#include "ch32v20x_usart.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_gpio.h"
#include "debug.h"
#include <string.h>

/* ���վ��n�^ */
static uint8_t UART_RxBuffer[UART_BUFFER_SIZE];
static uint16_t UART_RxBufferHead = 0;
static uint16_t UART_RxBufferTail = 0;
static volatile uint8_t UART_RxFlag = 0;

/* �l�;��n�^ */
static uint8_t UART_TxBuffer[UART_BUFFER_SIZE];
static uint16_t UART_TxCount = 0;
static uint16_t UART_TxIndex = 0;
static volatile uint8_t UART_TxBusy = 0;

/* ���ջ��{���� */
static UART_RxCallback_t UART_RxCallback = NULL;

/*********************************************************************
 * @fn      UART_Comm_Init
 *
 * @brief   ��ʼ��UART2ͨӍ
 *
 * @return  none
 */
void UART_Comm_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* ����USART2��GPIOA�r� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* ����USART2 Tx (PA2) ������ݔ��ģʽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ����USART2 Rx (PA3) �鸡��ݔ��ģʽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ����USART2���� */
    USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    /* ����USART2�Д� */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* ����USART2�����Д� */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* ����USART2 */
    USART_Cmd(USART2, ENABLE);
    
    /* ��ʼ�����n�^ */
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
 * @brief   ͨ�^UART�l�͔���
 *
 * @param   data - ���l�͔���ָ�
 *          length - ���l�͔����L��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
uint8_t UART_SendData(uint8_t *data, uint16_t length)
{
    uint16_t i;
    
    if (length > UART_BUFFER_SIZE)
    {
        return 1;  // ����̫�L���������n�^
    }
    
    /* �ȴ�֮ǰ�İl����� */
    while (UART_TxBusy)
    {
        ;
    }
    
    /* ��ؐ�������l�;��n�^ */
    for (i = 0; i < length; i++)
    {
        UART_TxBuffer[i] = data[i];
    }
    
    UART_TxCount = length;
    UART_TxIndex = 0;
    UART_TxBusy = 1;
    
    /* ���Ӱl�� */
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    
    return 0;
}

/*********************************************************************
 * @fn      UART_ProcessRxData
 *
 * @brief   ̎����Ք���
 *
 * @return  none
 */
void UART_ProcessRxData(void)
{
    uint16_t length;
    uint8_t data[UART_BUFFER_SIZE];
    
    if (UART_RxFlag)
    {
        /* �P�]�Д����̎�����g�������M�� */
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
        
        /* �������Д� */
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        
        /* ���л��{����̎�픵�� */
        if (UART_RxCallback != NULL && length > 0)
        {
            UART_RxCallback(data, length);
        }
    }
}

/*********************************************************************
 * @fn      UART_SetRxCallback
 *
 * @brief   �O�ý��ջ��{����
 *
 * @param   callback - ���{����ָ�
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
 * @brief   ����/���ý����Д�
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
 * @brief   ����/���ðl���Д�
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
 * @brief   USART2�Д�̎����
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        /* �xȡ���յ��Ĕ��� */
        uint8_t data = USART_ReceiveData(USART2);
        
        /* ������վ��n�^ */
        UART_RxBuffer[UART_RxBufferHead] = data;
        UART_RxBufferHead = (UART_RxBufferHead + 1) % UART_BUFFER_SIZE;
        
        /* ��ӛ���յ����� */
        UART_RxFlag = 1;
        
        /* ����Д����I */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if (UART_TxIndex < UART_TxCount)
        {
            /* �l��һ���ֹ� */
            USART_SendData(USART2, UART_TxBuffer[UART_TxIndex++]);
        }
        else
        {
            /* �l����ɣ����ðl���Д� */
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            
            /* ���ðl������Д� */
            USART_ITConfig(USART2, USART_IT_TC, ENABLE);
        }
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
    {
        /* ��ݔ��ɣ�������I */
        USART_ClearITPendingBit(USART2, USART_IT_TC);
        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        
        /* ��ӛ�l����� */
        UART_TxBusy = 0;
    }
}