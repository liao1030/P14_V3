/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_handler.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2024/05/08
 * Description        : P14多功能生化儀讀數藍牙與UART通訊協議處理
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

// 全局變量
static uint8_t txBuf[MAX_PACKET_SIZE + 6];  // 封包緩衝區
static time_info_t currentTime;             // 當前時間
static uint8_t currentCode = 25;            // 當前試紙校碼
static uint16_t currentEvent = EVENT_NONE;  // 當前事件類型
static uint16_t currentItem = MEASURE_ITEM_GLV; // 當前檢測項目

// 重試相關參數
static uint8_t retryCount = 0;              // 重試次數
static uint8_t lastCmdId = 0;               // 上一次命令ID
static uint8_t lastData[MAX_PACKET_SIZE];   // 上一次命令數據
static uint8_t lastDataLen = 0;             // 上一次命令數據長度
static uint32_t lastSendTime = 0;           // 上一次發送時間
static uint8_t waitingResponse = 0;         // 正在等待響應標誌

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

/*********************************************************************
 * @fn      protocol_init
 *
 * @brief   協議處理初始化
 *
 * @return  none
 */
void protocol_init(void) {
    PRINT("協議處理初始化\n");
    
    // 初始化當前時間為默認值
    currentTime.year = 2023;
    currentTime.month = 4;
    currentTime.day = 28;
    currentTime.hour = 12;
    currentTime.minute = 0;
    currentTime.second = 0;
    
    // 初始化重試相關參數
    retryCount = 0;
    lastCmdId = 0;
    lastDataLen = 0;
    lastSendTime = 0;
    waitingResponse = 0;
}

/*********************************************************************
 * @fn      protocol_calculate_checksum
 *
 * @brief   計算校驗和
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  校驗和
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
 * @brief   發送命令到UART
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  狀態
 */
uint8_t protocol_send_command(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int idx = 0;
    
    // 保存命令以便重試
    lastCmdId = cmdId;
    if (dataLen > 0 && data != NULL) {
        memcpy(lastData, data, dataLen);
    }
    lastDataLen = dataLen;
    lastSendTime = SYS_GetSysTickCnt();
    waitingResponse = 1;
    
    // 組裝封包
    txBuf[idx++] = PROTOCOL_START_MARKER;
    txBuf[idx++] = cmdId;
    txBuf[idx++] = dataLen;
    
    if (dataLen > 0 && data != NULL) {
        memcpy(&txBuf[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuf[idx++] = protocol_calculate_checksum(cmdId, data, dataLen);
    txBuf[idx++] = PROTOCOL_END_MARKER;
    
    // 通過UART發送至CH32V203
    app_uart_tx_data(txBuf, idx);
    
    PRINT("發送封包: ID=0x%02X, 長度=%d\n", cmdId, dataLen);
    
    return SUCCESS;
}

/*********************************************************************
 * @fn      protocol_send_error
 *
 * @brief   發送錯誤響應
 *
 * @param   originalCmdId - 原始命令ID
 * @param   errorCode - 錯誤代碼
 *
 * @return  狀態
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
 * @brief   將收到的UART數據轉發至BLE
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  none
 */
void protocol_forward_to_ble(uint8_t cmdId, uint8_t *data, uint8_t dataLen) {
    int totalLen = dataLen + 5; // 起始標記(1) + 命令ID(1) + 長度(1) + 數據(dataLen) + 校驗和(1) + 結束標記(1)
    uint8_t packet[MAX_PACKET_SIZE + 6];
    
    // 組裝完整的協議封包
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
    
    // 通過BLE發送給手機
    on_uart_to_ble_send(packet, idx);
    
    PRINT("轉發至BLE: ID=0x%02X, 長度=%d\n", cmdId, dataLen);
    
    // 如果是對命令的回應，則清除等待響應標誌
    if ((cmdId & 0x80) == 0x80 || cmdId == CMD_ERROR_ACK) {
        waitingResponse = 0;
        retryCount = 0;
    }
}

/*********************************************************************
 * @fn      protocol_retry_command
 *
 * @brief   重試發送命令
 *
 * @return  none
 */
void protocol_retry_command(void) {
    // 檢查是否需要重試
    if (waitingResponse && SYS_GetSysTickCnt() - lastSendTime > (500 * (retryCount + 1))) {
        if (retryCount < 3) {
            // 重發命令
            PRINT("重試命令: ID=0x%02X, 次數=%d\n", lastCmdId, retryCount + 1);
            protocol_send_command(lastCmdId, lastData, lastDataLen);
            retryCount++;
        } else {
            // 超過重試次數，報告通訊錯誤
            PRINT("命令超時，通訊錯誤: ID=0x%02X\n", lastCmdId);
            
            // 構造錯誤封包
            uint8_t errorData[2];
            errorData[0] = lastCmdId;
            errorData[1] = ERR_COMM_ERROR;
            
            // 轉發給手機
            protocol_forward_to_ble(CMD_ERROR_ACK, errorData, 2);
            
            // 重置重試狀態
            waitingResponse = 0;
            retryCount = 0;
        }
    }
}

/*********************************************************************
 * @fn      protocol_process_uart_data
 *
 * @brief   處理從UART收到的數據
 *
 * @param   data - 數據
 * @param   len - 長度
 *
 * @return  none
 */
void protocol_process_uart_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // 接收緩衝區
    static uint8_t rxIdx = 0;                   // 當前接收索引
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
    
    // 處理接收到的每個字節
    for (uint16_t i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        // 協議狀態機
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
                    // 數據長度超出上限，重置狀態
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
                    // 檢查校驗和
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, &rxBuf[3], dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // 校驗成功，將數據轉發至BLE
                        protocol_forward_to_ble(cmdId, &rxBuf[3], dataLen);
                    } else {
                        // 校驗錯誤
                        PRINT("校驗和錯誤: 收到=0x%02X, 計算=0x%02X\n", checksum, calculatedChecksum);
                        protocol_send_error(cmdId, ERR_CHECKSUM);
                    }
                }
                
                // 重置狀態
                rxIdx = 0;
                state = STATE_IDLE;
                break;
        }
        
        // 防止緩衝區溢出
        if (rxIdx >= sizeof(rxBuf)) {
            rxIdx = 0;
            state = STATE_IDLE;
        }
    }
}

