/**
 * @file strip_detection.c
 * @brief 試片類型判別功能實現
 */

#include "strip_detection.h"
#include "debug.h"
#include "ch32v20x.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_adc.h"
#include "ch32v20x_usart.h"

// 硬體接口定義
#define T1_IN_PIN                   GPIO_Pin_0    // PA0，連接試片第1腳
#define T1_OUT_PIN                  GPIO_Pin_6    // PA6，ADC6，測量T1_OUT電壓
#define T1_ENABLE_PIN               GPIO_Pin_8    // PA8，T1量測電路開關 (Low Enable)

// T1腳位ADC參考電壓閾值
#define T1_THRESHOLD_LOW            500           // ADC < 500 視為接近0V (12位ADC，範圍0-4095)
#define T1_THRESHOLD_HIGH           3000          // ADC > 3000 視為接近2.5V

// 全局變數
static volatile StripStatus_t stripStatus = STRIP_STATUS_NONE;
static volatile StripType_t stripType = STRIP_TYPE_DET_UNKNOWN;
static volatile uint8_t commandState = 0;         // UART命令處理狀態機

/**
 * @brief GPIO初始化函數
 */
static void Strip_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 啟用相關GPIO時鐘
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置T1_ENABLE_PIN為推挽輸出
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化時禁用T1量測電路
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);  // High = Disable
}

/**
 * @brief ADC初始化函數
 */
static void Strip_ADC_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 啟用ADC和GPIO時鐘
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置ADC轉換時鐘
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
    // 配置ADC輸入引腳 (T1_OUT)
    GPIO_InitStructure.GPIO_Pin = T1_OUT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 類比輸入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ADC1配置
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // 啟用ADC
    ADC_Cmd(ADC1, ENABLE);
    
    // ADC校準
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/**
 * @brief UART初始化函數
 */
static void Strip_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 啟用USART1時鐘和GPIO時鐘
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置USART1 Tx (PA9)為推挽輸出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置USART1 Rx (PA10)為浮動輸入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // USART1配置
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    // 啟用USART1接收中斷
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    // 配置USART1中斷
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 啟用USART1
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief 測量T1_OUT引腳的ADC值
 * @return ADC測量值 (0-4095)
 */
static uint16_t Measure_T1_OUT(void)
{
    // 選擇ADC通道並設置採樣時間
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);
    
    // 開始ADC轉換
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // 等待轉換完成
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    // 清除EOC標誌
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    
    // 返回轉換結果
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief 獲取腳位狀態
 * @return StripType_t 試片類型
 */
static StripType_t Detect_Strip_Type(void)
{
    uint8_t pin3_status, pin5_status;
    uint16_t t1_out_value;
    StripType_t detectedType = STRIP_TYPE_DET_UNKNOWN;
    
    // 讀取Strip_Detect_3腳位狀態
    pin3_status = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
    
    // 讀取Strip_Detect_5腳位狀態
    pin5_status = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
    
    // 啟用T1量測電路
    GPIO_ResetBits(GPIOA, T1_ENABLE_PIN);  // Low = Enable
    
    // 等待電路穩定
    Delay_Ms(5);
    
    // 測量T1_OUT電壓
    t1_out_value = Measure_T1_OUT();
    
    // 禁用T1量測電路
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);  // High = Disable
    
    printf("腳位狀態: Pin3=%d, Pin5=%d, T1_OUT=%d\r\n", pin3_status, pin5_status, t1_out_value);
    
    // 根據腳位狀態和T1_OUT電壓判斷試片類型
    if (pin3_status == 0 && pin5_status == 1 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_GLV;  // 血糖
    } else if (pin3_status == 0 && pin5_status == 1 && t1_out_value < T1_THRESHOLD_LOW) {
        detectedType = STRIP_TYPE_DET_U;    // 尿酸
    } else if (pin3_status == 0 && pin5_status == 0 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_C;    // 總膽固醇
    } else if (pin3_status == 1 && pin5_status == 0 && t1_out_value < T1_THRESHOLD_LOW) {
        detectedType = STRIP_TYPE_DET_TG;   // 三酸甘油脂
    } else if (pin3_status == 1 && pin5_status == 0 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_GAV;  // 血糖(GAV)
    } else {
        detectedType = STRIP_TYPE_DET_UNKNOWN;
    }
    
    return detectedType;
}

