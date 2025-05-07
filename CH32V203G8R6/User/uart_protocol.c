#include "uart_protocol.h"
#include "debug.h"
#include "flash_param.h"
#include <string.h>

/* ȫ��׃�����x */
static UartProtocol_TypeDef rxPacket;  // �����Y�ϰ�
static UartRxState_TypeDef rxState;    // ���ՠ�B�C
static uint8_t rxDataCount;            // �Y��Ӌ����
static uint8_t txBuffer[PROTOCOL_MAX_PACKET_LEN]; // �l�;��n�^
static TestRecord_TypeDef Test_Record; // �yԇӛ�

/* DMA ���վ��n�^ */
static uint8_t rxDMABuffer[PROTOCOL_MAX_PACKET_LEN];
static volatile uint8_t dmaRxCompleteFlag = 0;  // DMA������ɘ�־

/* ͨ���e�`푑����� */
static uint8_t errorData[2];

/* ����ǰ���� */
static void UART_Process_Packet(void);
static uint8_t UART_Find_Packet_In_Buffer(uint8_t *buffer, uint16_t size);

/*********************************************************************
 * @fn      UART_Calculate_Checksum
 *
 * @brief   Ӌ��У��
 *
 * @param   cmdId   - ָ��ID
 *          dataLen - �Y���L��
 *          data    - �Y��ָ�
 *
 * @return  Ӌ��õ���У��
 */
