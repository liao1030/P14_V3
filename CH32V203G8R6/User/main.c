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
 *P14_V2.1 主程式
 *CH32V203G8R6 MCU初始化與主要功能實現
 */

#include "debug.h"
#include "param_table.h"
#include "param_test.h"
#include "param_store.h"
#include "uart_protocol.h"

/* 全局定義 */
#define LED_RED_PIN     GPIO_Pin_6
#define LED_GREEN_PIN   GPIO_Pin_7
#define KEY_1_PIN       GPIO_Pin_11
#define KEY_2_PIN       GPIO_Pin_10
#define T1_ENABLE_PIN   GPIO_Pin_8
#define WE_ENABLE_PIN   GPIO_Pin_15

/* UART相關緩衝區 */
#define UART_RX_BUF_SIZE       128
static uint8_t uart2_rx_buf[UART_RX_BUF_SIZE];
static uint16_t uart2_rx_len = 0;

/* 試片檢測相關變數與定義 */
#define T1_ADC_CHANNEL        ADC_Channel_6   // PA6對應的ADC通道
#define T1_LOW_THRESHOLD      500             // T1電壓低閾值 (約0V)
#define T1_HIGH_THRESHOLD     3000            // T1電壓高閾值 (約2.5V)

typedef enum {
    STRIP_STATE_NONE = 0,           // 無試片
    STRIP_STATE_INSERTED,           // 試片已插入
    STRIP_STATE_TYPE_DETECTED,      // 試片類型已檢測
    STRIP_STATE_READY,              // 試片準備測量
    STRIP_STATE_MEASURING           // 測量中
} StripState_t;

typedef struct {
    uint8_t strip_type;             // 試片類型
    StripState_t state;             // 試片狀態
    uint16_t t1_voltage;            // T1電極電壓(mV)
} StripInfo_t;

static StripInfo_t stripInfo = {0};

/* 函數聲明 */
void GPIO_Config(void);
void UART2_Config(void);
void Key_Process(void);
void ParamTable_Test(void);
void USART_Receive_Byte(void);
void Process_UART_Data(void);
void Timer2_Config(void);
void ADC_Config(void);
uint16_t ADC_GetT1Voltage(void);
void HandleStartT1Measurement(uint8_t *data, uint8_t length);
void HandleStripDetected(uint8_t *data, uint8_t length);
void ProcessStripTypeDetection(uint8_t initialType, uint8_t pinStatus);
void HandleProtocolCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    /* 中斷優先級分組設置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* 系統時鐘初始化 - 使用HSI時鐘源運行在16MHz */
    SystemCoreClockUpdate();
    
    /* 延遲初始化 */
    Delay_Init();
    
    /* GPIO 初始化 */
    GPIO_Config();
    
    /* UART2 初始化 - 與CH582F通訊 */
    UART2_Config();
    
    /* ADC 初始化 - 用於T1電極電壓測量 */
    ADC_Config();
    
    /* 定時器2配置 - 用於周期性任務 */
    Timer2_Config();
    
    /* 通訊協議初始化 */
    UART_Protocol_Init();
    
    /* 調試用UART初始化 */
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    /* 初始化參數存儲模塊 */
    printf("初始化參數存儲模塊...\r\n");
    if (PARAM_Init() != 0) {
        printf("參數存儲模塊初始化失敗！\r\n");
    } else {
        printf("參數存儲模塊初始化成功！\r\n");
    }
    
    /* 測試相關部分可以不執行 */
    /*
    printf("按任意鍵開始參數存儲測試...\r\n");
    USART_Receive_Byte();
    PARAM_TestAll();
    */
    
    printf("系統初始化完成，進入循環\r\n");
    printf("藍牙UART協議已啟用\r\n");
    
    while(1)
    {
        /* 處理按鍵 */
        Key_Process();
        
        /* 處理UART數據 */
        Process_UART_Data();
        
        /* 延遲 */
        Delay_Ms(10);
    }
}

