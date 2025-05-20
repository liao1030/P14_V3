/********************************** (C) COPYRIGHT *******************************
 * File Name          : strip_detect.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : ԇƬ����ɜy�c�Ԅ��ЄeԇƬ������P����
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#include "strip_detect.h"
#include "uart_protocol.h"
#include "param_utils.h"
#include "param_code_table.h"

/* ˽��׃�����x */
static StripDetectInfo_TypeDef stripInfo = {
    .state = STRIP_STATE_NONE,
    .type = STRIP_TYPE_MAX,
    .pin3Status = 1,
    .pin5Status = 1,
    .t1Voltage = 0.0f,
    .insertTime = 0,
    .detectionComplete = 0,
    .ackSent = 0,
    .batteryVoltage = 3000,     // �A�O3000mV
    .battStatus = BATT_STATUS_NORMAL
};

/* ԇƬ������Q�� param_table.c �е� StripType_GetName �����ṩ */

/* ˽�к������� */
static void STRIP_DETECT_ADC_Init(void);
static uint16_t STRIP_DETECT_ReadADC(uint8_t channel);
static float STRIP_DETECT_GetT1Voltage(void);
static StripType_TypeDef STRIP_DETECT_DetermineStripType(void);
static void STRIP_DETECT_SendType(StripType_TypeDef type);

/*********************************************************************
 * @fn      STRIP_DETECT_Init
 *
 * @brief   ��ʼ��ԇƬ�ɜy���P�O��
 *
 * @return  none
 */
void STRIP_DETECT_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    /* ����GPIO�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* ����T1_ENABLE���_ (PA8) - ��춆���T1늘O�y���· */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // �A�O����ƽ (����T1�y��)
    
    /* ����T1_OUT���_ (PA6) - ADCݔ�룬��출y��T1늘O늉� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* ����WE_ENABLE���_ (PB15) - ����W늘O��PWM */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_15); // �A�O����ƽ
    
    /* ��ʼ��ADC */
    STRIP_DETECT_ADC_Init();
    
    printf("Strip Detection Module Initialized\r\n");
}

/*********************************************************************
 * @fn      STRIP_DETECT_ADC_Init
 *
 * @brief   ��ʼ��ADC���ԇƬ����Єe
 *
 * @return  none
 */
static void STRIP_DETECT_ADC_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADC�r� = PCLK2/8
    
    ADC_DeInit(ADC1);
    
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_Cmd(ADC1, ENABLE);
    
    /* У��ADC */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/*********************************************************************
 * @fn      STRIP_DETECT_ReadADC
 *
 * @brief   �xȡADCֵ
 *
 * @param   channel - ADCͨ��
 *
 * @return  ADCֵ (0-4095)
 */
