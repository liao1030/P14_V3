/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/02
 * Description        : �๦�������yԇ�x�{���cUARTͨӍ�f�h���F
 ********************************************************************************/

#include "protocol.h"
#include "uart_comm.h"
#include "string.h"
#include "debug.h"

/* �f�h���P���n�^ */
static uint8_t protocol_tx_buffer[UART_BUFFER_SIZE];
static uint8_t protocol_data_buffer[UART_BUFFER_SIZE];

/* �f�h��ʼ����B */
static uint8_t protocol_initialized = 0;

/* ��ǰϵ�y�r�g */
static Time_t current_time = {2025, 5, 2, 12, 0, 0};

/* ��ǰ�O����CODE��EVENT */
static uint8_t current_code = 0;
static uint16_t current_event = EVENT_NONE;

/* ��ǰ�y���Ŀ */
static uint16_t current_measure_item = ITEM_GLV;

/*********************************************************************
 * @fn      Protocol_CalcChecksum
 *
 * @brief   Ӌ��У��
 *
 * @param   cmd - ָ��ID
 *          data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  Ӌ�����У��
 */
static uint8_t Protocol_CalcChecksum(uint8_t cmd, uint8_t *data, uint8_t length)
{
    uint16_t sum = cmd;  // ��ָ��ID�_ʼӋ��
    uint8_t i;
    
    /* ���������Y���ֹ� */
    for (i = 0; i < length; i++)
    {
        sum += data[i];
    }
    
    /* ȡģ256 */
    return (uint8_t)(sum % 256);
}

/*********************************************************************
 * @fn      Protocol_VerifyChecksum
 *
 * @brief   ��CУ��
 *
 * @param   packet - ����Y��ָ��
 *
 * @return  0 - У�ɹ�, ��0 - У�ʧ��
 */
static uint8_t Protocol_VerifyChecksum(Protocol_Packet_t *packet)
{
    uint8_t checksum = Protocol_CalcChecksum(packet->cmd, packet->data, packet->length);
    
    return (checksum != packet->checksum);
}

/*********************************************************************
 * @fn      Protocol_UART_RxCallback
 *
 * @brief   UART���ջ��{����
 *
 * @param   data - �յ����Y��ָ��
 *          length - �Y���L��
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
 * @brief   ��ʼ��ͨӍ�f�h
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
uint8_t Protocol_Init(void)
{
    /* ��ʼ��UART */
    UART_Comm_Init();
    
    /* �O�ý��ջ��{ */
    UART_SetRxCallback(Protocol_UART_RxCallback);
    
    protocol_initialized = 1;
    
    return 0;
}

/*********************************************************************
 * @fn      Protocol_ParsePacket
 *
 * @brief   �������
 *
 * @param   data - �յ����Y��ָ��
 *          length - �Y���L��
 *          packet - ����Y��ָ��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
static uint8_t Protocol_ParsePacket(uint8_t *data, uint16_t length, Protocol_Packet_t *packet)
{
    uint16_t i = 0;
    
    /* ������ʼ��ӛ */
    while (i < length && data[i] != PROTOCOL_START_MARKER)
    {
        i++;
    }
    
    /* �z���Ƿ��ҵ���ʼ��ӛ */
    if (i >= length || (length - i) < 4) // ������Ҫ ��ʼ��ӛ+ָ��ID+�Y���L��+У��+�Y����ӛ
    {
        return 1;  // �]���ҵ���Ч���
    }
    
    /* �xȡָ��ID */
    packet->cmd = data[i + 1];
    
    /* �xȡ�Y���L�� */
    packet->length = data[i + 2];
    
    /* �z���������� */
    if (i + 3 + packet->length + 2 > length)
    {
        return 2;  // �Y�ϲ�����
    }
    
    /* �xȡ�Y�� */
    packet->data = &data[i + 3];
    
    /* �xȡУ�� */
    packet->checksum = data[i + 3 + packet->length];
    
    /* �z��Y����ӛ */
    if (data[i + 3 + packet->length + 1] != PROTOCOL_END_MARKER)
    {
        return 3;  // �Y����ӛ�e�`
    }
    
    /* ��CУ�� */
    if (Protocol_VerifyChecksum(packet))
    {
        return 4;  // У���e�`
    }
    
    return 0;  // �����ɹ�
}

