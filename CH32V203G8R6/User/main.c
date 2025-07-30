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
#include "math.h"
#include "param_table.h"
#include "uart_protocol.h"
#include "system_state.h"  // ϵ�y��B���x
#include "rtc.h"
#include "ch32v20x_opa.h"
#include "ch32v20x_tim.h"
#include "strip_detect.h"
#include "Calculation.h"  // ����Ѫ��Ӌ�㺯�����^�n

void USART2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

/* ϵ�y��B���x������ system_state.h */

/* ϵ�y��B׃�������� system_state.c */

/* Ѫ�ǜy�����P׃�� */
uint16_t W_ADC = 0;  // ����늘OADCֵ�����Ѫ��Ӌ��

/* �ضȜy�����P׃�� */
uint16_t TM_ADC = 0;      // �������ADCֵ
float currentTemperature = 25.0;  // ��ǰ�ض�ֵ
uint8_t temperatureTestMode = 0;  // �ضȜyԇģʽ���I
float manualResistance = 0.0;     // �ք�ݔ������ֵ

/* �ض�У������ */
typedef struct {
    float offset;        // �ض�ƫ��У��ֵ
    float gain;          // �ض�����У��ֵ
    uint16_t checksum;   // У������У��
} TempCalibration_TypeDef;

static TempCalibration_TypeDef tempCalibration = {
    .offset = 0.0,
    .gain = 1.0,
    .checksum = 0
};

/* �������� */
void State_Process (void);
float Temperature_Measure_With_Calibration(void);
uint16_t Temperature_ReadADC(void);
float Temperature_ADC_To_Resistance(uint16_t adcValue);
float Temperature_Resistance_To_Celsius(float resistance);
float Temperature_Celsius_To_Resistance(float temperature);
void Temperature_Set_Test_Mode(uint8_t enable, float resistance);
void Temperature_Test_By_Temperature(uint8_t enable, float temperature);
uint8_t Temperature_Load_Calibration(void);
uint8_t Temperature_Save_Calibration(float offset, float gain);
uint8_t Temperature_Factory_Calibration(float referenceTemp, uint8_t calibrationType);
void Temperature_Process_Command(const char* command);

/*********************************************************************
 * @fn      GetMidADC
 *
 * @brief   ��ָ��ADCͨ���M���B�mȡ�ӣ�������ȡ���gֵ��ƽ��ֵ
 *
 * @param   channel - ADCͨ�� (����: ADC_Channel_4)
 * @param   sampleCount - ��ȡ�ӴΔ�
 * @param   midCount - ���gȡ�Ӕ��� (���Ӌ��ƽ��ֵ)
 *
 * @return  ���gֵ��ƽ��ֵ
 */
uint16_t GetMidADC(uint32_t channel, uint16_t sampleCount, uint16_t midCount)
{
    uint16_t *samples;
    uint32_t sum = 0;
    uint16_t i, j;
    uint16_t temp;
    uint16_t startIndex, endIndex;
    
    // �ӑB����ӛ���w��惦ȡ��ֵ
    samples = (uint16_t*)malloc(sampleCount * sizeof(uint16_t));
    if (samples == NULL) {
        printf("Memory allocation failed for ADC samples\r\n");
        return 0;
    }
    
    // �_�����gȡ�Ӕ��������^��ȡ�Ӕ���
    if (midCount > sampleCount) {
        midCount = sampleCount;
    }
    
    // ����ADCͨ��
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    // �B�mȡ��
    for (i = 0; i < sampleCount; i++) {
        // ����ADC�D�Q
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        
        // �ȴ��D�Q���
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        
        // �@ȡADCֵ
        samples[i] = ADC_GetConversionValue(ADC1);
        
        // С���t�Դ_������ȡ��
        Delay_Us(10);
    }
    
    // ʹ�Ú�������ȡ��ֵ�M������
    for (i = 0; i < sampleCount - 1; i++) {
        for (j = 0; j < sampleCount - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }
    
    // Ӌ�����gֵ�Ĺ���
    startIndex = (sampleCount - midCount) / 2;
    endIndex = startIndex + midCount;
    
    // Ӌ�����gֵ�Ŀ���
    for (i = startIndex; i < endIndex; i++) {
        sum += samples[i];
    }
    
    // ጷ�ӛ���w
    free(samples);
    
    // ����ƽ��ֵ
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
 * @brief   ��ʼ��OPA2���O��ݔ���������cؓ��ݔ��ˡ�
 *
 * @return  none
 */
void OPA2_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    OPA_InitTypeDef OPA_InitStructure = {0};

    /* ����GPIOA��OPA�ĕr� */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);

    /* ����OPA2���P��GPIO���_ */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    /* ����OPA2 */
    OPA_InitStructure.OPA_NUM = OPA2;     /* �x��OPA2 */
    OPA_InitStructure.PSEL = CHP1;        /* �x��OPA2����ݔ��˞�CHP1(PA7) */
    OPA_InitStructure.NSEL = CHN1;        /* �x��OPA2ؓ��ݔ��˞�CHN1(PA5) */
    OPA_InitStructure.Mode = OUT_IO_OUT1; /* OPA2ݔ��ͨ����OUT1(PA4) */
    OPA_Init (&OPA_InitStructure);

    /* ����OPA2 */
    OPA_Cmd (OPA2, ENABLE);

    printf ("OPA2 Initialized\r\n");
}

