/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2023/04/28
 * Description        : P14多功能生化儀讀數藍牙與UART通訊協議處理
 *********************************************************************************
 * Copyright (c) 2023 HMD. All rights reserved.
 *******************************************************************************/

#include "CONFIG.h"
#include "protocol_handler.h"
#include "app_uart.h"
#include "peripheral.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"

// 全局變量
static uint8_t txBuf[MAX_PACKET_SIZE + 6];  // 封包緩衝區
static time_info_t currentTime;             // 當前時間
static uint8_t currentCode = 25;            // 當前試紙校碼
static uint16_t currentEvent = EVENT_NONE;  // 當前事件類型
static uint16_t currentItem = MEASURE_ITEM_GLV; // 當前檢測項目

// 模擬設備狀態
static device_status_t deviceStatus = {
    .measureItem = MEASURE_ITEM_GLV,
    .stripStatus = 0,
    .batteryVoltage = 3000,  // 3.0V
    .temperature = 250       // 25.0°C
};

// 模擬測量結果
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

// 協議處理初始化
void protocol_init(void) {
    PRINT("協議處理初始化\n");
    
    // 初始化當前時間為默認值
    currentTime.year = 2023;
    currentTime.month = 4;
    currentTime.day = 28;
    currentTime.hour = 12;
    currentTime.minute = 0;
    currentTime.second = 0;
}