static uint16_t STRIP_DETECT_ReadADC(uint8_t channel)
{
    /* ����ADCͨ������ӕr�g */
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    /* ����ADC�D�Q */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* �ȴ��D�Q��� */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    /* ����ADCֵ */
    return ADC_GetConversionValue(ADC1);
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetT1Voltage
 *
 * @brief   �@ȡT1늘O늉�
 *
 * @return  늉�ֵ (��λ: V)
 */
static float STRIP_DETECT_GetT1Voltage(void)
{
    uint16_t adcValue = STRIP_DETECT_ReadADC(ADC_Channel_6); // PA6 ���� ADC_Channel_6
    float voltage = (float)adcValue * 3.3f / 4096.0f;
    return voltage;
}

/*********************************************************************
 * @fn      STRIP_DETECT_Process
 *
 * @brief   ̎��ԇƬ�ɜy���P�΄�
 *
 * @return  none
 */
void STRIP_DETECT_Process(void)
{
    /* ��CH582F���յ�ԇƬ�����¼� */
    if (stripInfo.state == STRIP_STATE_INSERTED && !stripInfo.detectionComplete)
    {
        /* ����T1�y���· */
        GPIO_ResetBits(GPIOA, GPIO_Pin_8); // ���ƽ����T1�y��
        Delay_Ms(10); // �ȴ��·����
        
        /* �xȡT1늉� */
        stripInfo.t1Voltage = STRIP_DETECT_GetT1Voltage();
        
        /* �ЄeԇƬ��� */
        stripInfo.type = STRIP_DETECT_DetermineStripType();
        stripInfo.state = STRIP_STATE_DETECTED;
        stripInfo.detectionComplete = 1;
        
        /* �P�]T1�y���· */
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        
        /* �@ʾ�Єe�Y�� */
        printf("Strip Type Detected: %s (T1V=%.2fV, P3=%d, P5=%d)\r\n", 
               StripType_GetName(stripInfo.type),
               stripInfo.t1Voltage,
               stripInfo.pin3Status,
               stripInfo.pin5Status);
        
        /* ����ԇƬ��͵������� */
        PARAM_SetByte(PARAM_STRIP_TYPE, (uint8_t)stripInfo.type);
        
        /* �l��ԇƬ��͵�CH582F */
        STRIP_DETECT_SendType(stripInfo.type);
    }
}

/*********************************************************************
 * @fn      STRIP_DETECT_DetermineStripType
 *
 * @brief   �����_λ��늉���B�Д�ԇƬ���
 *
 * @return  ԇƬ���
 */
static StripType_TypeDef STRIP_DETECT_DetermineStripType(void)
{
    /* ����ԇƬ����ɜy�ļ����Д�߉݋ */
    /* P3 = Strip_Detect_3, P5 = Strip_Detect_5, T1V = T1_OUT늉� */
    
    /* �ж�늉����� */
    int t1VoltageIsHigh = (stripInfo.t1Voltage > 2.0f); // ������2V��ҕ��ӽ�2.5V
    
    /* �����Д�߉݋�_��ԇƬ��� */
    if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 1 && t1VoltageIsHigh) {
        return STRIP_TYPE_GLV; // Ѫ��(GLVԇƬ)
    } 
    else if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 1 && !t1VoltageIsHigh) {
        return STRIP_TYPE_U;   // ����
    } 
    else if (stripInfo.pin3Status == 0 && stripInfo.pin5Status == 0 && t1VoltageIsHigh) {
        return STRIP_TYPE_C;   // ��đ�̴�
    } 
    else if (stripInfo.pin3Status == 1 && stripInfo.pin5Status == 0 && !t1VoltageIsHigh) {
        return STRIP_TYPE_TG;  // �������֬
    } 
    else if (stripInfo.pin3Status == 1 && stripInfo.pin5Status == 0 && t1VoltageIsHigh) {
        return STRIP_TYPE_GAV; // Ѫ��(GAVԇƬ)
    }
    
    /* �o���_����� */
    return STRIP_TYPE_MAX;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SendType
 *
 * @brief   �l��ԇƬ��͵�CH582F
 *
 * @param   type - ԇƬ���
 *
 * @return  none
 */
