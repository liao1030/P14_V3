/**
 * @file strip_detection.c
 * @brief ԇƬ����Єe���܌��F
 */

#include "strip_detection.h"
#include "debug.h"
#include "ch32v20x.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_adc.h"
#include "ch32v20x_usart.h"

// Ӳ�w�ӿڶ��x
#define T1_IN_PIN                   GPIO_Pin_0    // PA0���B��ԇƬ��1�_
#define T1_OUT_PIN                  GPIO_Pin_6    // PA6��ADC6���y��T1_OUT늉�
#define T1_ENABLE_PIN               GPIO_Pin_8    // PA8��T1���y�·�_�P (Low Enable)

// T1�_λADC����늉��ֵ
#define T1_THRESHOLD_LOW            500           // ADC < 500 ҕ��ӽ�0V (12λADC������0-4095)
#define T1_THRESHOLD_HIGH           3000          // ADC > 3000 ҕ��ӽ�2.5V

// ȫ��׃��
static volatile StripStatus_t stripStatus = STRIP_STATUS_NONE;
static volatile StripType_t stripType = STRIP_TYPE_DET_UNKNOWN;
static volatile uint8_t commandState = 0;         // UART����̎���B�C

/**
 * @brief GPIO��ʼ������
 */
static void Strip_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // �������PGPIO�r�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    // ����T1_ENABLE_PIN������ݔ��
    GPIO_InitStructure.GPIO_Pin = T1_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ��ʼ���r����T1���y�·
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);  // High = Disable
}

/**
 * @brief ADC��ʼ������
 */
static void Strip_ADC_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ����ADC��GPIO�r�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    // ����ADC�D�Q�r�
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
    // ����ADCݔ�����_ (T1_OUT)
    GPIO_InitStructure.GPIO_Pin = T1_OUT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // �ݔ��ģʽ
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ADC1����
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // ����ADC
    ADC_Cmd(ADC1, ENABLE);
    
    // ADCУ��
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/**
 * @brief UART��ʼ������
 */
static void Strip_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // ����USART1�r犺�GPIO�r�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    // ����USART1 Tx (PA9)������ݔ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ����USART1 Rx (PA10)�鸡��ݔ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // USART1����
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    // ����USART1�����Д�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    // ����USART1�Д�
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ����USART1
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief �y��T1_OUT���_��ADCֵ
 * @return ADC�y��ֵ (0-4095)
 */
static uint16_t Measure_T1_OUT(void)
{
    // �x��ADCͨ���K�O�Ò�ӕr�g
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);
    
    // �_ʼADC�D�Q
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // �ȴ��D�Q���
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    // ���EOC���I
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    
    // �����D�Q�Y��
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief �@ȡ�_λ��B
 * @return StripType_t ԇƬ���
 */
static StripType_t Detect_Strip_Type(void)
{
    uint8_t pin3_status, pin5_status;
    uint16_t t1_out_value;
    StripType_t detectedType = STRIP_TYPE_DET_UNKNOWN;
    
    // �xȡStrip_Detect_3�_λ��B
    pin3_status = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
    
    // �xȡStrip_Detect_5�_λ��B
    pin5_status = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
    
    // ����T1���y�·
    GPIO_ResetBits(GPIOA, T1_ENABLE_PIN);  // Low = Enable
    
    // �ȴ��·����
    Delay_Ms(5);
    
    // �y��T1_OUT늉�
    t1_out_value = Measure_T1_OUT();
    
    // ����T1���y�·
    GPIO_SetBits(GPIOA, T1_ENABLE_PIN);  // High = Disable
    
    printf("�_λ��B: Pin3=%d, Pin5=%d, T1_OUT=%d\r\n", pin3_status, pin5_status, t1_out_value);
    
    // �����_λ��B��T1_OUT늉��Д�ԇƬ���
    if (pin3_status == 0 && pin5_status == 1 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_GLV;  // Ѫ��
    } else if (pin3_status == 0 && pin5_status == 1 && t1_out_value < T1_THRESHOLD_LOW) {
        detectedType = STRIP_TYPE_DET_U;    // ����
    } else if (pin3_status == 0 && pin5_status == 0 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_C;    // ��đ�̴�
    } else if (pin3_status == 1 && pin5_status == 0 && t1_out_value < T1_THRESHOLD_LOW) {
        detectedType = STRIP_TYPE_DET_TG;   // �������֬
    } else if (pin3_status == 1 && pin5_status == 0 && t1_out_value > T1_THRESHOLD_HIGH) {
        detectedType = STRIP_TYPE_DET_GAV;  // Ѫ��(GAV)
    } else {
        detectedType = STRIP_TYPE_DET_UNKNOWN;
    }
    
    return detectedType;
}

