/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 This example demonstrates receiving indeterminate length data via USART's IDLE interrupt and DMA.
 USART2_Tx(PA2), USART2_Rx(PA3).


*/

#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "param_table.h"
#include "uart_protocol.h"
#include "system_state.h"  // 系統狀態定義
#include "rtc.h"
#include "ch32v20x_opa.h"
#include "ch32v20x_tim.h"
#include "strip_detect.h"
#include "Calculation.h"  // 新增血糖計算函數標頭檔

void USART2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

/* 系統狀態定義已移至 system_state.h */

/* 系統狀態變量已移至 system_state.c */

/* 血糖測量相關變數 */
uint16_t W_ADC = 0;  // 工作電極ADC值，用於血糖計算

/* 狀態機處理函式宣告 */
void State_Process (void);

/*********************************************************************
 * @fn      GetMidADC
 *
 * @brief   對指定ADC通道進行連續取樣，排序後取中間值的平均值
 *
 * @param   channel - ADC通道 (例如: ADC_Channel_4)
 * @param   sampleCount - 總取樣次數
 * @param   midCount - 中間取樣數量 (用於計算平均值)
 *
 * @return  中間值的平均值
 */
uint16_t GetMidADC(uint32_t channel, uint16_t sampleCount, uint16_t midCount)
{
    uint16_t *samples;
    uint32_t sum = 0;
    uint16_t i, j;
    uint16_t temp;
    uint16_t startIndex, endIndex;
    
    // 動態分配記憶體來存儲取樣值
    samples = (uint16_t*)malloc(sampleCount * sizeof(uint16_t));
    if (samples == NULL) {
        printf("Memory allocation failed for ADC samples\r\n");
        return 0;
    }
    
    // 確保中間取樣數量不超過總取樣數量
    if (midCount > sampleCount) {
        midCount = sampleCount;
    }
    
    // 配置ADC通道
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    // 連續取樣
    for (i = 0; i < sampleCount; i++) {
        // 啟動ADC轉換
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        
        // 等待轉換完成
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        
        // 獲取ADC值
        samples[i] = ADC_GetConversionValue(ADC1);
        
        // 小延遲以確保穩定取樣
        Delay_Us(10);
    }
    
    // 使用氣泡排序對取樣值進行排序
    for (i = 0; i < sampleCount - 1; i++) {
        for (j = 0; j < sampleCount - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }
    
    // 計算中間值的範圍
    startIndex = (sampleCount - midCount) / 2;
    endIndex = startIndex + midCount;
    
    // 計算中間值的總和
    for (i = startIndex; i < endIndex; i++) {
        sum += samples[i];
    }
    
    // 釋放記憶體
    free(samples);
    
    // 返回平均值
    return (uint16_t)(sum / midCount);
}

// ring buffer size
#define RING_BUFFER_LEN (1024u)

// The length of a single buffer used by DMA
#define RX_BUFFER_LEN (128u)

#define USART_RX_CH DMA1_Channel6

struct
{
    volatile uint8_t DMA_USE_BUFFER;
    uint8_t Rx_Buffer[2][RX_BUFFER_LEN];

} USART_DMA_CTRL = {
    .DMA_USE_BUFFER = 0,
    .Rx_Buffer = {0},
};

struct
{
    uint8_t buffer[RING_BUFFER_LEN];
    volatile uint16_t RecvPos;  //
    volatile uint16_t SendPos;  //
    volatile uint16_t RemainCount;

} ring_buffer = {{0}, 0, 0, 0};

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 peripheral.
 *
 * @return  none
 */
void USARTx_CFG (uint32_t baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_USART2, ENABLE);

    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init (USART2, &USART_InitStructure);
    USART_ITConfig (USART2, USART_IT_IDLE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    USART_Cmd (USART2, ENABLE);
}

/*********************************************************************
 * @fn      DMA_INIT
 *
 * @brief   Configures the DMA for USART2.
 *
 * @return  none
 */
void DMA_INIT (void) {
    DMA_InitTypeDef DMA_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit (USART_RX_CH);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART_DMA_CTRL.Rx_Buffer[0];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_LEN;
    DMA_Init (USART_RX_CH, &DMA_InitStructure);

    DMA_ITConfig (USART_RX_CH, DMA_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    DMA_Cmd (USART_RX_CH, ENABLE);
    USART_DMACmd (USART2, USART_DMAReq_Rx, ENABLE);
}

/*********************************************************************
 * @fn      ring_buffer_push_huge
 *
 * @brief   Put a large amount of data into the ring buffer.
 *
 * @return  none
 */
void ring_buffer_push_huge (uint8_t *buffer, uint16_t len) {
    const uint16_t bufferRemainCount = RING_BUFFER_LEN - ring_buffer.RemainCount;
    if (bufferRemainCount < len) {
        len = bufferRemainCount;
    }

    const uint16_t bufferSize = RING_BUFFER_LEN - ring_buffer.RecvPos;
    if (bufferSize >= len) {
        memcpy (&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, len);
        ring_buffer.RecvPos += len;
    } else {
        uint16_t otherSize = len - bufferSize;
        memcpy (&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, bufferSize);
        memcpy (ring_buffer.buffer, &(buffer[bufferSize]), otherSize);
        ring_buffer.RecvPos = otherSize;
    }
    ring_buffer.RemainCount += len;
}

/*********************************************************************
 * @fn      ring_buffer_pop
 *
 * @brief   Get a data from the ring buffer.
 *
 * @return  the Data
 */
uint8_t ring_buffer_pop() {
    uint8_t data = ring_buffer.buffer[ring_buffer.SendPos];

    ring_buffer.SendPos++;
    if (ring_buffer.SendPos >= RING_BUFFER_LEN) {
        ring_buffer.SendPos = 0;
    }
    ring_buffer.RemainCount--;
    return data;
}

/*********************************************************************
 * @fn      OPA2_Init
 *
 * @brief   初始化OPA2，設定輸出、正向與負向輸入端。
 *
 * @return  none
 */
void OPA2_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    OPA_InitTypeDef OPA_InitStructure = {0};

    /* 啟用GPIOA和OPA的時鐘 */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);

    /* 配置OPA2相關的GPIO引腳 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    /* 配置OPA2 */
    OPA_InitStructure.OPA_NUM = OPA2;     /* 選擇OPA2 */
    OPA_InitStructure.PSEL = CHP1;        /* 選擇OPA2正向輸入端為CHP1(PA7) */
    OPA_InitStructure.NSEL = CHN1;        /* 選擇OPA2負向輸入端為CHN1(PA5) */
    OPA_InitStructure.Mode = OUT_IO_OUT1; /* OPA2輸出通道為OUT1(PA4) */
    OPA_Init (&OPA_InitStructure);

    /* 啟用OPA2 */
    OPA_Cmd (OPA2, ENABLE);

    printf ("OPA2 Initialized\r\n");
}

