/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/08/01
 * Description        : CH32V203 �Д�̎�����
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "ch32v20x_it.h"
#include "debug.h"
#include "P14_BLE_Protocol.h" // �����֧��ԇƬ�ɜy�f�h

/* ȫ��׃���� */
extern volatile uint32_t g_systicks;

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/* ��main.c�Ќ��F��SysTick̎���� */
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
 * @brief   ̎��USART2(�cCH582FͨӍ)�Д�
 *          ����ԇƬ�ɜy�����̎��
 *
 * @return  none
 */
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* ���Ք��� */
        uint8_t rx_data = USART_ReceiveData(USART2);
        
        /* ̎����յ��Ĕ��� */
        BLE_ProtocolRxHandler(rx_data);
        
        /* ����Д����I */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/*********************************************************************
 * @fn      SysTick_Handler
 *
 * @brief   ϵ�y�δ�Ӌ�r���Д�̎����
 *
 * @return  none
 */
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    /* ����ϵ�y�r�gӋ�� */
    g_systicks++;
    
    /* ����Д����I */
    SysTick->SR = 0;
}


