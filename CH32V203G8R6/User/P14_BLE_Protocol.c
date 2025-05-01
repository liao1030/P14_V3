/**
 * @file P14_BLE_Protocol.c
 * @brief 多功能生化檢測儀藍牙UART通訊協議實現
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "P14_BLE_Protocol.h"
#include "P14_Parameter_Table.h"
#include "P14_Flash_Storage.h"
#include "debug.h"
#include "string.h"
#include "ch32v20x.h"

/* 協議解析狀態 */
typedef enum {
    PROTOCOL_STATE_IDLE,
    PROTOCOL_STATE_COMMAND,
    PROTOCOL_STATE_LENGTH,
    PROTOCOL_STATE_DATA,
    PROTOCOL_STATE_CHECKSUM,
    PROTOCOL_STATE_END
} ProtocolState_TypeDef;

/* 全局變數 */
static ProtocolState_TypeDef g_protocolState = PROTOCOL_STATE_IDLE;
static BLEPacket_TypeDef g_rxPacket;
static uint8_t g_dataIndex = 0;
static BLEPacket_TypeDef g_txPacket;

/* 藍牙UART串口函數 - 由外部實現 */
extern void UART1_SendData(uint8_t *data, uint16_t len);

/**
 * @brief 計算校驗和
 * 
 * @param command 命令ID
 * @param data 資料區指針
 * @param length 資料長度
 * @return uint8_t 校驗和
 */
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length)
{
    uint16_t sum = command;
    
    for (uint8_t i = 0; i < length; i++) {
        sum += data[i];
    }
    
    /* 取模256 */
    return (uint8_t)(sum & 0xFF);
}

/**
 * @brief 創建協議封包
 * 
 * @param packet 封包結構指針
 * @param command 命令ID
 * @param data 資料區指針
 * @param dataLen 資料長度
 * @return uint8_t 是否成功
 */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen)
{
    if (dataLen > MAX_DATA_SIZE) {
        return 0; // 資料長度超過最大值
    }
    
    packet->start = PROTOCOL_START_MARKER;
    packet->command = command;
    packet->length = dataLen;
    
    if (data != NULL && dataLen > 0) {
        memcpy(packet->data, data, dataLen);
    }
    
    packet->checksum = BLE_CalculateChecksum(command, packet->data, dataLen);
    packet->end = PROTOCOL_END_MARKER;
    
    return 1;
}

