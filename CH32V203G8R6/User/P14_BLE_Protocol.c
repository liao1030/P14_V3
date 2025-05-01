/**
 * @file P14_BLE_Protocol.c
 * @brief 多功能生化測試儀UART通訊協議實現
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

/* 協議處理函數聲明 */
void BLE_ProcessCommand(void);
uint16_t Get_ADC_Value(uint8_t channel);

/* 全局變量 */
static ProtocolState_TypeDef g_protocolState = PROTOCOL_STATE_IDLE;
static BLEPacket_TypeDef g_rxPacket;
static uint8_t g_dataIndex = 0;
static BLEPacket_TypeDef g_txPacket;
static uint8_t g_cmdType = 0;
static uint8_t g_cmdData[16];
static uint8_t g_dataLength = 0;

/* 調用UART發送函數 - 由外部實現 */
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
                    g_protocolState = PROTOCOL_STATE_CMD_TYPE;
                }
                break;
                
            case PROTOCOL_STATE_CMD_TYPE:
                g_rxPacket.command = byte;
                g_protocolState = PROTOCOL_STATE_DATA;
                break;
                
            case PROTOCOL_STATE_DATA:
                /* 首先讀取長度字段 */
                if (g_dataIndex == 0) {
                    g_rxPacket.length = byte;
                    g_dataIndex++;
                } 
                /* 然後讀取數據 */
                else if (g_dataIndex <= g_rxPacket.length) {
                    g_rxPacket.data[g_dataIndex - 1] = byte;
                    g_dataIndex++;
                    
                    if (g_dataIndex > g_rxPacket.length) {
                        g_protocolState = PROTOCOL_STATE_END;
                    }
                }
                break;
                
            case PROTOCOL_STATE_END:
                /* 處理校驗和和結束標記 */
                if (g_dataIndex == g_rxPacket.length + 1) {
                    g_rxPacket.checksum = byte;
                    g_dataIndex++;
                } else if (g_dataIndex == g_rxPacket.length + 2) {
                    g_rxPacket.end = byte;
                    g_dataIndex = 0;
                    g_protocolState = PROTOCOL_STATE_IDLE;
                    
                    /* 處理完整協議包 */
                    if (byte == PROTOCOL_END_MARKER) {
                        if (BLE_PacketVerify(&g_rxPacket)) {
                            BLE_HandleCommand(&g_rxPacket, &g_txPacket);
                        } else {
                            BLE_SendErrorResponse(g_rxPacket.command, ERR_CHECKSUM);
                        }
                    }
                }
                break;
                
            default:
                g_protocolState = PROTOCOL_STATE_IDLE;
                g_dataIndex = 0;
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
    /* 初始化協議狀態機 */
    g_protocolState = PROTOCOL_STATE_IDLE;
    g_cmdType = 0;
    g_dataIndex = 0;
    g_dataLength = 0;
    
    printf("BLE通訊協議已初始化\r\n");
}

/**
 * @brief 處理接收到的UART數據
 * 
 * @param rx_data - 接收到的單個字節數據
 */
void BLE_ProtocolRxHandler(uint8_t rx_data)
{
    /* 協議狀態機處理 */
    switch (g_protocolState) {
        case PROTOCOL_STATE_IDLE:
            /* 等待命令開始標記 */
            if (rx_data == CMD_START_MARKER) {
                g_protocolState = PROTOCOL_STATE_CMD_TYPE;
            }
            break;
            
        case PROTOCOL_STATE_CMD_TYPE:
            /* 接收命令類型 */
            g_cmdType = rx_data;
            g_dataIndex = 0;
            
            /* 根據命令類型設置期望的數據長度 */
            switch (g_cmdType) {
                case CMD_MEASURE_T1_OUT:
                    g_dataLength = 0;  // 無數據
                    g_protocolState = PROTOCOL_STATE_END;
                    break;
                
                case CMD_STRIP_INSERTED:
                    g_dataLength = 1;  // 試片類型
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                case CMD_STRIP_TYPE_RESULT:
                    g_dataLength = 1;  // 結果代碼
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                case CMD_START_MEASUREMENT:
                    g_dataLength = 1;  // 測量類型
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                default:
                    /* 未知命令，重置狀態機 */
                    g_protocolState = PROTOCOL_STATE_IDLE;
                    break;
            }
            break;
            
        case PROTOCOL_STATE_DATA:
            /* 接收數據 */
            if (g_dataIndex < g_dataLength) {
                g_cmdData[g_dataIndex++] = rx_data;
                
                /* 檢查是否已接收完所有數據 */
                if (g_dataIndex >= g_dataLength) {
                    g_protocolState = PROTOCOL_STATE_END;
                }
            }
            break;
            
        case PROTOCOL_STATE_END:
            /* 等待命令結束標記 */
            if (rx_data == CMD_END_MARKER) {
                /* 處理完整命令 */
                BLE_ProcessCommand();
            }
            
            /* 無論如何都重置狀態機 */
            g_protocolState = PROTOCOL_STATE_IDLE;
            break;
            
        default:
            /* 重置狀態機 */
            g_protocolState = PROTOCOL_STATE_IDLE;
            break;
    }
}

