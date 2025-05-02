/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : 多功能生化測試儀藍牙與UART通訊協議實現
 ********************************************************************************/

#include "protocol.h"
#include "uart_comm.h"
#include "string.h"
#include "debug.h"

/* 協議相關緩衝區 */
static uint8_t protocol_tx_buffer[UART_BUFFER_SIZE];
static uint8_t protocol_data_buffer[UART_BUFFER_SIZE];

/* 協議初始化狀態 */
static uint8_t protocol_initialized = 0;

/* 當前系統時間 */
static Time_t current_time = {2025, 5, 2, 12, 0, 0};

/* 當前設定的CODE和EVENT */
static uint8_t current_code = 0;
static uint16_t current_event = EVENT_NONE;

/* 當前測量項目 */
static uint16_t current_measure_item = ITEM_GLV;

/*********************************************************************
 * @fn      Protocol_CalcChecksum
 *
 * @brief   計算校驗和
 *
 * @param   cmd - 指令ID
 *          data - 資料指標
 *          length - 資料長度
 *
 * @return  計算出的校驗和
 */
static uint8_t Protocol_CalcChecksum(uint8_t cmd, uint8_t *data, uint8_t length)
{
    uint16_t sum = cmd;  // 從指令ID開始計算
    uint8_t i;
    
    /* 加上所有資料字節 */
    for (i = 0; i < length; i++)
    {
        sum += data[i];
    }
    
    /* 取模256 */
    return (uint8_t)(sum % 256);
}

/*********************************************************************
 * @fn      Protocol_VerifyChecksum
 *
 * @brief   驗證校驗和
 *
 * @param   packet - 封包結構指標
 *
 * @return  0 - 校驗成功, 非0 - 校驗失敗
 */
static uint8_t Protocol_VerifyChecksum(Protocol_Packet_t *packet)
{
    uint8_t checksum = Protocol_CalcChecksum(packet->cmd, packet->data, packet->length);
    
    return (checksum != packet->checksum);
}

/*********************************************************************
 * @fn      Protocol_UART_RxCallback
 *
 * @brief   UART接收回調函數
 *
 * @param   data - 收到的資料指標
 *          length - 資料長度
 *
 * @return  none
 */
static void Protocol_UART_RxCallback(uint8_t *data, uint16_t length)
{
    Protocol_ProcessReceivedData(data, length);
}

/*********************************************************************
 * @fn      Protocol_Init
 *
 * @brief   初始化通訊協議
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_Init(void)
{
    /* 初始化UART */
    UART_Comm_Init();
    
    /* 設置接收回調 */
    UART_SetRxCallback(Protocol_UART_RxCallback);
    
    protocol_initialized = 1;
    
    return 0;
}

/*********************************************************************
 * @fn      Protocol_ParsePacket
 *
 * @brief   解析封包
 *
 * @param   data - 收到的資料指標
 *          length - 資料長度
 *          packet - 封包結構指標
 *
 * @return  0 - 成功, 非0 - 失敗
 */
static uint8_t Protocol_ParsePacket(uint8_t *data, uint16_t length, Protocol_Packet_t *packet)
{
    uint16_t i = 0;
    
    /* 查找起始標記 */
    while (i < length && data[i] != PROTOCOL_START_MARKER)
    {
        i++;
    }
    
    /* 檢查是否找到起始標記 */
    if (i >= length || (length - i) < 4) // 至少需要 起始標記+指令ID+資料長度+校驗和+結束標記
    {
        return 1;  // 沒有找到有效封包
    }
    
    /* 讀取指令ID */
    packet->cmd = data[i + 1];
    
    /* 讀取資料長度 */
    packet->length = data[i + 2];
    
    /* 檢查封包完整性 */
    if (i + 3 + packet->length + 2 > length)
    {
        return 2;  // 資料不完整
    }
    
    /* 讀取資料 */
    packet->data = &data[i + 3];
    
    /* 讀取校驗和 */
    packet->checksum = data[i + 3 + packet->length];
    
    /* 檢查結束標記 */
    if (data[i + 3 + packet->length + 1] != PROTOCOL_END_MARKER)
    {
        return 3;  // 結束標記錯誤
    }
    
    /* 驗證校驗和 */
    if (Protocol_VerifyChecksum(packet))
    {
        return 4;  // 校驗和錯誤
    }
    
    return 0;  // 解析成功
}