/**
 * @brief 驗證封包的完整性
 * 
 * @param packet 封包結構指針
 * @return uint8_t 是否有效
 */
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet)
{
    /* 檢查封包頭尾標記 */
    if (packet->start != PROTOCOL_START_MARKER || packet->end != PROTOCOL_END_MARKER) {
        return 0;
    }
    
    /* 檢查校驗和 */
    uint8_t calculatedChecksum = BLE_CalculateChecksum(packet->command, packet->data, packet->length);
    
    if (calculatedChecksum != packet->checksum) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief 發送錯誤回應
 * 
 * @param originalCmd 原始命令ID
 * @param errorCode 錯誤代碼
 */
void BLE_SendErrorResponse(uint8_t originalCmd, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = originalCmd;
    data[1] = errorCode;
    
    BLE_PacketCreate(&g_txPacket, CMD_ERROR_ACK, data, 2);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理同步時間命令
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_HandleSyncTime(uint8_t *data, uint8_t length)
{
    RTC_DateTimeTypeDef rtcTime;
    uint8_t responseData = 0x00; // 成功狀態
    
    if (length != 7) {
        BLE_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* 解析時間資料 */
    uint16_t year = (data[0] << 8) | data[1];
    rtcTime.Month = data[2];
    rtcTime.Day = data[3];
    rtcTime.Hour = data[4];
    rtcTime.Min = data[5];
    rtcTime.Sec = data[6];
    rtcTime.Year = year - 2000; // 轉換為偏移量
    
    /* 時間範圍驗證 */
    if (rtcTime.Month < 1 || rtcTime.Month > 12 ||
        rtcTime.Day < 1 || rtcTime.Day > 31 ||
        rtcTime.Hour > 23 || rtcTime.Min > 59 || rtcTime.Sec > 59) {
        BLE_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* 設定RTC時間 - 此處根據CH32V20x庫實現 */
    // RTC_SetTime(&rtcTime); // 需要根據實際RTC庫函數調用
    
    /* 發送成功響應 */
    BLE_PacketCreate(&g_txPacket, CMD_SYNC_TIME_ACK, &responseData, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理請求裝置狀態命令
 */
void BLE_HandleGetDeviceStatus(void)
{
    DeviceStatus_TypeDef status;
    uint8_t responseData[8];
    BasicSystemBlock systemParams;
    
    /* 讀取系統參數 */
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &systemParams, sizeof(BasicSystemBlock))) {
        BLE_SendErrorResponse(CMD_GET_DEVICE_STATUS, ERR_HARDWARE);
        return;
    }
    
    /* 讀取電池電壓和溫度 - 實際應用中需要替換為真實測量函數 */
    uint16_t batteryVoltage = 2817; // 示例: 2.817V
    uint16_t temperature = 281;     // 示例: 28.1度
    
    /* 填充狀態資訊 */
    status.testType = (uint16_t)systemParams.stripType;
    status.stripStatus = 0x0000; // 假設試紙正常
    status.batteryVoltage = batteryVoltage;
    status.temperature = temperature;
    
    /* 檢查電池電壓是否過低 */
    if (batteryVoltage < 2200) { // 假設2.2V是低電壓閾值
        status.stripStatus = ERR_BATTERY_LOW;
    }
    
    /* 檢查溫度是否在有效範圍內 */
    if (temperature > 350) { // 35.0度
        status.stripStatus = ERR_TEMP_TOO_HIGH;
    } else if (temperature < 150) { // 15.0度
        status.stripStatus = ERR_TEMP_TOO_LOW;
    }
    
    /* 將狀態結構轉換為資料陣列 */
    responseData[0] = (status.testType >> 8) & 0xFF;
    responseData[1] = status.testType & 0xFF;
    responseData[2] = (status.stripStatus >> 8) & 0xFF;
    responseData[3] = status.stripStatus & 0xFF;
    responseData[4] = (status.batteryVoltage >> 8) & 0xFF;
    responseData[5] = status.batteryVoltage & 0xFF;
    responseData[6] = (status.temperature >> 8) & 0xFF;
    responseData[7] = status.temperature & 0xFF;
    
    /* 發送狀態回應 */
    BLE_PacketCreate(&g_txPacket, CMD_DEVICE_STATUS_ACK, responseData, 8);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理設定CODE和EVENT參數命令
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_HandleSetCodeEvent(uint8_t *data, uint8_t length)
{
    uint8_t responseData = 0x00; // 成功狀態
    
    if (length != 3) {
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // uint8_t code = data[0]; // 未使用的變數，CODE參數暫不處理
    uint16_t event = (data[1] << 8) | data[2];
    
    /* 驗證參數範圍 */
    if (event > 3) { // 假設事件類型範圍是0-3
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    /* 更新系統參數 */
    BasicSystemBlock systemParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &systemParams, sizeof(BasicSystemBlock))) {
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_HARDWARE);
        return;
    }
    
    systemParams.defaultEvent = (uint8_t)event;
    
    if (!PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &systemParams, sizeof(BasicSystemBlock))) {
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_HARDWARE);
        return;
    }
    
    /* 發送成功響應 */
    BLE_PacketCreate(&g_txPacket, CMD_CODE_EVENT_ACK, &responseData, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理檢測血液狀態命令
 */
void BLE_HandleBloodSampleStatus(void)
{
    /* 這裡需要與實際的檢測硬體交互 */
    /* 該函數應該檢查是否已經檢測到血液樣本 */
    /* 在實際應用中，可能需要啟動定時器定期檢查 */
    
    /* 假設現在已經檢測到血液樣本，倒數5秒 */
    uint8_t countdownSeconds = 5;
    
    /* 發送通知 */
    BLE_PacketCreate(&g_txPacket, CMD_BLOOD_SAMPLE_ACK, &countdownSeconds, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理請求測試結果命令
 */
void BLE_HandleGetTestResult(void)
{
    TestResult_TypeDef result;
    // RTC_DateTimeTypeDef rtcTime; // 暫時未使用的變數
    uint8_t responseData[25];
    
    /* 獲取測試記錄 */
    TestRecord record;
    if (!PARAM_GetTestRecord(0, &record)) {
        BLE_SendErrorResponse(CMD_GET_TEST_RESULT, ERR_HARDWARE);
        return;
    }
    
    /* 填充結果結構 */
    result.resultStatus = 0x0000; // 成功
    result.testValue = record.resultValue;
    result.testType = (uint16_t)record.testType;
    result.event = (uint16_t)record.event;
    result.code = record.stripCode;
    
    /* 從記錄中獲取時間 */
    result.year = 2000 + record.year; // 假設年份格式為偏移量
    result.month = record.month;
    result.date = record.date;
    result.hour = record.hour;
    result.minute = record.minute;
    result.second = record.second;
    
    /* 讀取電池電壓和溫度 - 實際應用中需要替換為真實測量函數 */
    result.batteryVoltage = 2817; // 示例: 2.817V
    result.temperature = 281;     // 示例: 28.1度
    
    /* 將結果結構轉換為資料陣列 */
    responseData[0] = (result.resultStatus >> 8) & 0xFF;
    responseData[1] = result.resultStatus & 0xFF;
    responseData[2] = (result.testValue >> 8) & 0xFF;
    responseData[3] = result.testValue & 0xFF;
    responseData[4] = (result.testType >> 8) & 0xFF;
    responseData[5] = result.testType & 0xFF;
    responseData[6] = (result.event >> 8) & 0xFF;
    responseData[7] = result.event & 0xFF;
    responseData[8] = result.code;
    responseData[9] = (result.year >> 8) & 0xFF;
    responseData[10] = result.year & 0xFF;
    responseData[11] = (result.month >> 8) & 0xFF;
    responseData[12] = result.month & 0xFF;
    responseData[13] = (result.date >> 8) & 0xFF;
    responseData[14] = result.date & 0xFF;
    responseData[15] = (result.hour >> 8) & 0xFF;
    responseData[16] = result.hour & 0xFF;
    responseData[17] = (result.minute >> 8) & 0xFF;
    responseData[18] = result.minute & 0xFF;
    responseData[19] = (result.second >> 8) & 0xFF;
    responseData[20] = result.second & 0xFF;
    responseData[21] = (result.batteryVoltage >> 8) & 0xFF;
    responseData[22] = result.batteryVoltage & 0xFF;
    responseData[23] = (result.temperature >> 8) & 0xFF;
    responseData[24] = result.temperature & 0xFF;
    
    /* 發送結果回應 */
    BLE_PacketCreate(&g_txPacket, CMD_TEST_RESULT_ACK, responseData, 25);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理請求RAW DATA命令
 */
void BLE_HandleGetRawData(void)
{
    /* 在實際應用中，這需要從測量模組獲取原始數據 */
    /* 此處僅作示例 */
    
    uint8_t rawData[50];
    uint16_t rawDataLength = 50;
    
    /* 填充RAW DATA (這裡只是填充示例數據) */
    for (int i = 0; i < 50; i++) {
        rawData[i] = i;
    }
    
    /* 資料長度信息 (2 bytes) */
    uint8_t responseData[52]; // 2 bytes長度 + 50 bytes原始數據
    responseData[0] = (rawDataLength >> 8) & 0xFF;
    responseData[1] = rawDataLength & 0xFF;
    
    /* 複製原始數據 */
    memcpy(&responseData[2], rawData, rawDataLength);
    
    /* 發送RAW DATA回應 */
    BLE_PacketCreate(&g_txPacket, CMD_RAW_DATA_ACK, responseData, rawDataLength + 2);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief 處理接收到的命令
 * 
 * @param rxPacket 接收封包指針
 * @param txPacket 發送封包指針
 */
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket)
{
    /* 驗證封包 */
    if (!BLE_PacketVerify(rxPacket)) {
        BLE_SendErrorResponse(rxPacket->command, ERR_CHECKSUM);
        return;
    }
    
    /* 根據命令ID進行處理 */
    switch (rxPacket->command) {
        case CMD_SYNC_TIME:
            BLE_HandleSyncTime(rxPacket->data, rxPacket->length);
            break;
            
        case CMD_GET_DEVICE_STATUS:
            BLE_HandleGetDeviceStatus();
            break;
            
        case CMD_SET_CODE_EVENT:
            BLE_HandleSetCodeEvent(rxPacket->data, rxPacket->length);
            break;
            
        case CMD_BLOOD_SAMPLE_STATUS:
            BLE_HandleBloodSampleStatus();
            break;
            
        case CMD_GET_TEST_RESULT:
            BLE_HandleGetTestResult();
            break;
            
        case CMD_GET_RAW_DATA:
            BLE_HandleGetRawData();
            break;
            
        default:
            BLE_SendErrorResponse(rxPacket->command, ERR_UNSUPPORTED_CMD);
            break;
    }
}

/**
 * @brief 處理接收到的數據
 * 
 * @param data 數據指針
 * @param length 數據長度
 */
void BLE_ProcessReceivedData(uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        switch (g_protocolState) {
            case PROTOCOL_STATE_IDLE:
                if (byte == PROTOCOL_START_MARKER) {
                    g_rxPacket.start = byte;
                    g_protocolState = PROTOCOL_STATE_COMMAND;
                }
                break;
                
            case PROTOCOL_STATE_COMMAND:
                g_rxPacket.command = byte;
                g_protocolState = PROTOCOL_STATE_LENGTH;
                break;
                
            case PROTOCOL_STATE_LENGTH:
                g_rxPacket.length = byte;
                g_dataIndex = 0;
                
                if (byte > 0) {
                    g_protocolState = PROTOCOL_STATE_DATA;
                } else {
                    g_protocolState = PROTOCOL_STATE_CHECKSUM;
                }
                break;
                
            case PROTOCOL_STATE_DATA:
                if (g_dataIndex < g_rxPacket.length) {
                    g_rxPacket.data[g_dataIndex++] = byte;
                    
                    if (g_dataIndex >= g_rxPacket.length) {
                        g_protocolState = PROTOCOL_STATE_CHECKSUM;
                    }
                }
                break;
                
            case PROTOCOL_STATE_CHECKSUM:
                g_rxPacket.checksum = byte;
                g_protocolState = PROTOCOL_STATE_END;
                break;
                
            case PROTOCOL_STATE_END:
                g_rxPacket.end = byte;
                g_protocolState = PROTOCOL_STATE_IDLE;
                
                /* 處理完整封包 */
                if (byte == PROTOCOL_END_MARKER) {
                    BLE_HandleCommand(&g_rxPacket, &g_txPacket);
                }
                break;
                
            default:
                g_protocolState = PROTOCOL_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief 發送封包
 * 
 * @param packet 封包指針
 */
void BLE_SendPacket(BLEPacket_TypeDef *packet)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    uint16_t packetLength = 0;
    
    /* 填充封包資料 */
    buffer[packetLength++] = packet->start;
    buffer[packetLength++] = packet->command;
    buffer[packetLength++] = packet->length;
    
    for (uint8_t i = 0; i < packet->length; i++) {
        buffer[packetLength++] = packet->data[i];
    }
    
    buffer[packetLength++] = packet->checksum;
    buffer[packetLength++] = packet->end;
    
    /* 使用UART發送資料 */
    UART1_SendData(buffer, packetLength);
}

/**
 * @brief 初始化藍牙通訊協議
 */
void BLE_ProtocolInit(void)
{
    /* 重置協議狀態機 */
    g_protocolState = PROTOCOL_STATE_IDLE;
    g_dataIndex = 0;
} 