/**
 * @brief 處理接收到的完整命令
 */
void BLE_ProcessCommand(void)
{
    switch (g_cmdType) {
        case CMD_MEASURE_T1_OUT:
            /* 測量T1_OUT電壓並回傳結果 */
            BLE_MeasureT1Out();
            break;
            
        case CMD_STRIP_INSERTED:
            /* 處理試片插入通知 */
            if (g_dataLength >= 1) {
                StripType_TypeDef type = (StripType_TypeDef)g_cmdData[0];
                BLE_ProcessStripInsertion(type);
            }
            break;
            
        case CMD_STRIP_TYPE_RESULT:
            /* 處理試片類型判斷結果 */
            if (g_dataLength >= 1) {
                StripType_TypeDef type = (StripType_TypeDef)g_cmdData[0];
                printf("接收到試片類型判斷結果: %d\r\n", type);
                
                /* 更新系統參數 */
                BasicSystemBlock basicParams;
                if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
                    basicParams.stripType = (uint8_t)type;
                    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
                }
            }
            break;
            
        case CMD_START_MEASUREMENT:
            /* 開始測量 */
            if (g_dataLength >= 1) {
                uint8_t measureType = g_cmdData[0];
                printf("開始測量，類型: %d\r\n", measureType);
                // 在此處添加測量啟動代碼
            }
            break;
            
        default:
            /* 未知命令 */
            printf("收到未知命令: 0x%02X\r\n", g_cmdType);
            break;
    }
}

/**
 * @brief 測量T1_OUT電壓並回傳結果
 */
void BLE_MeasureT1Out(void)
{
    uint16_t adcValue;
    uint16_t adcTotal = 0;
    uint16_t adcValues[12]; // 保存多次測量值
    uint8_t adcSamples = 12; // 增加取樣次數
    uint8_t validSamples = 0;
    float temperature;
    float temperatureCoeff = 1.0f;
    
    /* 獲取當前溫度用於校準 - 使用內部溫度感測器 */
    temperature = Get_Chip_Temperature();
    
    /* 計算溫度補償係數 (假設22℃為標準溫度) */
    if (temperature > 15.0f && temperature < 35.0f) {
        temperatureCoeff = 1.0f + (temperature - 22.0f) * 0.005f; // 每度偏差0.5%
    }
    
    /* 啟用T1測量電路 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // T1_ENABLE = Low (Enable)
    
    /* 等待電路穩定 */
    Delay_Ms(30);  // 增加延遲時間以確保測量電路穩定
    
    /* 進行多次取樣 */
    for (uint8_t i = 0; i < adcSamples; i++) {
        /* 讀取ADC值 */
        adcValues[i] = Get_ADC_Value(ADC_Channel_6); // PA6 (T1_OUT)
        
        /* 取樣間隔 */
        Delay_Ms(5);
    }
    
    /* 排序測量值，用於去除異常值 */
    for (uint8_t i = 0; i < adcSamples - 1; i++) {
        for (uint8_t j = 0; j < adcSamples - i - 1; j++) {
            if (adcValues[j] > adcValues[j + 1]) {
                uint16_t temp = adcValues[j];
                adcValues[j] = adcValues[j + 1];
                adcValues[j + 1] = temp;
            }
        }
    }
    
    /* 去除最高和最低值後計算平均值 */
    for (uint8_t i = 2; i < adcSamples - 2; i++) {
        adcTotal += adcValues[i];
        validSamples++;
    }
    
    /* 確保有有效樣本 */
    if (validSamples > 0) {
        adcValue = adcTotal / validSamples;
        
        /* 應用溫度補償 */
        adcValue = (uint16_t)((float)adcValue * temperatureCoeff);
    } else {
        /* 如果沒有有效樣本，使用中間值 */
        adcValue = adcValues[adcSamples / 2];
    }
    
    /* 關閉T1測量電路 */
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // T1_ENABLE = High (Disable)
    
    /* 發送ADC測量結果 */
    BLE_SendADCValue(adcValue);
    
    printf("T1_OUT ADC測量值: %d (溫度: %.1f℃, 補償係數: %.3f)\r\n", 
           adcValue, temperature, temperatureCoeff);
    
    /* 判斷電壓是否接近2.5V */
    float voltage = (adcValue * 3.3f) / 4096.0f;
    printf("T1_OUT電壓: %.2fV\r\n", voltage);
    
    /* 加入電壓狀態判斷 */
    if (voltage > 2.3f && voltage < 2.7f) {
        printf("電壓接近2.5V，可能為GLV/C/GAV試片\r\n");
    } else {
        printf("電壓遠離2.5V，可能為U/TG試片\r\n");
    }
    
    /* 校驗測量穩定性 */
    uint16_t maxDiff = 0;
    for (uint8_t i = 2; i < adcSamples - 3; i++) {
        uint16_t diff = (adcValues[i+1] > adcValues[i]) ? 
                         (adcValues[i+1] - adcValues[i]) : 
                         (adcValues[i] - adcValues[i+1]);
        if (diff > maxDiff) maxDiff = diff;
    }
    
    if (maxDiff > 100) {
        printf("警告：T1_OUT測量波動較大 (最大偏差: %d)\r\n", maxDiff);
    }
}

