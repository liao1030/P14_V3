/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 This example demonstrates receiving indeterminate length data via USART's IDLE interrupt and DMA.
 USART2_Tx(PA2), USART2_Rx(PA3).


*/

#include "debug.h"
#include "string.h"
#include "param_table.h"

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

// ͨӍ�f�h���P���x
#define PKT_START_MARK    (0xAA)
#define PKT_END_MARK      (0x55)

// The length of a ring buffer used to store received data
#define RING_BUFFER_LEN   (512u)

// ����x
#define CMD_SYNC_TIME     (0x01)
#define CMD_TIME_SYNC_ACK (0x81)
#define CMD_REQ_STATUS    (0x02)
#define CMD_STATUS_ACK    (0x82)
#define CMD_SET_PARAMS    (0x03)
#define CMD_PARAMS_ACK    (0x83)
#define CMD_CHK_BLOOD     (0x04)
#define CMD_BLOOD_NOTIFY  (0x84)
#define CMD_REQ_RESULT    (0x05)
#define CMD_RESULT_ACK    (0x85)
#define CMD_REQ_RAW       (0x06)
#define CMD_RAW_ACK       (0x86)
#define CMD_ERROR_ACK     (0xFF)

// �e�`���a
#define ERR_NONE          (0x00)
#define ERR_LOW_BATTERY   (0x01)
#define ERR_TEMP_HIGH     (0x02)
#define ERR_TEMP_LOW      (0x03)
#define ERR_STRIP_EXPIRED (0x04)
#define ERR_STRIP_USED    (0x05)
#define ERR_STRIP_ERROR   (0x06)
#define ERR_BLOOD_INSUFF  (0x07)
#define ERR_TEST_TIMEOUT  (0x08)
#define ERR_CALIBRATION   (0x09)
#define ERR_HARDWARE      (0x0A)
#define ERR_COMM_ERROR    (0x0B)
#define ERR_DATA_FORMAT   (0x0C)
#define ERR_CHECKSUM      (0x0D)
#define ERR_CMD_UNSUPPORT (0x0E)
#define ERR_OUT_OF_RANGE  (0x0F)

// �yԇ���
#define TEST_TYPE_GLV     (0x0000)
#define TEST_TYPE_U       (0x0001)
#define TEST_TYPE_C       (0x0002)
#define TEST_TYPE_TG      (0x0003)
#define TEST_TYPE_GAV     (0x0004)

// �¼����
#define EVENT_NONE        (0x0000)
#define EVENT_AC          (0x0001)
#define EVENT_PC          (0x0002)
#define EVENT_QC          (0x0003)

// The length of a single buffer used by DMA
#define RX_BUFFER_LEN       (128u)

#define USART_RX_CH         DMA1_Channel6

// ���x������L��
#define MAX_PACKET_SIZE    (128u)

// ����Y�����x
typedef struct {
    uint8_t cmd;           // ����ID
    uint8_t length;        // �Y���L��
    uint8_t data[MAX_PACKET_SIZE]; // �Y�σ���
    uint8_t checksum;      // У��
} UART_Packet_t;

// ȫ��׃��
UART_Packet_t g_rxPacket;  // ���շ��
uint8_t g_txBuffer[MAX_PACKET_SIZE + 5]; // �l�;��n�^ (��ʼ��ӛ + ���� + �L�� + �Y�� + У�� + �Y����ӛ)
uint16_t g_testValue = 123; // ģ�M�yԇֵ
uint16_t g_testType = TEST_TYPE_GLV; // ģ�M�yԇ���
uint16_t g_eventType = EVENT_NONE; // ģ�M�¼����
uint8_t g_testCode = 25; // ģ�M�yԇCODE
uint8_t g_countdownSeconds = 5; // ģ�M�����딵

// ϵ�y�r�g�Y��
typedef struct {
    uint16_t year;   // ���
    uint8_t month;   // �·� (1-12)
    uint8_t day;     // ���� (1-31)
    uint8_t hour;    // С�r (0-23)
    uint8_t minute;  // ��� (0-59)
    uint8_t second;  // �� (0-59)
} DateTime_t;