/*********************************************************************
 * @fn      protocol_handle_ble_data
 *
 * @brief   處理從BLE收到的數據
 *
 * @param   data - 數據
 * @param   len - 長度
 *
 * @return  none
 */
void protocol_handle_ble_data(uint8_t *data, uint16_t len) {
    static uint8_t rxBuf[MAX_PACKET_SIZE + 6];  // 接收緩衝區
    static uint8_t rxIdx = 0;                   // 當前接收索引
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
    
    // 處理接收到的每個字節
    for (uint16_t i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        // 協議狀態機
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
                    // 數據長度超出上限，重置狀態
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
                    // 檢查校驗和
                    uint8_t calculatedChecksum = protocol_calculate_checksum(cmdId, &rxBuf[3], dataLen);
                    
                    if (calculatedChecksum == checksum) {
                        // 校驗成功，將數據轉發至UART
                        protocol_send_command(cmdId, &rxBuf[3], dataLen);
                    } else {
                        // 校驗錯誤
                        PRINT("校驗和錯誤: 收到=0x%02X, 計算=0x%02X\n", checksum, calculatedChecksum);
                        uint8_t errorData[2];
                        errorData[0] = cmdId;
                        errorData[1] = ERR_CHECKSUM;
                        protocol_forward_to_ble(CMD_ERROR_ACK, errorData, 2);
                    }
                }
                
                // 重置狀態
                rxIdx = 0;
                state = STATE_IDLE;
                break;
        }
        
        // 防止緩衝區溢出
        if (rxIdx >= sizeof(rxBuf)) {
            rxIdx = 0;
            state = STATE_IDLE;
        }
    }
}

/*********************************************************************
 * @fn      protocol_timer_event
 *
 * @brief   協議定時器事件處理
 *
 * @return  none
 */
void protocol_timer_event(void) {
    // 檢查是否需要重試發送命令
    protocol_retry_command();
}

/*********************************************************************
 * @fn      on_uart_to_ble_send
 *
 * @brief   從UART轉發數據到BLE
 *
 * @param   data - 數據
 * @param   len - 長度
 *
 * @return  none
 */
void on_uart_to_ble_send(uint8_t *data, uint16_t len) {
    // 通過BLE發送
    ble_uart_notify_data(data, len);
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