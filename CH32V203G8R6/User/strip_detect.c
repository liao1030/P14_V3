/********************************** (C) COPYRIGHT *******************************
 * File Name          : strip_detect.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : 試片插入偵測與自動判別試片類型相關功能
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#include "strip_detect.h"
#include "uart_protocol.h"
#include "param_utils.h"
#include "param_code_table.h"

/* 私有變數定義 */
static StripDetectInfo_TypeDef stripInfo = {
    .state = STRIP_STATE_NONE,
    .type = STRIP_TYPE_MAX,
    .pin3Status = 1,
    .pin5Status = 1,
    .t1Voltage = 0.0f,
    .insertTime = 0,
    .detectionComplete = 0,
    .ackSent = 0,
    .batteryVoltage = 3000,     // 預設3000mV
    .battStatus = BATT_STATUS_NORMAL
};

/* 試片類型名稱由 param_table.c 中的 StripType_GetName 函數提供 */

/* 私有函數宣告 */
static void STRIP_DETECT_ADC_Init(void);
static uint16_t STRIP_DETECT_ReadADC(uint8_t channel);
static float STRIP_DETECT_GetT1Voltage(void);
static StripType_TypeDef STRIP_DETECT_DetermineStripType(void);
static void STRIP_DETECT_SendType(StripType_TypeDef type);

/*********************************************************************
 * @fn      STRIP_DETECT_Init
 *
 * @brief   初始化試片偵測相關設置
 *
 * @return  none
 */
void STRIP_DETECT_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* 啟用GPIO時鐘 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* 配置T1_ENABLE引腳 (PA8) - 用於啟用T1電極測量電路 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // 預設為高電平 (禁用T1測量)
    
    /* 配置T1_OUT引腳 (PA6) - ADC輸入，用於測量T1電極電壓 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置WE_ENABLE引腳 (PB15) - 控制W電極的PWM */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // 預設為高電平
    
    /* 初始化ADC */
    STRIP_DETECT_ADC_Init();
    
    printf("Strip Detection Module Initialized\r\n");
}

/*********************************************************************
 * @fn      STRIP_DETECT_ADC_Init
 *
 * @brief   初始化ADC用於試片類型判別
 *
 * @return  none
 */
static void STRIP_DETECT_ADC_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADC時鐘 = PCLK2/8
    
    ADC_DeInit(ADC1);
    
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_Cmd(ADC1, ENABLE);
    
    /* 校準ADC */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/*********************************************************************
 * @fn      STRIP_DETECT_ReadADC
 *
 * @brief   讀取ADC值
 *
 * @param   channel - ADC通道
 *
 * @return  ADC值 (0-4095)
 */
