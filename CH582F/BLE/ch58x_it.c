/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch58x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F�Д�̎���ʽ
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* ȫ��׃�� */
uint8_t uart1RxBuffer[256];
volatile uint8_t uart1RxCount = 0;

/******************************************************************************/
/* �Д�̎���� */

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1�Д�̎���� - ̎���cCH32V203��ͨӍ
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void UART1_IRQHandler(void)
{
    uint8_t rxData;
    
    /* �����Д� */
    if(UART1_GetITFlag(RB_IER_RECV_RDY))
    {
        /* ����Д����I */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
        
        /* �xȡ���Ք��� */
        rxData = UART1_RecvByte();
        
        /* ������Ք��������n�^ */
        if(uart1RxCount < sizeof(uart1RxBuffer))
        {
            uart1RxBuffer[uart1RxCount++] = rxData;
        }
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO�Д�̎���� - ̎��ԇƬ����z�y
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void GPIO_IRQHandler(void)
{
    /* �Д��Ƿ��ԇƬ����z�y���_�Д� */
    if(GPIOB_ReadITFlagBit(GPIO_Pin_11))
    {
        /* ����Д����I */
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        
        /* ̎��ԇƬ����z�y�Д� */
        /* TODO: �ڴ����ԇƬ����̎��߉݋ */
    }
    
    if(GPIOA_ReadITFlagBit(GPIO_Pin_15))
    {
        /* ����Д����I */
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        
        /* ̎��ԇƬ����z�y�Д� */
        /* TODO: �ڴ����ԇƬ����̎��߉݋ */
    }
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   ���r��0�Д�̎����
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void TMR0_IRQHandler(void)
{
    /* ̎���r��0�Д� */
    /* TODO: �ڴ���Ӷ��r���Д�̎��߉݋ */
    
    /* ����Д����I */
    TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
} 