/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/12
 * Description        : �๦�������yԇ�xUARTͨӍ�f�h
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "uart_protocol.h"
#include "debug.h"
#include "string.h"
#include "param_table.h"
#include "strip_detect.h"
#include "rtc.h"

/* ȫ��׃�� */
static uint8_t rx_buffer[MAX_PACKET_SIZE];
static uint8_t tx_buffer[MAX_PACKET_SIZE];
static uint16_t rx_index = 0;
static uint8_t packet_received = 0;
uint16_t g_batteryVoltage = 0;  // ȫ��늳�늉�׃��

/* ��ǰ�yԇ��B */
static uint8_t test_in_progress = 0;
static uint8_t blood_detected = 0;
static uint8_t blood_countdown = 5; // �A�O�����r�g��5��

/* �yԇ�Y������^ */
static TestResult_TypeDef current_test_result;
static RawData_TypeDef current_raw_data;

/* �ⲿ�� - �@Щ������Ҫ��main�������n���Ќ��F */
extern uint8_t Check_Battery_Status(void);
extern uint16_t Get_Battery_Voltage(void);
extern uint16_t Get_Temperature(void);
extern uint8_t Check_Strip_Status(void);
extern uint8_t Check_Blood_Status(void);
extern uint8_t Get_Test_Data(TestResult_TypeDef *result);
extern uint8_t Get_Raw_Test_Data(RawData_TypeDef *rawData);

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   ��ʼ��UART�f�h̎��
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    /* ��ʼ�����վ��n�^���� */
    rx_index = 0;
    packet_received = 0;
    
    /* ��ʼ���yԇ��B */
    test_in_progress = 0;
    blood_detected = 0;
    blood_countdown = 5;
    
    /* ��ʼ���Y���Y�� */
    memset(&current_test_result, 0, sizeof(TestResult_TypeDef));
    memset(&current_raw_data, 0, sizeof(RawData_TypeDef));
    
    printf("UART Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      UART_Protocol_Process
 *
 * @brief   ̎����յ���UART�Y��
 *
 * @return  none
 */
void UART_Protocol_Process(void)
{
    /* �z���Ƿ�������������յ����@������Ҫ�c�ⲿUART���ՙC�����ϣ� */
    if(packet_received)
    {
        /* �����K̎���� */
        UART_ProcessPacket(rx_buffer, rx_index);
        
        /* ���O����׃�� */
        rx_index = 0;
        packet_received = 0;
    }
    
    /* ������ڜyԇ�У��z��ѪҺ��B */
    if(test_in_progress && !blood_detected)
    {
        if(Check_Blood_Status() == 1)
        {
            blood_detected = 1;
            /* �l��ѪҺ�z�y֪ͨ */
            UART_SendBloodNotify(blood_countdown);
        }
    }
}

/*********************************************************************
 * @fn      UART_CalculateChecksum
 *
 * @brief   Ӌ��z��
 *
 * @param   cmdId - ָ��ID
 *          length - �Y���L��
 *          data - �Y��ָ��
 *
 * @return  Ӌ��õ��ęz��
 */
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t length, uint8_t *data)
{
    uint16_t sum = cmdId + length;
    uint8_t i;
    
    for(i = 0; i < length; i++)
    {
        sum += data[i];
    }
    
    return (uint8_t)(sum % 256);
}

/*********************************************************************
 * @fn      UART_VerifyChecksum
 *
 * @brief   ��C����z��
 *
 * @param   packet - ����Y��
 *          length - ����L��
 *
 * @return  1:�z�ͨ�^ 0:�z�ʧ��
 */
uint8_t UART_VerifyChecksum(uint8_t *packet, uint16_t length)
{
    if(length < 5) // ��С����L�șz�� (��ʼ��ӛ+ָ��ID+�L��+У��+�Y����ӛ)
        return 0;
    
    uint8_t cmdId = packet[1];
    uint8_t dataLen = packet[2];
    uint8_t *data = &packet[3];
    uint8_t checksum = packet[3 + dataLen];
    
    uint8_t calculatedChecksum = UART_CalculateChecksum(cmdId, dataLen, data);
    
    return (checksum == calculatedChecksum) ? 1 : 0;
}

