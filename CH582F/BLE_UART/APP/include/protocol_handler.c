/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2023/04/28
 * Description        : P14�๦�������x�x���{���cUARTͨӍ�f�h̎��
 *********************************************************************************
 * Copyright (c) 2023 HMD. All rights reserved.
 *******************************************************************************/

#include "CONFIG.h"
#include "protocol_handler.h"
#include "app_uart.h"
#include "peripheral.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"

// ȫ��׃��
static uint8_t txBuf[MAX_PACKET_SIZE + 6];  // ������n�^
static time_info_t currentTime;             // ��ǰ�r�g
static uint8_t currentCode = 25;            // ��ǰԇ��У�a
static uint16_t currentEvent = EVENT_NONE;  // ��ǰ�¼����
static uint16_t currentItem = MEASURE_ITEM_GLV; // ��ǰ�z�y�Ŀ

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

// �f�h̎���ʼ��
void protocol_init(void) {
    PRINT("�f�h̎���ʼ��\n");
    
    // ��ʼ����ǰ�r�g��Ĭ�Jֵ
    currentTime.year = 2023;
    currentTime.month = 4;
    currentTime.day = 28;
    currentTime.hour = 12;
    currentTime.minute = 0;
    currentTime.second = 0;
}

// Ӌ��У��
uint8_t protocol_calculate_checksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    uint16_t sum = cmdId;
    for (uint8_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

// �l������
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int idx = 0;
    
    // �M�b���
    txBuf[idx++] = PROTOCOL_START_MARKER;
    txBuf[idx++] = cmdId;
    txBuf[idx++] = dataLen;
    
    if (dataLen > 0) {
        memcpy(&txBuf[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuf[idx++] = protocol_calculate_checksum(cmdId, data, dataLen);
    txBuf[idx++] = PROTOCOL_END_MARKER;
    
    // ͨ�^UART�l����CH32V203
    app_uart_tx_data(txBuf, idx);
    
    return SUCCESS;
}

// �l���e�`푑�
uint8_t protocol_send_error(uint8_t originalCmdId, uint8_t errorCode) {
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    return protocol_send_command(CMD_ERROR_ACK, data, 2);
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

// ̎��UART���յ��Ĕ���
void protocol_process_uart_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // ���վ��n�^
    static uint8_t rxIdx = 0;                   // ��ǰ��������
    static uint8_t isStartReceived = 0;         // �Ƿ��յ���ʼ��ӛ
    
    // ̎��ÿ�����յ����ֹ�
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        
        // �z����ʼ��ӛ
        if (byte == PROTOCOL_START_MARKER && !isStartReceived) {
            rxIdx = 0;
            isStartReceived = 1;
            rxBuf[rxIdx++] = byte;
        }
        // �ѽ��յ���ʼ��ӛ
        else if (isStartReceived) {
            // �惦����
            if (rxIdx < sizeof(rxBuf)) {
                rxBuf[rxIdx++] = byte;
            }
            
            // �z���Ƿ��յ��������
            if (byte == PROTOCOL_END_MARKER && rxIdx >= 5) {
                // ��ȡ�f�h�ֶ�
                uint8_t cmdId = rxBuf[1];
                uint8_t dataLen = rxBuf[2];
                uint8_t *pData = &rxBuf[3];
                uint8_t checksum = rxBuf[rxIdx - 2];
                
                // �z�����L���Ƿ����_
                if (rxIdx == dataLen + 5) {
                    // У�͙z��
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, pData, dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // У�ɹ���̎������
                        switch (cmdId) {
                            case CMD_SYNC_TIME:
                                handle_sync_time(pData, dataLen);
                                break;
                                
                            case CMD_REQ_DEVICE_STATUS:
                                handle_device_status_request();
                                break;
                                
                            case CMD_SET_CODE_EVENT:
                                handle_set_code_event(pData, dataLen);
                                break;
                                
                            case CMD_BLOOD_SAMPLE_STATUS:
                                handle_blood_sample_request();
                                break;
                                
                            case CMD_REQ_RESULT:
                                handle_result_request();
                                break;
                                
                            case CMD_REQ_RAW_DATA:
                                handle_raw_data_request();
                                break;
                                
                            default:
                                protocol_send_error(cmdId, ERR_COMMAND_UNSUPPORTED);
                                break;
                        }
                    }
                    else {
                        // У���e�`
                        protocol_send_error(cmdId, ERR_CHECKSUM);
                    }
                }
                else {
                    // �����L���e�`
                    protocol_send_error(rxBuf[1], ERR_DATA_FORMAT);
                }
                
                // ���ý��ՠ�B
                isStartReceived = 0;
                rxIdx = 0;
            }
        }
    }
}

// ��BLE�����D�l��UART
void protocol_handle_ble_data(uint8_t *data, uint16_t len) {
    // ӛ䛽��յ����{�����
    PRINT("BLE->UART: ");
    for (uint16_t i = 0; i < len && i < 16; i++) {
        PRINT("%02X ", data[i]);
    }
    if (len > 16) PRINT("...");
    PRINT("\n");
    
    // �_��������ʽ���_
    if (len >= 5 && data[0] == PROTOCOL_START_MARKER && data[len-1] == PROTOCOL_END_MARKER) {
        // �D�l��UART
        app_uart_tx_data(data, len);
    }
    else {
        PRINT("�e�`��BLE������ʽ\n");
    }
} 