DateTime_t g_systemTime = {2025, 3, 7, 15, 32, 59}; // ϵ�y�r�g

struct
{
    volatile uint8_t DMA_USE_BUFFER;
    uint8_t          Rx_Buffer[2][RX_BUFFER_LEN];

} USART_DMA_CTRL = {
    .DMA_USE_BUFFER = 0,
    .Rx_Buffer      = {0},
};

struct
{
    uint8_t           buffer[RING_BUFFER_LEN];
    volatile uint16_t RecvPos;  //
    volatile uint16_t SendPos;  //
    volatile uint16_t RemainCount;

} ring_buffer = {{0}, 0, 0, 0};

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 peripheral.
 *
 * @return  none
 */
void USARTx_CFG(uint32_t baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure  = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef  NVIC_InitStructure  = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}


/*********************************************************************
 * @fn      DMA_INIT
 *
 * @brief   Configures the DMA for USART2.
 *
 * @return  none
 */
void DMA_INIT(void)
{
    DMA_InitTypeDef  DMA_InitStructure  = {0};
    NVIC_InitTypeDef NVIC_InitStructure  = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(USART_RX_CH);
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)USART_DMA_CTRL.Rx_Buffer[0];
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = RX_BUFFER_LEN;
    DMA_Init(USART_RX_CH, &DMA_InitStructure);

    DMA_ITConfig(USART_RX_CH, DMA_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(USART_RX_CH, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}


/*********************************************************************
 * @fn      ring_buffer_push_huge
 *
 * @brief   Put a large amount of data into the ring buffer.
 *
 * @return  none
 */
void ring_buffer_push_huge(uint8_t *buffer, uint16_t len)
{
    const uint16_t bufferRemainCount = RING_BUFFER_LEN - ring_buffer.RemainCount;
    if (bufferRemainCount < len)
    {
        len = bufferRemainCount;
    }

    const uint16_t bufferSize = RING_BUFFER_LEN - ring_buffer.RecvPos;
    if (bufferSize >= len)
    {
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, len);
        ring_buffer.RecvPos += len;
    }
    else
    {
        uint16_t otherSize = len - bufferSize;
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, bufferSize);
        memcpy(ring_buffer.buffer, &(buffer[bufferSize]), otherSize);
        ring_buffer.RecvPos = otherSize;
    }
    ring_buffer.RemainCount += len;
}

/*********************************************************************
 * @fn      ring_buffer_pop
 *
 * @brief   Get a data from the ring buffer.
 *
 * @return  the Data
 */
uint8_t ring_buffer_pop()
{
    uint8_t data = ring_buffer.buffer[ring_buffer.SendPos];

    ring_buffer.SendPos++;
    if (ring_buffer.SendPos >= RING_BUFFER_LEN)
    {
        ring_buffer.SendPos = 0;
    }
    ring_buffer.RemainCount--;
    return data;
}

/*********************************************************************
 * @fn      Calculate_Checksum
 *
 * @brief   Calculate the checksum of a packet.
 *
 * @return  the checksum
 */
