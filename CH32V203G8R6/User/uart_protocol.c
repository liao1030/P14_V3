/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2025/04/28
 * Description        : UART通訊協議實現
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

#include "uart_protocol.h"
#include <string.h>
#include "debug.h"

/* 全局變數 */
static uint8_t tx_buffer[MAX_FRAME_LENGTH * 2];  // 發送緩衝區

/* 測量參數 */
Measure_Param_t measure_param = {0};

/* RTC時間 */
RTC_TimeStruct_t rtc_time = {0};

/* 重傳機制變數 */
static uint8_t retry_count = 0;
static uint8_t last_cmd_id = 0;
static uint16_t retry_timeout = 0;

/* 設備狀態 */
Device_Status_t device_status = {0};

/* 測量結果 */
Measure_Result_t measure_result = {0};

/* RAW數據 */
uint8_t raw_data[MAX_DATA_LENGTH];
uint16_t raw_data_length = 0;

/* 採血狀態 */
uint8_t blood_sampling_active = 0;
uint8_t blood_countdown_seconds = 0;

/* 外部函數聲明 */
extern void HandleProtocolCommand(uint8_t cmd_id, uint8_t *data, uint8_t length);

/* 試片相關函數宣告 */
void UART_Protocol_HandleStartT1Measurement(uint8_t *data, uint8_t length);
void UART_Protocol_HandleStripDetected(uint8_t *data, uint8_t length);

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   協議初始化
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    retry_count = 0;
    retry_timeout = 0;
    
    /* 初始化設備狀態 */
    device_status.measure_type = MEASURE_TYPE_GLV;
    device_status.strip_status = 0;
    device_status.battery_voltage = 3000; // 3.0V
    device_status.temperature = 250;      // 25.0°C
    
    /* 初始化測量參數 */
    measure_param.code = 0;
    measure_param.event = UART_EVENT_NONE;
    
    /* 初始化採血狀態 */
    blood_sampling_active = 0;
    blood_countdown_seconds = 0;
    
    /* 初始化RAW數據 */
    raw_data_length = 0;
    
    printf("UART Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      UART_Protocol_CalcChecksum
 *
 * @brief   計算封包校驗和
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  計算得到的校驗和
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
 * @brief   打包協議報文
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 * @param   buffer - 輸出緩衝區
 *
 * @return  打包後的總長度
 */
uint16_t UART_Protocol_PackFrame(uint8_t cmd_id, uint8_t *data, uint8_t length, uint8_t *buffer)
{
    uint8_t checksum;
    uint16_t index = 0;
    
    /* 計算校驗和 */
    checksum = UART_Protocol_CalcChecksum(cmd_id, data, length);
    
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
 * @fn      UART_Protocol_SendFrame
 *
 * @brief   發送協議報文
 *
 * @param   cmd_id - 指令ID
 * @param   data - 數據區指針
 * @param   length - 數據區長度
 *
 * @return  none
 */
void UART_Protocol_SendFrame(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    uint16_t frame_length;
    
    /* 打包報文 */
    frame_length = UART_Protocol_PackFrame(cmd_id, data, length, tx_buffer);
    
    /* 通過UART2發送 */
    for (uint16_t i = 0; i < frame_length; i++) {
        USART_SendData(USART2, tx_buffer[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    /* 記錄上一條命令 */
    last_cmd_id = cmd_id;
}

/*********************************************************************
 * @fn      UART_Protocol_SendErrorResponse
 *
 * @brief   發送錯誤響應
 *
 * @param   orig_cmd_id - 原始指令ID
 * @param   error_code - 錯誤代碼
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
 * @brief   解析接收到的數據
 *
 * @param   data - 接收到的數據指針
 *          length - 數據長度
 *
 * @return  0-成功解析一個完整命令, 1-未完成, 2-錯誤
 */
uint8_t UART_Protocol_Parse(uint8_t *data, uint16_t length)
{
    static uint8_t rx_buffer[MAX_FRAME_LENGTH];  // 接收緩衝區
    static uint8_t rx_index = 0;                // 當前接收位置
    static uint8_t frame_parsing = 0;           // 是否正在解析幀
    static uint8_t frame_cmd_id = 0;            // 當前命令ID
    static uint8_t frame_length = 0;            // 當前數據長度
    uint16_t i;
    uint8_t result = 1; // 默認未完成
    
    /* 遍歷接收到的數據 */
    for (i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        if (!frame_parsing) { // 等待起始標記
            if (byte == FRAME_START_BYTE) {
                /* 開始新的幀 */
                frame_parsing = 1;
                rx_index = 0;
                rx_buffer[rx_index++] = byte;
            }
        } else {
            /* 已經在解析一個幀 */
            rx_buffer[rx_index++] = byte;
            
            if (rx_index == 3) {
                /* 已經接收到命令ID和長度 */
                frame_cmd_id = rx_buffer[1];
                frame_length = rx_buffer[2];
            } else if (rx_index >= 5 && rx_index == frame_length + 5) {
                /* 已接收到完整幀 */
                if (byte == FRAME_END_BYTE) {
                    uint8_t calc_checksum;
                    uint8_t recv_checksum = rx_buffer[rx_index - 2];
                    
                    /* 計算校驗和 */
                    calc_checksum = UART_Protocol_CalcChecksum(frame_cmd_id, &rx_buffer[3], frame_length);
                    
                    /* 檢查校驗和 */
                    if (calc_checksum == recv_checksum) {
                        /* 解析成功，處理命令 */
                        HandleProtocolCommand(frame_cmd_id, &rx_buffer[3], frame_length);
                        
                        result = 0; // 解析成功
                    } else {
                        /* 校驗錯誤 */
                        UART_Protocol_SendErrorResponse(frame_cmd_id, ERR_CHECKSUM);
                        result = 2; // 解析錯誤
                    }
                } else {
                    /* 結束標記錯誤 */
                    UART_Protocol_SendErrorResponse(frame_cmd_id, ERR_DATA_FORMAT);
                    result = 2; // 解析錯誤
                }
                
                /* 重置解析狀態 */
                frame_parsing = 0;
            }
            
            /* 檢查緩衝區溢出 */
            if (rx_index >= MAX_FRAME_LENGTH) {
                frame_parsing = 0;
                result = 2; // 解析錯誤
            }
        }
    }
    
    return result;
}

/*********************************************************************
 * @fn      UART_Protocol_ProcessCommand
 *
 * @brief   處理接收到的協議命令
 *
 * @param   cmd_id - 命令ID
 * @param   data - 數據內容
 * @param   length - 數據長度
 *
 * @return  none
 */
void UART_Protocol_ProcessCommand(uint8_t cmd_id, uint8_t *data, uint8_t length)
{
    printf("Received command: %02X, length=%d\r\n", cmd_id, length);
    
    /* 根據命令類型分別處理 */
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
            /* 不支持的命令 */
            UART_Protocol_SendErrorResponse(cmd_id, ERR_COMMAND_NOTSUPPORT);
            break;
    }
}

/*********************************************************************
 * @fn      UART_Protocol_HandleStartT1Measurement
 *
 * @brief   處理開始T1測量命令
 *
 * @param   data - 數據內容
 * @param   length - 數據長度
 *
 * @return  none
 */
void UART_Protocol_HandleStartT1Measurement(uint8_t *data, uint8_t length)
{
    /* 委派給主程式中的處理函數 */
    extern void HandleStartT1Measurement(uint8_t *data, uint8_t length);
    HandleStartT1Measurement(data, length);
}

/*********************************************************************
 * @fn      UART_Protocol_HandleStripDetected
 *
 * @brief   處理試片檢測通知
 *
 * @param   data - 數據內容
 * @param   length - 數據長度
 *
 * @return  none
 */
void UART_Protocol_HandleStripDetected(uint8_t *data, uint8_t length)
{
    /* 委派給主程式中的處理函數 */
    extern void HandleStripDetected(uint8_t *data, uint8_t length);
    HandleStripDetected(data, length);
} 