static uint16_t STRIP_DETECT_ReadADC(uint8_t channel)
{
    /* 配置ADC通道、採樣時間 */
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    /* 啟動ADC轉換 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 等待轉換完成 */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    /* 返回ADC值 */
    return ADC_GetConversionValue(ADC1);
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetT1Voltage
 *
 * @brief   獲取T1電極電壓
 *
 * @return  電壓值 (單位: V)
 */
static float STRIP_DETECT_GetT1Voltage(void)
{
    uint16_t adcValue = STRIP_DETECT_ReadADC(ADC_Channel_6); // PA6 對應 ADC_Channel_6
    float voltage = (float)adcValue * 3.3f / 4096.0f;
    return voltage;
}

/*********************************************************************
 * @fn      STRIP_DETECT_Process
 *
 * @brief   處理試片偵測相關任務
 *
 * @return  none
 */
void STRIP_DETECT_Process(void)
{
    /* 從CH582F接收到試片插入事件 */
    if (stripInfo.state == STRIP_STATE_INSERTED && !stripInfo.detectionComplete)
    {
        /* 啟用T1測量電路 */
        GPIO_ResetBits(GPIOA, GPIO_Pin_8); // 低電平啟用T1測量
        Delay_Ms(10); // 等待電路穩定
        
        /* 讀取T1電壓 */
        stripInfo.t1Voltage = STRIP_DETECT_GetT1Voltage();
        
        /* 判別試片類型 */
        stripInfo.type = STRIP_DETECT_DetermineStripType();
        stripInfo.state = STRIP_STATE_DETECTED;
        stripInfo.detectionComplete = 1;
        
        /* 關閉T1測量電路 */
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        
        /* 顯示判別結果 */
        printf("Strip Type Detected: %s (T1V=%.2fV, P3=%d, P5=%d)\r\n", 
               StripType_GetName(stripInfo.type),
               stripInfo.t1Voltage,
               stripInfo.pin3Status,
               stripInfo.pin5Status);
        
        /* 保存試片類型到參數表 */
        PARAM_SetByte(PARAM_STRIP_TYPE, (uint8_t)stripInfo.type);
        
        /* 發送試片類型到CH582F */
        STRIP_DETECT_SendType(stripInfo.type);
    }
}

/*********************************************************************
 * @fn      STRIP_DETECT_DetermineStripType
 *
 * @brief   根據腳位和電壓狀態判斷試片類型
 *
 * @return  試片類型
 */
static StripType_TypeDef STRIP_DETECT_DetermineStripType(void)
{
    /* 根據試片插入偵測文件的判斷邏輯 */
    /* P3 = Strip_Detect_3, P5 = Strip_Detect_5, T1V = T1_OUT電壓 */
    
    /* 判定電壓範圍 */
    int t1VoltageIsHigh = (stripInfo.t1Voltage > 2.0f); // 如果大於2V，視為接近2.5V
    
    /* 根據判斷邏輯確定試片類型 */
    if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 1 && t1VoltageIsHigh) {
        return STRIP_TYPE_GLV; // 血糖(GLV試片)
    } 
    else if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 1 && !t1VoltageIsHigh) {
        return STRIP_TYPE_U;   // 尿酸
    } 
    else if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 0 && t1VoltageIsHigh) {
        return STRIP_TYPE_C;   // 總膽固醇
    } 
    else if (stripInfo.pin3Status == 1 && stripInfo.pin5Status == 0 && !t1VoltageIsHigh) {
        return STRIP_TYPE_TG;  // 三酸甘油脂
    } 
    else if (stripInfo.pin3Status == 1 && stripInfo.pin5Status == 0 && t1VoltageIsHigh) {
        return STRIP_TYPE_GAV; // 血糖(GAV試片)
    }
    
    /* 無法確定類型 */
    return STRIP_TYPE_MAX;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SendType
 *
 * @brief   發送試片類型到CH582F
 *
 * @param   type - 試片類型
 *
 * @return  none
 */
