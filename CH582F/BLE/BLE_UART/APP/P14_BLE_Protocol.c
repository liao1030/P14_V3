/**
 * @file P14_BLE_Protocol.c
 * @brief �๦�������z�y�xCH582F�{��ͨӍ�f�h���F
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

/* �f�h������B */
typedef enum {
    PROTOCOL_STATE_IDLE,
    PROTOCOL_STATE_COMMAND,
    PROTOCOL_STATE_LENGTH,
    PROTOCOL_STATE_DATA,
    PROTOCOL_STATE_CHECKSUM,
    PROTOCOL_STATE_END
} ProtocolState_TypeDef;

/* �ⲿ������ */
extern void app_uart_tx_data(uint8_t *data, uint16_t length);
extern bStatus_t ble_uart_notify_data(uint8_t *data, uint16_t length);
extern void ble_uart_set_callback(ble_uart_ProfileChangeCB_t cb);

/* ����ǰ������ */
static void P14_RegisterBLECallback(void);

/* ȫ��׃�� */
static ProtocolState_TypeDef g_protocolState = PROTOCOL_STATE_IDLE;
static BLEPacket_TypeDef g_rxPacket;
static uint8_t g_dataIndex = 0;
static BLEPacket_TypeDef g_txPacket;

/**
 * @brief Ӌ��У��
 * 
 * @param command ����ID
 * @param data �Y�υ^ָ�
 * @param length �Y���L��
 * @return uint8_t У��
 */
uint8_t BLE_CalculateChecksum(uint8_t command, uint8_t *data, uint8_t length)
{
    uint16_t sum = command;
    
    for (uint8_t i = 0; i < length; i++) {
        sum += data[i];
    }
    
    /* ȡģ256 */
    return (uint8_t)(sum & 0xFF);
}

/**
 * @brief �����f�h���
 * 
 * @param packet ����Y��ָ�
 * @param command ����ID
 * @param data �Y�υ^ָ�
 * @param dataLen �Y���L��
 * @return uint8_t �Ƿ�ɹ�
 */
uint8_t BLE_PacketCreate(BLEPacket_TypeDef *packet, uint8_t command, uint8_t *data, uint8_t dataLen)
{
    if (dataLen > MAX_DATA_SIZE) {
        return 0; // �Y���L�ȳ��^���ֵ
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
 * @brief ��C�����������
 * 
 * @param packet ����Y��ָ�
 * @return uint8_t �Ƿ���Ч
 */
uint8_t BLE_PacketVerify(BLEPacket_TypeDef *packet)
{
    /* �z�����^β��ӛ */
    if (packet->start != PROTOCOL_START_MARKER || packet->end != PROTOCOL_END_MARKER) {
        return 0;
    }
    
    /* �z��У�� */
    uint8_t calculatedChecksum = BLE_CalculateChecksum(packet->command, packet->data, packet->length);
    
    if (calculatedChecksum != packet->checksum) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief ������D�Q���ֹ�����
 * 
 * @param packet ����Y��ָ�
 * @param buffer ݔ�����n�^
 * @param bufferSize ���n�^��С
 * @return uint16_t ���Hʹ�õľ��n�^�L��
 */
uint16_t BLE_PacketToBuffer(BLEPacket_TypeDef *packet, uint8_t *buffer, uint16_t bufferSize)
{
    uint16_t requiredSize = 5 + packet->length; // start + cmd + len + data + checksum + end
    
    if (bufferSize < requiredSize) {
        return 0; // ���n�^̫С
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
 * @brief ��ʼ���{���f�h
 */
void P14_BLE_APP_ProtocolInit(void)
{
    /* ���Åf�h��B�C */
    g_protocolState = PROTOCOL_STATE_IDLE;
    g_dataIndex = 0;
    
    /* �]���{��UART���{ */
    P14_RegisterBLECallback();
}

/**
 * @brief ̎�����APP������
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_ProcessAppCommand(uint8_t *data, uint16_t length)
{
    /* ������ֱ���D�l�oMCU */
    BLE_SendToMCU(data, length);
}

/**
 * @brief ̎�����MCU�Ĕ���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
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
                
                /* ̎��������� */
                if (byte == PROTOCOL_END_MARKER) {
                    /* ��C��� */
                    if (BLE_PacketVerify(&g_rxPacket)) {
                        /* �������MCU�D�l��APP */
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
 * @brief ��APP�l�͔���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_SendToApp(uint8_t *data, uint16_t length)
{
    /* ʹ���{��UART���հl�͔�����APP */
    ble_uart_notify_data(data, length);
}

/**
 * @brief ��MCU�l�͔���
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_SendToMCU(uint8_t *data, uint16_t length)
{
    /* ʹ��UART�l�͔�����MCU */
    app_uart_tx_data(data, length);
}

/**
 * @brief �{��UART���ջ��{̎��
 * 
 * @param connection_handle �B�Ӿ��
 * @param p_evt �¼�ָ�
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
            
            /* ̎�����APP������ */
            BLE_ProcessAppCommand((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            break;
            
        default:
            break;
    }
}

/**
 * @brief �]���{���f�h���{����
 */
static void P14_RegisterBLECallback(void)
{
    /* �]���{��UART���ջ��{ */
    ble_uart_set_callback(P14_BLE_APP_UartCallback);
} 