/*********************************************************************
 * @fn      TIM1_PWM_Init
 *
 * @brief   ��ʼ��TIM1������PB15(TIM1_CH3N)��20KHz PWMݔ�����A�O�׿ձ�100%��
 *
 * @return  none
 */
void TIM1_PWM_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure = {0};

    /* ����GPIOB��TIM1�ĕr� */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB | RCC_APB2Periph_TIM1, ENABLE);

    /* ����PB15 (TIM1_CH3N) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    /* Ӌ��PWM�l������ą���
     * ϵ�y�r� = 16MHz (����SystemClk�A�Oֵ)
     * Ŀ���l�� = 20KHz
     * �A���l = 1
     * Ӌ���L�� = 16000000 / 20000 / 1 = 800
     */
    TIM_TimeBaseStructure.TIM_Period = 800 - 1;                 /* Ӌ���L�ڣ�����20KHz */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;                    /* �A���l = 1 */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;                /* �r犲����l */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; /* ����Ӌ��ģʽ */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit (TIM1, &TIM_TimeBaseStructure);

    /* ����PWMͨ��3 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;               /* PWMģʽ2 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; /* ���û��aݔ�� */
    TIM_OCInitStructure.TIM_Pulse = 800 - 1;                        /* �A�O 0% �׿ձ� */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC3Init (TIM1, &TIM_OCInitStructure);

    /* �O��TIM1�����^�͔�·���� */
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    TIM_BDTRInitStructure.TIM_DeadTime = 0;              /* �o���^�r�g */
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable; /* ���Ô�·���� */
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig (TIM1, &TIM_BDTRInitStructure);

    /* �����A���d */
    TIM_OC3PreloadConfig (TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM1, ENABLE);

    /* ����TIM1 */
    TIM_Cmd (TIM1, ENABLE);
    TIM_CtrlPWMOutputs (TIM1, ENABLE);

    printf ("TIM1 PWM Initialized (20KHz, 100%% duty)\r\n");
}

/*********************************************************************
 * @fn      ADC1_Init
 *
 * @brief   ��ʼ��ADC1�����ö���ͨ����춲�ͬ�y��
 *
 * @return  none
 */
void ADC1_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* ����GPIOA��GPIOB��GPIOC��ADC1�r� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                          RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);

    /* ����ADC���P��GPIO���_��ģ�Mݔ�� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    
    /* PA4 - GLU_OUT (ADC_Channel_4) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* PC0 - TM_IN (ADC_Channel_10) - ���OTM_IN����PC0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ����ADC1 */
    ADC_DeInit(ADC1);
    
    /* ����ADC1 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ����ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* ADCУ�� */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    printf("ADC1 Initialized and Calibrated\r\n");
}

/*********************************************************************
 * @fn      Temperature_Set_Test_Mode
 *
 * @brief   �O�ÜضȜyԇģʽ
 *
 * @param   enable - 1:���Üyԇģʽ, 0:�P�]�yԇģʽ
 * @param   resistance - �ք�ݔ������ֵ (Ohm)
 *
 * @return  none
 */
void Temperature_Set_Test_Mode(uint8_t enable, float resistance)
{
    temperatureTestMode = enable;
    manualResistance = resistance;
    
    if (enable) {
        printf("Temperature test mode ENABLED with resistance: %.1f Ohm\r\n", resistance);
        // Ӌ��K�@ʾ�����Ĝض�
        float testTemp = Temperature_Resistance_To_Celsius(resistance);
        printf("Calculated temperature: %.2f��C\r\n", testTemp);
        currentTemperature = testTemp;
    } else {
        printf("Temperature test mode DISABLED\r\n");
    }
}

/*********************************************************************
 * @fn      Temperature_ReadADC
 *
 * @brief   �xȡ��������ADCֵ
 *
 * @return  ADCֵ
 */
uint16_t Temperature_ReadADC(void)
{
    /* ʹ��GetMidADC�����M�о��_�y�� */
    /* �B�mȡ��50�Σ�������ȡ���g10����ƽ��ֵ */
    uint16_t adcValue = GetMidADC(ADC_Channel_10, 50, 10);
    
    TM_ADC = adcValue;  // ���浽ȫ��׃��
    
    return adcValue;
}

/*********************************************************************
 * @fn      Temperature_ADC_To_Resistance
 *
 * @brief   ��ADCֵ�D�Q����������ֵ
 *
 * @param   adcValue - ADC�xȡֵ (0-4095)
 *
 * @return  ���������ֵ (Ohm)
 */
float Temperature_ADC_To_Resistance(uint16_t adcValue)
{
    /* �·������
     * VCC = 2.5V (V2P5)
     * R16 = 50K (�������)
     * TM1 = ������� (50K@25��C)
     * 
     * �։��·��V_TM_IN = VCC * R16 / (TM1 + R16)
     * ��ˣ�TM1 = R16 * (VCC - V_TM_IN) / V_TM_IN
     *       TM1 = R16 * (4095 - adcValue) / adcValue
     */
    
    const float VCC = 2.5;  // ����늉�
    const float R_REF = 50000.0;  // ������� R16 = 50K��
    const float ADC_MAX = 4095.0;  // 12λADC���ֵ
    
    if (adcValue == 0) {
        return 0.0;  // ��������e�`
    }
    
    float voltage = (adcValue / ADC_MAX) * VCC;
    float resistance = R_REF * (VCC - voltage) / voltage;
    
    return resistance;
}