/*********************************************************************
 * @fn      GPIO_Config
 *
 * @brief   GPIO初始化配置
 *
 * @return  none
 */
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* 使能GPIOA、GPIOB時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* LED控制引腳配置 - PB6(紅色)、PB7(綠色) */
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 按鍵輸入配置 - PB10、PB11 (按鍵輸入，上拉輸入) */
    GPIO_InitStructure.GPIO_Pin = KEY_1_PIN | KEY_2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* T1電極量測致能控制 - PA8 (低電平致能) */
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W電極電壓PWM控制 - PB15 */
    GPIO_InitStructure.GPIO_Pin = WE_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 溫度電阻輸入 - PA1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 試片T1電極電壓輸入 - PA6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 試片T3電極電壓輸入 - PB0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 生化反應結果輸入 - PA4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* W電極電壓輸入 - PA7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 電池電壓檢測 - PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 初始設定輸出狀態 */
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);    /* T1電極量測禁用(高電平) */
    GPIO_SetBits(GPIOB, WE_ENABLE_PIN);    /* W電極電壓PWM輸出致能(高電平) */
    GPIO_ResetBits(GPIOB, LED_RED_PIN);    /* 紅色LED關閉 */
    GPIO_ResetBits(GPIOB, LED_GREEN_PIN);  /* 綠色LED關閉 */
}

/*********************************************************************
 * @fn      UART2_Config
 *
 * @brief   UART2配置 - 與CH582F通訊
 *
 * @return  none
 */
void UART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    /* 使能GPIOA和UART2時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    /* UART2 TX(PA2)引腳配置 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2 RX(PA3)引腳配置 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* UART2參數配置 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    /* 配置UART2 */
    USART_Init(USART2, &USART_InitStructure);
    
    /* 配置UART2中斷 */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 啟用UART2接收中斷 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    /* 使能UART2 */
    USART_Cmd(USART2, ENABLE);
    
    /* 清空接收緩衝區 */
    uart2_rx_len = 0;
}

/*********************************************************************
 * @fn      Timer2_Config
 *
 * @brief   定時器2配置 - 用於周期性任務
 *
 * @return  none
 */
void Timer2_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    /* 使能定時器2時鐘 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    /* 基礎定時器配置 */
    TIM_TimeBaseStructure.TIM_Period = 100-1;           // 100ms計數週期
    TIM_TimeBaseStructure.TIM_Prescaler = 16000-1;      // 16MHz / 16000 = 1kHz計數頻率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* 使能定時器2中斷 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    /* 配置定時器2中斷 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 啟動定時器 */
    TIM_Cmd(TIM2, ENABLE);
}

/*********************************************************************
 * @fn      Process_UART_Data
 *
 * @brief   處理UART接收到的數據
 *
 * @return  none
 */
void Process_UART_Data(void)
{
    /* 如果有數據需要處理 */
    if (uart2_rx_len > 0) {
        /* 通過協議解析處理 */
        UART_Protocol_Parse(uart2_rx_buf, uart2_rx_len);
        
        /* 清空緩衝區 */
        uart2_rx_len = 0;
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   USART2中斷處理函數
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    /* 處理接收中斷 */
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* 讀取接收到的數據 */
        uint8_t rx_data = USART_ReceiveData(USART2);
        
        /* 存入緩衝區 */
        if (uart2_rx_len < UART_RX_BUF_SIZE) {
            uart2_rx_buf[uart2_rx_len++] = rx_data;
        }
        
        /* 清除中斷標誌 */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/*********************************************************************
 * @fn      TIM2_IRQHandler
 *
 * @brief   TIM2中斷處理函數
 *
 * @return  none
 */
void TIM2_IRQHandler(void)
{
    static uint8_t led_state = 0;
    static uint16_t counter = 0;
    
    /* 檢查更新中斷 */
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        /* 清除中斷標誌 */
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        /* 100ms任務 */
        counter++;
        
        /* 閃爍LED以指示系統正在運行 */
        if (counter % 5 == 0) {  // 500ms
            if (led_state) {
                GPIO_SetBits(GPIOB, LED_GREEN_PIN);
            } else {
                GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
            }
            led_state = !led_state;
        }
        
        /* 其他周期性任務 */
        if (counter >= 600) {  // 每分鐘執行一次
            counter = 0;
            /* 定期任務，例如自檢等 */
        }
    }
}

/*********************************************************************
 * @fn      ParamTable_Test
 *
 * @brief   測試參數表功能，列印當前參數設定並修改某參數
 *
 * @return  none
 */
void ParamTable_Test(void);

/*********************************************************************
 * @fn      Key_Process
 *
 * @brief   處理按鍵輸入
 *
 * @return  none
 */
void Key_Process(void)
{
    static uint8_t key1_last = 1;
    static uint8_t key2_last = 1;
    uint8_t key1_curr, key2_curr;
    
    /* 讀取當前按鍵狀態 */
    key1_curr = GPIO_ReadInputDataBit(GPIOB, KEY_1_PIN);
    key2_curr = GPIO_ReadInputDataBit(GPIOB, KEY_2_PIN);
    
    /* 檢測KEY1按下 (下降沿) */
    if (key1_last == 1 && key1_curr == 0) {
        printf("KEY1 pressed - Executing parameter test\r\n");
        PARAM_TestAll(); /* 執行參數存儲測試 */
    }
    
    /* 檢測KEY2按下 (下降沿) */
    if (key2_last == 1 && key2_curr == 0) {
        printf("KEY2 pressed - Toggling LED\r\n");
        
        /* 切換LED狀態 */
        if (GPIO_ReadOutputDataBit(GPIOB, LED_GREEN_PIN)) {
            GPIO_ResetBits(GPIOB, LED_GREEN_PIN);
        } else {
            GPIO_SetBits(GPIOB, LED_GREEN_PIN);
        }
    }
    
    /* 更新按鍵狀態 */
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

/*********************************************************************
 * @fn      ADC_Config
 *
 * @brief   ADC配置 - 用於T1電極電壓測量等
 *
 * @return  none
 */
void ADC_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};
    
    /* 使能ADC時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);  // ADC時鐘設為PCLK2的8分頻
    
    /* ADC配置 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    /* 啟用ADC */
    ADC_Cmd(ADC1, ENABLE);
    
    /* ADC校準 */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/*********************************************************************
 * @fn      ADC_GetT1Voltage
 *
 * @brief   獲取T1電極電壓
 *
 * @return  T1電極電壓值(ADC原始值)
 */
uint16_t ADC_GetT1Voltage(void)
{
    uint16_t adcValue;
    
    /* 啟用T1測量電路 */
    GPIO_ResetBits(GPIOA, T1_ENABLE_PIN);  // 低電平致能T1測量
    
    /* 配置ADC通道 */
    ADC_RegularChannelConfig(ADC1, T1_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);
    
    /* 啟動ADC轉換 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 等待轉換完成 */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    /* 讀取轉換結果 */
    adcValue = ADC_GetConversionValue(ADC1);
    
    /* 禁用T1測量電路 */
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);  // 高電平禁用T1測量
    
    return adcValue;
}

