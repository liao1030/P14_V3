/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_UART.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2024/05/08
 * Description        : P14�๦�������x�x��UARTͨӍ̎��
 *********************************************************************************
 * Copyright (c) 2024 HMD. All rights reserved.
 *******************************************************************************/

#include "P14_UART.h"
#include "param_storage.h"
#include <string.h>

/* ���x���n�^ */
static uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
static uint16_t rxBufferHead = 0;
static uint16_t rxBufferTail = 0;

static uint8_t txBuffer[UART_TX_BUFFER_SIZE];
static uint8_t packetBuffer[MAX_PACKET_SIZE + 6]; // ��축��������ķ��

/* �f�h��B׃�� */
static uint8_t currentCode = 25;            // ��ǰԇ��У�a
static uint16_t currentEvent = EVENT_NONE;  // ��ǰ�¼����
uint8_t bloodSampleReady = 0;        // ѪҺ�ӱ��ʂ��B
uint8_t bloodCountdown = 0;          // ѪҺ�ӱ���Ӌ�r
uint8_t sampleProcessActive = 0;     // �ӱ�̎���B

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   ��ʼ��UARTͨӍ
 *
 * @return  none
 */
void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* USART2 TX (PA2) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2 RX (PA3) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2���� - ������115200��8����λ��1ֹͣλ���oУ�λ���o���� */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    /* ����USART2�����Д� */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* ���ý����Д� */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* ����USART2 */
    USART_Cmd(USART2, ENABLE);

    /* ��վ��n�^ */
    rxBufferHead = 0;
    rxBufferTail = 0;

    printf("UART��ʼ����ɣ�������115200\r\n");
}

/*********************************************************************
 * @fn      UART_SendData
 *
 * @brief   ͨ�^UART�l�͔���
 *
 * @param   data - ���l�͵Ĕ���
 * @param   len - �����L��
 *
 * @return  none
 */
void UART_SendData(uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, data[i]);
    }
}