/*********************************************************************
 * @fn      Temperature_Resistance_To_Celsius
 *
 * @brief   ʹ�ò�������������ֵ�D�Q��ض�
 *
 * @param   resistance - ���������ֵ (Ohm)
 *
 * @return  �ض�ֵ (�z�϶�)
 */
float Temperature_Resistance_To_Celsius(float resistance)
{
    /* 50K��������� (Beta = 3950K)
     * �ضȹ���: -10��C �� +50��C
     * ����ʽ: {�ض�, ��ֵ}
     */
    typedef struct {
        float temperature;
        float resistance;
    } TempResistancePair_TypeDef;
    
    static const TempResistancePair_TypeDef tempTable[] = {
        {-10.0, 277926.5},   // -10��C
        { -5.0, 212221.1},   //  -5��C
        {  0.0, 163500.0},   //   0��C
        {  5.0, 127036.5},   //   5��C
        { 10.0,  99504.5},   //  10��C
        { 15.0,  78540.0},   //  15��C
        { 20.0,  62447.6},   //  20��C
        { 25.0,  50000.0},   //  25��C
        { 30.0,  40301.0},   //  30��C
        { 35.0,  32690.9},   //  35��C
        { 37.0,  30117.4},   //  37��C (���w�ض�)
        { 40.0,  26679.8},   //  40��C
        { 45.0,  21901.5},   //  45��C
        { 50.0,  18080.0},   //  50��C
    };
    
    const uint8_t tableSize = sizeof(tempTable) / sizeof(tempTable[0]);
    
    if (resistance <= 0) {
        return -273.15;  // �oЧ��ֵ
    }
    
    // ������ֵ������񹠇���ʹ�ö��cֵ
    if (resistance >= tempTable[0].resistance) {
        return tempTable[0].temperature;  // ���-10��C
    }
    if (resistance <= tempTable[tableSize-1].resistance) {
        return tempTable[tableSize-1].temperature;  // ���50��C
    }
    
    // ���Բ�ֵ���
    for (uint8_t i = 0; i < tableSize - 1; i++) {
        if (resistance <= tempTable[i].resistance && resistance >= tempTable[i+1].resistance) {
            // �� tempTable[i] �� tempTable[i+1] ֮�g�M�о��Բ�ֵ
            float r1 = tempTable[i].resistance;
            float r2 = tempTable[i+1].resistance;
            float t1 = tempTable[i].temperature;
            float t2 = tempTable[i+1].temperature;
            
            // ���Բ�ֵ��ʽ: T = T1 + (R-R1)*(T2-T1)/(R2-R1)
            float temperature = t1 + (resistance - r1) * (t2 - t1) / (r2 - r1);
            
            return temperature;
        }
    }
    
    // ����]���ҵ����m�Ĺ���������25��C�����A�Oֵ
    return 25.0;
}

/*********************************************************************
 * @fn      Temperature_Celsius_To_Resistance
 *
 * @brief   ���ض��D�Q�錦���ğ��������ֵ (��출yԇģʽ)
 *
 * @param   temperature - �ض�ֵ (�z�϶�)
 *
 * @return  ���������ֵ (Ohm)
 */
float Temperature_Celsius_To_Resistance(float temperature)
{
    /* ʹ����ͬ�Ĳ���M�з������ */
    typedef struct {
        float temperature;
        float resistance;
    } TempResistancePair_TypeDef;
    
    static const TempResistancePair_TypeDef tempTable[] = {
        {-10.0, 277926.5},   // -10��C
        { -5.0, 212221.1},   //  -5��C
        {  0.0, 163500.0},   //   0��C
        {  5.0, 127036.5},   //   5��C
        { 10.0,  99504.5},   //  10��C
        { 15.0,  78540.0},   //  15��C
        { 20.0,  62447.6},   //  20��C
        { 25.0,  50000.0},   //  25��C
        { 30.0,  40301.0},   //  30��C
        { 35.0,  32690.9},   //  35��C
        { 37.0,  30117.4},   //  37��C
        { 40.0,  26679.8},   //  40��C
        { 45.0,  21901.5},   //  45��C
        { 50.0,  18080.0},   //  50��C
    };
    
    const uint8_t tableSize = sizeof(tempTable) / sizeof(tempTable[0]);
    
    // ����ضȳ�����񹠇���ʹ�ö��cֵ
    if (temperature <= tempTable[0].temperature) {
        return tempTable[0].resistance;  // ���-10��C
    }
    if (temperature >= tempTable[tableSize-1].temperature) {
        return tempTable[tableSize-1].resistance;  // ���50��C
    }
    
    // ���Բ�ֵ���
    for (uint8_t i = 0; i < tableSize - 1; i++) {
        if (temperature >= tempTable[i].temperature && temperature <= tempTable[i+1].temperature) {
            // �� tempTable[i] �� tempTable[i+1] ֮�g�M�о��Բ�ֵ
            float t1 = tempTable[i].temperature;
            float t2 = tempTable[i+1].temperature;
            float r1 = tempTable[i].resistance;
            float r2 = tempTable[i+1].resistance;
            
            // ���Բ�ֵ��ʽ: R = R1 + (T-T1)*(R2-R1)/(T2-T1)
            float resistance = r1 + (temperature - t1) * (r2 - r1) / (t2 - t1);
            
            return resistance;
        }
    }
    
    // ����]���ҵ����m�Ĺ���������25��C��������ֵ
    return 50000.0;
}