/*********************************************************************
 * @fn      HandleProtocolCommand
 *
 * @brief   處理接收到的協議命令
 *
 * @param   cmd_id - 命令ID
 *          data - 數據內容
 *          length - 數據長度
 *
 * @return  none
 */
void HandleProtocolCommand(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    switch(cmd_id)
    {
        case CMD_SYNC_TIME:
            UART_Protocol_HandleSyncTime(data, length);
            break;
            
        case CMD_REQUEST_STATUS:
            UART_Protocol_HandleRequestStatus(data, length);
            break;
            
        case CMD_SET_CODE_EVENT:
            UART_Protocol_HandleSetCodeEvent(data, length);
            break;
            
        case CMD_BLOOD_SAMPLE_CHECK:
            UART_Protocol_HandleBloodSampleCheck(data, length);
            break;
            
        case CMD_REQUEST_RESULT:
            UART_Protocol_HandleRequestResult(data, length);
            break;
            
        case CMD_REQUEST_RAW_DATA:
            UART_Protocol_HandleRequestRawData(data, length);
            break;
            
        case CMD_START_T1_MEASUREMENT:
            HandleStartT1Measurement(data, length);
            break;
            
        case CMD_STRIP_DETECTED:
            HandleStripDetected(data, length);
            break;
            
        default:
            /* 不支持的命令，返回錯誤 */
            UART_Protocol_SendErrorResponse(cmd_id, ERR_COMMAND_NOTSUPPORT);
            break;
    }
}

/*********************************************************************
 * @fn      HandleStartT1Measurement
 *
 * @brief   處理開始T1測量命令
 *
 * @param   data - 數據內容
 *          length - 數據長度
 *
 * @return  none
 */
void HandleStartT1Measurement(uint8_t *data, uint8_t length)
{
    T1_Measure_Result_t result;
    
    /* 進行T1電極電壓測量 */
    uint16_t adcValue = ADC_GetT1Voltage();
    
    /* 準備回應數據 */
    result.t1_voltage = adcValue;
    result.strip_detected = (stripInfo.state != STRIP_STATE_NONE) ? 1 : 0;
    
    /* 保存測量結果 */
    stripInfo.t1_voltage = adcValue;
    
    /* 發送測量結果 */
    uint8_t responseData[3];
    responseData[0] = (uint8_t)(result.t1_voltage & 0xFF);
    responseData[1] = (uint8_t)((result.t1_voltage >> 8) & 0xFF);
    responseData[2] = result.strip_detected;
    
    UART_Protocol_SendFrame(CMD_T1_MEASUREMENT_RESULT, responseData, 3);
}

