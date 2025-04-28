/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F����ʽ
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* ȫ�ֶ��x */
#define STRIP_DETECT_3_PIN     GPIO_Pin_11      /* PB11 ԇƬ����z�y */
#define T3_IN_SEL_PIN          GPIO_Pin_10      /* PB10 T3늘O���y���ܿ��� */
#define STRIP_DETECT_5_PIN     GPIO_Pin_15      /* PA15 ԇƬ����z�y */
#define V2P5_ENABLE_PIN        GPIO_Pin_14      /* PA14 ��늽oCH32V203���ܿ��� */
#define V_BACK_C_PIN           GPIO_Pin_13      /* PA13 �ⲿUART�O��Rx Level Shift�Դ�x����� */
#define VBUS_GET_PIN           GPIO_Pin_12      /* PA12 ���y�ⲿ�O���Ƿ�� */

/* ������ */
void SystemInit(void);
void GPIO_Init(void);
void GPIO_INT_Init(void);
void UART_Init(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main(void)
{
    /* ϵ�y������ʼ�� */
    SystemInit();
    
    /* GPIO��ʼ�� */
    GPIO_Init();
    
    /* GPIO�Д��ʼ�� */
    GPIO_INT_Init();
    
    /* UART��ʼ�� */
    UART_Init();
    
    /* �����yԇݔ�� */
    PRINT("P14_V2.1 BLE ��ʼ�����\n");
    PRINT("ϵ�y�r�: %d MHz\n", GetSysClock() / 1000000);
    
    /* ��ѭ�h */
    while(1)
    {
        /* ��ѭ�h���a */
    }
}

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   ϵ�y�r犳�ʼ��
 *
 * @return  none
 */
void SystemInit(void)
{
    /* ������O�}λ */
    PERIPHERAL_RESET();
    
    /* ϵ�y�r犳�ʼ�� */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* ϵ�y�Д��ʼ�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* �_���r� */
    GPIOA_ModeCfg(PIN_WAKEUP_GPIO_ALL, Pin_ModeIN_PD);
    LowPower_Idle();
}

/*********************************************************************
 * @fn      GPIO_Init
 *
 * @brief   GPIO��ʼ������
 *
 * @return  none
 */
void GPIO_Init(void)
{
    /* ԇƬ����z�y���_ - PB11 */
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    
    /* T3늘O���y���ܿ��� - PB10 (���ƽ����) */
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);
    
    /* ԇƬ����z�y���_ - PA15 */
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    
    /* ��늽oCH32V203���ܿ��� - PA14 (���ƽ����) */
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    
    /* �ⲿUART�O��Rx Level Shift�Դ�x����� - PA13 */
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);
    
    /* ���y�ⲿ�O���Ƿ�� - PA12 */
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);
    
    /* ��ʼ�O��ݔ����B */
    GPIOB_ResetBits(T3_IN_SEL_PIN);     /* T3늘O���y����(���ƽ) */
    GPIOA_SetBits(V2P5_ENABLE_PIN);     /* ��늽oCH32V203����(���ƽ) */
    GPIOA_ResetBits(V_BACK_C_PIN);      /* UART Level Shift�Դ�x��(���ƽ) */
}

/*********************************************************************
 * @fn      GPIO_INT_Init
 *
 * @brief   GPIO�Д��ʼ������
 *
 * @return  none
 */
void GPIO_INT_Init(void)
{
    /* ����ԇƬ����z�y���_�Д� - PB11 */
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    
    /* ����ԇƬ����z�y���_�Д� - PA15 */
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    
    /* ����GPIO�Д� */
    PFIC_EnableIRQ(GPIO_IRQn);
}

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   UART��ʼ������
 *
 * @return  none
 */
void UART_Init(void)
{
    /* UART1��ʼ�� - �cCH32V203ͨӍ */
    GPIOA_SetBits(GPIO_Pin_8);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);         /* RX (PA8) */
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);    /* TX (PA9) */
    
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    /* UART0��ʼ�� - �c�ⲿ�O��1ͨӍ */
    GPIOB_SetBits(GPIO_Pin_4);
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);         /* RX (PB4) */
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);    /* TX (PB7) */
    
    UART0_DefInit();
    UART0_BaudRateCfg(115200);
    
    /* UART3��ʼ�� - �c�ⲿ�O��2ͨӍ */
    GPIOA_SetBits(GPIO_Pin_4);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);         /* RX (PA4) */
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);    /* TX (PA5) */
    
    UART3_DefInit();
    UART3_BaudRateCfg(115200);
    
    /* ����UART1�Д���� */
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
} 