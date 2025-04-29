/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch58x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F�Д�̎���ʽ
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "ble_uart_protocol.h"

/* ȫ��׃�� */
uint8_t uart1RxBuffer[256];
volatile uint8_t uart1RxCount = 0;

/* ���������P׃�� */
#define DEBOUNCE_TIME 20    // 20ms�������r�g
volatile uint8_t stripDetect3Flag = 0;  // ԇƬ3�_�z�y���
volatile uint8_t stripDetect5Flag = 0;  // ԇƬ5�_�z�y���
volatile uint8_t stripDebounceInProgress = 0;  // ������̎���M�������

/* ԇƬ�z�y���P׃�� */
typedef struct {
    uint8_t pin3Status;     // ��3�_��B (0=High, 1=Low)
    uint8_t pin5Status;     // ��5�_��B (0=High, 1=Low)
} StripDetectStatus_t;

StripDetectStatus_t stripStatus = {0, 0};

/* ԇƬ��Ͷ��x */
typedef enum {
    PROTOCOL_STRIP_UNKNOWN = 0,   // δ֪���
    PROTOCOL_STRIP_GLV,           // Ѫ��(GLVԇƬ)
    PROTOCOL_STRIP_U,             // ����
    PROTOCOL_STRIP_C,             // ��đ�̴�
    PROTOCOL_STRIP_TG,            // �������֬
    PROTOCOL_STRIP_GAV            // Ѫ��(GAVԇƬ)
} StripType_t;

/* ������ */
void StripDetectProcess(void);
void SendStripDetectedMessage(uint8_t stripType);