/**
 * @brief �l��ԇƬ��ͽY���oCH582F
 */
static void Send_Strip_Type_Result(StripType_t type)
{
    // �l��������^
    USART_SendData(USART1, CMD_STRIP_TYPE_RESULT);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    
    // �l����ʹ��a
    USART_SendData(USART1, (uint8_t)type);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    
    printf("�Ѱl��ԇƬ��ͽY��: %d\r\n", (int)type);
}

/**
 * @brief ��ʼ��ԇƬ����Єe����
 */
void Strip_Detection_Init(void)
{
    printf("��ʼ��ԇƬ����Єe����...\r\n");
    
    // ��ʼ��GPIO
    Strip_GPIO_Init();
    
    // ��ʼ��ADC
    Strip_ADC_Init();
    
    // ��ʼ��UART
    Strip_UART_Init();
    
    // ��ʼ����B
    stripStatus = STRIP_STATUS_NONE;
    stripType = STRIP_TYPE_DET_UNKNOWN;
    commandState = 0;
    
    printf("ԇƬ����Єe��ʼ�����\r\n");
}

/**
 * @brief �_ʼԇƬ����Єe
 */
void Strip_StartTypeDetection(void)
{
    printf("�_ʼԇƬ����Єe...\r\n");
    
    // �z�yԇƬ���
    stripType = Detect_Strip_Type();
    
    // ��� �B
    if (stripType != STRIP_TYPE_DET_UNKNOWN) {
        stripStatus = STRIP_STATUS_IDENTIFIED;
        
        // ��ԇƬ��ͽY���l�ͽoCH582F
        Send_Strip_Type_Result(stripType);
        
        printf("ԇƬ����Єe�Y��: %d\r\n", (int)stripType);
    } else {
        stripStatus = STRIP_STATUS_INSERTED;
        printf("�o���ЄeԇƬ���\r\n");
        
        // �l��δ֪��ͽY��
        Send_Strip_Type_Result(STRIP_TYPE_DET_UNKNOWN);
    }
}

/**
 * @brief �@ȡԇƬ��B
 */
StripStatus_t Strip_GetStatus(void)
{
    return stripStatus;
}

/**
 * @brief �@ȡԇƬ���
 */
StripType_t Strip_GetType(void)
{
    return stripType;
}

/**
 * @brief UART����̎����
 */
void Strip_UART_Process(uint8_t receivedByte)
{
    // �����B�C
    switch (commandState) {
        case 0:  // �ȴ�����
            if (receivedByte == CMD_WAKEUP_MCU) {
                commandState = 1;
                printf("�յ���������\r\n");
            }
            break;
            
        case 1:  // ���յ���������
            if (receivedByte == CMD_STRIP_INSERTED) {
                commandState = 2;
                stripStatus = STRIP_STATUS_INSERTED;
                printf("�յ�ԇƬ����֪ͨ\r\n");
            } else {
                commandState = 0;  // ���������B
            }
            break;
            
        case 2:  // ���յ�ԇƬ����֪ͨ
            if (receivedByte == CMD_REQUEST_STRIP_TYPE) {
                commandState = 0;  // ���������B
                printf("�յ�����ЄeՈ��\r\n");
                
                // �_ʼԇƬ����Єe
                Strip_StartTypeDetection();
            } else {
                commandState = 0;  // ���������B
            }
            break;
            
        default:
            commandState = 0;  // ���������B
            break;
    }
}

/**
 * @brief USART1�Д�̎����
 */
void USART1_IRQHandler(void)
{
    uint8_t receivedByte;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // �xȡ���յ��Ĕ���
        receivedByte = USART_ReceiveData(USART1);
        
        // ̎����յ��Ĕ���
        Strip_UART_Process(receivedByte);
        
        // ����Д����I
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}