uint8_t Calculate_Checksum(uint8_t cmd, uint8_t length, uint8_t *data)
{
    uint16_t sum = cmd + length;
    for (uint8_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      UART_SendPacket
 *
 * @brief   Send a UART packet.
 *
 * @return  none
 */
void UART_SendPacket(uint8_t cmd, uint8_t length, uint8_t *data)
{
    uint16_t index = 0;
    
    // ��ʼ��ӛ
    g_txBuffer[index++] = PKT_START_MARK;
    
    // ����
    g_txBuffer[index++] = cmd;
    
    // �L��
    g_txBuffer[index++] = length;
    
    // �Y��
    for (uint8_t i = 0; i < length; i++) {
        g_txBuffer[index++] = data[i];
    }
    
    // У��
    g_txBuffer[index++] = Calculate_Checksum(cmd, length, data);
    
    // �Y����ӛ
    g_txBuffer[index++] = PKT_END_MARK;
    
    // ͨ�^USART2�l�͔���
    for (uint16_t i = 0; i < index; i++) {
        USART_SendData(USART2, g_txBuffer[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

/*********************************************************************
 * @fn      Send_Error_Response
 *
 * @brief   Send an error response.
 *
 * @return  none
 */
void Send_Error_Response(uint8_t original_cmd, uint8_t error_code)
{
    uint8_t data[2];
    data[0] = original_cmd;
    data[1] = error_code;
    UART_SendPacket(CMD_ERROR_ACK, 2, data);
}

/*********************************************************************
 * @fn      Handle_Sync_Time
 *
 * @brief   Handle the sync time command (0x01).
 *
 * @return  none
 */
void Handle_Sync_Time(uint8_t *data, uint8_t length)
{
    if(length == 7) {
        // �����r�g�Y��
        g_systemTime.year = 2000 + data[0];
        g_systemTime.month = data[1];
        g_systemTime.day = data[2];
        g_systemTime.hour = data[3];
        g_systemTime.minute = data[4];
        g_systemTime.second = data[5];
        
        // �ؑ��r�gͬ���_�J
        uint8_t response[1] = {ERR_NONE};
        UART_SendPacket(CMD_TIME_SYNC_ACK, 1, response);
    } else {
        // �����L���e�`
        Send_Error_Response(CMD_SYNC_TIME, ERR_DATA_FORMAT);
    }
}

/*********************************************************************
 * @fn      Handle_Request_Status
 *
 * @brief   Handle the request status command (0x02).
 *
 * @return  none
 */
void Handle_Request_Status(void)
{
    uint8_t response[8];
    uint16_t battery_mv = 2817; // ģ�M늳�늉��� 2.817V
    uint16_t temperature = 281; // ģ�M�ضȞ� 28.1��C
    
    // �M�b��B�ؑ�
    response[0] = (uint8_t)(g_testType >> 8);
    response[1] = (uint8_t)(g_testType & 0xFF);
    response[2] = 0x00; // ԇƬ��B��λ
    response[3] = 0x00; // ԇƬ��B��λ (����)
    response[4] = (uint8_t)(battery_mv >> 8);
    response[5] = (uint8_t)(battery_mv & 0xFF);
    response[6] = (uint8_t)(temperature >> 8);
    response[7] = (uint8_t)(temperature & 0xFF);
    
    UART_SendPacket(CMD_STATUS_ACK, 8, response);
}

/*********************************************************************
 * @fn      Handle_Set_Parameters
 *
 * @brief   Handle the set parameters command (0x03).
 *
 * @return  none
 */
void Handle_Set_Parameters(uint8_t *data, uint8_t length)
{
    if(length == 3) {
        // �O��CODE��EVENT
        g_testCode = data[0];
        g_eventType = (data[1] << 8) | data[2];
        
        // �ؑ������O���_�J
        uint8_t response[1] = {ERR_NONE};
        UART_SendPacket(CMD_PARAMS_ACK, 1, response);
    } else {
        // �����L���e�`
        Send_Error_Response(CMD_SET_PARAMS, ERR_DATA_FORMAT);
    }
}

/*********************************************************************
 * @fn      Handle_Check_Blood
 *
 * @brief   Handle the check blood command (0x04).
 *
 * @return  none
 */
void Handle_Check_Blood(void)
{
    // ģ�M�z�y��ѪҺ
    uint8_t response[1] = {g_countdownSeconds};
    UART_SendPacket(CMD_BLOOD_NOTIFY, 1, response);
}

/*********************************************************************
 * @fn      Handle_Request_Result
 *
 * @brief   Handle the request result command (0x05).
 *
 * @return  none
 */
void Handle_Request_Result(void)
{
    uint8_t response[25];
    uint16_t index = 0;
    
    // �Y����B
    response[index++] = 0x00; // ��B��λ
    response[index++] = 0x00; // ��B��λ (����)
    
    // �y��ֵ
    response[index++] = (uint8_t)(g_testValue >> 8);
    response[index++] = (uint8_t)(g_testValue & 0xFF);
    
    // �y���Ŀ
    response[index++] = (uint8_t)(g_testType >> 8);
    response[index++] = (uint8_t)(g_testType & 0xFF);
    
    // EVENT
    response[index++] = (uint8_t)(g_eventType >> 8);
    response[index++] = (uint8_t)(g_eventType & 0xFF);
    
    // CODE
    response[index++] = g_testCode;
    
    // ��
    response[index++] = (uint8_t)(g_systemTime.year >> 8);
    response[index++] = (uint8_t)(g_systemTime.year & 0xFF);
    
    // ��
    response[index++] = 0x00;
    response[index++] = g_systemTime.month;
    
    // ��
    response[index++] = 0x00;
    response[index++] = g_systemTime.day;
    
    // �r
    response[index++] = 0x00;
    response[index++] = g_systemTime.hour;
    
    // ��
    response[index++] = 0x00;
    response[index++] = g_systemTime.minute;
    
    // ��
    response[index++] = 0x00;
    response[index++] = g_systemTime.second;
    
    // 늳�늉� (2.817V = 2817mV)
    response[index++] = 0x0B;
    response[index++] = 0x01;
    
    // �ض� (28.1��C = 281)
    response[index++] = 0x01;
    response[index++] = 0x19;
    
    UART_SendPacket(CMD_RESULT_ACK, index, response);
}

/*********************************************************************
 * @fn      Handle_Request_Raw_Data
 *
 * @brief   Handle the request raw data command (0x06).
 *
 * @return  none
 */
void Handle_Request_Raw_Data(void)
{
    // ģ�MRAW DATA (�@�eֻ��һ��ʾ�������H��ԓ�ǜy���^�̵�ԭʼ����)
    uint8_t raw_data[50] = {0};
    uint16_t data_length = 50;
    
    // �������L����ӵ�ǰ�ɂ��ֹ�
    raw_data[0] = (uint8_t)(data_length >> 8);
    raw_data[1] = (uint8_t)(data_length & 0xFF);
    
    // ���һЩʾ������
    for(uint8_t i = 2; i < data_length; i++) {
        raw_data[i] = i;
    }
    
    UART_SendPacket(CMD_RAW_ACK, data_length, raw_data);
}

/*********************************************************************
 * @fn      Parse_Packet
 *
 * @brief   Parse the received UART packet.
 *
 * @return  0 if successful, otherwise error code
 */
uint8_t Parse_Packet(void)
{
    static enum {
        STATE_WAIT_START,
        STATE_GET_CMD,
        STATE_GET_LEN,
        STATE_GET_DATA,
        STATE_GET_CHECKSUM,
        STATE_GET_END
    } parseState = STATE_WAIT_START;
    
    static uint8_t dataIndex = 0;
    static uint8_t calculatedChecksum = 0;
    
    uint8_t error = 0;
    
    while(ring_buffer.RemainCount > 0 && error == 0) {
        uint8_t data = ring_buffer_pop();
        
        switch(parseState) {
            case STATE_WAIT_START:
                if(data == PKT_START_MARK) {
                    parseState = STATE_GET_CMD;
                }
                break;
                
            case STATE_GET_CMD:
                g_rxPacket.cmd = data;
                parseState = STATE_GET_LEN;
                break;
                
            case STATE_GET_LEN:
                g_rxPacket.length = data;
                dataIndex = 0;
                calculatedChecksum = g_rxPacket.cmd + g_rxPacket.length;
                if(g_rxPacket.length > 0) {
                    parseState = STATE_GET_DATA;
                } else {
                    parseState = STATE_GET_CHECKSUM;
                }
                break;
                
            case STATE_GET_DATA:
                if(dataIndex < g_rxPacket.length) {
                    g_rxPacket.data[dataIndex] = data;
                    calculatedChecksum += data;
                    dataIndex++;
                    if(dataIndex >= g_rxPacket.length) {
                        parseState = STATE_GET_CHECKSUM;
                    }
                }
                break;
                
            case STATE_GET_CHECKSUM:
                g_rxPacket.checksum = data;
                if(calculatedChecksum != g_rxPacket.checksum) {
                    error = ERR_CHECKSUM;
                }
                parseState = STATE_GET_END;
                break;
                
            case STATE_GET_END:
                if(data == PKT_END_MARK) {
                    // ��������ɹ�������0
                    parseState = STATE_WAIT_START;
                    return 0;
                } else {
                    error = ERR_DATA_FORMAT;
                }
                parseState = STATE_WAIT_START;
                break;
                
            default:
                parseState = STATE_WAIT_START;
                break;
        }
    }
    
    return error;
}

/*********************************************************************
 * @fn      Process_Packet
 *
 * @brief   Process the received packet.
 *
 * @return  none
 */
void Process_Packet(void)
{
    switch(g_rxPacket.cmd) {
        case CMD_SYNC_TIME:
            Handle_Sync_Time(g_rxPacket.data, g_rxPacket.length);
            break;
            
        case CMD_REQ_STATUS:
            Handle_Request_Status();
            break;
            
        case CMD_SET_PARAMS:
            Handle_Set_Parameters(g_rxPacket.data, g_rxPacket.length);
            break;
            
        case CMD_CHK_BLOOD:
            Handle_Check_Blood();
            break;
            
        case CMD_REQ_RESULT:
            Handle_Request_Result();
            break;
            
        case CMD_REQ_RAW:
            Handle_Request_Raw_Data();
            break;
            
        default:
            // ��֧Ԯ������
            Send_Error_Response(g_rxPacket.cmd, ERR_CMD_UNSUPPORT);
            break;
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Delay_Init();
    USART_Printf_Init(115200);
    USARTx_CFG(115200);
    DMA_INIT();
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("UART Communication Protocol Initialization\r\n");
    
    /* ��ʼ�������� */
    PARAM_Init();
    printf("Parameter Table Initialized\r\n");
    
    /* �@ʾһЩ����ϵ�y�YӍ */
    printf("Model No: %d\r\n", PARAM_GetByte(PARAM_MODEL_NO));
    printf("FW Version: %d.%d\r\n", PARAM_GetByte(PARAM_FW_NO)/10, PARAM_GetByte(PARAM_FW_NO)%10);
    printf("Parameter Table Version: %d\r\n", PARAM_GetWord(PARAM_CODE_TABLE_V));
    printf("Test Count: %d\r\n", PARAM_GetWord(PARAM_NOT));
    
    /* �@ʾ��ǰ�O����ԇƬ��� */
    StripType_TypeDef stripType = (StripType_TypeDef)PARAM_GetByte(PARAM_STRIP_TYPE);
    printf("Strip Type: %s\r\n", StripType_GetName(stripType));
    
    /* �@ʾ��ǰ�O���Ć�λ */
    Unit_TypeDef unit = (Unit_TypeDef)PARAM_GetByte(PARAM_MGDL);
    printf("Unit: %s\r\n", Unit_GetSymbol(unit));

    while (1)
    {
        // �z���Ƿ����µķ����Ҫ̎��
        if (ring_buffer.RemainCount > 0)
        {
            // �Lԇ�������
            uint8_t error = Parse_Packet();
            
            if (error == 0)
            {
                // ��������ɹ���̎����
                Process_Packet();
            }
            else if (error != 0 && g_rxPacket.cmd != 0)
            {
                // ��������e�`���l���e�`�ؑ�
                Send_Error_Response(g_rxPacket.cmd, error);
            }
        }
        
        Delay_Ms(10); // �̕��ӕr�Թ�ʡCPU�YԴ
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // IDLE
        uint16_t rxlen     = (RX_BUFFER_LEN - USART_RX_CH->CNTR);
        uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;

        USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

        DMA_Cmd(USART_RX_CH, DISABLE);
        DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);
        // Switch buffer
        USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
        DMA_Cmd(USART_RX_CH, ENABLE);

        USART_ReceiveData(USART2); // clear IDLE flag
        ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);
    }
}

/*********************************************************************
 * @fn      DMA1_Channel6_IRQHandler
 *
 * @brief   This function handles DMA1 Channel 6 global interrupt request.
 *
 * @return  none
 */
void DMA1_Channel6_IRQHandler(void)
{
    uint16_t rxlen     = RX_BUFFER_LEN;
    uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;
    // FULL

    USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

    DMA_Cmd(USART_RX_CH, DISABLE);
    DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);
    // Switch buffer
    USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
    DMA_Cmd(USART_RX_CH, ENABLE);

    ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);

    DMA_ClearITPendingBit(DMA1_IT_TC6);
}
