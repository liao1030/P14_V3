/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2024/05/08
 * Description        : P14�๦�������x�x���{���cUARTͨӍ�f�h̎��
 *********************************************************************************
 * Copyright (c) 2024 HMD. All rights reserved.
 *******************************************************************************/

#include "CONFIG.h"
#include "protocol_handler.h"
#include "app_uart.h"
#include "peripheral.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"
#include "CH58x_sys.h"

// ȫ��׃��
static uint8_t txBuf[MAX_PACKET_SIZE + 6];  // ������n�^
static time_info_t currentTime;             // ��ǰ�r�g
static uint8_t currentCode = 25;            // ��ǰԇ��У�a
static uint16_t currentEvent = EVENT_NONE;  // ��ǰ�¼����
static uint16_t currentItem = MEASURE_ITEM_GLV; // ��ǰ�z�y�Ŀ

// ��ԇ���P����
static uint8_t retryCount = 0;              // ��ԇ�Δ�
static uint8_t lastCmdId = 0;               // ��һ������ID
static uint8_t lastData[MAX_PACKET_SIZE];   // ��һ�������
static uint8_t lastDataLen = 0;             // ��һ��������L��
static uint32_t lastSendTime = 0;           // ��һ�ΰl�͕r�g
static uint8_t waitingResponse = 0;         // ���ڵȴ�푑����I

// ģ�M�O���B
static device_status_t deviceStatus = {
    .measureItem = MEASURE_ITEM_GLV,
    .stripStatus = 0,
    .batteryVoltage = 3000,  // 3.0V
    .temperature = 250       // 25.0��C
};

// ģ�M�y���Y��
static measure_result_t measureResult = {
    .resultStatus = 0,
    .measureValue = 123,
    .measureItem = MEASURE_ITEM_GLV,
    .event = EVENT_AC,
    .code = 25,
    .time = {2023, 4, 28, 15, 30, 0},
    .batteryVoltage = 3000,
    .temperature = 250
};

/*********************************************************************
 * @fn      protocol_init
 *
 * @brief   �f�h̎���ʼ��
 *
 * @return  none
 */
void protocol_init(void) {
    PRINT("�f�h̎���ʼ��\n");
    
    // ��ʼ����ǰ�r�g��Ĭ�Jֵ
    currentTime.year = 2023;
    currentTime.month = 4;
    currentTime.day = 28;
    currentTime.hour = 12;
    currentTime.minute = 0;
    currentTime.second = 0;
    
    // ��ʼ����ԇ���P����
    retryCount = 0;
    lastCmdId = 0;
    lastDataLen = 0;
    lastSendTime = 0;
    waitingResponse = 0;
}

/*********************************************************************
 * @fn      protocol_calculate_checksum
 *
 * @brief   Ӌ��У��
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  У��
 */
uint8_t protocol_calculate_checksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    uint16_t sum = cmdId;
    for (uint8_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      protocol_send_command
 *
 * @brief   �l�����UART
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  ��B
 */
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int idx = 0;
    
    // ���������Ա���ԇ
    lastCmdId = cmdId;
    if (dataLen > 0 && data != NULL) {
        memcpy(lastData, data, dataLen);
    }
    lastDataLen = dataLen;
    lastSendTime = SYS_GetSysTickCnt();
    waitingResponse = 1;
    
    // �M�b���
    txBuf[idx++] = PROTOCOL_START_MARKER;
    txBuf[idx++] = cmdId;
    txBuf[idx++] = dataLen;
    
    if (dataLen > 0 && data != NULL) {
        memcpy(&txBuf[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuf[idx++] = protocol_calculate_checksum(cmdId, data, dataLen);
    txBuf[idx++] = PROTOCOL_END_MARKER;
    
    // ͨ�^UART�l����CH32V203
    app_uart_tx_data(txBuf, idx);
    
    PRINT("�l�ͷ��: ID=0x%02X, �L��=%d\n", cmdId, dataLen);
    
    return SUCCESS;
}

/*********************************************************************
 * @fn      protocol_send_error
 *
 * @brief   �l���e�`푑�
 *
 * @param   originalCmdId - ԭʼ����ID
 * @param   errorCode - �e�`���a
 *
 * @return  ��B
 */
uint8_t protocol_send_error(uint8_t originalCmdId, uint8_t errorCode) {
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    return protocol_send_command(CMD_ERROR_ACK, data, 2);
}

/*********************************************************************
 * @fn      protocol_forward_to_ble
 *
 * @brief   ���յ���UART�����D�l��BLE
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  none
 */
void protocol_forward_to_ble(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int totalLen = dataLen + 5; // ��ʼ��ӛ(1) + ����ID(1) + �L��(1) + ����(dataLen) + У��(1) + �Y����ӛ(1)
    uint8_t packet[MAX_PACKET_SIZE + 6];
    
    // �M�b�����ąf�h���
    int idx = 0;
    packet[idx++] = PROTOCOL_START_MARKER;
    packet[idx++] = cmdId;
    packet[idx++] = dataLen;
    
    if (dataLen > 0 && data != NULL) {
        memcpy(&packet[idx], data, dataLen);
        idx += dataLen;
    }
    
    packet[idx++] = protocol_calculate_checksum(cmdId, data, dataLen);
    packet[idx++] = PROTOCOL_END_MARKER;
    
    // ͨ�^BLE�l�ͽo�֙C
    on_uart_to_ble_send(packet, idx);
    
    PRINT("�D�l��BLE: ID=0x%02X, �L��=%d\n", cmdId, dataLen);
    
    // ����ǌ�����Ļؑ����t����ȴ�푑����I
    if ((cmdId & 0x80) == 0x80 || cmdId == CMD_ERROR_ACK) {
        waitingResponse = 0;
        retryCount = 0;
    }
}

/*********************************************************************
 * @fn      protocol_retry_command
 *
 * @brief   ��ԇ�l������
 *
 * @return  none
 */
void protocol_retry_command(void) {
    // �z���Ƿ���Ҫ��ԇ
    if (waitingResponse && SYS_GetSysTickCnt() - lastSendTime > (500 * (retryCount + 1))) {
        if (retryCount < 3) {
            // �ذl����
            PRINT("��ԇ����: ID=0x%02X, �Δ�=%d\n", lastCmdId, retryCount + 1);
            protocol_send_command(lastCmdId, lastData, lastDataLen);
            retryCount++;
        } else {
            // ���^��ԇ�Δ������ͨӍ�e�`
            PRINT("����r��ͨӍ�e�`: ID=0x%02X\n", lastCmdId);
            
            // �����e�`���
            uint8_t errorData[2];
            errorData[0] = lastCmdId;
            errorData[1] = ERR_COMM_ERROR;
            
            // �D�l�o�֙C
            protocol_forward_to_ble(CMD_ERROR_ACK, errorData, 2);
            
            // ������ԇ��B
            waitingResponse = 0;
            retryCount = 0;
        }
    }
}

/*********************************************************************
 * @fn      protocol_process_uart_data
 *
 * @brief   ̎���UART�յ��Ĕ���
 *
 * @param   data - ����
 * @param   len - �L��
 *
 * @return  none
 */
void protocol_process_uart_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // ���վ��n�^
    static uint8_t rxIdx = 0;                   // ��ǰ��������
    static enum {
        STATE_IDLE,
        STATE_CMD,
        STATE_LEN,
        STATE_DATA,
        STATE_CHECKSUM,
        STATE_END
    } state = STATE_IDLE;
    
    static uint8_t cmdId = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataIdx = 0;
    static uint8_t checksum = 0;
    
    // ̎����յ���ÿ���ֹ�
    for (uint16_t i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        // �f�h��B�C
        switch (state) {
            case STATE_IDLE:
                if (ch == PROTOCOL_START_MARKER) {
                    rxBuf[rxIdx++] = ch;
                    state = STATE_CMD;
                }
                break;
                
            case STATE_CMD:
                cmdId = ch;
                rxBuf[rxIdx++] = ch;
                state = STATE_LEN;
                break;
                
            case STATE_LEN:
                dataLen = ch;
                rxBuf[rxIdx++] = ch;
                
                if (dataLen == 0) {
                    state = STATE_CHECKSUM;
                } else if (dataLen > MAX_PACKET_SIZE) {
                    // �����L�ȳ������ޣ����à�B
                    rxIdx = 0;
                    state = STATE_IDLE;
                } else {
                    dataIdx = 0;
                    state = STATE_DATA;
                }
                break;
                
            case STATE_DATA:
                rxBuf[rxIdx++] = ch;
                dataIdx++;
                
                if (dataIdx >= dataLen) {
                    state = STATE_CHECKSUM;
                }
                break;
                
            case STATE_CHECKSUM:
                checksum = ch;
                rxBuf[rxIdx++] = ch;
                state = STATE_END;
                break;
                
            case STATE_END:
                rxBuf[rxIdx++] = ch;
                
                if (ch == PROTOCOL_END_MARKER) {
                    // �z��У��
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, &rxBuf[3], dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // У�ɹ����������D�l��BLE
                        protocol_forward_to_ble(cmdId, &rxBuf[3], dataLen);
                    } else {
                        // У��e�`
                        PRINT("У���e�`: �յ�=0x%02X, Ӌ��=0x%02X\n", checksum, calculatedChecksum);
                        protocol_send_error(cmdId, ERR_CHECKSUM);
                    }
                }
                
                // ���à�B
                rxIdx = 0;
                state = STATE_IDLE;
                break;
        }
        
        // ��ֹ���n�^���
        if (rxIdx >= sizeof(rxBuf)) {
            rxIdx = 0;
            state = STATE_IDLE;
        }
    }
}

/*********************************************************************
 * @fn      protocol_handle_ble_data
 *
 * @brief   ̎���BLE�յ��Ĕ���
 *
 * @param   data - ����
 * @param   len - �L��
 *
 * @return  none
 */
void protocol_handle_ble_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // ���վ��n�^
    static uint8_t rxIdx = 0;                   // ��ǰ��������
    static enum {
        STATE_IDLE,
        STATE_CMD,
        STATE_LEN,
        STATE_DATA,
        STATE_CHECKSUM,
        STATE_END
    } state = STATE_IDLE;
    
    static uint8_t cmdId = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataIdx = 0;
    static uint8_t checksum = 0;
    
    // ̎����յ���ÿ���ֹ�
    for (uint16_t i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        // �f�h��B�C
        switch (state) {
            case STATE_IDLE:
                if (ch == PROTOCOL_START_MARKER) {
                    rxBuf[rxIdx++] = ch;
                    state = STATE_CMD;
                }
                break;
                
            case STATE_CMD:
                cmdId = ch;
                rxBuf[rxIdx++] = ch;
                state = STATE_LEN;
                break;
                
            case STATE_LEN:
                dataLen = ch;
                rxBuf[rxIdx++] = ch;
                
                if (dataLen == 0) {
                    state = STATE_CHECKSUM;
                } else if (dataLen > MAX_PACKET_SIZE) {
                    // �����L�ȳ������ޣ����à�B
                    rxIdx = 0;
                    state = STATE_IDLE;
                } else {
                    dataIdx = 0;
                    state = STATE_DATA;
                }
                break;
                
            case STATE_DATA:
                rxBuf[rxIdx++] = ch;
                dataIdx++;
                
                if (dataIdx >= dataLen) {
                    state = STATE_CHECKSUM;
                }
                break;
                
            case STATE_CHECKSUM:
                checksum = ch;
                rxBuf[rxIdx++] = ch;
                state = STATE_END;
                break;
                
            case STATE_END:
                rxBuf[rxIdx++] = ch;
                
                if (ch == PROTOCOL_END_MARKER) {
                    // �z��У��
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, &rxBuf[3], dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // У�ɹ����������D�l��UART
                        protocol_send_command(cmdId, &rxBuf[3], dataLen);
                    } else {
                        // У��e�`
                        PRINT("У���e�`: �յ�=0x%02X, Ӌ��=0x%02X\n", checksum, calculatedChecksum);
                        uint8_t errorData[2];
                        errorData[0] = cmdId;
                        errorData[1] = ERR_CHECKSUM;
                        protocol_forward_to_ble(CMD_ERROR_ACK, errorData, 2);
                    }
                }
                
                // ���à�B
                rxIdx = 0;
                state = STATE_IDLE;
                break;
        }
        
        // ��ֹ���n�^���
        if (rxIdx >= sizeof(rxBuf)) {
            rxIdx = 0;
            state = STATE_IDLE;
        }
    }
}

