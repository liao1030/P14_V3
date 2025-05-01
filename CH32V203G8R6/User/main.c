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
    BasicSystemBlock basicParams;
    
    /* 讀取當前基本系統參數 */
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("讀取基本系統參數失敗\r\n");
        return;
    }
    
    /* 切換測試項目類型 */
    StripType_TypeDef currentType = (StripType_TypeDef)basicParams.stripType;
    StripType_TypeDef newType = (currentType + 1) % 5; // 循環切換0-4之間的值
    
    /* 更新參數 */
    basicParams.stripType = (uint8_t)newType;
    
    /* 寫入更新後的參數 */
    if (!PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("更新基本系統參數失敗\r\n");
        return;
    }
    
    /* 顯示切換後的項目 */
    printf("測試項目切換為: %s\r\n", P14_ParamTable_GetStripTypeName(newType));
    
    /* 測試記錄儲存功能 */
    uint16_t randomValue = g_systicks % 300 + 100;  // 模擬測量值 (100-399)
    if (PARAM_SaveTestRecord(randomValue, 0, basicParams.defaultEvent, 0, 3000, 250)) {
        printf("儲存測試記錄成功，測量值: %d\r\n", randomValue);
    } else {
        printf("儲存測試記錄失敗\r\n");
    }
    
    /* 顯示當前測試次數 */
    uint16_t testCount = PARAM_GetTestCount();
    printf("當前測試次數: %d\r\n", testCount);
    
    /* 顯示測試記錄總數 */
    uint16_t recordCount = PARAM_GetTestRecordCount();
    printf("測試記錄總數: %d\r\n", recordCount);
    
    /* 如果有測試記錄，顯示最新的一條 */
    if (recordCount > 0) {
        TestRecord record;
        if (PARAM_GetTestRecord(0, &record)) {
            printf("最新測試記錄: 類型=%d, 結果=%d, 事件=%d, 時間=%02d-%02d-%02d %02d:%02d:%02d\r\n",
                   record.testType, record.resultValue, record.event,
                   record.year, record.month, record.date,
                   record.hour, record.minute, record.second);
        }
    }
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