/*********************************************************************
 * @fn      Temperature_Test_By_Temperature
 *
 * @brief   �����ض�ֵ�O���yԇģʽ (�Ԅ�Ӌ�㌦�����ֵ)
 *
 * @param   enable - 1:���Üyԇģʽ, 0:�P�]�yԇģʽ  
 * @param   temperature - Ŀ�˜ض�ֵ (�z�϶�)
 *
 * @return  none
 */
void Temperature_Test_By_Temperature(uint8_t enable, float temperature)
{
    if (enable) {
        float resistance = Temperature_Celsius_To_Resistance(temperature);
        Temperature_Set_Test_Mode(1, resistance);
        printf("Temperature test mode set to %.1f��C (resistance: %.1f Ohm)\r\n", 
               temperature, resistance);
    } else {
        Temperature_Set_Test_Mode(0, 0);
    }
}

/*********************************************************************
 * @fn      Temperature_Measure_With_Calibration
 *
 * @brief   ���Ў�У���ĜضȜy������
 *
 * @return  У����Ĝض�ֵ (�z�϶�)
 */
float Temperature_Measure_With_Calibration(void)
{
    float rawTemperature;
    
    if (temperatureTestMode) {
        // �yԇģʽ��ʹ���ք�ݔ������ֵ
        rawTemperature = Temperature_Resistance_To_Celsius(manualResistance);
        printf("Test Mode - Manual Resistance: %.1f Ohm, Raw Temp: %.2f��C\r\n", 
               manualResistance, rawTemperature);
    } else {
        // ����ģʽ���xȡADCֵ�K�D�Q
        uint16_t adcValue = Temperature_ReadADC();
        float resistance = Temperature_ADC_To_Resistance(adcValue);
        rawTemperature = Temperature_Resistance_To_Celsius(resistance);
        
        printf("Normal Mode - ADC: %d, Resistance: %.1f Ohm, Raw Temp: %.2f��C\r\n", 
               adcValue, resistance, rawTemperature);
    }
    
    // ����У��
    float calibratedTemperature = (rawTemperature * tempCalibration.gain) + tempCalibration.offset;
    
    // ���浽ȫ��׃��
    currentTemperature = calibratedTemperature;
    
    printf("Calibrated Temperature: %.2f��C (offset: %.2f, gain: %.3f)\r\n", 
           calibratedTemperature, tempCalibration.offset, tempCalibration.gain);
    
    return calibratedTemperature;
}

/*********************************************************************
 * @fn      Temperature_Load_Calibration
 *
 * @brief   ��Flash�d��ض�У������
 *
 * @return  1:�ɹ�, 0:ʧ��
 */
uint8_t Temperature_Load_Calibration(void)
{
    // Flash��ַ���x (ʹ��δʹ�õ�Flash�^��)
    #define TEMP_CALIB_FLASH_ADDR   0x0800F800  // ���Oʹ�õ����ڶ������
    
    uint32_t *flashData = (uint32_t*)TEMP_CALIB_FLASH_ADDR;
    
    // �xȡУ������
    memcpy(&tempCalibration, flashData, sizeof(TempCalibration_TypeDef));
    
    // Ӌ��У��
    uint16_t calculatedChecksum = 0;
    uint8_t *data = (uint8_t*)&tempCalibration;
    for (int i = 0; i < sizeof(TempCalibration_TypeDef) - sizeof(uint16_t); i++) {
        calculatedChecksum += data[i];
    }
    
    // ��CУ��
    if (tempCalibration.checksum == calculatedChecksum) {
        printf("Temperature calibration loaded: offset=%.3f, gain=%.3f\r\n", 
               tempCalibration.offset, tempCalibration.gain);
        return 1;
    } else {
        // У���e�`��ʹ���A�Oֵ
        printf("Temperature calibration checksum error, using defaults\r\n");
        tempCalibration.offset = 0.0;
        tempCalibration.gain = 1.0;
        tempCalibration.checksum = 0;
        return 0;
    }
}

/*********************************************************************
 * @fn      Temperature_Save_Calibration
 *
 * @brief   ���ض�У���������浽Flash
 *
 * @param   offset - �ض�ƫ��У��ֵ
 * @param   gain - �ض�����У��ֵ
 *
 * @return  1:�ɹ�, 0:ʧ��
 */