/**
 * @brief 發送試片類型結果給CH582F
 */
static void Send_Strip_Type_Result(StripType_t type)
{
    // 發送命令標頭
    USART_SendData(USART1, CMD_STRIP_TYPE_RESULT);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    
    // 發送類型代碼
    USART_SendData(USART1, (uint8_t)type);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    
    printf("已發送試片類型結果: %d\r\n", (int)type);
}

/**
 * @brief 初始化試片類型判別功能
 */
void Strip_Detection_Init(void)
{
    printf("初始化試片類型判別功能...\r\n");
    
    // 初始化GPIO
    Strip_GPIO_Init();
    
    // 初始化ADC
    Strip_ADC_Init();
    
    // 初始化UART
    Strip_UART_Init();
    
    // 初始化狀態
    stripStatus = STRIP_STATUS_NONE;
    stripType = STRIP_TYPE_DET_UNKNOWN;
    commandState = 0;
    
    printf("試片類型判別初始化完成\r\n");
}

/**
 * @brief 開始試片類型判別
 */
void Strip_StartTypeDetection(void)
{
    printf("開始試片類型判別...\r\n");
    
    // 檢測試片類型
    stripType = Detect_Strip_Type();
    
    // 更新狀態
    if (stripType != STRIP_TYPE_DET_UNKNOWN) {
        stripStatus = STRIP_STATUS_IDENTIFIED;
        
        // 將試片類型結果發送給CH582F
        Send_Strip_Type_Result(stripType);
        
        printf("試片類型判別結果: %d\r\n", (int)stripType);
    } else {
        stripStatus = STRIP_STATUS_INSERTED;
        printf("無法判別試片類型\r\n");
        
        // 發送未知類型結果
        Send_Strip_Type_Result(STRIP_TYPE_DET_UNKNOWN);
    }
}

/**
 * @brief 獲取試片狀態
 */
StripStatus_t Strip_GetStatus(void)
{
    return stripStatus;
}

/**
 * @brief 獲取試片類型
 */
StripType_t Strip_GetType(void)
{
    return stripType;
}

/**
 * @brief UART接收處理函數
 */
void Strip_UART_Process(uint8_t receivedByte)
{
    // 命令狀態機
    switch (commandState) {
        case 0:  // 等待命令
            if (receivedByte == CMD_WAKEUP_MCU) {
                commandState = 1;
                printf("收到喚醒命令\r\n");
            }
            break;
            
        case 1:  // 已收到喚醒命令
            if (receivedByte == CMD_STRIP_INSERTED) {
                commandState = 2;
                stripStatus = STRIP_STATUS_INSERTED;
                printf("收到試片插入通知\r\n");
            } else {
                commandState = 0;  // 重置命令狀態
            }
            break;
            
        case 2:  // 已收到試片插入通知
            if (receivedByte == CMD_REQUEST_STRIP_TYPE) {
                commandState = 0;  // 重置命令狀態
                printf("收到類型判別請求\r\n");
                
                // 開始試片類型判別
                Strip_StartTypeDetection();
            } else {
                commandState = 0;  // 重置命令狀態
            }
            break;
            
        default:
            commandState = 0;  // 重置命令狀態
            break;
    }
}

/**
 * @brief USART1中斷處理函數
 */
void USART1_IRQHandler(void)
{
    uint8_t receivedByte;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // 讀取接收到的數據
        receivedByte = USART_ReceiveData(USART1);
        
        // 處理接收到的數據
        Strip_UART_Process(receivedByte);
        
        // 清除中斷標誌
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}