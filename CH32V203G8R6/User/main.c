/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

/*
 *@Note
 *P14_V2.1 ����ʽ
 *CH32V203G8R6 MCU��ʼ���c��Ҫ���܌��F
 */

#include "debug.h"
#include "param_table.h"
#include "param_test.h"
#include "param_store.h"
#include "uart_protocol.h"

/* ȫ�ֶ��x */
#define LED_RED_PIN     GPIO_Pin_6
#define LED_GREEN_PIN   GPIO_Pin_7
#define KEY_1_PIN       GPIO_Pin_11
#define KEY_2_PIN       GPIO_Pin_10
#define T1_ENABLE_PIN   GPIO_Pin_8
#define WE_ENABLE_PIN   GPIO_Pin_15

/* UART���P���n�^ */
#define UART_RX_BUF_SIZE       128
static uint8_t uart2_rx_buf[UART_RX_BUF_SIZE];
static uint16_t uart2_rx_len = 0;

/* ������ */
void GPIO_Config(void);
void UART2_Config(void);
void Key_Process(void);
void ParamTable_Test(void);
void USART_Receive_Byte(void);
void Process_UART_Data(void);
void Timer2_Config(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    /* �Д����ȼ��ֽM�O�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* ϵ�y�r犳�ʼ�� - ʹ��HSI�r�Դ�\����16MHz */
    SystemCoreClockUpdate();
    
    /* ���t��ʼ�� */
    Delay_Init();
    
    /* GPIO ��ʼ�� */
    GPIO_Config();
    
    /* UART2 ��ʼ�� - �cCH582FͨӍ */
    UART2_Config();
    
    /* ���r��2���� - ����������΄� */
    Timer2_Config();
    
    /* ͨӍ�f�h��ʼ�� */
    UART_Protocol_Init();
    
    /* �{ԇ��UART��ʼ�� */
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    /* ��ʼ�������惦ģ�K */
    printf("��ʼ�������惦ģ�K...\r\n");
    if (PARAM_Init() != 0) {
        printf("�����惦ģ�K��ʼ��ʧ����\r\n");
    } else {
        printf("�����惦ģ�K��ʼ���ɹ���\r\n");
    }
    
    /* �yԇ���P���ֿ��Բ����� */
    /*
    printf("�������I�_ʼ�����惦�yԇ...\r\n");
    USART_Receive_Byte();
    PARAM_TestAll();
    */
    
    printf("ϵ�y��ʼ����ɣ��M��ѭ�h\r\n");
    printf("�{��UART�f�h�ц���\r\n");
    
    while(1)
    {
        /* ̎���I */
        Key_Process();
        
        /* ̎��UART���� */
        Process_UART_Data();
        
        /* ���t */
        Delay_Ms(10);
    }
}

/*********************************************************************
 * @fn      GPIO_Config
 *
 * @brief   GPIO��ʼ������
 *
 * @return  none
 */
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* ʹ��GPIOA��GPIOB�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* LED�������_���� - PB6(�tɫ)��PB7(�Gɫ) */
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ���Iݔ������ - PB10��PB11 (���Iݔ�룬����ݔ��) */
    GPIO_InitStructure.GPIO_Pin = KEY_1_PIN | KEY_2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* T1늘O���y���ܿ��� - PA8 (���ƽ����) */
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W늘O늉�PWM���� - PB15 */
    GPIO_InitStructure.GPIO_Pin = WE_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* �ض����ݔ�� - PA1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ԇƬT1늘O늉�ݔ�� - PA6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ԇƬT3늘O늉�ݔ�� - PB0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ���������Y��ݔ�� - PA4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W늘O늉�ݔ�� - PA7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 늳�늉��z�y - PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ��ʼ�O��ݔ����B */
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);    /* T1늘O���y����(���ƽ) */
    GPIO_SetBits(GPIOB, WE_ENABLE_PIN);    /* W늘O늉�PWMݔ������(���ƽ) */
    GPIO_ResetBits(GPIOB, LED_RED_PIN);    /* �tɫLED�P�] */
    GPIO_ResetBits(GPIOB, LED_GREEN_PIN);  /* �GɫLED�P�] */
}

/*********************************************************************
 * @fn      UART2_Config
 *
 * @brief   UART2���� - �cCH582FͨӍ
 *
 * @return  none
 */