uint8_t UART_Calculate_Checksum(uint8_t cmdId, uint8_t dataLen, uint8_t *data)
{
    uint16_t sum = cmdId + dataLen;
    
    for(uint8_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      UART_Find_Packet_In_Buffer
 *
 * @brief   ��DMA���n�^�в��������Ĕ�����
 *
 * @param   buffer  - DMA���n�^
 *          size    - ���n�^��С
 *
 * @return  �ҵ���������������1����t����0
 */
static uint8_t UART_Find_Packet_In_Buffer(uint8_t *buffer, uint16_t size)
{
    uint16_t i = 0;
    uint8_t dataLen = 0;
    
    // ������ʼ��ӛ
    while(i < size - PROTOCOL_HEADER_LEN - PROTOCOL_FOOTER_LEN)
    {
        if(buffer[i] == PROTOCOL_START_MARK)
        {
            // �@ȡ�����L��
            dataLen = buffer[i + 2];
            
            // �z�龏�n�^�Ƿ������Ĕ���
            if(i + PROTOCOL_HEADER_LEN + dataLen + PROTOCOL_FOOTER_LEN <= size)
            {
                // �z��Y����ӛ
                if(buffer[i + PROTOCOL_HEADER_LEN + dataLen + 1] == PROTOCOL_END_MARK)
                {
                    // �}�u������rxPacket
                    rxPacket.startMark = buffer[i];
                    rxPacket.cmdId = buffer[i + 1];
                    rxPacket.dataLen = dataLen;
                    memcpy(rxPacket.data, &buffer[i + 3], dataLen);
                    rxPacket.checksum = buffer[i + PROTOCOL_HEADER_LEN + dataLen];
                    rxPacket.endMark = buffer[i + PROTOCOL_HEADER_LEN + dataLen + 1];
                    
                    // ̎������������
                    UART_Process_Packet();
                    return 1;
                }
            }
        }
        i++;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   ��ʼ��UARTͨӍ�f�h
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    // ��ʼ��UART2
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};
    
    // ����UART2��GPIOA��DMA1�r�
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // �O��UART2 Tx (PA2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // �O��UART2 Rx (PA3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ����UART2����
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    // ����DMA���գ�UART2 Rxʹ��DMA1 Channel6
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rxDMABuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = PROTOCOL_MAX_PACKET_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    // ����DMA�Д�
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
    
    // ����DMA�Д����ȼ�
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ����UART2 DMA����
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    
    // ����DMAͨ��
    DMA_Cmd(DMA1_Channel6, ENABLE);
    
    // ����UART2�Д����ȼ� (����UART�Д���̎���e�`�Ϳ��f�z�y)
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ����UART2���պ��e�`�Д࣬�Լ����f�Д�
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); // ����RXNE�Д࣬ʹ��DMA����
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);  // ����IDLE�Д࣬��춙z�y�������Y��
    USART_ITConfig(USART2, USART_IT_PE | USART_IT_ERR, ENABLE); // ������żУ��e�`�������e�`�Д�
    
    // ����UART2
    USART_Cmd(USART2, ENABLE);
    
    // ��ʼ�����ՠ�B�C
    rxState = UART_RX_WAIT_START;
    rxDataCount = 0;
    dmaRxCompleteFlag = 0;
    
    // ���DMA���վ��n�^
    memset(rxDMABuffer, 0, PROTOCOL_MAX_PACKET_LEN);
}

/*********************************************************************
 * @fn      UART_Send_Byte
 *
 * @brief   �l�͆΂��ֹ�����
 *
 * @param   b - Ҫ�l�͵��ֹ�
 *
 * @return  none
 */
static void UART_Send_Byte(uint8_t b)
{
    // �ȴ��l�;��n�^���
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    
    // �l�͔���
    USART_SendData(USART2, b);
}

/*********************************************************************
 * @fn      UART_Send_Packet
 *
 * @brief   �l�͔�����
 *
 * @param   cmdId   - ָ��ID
 *          data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Send_Packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint8_t checksum;
    uint16_t i = 0;
    
    // Ӌ��У��
    checksum = UART_Calculate_Checksum(cmdId, dataLen, data);
    
    // �����l�͔���
    txBuffer[i++] = PROTOCOL_START_MARK;
    txBuffer[i++] = cmdId;
    txBuffer[i++] = dataLen;
    
    // ��Ӕ���
    for(uint8_t j = 0; j < dataLen; j++) {
        txBuffer[i++] = data[j];
    }
    
    // ���У�ͺͽY����ӛ
    txBuffer[i++] = checksum;
    txBuffer[i++] = PROTOCOL_END_MARK;
    
    // �l�͔�����
    for(uint16_t j = 0; j < i; j++) {
        UART_Send_Byte(txBuffer[j]);
    }
}

/*********************************************************************
 * @fn      UART_Send_Error
 *
 * @brief   �l���e�`푑�
 *
 * @param   originalCmd - ԭʼָ��ID
 *          errorCode   - �e�`���a
 *
 * @return  none
 */
void UART_Send_Error(uint8_t originalCmd, uint8_t errorCode)
{
    errorData[0] = originalCmd;
    errorData[1] = errorCode;
    
    UART_Send_Packet(CMD_ERROR_ACK, errorData, 2);
}

/*********************************************************************
 * @fn      UART_Process_Packet
 *
 * @brief   ̎���������յĔ�����
 *
 * @return  none
 */
static void UART_Process_Packet(void)
{
    // ����Ӌ��У��
    uint8_t calcChecksum = UART_Calculate_Checksum(rxPacket.cmdId, rxPacket.dataLen, rxPacket.data);
    
    // У�͙z��
    if(calcChecksum != rxPacket.checksum) {
        UART_Send_Error(rxPacket.cmdId, ERR_CHECKSUM);
        return;
    }
    
    // ����ָ��ID��������̎��
    switch(rxPacket.cmdId) {
        case CMD_SYNC_TIME:
            UART_Handle_SyncTime(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_STATUS:
            UART_Handle_ReqStatus(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_SET_CODE_EVENT:
            UART_Handle_SetCodeEvent(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_CHECK_BLOOD:
            UART_Handle_CheckBlood(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_RESULT:
            UART_Handle_ReqResult(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_RAW_DATA:
            UART_Handle_ReqRawData(rxPacket.data, rxPacket.dataLen);
            break;
            
        default:
            // ��֧Ԯ��ָ��
            UART_Send_Error(rxPacket.cmdId, ERR_COMMAND_UNSUPPORTED);
            break;
    }
}

/*********************************************************************
 * @fn      UART_Protocol_Process
 *
 * @brief   ̎����յ����ֹ�����
 *
 * @param   rxByte - ���յ����ֹ�
 *
 * @return  none
 */
void UART_Protocol_Process(uint8_t rxByte)
{
    switch(rxState) {
        case UART_RX_WAIT_START:
            if(rxByte == PROTOCOL_START_MARK) {
                rxPacket.startMark = rxByte;
                rxState = UART_RX_WAIT_CMD;
            }
            break;
            
        case UART_RX_WAIT_CMD:
            rxPacket.cmdId = rxByte;
            rxState = UART_RX_WAIT_LEN;
            break;
            
        case UART_RX_WAIT_LEN:
            rxPacket.dataLen = rxByte;
            rxDataCount = 0;
            
            if(rxPacket.dataLen > 0) {
                rxState = UART_RX_WAIT_DATA;
            } else {
                rxState = UART_RX_WAIT_CHECKSUM;
            }
            break;
            
        case UART_RX_WAIT_DATA:
            rxPacket.data[rxDataCount++] = rxByte;
            
            if(rxDataCount >= rxPacket.dataLen) {
                rxState = UART_RX_WAIT_CHECKSUM;
            }
            break;
            
        case UART_RX_WAIT_CHECKSUM:
            rxPacket.checksum = rxByte;
            rxState = UART_RX_WAIT_END;
            break;
            
        case UART_RX_WAIT_END:
            rxPacket.endMark = rxByte;
            
            if(rxByte == PROTOCOL_END_MARK) {
                // ���������M��̎��
                UART_Process_Packet();
            } else {
                // �Y����ӛ�e�`
                UART_Send_Error(rxPacket.cmdId, ERR_DATA_FORMAT);
            }
            
            // ���O��B�C
            rxState = UART_RX_WAIT_START;
            break;
            
        default:
            rxState = UART_RX_WAIT_START;
            break;
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
    /* ̎��USART2���f�Д࣬��춙z�yһ�������Ľ������ */
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        /* �xȡUSART2->STATR�����xȡUSART2->DATAR���IDLE�Д����I */
        volatile uint8_t temp = USART2->STATR;
        temp = USART2->DATAR;
        (void)temp; // ���⾎�g������
        
        /* Ӌ����յ��Ĕ����L�� */
        uint16_t rxCount = PROTOCOL_MAX_PACKET_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        /* �����Ք����L�ȴ����С��Ч�������L�ȕr���Lԇ���� */
        if(rxCount >= (PROTOCOL_HEADER_LEN + PROTOCOL_FOOTER_LEN))
        {
            /* �O��DMA������ɘ�־������ѭ�h��̎�� */
            dmaRxCompleteFlag = 1;
        }
    }
    
    /* ̎������UART�e�`�Д� */
    if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE | USART_IT_ORE) != RESET)
    {
        /* ����e�`�Д����I������o���Д� */
        USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE | USART_IT_ORE);
    }
}