/******************************************************************************/
/* �Д�̎���� */

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1�Д�̎���� - ̎���cCH32V203��ͨӍ
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void UART1_IRQHandler(void)
{
    uint8_t rxData;
    
    /* �����Д� */
    if(UART1_GetITFlag(RB_IER_RECV_RDY))
    {
        /* ����Д����I */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
        
        /* �xȡ���Ք��� */
        rxData = UART1_RecvByte();
        
        /* ������Ք��������n�^ */
        if(uart1RxCount < sizeof(uart1RxBuffer))
        {
            uart1RxBuffer[uart1RxCount++] = rxData;
        }
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO�Д�̎���� - ̎��ԇƬ����z�y
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void GPIO_IRQHandler(void)
{
    /* �Д��Ƿ��ԇƬ����z�y���_�Д� - PB11 (Strip_Detect_3) */
    if(GPIOB_ReadITFlagBit(GPIO_Pin_11))
    {
        /* ����Д����I */
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        
        /* �O�õ�3�_�z�y��� */
        stripDetect3Flag = 1;
        
        /* ���ӷ�����̎�� */
        if (!stripDebounceInProgress) {
            stripDebounceInProgress = 1;
            TMR0_TimerInit(FCLK/1000*DEBOUNCE_TIME);  // �O�ö��r����DEBOUNCE_TIME����
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);    // ���ö��r���Y���Д�
            PFIC_EnableIRQ(TMR0_IRQn);                // ���ö��r���Д�
        }
    }
    
    /* �Д��Ƿ��ԇƬ����z�y���_�Д� - PA15 (Strip_Detect_5) */
    if(GPIOA_ReadITFlagBit(GPIO_Pin_15))
    {
        /* ����Д����I */
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        
        /* �O�õ�5�_�z�y��� */
        stripDetect5Flag = 1;
        
        /* ���ӷ�����̎�� */
        if (!stripDebounceInProgress) {
            stripDebounceInProgress = 1;
            TMR0_TimerInit(FCLK/1000*DEBOUNCE_TIME);  // �O�ö��r����DEBOUNCE_TIME����
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);    // ���ö��r���Y���Д�
            PFIC_EnableIRQ(TMR0_IRQn);                // ���ö��r���Д�
        }
    }
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   ���r��0�Д�̎���� - ���ԇƬ���������̎��
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void TMR0_IRQHandler(void)
{
    /* ����Д����I */
    TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
    
    /* ͣ�ö��r�� */
    TMR0_ITCfg(DISABLE, TMR0_3_IT_CYC_END);
    PFIC_DisableIRQ(TMR0_IRQn);
    
    /* �������r�g�Y����̎��ԇƬ�z�y */
    stripDebounceInProgress = 0;
    
    /* ����Йz�y��˱��O�ã�����ԇƬ�z�y̎�� */
    if (stripDetect3Flag || stripDetect5Flag) {
        StripDetectProcess();
        
        /* ����z�y��� */
        stripDetect3Flag = 0;
        stripDetect5Flag = 0;
    }
}

/*********************************************************************
 * @fn      StripDetectProcess
 *
 * @brief   ԇƬ�z�y̎���� - �z�yԇƬ��́K֪ͨCH32V203
 *
 * @return  none
 */
void StripDetectProcess(void)
{
    uint8_t stripType = PROTOCOL_STRIP_UNKNOWN;
    
    /* �xȡStrip_Detect_3��B */
    if (GPIOB_ReadPortPin(GPIO_Pin_11)) {
        stripStatus.pin3Status = 0;  // ���ƽ
    } else {
        stripStatus.pin3Status = 1;  // ���ƽ
    }
    
    /* �xȡStrip_Detect_5��B */
    if (GPIOA_ReadPortPin(GPIO_Pin_15)) {
        stripStatus.pin5Status = 0;  // ���ƽ
    } else {
        stripStatus.pin5Status = 1;  // ���ƽ
    }
    
    /* ����MCU�ϵ�T1���y�·��������m��T1_OUT늉��y�� */
    /* �l������oCH32V203�_ʼT1���y */
    uint8_t cmd_data[1] = {0x01};  // 0x01��ʾ�_ʼT1���y
    BLE_Protocol_SendFrame(0x10, cmd_data, 1);  // ʹ������ID 0x10ָʾ�_ʼT1���y
    
    /* �ȴ�CH32V203����T1���y�Y�� */
    /* ע�⣺���H�����Б�ʹ�î���̎��򶨕r�����@�e������ͬ���ȴ� */
    Delay_Ms(10);
    
    /* ����ԇƬ��Ͱl��֪ͨ */
    /* ע�⣺�@�e���O�ѽ��յ���T1_OUT�y���Y�������H����Ҫ�ڻؑ�̎������� */
    /* ��̎�H�����_λ�����Д�ԇƬ��ͣ������Д���Y��T1_OUT늉�ֵ */
    
    if (stripStatus.pin3Status == 1 && stripStatus.pin5Status == 0) {
        /* ������GLV��U��ԇƬ����Y��T1_OUT늉��Д� */
        stripType = PROTOCOL_STRIP_GLV;  // ������GLV��
    } else if (stripStatus.pin3Status == 1 && stripStatus.pin5Status == 1) {
        stripType = PROTOCOL_STRIP_C;  // ��đ�̴�
    } else if (stripStatus.pin3Status == 0 && stripStatus.pin5Status == 1) {
        stripType = PROTOCOL_STRIP_TG;  // �������֬
    } else if (stripStatus.pin3Status == 0 && stripStatus.pin5Status == 0) {
        stripType = PROTOCOL_STRIP_GAV;  // GAV��Ѫ��
    }
    
    /* �l��ԇƬ���֪ͨ */
    SendStripDetectedMessage(stripType);
}

/*********************************************************************
 * @fn      SendStripDetectedMessage
 *
 * @brief   �l��ԇƬ�z�y֪ͨ�oCH32V203
 *
 * @param   stripType - �z�y����ԇƬ���
 *
 * @return  none
 */
void SendStripDetectedMessage(uint8_t stripType)
{
    uint8_t data[2];
    
    /* �ʂ�ԇƬ�z�y���� */
    data[0] = stripType;                   // ԇƬ���
    data[1] = (stripStatus.pin3Status << 1) | stripStatus.pin5Status; // �_λ��B
    
    /* �l��ԇƬ�z�y֪ͨ */
    BLE_Protocol_SendFrame(0x11, data, 2);  // ʹ������ID 0x11ָʾԇƬ���֪ͨ
    
    /* �������֪��ͣ����������Ĝy���· */
    if (stripType == PROTOCOL_STRIP_GAV) {
        /* ���GAVԇƬ������T3늘O���y */
        GPIOB_SetBits(GPIO_Pin_10);  // T3_IN_SEL���_����
    }
} 