/*********************************************************************
 * @fn      UART_CalculateChecksum
 *
 * @brief   Ӌ��У��
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  У��
 */
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint16_t sum = cmdId;
    for(uint8_t i = 0; i < dataLen; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      UART_SendPacket
 *
 * @brief   �l�ͅf�h���
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  none
 */
void UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    int idx = 0;
    
    // �M�b���
    txBuffer[idx++] = PROTOCOL_START_MARKER;
    txBuffer[idx++] = cmdId;
    txBuffer[idx++] = dataLen;
    
    if(dataLen > 0 && data != NULL)
    {
        memcpy(&txBuffer[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuffer[idx++] = UART_CalculateChecksum(cmdId, data, dataLen);
    txBuffer[idx++] = PROTOCOL_END_MARKER;
    
    // �l�ͷ��
    UART_SendData(txBuffer, idx);
    
    printf("�l�ͷ��: ID=0x%02X, �L��=%d\r\n", cmdId, dataLen);
}

/*********************************************************************
 * @fn      UART_SendError
 *
 * @brief   �l���e�`푑�
 *
 * @param   originalCmdId - ԭʼ����ID
 * @param   errorCode - �e�`���a
 *
 * @return  none
 */
void UART_SendError(uint8_t originalCmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    UART_SendPacket(CMD_ERROR_ACK, data, 2);
    
    printf("�l���e�`: ԭʼ����=0x%02X, �e�`�a=0x%02X\r\n", originalCmdId, errorCode);
}

/*********************************************************************
 * @fn      UART_HandleSyncTime
 *
 * @brief   ̎��ͬ���r�g����
 *
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  none
 */
void UART_HandleSyncTime(uint8_t *data, uint8_t dataLen)
{
    // �z�锵���L��
    if(dataLen != 7)
    {
        UART_SendError(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // �����r�g����
    uint16_t year = (data[0] << 8) | data[1];
    uint8_t month = data[2];
    uint8_t date = data[3];
    uint8_t hour = data[4];
    uint8_t minute = data[5];
    uint8_t second = data[6];
    
    // �O��ϵ�y�r�g
    if(PARAM_UpdateDateTime(year, month, date, hour, minute, second))
    {
        printf("ͬ���r�g: 20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
               year, month, date, hour, minute, second);
        
        // �l�ͳɹ�푑�
        uint8_t response = 0x00;  // �ɹ�
        UART_SendPacket(CMD_SYNC_TIME_ACK, &response, 1);
    }
    else
    {
        // �l���e�`푑�
        UART_SendError(CMD_SYNC_TIME, ERR_DATA_FORMAT);
    }
}

/*********************************************************************
 * @fn      UART_HandleDeviceStatusRequest
 *
 * @brief   ̎���O���BՈ��
 *
 * @return  none
 */
void UART_HandleDeviceStatusRequest(void)
{
    uint8_t data[8];
    uint16_t batteryVoltage;
    uint16_t temperature;
    uint16_t stripType;
    uint16_t stripStatus = 0; // �A�O������
    
    // �@ȡϵ�y����
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // �@ȡ��ǰ�y������
    stripType = basicParams.stripType;
    
    // �@ȡ늳�늉��͜ض�
    batteryVoltage = 3000; // ģ�Mֵ�����H����ADC�xȡ
    temperature = 250;     // ģ�Mֵ�����H���ĜضȸМy���xȡ
    
    // �z��늳�늉��Ƿ��^��
    if(batteryVoltage < 2600)  // 2.6V��������^��
    {
        stripStatus = ERR_LOW_BATTERY;
    }
    
    // �z��ض��Ƿ��ڹ�����
    if(temperature > basicParams.tempHighLimit * 10)
    {
        stripStatus = ERR_TEMP_HIGH;
    }
    else if(temperature < basicParams.tempLowLimit * 10)
    {
        stripStatus = ERR_TEMP_LOW;
    }
    
    // �M�b����
    data[0] = (stripType >> 8) & 0xFF;
    data[1] = stripType & 0xFF;
    data[2] = (stripStatus >> 8) & 0xFF;
    data[3] = stripStatus & 0xFF;
    data[4] = (batteryVoltage >> 8) & 0xFF;
    data[5] = batteryVoltage & 0xFF;
    data[6] = (temperature >> 8) & 0xFF;
    data[7] = temperature & 0xFF;
    
    // �l�ͻؑ�
    UART_SendPacket(CMD_DEVICE_STATUS_ACK, data, 8);
    
    printf("�O���BՈ��푑�: �Ŀ=%d, ��B=%d, 늉�=%dmV, �ض�=%.1f��C\r\n",
          stripType, stripStatus, batteryVoltage, temperature / 10.0);
}

/*********************************************************************
 * @fn      UART_HandleSetCodeEvent
 *
 * @brief   ̎���O��CODE��EVENT����
 *
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  none
 */
void UART_HandleSetCodeEvent(uint8_t *data, uint8_t dataLen)
{
    // �z�锵���L��
    if(dataLen != 3)
    {
        UART_SendError(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // �O��CODE��EVENT
    currentCode = data[0];
    currentEvent = (data[1] << 8) | data[2];
    
    printf("�O��CODE=%d, EVENT=%d\r\n", currentCode, currentEvent);
    
    // �O��ϵ�y����
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    basicParams.defaultEvent = currentEvent;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // 푑��_�J
    uint8_t response = 0x00;  // �ɹ�
    UART_SendPacket(CMD_CODE_EVENT_ACK, &response, 1);
}

/*********************************************************************
 * @fn      UART_HandleBloodSampleRequest
 *
 * @brief   ̎��ѪҺ�ӱ��z�yՈ��
 *
 * @return  none
 */
void UART_HandleBloodSampleRequest(void)
{
    // �z���O���B
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // ģ�M�z�y�^��
    if(bloodSampleReady == 0)
    {
        // ģ�MѪҺ߀δ�ʂ��
        UART_SendError(CMD_BLOOD_SAMPLE_STATUS, ERR_MEASURE_TIMEOUT);
    }
    else
    {
        // ѪҺ�ʂ���ˣ����ӵ�Ӌ�r
        if(!sampleProcessActive)
        {
            bloodCountdown = 5; // 5�뵹Ӌ�r
            sampleProcessActive = 1;
        }
        
        // �l�͵�Ӌ�r푑�
        UART_SendPacket(CMD_BLOOD_SAMPLE_ACK, &bloodCountdown, 1);
        
        printf("ѪҺ�z�y֪ͨ: ��Ӌ�r%d��\r\n", bloodCountdown);
        
        // ���µ�Ӌ�r
        if(bloodCountdown > 0)
        {
            bloodCountdown--;
        }
        else
        {
            // ��Ӌ�r�Y�����ʂ�l�ͽY��
            sampleProcessActive = 0;
        }
    }
}

/*********************************************************************
 * @fn      UART_HandleResultRequest
 *
 * @brief   ̎��Ո��yԇ�Y������
 *
 * @return  none
 */
void UART_HandleResultRequest(void)
{
    uint8_t data[25];
    int idx = 0;
    
    // �z���Ƿ��Мyԇ�Y��
    if(!sampleProcessActive && bloodCountdown == 0 && bloodSampleReady)
    {
        // �@ȡ��ǰ�r�g
        uint8_t year, month, date, hour, minute, second;
        PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second);
        
        // �@ȡ��ǰ�yԇ�Ŀ
        BasicSystemBlock basicParams;
        PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
        
        // �@ȡ���yԇӛ�
        TestRecord testRecord;
        uint16_t recordCount = PARAM_GetTestRecordCount();
        uint16_t actualCount = 0;
        
        if(recordCount > 0)
        {
            // �xȡ����ӛ�
            PARAM_ReadTestRecords(&testRecord, recordCount - 1, 1, &actualCount);
            
            // �M�b����
            data[idx++] = (testRecord.resultStatus >> 8) & 0xFF;
            data[idx++] = testRecord.resultStatus & 0xFF;
            data[idx++] = (testRecord.resultValue >> 8) & 0xFF;
            data[idx++] = testRecord.resultValue & 0xFF;
            data[idx++] = (testRecord.testType >> 8) & 0xFF;
            data[idx++] = testRecord.testType & 0xFF;
            data[idx++] = (testRecord.eventType >> 8) & 0xFF;
            data[idx++] = testRecord.eventType & 0xFF;
            data[idx++] = currentCode;  // ʹ�î�ǰУ�a
            
            // �r�g
            data[idx++] = (testRecord.year >> 8) & 0xFF;
            data[idx++] = testRecord.year & 0xFF;
            data[idx++] = (testRecord.month >> 8) & 0xFF;
            data[idx++] = testRecord.month & 0xFF;
            data[idx++] = (testRecord.date >> 8) & 0xFF;
            data[idx++] = testRecord.date & 0xFF;
            data[idx++] = (testRecord.hour >> 8) & 0xFF;
            data[idx++] = testRecord.hour & 0xFF;
            data[idx++] = (testRecord.minute >> 8) & 0xFF;
            data[idx++] = testRecord.minute & 0xFF;
            data[idx++] = (testRecord.second >> 8) & 0xFF;
            data[idx++] = testRecord.second & 0xFF;
            
            // 늉��͜ض�
            data[idx++] = (testRecord.batteryVoltage >> 8) & 0xFF;
            data[idx++] = testRecord.batteryVoltage & 0xFF;
            data[idx++] = (testRecord.temperature >> 8) & 0xFF;
            data[idx++] = testRecord.temperature & 0xFF;
            
            // �l�ͻؑ�
            UART_SendPacket(CMD_RESULT_ACK, data, idx);
            
            printf("�yԇ�Y��: ֵ=%d, �Ŀ=%d, �¼�=%d, У�a=%d\r\n",
                   testRecord.resultValue, testRecord.testType, 
                   testRecord.eventType, currentCode);
            
            // ����ѪҺ�ӱ���B
            bloodSampleReady = 0;
        }
        else
        {
            // �]�Мyԇӛ䛣��l���e�`
            UART_SendError(CMD_REQ_RESULT, ERR_DATA_FORMAT);
        }
    }
    else
    {
        // �l���e�`
        UART_SendError(CMD_REQ_RESULT, ERR_MEASURE_TIMEOUT);
    }
}

/*********************************************************************
 * @fn      UART_HandleRawDataRequest
 *
 * @brief   ̎��Ո��RAW DATA����
 *
 * @return  none
 */
void UART_HandleRawDataRequest(void)
{
    // RAW DATA���rʹ��ģ�M����
    uint8_t rawData[50];
    
    // ���ģ�M��RAW DATA
    for(int i = 0; i < 50; i++)
    {
        rawData[i] = i;
    }
    
    uint8_t data[52];
    data[0] = 0;  // ��λ
    data[1] = 50; // �L��
    memcpy(&data[2], rawData, 50);
    
    // �l�ͻؑ�
    UART_SendPacket(CMD_RAW_DATA_ACK, data, 52);
    
    printf("RAW DATAՈ����푑�\r\n");
}

/*********************************************************************
 * @fn      UART_ForwardBLECommand
 *
 * @brief   ��BLE�����D�l��CH32V203G8R6
 *
 * @param   cmdId - ����ID
 * @param   data - ����
 * @param   dataLen - �����L��
 *
 * @return  none
 */
void UART_ForwardBLECommand(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // ��������ID�M������̎��
    switch(cmdId)
    {
        case CMD_SYNC_TIME:
            UART_HandleSyncTime(data, dataLen);
            break;
            
        case CMD_REQ_DEVICE_STATUS:
            UART_HandleDeviceStatusRequest();
            break;
            
        case CMD_SET_CODE_EVENT:
            UART_HandleSetCodeEvent(data, dataLen);
            break;
            
        case CMD_BLOOD_SAMPLE_STATUS:
            UART_HandleBloodSampleRequest();
            break;
            
        case CMD_REQ_RESULT:
            UART_HandleResultRequest();
            break;
            
        case CMD_REQ_RAW_DATA:
            UART_HandleRawDataRequest();
            break;
            
        default:
            // ��֧�ֵ�����
            UART_SendError(cmdId, ERR_COMMAND_UNSUPPORTED);
            break;
    }
}

/*********************************************************************
 * @fn      UART_ProcessData
 *
 * @brief   ̎����յ���UART����
 *
 * @return  none
 */
void UART_ProcessData(void)
{
    static enum {
        STATE_IDLE,
        STATE_CMD,
        STATE_LEN,
        STATE_DATA,
        STATE_CHECKSUM,
        STATE_END
    } state = STATE_IDLE;
    
    static uint8_t cmdId = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataIdx = 0;
    static uint8_t checksum = 0;
    
    // �z���Ƿ��Д�����̎��
    while(rxBufferHead != rxBufferTail)
    {
        uint8_t data = rxBuffer[rxBufferTail];
        rxBufferTail = (rxBufferTail + 1) % UART_RX_BUFFER_SIZE;
        
        // �f�h��B�C
        switch(state)
        {
            case STATE_IDLE:
                if(data == PROTOCOL_START_MARKER)
                {
                    packetBuffer[0] = data;
                    state = STATE_CMD;
                }
                break;
                
            case STATE_CMD:
                cmdId = data;
                packetBuffer[1] = data;
                state = STATE_LEN;
                break;
                
            case STATE_LEN:
                dataLen = data;
                packetBuffer[2] = data;
                
                if(dataLen == 0)
                {
                    state = STATE_CHECKSUM;
                }
                else if(dataLen > MAX_PACKET_SIZE)
                {
                    // �����L�ȳ������ޣ����à�B
                    state = STATE_IDLE;
                }
                else
                {
                    dataIdx = 0;
                    state = STATE_DATA;
                }
                break;
                
            case STATE_DATA:
                packetBuffer[3 + dataIdx] = data;
                dataIdx++;
                
                if(dataIdx >= dataLen)
                {
                    state = STATE_CHECKSUM;
                }
                break;
                
            case STATE_CHECKSUM:
                checksum = data;
                packetBuffer[3 + dataLen] = data;
                state = STATE_END;
                break;
                
            case STATE_END:
                packetBuffer[4 + dataLen] = data;
                
                if(data == PROTOCOL_END_MARKER)
                {
                    // �z��У��
                    uint8_t calculatedChecksum = UART_CalculateChecksum(cmdId, &packetBuffer[3], dataLen);
                    
                    if(calculatedChecksum == checksum)
                    {
                        // У����_��̎������
                        UART_ForwardBLECommand(cmdId, &packetBuffer[3], dataLen);
                    }
                    else
                    {
                        // У��e�`
                        UART_SendError(cmdId, ERR_CHECKSUM);
                    }
                }
                
                // ���à�B
                state = STATE_IDLE;
                break;
        }
    }
}

/*********************************************************************
 * @fn      UART_RxCallback
 *
 * @brief   UART���Ք������{
 *
 * @param   data - ����
 * @param   len - �L��
 *
 * @return  none
 */
void UART_RxCallback(uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        // ������������վ��n�^
        rxBuffer[rxBufferHead] = data[i];
        rxBufferHead = (rxBufferHead + 1) % UART_RX_BUFFER_SIZE;
        
        // �z�龏�n�^�Ƿ����
        if(rxBufferHead == rxBufferTail)
        {
            // ���n�^�ѝM���G������Ĕ���
            rxBufferTail = (rxBufferTail + 1) % UART_RX_BUFFER_SIZE;
        }
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   USART2�Д�̎����
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // �xȡ���յ��Ĕ���
        uint8_t data = USART_ReceiveData(USART2);
        
        // �{�ý��ջ��{
        UART_RxCallback(&data, 1);
        
        // ����Д����I
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
} 