/*********************************************************************
 * @fn      DMA1_Channel6_IRQHandler
 *
 * @brief   DMA1 Channel6��UART2���գ��Д�̎����
 *
 * @return  none
 */
void DMA1_Channel6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC6))
    {
        /* ���DMA��ݔ����Д����I */
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        
        /* �O��DMA������ɘ�־������ѭ�h��̎����յ��Ĕ��� */
        dmaRxCompleteFlag = 1;
    }
}

/*********************************************************************
 * @fn      UART_Handle_SyncTime
 *
 * @brief   ̎��ͬ���r�gָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_SyncTime(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    
    // �z�锵���L��
    if(dataLen != 7) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // �����r�g����
    uint16_t year = (data[0] << 8) | data[1];
    uint8_t month = data[2];
    uint8_t day = data[3];
    uint8_t hour = data[4];
    uint8_t minute = data[5];
    uint8_t second = data[6];
    
    // �z��r�gֵ�ĺϷ���
    if(year < 2000 || year > 2099 || 
       month < 1 || month > 12 || 
       day < 1 || day > 31 || 
       hour > 23 || minute > 59 || second > 59) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // ����ϵ�y�r�g (�浽��������)
    ParamCodeTable.Time.YEAR = year - 2000;
    ParamCodeTable.Time.MONTH = month;
    ParamCodeTable.Time.DATE = day;
    ParamCodeTable.Time.HOUR = hour;
    ParamCodeTable.Time.MINUTE = minute;
    ParamCodeTable.Time.SECOND = second;
    
    // ����r�g��Flash��ʹ��PARAM_UpdateDateTime���PARAM_SaveTime
    FLASH_ParamResult_TypeDef result = PARAM_UpdateDateTime(
        year - 2000, month, day, hour, minute, second);
    
    if(result != PARAM_SUCCESS) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_HARDWARE);
        return;
    }
    
    // �ɹ�푑�
    response[0] = 0x00;  // �ɹ���B
    UART_Send_Packet(CMD_SYNC_TIME_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_ReqStatus
 *
 * @brief   ̎��Ո���b�à�Bָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_ReqStatus(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[8];
    uint16_t stripType, stripStatus, batteryVoltage, temperature;
    
    // �@ȡ��ǰ�yԇ�Ŀ
    stripType = ParamCodeTable.Display.Strip_Type;
    
    // �@ȡԇƬ��B
    // �@�e��ԓ��Ӳ�w�z�yԇƬ��B�Ĵ��a
    // ���r���OԇƬ����
    stripStatus = 0x0000;
    
    // �@ȡ늳�늉� (��λ: mV)
    // �@�e��ԓ��ADC�xȡ늳�늉��Ĵ��a
    // ���rʹ��ʾ��ֵ
    batteryVoltage = 0x0B01;  // 2.817V
    
    // �@ȡ�h���ض� (��λ: �z�϶� x 10)
    // �@�e��ԓ���xȡ�ضȸМy���Ĵ��a
    // ���rʹ��ʾ��ֵ
    temperature = 0x0119;  // 28.1��C
    
    // �z��늳غ͜ضȠ�B
    if(batteryVoltage < 2200) {  // 늳ص��2.2V
        stripStatus = 0x0001;  // 늳�����^��
    } else if(temperature > 400) {  // �ضȸ��40��C
        stripStatus = 0x0002;  // �ض��^��
    } else if(temperature < 100) {  // �ضȵ��10��C
        stripStatus = 0x0003;  // �ض��^��
    }
    
    // �����ؑ�����
    response[0] = (stripType >> 8) & 0xFF;
    response[1] = stripType & 0xFF;
    response[2] = (stripStatus >> 8) & 0xFF;
    response[3] = stripStatus & 0xFF;
    response[4] = (batteryVoltage >> 8) & 0xFF;
    response[5] = batteryVoltage & 0xFF;
    response[6] = (temperature >> 8) & 0xFF;
    response[7] = temperature & 0xFF;
    
    // �l�ͻؑ�
    UART_Send_Packet(CMD_STATUS_ACK, response, 8);
}

/*********************************************************************
 * @fn      UART_Handle_SetCodeEvent
 *
 * @brief   ̎���O��CODE��EVENTָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_SetCodeEvent(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    
    // �z�锵���L��
    if(dataLen != 3) {
        UART_Send_Error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // ��������
    uint8_t code = data[0];
    uint16_t event = (data[1] << 8) | data[2];
    
    // �z��EVENTֵ�ĺϷ���
    if(event > EVENT_QC) {
        UART_Send_Error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // ����CODE��EVENT���yԇӛ�
    Test_Record.Strip_Code = code;
    Test_Record.EVENT = event;
    
    // �ɹ�푑�
    response[0] = 0x00;  // �ɹ���B
    UART_Send_Packet(CMD_CODE_EVENT_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_CheckBlood
 *
 * @brief   ̎��z�y��BՈ��ָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_CheckBlood(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    uint8_t bloodDetected = 0;  // �Ƿ�z�y��ѪҺ
    uint8_t countdownSec = 5;   // ����Ӌ�r�딵
    
    // �z���Ƿ��ѽ��z�y��ѪҺ
    // �@�e��ԓ�Йz�yѪҺ��Ӳ�w���a
    // ���O��ǰ�]�Йz�y��ѪҺ�������e�`
    if(!bloodDetected) {
        UART_Send_Error(CMD_CHECK_BLOOD, ERR_TIMEOUT);
        return;
    }
    
    // �����ؑ�����
    response[0] = countdownSec;
    
    // �l�ͻؑ�
    UART_Send_Packet(CMD_BLOOD_DETECTED_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_ReqResult
 *
 * @brief   ̎��Ո��yԇ�Y��ָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_ReqResult(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[25];
    uint16_t resultStatus, testValue, stripType;
    uint16_t year, month, day, hour, minute, second;
    uint16_t batteryVoltage, temperature;
    
    // �z��yԇ�Ƿ������
    // �@�e��ԓ�Йz��yԇ��B�Ĵ��a
    // ���r���O�yԇ������ҳɹ�
    resultStatus = 0x0000;  // �ɹ�
    
    // �@ȡ�yԇ�Y��
    // �����yԇ�Ŀ��ͫ@ȡ�yԇֵ
    stripType = ParamCodeTable.Display.Strip_Type;
    switch(stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            testValue = 123;  // ���Oֵ��12.3 mmol/L
            break;
            
        case STRIP_TYPE_U:
            testValue = 350;  // ���Oֵ��350 umol/L
            break;
            
        case STRIP_TYPE_C:
            testValue = 52;   // ���Oֵ��5.2 mmol/L
            break;
            
        case STRIP_TYPE_TG:
            testValue = 17;   // ���Oֵ��1.7 mmol/L
            break;
            
        default:
            testValue = 0;
            break;
    }
    
    // �@ȡ�r�g
    year = 2000 + ParamCodeTable.Time.YEAR;
    month = ParamCodeTable.Time.MONTH;
    day = ParamCodeTable.Time.DATE;
    hour = ParamCodeTable.Time.HOUR;
    minute = ParamCodeTable.Time.MINUTE;
    second = ParamCodeTable.Time.SECOND;
    
    // �@ȡ늳�늉�
    batteryVoltage = 0x0B01;  // 2.817V
    
    // �@ȡ�h���ض�
    temperature = 0x0119;  // 28.1��C
    
    // �����ؑ�����
    response[0] = (resultStatus >> 8) & 0xFF;
    response[1] = resultStatus & 0xFF;
    response[2] = (testValue >> 8) & 0xFF;
    response[3] = testValue & 0xFF;
    response[4] = (stripType >> 8) & 0xFF;
    response[5] = stripType & 0xFF;
    response[6] = (Test_Record.EVENT >> 8) & 0xFF;
    response[7] = Test_Record.EVENT & 0xFF;
    response[8] = Test_Record.Strip_Code;
    response[9] = (year >> 8) & 0xFF;
    response[10] = year & 0xFF;
    response[11] = (month >> 8) & 0xFF;
    response[12] = month & 0xFF;
    response[13] = (day >> 8) & 0xFF;
    response[14] = day & 0xFF;
    response[15] = (hour >> 8) & 0xFF;
    response[16] = hour & 0xFF;
    response[17] = (minute >> 8) & 0xFF;
    response[18] = minute & 0xFF;
    response[19] = (second >> 8) & 0xFF;
    response[20] = second & 0xFF;
    response[21] = (batteryVoltage >> 8) & 0xFF;
    response[22] = batteryVoltage & 0xFF;
    response[23] = (temperature >> 8) & 0xFF;
    response[24] = temperature & 0xFF;
    
    // �l�ͻؑ�
    UART_Send_Packet(CMD_RESULT_ACK, response, 25);
}

/*********************************************************************
 * @fn      UART_Handle_ReqRawData
 *
 * @brief   ̎��Ո��RAW DATAָ��
 *
 * @param   data    - �Y��ָ�
 *          dataLen - �Y���L��
 *
 * @return  none
 */