/*********************************************************************
 * @fn      protocol_timer_event
 *
 * @brief   �f�h���r���¼�̎��
 *
 * @return  none
 */
void protocol_timer_event(void) {
    // �z���Ƿ���Ҫ��ԇ�l������
    protocol_retry_command();
}

/*********************************************************************
 * @fn      on_uart_to_ble_send
 *
 * @brief   ��UART�D�l������BLE
 *
 * @param   data - ����
 * @param   len - �L��
 *
 * @return  none
 */
void on_uart_to_ble_send(uint8_t *data, uint16_t len) {
    // ͨ�^BLE�l��
    ble_uart_notify_data(data, len);
}

// ̎��ͬ���r�g����
static void handle_sync_time(uint8_t *data, uint8_t dataLen) {
    // У򞔵���L��
    if (dataLen != 7) {
        protocol_send_error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // �����r�g����
    currentTime.year = (data[0] << 8) | data[1];
    currentTime.month = data[2];
    currentTime.day = data[3];
    currentTime.hour = data[4];
    currentTime.minute = data[5];
    currentTime.second = data[6];
    
    PRINT("ͬ���r�g: %d-%02d-%02d %02d:%02d:%02d\n", 
          currentTime.year, currentTime.month, currentTime.day,
          currentTime.hour, currentTime.minute, currentTime.second);
    
    // �ؑ��_�J
    uint8_t response = 0x00;  // �ɹ�
    protocol_send_command(CMD_SYNC_TIME_ACK, &response, 1);
}

// ̎��Ո���O���B����
static void handle_device_status_request(void) {
    uint8_t data[8];
    
    // �M�b����
    data[0] = (deviceStatus.measureItem >> 8) & 0xFF;
    data[1] = deviceStatus.measureItem & 0xFF;
    data[2] = (deviceStatus.stripStatus >> 8) & 0xFF;
    data[3] = deviceStatus.stripStatus & 0xFF;
    data[4] = (deviceStatus.batteryVoltage >> 8) & 0xFF;
    data[5] = deviceStatus.batteryVoltage & 0xFF;
    data[6] = (deviceStatus.temperature >> 8) & 0xFF;
    data[7] = deviceStatus.temperature & 0xFF;
    
    // �l�ͻؑ�
    protocol_send_command(CMD_DEVICE_STATUS_ACK, data, 8);
    
    PRINT("�O���BՈ��ؑ�: �Ŀ=%d, ��B=%d, 늉�=%dmV, �ض�=%.1f��C\n",
          deviceStatus.measureItem, deviceStatus.stripStatus,
          deviceStatus.batteryVoltage, deviceStatus.temperature / 10.0);
}

// ̎���O��CODE��EVENT����
static void handle_set_code_event(uint8_t *data, uint8_t dataLen) {
    // У򞔵���L��
    if (dataLen != 3) {
        protocol_send_error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // ����CODE��EVENT
    currentCode = data[0];
    currentEvent = (data[1] << 8) | data[2];
    
    PRINT("�O��CODE=%d, EVENT=%d\n", currentCode, currentEvent);
    
    // �ؑ��_�J
    uint8_t response = 0x00;  // �ɹ�
    protocol_send_command(CMD_CODE_EVENT_ACK, &response, 1);
    
    // ���y���Y���е�CODE��EVENT
    measureResult.code = currentCode;
    measureResult.event = currentEvent;
}

// ̎��ѪҺ�z�y��BՈ��
static void handle_blood_sample_request(void) {
    // �@�eģ�M�z�y��ѪҺ����Ӌ�r5��
    uint8_t countdown = 5;
    
    PRINT("ѪҺ�z�y֪ͨ: ��Ӌ�r%d��\n", countdown);
    
    // �l�ͻؑ�
    protocol_send_command(CMD_BLOOD_SAMPLE_ACK, &countdown, 1);
}

// ̎��Ո��z�y�Y������
static void handle_result_request(void) {
    uint8_t data[25];
    int idx = 0;
    
    // ʹ�î�ǰ�r�g���y���Y���r�g
    measureResult.time = currentTime;
    
    // �M�b����
    data[idx++] = (measureResult.resultStatus >> 8) & 0xFF;
    data[idx++] = measureResult.resultStatus & 0xFF;
    data[idx++] = (measureResult.measureValue >> 8) & 0xFF;
    data[idx++] = measureResult.measureValue & 0xFF;
    data[idx++] = (measureResult.measureItem >> 8) & 0xFF;
    data[idx++] = measureResult.measureItem & 0xFF;
    data[idx++] = (measureResult.event >> 8) & 0xFF;
    data[idx++] = measureResult.event & 0xFF;
    data[idx++] = measureResult.code;
    
    // �r�g
    data[idx++] = (measureResult.time.year >> 8) & 0xFF;
    data[idx++] = measureResult.time.year & 0xFF;
    data[idx++] = (measureResult.time.month >> 8) & 0xFF;
    data[idx++] = measureResult.time.month & 0xFF;
    data[idx++] = (measureResult.time.day >> 8) & 0xFF;
    data[idx++] = measureResult.time.day & 0xFF;
    data[idx++] = (measureResult.time.hour >> 8) & 0xFF;
    data[idx++] = measureResult.time.hour & 0xFF;
    data[idx++] = (measureResult.time.minute >> 8) & 0xFF;
    data[idx++] = measureResult.time.minute & 0xFF;
    data[idx++] = (measureResult.time.second >> 8) & 0xFF;
    data[idx++] = measureResult.time.second & 0xFF;
    
    // 늳غ͜ض�
    data[idx++] = (measureResult.batteryVoltage >> 8) & 0xFF;
    data[idx++] = measureResult.batteryVoltage & 0xFF;
    data[idx++] = (measureResult.temperature >> 8) & 0xFF;
    data[idx++] = measureResult.temperature & 0xFF;
    
    // �l�ͻؑ�
    protocol_send_command(CMD_RESULT_ACK, data, idx);
    
    PRINT("�z�y�Y��: ��B=%d, ֵ=%d, �Ŀ=%d, �¼�=%d, У�a=%d\n",
          measureResult.resultStatus, measureResult.measureValue,
          measureResult.measureItem, measureResult.event, measureResult.code);
}

// ̎��Ո��RAW DATA����
static void handle_raw_data_request(void) {
    uint8_t rawData[50];
    
    // ���ģ�M��RAW DATA
    for (int i = 0; i < 50; i++) {
        rawData[i] = i;
    }
    
    uint8_t data[52];
    data[0] = 0;  // ��λ
    data[1] = 50; // �L��
    memcpy(&data[2], rawData, 50);
    
    // �l�ͻؑ�
    protocol_send_command(CMD_RAW_DATA_ACK, data, 52);
    
    PRINT("RAW DATAՈ����푑�\n");
} 