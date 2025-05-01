/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *P14 V2.1 主程式:
 *這個範例展示了P14 V2.1項目的主程式。
 */

#include "debug.h"
#include "P14_Init.h"

/* Global typedef */

/* Global define */
#define KEY1_PRESSED() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
#define KEY2_PRESSED() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)

/* 按鍵防抖配置 */
#define KEY_DEBOUNCE_MS       20      // 防抖延時時間(毫秒)
#define KEY_LONG_PRESS_MS     1000    // 長按觸發時間(毫秒)

/* 按鍵狀態枚舉 */
typedef enum {
    KEY_STATE_IDLE,           // 空閒狀態
    KEY_STATE_DEBOUNCE,       // 防抖狀態
    KEY_STATE_SHORT_PRESS,    // 短按確認
    KEY_STATE_LONG_PRESS      // 長按確認
} KeyState_TypeDef;

/* 按鍵結構體 */
typedef struct {
    KeyState_TypeDef state;   // 按鍵當前狀態
    uint32_t timestamp;       // 狀態切換時間戳
    uint8_t pressed;          // 當前是否按下
    uint8_t longPressed;      // 是否觸發長按
} Key_TypeDef;

/* Global Variable */
volatile uint32_t g_systicks = 0;  /* 將全局變量聲明為volatile，以便在中斷處理程序中修改 */
Key_TypeDef g_key1 = {KEY_STATE_IDLE, 0, 0, 0};
Key_TypeDef g_key2 = {KEY_STATE_IDLE, 0, 0, 0};

/*********************************************************************
 * @fn      TestParameterTable
 *
 * @brief   測試參數表功能
 *
 * @return  none
 */
void TestParameterTable(void)
{
    StripType_TypeDef currentType, newType;
    uint8_t testCount;
    
    /* 切換測試項目類型 */
    currentType = (StripType_TypeDef)P14_ParamTable_Read(PARAM_STRIP_TYPE);
    newType = (currentType + 1) % 5; // 循環切換0-4之間的值
    
    /* 更新參數表 */
    P14_ParamTable_Write(PARAM_STRIP_TYPE, (uint8_t)newType);
    P14_ParamTable_UpdateChecksum();
    P14_ParamTable_Save();
    
    /* 顯示切換後的項目 */
    printf("測試項目切換為: %s\r\n", P14_ParamTable_GetStripTypeName(newType));
    
    /* 增加測試次數 */
    P14_ParamTable_IncrementTestCount();
    
    /* 顯示當前測試次數 */
    testCount = P14_ParamTable_Read16(PARAM_NOT);
    printf("當前測試次數: %d\r\n", testCount);
}

/*********************************************************************
 * @fn      InitSysTick
 *
 * @brief   初始化系統計時器
 *
 * @return  none
 */
void InitSysTick(void)
{
    /* 設置SysTick為1ms中斷 */
    SysTick->CTLR = 0;
    SysTick->SR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock / 1000;  /* 1ms中斷一次 */
    
    /* 使能計數器和中斷 */
    SysTick->CTLR = 0x0F;  /* 使能計數器、中斷，並使用系統時鐘 */
}

/*********************************************************************
 * @fn      UpdateKeyState
 *
 * @brief   更新按鍵狀態機
 *
 * @param   key - 按鍵結構體指針
 * @param   isPressed - 當前是否按下
 *
 * @return  1: 按鍵發生狀態變化, 0: 無變化
 */
uint8_t UpdateKeyState(Key_TypeDef *key, uint8_t isPressed)
{
    uint8_t stateChanged = 0;
    uint32_t currentTime = g_systicks;
    
    switch (key->state) {
        case KEY_STATE_IDLE:
            if (isPressed) {
                key->state = KEY_STATE_DEBOUNCE;
                key->timestamp = currentTime;
            }
            break;
            
        case KEY_STATE_DEBOUNCE:
            if (isPressed) {
                if (currentTime - key->timestamp >= KEY_DEBOUNCE_MS) {
                    key->state = KEY_STATE_SHORT_PRESS;
                    key->pressed = 1;
                    stateChanged = 1;
                }
            } else {
                key->state = KEY_STATE_IDLE;
            }
            break;
            
        case KEY_STATE_SHORT_PRESS:
            if (isPressed) {
                if (currentTime - key->timestamp >= KEY_LONG_PRESS_MS) {
                    key->state = KEY_STATE_LONG_PRESS;
                    key->longPressed = 1;
                    stateChanged = 1;
                }
            } else {
                key->state = KEY_STATE_IDLE;
                key->pressed = 0;
                stateChanged = 1;
            }
            break;
            
        case KEY_STATE_LONG_PRESS:
            if (!isPressed) {
                key->state = KEY_STATE_IDLE;
                key->pressed = 0;
                key->longPressed = 0;
                stateChanged = 1;
            }
            break;
            
        default:
            key->state = KEY_STATE_IDLE;
            break;
    }
    
    return stateChanged;
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
    uint8_t key1Changed = 0;
    uint8_t key2Changed = 0;
    
    /* 系統初始化 */
    P14_CH32V203_System_Init();
    
    /* 初始化系統計時器 */
    InitSysTick();
    
    printf("P14 V2.1 系統已啟動\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
    
    /* 開啟綠色LED */
    GPIO_SetBits(GPIOB, GPIO_Pin_7);
    
    /* 主循環 */
    while(1)
    {
        /* 更新按鍵狀態 */
        key1Changed = UpdateKeyState(&g_key1, KEY1_PRESSED());
        key2Changed = UpdateKeyState(&g_key2, KEY2_PRESSED());
        
        /* 處理KEY1按鍵操作 */
        if (key1Changed && g_key1.pressed) {
            printf("檢測到KEY1按鍵按下\r\n");
            
            /* 切換紅色LED狀態 */
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_6)));
        }
        
        if (key1Changed && g_key1.longPressed) {
            printf("檢測到KEY1長按\r\n");
            /* 長按處理邏輯可以放在這裡 */
        }
        
        /* 處理KEY2按鍵操作 */
        if (key2Changed && g_key2.pressed) {
            printf("檢測到KEY2按鍵按下\r\n");
            
            /* 測試參數表功能 */
            TestParameterTable();
        }
        
        if (key2Changed && g_key2.longPressed) {
            printf("檢測到KEY2長按\r\n");
            /* 長按處理邏輯可以放在這裡 */
        }
        
        /* 系統延時 */
        Delay_Ms(10);
        
        /* 綠色LED閃爍作為系統運行指示 */
        if ((g_systicks / 1000) % 2 == 0) {
            GPIO_SetBits(GPIOB, GPIO_Pin_7);
        } else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
        }
    }
}
