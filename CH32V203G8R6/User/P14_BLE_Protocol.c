/**
 * @file P14_BLE_Protocol.c
 * @brief �๦�������yԇ�xUARTͨӍ�f�h���F
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

/* �f�h̎������ */
void BLE_ProcessCommand(void);
uint16_t Get_ADC_Value(uint8_t channel);

/* ȫ��׃�� */
static ProtocolState_TypeDef g_protocolState = PROTOCOL_STATE_IDLE;
static BLEPacket_TypeDef g_rxPacket;
static uint8_t g_dataIndex = 0;
static BLEPacket_TypeDef g_txPacket;
static uint8_t g_cmdType = 0;
static uint8_t g_cmdData[16];
static uint8_t g_dataLength = 0;

/* �{��UART�l�ͺ��� - ���ⲿ���F */
extern void UART1_SendData(uint8_t *data, uint16_t len);

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
 * @brief �l���e�`�ؑ�
 * 
 * @param originalCmd ԭʼ����ID
 * @param errorCode �e�`���a
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
 * @brief ̎��ͬ���r�g����
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_HandleSyncTime(uint8_t *data, uint8_t length)
{
    RTC_DateTimeTypeDef rtcTime;
    uint8_t responseData = 0x00; // �ɹ���B
    
    if (length != 7) {
        BLE_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* �����r�g�Y�� */
    uint16_t year = (data[0] << 8) | data[1];
    rtcTime.Month = data[2];
    rtcTime.Day = data[3];
    rtcTime.Hour = data[4];
    rtcTime.Min = data[5];
    rtcTime.Sec = data[6];
    rtcTime.Year = year - 2000; // �D�Q��ƫ����
    
    /* �r�g������C */
    if (rtcTime.Month < 1 || rtcTime.Month > 12 ||
        rtcTime.Day < 1 || rtcTime.Day > 31 ||
        rtcTime.Hour > 23 || rtcTime.Min > 59 || rtcTime.Sec > 59) {
        BLE_SendErrorResponse(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    /* �O��RTC�r�g - ��̎����CH32V20x�쌍�F */
    // RTC_SetTime(&rtcTime); // ��Ҫ�������HRTC�캯���{��
    
    /* �l�ͳɹ�푑� */
    BLE_PacketCreate(&g_txPacket, CMD_SYNC_TIME_ACK, &responseData, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎��Ո���b�à�B����
 */
void BLE_HandleGetDeviceStatus(void)
{
    DeviceStatus_TypeDef status;
    uint8_t responseData[8];
    BasicSystemBlock systemParams;
    
    /* �xȡϵ�y���� */
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &systemParams, sizeof(BasicSystemBlock))) {
        BLE_SendErrorResponse(CMD_GET_DEVICE_STATUS, ERR_HARDWARE);
        return;
    }
    
    /* �xȡ늳�늉��͜ض� - ���H��������Ҫ��Q���挍�y������ */
    uint16_t batteryVoltage = 2817; // ʾ��: 2.817V
    uint16_t temperature = 281;     // ʾ��: 28.1��
    
    /* ����B�YӍ */
    status.testType = (uint16_t)systemParams.stripType;
    status.stripStatus = 0x0000; // ���Oԇ������
    status.batteryVoltage = batteryVoltage;
    status.temperature = temperature;
    
    /* �z��늳�늉��Ƿ��^�� */
    if (batteryVoltage < 2200) { // ���O2.2V�ǵ�늉��ֵ
        status.stripStatus = ERR_BATTERY_LOW;
    }
    
    /* �z��ض��Ƿ�����Ч������ */
    if (temperature > 350) { // 35.0��
        status.stripStatus = ERR_TEMP_TOO_HIGH;
    } else if (temperature < 150) { // 15.0��
        status.stripStatus = ERR_TEMP_TOO_LOW;
    }
    
    /* ����B�Y���D�Q���Y����� */
    responseData[0] = (status.testType >> 8) & 0xFF;
    responseData[1] = status.testType & 0xFF;
    responseData[2] = (status.stripStatus >> 8) & 0xFF;
    responseData[3] = status.stripStatus & 0xFF;
    responseData[4] = (status.batteryVoltage >> 8) & 0xFF;
    responseData[5] = status.batteryVoltage & 0xFF;
    responseData[6] = (status.temperature >> 8) & 0xFF;
    responseData[7] = status.temperature & 0xFF;
    
    /* �l�͠�B�ؑ� */
    BLE_PacketCreate(&g_txPacket, CMD_DEVICE_STATUS_ACK, responseData, 8);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎���O��CODE��EVENT��������
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 */
void BLE_HandleSetCodeEvent(uint8_t *data, uint8_t length)
{
    uint8_t responseData = 0x00; // �ɹ���B
    
    if (length != 3) {
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // uint8_t code = data[0]; // δʹ�õ�׃����CODE��������̎��
    uint16_t event = (data[1] << 8) | data[2];
    
    /* ��C�������� */
    if (event > 3) { // ���O�¼���͹�����0-3
        BLE_SendErrorResponse(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    /* ����ϵ�y���� */
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
    
    /* �l�ͳɹ�푑� */
    BLE_PacketCreate(&g_txPacket, CMD_CODE_EVENT_ACK, &responseData, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎��z�yѪҺ��B����
 */
void BLE_HandleBloodSampleStatus(void)
{
    /* �@�e��Ҫ�c���H�ęz�yӲ�w���� */
    /* ԓ������ԓ�z���Ƿ��ѽ��z�y��ѪҺ�ӱ� */
    /* �ڌ��H�����У�������Ҫ���Ӷ��r�����ڙz�� */
    
    /* ���O�F���ѽ��z�y��ѪҺ�ӱ�������5�� */
    uint8_t countdownSeconds = 5;
    
    /* �l��֪ͨ */
    BLE_PacketCreate(&g_txPacket, CMD_BLOOD_SAMPLE_ACK, &countdownSeconds, 1);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎��Ո��yԇ�Y������
 */
void BLE_HandleGetTestResult(void)
{
    TestResult_TypeDef result;
    // RTC_DateTimeTypeDef rtcTime; // ���rδʹ�õ�׃��
    uint8_t responseData[25];
    
    /* �@ȡ�yԇӛ� */
    TestRecord record;
    if (!PARAM_GetTestRecord(0, &record)) {
        BLE_SendErrorResponse(CMD_GET_TEST_RESULT, ERR_HARDWARE);
        return;
    }
    
    /* ���Y���Y�� */
    result.resultStatus = 0x0000; // �ɹ�
    result.testValue = record.resultValue;
    result.testType = (uint16_t)record.testType;
    result.event = (uint16_t)record.event;
    result.code = record.stripCode;
    
    /* ��ӛ��Ы@ȡ�r�g */
    result.year = 2000 + record.year; // ���O��ݸ�ʽ��ƫ����
    result.month = record.month;
    result.date = record.date;
    result.hour = record.hour;
    result.minute = record.minute;
    result.second = record.second;
    
    /* �xȡ늳�늉��͜ض� - ���H��������Ҫ��Q���挍�y������ */
    result.batteryVoltage = 2817; // ʾ��: 2.817V
    result.temperature = 281;     // ʾ��: 28.1��
    
    /* ���Y���Y���D�Q���Y����� */
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
    
    /* �l�ͽY���ؑ� */
    BLE_PacketCreate(&g_txPacket, CMD_TEST_RESULT_ACK, responseData, 25);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎��Ո��RAW DATA����
 */
void BLE_HandleGetRawData(void)
{
    /* �ڌ��H�����У��@��Ҫ�Ĝy��ģ�M�@ȡԭʼ���� */
    /* ��̎�H��ʾ�� */
    
    uint8_t rawData[50];
    uint16_t rawDataLength = 50;
    
    /* ���RAW DATA (�@�eֻ�����ʾ������) */
    for (int i = 0; i < 50; i++) {
        rawData[i] = i;
    }
    
    /* �Y���L����Ϣ (2 bytes) */
    uint8_t responseData[52]; // 2 bytes�L�� + 50 bytesԭʼ����
    responseData[0] = (rawDataLength >> 8) & 0xFF;
    responseData[1] = rawDataLength & 0xFF;
    
    /* �}�uԭʼ���� */
    memcpy(&responseData[2], rawData, rawDataLength);
    
    /* �l��RAW DATA�ؑ� */
    BLE_PacketCreate(&g_txPacket, CMD_RAW_DATA_ACK, responseData, rawDataLength + 2);
    BLE_SendPacket(&g_txPacket);
}

/**
 * @brief ̎����յ�������
 * 
 * @param rxPacket ���շ��ָ�
 * @param txPacket �l�ͷ��ָ�
 */
void BLE_HandleCommand(BLEPacket_TypeDef *rxPacket, BLEPacket_TypeDef *txPacket)
{
    /* ��C��� */
    if (!BLE_PacketVerify(rxPacket)) {
        BLE_SendErrorResponse(rxPacket->command, ERR_CHECKSUM);
        return;
    }
    
    /* ��������ID�M��̎�� */
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
 * @brief ̎����յ��Ĕ���
 * 
 * @param data ����ָ�
 * @param length �����L��
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
                /* �����xȡ�L���ֶ� */
                if (g_dataIndex == 0) {
                    g_rxPacket.length = byte;
                    g_dataIndex++;
                } 
                /* Ȼ���xȡ���� */
                else if (g_dataIndex <= g_rxPacket.length) {
                    g_rxPacket.data[g_dataIndex - 1] = byte;
                    g_dataIndex++;
                    
                    if (g_dataIndex > g_rxPacket.length) {
                        g_protocolState = PROTOCOL_STATE_END;
                    }
                }
                break;
                
            case PROTOCOL_STATE_END:
                /* ̎��У�ͺͽY����ӛ */
                if (g_dataIndex == g_rxPacket.length + 1) {
                    g_rxPacket.checksum = byte;
                    g_dataIndex++;
                } else if (g_dataIndex == g_rxPacket.length + 2) {
                    g_rxPacket.end = byte;
                    g_dataIndex = 0;
                    g_protocolState = PROTOCOL_STATE_IDLE;
                    
                    /* ̎�������f�h�� */
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
 * @brief �l�ͷ��
 * 
 * @param packet ���ָ�
 */
void BLE_SendPacket(BLEPacket_TypeDef *packet)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    uint16_t packetLength = 0;
    
    /* ������Y�� */
    buffer[packetLength++] = packet->start;
    buffer[packetLength++] = packet->command;
    buffer[packetLength++] = packet->length;
    
    for (uint8_t i = 0; i < packet->length; i++) {
        buffer[packetLength++] = packet->data[i];
    }
    
    buffer[packetLength++] = packet->checksum;
    buffer[packetLength++] = packet->end;
    
    /* ʹ��UART�l���Y�� */
    UART1_SendData(buffer, packetLength);
}

/**
 * @brief ��ʼ���{��ͨӍ�f�h
 */
void BLE_ProtocolInit(void)
{
    /* ��ʼ���f�h��B�C */
    g_protocolState = PROTOCOL_STATE_IDLE;
    g_cmdType = 0;
    g_dataIndex = 0;
    g_dataLength = 0;
    
    printf("BLEͨӍ�f�h�ѳ�ʼ��\r\n");
}

/**
 * @brief ̎����յ���UART����
 * 
 * @param rx_data - ���յ��Ć΂��ֹ�����
 */
void BLE_ProtocolRxHandler(uint8_t rx_data)
{
    /* �f�h��B�C̎�� */
    switch (g_protocolState) {
        case PROTOCOL_STATE_IDLE:
            /* �ȴ������_ʼ��ӛ */
            if (rx_data == CMD_START_MARKER) {
                g_protocolState = PROTOCOL_STATE_CMD_TYPE;
            }
            break;
            
        case PROTOCOL_STATE_CMD_TYPE:
            /* ����������� */
            g_cmdType = rx_data;
            g_dataIndex = 0;
            
            /* ������������O�������Ĕ����L�� */
            switch (g_cmdType) {
                case CMD_MEASURE_T1_OUT:
                    g_dataLength = 0;  // �o����
                    g_protocolState = PROTOCOL_STATE_END;
                    break;
                
                case CMD_STRIP_INSERTED:
                    g_dataLength = 1;  // ԇƬ���
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                case CMD_STRIP_TYPE_RESULT:
                    g_dataLength = 1;  // �Y�����a
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                case CMD_START_MEASUREMENT:
                    g_dataLength = 1;  // �y�����
                    g_protocolState = PROTOCOL_STATE_DATA;
                    break;
                
                default:
                    /* δ֪������à�B�C */
                    g_protocolState = PROTOCOL_STATE_IDLE;
                    break;
            }
            break;
            
        case PROTOCOL_STATE_DATA:
            /* ���Ք��� */
            if (g_dataIndex < g_dataLength) {
                g_cmdData[g_dataIndex++] = rx_data;
                
                /* �z���Ƿ��ѽ��������Д��� */
                if (g_dataIndex >= g_dataLength) {
                    g_protocolState = PROTOCOL_STATE_END;
                }
            }
            break;
            
        case PROTOCOL_STATE_END:
            /* �ȴ�����Y����ӛ */
            if (rx_data == CMD_END_MARKER) {
                /* ̎���������� */
                BLE_ProcessCommand();
            }
            
            /* �oՓ��ζ����à�B�C */
            g_protocolState = PROTOCOL_STATE_IDLE;
            break;
            
        default:
            /* ���à�B�C */
            g_protocolState = PROTOCOL_STATE_IDLE;
            break;
    }
}

/**
 * @brief ̎����յ�����������
 */
void BLE_ProcessCommand(void)
{
    switch (g_cmdType) {
        case CMD_MEASURE_T1_OUT:
            /* �y��T1_OUT늉��K�؂��Y�� */
            BLE_MeasureT1Out();
            break;
            
        case CMD_STRIP_INSERTED:
            /* ̎��ԇƬ����֪ͨ */
            if (g_dataLength >= 1) {
                StripType_TypeDef type = (StripType_TypeDef)g_cmdData[0];
                BLE_ProcessStripInsertion(type);
            }
            break;
            
        case CMD_STRIP_TYPE_RESULT:
            /* ̎��ԇƬ����Д�Y�� */
            if (g_dataLength >= 1) {
                StripType_TypeDef type = (StripType_TypeDef)g_cmdData[0];
                printf("���յ�ԇƬ����Д�Y��: %d\r\n", type);
                
                /* ����ϵ�y���� */
                BasicSystemBlock basicParams;
                if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
                    basicParams.stripType = (uint8_t)type;
                    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
                }
            }
            break;
            
        case CMD_START_MEASUREMENT:
            /* �_ʼ�y�� */
            if (g_dataLength >= 1) {
                uint8_t measureType = g_cmdData[0];
                printf("�_ʼ�y�������: %d\r\n", measureType);
                // �ڴ�̎��Ӝy�����Ӵ��a
            }
            break;
            
        default:
            /* δ֪���� */
            printf("�յ�δ֪����: 0x%02X\r\n", g_cmdType);
            break;
    }
}

/**
 * @brief �y��T1_OUT늉��K�؂��Y��
 */
void BLE_MeasureT1Out(void)
{
    uint16_t adcValue;
    uint16_t adcTotal = 0;
    uint16_t adcValues[12]; // �����Μy��ֵ
    uint8_t adcSamples = 12; // ����ȡ�ӴΔ�
    uint8_t validSamples = 0;
    float temperature;
    float temperatureCoeff = 1.0f;
    
    /* �@ȡ��ǰ�ض����У�� - ʹ�ÃȲ��ضȸМy�� */
    temperature = Get_Chip_Temperature();
    
    /* Ӌ��ض��a���S�� (���O22���˜ʜض�) */
    if (temperature > 15.0f && temperature < 35.0f) {
        temperatureCoeff = 1.0f + (temperature - 22.0f) * 0.005f; // ÿ��ƫ��0.5%
    }
    
    /* ����T1�y���· */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // T1_ENABLE = Low (Enable)
    
    /* �ȴ��·���� */
    Delay_Ms(30);  // �������t�r�g�Դ_���y���·����
    
    /* �M�ж��ȡ�� */
    for (uint8_t i = 0; i < adcSamples; i++) {
        /* �xȡADCֵ */
        adcValues[i] = Get_ADC_Value(ADC_Channel_6); // PA6 (T1_OUT)
        
        /* ȡ���g�� */
        Delay_Ms(5);
    }
    
    /* ����y��ֵ�����ȥ������ֵ */
    for (uint8_t i = 0; i < adcSamples - 1; i++) {
        for (uint8_t j = 0; j < adcSamples - i - 1; j++) {
            if (adcValues[j] > adcValues[j + 1]) {
                uint16_t temp = adcValues[j];
                adcValues[j] = adcValues[j + 1];
                adcValues[j + 1] = temp;
            }
        }
    }
    
    /* ȥ����ߺ����ֵ��Ӌ��ƽ��ֵ */
    for (uint8_t i = 2; i < adcSamples - 2; i++) {
        adcTotal += adcValues[i];
        validSamples++;
    }
    
    /* �_������Ч�ӱ� */
    if (validSamples > 0) {
        adcValue = adcTotal / validSamples;
        
        /* ���Üض��a�� */
        adcValue = (uint16_t)((float)adcValue * temperatureCoeff);
    } else {
        /* ����]����Ч�ӱ���ʹ�����gֵ */
        adcValue = adcValues[adcSamples / 2];
    }
    
    /* �P�]T1�y���· */
    GPIO_SetBits(GPIOA, GPIO_Pin_8); // T1_ENABLE = High (Disable)
    
    /* �l��ADC�y���Y�� */
    BLE_SendADCValue(adcValue);
    
    printf("T1_OUT ADC�y��ֵ: %d (�ض�: %.1f��, �a���S��: %.3f)\r\n", 
           adcValue, temperature, temperatureCoeff);
    
    /* �Д�늉��Ƿ�ӽ�2.5V */
    float voltage = (adcValue * 3.3f) / 4096.0f;
    printf("T1_OUT늉�: %.2fV\r\n", voltage);
    
    /* ����늉���B�Д� */
    if (voltage > 2.3f && voltage < 2.7f) {
        printf("늉��ӽ�2.5V�����ܞ�GLV/C/GAVԇƬ\r\n");
    } else {
        printf("늉��h�x2.5V�����ܞ�U/TGԇƬ\r\n");
    }
    
    /* У�y�������� */
    uint16_t maxDiff = 0;
    for (uint8_t i = 2; i < adcSamples - 3; i++) {
        uint16_t diff = (adcValues[i+1] > adcValues[i]) ? 
                         (adcValues[i+1] - adcValues[i]) : 
                         (adcValues[i] - adcValues[i+1]);
        if (diff > maxDiff) maxDiff = diff;
    }
    
    if (maxDiff > 100) {
        printf("���棺T1_OUT�y�������^�� (���ƫ��: %d)\r\n", maxDiff);
    }
}

/**
 * @brief �@ȡоƬ�Ȳ��ض�
 * 
 * @return float �ض�ֵ(�z�϶�)
 */
float Get_Chip_Temperature(void)
{
    uint16_t adcValue;
    float temperature;
    
    /* ����ADC���xȡ�Ȳ��ضȸМy�� */
    ADC_TempSensorVrefintCmd(ENABLE);
    Delay_Ms(1);
    
    /* �xȡADCֵ */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);
    
    /* �_ʼ�D�Q */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* �ȴ��D�Q��� */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* �xȡ�D�Q�Y�� */
    adcValue = ADC_GetConversionValue(ADC1);
    
    /* �P�]�ضȸМy�� */
    ADC_TempSensorVrefintCmd(DISABLE);
    
    /* ��ADCֵ�D�Q��ض� (����CH32V203�����փ��еĜضȸМy������) */
    temperature = ((float)adcValue * 3.3f / 4096.0f - 0.76f) / 0.0025f + 25.0f;
    
    return temperature;
}

/**
 * @brief ADC�y������
 * 
 * @param channel - ADCͨ��
 * @return uint16_t - ADC�y��ֵ (0-4095)
 */
uint16_t Get_ADC_Value(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    /* �_ʼ�D�Q */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* �ȴ��D�Q��� */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* �xȡ�D�Q�Y�� */
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief �l��ADC�y���Y����CH582F
 * 
 * @param adc_value - ADC�y��ֵ
 */
void BLE_SendADCValue(uint16_t adc_value)
{
    uint8_t tx_buffer[5];
    
    /* �����ؑ������� */
    tx_buffer[0] = CMD_START_MARKER;
    tx_buffer[1] = CMD_MEASURE_T1_OUT;
    tx_buffer[2] = (uint8_t)(adc_value >> 8);   // ��λԪ�M
    tx_buffer[3] = (uint8_t)(adc_value & 0xFF); // ��λԪ�M
    tx_buffer[4] = CMD_END_MARKER;
    
    /* �l�ͻؑ� */
    for (uint8_t i = 0; i < 5; i++) {
        USART_SendData(USART2, tx_buffer[i]);
        
        /* �ȴ��l����� */
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

/**
 * @brief ̎��ԇƬ����֪ͨ
 * 
 * @param type - ԇƬ���
 */
void BLE_ProcessStripInsertion(StripType_TypeDef type)
{
    printf("�z�y��ԇƬ���룬���: ");
    
    switch (type) {
        case STRIP_TYPE_GLV:
            printf("Ѫ��(GLV)\r\n");
            break;
        case STRIP_TYPE_U:
            printf("����\r\n");
            break;
        case STRIP_TYPE_C:
            printf("��đ�̴�\r\n");
            break;
        case STRIP_TYPE_TG:
            printf("�������֬\r\n");
            break;
        case STRIP_TYPE_GAV:
            printf("Ѫ��(GAV)\r\n");
            break;
        default:
            printf("δ֪���\r\n");
            break;
    }
    
    /* ����ϵ�y�����е�ԇƬ��� */
    BasicSystemBlock basicParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        basicParams.stripType = (uint8_t)type;
        PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
        
        /* ֪ͨ�όӑ���ԇƬ�����׃�� */
        BLE_NotifyStripType(type);
    }
    
    /* ����ԇƬ����O���������� */
    if (type == STRIP_TYPE_GAV) {
        /* GAVԇƬʹ��T3늘O */
        // ��ʼ��T3늘O����...
    } else {
        /* ����ԇƬʹ��WE늘O */
        // ��ʼ��WE늘O����...
    }
}

/**
 * @brief ֪ͨ�όӑ���ԇƬ�����׃��
 * 
 * @param type - ԇƬ���
 */
void BLE_NotifyStripType(StripType_TypeDef type)
{
    /* �@�e������ӌ�ԇƬ���֪ͨ�o�όӑ��õĴ��a������ͨ�^�{��֪ͨ���֙CAPP */
    printf("ԇƬ����Ѹ���: %d\r\n", type);
}

/**
 * @brief �����_λ��B�Д�ԇƬ���
 * 
 * @param pin3_state - Strip_Detect_3�Ġ�B
 * @param pin5_state - Strip_Detect_5�Ġ�B
 * @param t1_out_near_2p5v - T1_OUT늉��Ƿ�ӽ�2.5V
 *
 * @return StripType_TypeDef - ԇƬ���
 */
StripType_TypeDef BLE_IdentifyStripType(uint8_t pin3_state, uint8_t pin5_state, uint8_t t1_out_near_2p5v)
{
    /* �����z�y�����_λ��B�Д�ԇƬ��� */
    if (pin3_state == 0 && pin5_state == 1 && t1_out_near_2p5v) {
        return STRIP_TYPE_GLV;     // Ѫ��(GLVԇƬ)
    } else if (pin3_state == 0 && pin5_state == 1 && !t1_out_near_2p5v) {
        return STRIP_TYPE_U;       // ����
    } else if (pin3_state == 0 && pin5_state == 0 && t1_out_near_2p5v) {
        return STRIP_TYPE_C;       // ��đ�̴�
    } else if (pin3_state == 1 && pin5_state == 0 && !t1_out_near_2p5v) {
        return STRIP_TYPE_TG;      // ���������
    } else if (pin3_state == 1 && pin5_state == 0 && t1_out_near_2p5v) {
        return STRIP_TYPE_GAV;     // Ѫ��(GAVԇƬ)
    } else {
        return STRIP_TYPE_UNKNOWN; // δ֪���
    }
} 