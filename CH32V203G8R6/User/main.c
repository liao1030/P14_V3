/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH & HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : �๦�������yԇ�x������
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "debug.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_usart.h"
#include "param_table.h" // ��Ӆ������^�ļ�

/* Global typedef */

/* Global define */
#define FW_VERSION      1
#define MODEL_NUMBER    10

/* Global Variable */
// �Ƴ����}���x��MCU_Version׃����������ѽ���ch32v20x_gpio.c�ж��x
extern uint8_t MCU_Version;

/* CH32V203 ��ʼ������ */
void CH32V203_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* ϵ�y�r犳�ʼ����ʹ��HSI����ϵ�y�r�Դ��16MHz */
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    
    /* GPIO �r�ʹ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* UART2 �r�ʹ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* ����LED���_ (PB6, PB7) ��ݔ��ģʽ����ʼ��B����ƽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* LED��ʼ��B�O����ƽ(Ϩ��) */
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // �tɫLED
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // �GɫLED
    
    /* ����WE_ENABLE (PB15) �� T1_ENABLE (PA8) ��ݔ��ģʽ����ʼ��B����ƽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // WE_ENABLE ���ƽ
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // T1_ENABLE ���ƽ
    
    /* ���ð��Iݔ�� KEY_1 (PB11) �� KEY_2 (PB10) ������ݔ��ģʽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ����ADCݔ�����_�鸡��ݔ��ģʽ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* ����UART2���_ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // TX���_
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // RX���_
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2��ʼ�� - ������115200��8λԪ�Y�ϡ�1λԪֹͣλ���oͬλ�z�� */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    
    /* ��ʼ�����t���� */
    Delay_Init();
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    uint32_t ticks = 0;
    
    /* ��ʼ��CH32V203 */
    CH32V203_Init();
    
    /* ϵ�y������Ϣ */
    printf("\r\n\r\n");
    printf("==============================\r\n");
    printf(" �๦�������yԇ�x V%d.%d\r\n", MODEL_NUMBER, FW_VERSION);
    printf(" MCU: CH32V203G8R6\r\n");
    printf(" Build: %s %s\r\n", __DATE__, __TIME__);
    printf("==============================\r\n\r\n");
    
    /* ��ʼ�������� */
    printf("��ʼ��������...\r\n");
    PARAM_TABLE_Init();
    
    /* ӡ���������YӍ */
    PARAM_TABLE_PrintInfo();
    
    /* ���°汾̖ */
    g_ParamTable.MODEL_NO = MODEL_NUMBER;
    g_ParamTable.FW_NO = FW_VERSION;
    
    /* ��ѭ�h���� */
    while(1)
    {
        /* ÿ�����һ�ε��΄� */
        if(ticks % 100 == 0) {
            /* �W�qLEDָʾ�� (PB7) */
            GPIO_WriteBit(GPIOB, GPIO_Pin_7, !GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7));
            
            /* �������ڕr�g (�@�eֻ��ʾ�������H���ÿ���ʹ��RTC) */
            g_ParamTable.SECOND++;
            if(g_ParamTable.SECOND >= 60) {
                g_ParamTable.SECOND = 0;
                g_ParamTable.MINUTE++;
                if(g_ParamTable.MINUTE >= 60) {
                    g_ParamTable.MINUTE = 0;
                    g_ParamTable.HOUR++;
                    if(g_ParamTable.HOUR >= 24) {
                        g_ParamTable.HOUR = 0;
                    }
                }
            }
        }
        
        /* �z�y���Iݔ�� */
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) { // KEY_1 ���� (���ƽ��Ч)
            /* ģ�M�y����ɣ����Ӝyԇ�Δ� */
            g_ParamTable.NOT++;
            /* ���慢����Flash */
            PARAM_TABLE_SaveToFlash();
            printf("�yԇ��ɣ��yԇ�Δ��Ѹ���: %d\r\n", g_ParamTable.NOT);
            
            /* ��������ʱ */
            Delay_Ms(300);
        }
        
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0) { // KEY_2 ���� (���ƽ��Ч)
            /* �ГQ�yԇ�Ŀ */
            g_ParamTable.Strip_Type = (g_ParamTable.Strip_Type + 1) % 5;
            printf("�ГQ�yԇ�Ŀ��: ");
            switch(g_ParamTable.Strip_Type) {
                case STRIP_TYPE_GLV: printf("Ѫ��(GLV)\r\n"); break;
                case STRIP_TYPE_U:   printf("����(U)\r\n"); break;
                case STRIP_TYPE_C:   printf("��đ�̴�(C)\r\n"); break;
                case STRIP_TYPE_TG:  printf("�������֬(TG)\r\n"); break;
                case STRIP_TYPE_GAV: printf("Ѫ��(GAV)\r\n"); break;
                default: printf("δ֪\r\n"); break;
            }
            
            /* ��������ʱ */
            Delay_Ms(300);
        }
        
        /* �ӕr10ms */
        Delay_Ms(10);
        ticks++;
    }
}