uint8_t Temperature_Save_Calibration(float offset, float gain)
{
    #define TEMP_CALIB_FLASH_ADDR   0x0800F800
    
    // ����У������
    tempCalibration.offset = offset;
    tempCalibration.gain = gain;
    
    // Ӌ��У��
    tempCalibration.checksum = 0;
    uint8_t *data = (uint8_t*)&tempCalibration;
    for (int i = 0; i < sizeof(TempCalibration_TypeDef) - sizeof(uint16_t); i++) {
        tempCalibration.checksum += data[i];
    }
    
    // ���iFlash
    FLASH_Unlock();
    
    // �������
    FLASH_ErasePage(TEMP_CALIB_FLASH_ADDR);
    
    // ����У������
    uint32_t *sourceData = (uint32_t*)&tempCalibration;
    uint32_t writeAddr = TEMP_CALIB_FLASH_ADDR;
    
    for (int i = 0; i < (sizeof(TempCalibration_TypeDef) + 3) / 4; i++) {
        if (FLASH_ProgramWord(writeAddr, sourceData[i]) != FLASH_COMPLETE) {
            FLASH_Lock();
            printf("Temperature calibration save failed at word %d\r\n", i);
            return 0;
        }
        writeAddr += 4;
    }
    
    // �i��Flash
    FLASH_Lock();
    
    printf("Temperature calibration saved: offset=%.3f, gain=%.3f, checksum=0x%04X\r\n", 
           offset, gain, tempCalibration.checksum);
    
    return 1;
}

/*********************************************************************
 * @fn      Temperature_Factory_Calibration
 *
 * @brief   ���S�ض�У������
 *
 * @param   referenceTemp - �����ض�ֵ (�z�϶�)
 * @param   calibrationType - У����� (0:ƫ��У��, 1:����У��)
 *
 * @return  1:�ɹ�, 0:ʧ��
 */
uint8_t Temperature_Factory_Calibration(float referenceTemp, uint8_t calibrationType)
{
    // �y����ǰ�ض� (��ʹ��У��)
    float tempOffset = tempCalibration.offset;
    float tempGain = tempCalibration.gain;
    
    // ���rȡ��У���M��ԭʼ�y��
    tempCalibration.offset = 0.0;
    tempCalibration.gain = 1.0;
    
    float measuredTemp = Temperature_Measure_With_Calibration();
    
    // �֏�ԭʼУ��ֵ
    tempCalibration.offset = tempOffset;
    tempCalibration.gain = tempGain;
    
    if (calibrationType == 0) {
        // ƫ��У�� (һ�cУ��)
        float newOffset = referenceTemp - measuredTemp;
        printf("Factory Offset Calibration:\r\n");
        printf("  Reference: %.2f��C\r\n", referenceTemp);
        printf("  Measured: %.2f��C\r\n", measuredTemp);
        printf("  Calculated Offset: %.3f��C\r\n", newOffset);
        
        // �����µ�ƫ��ֵ
        return Temperature_Save_Calibration(newOffset, tempCalibration.gain);
        
    } else if (calibrationType == 1) {
        // ����У�� (��Ҫ����ƫ��У��)
        float adjustedMeasured = measuredTemp + tempCalibration.offset;
        float newGain = referenceTemp / adjustedMeasured;
        
        printf("Factory Gain Calibration:\r\n");
        printf("  Reference: %.2f��C\r\n", referenceTemp);
        printf("  Measured: %.2f��C\r\n", measuredTemp);
        printf("  Adjusted: %.2f��C\r\n", adjustedMeasured);
        printf("  Calculated Gain: %.4f\r\n", newGain);
        
        // �����µ�����ֵ
        return Temperature_Save_Calibration(tempCalibration.offset, newGain);
    }
    
    return 0;
}

/*********************************************************************
 * @fn      Temperature_Process_Command
 *
 * @brief   ̎��ض����P������
 *
 * @param   command - �����ִ�
 *
 * @return  none
 */