/*********************************************************************
 * @fn      TIM1_PWM_Init
 *
 * @brief   初始化TIM1，配置PB15(TIM1_CH3N)為20KHz PWM輸出，預設佔空比100%。
 *
 * @return  none
 */
void TIM1_PWM_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure = {0};

    /* 啟用GPIOB和TIM1的時鐘 */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB | RCC_APB2Periph_TIM1, ENABLE);

    /* 配置PB15 (TIM1_CH3N) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    /* 計算PWM頻率所需的參數
     * 系統時鐘 = 16MHz (根據SystemClk預設值)
     * 目標頻率 = 20KHz
     * 預分頻 = 1
     * 計數週期 = 16000000 / 20000 / 1 = 800
     */
    TIM_TimeBaseStructure.TIM_Period = 800 - 1;                 /* 計數週期，對應20KHz */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;                    /* 預分頻 = 1 */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;                /* 時鐘不分頻 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; /* 向上計數模式 */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit (TIM1, &TIM_TimeBaseStructure);

    /* 配置PWM通道3 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;               /* PWM模式2 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; /* 啟用互補輸出 */
    TIM_OCInitStructure.TIM_Pulse = 800 - 1;                        /* 預設 0% 佔空比 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC3Init (TIM1, &TIM_OCInitStructure);

    /* 設定TIM1的死區和斷路控制 */
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    TIM_BDTRInitStructure.TIM_DeadTime = 0;              /* 無死區時間 */
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable; /* 禁用斷路功能 */
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig (TIM1, &TIM_BDTRInitStructure);

    /* 啟用預加載 */
    TIM_OC3PreloadConfig (TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM1, ENABLE);

    /* 啟用TIM1 */
    TIM_Cmd (TIM1, ENABLE);
    TIM_CtrlPWMOutputs (TIM1, ENABLE);

    printf ("TIM1 PWM Initialized (20KHz, 100%% duty)\r\n");
}

/*********************************************************************
 * @fn      State_Process
 *
 * @brief   處理系統狀態機
 *
 * @return  none
 */
