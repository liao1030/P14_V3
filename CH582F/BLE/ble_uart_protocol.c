/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_protocol.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : 藍牙與UART通訊協議實現
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "ble_uart_protocol.h"
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"

/* 全局變量 */
static uint8_t rx_buffer[MAX_FRAME_LENGTH * 2];  // UART接收緩衝區
static uint16_t rx_index = 0;                    // 當前接收位置
static uint8_t tx_buffer[MAX_FRAME_LENGTH * 2];  // 發送緩衝區

/* 藍牙連接標誌 */
static uint8_t ble_connected = 0;

/* 重傳機制變量 */
static uint8_t retry_count = 0;
static uint8_t last_cmd_id = 0;
static uint16_t retry_timeout = 0;

/* 協議狀態機變量 */
static uint8_t frame_parsing = 0;
static uint8_t frame_length = 0;
static uint8_t frame_cmd_id = 0;
static uint8_t frame_checksum = 0;

/*********************************************************************
 * @fn      BLE_Protocol_Init
 *
 * @brief   協議初始化
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
 * @brief   計算封包校驗和
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  計算得到的校驗和
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
 * @brief   打包協議報文
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 * @param   buffer - 輸出緩衝區
 *
 * @return  打包後的總長度
 */
uint16_t BLE_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer)
{
    uint8_t checksum;
    uint16_t index = 0;
    
    /* 計算校驗和 */
    checksum = BLE_Protocol_CalcChecksum(cmd_id, data, length);
    
    /* 組裝報文 */
    buffer[index++] = FRAME_START_BYTE;  // 起始標記
    buffer[index++] = cmd_id;            // 指令ID
    buffer[index++] = length;            // 長度
    
    /* 複製數據區 */
    if (length > 0 && data != NULL) {
        memcpy(&buffer[index], data, length);
        index += length;
    }
    
    buffer[index++] = checksum;          // 校驗和
    buffer[index++] = FRAME_END_BYTE;    // 結束標記
    
    return index;
}

/*********************************************************************
 * @fn      BLE_Protocol_SendResponse
 *
 * @brief   發送協議響應
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void BLE_Protocol_SendResponse(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint16_t frame_length;
    
    /* 打包響應報文 */
    frame_length = BLE_Protocol_PackFrame(cmd_id, data, length, tx_buffer);
    
    /* 通過BLE發送 */
    UART_To_BLE_Forward(tx_buffer, frame_length);
}

/*********************************************************************
 * @fn      BLE_Protocol_SendErrorResponse
 *
 * @brief   發送錯誤響應
 *
 * @param   orig_cmd_id - 原始指令ID
 * @param   error_code - 錯誤代碼
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
 * @brief   解析收到的數據
 *
 * @param   data - 收到的數據指針
 * @param   length - 數據長度
 *
 * @return  0-成功解析一個完整報文, 1-未完成, 2-錯誤
 */
uint8_t BLE_Protocol_Parse(uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint8_t result = 1; // 默認未完成
    
    /* 遍歷收到的數據 */
    for (i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        if (!frame_parsing) { // 等待起始標記
            if (byte == FRAME_START_BYTE) {
                /* 新幀開始 */
                frame_parsing = 1;
                rx_index = 0;
                rx_buffer[rx_index++] = byte;
            }
        } else {
            /* 已經在解析一個幀 */
            rx_buffer[rx_index++] = byte;
            
            if (rx_index == 3) {
                /* 已接收到指令ID和長度 */
                frame_cmd_id = rx_buffer[1];
                frame_length = rx_buffer[2];
            } else if (rx_index >= 5 && rx_index == frame_length + 5) {
                /* 完整幀接收完成 */
                if (byte == FRAME_END_BYTE) {
                    uint8_t calc_checksum;
                    uint8_t recv_checksum = rx_buffer[rx_index - 2];
                    
                    /* 計算校驗和 */
                    calc_checksum = BLE_Protocol_CalcChecksum(frame_cmd_id, &rx_buffer[3], frame_length);
                    
                    /* 檢查校驗和 */
                    if (calc_checksum == recv_checksum) {
                        /* 解析成功, 處理指令 */
                        BLE_To_UART_Forward(rx_buffer, rx_index);
                        
                        result = 0; // 解析成功
                    } else {
                        /* 校驗錯誤 */
                        BLE_Protocol_SendErrorResponse(frame_cmd_id, ERR_CHECKSUM);
                        result = 2; // 解析錯誤
                    }
                } else {
                    /* 結束標記錯誤 */
                    BLE_Protocol_SendErrorResponse(frame_cmd_id, ERR_DATA_FORMAT);
                    result = 2; // 解析錯誤
                }
                
                /* 重置解析狀態 */
                frame_parsing = 0;
            }
            
            /* 檢查緩衝區溢出 */
            if (rx_index >= MAX_FRAME_LENGTH * 2) {
                frame_parsing = 0;
                result = 2; // 解析錯誤
            }
        }
    }
    
    return result;
}

/*********************************************************************
 * @fn      BLE_To_UART_Forward
 *
 * @brief   從BLE接收數據轉發到UART
 *
 * @param   data - 數據指針
 * @param   length - 數據長度
 *
 * @return  none
 */
void BLE_To_UART_Forward(uint8_t *data, uint16_t length)
{
    uint16_t i;
    
    /* 逐字節通過UART1發送到CH32V203 */
    for (i = 0; i < length; i++) {
        UART1_SendByte(data[i]);
    }
}

/*********************************************************************
 * @fn      UART_To_BLE_Forward
 *
 * @brief   從UART接收數據轉發到BLE
 *
 * @param   data - 數據指針
 * @param   length - 數據長度
 *
 * @return  none
 */
void UART_To_BLE_Forward(uint8_t *data, uint16_t length)
{
    /* 檢查是否連接到BLE */
    if (ble_connected) {
        /* 通過BLE發送數據 */
        uint16_t remain = length;
        uint16_t offset = 0;
        uint16_t send_len;
        
        /* 由於MTU限制，可能需要分包發送 */
        while (remain > 0) {
            send_len = (remain > 20) ? 20 : remain;
            
            /* 使用BLE庫函數發送數據 */
            BLE_SendData(&data[offset], send_len);
            
            offset += send_len;
            remain -= send_len;
            
            /* 連續發送間需要延時 */
            DelayMs(30);
        }
    }
}

/*********************************************************************
 * @fn      BLE_UART_SendData
 *
 * @brief   發送數據到UART或BLE
 *
 * @param   data - 數據指針
 * @param   length - 數據長度
 *
 * @return  none
 */
void BLE_UART_SendData(uint8_t *data, uint16_t length)
{
    /* 將數據轉發到UART和BLE */
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
 * @brief   設置BLE連接狀態
 *
 * @param   connected - 連接標誌, 1連接, 0斷開
 *
 * @return  none
 */
void BLE_SetConnectedState(uint8_t connected)
{
    ble_connected = connected;
    
    PRINT("BLE %s\n", connected ? "Connected" : "Disconnected");
} 