/*********************************************************************
 * @fn      Protocol_ProcessReceivedData
 *
 * @brief   處理接收到的數據
 *
 * @param   data - 接收到的數據指標
 *          length - 數據長度
 *
 * @return  0 - 成功處理, 非0 - 處理失敗
 */
uint8_t Protocol_ProcessReceivedData(uint8_t *data, uint16_t length)
{
    Protocol_Packet_t packet;
    uint8_t result;
    Time_t *time_ptr;
    uint8_t code;
    uint16_t event;
    
    /* 解析封包 */
    result = Protocol_ParsePacket(data, length, &packet);
    if (result != 0)
    {
        /* 解析錯誤，返回錯誤碼 */
        if (result == 4) // 校驗和錯誤
        {
            Protocol_SendErrorResponse(data[1], ERR_CHECKSUM);
        }
        else // 其他格式錯誤
        {
            Protocol_SendErrorResponse(data[1], ERR_DATA_FORMAT);
        }
        return result;
    }
    
    /* 根據指令ID處理不同的指令 */
    switch (packet.cmd)
    {
        case CMD_SYNC_TIME:
            /* 同步時間 */
            if (packet.length != 7)
            {
                Protocol_SendErrorResponse(packet.cmd, ERR_DATA_FORMAT);
                break;
            }
            
            /* 更新系統時間 */
            time_ptr = &current_time;
            time_ptr->year = (packet.data[0] << 8) | packet.data[1];
            time_ptr->month = packet.data[2];
            time_ptr->day = packet.data[3];
            time_ptr->hour = packet.data[4];
            time_ptr->minute = packet.data[5];
            time_ptr->second = packet.data[6];
            
            /* 回覆時間同步確認 */
            Protocol_SendSyncTimeAck(0); // 0表示成功
            break;
            
        case CMD_GET_DEVICE_STATUS:
            /* 請求裝置狀態 */
            {
                Device_Status_t status;
                
                /* 填充裝置狀態 */
                status.measure_item = current_measure_item;
                status.strip_status = 0; // 正常狀態, 假設這裡取得實際試片狀態
                status.battery_voltage = 3000; // 假設這裡取得實際電池電壓
                status.temperature = 250; // 假設這裡取得實際溫度(25.0度)
                
                /* 發送裝置狀態回應 */
                Protocol_SendDeviceStatus(&status);
            }
            break;
            
        case CMD_SET_CODE_EVENT:
            /* 設定CODE和EVENT */
            if (packet.length != 3)
            {
                Protocol_SendErrorResponse(packet.cmd, ERR_DATA_FORMAT);
                break;
            }
            
            /* 更新CODE和EVENT */
            code = packet.data[0];
            event = (packet.data[1] << 8) | packet.data[2];
            
            current_code = code;
            current_event = event;
            
            /* 回覆設定確認 */
            Protocol_SendCodeEventAck(0); // 0表示成功
            break;
            
        case CMD_BLOOD_STATUS_REQ:
            /* 檢測狀態請求 */
            {
                /* 假設這裡檢測血液樣本是否足夠 */
                uint8_t blood_detected = 1; // 假設現在有足夠血液樣本
                
                if (blood_detected)
                {
                    /* 回覆血液檢測通知，倒數5秒 */
                    Protocol_SendBloodStatusNotify(5);
                }
                else
                {
                    /* 回覆錯誤 - 測量超時 */
                    Protocol_SendErrorResponse(packet.cmd, ERR_MEASURE_TIMEOUT);
                }
            }
            break;
            
        case CMD_REQUEST_RESULT:
            /* 請求測量結果 */
            {
                Measure_Result_t result;
                
                /* 填充測量結果 */
                result.result_status = 0; // 成功
                result.measure_value = 123; // 假設這裡取得實際測量值
                result.measure_item = current_measure_item;
                result.event = current_event;
                result.code = current_code;
                result.year = current_time.year;
                result.month = current_time.month;
                result.day = current_time.day;
                result.hour = current_time.hour;
                result.minute = current_time.minute;
                result.second = current_time.second;
                result.battery_voltage = 3000; // 假設這裡取得實際電池電壓
                result.temperature = 250; // 假設這裡取得實際溫度(25.0度)
                
                /* 發送測量結果 */
                Protocol_SendMeasureResult(&result);
            }
            break;
            
        case CMD_REQUEST_RAW_DATA:
            /* 請求RAW DATA */
            {
                /* 假設RAW DATA */
                uint8_t raw_data[] = {
                    0x00, 0x32, // RAW DATA長度 (50字節)
                    0x00, 0x00, 0x00, 0x7B, // 測量值123
                    0x00, 0x19, 0x00, 0x03, 0x00, 0x07, // 年月日
                    0x00, 0x0F, 0x00, 0x20, 0x00, 0x3B, // 時分秒
                    0x00, 0x00, 0x00, 0x01, // 測量項目血糖
                    0x0B, 0x01, 0x01, 0x19, // 電壓和溫度
                    0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, // 原始ADC值
                    0x07, 0xD0, 0x03, 0xE8, 0x03, 0x20, // 更多ADC值
                    0x01, 0x03, // 測量次數
                    'A', 'A', 'S', '1', '2', '3', '4', '5', '8', 0x00, // ID
                    'A', 'A', 'S', '1', '2', '3', '4', '5', '8', 0x00, // 序號
                };
                
                /* 發送RAW DATA */
                Protocol_SendRawData(raw_data, sizeof(raw_data));
            }
            break;
            
        default:
            /* 不支援的指令 */
            Protocol_SendErrorResponse(packet.cmd, ERR_CMD_NOT_SUPPORTED);
            break;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      Protocol_SendPacket
 *
 * @brief   發送協議封包
 *
 * @param   cmd - 指令ID
 *          data - 資料指標
 *          length - 資料長度
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendPacket(uint8_t cmd, uint8_t *data, uint8_t length)
{
    uint8_t checksum;
    uint16_t packet_length = 0;
    
    /* 檢查是否已初始化 */
    if (!protocol_initialized)
    {
        return 1;
    }
    
    /* 檢查資料長度 */
    if (length > UART_BUFFER_SIZE - 5) // 5 = 起始標記(1) + 指令ID(1) + 資料長度(1) + 校驗和(1) + 結束標記(1)
    {
        return 2;
    }
    
    /* 計算校驗和 */
    checksum = Protocol_CalcChecksum(cmd, data, length);
    
    /* 組裝封包 */
    protocol_tx_buffer[packet_length++] = PROTOCOL_START_MARKER;
    protocol_tx_buffer[packet_length++] = cmd;
    protocol_tx_buffer[packet_length++] = length;
    
    /* 複製資料 */
    if (length > 0 && data != NULL)
    {
        memcpy(&protocol_tx_buffer[packet_length], data, length);
        packet_length += length;
    }
    
    protocol_tx_buffer[packet_length++] = checksum;
    protocol_tx_buffer[packet_length++] = PROTOCOL_END_MARKER;
    
    /* 通過UART發送 */
    return UART_SendData(protocol_tx_buffer, packet_length);
}

/*********************************************************************
 * @fn      Protocol_SendErrorResponse
 *
 * @brief   發送錯誤回應
 *
 * @param   original_cmd - 原始指令ID
 *          error_code - 錯誤代碼
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendErrorResponse(uint8_t original_cmd, uint8_t error_code)
{
    uint8_t data[2];
    
    data[0] = original_cmd;
    data[1] = error_code;
    
    return Protocol_SendPacket(CMD_ERROR_RESPONSE, data, 2);
}

/*********************************************************************
 * @fn      Protocol_SendDeviceStatus
 *
 * @brief   發送裝置狀態
 *
 * @param   status - 裝置狀態結構指標
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendDeviceStatus(Device_Status_t *status)
{
    uint8_t data[8];
    
    data[0] = (status->measure_item >> 8) & 0xFF;
    data[1] = status->measure_item & 0xFF;
    data[2] = (status->strip_status >> 8) & 0xFF;
    data[3] = status->strip_status & 0xFF;
    data[4] = (status->battery_voltage >> 8) & 0xFF;
    data[5] = status->battery_voltage & 0xFF;
    data[6] = (status->temperature >> 8) & 0xFF;
    data[7] = status->temperature & 0xFF;
    
    return Protocol_SendPacket(CMD_DEVICE_STATUS_ACK, data, 8);
}

/*********************************************************************
 * @fn      Protocol_SendMeasureResult
 *
 * @brief   發送測量結果
 *
 * @param   result - 測量結果結構指標
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendMeasureResult(Measure_Result_t *result)
{
    uint8_t data[25];
    uint8_t index = 0;
    
    /* 填充資料 */
    data[index++] = (result->result_status >> 8) & 0xFF;
    data[index++] = result->result_status & 0xFF;
    data[index++] = (result->measure_value >> 8) & 0xFF;
    data[index++] = result->measure_value & 0xFF;
    data[index++] = (result->measure_item >> 8) & 0xFF;
    data[index++] = result->measure_item & 0xFF;
    data[index++] = (result->event >> 8) & 0xFF;
    data[index++] = result->event & 0xFF;
    data[index++] = result->code;
    data[index++] = (result->year >> 8) & 0xFF;
    data[index++] = result->year & 0xFF;
    data[index++] = (result->month >> 8) & 0xFF;
    data[index++] = result->month & 0xFF;
    data[index++] = (result->day >> 8) & 0xFF;
    data[index++] = result->day & 0xFF;
    data[index++] = (result->hour >> 8) & 0xFF;
    data[index++] = result->hour & 0xFF;
    data[index++] = (result->minute >> 8) & 0xFF;
    data[index++] = result->minute & 0xFF;
    data[index++] = (result->second >> 8) & 0xFF;
    data[index++] = result->second & 0xFF;
    data[index++] = (result->battery_voltage >> 8) & 0xFF;
    data[index++] = result->battery_voltage & 0xFF;
    data[index++] = (result->temperature >> 8) & 0xFF;
    data[index++] = result->temperature & 0xFF;
    
    return Protocol_SendPacket(CMD_RESULT_RESPONSE, data, index);
}

/*********************************************************************
 * @fn      Protocol_SendSyncTimeAck
 *
 * @brief   發送時間同步確認
 *
 * @param   status - 同步狀態 (0表示成功)
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendSyncTimeAck(uint8_t status)
{
    uint8_t data[1];
    
    data[0] = status;
    
    return Protocol_SendPacket(CMD_SYNC_TIME_ACK, data, 1);
}

/*********************************************************************
 * @fn      Protocol_SendCodeEventAck
 *
 * @brief   發送CODE/EVENT設定確認
 *
 * @param   status - 設定狀態 (0表示成功)
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendCodeEventAck(uint8_t status)
{
    uint8_t data[1];
    
    data[0] = status;
    
    return Protocol_SendPacket(CMD_CODE_EVENT_ACK, data, 1);
}

/*********************************************************************
 * @fn      Protocol_SendBloodStatusNotify
 *
 * @brief   發送血液檢測通知
 *
 * @param   countdown - 倒數秒數
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendBloodStatusNotify(uint8_t countdown)
{
    uint8_t data[1];
    
    data[0] = countdown;
    
    return Protocol_SendPacket(CMD_BLOOD_STATUS_NOTIFY, data, 1);
}

/*********************************************************************
 * @fn      Protocol_SendRawData
 *
 * @brief   發送RAW DATA
 *
 * @param   data - RAW DATA指標
 *          length - RAW DATA長度
 *
 * @return  0 - 成功, 非0 - 失敗
 */
uint8_t Protocol_SendRawData(uint8_t *data, uint16_t length)
{
    /* 直接將原始資料做為資料部分傳送 */
    return Protocol_SendPacket(CMD_RAW_DATA_RESPONSE, data, length);
}