void State_Process (void) {
    static SystemState_TypeDef lastState = STATE_IDLE;
    SystemState_TypeDef currentState = System_GetState();
    
    // 如果狀態發生改變，重置相關標誌
    if (currentState != lastState) {
        // 重置所有狀態相關的靜態變數
        if (lastState == STATE_RESULT_READY) {
            // 當離開 RESULT_READY 狀態時，可以進行清理工作
            printf("Leaving RESULT_READY state\r\n");
        }
        lastState = currentState;
    }
    
    switch (currentState) {
    case STATE_IDLE:
        // 在空閒狀態下檢查是否有試片插入
        // 實作部分後續再增加
        break;

    case STATE_STRIP_DETECTED:
        // 處理檢測到試片狀態 - 這部分已由 STRIP_DETECT_Process 處理
        // 在確認試片類型後自動進入 STATE_WAIT_FOR_BLOOD 狀態
        break;

    case STATE_STRIP_VALIDATION:
        // 處理試片驗證階段
        // 實作部分後續再增加
        break;

    case STATE_PARAMETER_SETUP:
        // 處理參數設置階段
        // 實作部分後續再增加
        break;

    case STATE_WAIT_FOR_BLOOD: {
        // 處理等待血液滴入階段
        uint16_t adcValue, bloodInThreshold;
        uint16_t ndl, udl;  // 未使用，但需要在 PARAM_GetStripParameters 參數中

        // 從 PA4/ADC4 讀取 GLU_OUT 的 ADC 值
        // 配置 ADC 通道 4 (PA4)
        ADC_RegularChannelConfig (ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);

        // 啟動 ADC 轉換
        ADC_SoftwareStartConvCmd (ADC1, ENABLE);

        // 等待轉換完成
        while (!ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC))
            ;

        // 獲取ADC值
        adcValue = ADC_GetConversionValue (ADC1);

        // 獲取當前試片類型的血液檢測閾值
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();

        // 從參數表獲取對應試片類型的血液檢測閾值
        if (PARAM_GetStripParameters (currentStripType, &ndl, &udl, &bloodInThreshold) && (currentStripType < STRIP_TYPE_MAX)) {
            // 檢查ADC值是否超過閾值
            if (adcValue > bloodInThreshold) {
                // 檢測到血液，切換到測量狀態
                System_SetState (STATE_MEASURING);                
            }
            
        } else {           
            System_SetState (STATE_ERROR);
        }

        break;
    }

    case STATE_MEASURING: {
        static uint8_t measureStep = 1;
        static uint32_t stepStartTime = 0;
        static uint16_t evWidth1, tpl1, trd1, evWidth2, tpl2, trd2;
        static uint8_t parametersLoaded = 0;
        static uint32_t systemTick = 0;
        
        // 簡單的毫秒計數器（每次調用增加約10ms）
        systemTick += 10;
        
        // 第一次進入此狀態時，載入時序參數
        if (!parametersLoaded) {
            StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
            if (PARAM_GetTimingParameters(currentStripType, &tpl1, &trd1, &evWidth1, 1) &&
                PARAM_GetTimingParameters(currentStripType, &tpl2, &trd2, &evWidth2, 2)) {
                parametersLoaded = 1;
                stepStartTime = systemTick;
                measureStep = 1;
                printf("Measurement started for strip type: %s\r\n", StripType_GetName(currentStripType));
                printf("Timing params: EV1=%d, TPL1=%d, TRD1=%d, EV2=%d, TPL2=%d, TRD2=%d\r\n",
                       evWidth1, tpl1, trd1, evWidth2, tpl2, trd2);
                
                // 確保PWM開始輸出
                TIM_CtrlPWMOutputs(TIM1, ENABLE);
                printf("Step 1: PWM enabled for EVWIDTH1 (%d ms)\r\n", evWidth1);
            } else {
                printf("Failed to load timing parameters\r\n");
                System_SetState(STATE_ERROR);
                break;
            }
        }
        
        uint32_t elapsedTime = systemTick - stepStartTime;
        
        switch (measureStep) {
            case 1: // Step 1: PWM for EVWIDTH1
                if (elapsedTime >= evWidth1) {
                    // 禁用PWM，設置PB15為HIGH
                    TIM_CtrlPWMOutputs(TIM1, DISABLE);
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    GPIO_SetBits(GPIOB, GPIO_Pin_15);
                    
                    stepStartTime = systemTick;
                    measureStep = 2;
                    printf("Step 2: HIGH output for TPL1 (%d ms)\r\n", tpl1);
                }
                break;
                
            case 2: // Step 2: HIGH for TPL1
                if (elapsedTime >= tpl1) {
                    // 重新配置為PWM輸出
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    TIM_CtrlPWMOutputs(TIM1, ENABLE);
                    
                    stepStartTime = systemTick;
                    measureStep = 3;
                    printf("Step 3: PWM enabled for TRD1 (%d ms)\r\n", trd1);
                }
                break;
                
            case 3: // Step 3: PWM for TRD1
                if (elapsedTime >= trd1) {
                    // 檢查 tpl2 是否為 0
                    if (tpl2 == 0) {
                        // 如果 tpl2=0，跳過 Step 4~6，直接到 Step 7
                        stepStartTime = systemTick;
                        measureStep = 7;
                        printf("TPL2=0, skipping Steps 4-6, jumping to Step 7: Reading GLU_OUT ADC value\r\n");
                    } else {
                        // 否則繼續執行 Step 4
                        stepStartTime = systemTick;
                        measureStep = 4;
                        printf("Step 4: PWM continues for EVWIDTH2 (%d ms)\r\n", evWidth2);
                    }
                }
                break;
                
            case 4: // Step 4: PWM for EVWIDTH2
                if (elapsedTime >= evWidth2) {
                    // 禁用PWM，設置PB15為HIGH
                    TIM_CtrlPWMOutputs(TIM1, DISABLE);
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    GPIO_SetBits(GPIOB, GPIO_Pin_15);
                    
                    stepStartTime = systemTick;
                    measureStep = 5;
                    printf("Step 5: HIGH output for TPL2 (%d ms)\r\n", tpl2);
                }
                break;
                
            case 5: // Step 5: HIGH for TPL2
                if (elapsedTime >= tpl2) {
                    // 重新配置為PWM輸出
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    TIM_CtrlPWMOutputs(TIM1, ENABLE);
                    
                    stepStartTime = systemTick;
                    measureStep = 6;
                    printf("Step 6: PWM enabled for TRD2 (%d ms)\r\n", trd2);
                }
                break;
                
            case 6: // Step 6: PWM for TRD2
                if (elapsedTime >= trd2) {
                    stepStartTime = systemTick;
                    measureStep = 7;
                    printf("Step 7: Reading GLU_OUT ADC value\r\n");
                }
                break;
                
            case 7: // Step 7: 讀取GLU_OUT ADC值
                {
                    // 使用 GetMidADC 函數進行精確測量
                    // 連續取樣100次，排序後取中間20個的平均值
                    uint16_t adcValue = GetMidADC(ADC_Channel_4, 100, 20);
                    
                    // 將ADC值存入W_ADC變數，供後續血糖計算使用
                    W_ADC = adcValue;
                    
                    printf("Measurement complete! GLU_OUT ADC value: %d (stored in W_ADC)\r\n", adcValue);
                    
                    // 停止PWM輸出，設置為高電平
                    TIM_CtrlPWMOutputs(TIM1, DISABLE);
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    GPIO_SetBits(GPIOB, GPIO_Pin_15);
                    
                    // Step 8: 進入STATE_RESULT_READY
                    System_SetState(STATE_RESULT_READY);
                    
                    // 重置測量狀態變數
                    measureStep = 1;
                    parametersLoaded = 0;
                    stepStartTime = 0;
                    
                    printf("Step 8: Entering RESULT_READY state\r\n");
                }
                break;
                
            default:
                // 異常狀態，重置並進入錯誤狀態
                printf("Invalid measure step: %d\r\n", measureStep);
                measureStep = 1;
                parametersLoaded = 0;
                stepStartTime = 0;
                System_SetState(STATE_ERROR);
                break;
        }
        break;
    }

    case STATE_RESULT_READY: {
        static uint8_t calculationDone = 0;
        
        // 如果是新進入此狀態，重置標誌
        if (currentState != lastState) {
            calculationDone = 0;
        }
        
        if (!calculationDone) {
            // 處理結果準備好狀態
            printf("Measurement result is ready for processing\r\n");
            
            // 呼叫血糖計算函數，將W_ADC轉換為血糖值
            CalGlucose(W_ADC);
            printf("Blood glucose measurement completed: %d %s\r\n", 
                   wGlucose, Unit_GetSymbol((Unit_TypeDef)PARAM_GetByte(PARAM_MGDL)));
            
            // 標記計算已完成
            calculationDone = 1;
            
            printf("Blood glucose calculation completed. Ready for next measurement.\r\n");
        }
        
        break;
    }

    case STATE_ERROR:
        // 處理錯誤狀態
        // 實作部分後續再增加
        break;

    default:
        // 未知狀態，重置為空閒狀態
        System_SetState (STATE_IDLE);
        break;
    }
}

