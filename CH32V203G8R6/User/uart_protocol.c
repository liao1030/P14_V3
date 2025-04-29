/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UARTͨӍ�f�h���F
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "uart_protocol.h"
#include <string.h>
#include "debug.h"

/* ȫ��׃�� */
static uint8_t tx_buffer[MAX_FRAME_LENGTH * 2];  // �l�;��n�^

/* �y������ */
Measure_Param_t measure_param = {0};

/* RTC�r�g */
RTC_TimeStruct_t rtc_time = {0};

/* �؂��C��׃�� */
static uint8_t retry_count = 0;
static uint8_t last_cmd_id = 0;
static uint16_t retry_timeout = 0;

/* �O���B */
Device_Status_t device_status = {0};

/* �y���Y�� */
Measure_Result_t measure_result = {0};

/* RAW���� */
uint8_t raw_data[MAX_DATA_LENGTH];
uint16_t raw_data_length = 0;

/* ��Ѫ��B */
uint8_t blood_sampling_active = 0;
uint8_t blood_countdown_seconds = 0;

/* �ⲿ������ */
extern void HandleProtocolCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* ԇƬ���P�������� */
void UART_Protocol_HandleStartT1Measurement(uint8_t *data, uint8_t length);
void UART_Protocol_HandleStripDetected(uint8_t *data, uint8_t length);

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   �f�h��ʼ��
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    retry_count = 0;
    retry_timeout = 0;
    
    /* ��ʼ���O���B */
    device_status.measure_type = MEASURE_TYPE_GLV;
    device_status.strip_status = 0;
    device_status.battery_voltage = 3000; // 3.0V
    device_status.temperature = 250;      // 25.0��C
    
    /* ��ʼ���y������ */
    measure_param.code = 0;
    measure_param.event = UART_EVENT_NONE;
    
    /* ��ʼ����Ѫ��B */
    blood_sampling_active = 0;
    blood_countdown_seconds = 0;
    
    /* ��ʼ��RAW���� */
    raw_data_length = 0;
    
    printf("UART Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      UART_Protocol_CalcChecksum
 *
 * @brief   Ӌ����У��
 *
 * @param   cmd_id - ָ��ID
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  Ӌ��õ���У��
 */
uint8_t UART_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint8_t checksum = cmd_id;
    uint8_t i;
    
    for (i = 0; i < length; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/*********************************************************************
 * @fn      UART_Protocol_PackFrame
 *
 * @brief   ����f�h����
 *
 * @param   cmd_id - ָ��ID
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 * @param   buffer - ݔ�����n�^
 *
 * @return  �����Ŀ��L��
 */
uint16_t UART_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer)
{
    uint8_t checksum;
    uint16_t index = 0;
    
    /* Ӌ��У�� */
    checksum = UART_Protocol_CalcChecksum(cmd_id, data, length);
    
    /* �M�b���� */
    buffer[index++] = FRAME_START_BYTE;  // ��ʼ��ӛ
    buffer[index++] = cmd_id;            // ָ��ID
    buffer[index++] = length;            // �L��
    
    /* �}�u�����^ */
    if (length > 0 && data != NULL) {
        memcpy(&buffer[index], data, length);
        index += length;
    }
    
    buffer[index++] = checksum;          // У��
    buffer[index++] = FRAME_END_BYTE;    // �Y����ӛ
    
    return index;
}

/*********************************************************************
 * @fn      UART_Protocol_SendFrame
 *
 * @brief   �l�ͅf�h����
 *
 * @param   cmd_id - ָ��ID
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void UART_Protocol_SendFrame(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint16_t frame_length;
    
    /* ������� */
    frame_length = UART_Protocol_PackFrame(cmd_id, data, length, tx_buffer);
    
    /* ͨ�^UART2�l�� */
    for (uint16_t i = 0; i < frame_length; i++) {
        USART_SendData(USART2, tx_buffer[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    /* ӛ���һ�l���� */
    last_cmd_id = cmd_id;
}

/*********************************************************************
 * @fn      UART_Protocol_SendErrorResponse
 *
 * @brief   �l���e�`푑�
 *
 * @param   orig_cmd_id - ԭʼָ��ID
 * @param   error_code - �e�`���a
 *
 * @return  none
 */
void UART_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code)
{
    uint8_t error_data[2];
    
    error_data[0] = orig_cmd_id;
    error_data[1] = error_code;
    
    UART_Protocol_SendFrame(CMD_ERROR_RESPONSE, error_data, 2);
    
    printf("Error response: cmd=%02X, error=%02X\r\n", orig_cmd_id, error_code);
}

/*********************************************************************
 * @fn      UART_Protocol_Parse
 *
 * @brief   �������յ��Ĕ���
 *
 * @param   data - ���յ��Ĕ���ָ�
 *          length - �����L��
 *
 * @return  0-�ɹ�����һ����������, 1-δ���, 2-�e�`
 */
uint8_t UART_Protocol_Parse(uint8_t *data, uint16_t length)
{
    static uint8_t rx_buffer[MAX_FRAME_LENGTH];  // ���վ��n�^
    static uint8_t rx_index = 0;                // ��ǰ����λ��
    static uint8_t frame_parsing = 0;           // �Ƿ����ڽ�����
    static uint8_t frame_cmd_id = 0;            // ��ǰ����ID
    static uint8_t frame_length = 0;            // ��ǰ�����L��
    uint16_t i;
    uint8_t result = 1; // Ĭ�Jδ���
    
    /* ��v���յ��Ĕ��� */
    for (i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        if (!frame_parsing) { // �ȴ���ʼ��ӛ
            if (byte == FRAME_START_BYTE) {
                /* �_ʼ�µĎ� */
                frame_parsing = 1;
                rx_index = 0;
                rx_buffer[rx_index++] = byte;
            }
        } else {
            /* �ѽ��ڽ���һ���� */
            rx_buffer[rx_index++] = byte;
            
            if (rx_index == 3) {
                /* �ѽ����յ�����ID���L�� */
                frame_cmd_id = rx_buffer[1];
                frame_length = rx_buffer[2];
            } else if (rx_index >= 5 && rx_index == frame_length + 5) {
                /* �ѽ��յ������� */
                if (byte == FRAME_END_BYTE) {
                    uint8_t calc_checksum;
                    uint8_t recv_checksum = rx_buffer[rx_index - 2];
                    
                    /* Ӌ��У�� */
                    calc_checksum = UART_Protocol_CalcChecksum(frame_cmd_id, &rx_buffer[3], frame_length);
                    
                    /* �z��У�� */
                    if (calc_checksum == recv_checksum) {
                        /* �����ɹ���̎������ */
                        HandleProtocolCommand(frame_cmd_id, &rx_buffer[3], frame_length);
                        
                        result = 0; // �����ɹ�
                    } else {
                        /* У��e�` */
                        UART_Protocol_SendErrorResponse(frame_cmd_id, ERR_CHECKSUM);
                        result = 2; // �����e�`
                    }
                } else {
                    /* �Y����ӛ�e�` */
                    UART_Protocol_SendErrorResponse(frame_cmd_id, ERR_DATA_FORMAT);
                    result = 2; // �����e�`
                }
                
                /* ���ý�����B */
                frame_parsing = 0;
            }
            
            /* �z�龏�n�^��� */
            if (rx_index >= MAX_FRAME_LENGTH) {
                frame_parsing = 0;
                result = 2; // �����e�`
            }
        }
    }
    
    return result;
}

/*********************************************************************
 * @fn      UART_Protocol_ProcessCommand
 *
 * @brief   ̎����յ��ąf�h����
 *
 * @param   cmd_id - ����ID
 * @param   data - ��������
 * @param   length - �����L��
 *
 * @return  none
 */
void UART_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    printf("Received command: %02X, length=%d\r\n", cmd_id, length);
    
    /* ����������ͷքe̎�� */
    switch (cmd_id) {
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
            UART_Protocol_HandleStartT1Measurement(data, length);
            break;
            
        case CMD_STRIP_DETECTED:
            UART_Protocol_HandleStripDetected(data, length);
            break;
            
        default:
            /* ��֧�ֵ����� */
            UART_Protocol_SendErrorResponse(cmd_id, ERR_COMMAND_NOTSUPPORT);
            break;
    }
}

/*********************************************************************
 * @fn      UART_Protocol_HandleStartT1Measurement
 *
 * @brief   ̎���_ʼT1�y������
 *
 * @param   data - ��������
 * @param   length - �����L��
 *
 * @return  none
 */
void UART_Protocol_HandleStartT1Measurement(uint8_t *data, uint8_t length)
{
    /* ί�ɽo����ʽ�е�̎���� */
    extern void HandleStartT1Measurement(uint8_t *data, uint8_t length);
    HandleStartT1Measurement(data, length);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleStripDetected
 *
 * @brief   ̎��ԇƬ�z�y֪ͨ
 *
 * @param   data - ��������
 * @param   length - �����L��
 *
 * @return  none
 */
void UART_Protocol_HandleStripDetected(uint8_t *data, uint8_t length)
{
    /* ί�ɽo����ʽ�е�̎���� */
    extern void HandleStripDetected(uint8_t *data, uint8_t length);
    HandleStripDetected(data, length);
} 