static void STRIP_DETECT_SendType(StripType_TypeDef type)
{
    if (stripInfo.ackSent) return;
    
    /* 構建回應數據包 */
    uint8_t txBuf[5];
    txBuf[0] = 0xAA;                // 起始標記
    txBuf[1] = 0xA0;                // 命令: 試片類型回應 (與CH582F中的PROTOCOL_STRIP_TYPE_ACK一致)
    txBuf[2] = 0x01;                // 數據長度
    txBuf[3] = (uint8_t)type;       // 試片類型
    txBuf[4] = (txBuf[1] + txBuf[2] + txBuf[3]) % 256; // 校驗和
    
    /* 發送回應 */
    for(uint8_t i = 0; i < 5; i++) {
        USART_SendData(USART2, txBuf[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    /* 確認已回應 */
    stripInfo.ackSent = 1;
    
    printf("Sent Strip Type to BLE: %d\r\n", type);
}

/*********************************************************************
 * @fn      STRIP_DETECT_HandleInsertedEvent
 *
 * @brief   處理試片插入事件
 *
 * @return  none
 */
void STRIP_DETECT_HandleInsertedEvent(void)
{
    /* 收到CH582F的試片插入通知 */
    stripInfo.state = STRIP_STATE_INSERTED;
    stripInfo.detectionComplete = 0;
    stripInfo.ackSent = 0;
    stripInfo.insertTime = 0; // 目前不使用時間戳
    
    printf("Strip Inserted Event Received\r\n");
    
    /* 啟動T1電壓測量 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // 啟用T1測量
    
    /* 延遲一段時間讓T1電壓穩定 */
    Delay_Ms(50);
    
    /* 讀取T1電壓 */
    stripInfo.t1Voltage = STRIP_DETECT_GetT1Voltage();
    
    /* 判斷試片類型 */
    StripType_TypeDef detectedType = STRIP_DETECT_DetermineStripType();
    stripInfo.type = detectedType;
    
    /* 發送試片類型回應到藍牙模組 */
    UART_SendStripTypeAck(detectedType);
    
    /* 更新試片狀態為已檢測 */
    stripInfo.state = STRIP_STATE_DETECTED;
    stripInfo.detectionComplete = 1;
    
    printf("Strip Detected as type: %s\r\n", StripType_GetName(detectedType));
}

/*********************************************************************
 * @fn      STRIP_DETECT_IsInserted
 *
 * @brief   檢查試片是否已插入
 *
 * @return  0=未插入, 1=已插入
 */
uint8_t STRIP_DETECT_IsInserted(void)
{
    return (stripInfo.state == STRIP_STATE_INSERTED || 
            stripInfo.state == STRIP_STATE_DETECTED) ? 1 : 0;
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetStripType
 *
 * @brief   獲取試片類型
 *
 * @return  試片類型
 */
StripType_TypeDef STRIP_DETECT_GetStripType(void)
{
    return stripInfo.type;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetStripType
 *
 * @brief   設置試片類型 (通常用於外部直接設置)
 *
 * @param   type - 試片類型
 *
 * @return  none
 */
void STRIP_DETECT_SetStripType(StripType_TypeDef type)
{
    stripInfo.type = type;
    stripInfo.state = STRIP_STATE_DETECTED;
    
    /* 保存試片類型到參數表 */
    PARAM_SetByte(PARAM_STRIP_TYPE, (uint8_t)type);
    
    printf("Strip Type Manually Set: %s\r\n", StripType_GetName(type));
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetPinStatus
 *
 * @brief   設置腳位狀態 (接收自CH582F)
 *
 * @param   pin3 - 第3腳狀態 (0=低, 1=高)
 * @param   pin5 - 第5腳狀態 (0=低, 1=高)
 *
 * @return  none
 */
void STRIP_DETECT_SetPinStatus(uint8_t pin3, uint8_t pin5)
{
    stripInfo.pin3Status = pin3;
    stripInfo.pin5Status = pin5;
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetBatteryVoltage
 *
 * @brief   獲取電池電壓
 *
 * @return  電池電壓值(mV)
 */
uint16_t STRIP_DETECT_GetBatteryVoltage(void)
{
    return stripInfo.batteryVoltage;
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetBatteryStatus
 *
 * @brief   獲取電池狀態
 *
 * @return  電池狀態
 */
BatteryStatus_TypeDef STRIP_DETECT_GetBatteryStatus(void)
{
    return stripInfo.battStatus;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetBatteryInfo
 *
 * @brief   設置電池電壓信息
 *
 * @param   voltage - 電池電壓(mV)
 *
 * @return  none
 */
void STRIP_DETECT_SetBatteryInfo(uint16_t voltage)
{
    stripInfo.batteryVoltage = voltage;
    
    /* 根據電壓設置電池狀態 */
    if (voltage < 2500) { // 低於2.5V視為電量低
        stripInfo.battStatus = BATT_STATUS_LOW;
    } else {
        stripInfo.battStatus = BATT_STATUS_NORMAL;
    }
}