static void STRIP_DETECT_SendType(StripType_TypeDef type)
{
    if (stripInfo.ackSent) return;
    
    /* �����ؑ������� */
    uint8_t txBuf[5];
    txBuf[0] = 0xAA;                // ��ʼ��ӛ
    txBuf[1] = 0xA0;                // ����: ԇƬ��ͻؑ� (�cCH582F�е�PROTOCOL_STRIP_TYPE_ACKһ��)
    txBuf[2] = 0x01;                // �����L��
    txBuf[3] = (uint8_t)type;       // ԇƬ���
    txBuf[4] = (txBuf[1] + txBuf[2] + txBuf[3]) % 256; // У��
    
    /* �l�ͻؑ� */
    for(uint8_t i = 0; i < 5; i++) {
        USART_SendData(USART2, txBuf[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    /* �_�J�ѻؑ� */
    stripInfo.ackSent = 1;
    
    printf("Sent Strip Type to BLE: %d\r\n", type);
}

/*********************************************************************
 * @fn      STRIP_DETECT_HandleInsertedEvent
 *
 * @brief   ̎��ԇƬ�����¼�
 *
 * @return  none
 */
void STRIP_DETECT_HandleInsertedEvent(void)
{
    /* �յ�CH582F��ԇƬ����֪ͨ */
    stripInfo.state = STRIP_STATE_INSERTED;
    stripInfo.detectionComplete = 0;
    stripInfo.ackSent = 0;
    stripInfo.insertTime = 0; // Ŀǰ��ʹ�Õr�g��
    
    printf("Strip Inserted Event Received\r\n");
    
    /* ����T1늉��y�� */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // ����T1�y��
    
    /* ���tһ�Εr�g׌T1늉����� */
    Delay_Ms(50);
    
    /* �xȡT1늉� */
    stripInfo.t1Voltage = STRIP_DETECT_GetT1Voltage();
    
    /* �Д�ԇƬ��� */
    StripType_TypeDef detectedType = STRIP_DETECT_DetermineStripType();
    stripInfo.type = detectedType;
    
    /* �l��ԇƬ��ͻؑ����{��ģ�M */
    UART_SendStripTypeAck(detectedType);
    
    /* ����ԇƬ��B���љz�y */
    stripInfo.state = STRIP_STATE_DETECTED;
    stripInfo.detectionComplete = 1;
    
    printf("Strip Detected as type: %s\r\n", StripType_GetName(detectedType));
}

/*********************************************************************
 * @fn      STRIP_DETECT_IsInserted
 *
 * @brief   �z��ԇƬ�Ƿ��Ѳ���
 *
 * @return  0=δ����, 1=�Ѳ���
 */
uint8_t STRIP_DETECT_IsInserted(void)
{
    return (stripInfo.state == STRIP_STATE_INSERTED || 
            stripInfo.state == STRIP_STATE_DETECTED) ? 1 : 0;
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetStripType
 *
 * @brief   �@ȡԇƬ���
 *
 * @return  ԇƬ���
 */
StripType_TypeDef STRIP_DETECT_GetStripType(void)
{
    return stripInfo.type;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetStripType
 *
 * @brief   �O��ԇƬ��� (ͨ������ⲿֱ���O��)
 *
 * @param   type - ԇƬ���
 *
 * @return  none
 */
void STRIP_DETECT_SetStripType(StripType_TypeDef type)
{
    stripInfo.type = type;
    stripInfo.state = STRIP_STATE_DETECTED;
    
    /* ����ԇƬ��͵������� */
    PARAM_SetByte(PARAM_STRIP_TYPE, (uint8_t)type);
    
    printf("Strip Type Manually Set: %s\r\n", StripType_GetName(type));
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetPinStatus
 *
 * @brief   �O���_λ��B (������CH582F)
 *
 * @param   pin3 - ��3�_��B (0=��, 1=��)
 * @param   pin5 - ��5�_��B (0=��, 1=��)
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
 * @brief   �@ȡ늳�늉�
 *
 * @return  늳�늉�ֵ(mV)
 */
uint16_t STRIP_DETECT_GetBatteryVoltage(void)
{
    return stripInfo.batteryVoltage;
}

/*********************************************************************
 * @fn      STRIP_DETECT_GetBatteryStatus
 *
 * @brief   �@ȡ늳ؠ�B
 *
 * @return  늳ؠ�B
 */
BatteryStatus_TypeDef STRIP_DETECT_GetBatteryStatus(void)
{
    return stripInfo.battStatus;
}

/*********************************************************************
 * @fn      STRIP_DETECT_SetBatteryInfo
 *
 * @brief   �O��늳�늉���Ϣ
 *
 * @param   voltage - 늳�늉�(mV)
 *
 * @return  none
 */
void STRIP_DETECT_SetBatteryInfo(uint16_t voltage)
{
    stripInfo.batteryVoltage = voltage;
    
    /* ����늉��O��늳ؠ�B */
    if (voltage < 2500) { // ���2.5Vҕ�������
        stripInfo.battStatus = BATT_STATUS_LOW;
    } else {
        stripInfo.battStatus = BATT_STATUS_NORMAL;
    }
}