/**
 * @brief 獲取芯片內部溫度
 * 
 * @return float 溫度值(攝氏度)
 */
float Get_Chip_Temperature(void)
{
    uint16_t adcValue;
    float temperature;
    
    /* 配置ADC以讀取內部溫度感測器 */
    ADC_TempSensorVrefintCmd(ENABLE);
    Delay_Ms(1);
    
    /* 讀取ADC值 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);
    
    /* 開始轉換 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 等待轉換完成 */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* 讀取轉換結果 */
    adcValue = ADC_GetConversionValue(ADC1);
    
    /* 關閉溫度感測器 */
    ADC_TempSensorVrefintCmd(DISABLE);
    
    /* 將ADC值轉換為溫度 (依據CH32V203數據手冊中的溫度感測器特性) */
    temperature = ((float)adcValue * 3.3f / 4096.0f - 0.76f) / 0.0025f + 25.0f;
    
    return temperature;
}

/**
 * @brief ADC測量函數
 * 
 * @param channel - ADC通道
 * @return uint16_t - ADC測量值 (0-4095)
 */
uint16_t Get_ADC_Value(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    /* 開始轉換 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 等待轉換完成 */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* 讀取轉換結果 */
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief 發送ADC測量結果到CH582F
 * 
 * @param adc_value - ADC測量值
 */
void BLE_SendADCValue(uint16_t adc_value)
{
    uint8_t tx_buffer[5];
    
    /* 構建回應數據包 */
    tx_buffer[0] = CMD_START_MARKER;
    tx_buffer[1] = CMD_MEASURE_T1_OUT;
    tx_buffer[2] = (uint8_t)(adc_value >> 8);   // 高位元組
    tx_buffer[3] = (uint8_t)(adc_value & 0xFF); // 低位元組
    tx_buffer[4] = CMD_END_MARKER;
    
    /* 發送回應 */
    for (uint8_t i = 0; i < 5; i++) {
        USART_SendData(USART2, tx_buffer[i]);
        
        /* 等待發送完成 */
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

/**
 * @brief 處理試片插入通知
 * 
 * @param type - 試片類型
 */
void BLE_ProcessStripInsertion(StripType_TypeDef type)
{
    printf("檢測到試片插入，類型: ");
    
    switch (type) {
        case STRIP_TYPE_GLV:
            printf("血糖(GLV)\r\n");
            break;
        case STRIP_TYPE_U:
            printf("尿酸\r\n");
            break;
        case STRIP_TYPE_C:
            printf("總膽固醇\r\n");
            break;
        case STRIP_TYPE_TG:
            printf("三酸甘油脂\r\n");
            break;
        case STRIP_TYPE_GAV:
            printf("血糖(GAV)\r\n");
            break;
        default:
            printf("未知類型\r\n");
            break;
    }
    
    /* 更新系統參數中的試片類型 */
    BasicSystemBlock basicParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        basicParams.stripType = (uint8_t)type;
        PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
        
        /* 通知上層應用試片類型已變更 */
        BLE_NotifyStripType(type);
    }
    
    /* 根據試片類型設定相應參數 */
    if (type == STRIP_TYPE_GAV) {
        /* GAV試片使用T3電極 */
        // 初始化T3電極參數...
    } else {
        /* 其他試片使用WE電極 */
        // 初始化WE電極參數...
    }
}

/**
 * @brief 通知上層應用試片類型已變更
 * 
 * @param type - 試片類型
 */
void BLE_NotifyStripType(StripType_TypeDef type)
{
    /* 這裡可以添加將試片類型通知給上層應用的代碼，例如通過藍牙通知到手機APP */
    printf("試片類型已更新為: %d\r\n", type);
}

/**
 * @brief 依據腳位狀態判斷試片類型
 * 
 * @param pin3_state - Strip_Detect_3的狀態
 * @param pin5_state - Strip_Detect_5的狀態
 * @param t1_out_near_2p5v - T1_OUT電壓是否接近2.5V
 *
 * @return StripType_TypeDef - 試片類型
 */
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v)
{
    /* 根據檢測到的腳位狀態判斷試片類型 */
    if (pin3_state == 0 && pin5_state == 1 && t1_out_near_2p5v) {
        return STRIP_TYPE_GLV;     // 血糖(GLV試片)
    } else if (pin3_state == 0 && pin5_state == 1 && !t1_out_near_2p5v) {
        return STRIP_TYPE_U;       // 尿酸
    } else if (pin3_state == 0 && pin5_state == 0 && t1_out_near_2p5v) {
        return STRIP_TYPE_C;       // 總膽固醇
    } else if (pin3_state == 1 && pin5_state == 0 && !t1_out_near_2p5v) {
        return STRIP_TYPE_TG;      // 三酸甘油酯
    } else if (pin3_state == 1 && pin5_state == 0 && t1_out_near_2p5v) {
        return STRIP_TYPE_GAV;     // 血糖(GAV試片)
    } else {
        return STRIP_TYPE_UNKNOWN; // 未知類型
    }
} 