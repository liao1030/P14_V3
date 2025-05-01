/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v20x_it.h"
#include "debug.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
  while (1)
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
  NVIC_SystemReset();
  while (1)
  {
  }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 interrupt.
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    uint8_t receivedData;

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // ����Д����I
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        
        // �xȡ���յ��Ĕ���
        receivedData = USART_ReceiveData(USART2);
        
        // ̎����յ��Ĕ���
        // �������@�e����Զ��x߉݋
        
        // ʾ��: ���@�յ��Ĕ��� (�yԇ��)
        USART_SendData(USART2, receivedData);
    }
}