void Temperature_Process_Command(const char* command)
{
    if (strncmp(command, "TEMP_TEST_R", 11) == 0) {
        // ��ʽ: TEMP_TEST_R <resistance>
        float resistance;
        if (sscanf(command, "TEMP_TEST_R %f", &resistance) == 1) {
            Temperature_Set_Test_Mode(1, resistance);
        } else {
            printf("Usage: TEMP_TEST_R <resistance_ohm>\r\n");
            printf("Example: TEMP_TEST_R 50000 (for ~25��C)\r\n");
        }
    }
    else if (strncmp(command, "TEMP_TEST_T", 11) == 0) {
        // ��ʽ: TEMP_TEST_T <temperature>
        float temperature;
        if (sscanf(command, "TEMP_TEST_T %f", &temperature) == 1) {
            Temperature_Test_By_Temperature(1, temperature);
        } else {
            printf("Usage: TEMP_TEST_T <temperature_celsius>\r\n");
            printf("Example: TEMP_TEST_T 25.0 (for 25��C)\r\n");
        }
    }
    else if (strcmp(command, "TEMP_NORMAL") == 0) {
        Temperature_Set_Test_Mode(0, 0);
    }
    else if (strcmp(command, "TEMP_MEASURE") == 0) {
        Temperature_Measure_With_Calibration();
    }
    else if (strncmp(command, "TEMP_CAL_OFFSET", 15) == 0) {
        // ��ʽ: TEMP_CAL_OFFSET <reference_temp>
        float refTemp;
        if (sscanf(command, "TEMP_CAL_OFFSET %f", &refTemp) == 1) {
            if (Temperature_Factory_Calibration(refTemp, 0)) {
                printf("Offset calibration completed\r\n");
            } else {
                printf("Offset calibration failed\r\n");
            }
        } else {
            printf("Usage: TEMP_CAL_OFFSET <reference_temperature>\r\n");
            printf("Example: TEMP_CAL_OFFSET 25.0\r\n");
        }
    }
    else if (strncmp(command, "TEMP_CAL_GAIN", 13) == 0) {
        // ��ʽ: TEMP_CAL_GAIN <reference_temp>
        float refTemp;
        if (sscanf(command, "TEMP_CAL_GAIN %f", &refTemp) == 1) {
            if (Temperature_Factory_Calibration(refTemp, 1)) {
                printf("Gain calibration completed\r\n");
            } else {
                printf("Gain calibration failed\r\n");
            }
        } else {
            printf("Usage: TEMP_CAL_GAIN <reference_temperature>\r\n");
            printf("Example: TEMP_CAL_GAIN 37.0\r\n");
        }
    }
    else if (strcmp(command, "TEMP_CAL_RESET") == 0) {
        if (Temperature_Save_Calibration(0.0, 1.0)) {
            printf("Temperature calibration reset to default\r\n");
        } else {
            printf("Failed to reset temperature calibration\r\n");
        }
    }
    else if (strcmp(command, "TEMP_CAL_INFO") == 0) {
        printf("Temperature Calibration Info:\r\n");
        printf("  Offset: %.3f��C\r\n", tempCalibration.offset);
        printf("  Gain: %.4f\r\n", tempCalibration.gain);
        printf("  Checksum: 0x%04X\r\n", tempCalibration.checksum);
        printf("  Test Mode: %s\r\n", temperatureTestMode ? "ENABLED" : "DISABLED");
        if (temperatureTestMode) {
            printf("  Manual Resistance: %.1f Ohm\r\n", manualResistance);
            printf("  Calculated Temperature: %.1f��C\r\n", currentTemperature);
        }
    }
    else if (strcmp(command, "TEMP_TABLE") == 0) {
        printf("Temperature-Resistance Reference Table:\r\n");
        printf("Temp(��C)  Resistance(Ohm)\r\n");
        printf("  -10       277927\r\n");
        printf("   -5       212221\r\n");
        printf("    0       163500\r\n");
        printf("    5       127037\r\n");
        printf("   10        99505\r\n");
        printf("   15        78540\r\n");
        printf("   20        62448\r\n");
        printf("   25        50000\r\n");
        printf("   30        40301\r\n");
        printf("   35        32691\r\n");
        printf("   37        30117\r\n");
        printf("   40        26680\r\n");
        printf("   45        21902\r\n");
        printf("   50        18080\r\n");
    }
    else {
        printf("Unknown temperature command: %s\r\n", command);
        printf("Available commands:\r\n");
        printf("  TEMP_TEST_R <resistance>   - Set test mode with manual resistance\r\n");
        printf("  TEMP_TEST_T <temperature>  - Set test mode with target temperature\r\n");
        printf("  TEMP_NORMAL               - Return to normal measurement mode\r\n");
        printf("  TEMP_MEASURE              - Perform temperature measurement\r\n");
        printf("  TEMP_CAL_OFFSET <temp>    - Factory offset calibration\r\n");
        printf("  TEMP_CAL_GAIN <temp>      - Factory gain calibration\r\n");
        printf("  TEMP_CAL_RESET            - Reset calibration to default\r\n");
        printf("  TEMP_CAL_INFO             - Show calibration information\r\n");
        printf("  TEMP_TABLE                - Show temperature-resistance table\r\n");
    }
}

/*********************************************************************
 * @fn      State_Process
 *
 * @brief   ̎��ϵ�y��B�C
 *
 * @return  none
 */