/*********************************************************************
 * @fn      Protocol_ProcessReceivedData
 *
 * @brief   ̎����յ��Ĕ���
 *
 * @param   data - ���յ��Ĕ���ָ��
 *          length - �����L��
 *
 * @return  0 - �ɹ�̎��, ��0 - ̎��ʧ��
 */
uint8_t Protocol_ProcessReceivedData(uint8_t *data, uint16_t length)
{
    Protocol_Packet_t packet;
    uint8_t result;
    Time_t *time_ptr;
    uint8_t code;
    uint16_t event;
    
    /* ������� */
    result = Protocol_ParsePacket(data, length, &packet);
    if (result != 0)
    {
        /* �����e�`�������e�`�a */
        if (result == 4) // У���e�`
        {
            Protocol_SendErrorResponse(data[1], ERR_CHECKSUM);
        }
        else // ������ʽ�e�`
        {
            Protocol_SendErrorResponse(data[1], ERR_DATA_FORMAT);
        }
        return result;
    }
    
    /* ����ָ��ID̎��ͬ��ָ�� */
    switch (packet.cmd)
    {
        case CMD_SYNC_TIME:
            /* ͬ���r�g */
            if (packet.length != 7)
            {
                Protocol_SendErrorResponse(packet.cmd, ERR_DATA_FORMAT);
                break;
            }
            
            /* ����ϵ�y�r�g */
            time_ptr = &current_time;
            time_ptr->year = (packet.data[0] << 8) | packet.data[1];
            time_ptr->month = packet.data[2];
            time_ptr->day = packet.data[3];
            time_ptr->hour = packet.data[4];
            time_ptr->minute = packet.data[5];
            time_ptr->second = packet.data[6];
            
            /* �ظ��r�gͬ���_�J */
            Protocol_SendSyncTimeAck(0); // 0��ʾ�ɹ�
            break;
            
        case CMD_GET_DEVICE_STATUS:
            /* Ո���b�à�B */
            {
                Device_Status_t status;
                
                /* ����b�à�B */
                status.measure_item = current_measure_item;
                status.strip_status = 0; // ������B, ���O�@�eȡ�Ì��HԇƬ��B
                status.battery_voltage = 3000; // ���O�@�eȡ�Ì��H늳�늉�
                status.temperature = 250; // ���O�@�eȡ�Ì��H�ض�(25.0��)
                
                /* �l���b�à�B�ؑ� */
                Protocol_SendDeviceStatus(&status);
            }
            break;
            
        case CMD_SET_CODE_EVENT:
            /* �O��CODE��EVENT */
            if (packet.length != 3)
            {
                Protocol_SendErrorResponse(packet.cmd, ERR_DATA_FORMAT);
                break;
            }
            
            /* ����CODE��EVENT */
            code = packet.data[0];
            event = (packet.data[1] << 8) | packet.data[2];
            
            current_code = code;
            current_event = event;
            
            /* �ظ��O���_�J */
            Protocol_SendCodeEventAck(0); // 0��ʾ�ɹ�
            break;
            
        case CMD_BLOOD_STATUS_REQ:
            /* �z�y��BՈ�� */
            {
                /* ���O�@�e�z�yѪҺ�ӱ��Ƿ���� */
                uint8_t blood_detected = 1; // ���O�F�������ѪҺ�ӱ�
                
                if (blood_detected)
                {
                    /* �ظ�ѪҺ�z�y֪ͨ������5�� */
                    Protocol_SendBloodStatusNotify(5);
                }
                else
                {
                    /* �ظ��e�` - �y�����r */
                    Protocol_SendErrorResponse(packet.cmd, ERR_MEASURE_TIMEOUT);
                }
            }
            break;
            
        case CMD_REQUEST_RESULT:
            /* Ո��y���Y�� */
            {
                Measure_Result_t result;
                
                /* ���y���Y�� */
                result.result_status = 0; // �ɹ�
                result.measure_value = 123; // ���O�@�eȡ�Ì��H�y��ֵ
                result.measure_item = current_measure_item;
                result.event = current_event;
                result.code = current_code;
                result.year = current_time.year;
                result.month = current_time.month;
                result.day = current_time.day;
                result.hour = current_time.hour;
                result.minute = current_time.minute;
                result.second = current_time.second;
                result.battery_voltage = 3000; // ���O�@�eȡ�Ì��H늳�늉�
                result.temperature = 250; // ���O�@�eȡ�Ì��H�ض�(25.0��)
                
                /* �l�͜y���Y�� */
                Protocol_SendMeasureResult(&result);
            }
            break;
            
        case CMD_REQUEST_RAW_DATA:
            /* Ո��RAW DATA */
            {
                /* ���ORAW DATA */
                uint8_t raw_data[] = {
                    0x00, 0x32, // RAW DATA�L�� (50�ֹ�)
                    0x00, 0x00, 0x00, 0x7B, // �y��ֵ123
                    0x00, 0x19, 0x00, 0x03, 0x00, 0x07, // ������
                    0x00, 0x0F, 0x00, 0x20, 0x00, 0x3B, // �r����
                    0x00, 0x00, 0x00, 0x01, // �y���ĿѪ��
                    0x0B, 0x01, 0x01, 0x19, // 늉��͜ض�
                    0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, // ԭʼADCֵ
                    0x07, 0xD0, 0x03, 0xE8, 0x03, 0x20, // ����ADCֵ
                    0x01, 0x03, // �y���Δ�
                    'A', 'A', 'S', '1', '2', '3', '4', '5', '8', 0x00, // ID
                    'A', 'A', 'S', '1', '2', '3', '4', '5', '8', 0x00, // ��̖
                };
                
                /* �l��RAW DATA */
                Protocol_SendRawData(raw_data, sizeof(raw_data));
            }
            break;
            
        default:
            /* ��֧Ԯ��ָ�� */
            Protocol_SendErrorResponse(packet.cmd, ERR_CMD_NOT_SUPPORTED);
            break;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      Protocol_SendPacket
 *
 * @brief   �l�ͅf�h���
 *
 * @param   cmd - ָ��ID
 *          data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
uint8_t Protocol_SendPacket(uint8_t cmd, uint8_t *data, uint8_t length)
{
    uint8_t checksum;
    uint16_t packet_length = 0;
    
    /* �z���Ƿ��ѳ�ʼ�� */
    if (!protocol_initialized)
    {
        return 1;
    }
    
    /* �z���Y���L�� */
    if (length > UART_BUFFER_SIZE - 5) // 5 = ��ʼ��ӛ(1) + ָ��ID(1) + �Y���L��(1) + У��(1) + �Y����ӛ(1)
    {
        return 2;
    }
    
    /* Ӌ��У�� */
    checksum = Protocol_CalcChecksum(cmd, data, length);
    
    /* �M�b��� */
    protocol_tx_buffer[packet_length++] = PROTOCOL_START_MARKER;
    protocol_tx_buffer[packet_length++] = cmd;
    protocol_tx_buffer[packet_length++] = length;
    
    /* �}�u�Y�� */
    if (length > 0 && data != NULL)
    {
        memcpy(&protocol_tx_buffer[packet_length], data, length);
        packet_length += length;
    }
    
    protocol_tx_buffer[packet_length++] = checksum;
    protocol_tx_buffer[packet_length++] = PROTOCOL_END_MARKER;
    
    /* ͨ�^UART�l�� */
    return UART_SendData(protocol_tx_buffer, packet_length);
}

/*********************************************************************
 * @fn      Protocol_SendErrorResponse
 *
 * @brief   �l���e�`�ؑ�
 *
 * @param   original_cmd - ԭʼָ��ID
 *          error_code - �e�`���a
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
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
 * @brief   �l���b�à�B
 *
 * @param   status - �b�à�B�Y��ָ��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
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
 * @brief   �l�͜y���Y��
 *
 * @param   result - �y���Y���Y��ָ��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
uint8_t Protocol_SendMeasureResult(Measure_Result_t *result)
{
    uint8_t data[25];
    uint8_t index = 0;
    
    /* ����Y�� */
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
 * @brief   �l�͕r�gͬ���_�J
 *
 * @param   status - ͬ����B (0��ʾ�ɹ�)
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
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
 * @brief   �l��CODE/EVENT�O���_�J
 *
 * @param   status - �O����B (0��ʾ�ɹ�)
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
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
 * @brief   �l��ѪҺ�z�y֪ͨ
 *
 * @param   countdown - �����딵
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
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
 * @brief   �l��RAW DATA
 *
 * @param   data - RAW DATAָ��
 *          length - RAW DATA�L��
 *
 * @return  0 - �ɹ�, ��0 - ʧ��
 */
uint8_t Protocol_SendRawData(uint8_t *data, uint16_t length)
{
    /* ֱ�ӌ�ԭʼ�Y�������Y�ϲ��ւ��� */
    return Protocol_SendPacket(CMD_RAW_DATA_RESPONSE, data, length);
}