void UART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    /* ʹ��GPIOA��UART2�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* UART2 TX(PA2)���_���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2 RX(PA3)���_���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2�������� */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    /* ����UART2 */
    USART_Init(USART2, &USART_InitStructure);
    
    /* ����UART2�Д� */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* ����UART2�����Д� */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* ʹ��UART2 */
    USART_Cmd(USART2, ENABLE);
    
    /* ��ս��վ��n�^ */
    uart2_rx_len = 0;
}

/*********************************************************************
 * @fn      Timer2_Config
 *
 * @brief   ���r��2���� - ����������΄�
 *
 * @return  none
 */
void Timer2_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    /* ʹ�ܶ��r��2�r� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    /* ���A���r������ */
    TIM_TimeBaseStructure.TIM_Period = 100-1;           // 100msӋ���L��
    TIM_TimeBaseStructure.TIM_Prescaler = 16000-1;      // 16MHz / 16000 = 1kHzӋ���l��
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* ʹ�ܶ��r��2�Д� */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    /* ���ö��r��2�Д� */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* ���Ӷ��r�� */
    TIM_Cmd(TIM2, ENABLE);
}

/*********************************************************************
 * @fn      Process_UART_Data
 *
 * @brief   ̎��UART���յ��Ĕ���
 *
 * @return  none
 */
void Process_UART_Data(void)
{
    /* ����Д�����Ҫ̎�� */
    if (uart2_rx_len > 0) {
        /* ͨ�^�f�h̎�� */
        UART_Protocol_Parse(uart2_rx_buf, uart2_rx_len);
        
        /* ��վ��n�^ */
        uart2_rx_len = 0;
    }
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
    /* ̎������Д� */
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* �xȡ���յ��Ĕ��� */
        uint8_t rx_data = USART_ReceiveData(USART2);
        
        /* ���뾏�n�^ */
        if (uart2_rx_len < UART_RX_BUF_SIZE) {
            uart2_rx_buf[uart2_rx_len++] = rx_data;
        }
        
        /* ����Д����I */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/*********************************************************************
 * @fn      TIM2_IRQHandler
 *
 * @brief   TIM2�Д�̎����
 *
 * @return  none
 */
void TIM2_IRQHandler(void)
{
    static uint8_t led_state = 0;
    static uint16_t counter = 0;
    
    /* �z������Д� */
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        /* ����Д����I */
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        /* 100ms�΄� */
        counter++;
        
        /* �W�qLED��ָʾϵ�y�����\�� */
        if (counter % 5 == 0) {  // 500ms
            if (led_state) {
                GPIO_SetBits(GPIOB, LED_GREEN_PIN);
            } else {
                GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
            }
            led_state = !led_state;
        }
        
        /* �����������΄� */
        if (counter >= 600) {  // ÿ��犈���һ��
            counter = 0;
            /* �����΄գ������ԙz�� */
        }
    }
}

/*********************************************************************
 * @fn      ParamTable_Test
 *
 * @brief   �yԇ�������ܣ���ӡ��ǰ�����O���K�޸�ĳ����
 *
 * @return  none
 */
void ParamTable_Test(void);

/*********************************************************************
 * @fn      Key_Process
 *
 * @brief   ̎���Iݔ��
 *
 * @return  none
 */
void Key_Process(void)
{
    static uint8_t key1_last = 1;
    static uint8_t key2_last = 1;
    uint8_t key1_curr, key2_curr;
    
    /* �xȡ��ǰ���I��B */
    key1_curr = GPIO_ReadInputDataBit(GPIOB, KEY_1_PIN);
    key2_curr = GPIO_ReadInputDataBit(GPIOB, KEY_2_PIN);
    
    /* �z�yKEY1���� (�½���) */
    if (key1_last == 1 && key1_curr == 0) {
        printf("KEY1 pressed - Executing parameter test\r\n");
        PARAM_TestAll(); /* ���Ѕ����惦�yԇ */
    }
    
    /* �z�yKEY2���� (�½���) */
    if (key2_last == 1 && key2_curr == 0) {
        printf("KEY2 pressed - Toggling LED\r\n");
        
        /* �ГQLED��B */
        if (GPIO_ReadOutputDataBit(GPIOB, LED_GREEN_PIN)) {
            GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
        } else {
            GPIO_SetBits(GPIOB, LED_GREEN_PIN);
        }
    }
    
    /* ���°��I��B */
    key1_last = key1_curr;
    key2_last = key2_curr;
}

/*********************************************************************
 * @fn      USART_Receive_Byte
 *
 * @brief   Wait and receive a byte from USART.
 *
 * @return  none
 */
void USART_Receive_Byte(void)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    USART_ReceiveData(USART1);
}