void State_Process (void) {
    static SystemState_TypeDef lastState = STATE_IDLE;
    static uint8_t temperatureMeasured = 0;  // �ضȜy�����I
    SystemState_TypeDef currentState = System_GetState();
    
    // �����B�l����׃���������P���I
    if (currentState != lastState) {
        // �������Р�B���P���o�B׃��
        if (lastState == STATE_RESULT_READY) {
            // ���x�_ RESULT_READY ��B�r�������M��������
            printf("Leaving RESULT_READY state\r\n");
        }
        
        // ���M��� �B�r�����ÜضȜy�����I
        if (currentState == STATE_STRIP_DETECTED || 
            currentState == STATE_STRIP_VALIDATION ||
            currentState == STATE_PARAMETER_SETUP) {
            temperatureMeasured = 0;
        }
        
        lastState = currentState;
    }
    
    switch (currentState) {
    case STATE_IDLE:
        // �ڿ��f��B�z���Ƿ���ԇƬ����
        // ���ÜضȜy�����I
        temperatureMeasured = 0;
        break;

    case STATE_STRIP_DETECTED:
        // ̎��z�y��ԇƬ��B
        // һ���z�y��ԇƬ��ͣ������M�МضȜy��
        if (!temperatureMeasured) {
            StripType_TypeDef detectedStripType = STRIP_DETECT_GetStripType();
            if (detectedStripType != STRIP_TYPE_U && detectedStripType < STRIP_TYPE_MAX) {
                printf("Strip detected: %s, measuring temperature...\r\n", 
                       StripType_GetName(detectedStripType));
                
                // ���МضȜy��
                float measuredTemp = Temperature_Measure_With_Calibration();
                printf("Temperature measurement for %s: %.1f��C\r\n", 
                       StripType_GetName(detectedStripType), measuredTemp);
                
                temperatureMeasured = 1;  // ��ӛ�ض��ќy��
            }
        }
        break;

    case STATE_STRIP_VALIDATION:
        // ̎��ԇƬ��C�A��
        // ���߀δ�y���ضȣ��ڴ��A��Ҳ���Ԝy��
        if (!temperatureMeasured) {
            StripType_TypeDef validatedStripType = STRIP_DETECT_GetStripType();
            if (validatedStripType != STRIP_TYPE_U && validatedStripType < STRIP_TYPE_MAX) {
                printf("Strip validation phase, measuring temperature...\r\n");
                
                float measuredTemp = Temperature_Measure_With_Calibration();
                printf("Temperature measurement during validation: %.1f��C\r\n", measuredTemp);
                
                temperatureMeasured = 1;
            }
        }
        break;

    case STATE_PARAMETER_SETUP:
        // ̎�텢���O���A��
        // �_���څ����O�Õr���Мض��Y��
        if (!temperatureMeasured) {
            printf("Parameter setup phase, ensuring temperature is measured...\r\n");
            
            float measuredTemp = Temperature_Measure_With_Calibration();
            printf("Temperature measurement during parameter setup: %.1f��C\r\n", measuredTemp);
            
            temperatureMeasured = 1;
        }
        break;

    case STATE_WAIT_FOR_BLOOD: {
        // ̎��ȴ�ѪҺ�����A��
        uint16_t adcValue, bloodInThreshold;
        uint16_t ndl, udl;  // δʹ�ã�����Ҫ�� PARAM_GetStripParameters ������

        // �� PA4/ADC4 �xȡ GLU_OUT �� ADC ֵ
        // ���� ADC ͨ�� 4 (PA4)
        ADC_RegularChannelConfig (ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);

        // ���� ADC �D�Q
        ADC_SoftwareStartConvCmd (ADC1, ENABLE);

        // �ȴ��D�Q���
        while (!ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC))
            ;

        // �@ȡADCֵ
        adcValue = ADC_GetConversionValue (ADC1);

        // �@ȡ��ǰԇƬ��͵�ѪҺ�z�y�ֵ
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();

        // �ą�����@ȡ����ԇƬ��͵�ѪҺ�z�y�ֵ
        if (PARAM_GetStripParameters (currentStripType, &ndl, &udl, &bloodInThreshold) && (currentStripType < STRIP_TYPE_MAX)) {
            // �z��ADCֵ�Ƿ��^�ֵ
            if (adcValue > bloodInThreshold) {
                // �z�y��ѪҺ���@ʾ��ǰ�ضȁK�ГQ���y����B
                printf("Blood detected! Current temperature: %.1f��C\r\n", currentTemperature);
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
        
        // ���εĺ���Ӌ������ÿ���{�����Ӽs10ms��
        systemTick += 10;
        
        // ��һ���M��ˠ�B�r���d��r�򅢔�
        if (!parametersLoaded) {
            StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
            if (PARAM_GetTimingParameters(currentStripType, &tpl1, &trd1, &evWidth1, 1) &&
                PARAM_GetTimingParameters(currentStripType, &tpl2, &trd2, &evWidth2, 2)) {
                parametersLoaded = 1;
                stepStartTime = systemTick;
                measureStep = 1;
                printf("Measurement started for strip type: %s at temperature: %.1f��C\r\n", 
                       StripType_GetName(currentStripType), currentTemperature);
                printf("Timing params: EV1=%d, TPL1=%d, TRD1=%d, EV2=%d, TPL2=%d, TRD2=%d\r\n",
                       evWidth1, tpl1, trd1, evWidth2, tpl2, trd2);
                
                // �_��PWM�_ʼݔ��
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
                    // ����PWM���O��PB15��HIGH
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
                    // �������Þ�PWMݔ��
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
                    // �z�� tpl2 �Ƿ�� 0
                    if (tpl2 == 0) {
                        // ��� tpl2=0�����^ Step 4~6��ֱ�ӵ� Step 7
                        stepStartTime = systemTick;
                        measureStep = 7;
                        printf("TPL2=0, skipping Steps 4-6, jumping to Step 7: Reading GLU_OUT ADC value\r\n");
                    } else {
                        // ��t�^�m���� Step 4
                        stepStartTime = systemTick;
                        measureStep = 4;
                        printf("Step 4: PWM continues for EVWIDTH2 (%d ms)\r\n", evWidth2);
                    }
                }
                break;
                
            case 4: // Step 4: PWM for EVWIDTH2
                if (elapsedTime >= evWidth2) {
                    // ����PWM���O��PB15��HIGH
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
                    // �������Þ�PWMݔ��
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
                
            case 7: // Step 7: �xȡGLU_OUT ADCֵ
                {
                    // ʹ�� GetMidADC �����M�о��_�y��
                    // �B�mȡ��100�Σ�������ȡ���g20����ƽ��ֵ
                    uint16_t adcValue = GetMidADC(ADC_Channel_4, 100, 20);
                    
                    // ��ADCֵ����W_ADC׃���������mѪ��Ӌ��ʹ��
                    W_ADC = adcValue;
                    
                    printf("Measurement complete! GLU_OUT ADC value: %d (stored in W_ADC)\r\n", adcValue);
                    printf("Measurement temperature: %.1f��C\r\n", currentTemperature);
                    
                    // ֹͣPWMݔ�����O�Þ���ƽ
                    TIM_CtrlPWMOutputs(TIM1, DISABLE);
                    GPIO_InitTypeDef GPIO_InitStructure = {0};
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);
                    GPIO_SetBits(GPIOB, GPIO_Pin_15);
                    
                    // Step 8: �M��STATE_RESULT_READY
                    System_SetState(STATE_RESULT_READY);
                    
                    // ���Üy����B׃��
                    measureStep = 1;
                    parametersLoaded = 0;
                    stepStartTime = 0;
                    
                    printf("Step 8: Entering RESULT_READY state\r\n");
                }
                break;
                
            default:
                // ������B�����ÁK�M���e�`��B
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
        
        // ��������M��ˠ�B�����Ø��I
        if (currentState != lastState) {
            calculationDone = 0;
        }
        
        if (!calculationDone) {
            // ̎��Y���ʂ�à�B
            printf("Measurement result is ready for processing\r\n");
            
            // ����Ѫ��Ӌ�㺯������W_ADC�D�Q��Ѫ��ֵ
            CalGlucose(W_ADC);
            printf("Blood glucose measurement completed: %d %s (at %.1f��C)\r\n", 
                   wGlucose, Unit_GetSymbol((Unit_TypeDef)PARAM_GetByte(PARAM_MGDL)), currentTemperature);
            
            // ��ӛӋ�������
            calculationDone = 1;
            
            printf("Blood glucose calculation completed. Ready for next measurement.\r\n");
        }
        
        break;
    }

    case STATE_ERROR:
        // ̎���e�`��B
        // ���ÜضȜy�����I
        temperatureMeasured = 0;
        break;

    default:
        // δ֪��B�����Þ���f��B
        System_SetState (STATE_IDLE);
        break;
    }
}

/* System_SetState �� System_GetState ���������� system_state.c */

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

    /* ��ʼ��ADC */
    ADC1_Init();

    /* ��ʼ��OPA2 */
    OPA2_Init();

    /* ��ʼ��TIM1 PWM (WE_ENABLE, PB15, 20KHz) */
    TIM1_PWM_Init();

    /* �d��ض�У������ */
    Temperature_Load_Calibration();

    /* ��ʼ�������� */
    PARAM_Init();
    printf ("Parameter Table Initialized\r\n");

    /* ��ʼ��UARTͨӍ�f�h */
    UART_Protocol_Init();
    printf ("UART Protocol Initialized\r\n");

    /* ��ʼ��ԇƬ�ɜy���P�O�� */
    STRIP_DETECT_Init();

    /* ��ʼ��RTC */
    RTC_Config();
    printf("RTC Configured\r\n");

    /* �@ʾһЩ����ϵ�y�YӍ */
    printf ("Model No: %d\r\n", PARAM_GetByte (PARAM_MODEL_NO));
    printf ("FW Version: %d.%d\r\n", PARAM_GetByte (PARAM_FW_NO) / 10, PARAM_GetByte (PARAM_FW_NO) % 10);
    printf ("Parameter Table Version: %d\r\n", PARAM_GetWord (PARAM_CODE_TABLE_V));
    printf ("Test Count: %d\r\n", PARAM_GetWord (PARAM_NOT));

    /* �@ʾ��ǰ�O����ԇƬ��� */
    StripType_TypeDef stripType = (StripType_TypeDef)PARAM_GetByte (PARAM_STRIP_TYPE);
    printf ("Strip Type: %s\r\n", StripType_GetName (stripType));

    /* �@ʾ��ǰ�O���Ć�λ */
    Unit_TypeDef unit = (Unit_TypeDef)PARAM_GetByte (PARAM_MGDL);
    printf ("Unit: %s\r\n", Unit_GetSymbol (unit));

    printf("\r\n=== Temperature Measurement System Ready ===\r\n");
    printf("Use TEMP_CAL_INFO to check calibration status\r\n");
    printf("Use TEMP_TEST commands for verification\r\n\r\n");
    
    while (1) {
        // ̎��UART�f�h�Y��
        UART_Protocol_Process();

        // ̎��ԇƬ�ɜy���P�΄�
        STRIP_DETECT_Process();

        // ̎��ϵ�y��B�C
        State_Process();

        Delay_Ms (5);  // �p�����t�r�g������̎���ٶ�
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

        // ������յ��Y�ϣ�����UART�f�h̎����
        uint8_t i;
        for (i = 0; i < rxlen; i++) {
            UART2_Receive_Byte_ISR (USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
        }
    }

    // �z���Ƿ��н��յ��Y��
    if (USART_GetITStatus (USART2, USART_IT_RXNE) != RESET) {
        // �xȡ���յ���λԪ�M�K̎��
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

    // ̎��DMA���յ����Y��
    uint8_t i;
    for (i = 0; i < rxlen; i++) {
        UART2_Receive_Byte_ISR (USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
    }

    DMA_ClearITPendingBit (DMA1_IT_TC6);
}
