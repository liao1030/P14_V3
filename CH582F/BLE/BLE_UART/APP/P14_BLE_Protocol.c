/**
 * @file P14_BLE_Protocol.c
 * @brief 多功能生化檢測儀CH582F藍牙通訊協議實現
 * @version 1.0
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "P14_BLE_Protocol.h"
#include "CONFIG.h"
#include "peripheral.h"
#include "app_uart.h"
#include "ble_uart_service.h"
#include <string.h>

/* 協議解析狀態 */
typedef enum {
    PROTOCOL_STATE_IDLE,
    PROTOCOL_STATE_COMMAND,
    PROTOCOL_STATE_LENGTH,
    PROTOCOL_STATE_DATA,
    PROTOCOL_STATE_CHECKSUM,
    PROTOCOL_STATE_END
} ProtocolState_TypeDef;

/* 外部函數聲明 */
extern void app_uart_tx_data(uint8_t *data, uint16_t length);
extern bStatus_t ble_uart_notify_data(uint8_t *data, uint16_t length);
extern void ble_uart_set_callback(ble_uart_ProfileChangeCB_t cb);

/* 函數前向宣告 */
static void P14_RegisterBLECallback(void);

/* 全局變數 */
static ProtocolState_TypeDef g_protocolState = PROTOCOL_STATE_IDLE;
static BLEPacket_TypeDef g_rxPacket;
static uint8_t g_dataIndex = 0;
static BLEPacket_TypeDef g_txPacket;

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
 * @brief 將封包轉換為字節序列
 * 
 * @param packet 封包結構指針
 * @param buffer 輸出緩衝區
 * @param bufferSize 緩衝區大小
 * @return uint16_t 實際使用的緩衝區長度
 */
uint16_t BLE_PacketToBuffer(BLEPacket_TypeDef *packet, uint8_t *buffer, uint16_t bufferSize)
{
    uint16_t requiredSize = 5 + packet->length; // start + cmd + len + data + checksum + end
    
    if (bufferSize < requiredSize) {
        return 0; // 緩衝區太小
    }
    
    uint16_t index = 0;
    
    buffer[index++] = packet->start;
    buffer[index++] = packet->command;
    buffer[index++] = packet->length;
    
    for (uint8_t i = 0; i < packet->length; i++) {
        buffer[index++] = packet->data[i];
    }
    
    buffer[index++] = packet->checksum;
    buffer[index++] = packet->end;
    
    return index;
}

/**
 * @brief 初始化藍牙協議
 */
void P14_BLE_APP_ProtocolInit(void)
{
    /* 重置協議狀態機 */
    g_protocolState = PROTOCOL_STATE_IDLE;
    g_dataIndex = 0;
    
    /* 註冊藍牙UART回調 */
    P14_RegisterBLECallback();
}

/**
 * @brief 處理來自APP的命令
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_ProcessAppCommand(uint8_t *data, uint16_t length)
{
    /* 將數據直接轉發給MCU */
    BLE_SendToMCU(data, length);
}

/**
 * @brief 處理來自MCU的數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_ProcessMcuData(uint8_t *data, uint16_t length)
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
                    /* 驗證封包 */
                    if (BLE_PacketVerify(&g_rxPacket)) {
                        /* 將封包從MCU轉發到APP */
                        uint8_t buffer[MAX_PACKET_SIZE];
                        uint16_t packetSize = BLE_PacketToBuffer(&g_rxPacket, buffer, MAX_PACKET_SIZE);
                        
                        if (packetSize > 0) {
                            BLE_SendToApp(buffer, packetSize);
                        }
                    }
                }
                break;
                
            default:
                g_protocolState = PROTOCOL_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief 向APP發送數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_SendToApp(uint8_t *data, uint16_t length)
{
    /* 使用藍牙UART服務發送數據到APP */
    ble_uart_notify_data(data, length);
}

/**
 * @brief 向MCU發送數據
 * 
 * @param data 資料指針
 * @param length 資料長度
 */
void BLE_SendToMCU(uint8_t *data, uint16_t length)
{
    /* 使用UART發送數據到MCU */
    app_uart_tx_data(data, length);
}

/**
 * @brief 藍牙UART服務回調處理
 * 
 * @param connection_handle 連接句柄
 * @param p_evt 事件指針
 */
void P14_BLE_APP_UartCallback(uint16_t connection_handle, ble_uart_evt_t *p_evt)
{
    switch(p_evt->type)
    {
        case BLE_UART_EVT_TX_NOTI_DISABLED:
            PRINT("%02x: BLE UART TX Notification Disabled\r\n", connection_handle);
            break;
            
        case BLE_UART_EVT_TX_NOTI_ENABLED:
            PRINT("%02x: BLE UART TX Notification Enabled\r\n", connection_handle);
            break;
            
        case BLE_UART_EVT_BLE_DATA_RECIEVED:
            PRINT("BLE RX DATA len: %d\r\n", p_evt->data.length);
            
            /* 處理來自APP的命令 */
            BLE_ProcessAppCommand((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            break;
            
        default:
            break;
    }
}

/**
 * @brief 註冊藍牙協議回調函數
 */
static void P14_RegisterBLECallback(void)
{
    /* 註冊藍牙UART服務回調 */
    ble_uart_set_callback(P14_BLE_APP_UartCallback);
} 