/*********************************************************************
 * @fn      UART_SendPacket
 *
 * @brief   �l�ͅf�h���
 *
 * @param   cmdId - ָ��ID
 *          data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t length)
{
    if(length > MAX_DATA_SIZE)
        return 0;
    
    uint8_t txIndex = 0;
    
    /* ���b��� */
    tx_buffer[txIndex++] = PROTOCOL_START_MARK;     // ��ʼ��ӛ
    tx_buffer[txIndex++] = cmdId;                  // ָ��ID
    tx_buffer[txIndex++] = length;                 // �Y���L��
    
    /* �Y�υ^ */
    if(length > 0 && data != NULL)
    {
        memcpy(&tx_buffer[txIndex], data, length);
        txIndex += length;
    }
    
    /* Ӌ��K����У�� */
    tx_buffer[txIndex++] = UART_CalculateChecksum(cmdId, length, data);
    
    /* ����Y����ӛ */
    tx_buffer[txIndex++] = PROTOCOL_END_MARK;
    
    /* ͨ�^UART�l�ͷ��(�@�e��Ҫ�c���HUART�l�͙C������) */
    for(uint8_t i = 0; i < txIndex; i++)
    {
        USART_SendData(USART2, tx_buffer[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessPacket
 *
 * @brief   ̎���յ��ķ��
 *
 * @param   packet - ����Y��
 *          length - ����L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessPacket(uint8_t *packet, uint16_t length)
{
    /* �z���Ƿ��������ķ�� */
    if(packet[0] != PROTOCOL_START_MARK || packet[length-1] != PROTOCOL_END_MARK)
    {
        return 0;
    }
    
    /* ��CУ�� */
    if(!UART_VerifyChecksum(packet, length))
    {
        /* У��e�`���l���e�`�ؑ� */
        UART_SendErrorAck(packet[1], ERR_CHECKSUM_ERROR);
        return 0;
    }
    
    /* ������� */
    uint8_t cmdId = packet[1];
    uint8_t dataLen = packet[2];
    uint8_t *data = &packet[3];
    
    /* ��������ID̎��ͬ��ָ�� */
    switch(cmdId)
    {
        case CMD_SYNC_TIME:
            /* ͬ���r�gָ�� */
            UART_ProcessSyncTime(data, dataLen);
            break;
        
        case CMD_GET_STATUS:
            /* Ո���b�à�Bָ�� */
            UART_ProcessGetStatus(data, dataLen);
            break;
        
        case CMD_SET_PARAM:
            /* �O�Å���ָ�� */
            UART_ProcessSetParam(data, dataLen);
            break;
        
        case CMD_CHECK_BLOOD:
            /* �z�yѪҺ��Bָ�� */
            UART_ProcessCheckBlood(data, dataLen);
            break;
        
        case CMD_GET_RESULT:
            /* Ո��yԇ�Y��ָ�� */
            UART_ProcessGetResult(data, dataLen);
            break;
        
        case CMD_GET_RAW_DATA:
            /* Ո��RAW DATAָ�� */
            UART_ProcessGetRawData(data, dataLen);
            break;
            
        case CMD_STRIP_INSERTED:
            /* ԇƬ����֪ͨ */
            UART_ProcessStripInsertedCmd(data, dataLen);
            break;
            
        default:
            /* ��֧Ԯ��ָ�� */
            UART_SendErrorAck(cmdId, ERR_COMMAND_NOTSUPP);
            return 0;
    }
}

/*********************************************************************
 * @fn      UART_ProcessSyncTime
 *
 * @brief   ̎��ͬ���r�gָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessSyncTime(uint8_t *data, uint8_t length)
{
    /* �z���Y���L���Ƿ����_ */
    if(length != 12) // 6��16λԪ�ĕr�g�Y��
    {
        UART_SendErrorAck(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �����r�g�Y�� */
    uint16_t year = (data[0] << 8) | data[1];
    uint16_t month = (data[2] << 8) | data[3];
    uint16_t date = (data[4] << 8) | data[5];
    uint16_t hour = (data[6] << 8) | data[7];
    uint16_t minute = (data[8] << 8) | data[9];
    uint16_t second = (data[10] << 8) | data[11];
    
    /* ��C�r�g�Y�Ϻ����� */
    if(year < 2000 || year > 2099 || month < 1 || month > 12 || 
       date < 1 || date > 31 || hour > 23 || minute > 59 || second > 59)
    {
        UART_SendErrorAck(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �O��ϵ�y�r�g�������� */
    uint8_t result = PARAM_SetDateTime(year - 2000, month, date, hour, minute, second);
    
    /* ͬ�r����RTC�r�g */
    RTC_SetTime(year - 2000, month, date, hour, minute, second);
    
    /* �l�ʹ_�J�ؑ� */
    UART_SendSyncTimeAck(result ? 0 : ERR_DATA_FORMAT);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetStatus
 *
 * @brief   ̎��@ȡ�b�à�Bָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessGetStatus(uint8_t *data, uint8_t length)
{
    /* �z���Y���L��, ��ָ��o�Y�� */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_STATUS, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �@ȡ��ǰ�b�à�B */
    DeviceStatus_TypeDef status;
    
    status.stripType = (uint16_t)PARAM_GetByte(PARAM_STRIP_TYPE);
    status.stripStatus = (uint16_t)Check_Strip_Status();
    status.batteryVoltage = Get_Battery_Voltage();
    status.temperature = Get_Temperature();
    
    /* �l�͠�B�ؑ� */
    UART_SendStatusAck(&status);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessSetParam
 *
 * @brief   ̎���O��CODE��EVENT����ָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessSetParam(uint8_t *data, uint8_t length)
{
    /* �z���Y���L�� */
    if(length != 4) // 2��16λԪ�ą���(CODE��EVENT)
    {
        UART_SendErrorAck(CMD_SET_PARAM, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �������� */
    uint16_t code = (data[0] << 8) | data[1];
    uint16_t event = (data[2] << 8) | data[3];
    
    /* ��C���������� */
    if(event >= EVENT_MAX)
    {
        UART_SendErrorAck(CMD_SET_PARAM, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �O�Å��� */
    PARAM_SetWord(PARAM_CODE_TABLE_V, code);
    PARAM_SetByte(PARAM_EVENT, event);
    
    /* ͬ�����yԇ�Y���Y�� */
    current_test_result.eventType = event;
    current_test_result.stripCode = code;
    
    /* �l�ʹ_�J�ؑ� */
    UART_SendParamAck(0); // 0��ʾ�ɹ�
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessCheckBlood
 *
 * @brief   ̎��z�yѪҺ��Bָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessCheckBlood(uint8_t *data, uint8_t length)
{
    /* �z���Y���L��, ��ָ��o�Y�� */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_CHECK_BLOOD, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �z��ԇƬ��B */
    uint8_t stripStatus = Check_Strip_Status();
    if(stripStatus != 0)
    {
        UART_SendErrorAck(CMD_CHECK_BLOOD, stripStatus);
        return 0;
    }
    
    /* ����ѪҺ�z�y */
    test_in_progress = 1;
    blood_detected = 0;
    
    /* �z���Ƿ��ѽ���ѪҺ�ӱ� */
    if(Check_Blood_Status() == 1)
    {
        blood_detected = 1;
        UART_SendBloodNotify(blood_countdown);
    }
    else
    {
        /* �]�Йz�y��ѪҺ, �ڳ��rǰ�ȴ� */
        UART_SendErrorAck(CMD_CHECK_BLOOD, ERR_TEST_TIMEOUT);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetResult
 *
 * @brief   ̎��@ȡ�yԇ�Y��ָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessGetResult(uint8_t *data, uint8_t length)
{
    /* �z���Y���L��, ��ָ��o�Y�� */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_RESULT, ERR_DATA_FORMAT);
        return 0;
    }
    
    //Ŀǰ�Ⱥ���test_in_progress�cblood_detected�ęz��

    // /* �z���Ƿ�yԇ�M���� */
    // if(test_in_progress!=0)
    // {
    //     UART_SendErrorAck(CMD_GET_RESULT, ERR_TEST_TIMEOUT);
    //     return 0;
    // }
    
    // /* �z���Ƿ��ѽ��z�y��ѪҺ */
    // if(!blood_detected)
    // {
    //     UART_SendErrorAck(CMD_GET_RESULT, ERR_BLOOD_NOT_ENOUGH);
    //     return 0;
    // }
    
    /* �@ȡ�yԇ�Y�� */
    TestResult_TypeDef result;
    if(!Get_Test_Data(&result))
    {
        UART_SendErrorAck(CMD_GET_RESULT, ERR_HARDWARE_ERROR);
        test_in_progress = 0; // �yԇ�Y��
        return 0;
    }
    
    /* ����yԇ�Y�� */
    memcpy(&current_test_result, &result, sizeof(TestResult_TypeDef));
    
    /* �l�ͽY���ؑ� */
    UART_SendResultAck(&result);
    
    /* �yԇ�Y�� */
    test_in_progress = 0;
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetRawData
 *
 * @brief   ̎��@ȡRAW DATAָ��
 *
 * @param   data - �Y��ָ��
 *          length - �Y���L��
 *
 * @return  ̎��Y��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_ProcessGetRawData(uint8_t *data, uint8_t length)
{
    /* �z���Y���L��, ��ָ��o�Y�� */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_RAW_DATA, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* �@ȡԭʼ�yԇ�Y�� */
    RawData_TypeDef rawData;
    if(!Get_Raw_Test_Data(&rawData))
    {
        UART_SendErrorAck(CMD_GET_RAW_DATA, ERR_HARDWARE_ERROR);
        return 0;
    }
    
    /* ����ԭʼ�Y�� */
    memcpy(&current_raw_data, &rawData, sizeof(RawData_TypeDef));
    
    /* �l��ԭʼ�Y�ϻؑ� */
    UART_SendRawDataAck(&rawData);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_SendSyncTimeAck
 *
 * @brief   �l�͕r�gͬ���_�J�ؑ�
 *
 * @param   status - ��B�a(0�ɹ�, ��0�e�`)
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendSyncTimeAck(uint8_t status)
{
    uint8_t data[1];
    data[0] = status;
    
    return UART_SendPacket(CMD_SYNC_TIME_ACK, data, 1);
}

/*********************************************************************
 * @fn      UART_SendStatusAck
 *
 * @brief   �l���b�à�B�ؑ�
 *
 * @param   status - �b�à�B�Y��ָ��
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendStatusAck(DeviceStatus_TypeDef *status)
{
    uint8_t data[8]; // 4��16λԪ����
    
    /* ���b�Y�� */
    data[0] = (status->stripType >> 8) & 0xFF;
    data[1] = status->stripType & 0xFF;
    
    data[2] = (status->stripStatus >> 8) & 0xFF;
    data[3] = status->stripStatus & 0xFF;
    
    data[4] = (status->batteryVoltage >> 8) & 0xFF;
    data[5] = status->batteryVoltage & 0xFF;
    
    data[6] = (status->temperature >> 8) & 0xFF;
    data[7] = status->temperature & 0xFF;
    
    return UART_SendPacket(CMD_STATUS_ACK, data, 8);
}

/*********************************************************************
 * @fn      UART_SendParamAck
 *
 * @brief   �l�ͅ����O�ô_�J�ؑ�
 *
 * @param   status - ��B�a(0�ɹ�, ��0�e�`)
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendParamAck(uint8_t status)
{
    uint8_t data[1];
    data[0] = status;
    
    return UART_SendPacket(CMD_SET_PARAM_ACK, data, 1);
}

/*********************************************************************
 * @fn      UART_SendBloodNotify
 *
 * @brief   �l��ѪҺ�z�y֪ͨ
 *
 * @param   countdown - �����딵
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendBloodNotify(uint8_t countdown)
{
    uint8_t data[1];
    data[0] = countdown;
    
    return UART_SendPacket(CMD_BLOOD_NOTIFY, data, 1);
}

/*********************************************************************
 * @fn      UART_SendResultAck
 *
 * @brief   �l�͜yԇ�Y���ؑ�
 *
 * @param   result - �yԇ�Y���Y��ָ��
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendResultAck(TestResult_TypeDef *result)
{
    uint8_t data[26]; // 13��16λԪ����
    uint8_t index = 0;
    
    /* ���b�Y�� */
    data[index++] = (result->resultStatus >> 8) & 0xFF;
    data[index++] = result->resultStatus & 0xFF;
    
    data[index++] = (result->testValue >> 8) & 0xFF;
    data[index++] = result->testValue & 0xFF;
    
    data[index++] = (result->stripType >> 8) & 0xFF;
    data[index++] = result->stripType & 0xFF;
    
    data[index++] = (result->eventType >> 8) & 0xFF;
    data[index++] = result->eventType & 0xFF;
    
    data[index++] = (result->stripCode >> 8) & 0xFF;
    data[index++] = result->stripCode & 0xFF;
    
    data[index++] = (result->year >> 8) & 0xFF;
    data[index++] = result->year & 0xFF;
    
    data[index++] = (result->month >> 8) & 0xFF;
    data[index++] = result->month & 0xFF;
    
    data[index++] = (result->date >> 8) & 0xFF;
    data[index++] = result->date & 0xFF;
    
    data[index++] = (result->hour >> 8) & 0xFF;
    data[index++] = result->hour & 0xFF;
    
    data[index++] = (result->minute >> 8) & 0xFF;
    data[index++] = result->minute & 0xFF;
    
    data[index++] = (result->second >> 8) & 0xFF;
    data[index++] = result->second & 0xFF;
    
    data[index++] = (result->batteryVoltage >> 8) & 0xFF;
    data[index++] = result->batteryVoltage & 0xFF;
    
    data[index++] = (result->temperature >> 8) & 0xFF;
    data[index++] = result->temperature & 0xFF;
    
    return UART_SendPacket(CMD_RESULT_ACK, data, index);
}

/*********************************************************************
 * @fn      UART_SendRawDataAck
 *
 * @brief   �l��RAW DATA�ؑ�
 *
 * @param   rawData - RAW DATA�Y��ָ��
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendRawDataAck(RawData_TypeDef *rawData)
{
    uint8_t data[60]; // RAW DATA�Y���ķ��b
    uint8_t index = 0;
    
    /* ���b�Y�� */
    data[index++] = (rawData->resultStatus >> 8) & 0xFF;
    data[index++] = rawData->resultStatus & 0xFF;
    
    data[index++] = (rawData->testValue >> 8) & 0xFF;
    data[index++] = rawData->testValue & 0xFF;
    
    data[index++] = (rawData->year >> 8) & 0xFF;
    data[index++] = rawData->year & 0xFF;
    
    data[index++] = (rawData->month >> 8) & 0xFF;
    data[index++] = rawData->month & 0xFF;
    
    data[index++] = (rawData->date >> 8) & 0xFF;
    data[index++] = rawData->date & 0xFF;
    
    data[index++] = (rawData->hour >> 8) & 0xFF;
    data[index++] = rawData->hour & 0xFF;
    
    data[index++] = (rawData->minute >> 8) & 0xFF;
    data[index++] = rawData->minute & 0xFF;
    
    data[index++] = (rawData->second >> 8) & 0xFF;
    data[index++] = rawData->second & 0xFF;
    
    data[index++] = (rawData->stripType >> 8) & 0xFF;
    data[index++] = rawData->stripType & 0xFF;
    
    data[index++] = (rawData->eventType >> 8) & 0xFF;
    data[index++] = rawData->eventType & 0xFF;
    
    data[index++] = (rawData->batteryVoltage >> 8) & 0xFF;
    data[index++] = rawData->batteryVoltage & 0xFF;
    
    data[index++] = (rawData->temperature >> 8) & 0xFF;
    data[index++] = rawData->temperature & 0xFF;
    
    data[index++] = (rawData->w1Adv >> 8) & 0xFF;
    data[index++] = rawData->w1Adv & 0xFF;
    
    data[index++] = (rawData->w2Adv >> 8) & 0xFF;
    data[index++] = rawData->w2Adv & 0xFF;
    
    data[index++] = (rawData->t1Ampl >> 8) & 0xFF;
    data[index++] = rawData->t1Ampl & 0xFF;
    
    data[index++] = (rawData->t1AcMax >> 8) & 0xFF;
    data[index++] = rawData->t1AcMax & 0xFF;
    
    data[index++] = (rawData->t1AcMin >> 8) & 0xFF;
    data[index++] = rawData->t1AcMin & 0xFF;
    
    data[index++] = (rawData->t3Adc >> 8) & 0xFF;
    data[index++] = rawData->t3Adc & 0xFF;
    
    data[index++] = (rawData->testCount >> 8) & 0xFF;
    data[index++] = rawData->testCount & 0xFF;
    
    data[index++] = (rawData->stripCode >> 8) & 0xFF;
    data[index++] = rawData->stripCode & 0xFF;
    
    /* ����ASCII���a�Ĳ�����ID�͈D��̖ */
    for(int i = 0; i < 10; i++)
    {
        data[index++] = rawData->operatorId[i];
    }
    
    for(int i = 0; i < 10; i++)
    {
        data[index++] = rawData->chartNo[i];
    }
    
    return UART_SendPacket(CMD_RAW_DATA_ACK, data, index);
}

/*********************************************************************
 * @fn      UART_SendErrorAck
 *
 * @brief   �l���e�`�ؑ�
 *
 * @param   cmdId - ԭʼָ��ID
 *          errorCode - �e�`���a
 *
 * @return  �l�ͽY��(1:�ɹ� 0:ʧ��)
 */
uint8_t UART_SendErrorAck(uint8_t cmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = cmdId;
    data[1] = errorCode;
    
    return UART_SendPacket(CMD_ERROR_ACK, data, 2);
}

/*********************************************************************
 * @fn      UART2_Receive_Byte_ISR
 *
 * @brief   UART����λԪ�M�Д���պ���
 *          �˺����������ϵ�UART2�Ľ����Д�̎����
 *
 * @param   byte - �յ���λԪ�M
 *
 * @return  none
 */
void UART2_Receive_Byte_ISR(uint8_t byte)
{
    static uint8_t state = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataCnt = 0;
    
    switch(state)
    {
        case 0: // �ȴ���ʼ��ӛ
            if(byte == PROTOCOL_START_MARK)
            {
                rx_buffer[0] = byte;
                rx_index = 1;
                state = 1;
            }
            break;
            
        case 1: // ����ָ��ID
            rx_buffer[rx_index++] = byte;
            state = 2;
            break;
            
        case 2: // �����Y���L��
            rx_buffer[rx_index++] = byte;
            dataLen = byte;
            dataCnt = 0;
            
            if(dataLen == 0)
                state = 4; // �o�Y�ϣ�ֱ�ӽ���У��
            else
                state = 3; // �����Y��
            break;
            
        case 3: // �����Y��
            rx_buffer[rx_index++] = byte;
            dataCnt++;
            
            if(dataCnt >= dataLen)
                state = 4; // �Y�Ͻ����ꮅ������У��
            break;
            
        case 4: // ����У��
            rx_buffer[rx_index++] = byte;
            state = 5;
            break;
            
        case 5: // ���սY����ӛ
            if(byte == PROTOCOL_END_MARK)
            {
                rx_buffer[rx_index++] = byte;
                packet_received = 1; // ����������
            }
            else
            {
                // �Y����ӛ�e�`�����ý���
                rx_index = 0;
            }
            
            state = 0; // ���à�B�C
            break;
            
        default:
            state = 0;
            rx_index = 0;
            break;
    }
    
    /* ���n�^������o */
    if(rx_index >= MAX_PACKET_SIZE)
    {
        state = 0;
        rx_index = 0;
    }
}

/*********************************************************************
 * @fn      UART_UpdateBloodCountdown
 *
 * @brief   ����ԇƬ��͸���ѪҺ�z�y�����r�g
 *
 * @param   stripType - ԇƬ���
 *
 * @return  none
 */
void UART_UpdateBloodCountdown(StripType_TypeDef stripType)
{
    uint16_t tpl1, trd1, evWidth1;
    uint16_t tpl2, trd2, evWidth2; // δʹ�õ�������Ҫ
    
    /* �@ȡԇƬ�r�򅢔� */
    if (PARAM_GetTimingParameters(stripType, &tpl1, &trd1, &evWidth1, 1) &&
        PARAM_GetTimingParameters(stripType, &tpl2, &trd2, &evWidth2, 2)) {
        
        /* Ӌ��ѪҺ�����r�g��(tpl1 + trd1 + evWidth1) / 1000 */
        uint32_t totalTime = (uint32_t)tpl1 + (uint32_t)trd1 + (uint32_t)evWidth1;
        blood_countdown = (uint8_t)(totalTime / 1000);

        if ((totalTime%1000) > 0) {
            blood_countdown++;
        }
        
        /* �_��������1��ĵ����r�g */
        if (blood_countdown < 1) {
            blood_countdown = 1;
        }
        
        printf("Blood countdown updated for strip type %s: %d seconds\r\n", 
               StripType_GetName(stripType), blood_countdown);
    } else {
        /* �o���@ȡ�r�򅢔���ʹ���A�Oֵ */
        blood_countdown = 5;
        printf("Failed to get timing parameters for strip type %s, using default countdown: %d seconds\r\n", 
               StripType_GetName(stripType), blood_countdown);
    }
}

/*********************************************************************
 * @fn      UART_ProcessStripInsertedCmd
 *
 * @brief   ̎��ԇƬ����֪ͨ����
 *
 * @param   data - ����ָ�
 * @param   length - �����L��
 *
 * @return  ̎��Y�� (0=ʧ��, 1=�ɹ�)
 */
uint8_t UART_ProcessStripInsertedCmd(uint8_t *data, uint8_t length)
{
    uint16_t batteryVoltage = 0;
    
    /* �xȡԇƬPin3��Pin5�Ġ�B��늳�늉���������ṩ�� */
    if (length >= 4) {
        uint8_t pin3Status = data[0];
        uint8_t pin5Status = data[1];
        
        /* �xȡ늳�늉� */
        batteryVoltage = (uint16_t)((data[2] << 8) | data[3]);
        
        /* ����ϵ�y�е�늳�늉�׃�� */
        extern uint16_t g_batteryVoltage; // �@��Ҫ��ĳ���ط����x
        g_batteryVoltage = batteryVoltage;
        
        /* �O��ԇƬ�_λ��B */
        STRIP_DETECT_SetPinStatus(pin3Status, pin5Status);
        
        /* �|�l����̎�� */
        STRIP_DETECT_HandleInsertedEvent();
        
        /* �@ȡ�Д��ԇƬ��́K����ѪҺ�����r�g */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
        
        printf("Strip inserted notification received. Pin3=%d, Pin5=%d, BatteryVoltage=%dmV\r\n", 
               pin3Status, pin5Status, batteryVoltage);
    } else if (length >= 2) {
        uint8_t pin3Status = data[0];
        uint8_t pin5Status = data[1];
        
        /* �O��ԇƬ�_λ��B */
        STRIP_DETECT_SetPinStatus(pin3Status, pin5Status);
        
        /* �|�l����̎�� */
        STRIP_DETECT_HandleInsertedEvent();
        
        /* �@ȡ�Д��ԇƬ��́K����ѪҺ�����r�g */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
        
        printf("Strip inserted notification received. Pin3=%d, Pin5=%d\r\n", 
               pin3Status, pin5Status);
    } else {
        printf("Strip inserted notification without pin status\r\n");
        STRIP_DETECT_HandleInsertedEvent();
        
        /* �@ȡ�Д��ԇƬ��́K����ѪҺ�����r�g */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_SendStripTypeAck
 *
 * @brief   �l��ԇƬ��ͻؑ�
 *
 * @param   stripType - �Д��ԇƬ���
 *
 * @return  �Ƿ�ɹ��l��
 */
uint8_t UART_SendStripTypeAck(StripType_TypeDef stripType)
{
    uint8_t data[3];
    extern uint16_t g_batteryVoltage;
    
    data[0] = stripType;
    data[1] = (uint8_t)(g_batteryVoltage >> 8);     // 늳�늉���λԪ�M
    data[2] = (uint8_t)(g_batteryVoltage & 0xFF);   // 늳�늉���λԪ�M
    
    printf("Sending strip type ack: %s, Battery: %dmV\r\n", StripType_GetName(stripType), g_batteryVoltage);
    
    return UART_SendPacket(CMD_STRIP_TYPE_ACK, data, 3);
}
