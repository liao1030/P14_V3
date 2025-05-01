/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14�ĿԇƬ����ɜy�c����Єeģ�M
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_Init.h"

/* �ⲿ������ */
uint32_t millis(void);  // �@ȡϵ�y�r�g(����)

/* ȫ��׃�� */
static volatile StripState_TypeDef g_stripState = STRIP_STATE_NONE;
static volatile uint32_t g_stripInsertTime = 0;
static volatile uint8_t g_stripDetectFlag = 0;
static volatile StripType_TypeDef g_stripType = STRIP_TYPE_UNKNOWN;

/* �����CH32V203�l������ľ��� */
static uint8_t g_cmdBuffer[8];

    /* UART���վ��n�^ */
static volatile uint8_t g_rxBuffer[16];
static volatile uint8_t g_rxIndex = 0;
static volatile uint8_t g_rxComplete = 0;
static volatile uint32_t g_lastRxTime = 0;
#define UART_TIMEOUT_MS  100   // UART���ճ��r�r�g

/*********************************************************************
 * @fn      P14_StripDetectInit
 *
 * @brief   ��ʼ��ԇƬ�ɜy����
 *
 * @param   none
 *
 * @return  none
 */
void P14_StripDetectInit(void)
{
    /* ��ʼ���Д�PIN */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);     // Strip_Detect_3, ����ݔ��
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);     // Strip_Detect_5, ����ݔ��
    
    /* �O�����½����|�l�Д� */
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge);
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge);
    
    /* ����GPIO�Д� */
    PFIC_EnableIRQ(GPIO_B_IRQn);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    
    /* ��ʼ����B */
    g_stripState = STRIP_STATE_NONE;
    g_stripDetectFlag = 0;
    g_stripType = STRIP_TYPE_UNKNOWN;
    g_rxIndex = 0;
    g_rxComplete = 0;
}

/*********************************************************************
 * @fn      P14_CheckStripInsertion
 *
 * @brief   �z��ԇƬ�����B��̎�������
 *
 * @param   none
 *
 * @return  1: ԇƬ�_�J���룬���M������Єe
 *          0: ԇƬδ��ɲ�������
 */
uint8_t P14_CheckStripInsertion(void)
{
    uint8_t stripDetected = 0;
    uint32_t currentTime = millis();
    
    /* �z���Ƿ���ԇƬ�����¼��l�� */
    if (g_stripDetectFlag) {
        switch (g_stripState) {
            case STRIP_STATE_NONE:
                /* �M�����������A�� */
                g_stripState = STRIP_STATE_INSERTING;
                g_stripInsertTime = currentTime;
                break;
                
            case STRIP_STATE_INSERTING:
                /* �������r�g�z�� */
                if (currentTime - g_stripInsertTime > STRIP_DEBOUNCE_TIME) {
                    /* �ٴδ_�JԇƬ�_�����루ͨ�^Strip_Detect_3��Strip_Detect_5���ƽ�� */
                    if ((!(GPIOB_ReadPortPin(GPIO_Pin_11)) || !(GPIOA_ReadPortPin(GPIO_Pin_15)))) {
                        g_stripState = STRIP_STATE_INSERTED;
                        stripDetected = 1;
                    } else {
                        /* �`�|�l���֏͞�oԇƬ��B */
                        g_stripState = STRIP_STATE_NONE;
                    }
                    g_stripDetectFlag = 0;
                }
                break;
                
            default:
                g_stripDetectFlag = 0;
                break;
        }
    }
    
    return stripDetected;
}

/*********************************************************************
 * @fn      P14_UART1_RxHandler
 *
 * @brief   UART1�����Y��̎��
 *
 * @param   rx_data - ���յ��Y��
 *
 * @return  none
 */
