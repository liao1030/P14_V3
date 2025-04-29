/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_protocol.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : �{���cUARTͨӍ�f�h���F
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "ble_uart_protocol.h"
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* ȫ��׃�� */
static uint8_t rx_buffer[MAX_FRAME_LENGTH * 2];  // UART���վ��n�^
static uint16_t rx_index = 0;                    // ��ǰ����λ��
static uint8_t tx_buffer[MAX_FRAME_LENGTH * 2];  // �l�;��n�^

/* �{���B�Ә��I */
static uint8_t ble_connected = 0;

/* �؂��C��׃�� */
static uint8_t retry_count = 0;
static uint8_t last_cmd_id = 0;
static uint16_t retry_timeout = 0;

/* �f�h��B�C׃�� */
static uint8_t frame_parsing = 0;
static uint8_t frame_length = 0;
static uint8_t frame_cmd_id = 0;
static uint8_t frame_checksum = 0;

/*********************************************************************
 * @fn      BLE_Protocol_Init
 *
 * @brief   �f�h��ʼ��
 *
 * @return  none
 */
void BLE_Protocol_Init(void)
{
    rx_index = 0;
    frame_parsing = 0;
    ble_connected = 0;
    retry_count = 0;
    retry_timeout = 0;
}

/*********************************************************************
 * @fn      BLE_Protocol_CalcChecksum
 *
 * @brief   Ӌ����У��
 *
 * @param   cmd_id - ָ��ID
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  Ӌ��õ���У��
 */
uint8_t BLE_Protocol_CalcChecksum(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint8_t checksum = cmd_id;
    uint8_t i;
    
    for (i = 0; i < length; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/*********************************************************************
 * @fn      BLE_Protocol_PackFrame
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
uint16_t BLE_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer)
{
    uint8_t checksum;
    uint16_t index = 0;
    
    /* Ӌ��У�� */
    checksum = BLE_Protocol_CalcChecksum(cmd_id, data, length);
    
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
 * @fn      BLE_Protocol_SendResponse
 *
 * @brief   �l�ͅf�h푑�
 *
 * @param   cmd_id - ָ��ID
 * @param   data - �����^ָ�
 * @param   length - �����^�L��
 *
 * @return  none
 */
void BLE_Protocol_SendResponse(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint16_t frame_length;
    
    /* ���푑����� */
    frame_length = BLE_Protocol_PackFrame(cmd_id, data, length, tx_buffer);
    
    /* ͨ�^BLE�l�� */
    UART_To_BLE_Forward(tx_buffer, frame_length);
}

/*********************************************************************
 * @fn      BLE_Protocol_SendErrorResponse
 *
 * @brief   �l���e�`푑�
 *
 * @param   orig_cmd_id - ԭʼָ��ID
 * @param   error_code - �e�`���a
 *
 * @return  none
 */
void BLE_Protocol_SendErrorResponse(uint8_t orig_cmd_id, uint8_t error_code)
{
    uint8_t error_data[2];
    
    error_data[0] = orig_cmd_id;
    error_data[1] = error_code;
    
    BLE_Protocol_SendResponse(CMD_ERROR_RESPONSE, error_data, 2);
    
    PRINT("Error response: cmd=%02X, error=%02X\n", orig_cmd_id, error_code);
}

/*********************************************************************
 * @fn      BLE_Protocol_Parse
 *
 * @brief   �����յ��Ĕ���
 *
 * @param   data - �յ��Ĕ���ָ�
 * @param   length - �����L��
 *
 * @return  0-�ɹ�����һ����������, 1-δ���, 2-�e�`
 */
uint8_t BLE_Protocol_Parse(uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint8_t result = 1; // Ĭ�Jδ���
    
    /* ��v�յ��Ĕ��� */
    for (i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        if (!frame_parsing) { // �ȴ���ʼ��ӛ
            if (byte == FRAME_START_BYTE) {
                /* ���_ʼ */
                frame_parsing = 1;
                rx_index = 0;
                rx_buffer[rx_index++] = byte;
            }
        } else {
            /* �ѽ��ڽ���һ���� */
            rx_buffer[rx_index++] = byte;
            
            if (rx_index == 3) {
                /* �ѽ��յ�ָ��ID���L�� */
                frame_cmd_id = rx_buffer[1];
                frame_length = rx_buffer[2];
            } else if (rx_index >= 5 && rx_index == frame_length + 5) {
                /* ������������� */
                if (byte == FRAME_END_BYTE) {
                    uint8_t calc_checksum;
                    uint8_t recv_checksum = rx_buffer[rx_index - 2];
                    
                    /* Ӌ��У�� */
                    calc_checksum = BLE_Protocol_CalcChecksum(frame_cmd_id, &rx_buffer[3], frame_length);
                    
                    /* �z��У�� */
                    if (calc_checksum == recv_checksum) {
                        /* �����ɹ�, ̎��ָ�� */
                        BLE_To_UART_Forward(rx_buffer, rx_index);
                        
                        result = 0; // �����ɹ�
                    } else {
                        /* У��e�` */
                        BLE_Protocol_SendErrorResponse(frame_cmd_id, ERR_CHECKSUM);
                        result = 2; // �����e�`
                    }
                } else {
                    /* �Y����ӛ�e�` */
                    BLE_Protocol_SendErrorResponse(frame_cmd_id, ERR_DATA_FORMAT);
                    result = 2; // �����e�`
                }
                
                /* ���ý�����B */
                frame_parsing = 0;
            }
            
            /* �z�龏�n�^��� */
            if (rx_index >= MAX_FRAME_LENGTH * 2) {
                frame_parsing = 0;
                result = 2; // �����e�`
            }
        }
    }
    
    return result;
}

/*********************************************************************
 * @fn      BLE_To_UART_Forward
 *
 * @brief   ��BLE���Ք����D�l��UART
 *
 * @param   data - ����ָ�
 * @param   length - �����L��
 *
 * @return  none
 */
void BLE_To_UART_Forward(uint8_t *data, uint16_t length)
{
    uint16_t i;
    
    /* ���ֹ�ͨ�^UART1�l�͵�CH32V203 */
    for (i = 0; i < length; i++) {
        UART1_SendByte(data[i]);
    }
}

/*********************************************************************
 * @fn      UART_To_BLE_Forward
 *
 * @brief   ��UART���Ք����D�l��BLE
 *
 * @param   data - ����ָ�
 * @param   length - �����L��
 *
 * @return  none
 */
void UART_To_BLE_Forward(uint8_t *data, uint16_t length)
{
    /* �z���Ƿ��B�ӵ�BLE */
    if (ble_connected) {
        /* ͨ�^BLE�l�͔��� */
        uint16_t remain = length;
        uint16_t offset = 0;
        uint16_t send_len;
        
        /* ���MTU���ƣ�������Ҫ�ְ��l�� */
        while (remain > 0) {
            send_len = (remain > 20) ? 20 : remain;
            
            /* ʹ��BLE�캯���l�͔��� */
            BLE_SendData(&data[offset], send_len);
            
            offset += send_len;
            remain -= send_len;
            
            /* �B�m�l���g��Ҫ�ӕr */
            DelayMs(30);
        }
    }
}

/*********************************************************************
 * @fn      BLE_UART_SendData
 *
 * @brief   �l�͔�����UART��BLE
 *
 * @param   data - ����ָ�
 * @param   length - �����L��
 *
 * @return  none
 */
void BLE_UART_SendData(uint8_t *data, uint16_t length)
{
    /* �������D�l��UART��BLE */
    UART1_SendString(data, length);
    
    if (ble_connected) {
        uint16_t remain = length;
        uint16_t offset = 0;
        uint16_t send_len;
        
        while (remain > 0) {
            send_len = (remain > 20) ? 20 : remain;
            BLE_SendData(&data[offset], send_len);
            
            offset += send_len;
            remain -= send_len;
            
            DelayMs(30);
        }
    }
}

/*********************************************************************
 * @fn      BLE_SetConnectedState
 *
 * @brief   �O��BLE�B�Ӡ�B
 *
 * @param   connected - �B�Ә��I, 1�B��, 0���_
 *
 * @return  none
 */
void BLE_SetConnectedState(uint8_t connected)
{
    ble_connected = connected;
    
    PRINT("BLE %s\n", connected ? "Connected" : "Disconnected");
} 