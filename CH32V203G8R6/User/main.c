/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH & HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : 多功能生化測試儀主程序
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "debug.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_usart.h"
#include "param_table.h" // 添加參數表頭文件

/* Global typedef */

/* Global define */
#define FW_VERSION      1
#define MODEL_NUMBER    10

/* Global Variable */
// 移除重複定義的MCU_Version變量，因為它已經在ch32v20x_gpio.c中定義
extern uint8_t MCU_Version;

/* CH32V203 初始化函數 */
void CH32V203_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 系統時鐘初始化，使用HSI作為系統時鐘源，16MHz */
    RCC_DeInit();
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    
    /* GPIO 時鐘使能 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* UART2 時鐘使能 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* 配置LED引腳 (PB6, PB7) 為輸出模式，初始狀態為低電平 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* LED初始狀態設為低電平(熄滅) */
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // 紅色LED
    GPIO_ResetBits(GPIOB, GPIO_Pin_7); // 綠色LED
    
    /* 配置WE_ENABLE (PB15) 和 T1_ENABLE (PA8) 為輸出模式，初始狀態為高電平 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // WE_ENABLE 高電平
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // T1_ENABLE 高電平
    
    /* 配置按鍵輸入 KEY_1 (PB11) 和 KEY_2 (PB10) 為上拉輸入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置ADC輸入引腳為浮空輸入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 配置UART2引腳 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // TX引腳
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // RX引腳
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2初始化 - 波特率115200、8位元資料、1位元停止位、無同位檢查 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    
    /* 初始化延遲函數 */
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
    
    /* 初始化CH32V203 */
    CH32V203_Init();
    
    /* 系統啟動消息 */
    printf("\r\n\r\n");
    printf("==============================\r\n");
    printf(" 多功能生化測試儀 V%d.%d\r\n", MODEL_NUMBER, FW_VERSION);
    printf(" MCU: CH32V203G8R6\r\n");
    printf(" Build: %s %s\r\n", __DATE__, __TIME__);
    printf("==============================\r\n\r\n");
    
    /* 初始化參數表 */
    printf("初始化參數表...\r\n");
    PARAM_TABLE_Init();
    
    /* 印出參數表資訊 */
    PARAM_TABLE_PrintInfo();
    
    /* 更新版本號 */
    g_ParamTable.MODEL_NO = MODEL_NUMBER;
    g_ParamTable.FW_NO = FW_VERSION;
    
    /* 主循環程序 */
    while(1)
    {
        /* 每秒執行一次的任務 */
        if(ticks % 100 == 0) {
            /* 閃爍LED指示燈 (PB7) */
            GPIO_WriteBit(GPIOB, GPIO_Pin_7, !GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7));
            
            /* 更新日期時間 (這裡只是示範，實際應用可能使用RTC) */
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
        
        /* 檢測按鍵輸入 */
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) { // KEY_1 按下 (低電平有效)
            /* 模擬測量完成，增加測試次數 */
            g_ParamTable.NOT++;
            /* 保存參數表到Flash */
            PARAM_TABLE_SaveToFlash();
            printf("測試完成，測試次數已更新: %d\r\n", g_ParamTable.NOT);
            
            /* 防抖動延时 */
            Delay_Ms(300);
        }
        
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0) { // KEY_2 按下 (低電平有效)
            /* 切換測試項目 */
            g_ParamTable.Strip_Type = (g_ParamTable.Strip_Type + 1) % 5;
            printf("切換測試項目為: ");
            switch(g_ParamTable.Strip_Type) {
                case STRIP_TYPE_GLV: printf("血糖(GLV)\r\n"); break;
                case STRIP_TYPE_U:   printf("尿酸(U)\r\n"); break;
                case STRIP_TYPE_C:   printf("總膽固醇(C)\r\n"); break;
                case STRIP_TYPE_TG:  printf("三酸甘油脂(TG)\r\n"); break;
                case STRIP_TYPE_GAV: printf("血糖(GAV)\r\n"); break;
                default: printf("未知\r\n"); break;
            }
            
            /* 防抖動延时 */
            Delay_Ms(300);
        }
        
        /* 延時10ms */
        Delay_Ms(10);
        ticks++;
    }
}