void P14_UART1_RxHandler(uint8_t rx_data)
{
    /* ���½��Օr�g�� */
    g_lastRxTime = millis();
    
    /* �_ʼ��ӛ�z�� */
    if (g_rxIndex == 0 && rx_data == 0xAA) {
        g_rxBuffer[g_rxIndex++] = rx_data;
    }
    /* �Y�Ͻ��� */
    else if (g_rxIndex > 0) {
        /* ��ֹ���n�^��� */
        if (g_rxIndex < sizeof(g_rxBuffer)) {
            g_rxBuffer[g_rxIndex++] = rx_data;
        }
        
        /* �z��Y����ӛ */
        if (rx_data == 0xBB && g_rxIndex >= 4) {
            g_rxComplete = 1;
        }
    }
}

/*********************************************************************
 * @fn      P14_UART1_CheckTimeout
 *
 * @brief   �z��UART1���ճ��r
 *
 * @param   none
 *
 * @return  none
 */
void P14_UART1_CheckTimeout(void)
{
    /* ������ڽ����Y�ϵ��ѳ��r�����ý��վ��n�^ */
    uint32_t currentTime = millis();
    if (g_rxIndex > 0 && !g_rxComplete && 
        (currentTime - g_lastRxTime > UART_TIMEOUT_MS)) {
        g_rxIndex = 0;
    }
}

/*********************************************************************
 * @fn      P14_IdentifyStripType
 *
 * @brief   �Д�ԇƬ���
 *
 * @param   none
 *
 * @return  StripType_TypeDef: ԇƬ���
 */
StripType_TypeDef P14_IdentifyStripType(void)
{
    StripType_TypeDef type = STRIP_TYPE_UNKNOWN;
    uint16_t t1_out_value = 0;
    uint8_t t1_out_near_2p5v = 0;
    uint8_t retry_count = 0;
    uint32_t timeout_start = 0;
    
    /* �xȡPin3��Pin5�Ġ�B */
    uint8_t pin3_state = GPIOB_ReadPortPin(GPIO_Pin_11) ? 1 : 0;  // Strip_Detect_3
    uint8_t pin5_state = GPIOA_ReadPortPin(GPIO_Pin_15) ? 1 : 0;  // Strip_Detect_5
    
    /* ���O���ՠ�B */
    g_rxIndex = 0;
    g_rxComplete = 0;
    
    /* �Lԇ���3�Ϋ@ȡT1_OUT늉� */
    while (retry_count < 3) {
        /* �l������oCH32V203�M��T1_OUT늉��y�� */
        g_cmdBuffer[0] = 0xAA;  // �����_ʼ��ӛ
        g_cmdBuffer[1] = 0x01;  // ������ͣ��y��T1_OUT
        g_cmdBuffer[2] = 0xBB;  // ����Y����ӛ
        
        /* �l������ */
        for (uint8_t i = 0; i < 3; i++) {
            UART1_SendByte(g_cmdBuffer[i]);
        }
        
        /* �ȴ��ؑ���ʹ�ó��r�C�� */
        timeout_start = millis();
        while (!g_rxComplete) {
            /* �z�鳬�r */
            if (millis() - timeout_start > 200) {
                break;
            }
            
            /* �z����ճ��r */
            P14_UART1_CheckTimeout();
            
            /* �̕����t */
            DelayMs(1);
        }
        
        /* �z���Ƿ��յ������ؑ� */
        if (g_rxComplete && g_rxIndex >= 5) {
            /* �z��ؑ���ʽ [0xAA][CMD][ADC_VALUE_H][ADC_VALUE_L][0xBB] */
            if (g_rxBuffer[0] == 0xAA && g_rxBuffer[1] == 0x01 && g_rxBuffer[4] == 0xBB) {
                /* ����ADCֵ */
                t1_out_value = ((uint16_t)g_rxBuffer[2] << 8) | g_rxBuffer[3];
                
                /* ���O���ՠ�B */
                g_rxIndex = 0;
                g_rxComplete = 0;
                
                /* �ɹ��@ȡ����������ѭ�h */
                break;
            }
        }
        
        /* ���O���ՠ�B */
        g_rxIndex = 0;
        g_rxComplete = 0;
        
        /* ������ԇӋ���� */
        retry_count++;
        
        /* ��ԇ�g�� */
        DelayMs(50);
    }
    
    /* �Д�T1_OUT늉��Ƿ�ӽ�2.5V (��s3000/4096 * 3.3V = 2.4V) */
    t1_out_near_2p5v = (t1_out_value > 3000) ? 1 : 0;
    
    /* ����ԇƬ����Єe�C���ļ��M������Д� */
    if (pin3_state == 0 && pin5_state == 1 && t1_out_near_2p5v) {
        type = STRIP_TYPE_GLV;     // Ѫ��(GLVԇƬ)
    } else if (pin3_state == 0 && pin5_state == 1 && !t1_out_near_2p5v) {
        type = STRIP_TYPE_U;       // ����
    } else if (pin3_state == 0 && pin5_state == 0 && t1_out_near_2p5v) {
        type = STRIP_TYPE_C;       // ��đ�̴�
    } else if (pin3_state == 1 && pin5_state == 0 && !t1_out_near_2p5v) {
        type = STRIP_TYPE_TG;      // ���������
    } else if (pin3_state == 1 && pin5_state == 0 && t1_out_near_2p5v) {
        type = STRIP_TYPE_GAV;     // Ѫ��(GAVԇƬ)
    } else {
        type = STRIP_TYPE_UNKNOWN; // δ֪���
    }
    
    /* ����z�y����ԇƬ��� */
    g_stripType = type;
    g_stripState = STRIP_STATE_READY;
    
    return type;
}