/*********************************************************************
 * @fn      HandleStripDetected
 *
 * @brief   處理試片檢測通知
 *
 * @param   data - 數據內容
 *          length - 數據長度
 *
 * @return  none
 */
void HandleStripDetected(uint8_t *data, uint8_t length)
{
    uint8_t stripType;
    uint8_t pinStatus;
    
    /* 檢查數據長度 */
    if (length < 2) {
        UART_Protocol_SendErrorResponse(CMD_STRIP_DETECTED, ERR_DATA_FORMAT);
        return;
    }
    
    /* 解析數據 */
    stripType = data[0];
    pinStatus = data[1];
    
    /* 進行試片類型檢測與確認 */
    ProcessStripTypeDetection(stripType, pinStatus);
}

/*********************************************************************
 * @fn      ProcessStripTypeDetection
 *
 * @brief   處理試片類型檢測邏輯
 *
 * @param   initialType - 初步檢測的試片類型
 *          pinStatus - 接腳狀態
 *
 * @return  none
 */
void ProcessStripTypeDetection(uint8_t initialType, uint8_t pinStatus)
{
    uint8_t finalStripType = PROTOCOL_STRIP_UNKNOWN;
    uint8_t pin3Status = (pinStatus >> 1) & 0x01;
    uint8_t pin5Status = pinStatus & 0x01;
    
    /* 根據T1_OUT電壓進一步確認試片類型 */
    if (stripInfo.t1_voltage < T1_LOW_THRESHOLD) {
        /* T1電壓接近0V，對應第1腳與接地腳短路 */
        if (pin3Status == 1 && pin5Status == 0) {
            finalStripType = PROTOCOL_STRIP_U;  // 尿酸
        } else if (pin3Status == 0 && pin5Status == 1) {
            finalStripType = PROTOCOL_STRIP_TG;  // 三酸甘油脂
        }
    } else if (stripInfo.t1_voltage > T1_HIGH_THRESHOLD) {
        /* T1電壓接近2.5V，對應第1腳未與接地腳短路 */
        if (pin3Status == 1 && pin5Status == 0) {
            finalStripType = PROTOCOL_STRIP_GLV;  // 血糖(GLV試片)
        } else if (pin3Status == 1 && pin5Status == 1) {
            finalStripType = PROTOCOL_STRIP_C;  // 總膽固醇
        } else if (pin3Status == 0 && pin5Status == 0) {
            finalStripType = PROTOCOL_STRIP_GAV;  // 血糖(GAV試片)
        }
    }
    
    /* 更新試片信息 */
    stripInfo.strip_type = finalStripType;
    stripInfo.state = (finalStripType != PROTOCOL_STRIP_UNKNOWN) ? 
                       STRIP_STATE_TYPE_DETECTED : STRIP_STATE_INSERTED;
    
    /* 發送試片類型確認 */
    UART_Protocol_SendFrame(CMD_STRIP_TYPE_CONFIRM, &finalStripType, 1);
    
    /* 更新設備狀態 */
    if (finalStripType != PROTOCOL_STRIP_UNKNOWN) {
        /* 將試片類型轉換為測量類型 */
        uint16_t measureType;
        switch (finalStripType) {
            case PROTOCOL_STRIP_GLV: measureType = MEASURE_TYPE_GLV; break;
            case PROTOCOL_STRIP_U:   measureType = MEASURE_TYPE_U; break;
            case PROTOCOL_STRIP_C:   measureType = MEASURE_TYPE_C; break;
            case PROTOCOL_STRIP_TG:  measureType = MEASURE_TYPE_TG; break;
            case PROTOCOL_STRIP_GAV: measureType = MEASURE_TYPE_GAV; break;
            default: measureType = MEASURE_TYPE_UNKNOWN; break;
        }
        
        /* 更新並發送設備狀態 */
        Device_Status_t status;
        status.measure_type = measureType;
        status.strip_status = STRIP_STATE_TYPE_DETECTED;
        status.battery_voltage = 3000;  // 假設電池電壓3.0V
        status.temperature = 250;       // 假設溫度25.0度
        
        UART_Protocol_SendStatus(&status);
    }
}