/* System_SetState 和 System_GetState 函數已移至 system_state.c */

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    Delay_Init();
    USART_Printf_Init (115200);
    USARTx_CFG (115200);
    DMA_INIT();
    printf ("SystemClk:%d\r\n", SystemCoreClock);
    printf ("This is an example\r\n");

    /* 初始化OPA2 */
    OPA2_Init();

    /* 初始化TIM1 PWM (WE_ENABLE, PB15, 20KHz) */
    TIM1_PWM_Init();

    /* 初始化參數表 */
    PARAM_Init();
    printf ("Parameter Table Initialized\r\n");

    /* 初始化UART通訊協議 */
    UART_Protocol_Init();
    printf ("UART Protocol Initialized\r\n");

    /* 初始化試片偵測相關設置 */
    STRIP_DETECT_Init();

    /* 初始化RTC */
    RTC_Config();
    printf("RTC Configured\r\n");

    /* 顯示一些基本系統資訊 */
    printf ("Model No: %d\r\n", PARAM_GetByte (PARAM_MODEL_NO));
    printf ("FW Version: %d.%d\r\n", PARAM_GetByte (PARAM_FW_NO) / 10, PARAM_GetByte (PARAM_FW_NO) % 10);
    printf ("Parameter Table Version: %d\r\n", PARAM_GetWord (PARAM_CODE_TABLE_V));
    printf ("Test Count: %d\r\n", PARAM_GetWord (PARAM_NOT));

    /* 顯示當前設定的試片類型 */
    StripType_TypeDef stripType = (StripType_TypeDef)PARAM_GetByte (PARAM_STRIP_TYPE);
    printf ("Strip Type: %s\r\n", StripType_GetName (stripType));

    /* 顯示當前設定的單位 */
    Unit_TypeDef unit = (Unit_TypeDef)PARAM_GetByte (PARAM_MGDL);
    printf ("Unit: %s\r\n", Unit_GetSymbol (unit));

    while (1) {
        // 處理UART協議資料
        UART_Protocol_Process();

        // 處理試片偵測相關任務
        STRIP_DETECT_Process();

        // 處理系統狀態機
        State_Process();

        // /* 測試環境可以保留以下程式碼，以便於偵錯 */
        // if (ring_buffer.RemainCount > 0)
        // {
        //     printf("UART Echo Test: %d bytes\r\n", ring_buffer.RemainCount);

        //  while (ring_buffer.RemainCount > 0)
        //  {
        //      uint8_t data = ring_buffer_pop();
        //      // 通過 USART2 將接收到的資料發送回去
        //      USART_SendData(USART2, data);
        //      // 等待發送完成
        //      while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        //  }

        //  printf("Echo Complete\r\n");
        // }

        Delay_Ms (5);  // 減少延遲時間以增加處理速度
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void USART2_IRQHandler (void) {
    if (USART_GetITStatus (USART2, USART_IT_IDLE) != RESET) {
        // IDLE
        uint16_t rxlen = (RX_BUFFER_LEN - USART_RX_CH->CNTR);
        uint8_t oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;

        USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

        DMA_Cmd (USART_RX_CH, DISABLE);
        DMA_SetCurrDataCounter (USART_RX_CH, RX_BUFFER_LEN);
        // Switch buffer
        USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
        DMA_Cmd (USART_RX_CH, ENABLE);

        USART_ReceiveData (USART2);  // clear IDLE flag
        ring_buffer_push_huge (USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);

        // 如果接收到資料，呼叫UART協議處理函數
        uint8_t i;
        for (i = 0; i < rxlen; i++) {
            UART2_Receive_Byte_ISR (USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
        }
    }

    // 檢查是否有接收到資料
    if (USART_GetITStatus (USART2, USART_IT_RXNE) != RESET) {
        // 讀取接收到的位元組並處理
        uint8_t data = USART_ReceiveData (USART2);
        UART2_Receive_Byte_ISR (data);
    }
}

/*********************************************************************
 * @fn      DMA1_Channel6_IRQHandler
 *
 * @brief   This function handles DMA1 Channel 6 global interrupt request.
 *
 * @return  none
 */
void DMA1_Channel6_IRQHandler (void) {
    uint16_t rxlen = RX_BUFFER_LEN;
    uint8_t oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;
    // FULL

    USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

    DMA_Cmd (USART_RX_CH, DISABLE);
    DMA_SetCurrDataCounter (USART_RX_CH, RX_BUFFER_LEN);
    // Switch buffer
    USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
    DMA_Cmd (USART_RX_CH, ENABLE);

    ring_buffer_push_huge (USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);

    // 處理DMA接收到的資料
    uint8_t i;
    for (i = 0; i < rxlen; i++) {
        UART2_Receive_Byte_ISR (USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
    }

    DMA_ClearITPendingBit (DMA1_IT_TC6);
}