/*********************************************************************
 * @fn      P14_NotifyStripInserted
 *
 * @brief   ֪ͨCH32V203�љz�y��ԇƬ�K���Єe���
 *
 * @param   type - ԇƬ���
 *
 * @return  none
 */
void P14_NotifyStripInserted(StripType_TypeDef type)
{
    /* ����֪ͨ���� */
    g_cmdBuffer[0] = 0xAA;           // �����_ʼ��ӛ
    g_cmdBuffer[1] = 0x02;           // ������ͣ�ԇƬ����֪ͨ
    g_cmdBuffer[2] = (uint8_t)type;  // ԇƬ���
    g_cmdBuffer[3] = 0xBB;           // ����Y����ӛ
    
    /* �l������ */
    for (uint8_t i = 0; i < 4; i++) {
        UART1_SendByte(g_cmdBuffer[i]);
    }
}

/*********************************************************************
 * @fn      P14_StripStateReset
 *
 * @brief   ���OԇƬ��B
 *
 * @param   none
 *
 * @return  none
 */
void P14_StripStateReset(void)
{
    g_stripState = STRIP_STATE_NONE;
    g_stripDetectFlag = 0;
    g_stripType = STRIP_TYPE_UNKNOWN;
}

/*********************************************************************
 * @fn      GPIOB_IRQHandler
 *
 * @brief   GPIOB�Д�̎���������Strip_Detect_3�ęz�y
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT void GPIOB_IRQHandler(void)
{
    /* �z���Ƿ��Strip_Detect_3(PB11)�|�l */
    if ((R16_PB_INT_IF & GPIO_Pin_11) && (g_stripState == STRIP_STATE_NONE)) {
        g_stripDetectFlag = 1;
    }
    
    /* ����Д����I */
    R16_PB_INT_IF = GPIO_Pin_11;
}

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA�Д�̎���������Strip_Detect_5�ęz�y
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT void GPIOA_IRQHandler(void)
{
    /* �z���Ƿ��Strip_Detect_5(PA15)�|�l */
    if ((R16_PA_INT_IF & GPIO_Pin_15) && (g_stripState == STRIP_STATE_NONE)) {
        g_stripDetectFlag = 1;
    }
    
    /* ����Д����I */
    R16_PA_INT_IF = GPIO_Pin_15;
} 