// 計算校驗和
uint8_t protocol_calculate_checksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    uint16_t sum = cmdId;
    for (uint8_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

// 發送命令
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int idx = 0;
    
    // 組裝封包
    txBuf[idx++] = PROTOCOL_START_MARKER;
    txBuf[idx++] = cmdId;
    txBuf[idx++] = dataLen;
    
    if (dataLen > 0) {
        memcpy(&txBuf[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuf[idx++] = protocol_calculate_checksum(cmdId, data, dataLen);
    txBuf[idx++] = PROTOCOL_END_MARKER;
    
    // 通過UART發送至CH32V203
    app_uart_tx_data(txBuf, idx);
    
    return SUCCESS;
}

// 發送錯誤響應
uint8_t protocol_send_error(uint8_t originalCmdId, uint8_t errorCode) {
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    return protocol_send_command(CMD_ERROR_ACK, data, 2);
}

// 處理同步時間命令
static void handle_sync_time(uint8_t *data, uint8_t dataLen) {
    // 校驗數據長度
    if (dataLen != 7) {
        protocol_send_error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // 解析時間數據
    currentTime.year = (data[0] << 8) | data[1];
    currentTime.month = data[2];
    currentTime.day = data[3];
    currentTime.hour = data[4];
    currentTime.minute = data[5];
    currentTime.second = data[6];
    
    PRINT("同步時間: %d-%02d-%02d %02d:%02d:%02d\n", 
          currentTime.year, currentTime.month, currentTime.day,
          currentTime.hour, currentTime.minute, currentTime.second);
    
    // 回應確認
    uint8_t response = 0x00;  // 成功
    protocol_send_command(CMD_SYNC_TIME_ACK, &response, 1);
}

// 處理請求設備狀態命令
static void handle_device_status_request(void) {
    uint8_t data[8];
    
    // 組裝數據
    data[0] = (deviceStatus.measureItem >> 8) & 0xFF;
    data[1] = deviceStatus.measureItem & 0xFF;
    data[2] = (deviceStatus.stripStatus >> 8) & 0xFF;
    data[3] = deviceStatus.stripStatus & 0xFF;
    data[4] = (deviceStatus.batteryVoltage >> 8) & 0xFF;
    data[5] = deviceStatus.batteryVoltage & 0xFF;
    data[6] = (deviceStatus.temperature >> 8) & 0xFF;
    data[7] = deviceStatus.temperature & 0xFF;
    
    // 發送回應
    protocol_send_command(CMD_DEVICE_STATUS_ACK, data, 8);
    
    PRINT("設備狀態請求回應: 項目=%d, 狀態=%d, 電壓=%dmV, 溫度=%.1f°C\n",
          deviceStatus.measureItem, deviceStatus.stripStatus,
          deviceStatus.batteryVoltage, deviceStatus.temperature / 10.0);
}

// 處理設定CODE和EVENT命令
static void handle_set_code_event(uint8_t *data, uint8_t dataLen) {
    // 校驗數據長度
    if (dataLen != 3) {
        protocol_send_error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // 解析CODE和EVENT
    currentCode = data[0];
    currentEvent = (data[1] << 8) | data[2];
    
    PRINT("設定CODE=%d, EVENT=%d\n", currentCode, currentEvent);
    
    // 回應確認
    uint8_t response = 0x00;  // 成功
    protocol_send_command(CMD_CODE_EVENT_ACK, &response, 1);
    
    // 更新測量結果中的CODE和EVENT
    measureResult.code = currentCode;
    measureResult.event = currentEvent;
}

// 處理血液檢測狀態請求
static void handle_blood_sample_request(void) {
    // 這裡模擬檢測到血液，倒計時5秒
    uint8_t countdown = 5;
    
    PRINT("血液檢測通知: 倒計時%d秒\n", countdown);
    
    // 發送回應
    protocol_send_command(CMD_BLOOD_SAMPLE_ACK, &countdown, 1);
}

// 處理請求檢測結果命令
static void handle_result_request(void) {
    uint8_t data[25];
    int idx = 0;
    
    // 使用當前時間更新測量結果時間
    measureResult.time = currentTime;
    
    // 組裝數據
    data[idx++] = (measureResult.resultStatus >> 8) & 0xFF;
    data[idx++] = measureResult.resultStatus & 0xFF;
    data[idx++] = (measureResult.measureValue >> 8) & 0xFF;
    data[idx++] = measureResult.measureValue & 0xFF;
    data[idx++] = (measureResult.measureItem >> 8) & 0xFF;
    data[idx++] = measureResult.measureItem & 0xFF;
    data[idx++] = (measureResult.event >> 8) & 0xFF;
    data[idx++] = measureResult.event & 0xFF;
    data[idx++] = measureResult.code;
    
    // 時間
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
    
    // 電池和溫度
    data[idx++] = (measureResult.batteryVoltage >> 8) & 0xFF;
    data[idx++] = measureResult.batteryVoltage & 0xFF;
    data[idx++] = (measureResult.temperature >> 8) & 0xFF;
    data[idx++] = measureResult.temperature & 0xFF;
    
    // 發送回應
    protocol_send_command(CMD_RESULT_ACK, data, idx);
    
    PRINT("檢測結果: 狀態=%d, 值=%d, 項目=%d, 事件=%d, 校碼=%d\n",
          measureResult.resultStatus, measureResult.measureValue,
          measureResult.measureItem, measureResult.event, measureResult.code);
}

// 處理請求RAW DATA命令
static void handle_raw_data_request(void) {
    uint8_t rawData[50];
    
    // 填充模擬的RAW DATA
    for (int i = 0; i < 50; i++) {
        rawData[i] = i;
    }
    
    uint8_t data[52];
    data[0] = 0;  // 高位
    data[1] = 50; // 長度
    memcpy(&data[2], rawData, 50);
    
    // 發送回應
    protocol_send_command(CMD_RAW_DATA_ACK, data, 52);
    
    PRINT("RAW DATA請求已響應\n");
}

// 處理UART接收到的數據
void protocol_process_uart_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // 接收緩衝區
    static uint8_t rxIdx = 0;                   // 當前接收索引
    static uint8_t isStartReceived = 0;         // 是否收到起始標記
    
    // 處理每個接收到的字節
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        
        // 檢查起始標記
        if (byte == PROTOCOL_START_MARKER && !isStartReceived) {
            rxIdx = 0;
            isStartReceived = 1;
            rxBuf[rxIdx++] = byte;
        }
        // 已經收到起始標記
        else if (isStartReceived) {
            // 存儲數據
            if (rxIdx < sizeof(rxBuf)) {
                rxBuf[rxIdx++] = byte;
            }
            
            // 檢查是否收到完整封包
            if (byte == PROTOCOL_END_MARKER && rxIdx >= 5) {
                // 提取協議字段
                uint8_t cmdId = rxBuf[1];
                uint8_t dataLen = rxBuf[2];
                uint8_t *pData = &rxBuf[3];
                uint8_t checksum = rxBuf[rxIdx - 2];
                
                // 檢查封包長度是否正確
                if (rxIdx == dataLen + 5) {
                    // 校驗和檢查
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, pData, dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // 校驗成功，處理命令
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
                        // 校驗和錯誤
                        protocol_send_error(cmdId, ERR_CHECKSUM);
                    }
                }
                else {
                    // 數據長度錯誤
                    protocol_send_error(rxBuf[1], ERR_DATA_FORMAT);
                }
                
                // 重置接收狀態
                isStartReceived = 0;
                rxIdx = 0;
            }
        }
    }
}

// 將BLE數據轉發到UART
void protocol_handle_ble_data(uint8_t *data, uint16_t len) {
    // 記錄接收到的藍牙封包
    PRINT("BLE->UART: ");
    for (uint16_t i = 0; i < len && i < 16; i++) {
        PRINT("%02X ", data[i]);
    }
    if (len > 16) PRINT("...");
    PRINT("\n");
    
    // 確保數據格式正確
    if (len >= 5 && data[0] == PROTOCOL_START_MARKER && data[len-1] == PROTOCOL_END_MARKER) {
        // 轉發到UART
        app_uart_tx_data(data, len);
    }
    else {
        PRINT("錯誤的BLE數據格式\n");
    }
} 