void UART_Handle_ReqRawData(uint8_t *data, uint8_t dataLen)
{
    // RAW DATA푑�����������������Ҫ�ֶ�̎��
    // �@�eֻչʾ�����Y�������H��������Ҫ�������H��RAW DATA����
    
    // ʾ��: ������С��RAW DATA푑�
    uint8_t response[50];  // ���H�����п�����Ҫ����ľ��n�^
    uint16_t rawDataLen = 48;  // RAW DATA���L��
    
    // �O��RAW DATA�L��
    response[0] = (rawDataLen >> 8) & 0xFF;
    response[1] = rawDataLen & 0xFF;
    
    // �@�e���댍�H��RAW DATA
    // ���H�����У��@Щ������ԓ�Ĝyԇ�^���Ы@ȡ
    // ���ʾ������
    memset(&response[2], 0, 48);
    
    // �l��RAW DATA푑�
    UART_Send_Packet(CMD_RAW_DATA_ACK, response, 50);
}

/*********************************************************************
 * @fn      UART_Check_DMA_Received_Data
 *
 * @brief   �z��K̎��DMA���յ��Ĕ���
 *
 * @return  none
 */
void UART_Check_DMA_Received_Data(void)
{
    if(dmaRxCompleteFlag)
    {
        /* ���DMA������ɘ�־ */
        dmaRxCompleteFlag = 0;
        
        /* Ӌ����յ��Ĕ����L�� */
        uint16_t rxCount = PROTOCOL_MAX_PACKET_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        /* �����Ք����L�ȴ����С��Ч�������L�ȕr���Lԇ���� */
        if(rxCount >= (PROTOCOL_HEADER_LEN + PROTOCOL_FOOTER_LEN))
        {
            /* ��DMA���n�^�Ќ��ҁK̎������������ */
            UART_Find_Packet_In_Buffer(rxDMABuffer, rxCount);
        }
    }
}