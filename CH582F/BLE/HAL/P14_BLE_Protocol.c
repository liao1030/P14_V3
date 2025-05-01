/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_BLE_Protocol.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14�ĿCH582F�{��ͨӍ�f�h���F
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_BLE_Protocol.h"
#include "P14_Init.h"

/* ȫ��׃�� */
static uint8_t g_rxBuffer[32];
static uint8_t g_rxIndex = 0;
static uint8_t g_cmdState = 0;  // 0: �ȴ������_ʼ, 1: �ȴ��������, 2: �ȴ�����, 3: �ȴ��Y��

/* ADC�y������ - ��̎��ģ�M���F */
static uint16_t Get_ADC_Value_Mock(void) {
    /* �ڌ��H�����У��˺�����ԓʹ��CH582F��ADCģ�M��y�����H늉� */
    /* �@�e�H����һ��ģ�Mֵ��출yԇ */
    return 3500; // ģ�M�ӽ�3.5V��ADCֵ
}

/*********************************************************************
 * @fn      P14_BLE_ProtocolInit
 *
 * @brief   ��ʼ���{���f�h̎��
 *
 * @param   none
 *
 * @return  none
 */
void P14_BLE_ProtocolInit(void)
{
    /* ���ý��վ��n�^ */
    memset(g_rxBuffer, 0, sizeof(g_rxBuffer));
    g_rxIndex = 0;
    g_cmdState = 0;
    
    PRINT("P14 BLE Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      P14_BLE_UartCallback
 *
 * @brief   UART���Ք������{����
 *
 * @param   data - ���յ��Ĕ���
 * @param   length - �����L��
 *
 * @return  none
 */
void P14_BLE_UartCallback(uint8_t *data, uint16_t length)
{
    /* ̎���CH32V203���յ��Ĕ��� */
    if (data && length > 0) {
        for (uint16_t i = 0; i < length; i++) {
            /* �f�h��B�C */
            switch (g_cmdState) {
                case 0:  // �ȴ������_ʼ
                    if (data[i] == CMD_START_MARKER) {
                        g_rxBuffer[0] = CMD_START_MARKER;
                        g_rxIndex = 1;
                        g_cmdState = 1;
                    }
                    break;
                    
                case 1:  // �ȴ��������
                    g_rxBuffer[g_rxIndex++] = data[i];
                    
                    /* ����������͛Q�����m̎�� */
                    switch (data[i]) {
                        case CMD_MEASURE_T1_OUT:
                            /* �յ�T1_OUT�y���Y�� */
                            g_cmdState = 2;
                            break;
                            
                        case CMD_STRIP_TYPE_RESULT:
                            /* �յ�ԇƬ��ͽY�� */
                            g_cmdState = 2;
                            break;
                            
                        default:
                            /* δ֪������ͣ����à�B�C */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                            break;
                    }
                    break;
                    
                case 2:  // �ȴ�����(ADC���ֹ���Y�����a)
                    g_rxBuffer[g_rxIndex++] = data[i];
                    g_cmdState = 3;
                    break;
                    
                case 3:  // �ȴ���һ��������Y����ӛ
                    g_rxBuffer[g_rxIndex++] = data[i];
                    
                    /* ������һ����������Д� */
                    if (g_rxBuffer[1] == CMD_MEASURE_T1_OUT) {
                        /* T1_OUT�y���Y����Ҫ4���ֹ�������: [0xAA][0x01][HIGH][LOW][0xBB] */
                        if (g_rxIndex >= 4 && data[i] == CMD_END_MARKER) {
                            /* ̎��������T1_OUT�y���Y�� */
                            uint16_t adcValue = (uint16_t)g_rxBuffer[2] << 8 | g_rxBuffer[3];
                            PRINT("T1_OUT ADCֵ: %d\r\n", adcValue);
                            
                            /* ���à�B�C */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        } else if (g_rxIndex > 5) {
                            /* ����̫�L���������e�`��������à�B�C */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        }
                    } else if (g_rxBuffer[1] == CMD_STRIP_TYPE_RESULT) {
                        /* ԇƬ��ͽY����Ҫ3���ֹ�������: [0xAA][0x03][TYPE][0xBB] */
                        if (g_rxIndex >= 3 && data[i] == CMD_END_MARKER) {
                            /* ̎��������ԇƬ��ͽY�� */
                            StripType_TypeDef type = (StripType_TypeDef)g_rxBuffer[2];
                            PRINT("���յ�ԇƬ��ͽY��: %d\r\n", type);
                            
                            /* ���à�B�C */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        } else if (g_rxIndex > 4) {
                            /* ����̫�L���������e�`��������à�B�C */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        }
                    } else {
                        /* δ֪������ͣ����à�B�C */
                        g_cmdState = 0;
                        g_rxIndex = 0;
                    }
                    break;
                    
                default:
                    /* δ֪��B�����à�B�C */
                    g_cmdState = 0;
                    g_rxIndex = 0;
                    break;
            }
            
            /* ��ֹ���n�^��� */
            if (g_rxIndex >= sizeof(g_rxBuffer)) {
                g_cmdState = 0;
                g_rxIndex = 0;
            }
        }
    }
} 