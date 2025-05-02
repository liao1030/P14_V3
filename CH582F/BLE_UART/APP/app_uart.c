/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : zhangxiyi @WCH
 * Version            : v0.1
 * Date               : 2020/11/26
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"
#include "app_uart.h"

/*********************************************************************
 * MACROS
 */
//The buffer length should be a power of 2
#define APP_UART_TX_BUFFER_LENGTH    512U
#define APP_UART_RX_BUFFER_LENGTH    2048U

// �����ⲿ׃��
extern uint8_t Peripheral_TaskID;

/*********************************************************************
 * ͨӍ�f�h�������x
 */
#define PKT_START_MARK       0xAA    // ��ʼ��ӛ
#define PKT_END_MARK         0x55    // �Y����ӛ

// ָ��ID���x
#define CMD_SYNC_TIME        0x01    // ͬ���r�g
#define CMD_SYNC_TIME_ACK    0x81    // �r�gͬ���_�J
#define CMD_REQ_STATUS       0x02    // Ո���b�à�B
#define CMD_STATUS_ACK       0x82    // �b�à�B�ظ�
#define CMD_SET_CODE_EVENT   0x03    // �O��CODE��EVENT
#define CMD_CODE_EVENT_ACK   0x83    // CODE/EVENT�O���_�J
#define CMD_BLOOD_STATUS     0x04    // �z�y��BՈ��
#define CMD_BLOOD_NOTIFY     0x84    // ѪҺ�z�y֪ͨ
#define CMD_REQ_RESULT       0x05    // Ո��y���Y��
#define CMD_RESULT_ACK       0x85    // �y���Y���ظ�
#define CMD_REQ_RAW_DATA     0x06    // Ո��RAW DATA
#define CMD_RAW_DATA_ACK     0x86    // RAW DATA�ظ�
#define CMD_ERROR_ACK        0xFF    // �e�`�ظ�

// �e�`���a
#define ERR_BATTERY_LOW      0x01    // 늳�����^��
#define ERR_TEMP_HIGH        0x02    // �ض��^��
#define ERR_TEMP_LOW         0x03    // �ض��^��
#define ERR_STRIP_EXPIRED    0x04    // ԇƬ�^�ڻ�p��
#define ERR_STRIP_USED       0x05    // ԇƬ��ʹ��
#define ERR_STRIP_ERROR      0x06    // ԇƬ�����e�`
#define ERR_BLOOD_NOTENOUGH  0x07    // ѪҺ�ӱ�����
#define ERR_MEASURE_TIMEOUT  0x08    // �y�����r
#define ERR_CALIBRATION      0x09    // У���e�`
#define ERR_HARDWARE         0x0A    // Ӳ�w�e�`
#define ERR_COMM_ERROR       0x0B    // ͨӍ�e�`
#define ERR_DATA_FORMAT      0x0C    // �Y�ϸ�ʽ�e�`
#define ERR_CHECKSUM         0x0D    // У���e�`
#define ERR_CMD_NOTSUPPORT   0x0E    // ָ�֧��
#define ERR_RESULT_OUTRANGE  0x0F    // �y���Y����������

// �؂����P����
#define RETRANSMIT_TIMEOUT   500     // �؂����r�r�g (ms)
#define MAX_RETRANSMIT_COUNT 3       // ����؂��Δ�

// ����Y�����x
typedef struct {
    uint8_t startMark;           // ��ʼ��ӛ (0xAA)
    uint8_t cmdId;               // ָ��ID
    uint8_t dataLen;             // �Y���L��
    uint8_t data[255];           // �Y�ϙ�λ (��׃�L��)
    uint8_t checksum;            // У��
    uint8_t endMark;             // �Y����ӛ (0x55)
} __attribute__((packed)) protocol_packet_t;

// �f�h��B����
typedef struct {
    uint8_t curCmdId;                // ��ǰָ��ID
    uint8_t retransmitCount;         // �؂��Δ�
    uint32_t lastTransmitTime;       // ������͕r�g
    bool waitForResponse;            // �ȴ��ؑ����I
    uint8_t stripCode;               // ��ǰԇƬCODEֵ
    uint16_t eventType;              // ��ǰ�¼����
    bool measureInProgress;          // �y���M���И��I
} protocol_state_t;

/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * ��������
 */
// �@ȡϵ�y�r�g�ĺ���
static uint32_t HAL_GetTick(void);
// Ӌ��У��
static uint8_t calculate_checksum(uint8_t *data, uint16_t len);
// ���b�f�h���
static uint16_t pack_protocol_packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// �l�ͷ����MCU
static void send_packet_to_mcu(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// �l���e�`�ظ������APP
static void send_error_packet_to_app(uint8_t originalCmdId, uint8_t errorCode);
// �l�ͻظ������APP
static void send_response_to_app(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// ̎���MCU���յ��ąf�h���
static bool process_packet_from_mcu(uint8_t *data, uint16_t len);
// ̎���APP���յ��ąf�h���
static bool process_packet_from_app(uint8_t *data, uint16_t len);
// ̎��f�h�؂�
static void handle_retransmission(void);

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t to_test_buffer[BLE_BUFF_MAX_LEN - 4 - 3];

app_drv_fifo_t app_uart_tx_fifo;
app_drv_fifo_t app_uart_rx_fifo;

//interupt uart rx flag ,clear at main loop
bool uart_rx_flag = false;

//for interrupt rx blcak hole ,when uart rx fifo full
uint8_t for_uart_rx_black_hole = 0;

//fifo length less that MTU-3, retry times
uint32_t uart_to_ble_send_evt_cnt = 0;

// �f�h̎�����P׃��
static protocol_state_t protocolState = {0};
static protocol_packet_t rxPacket = {0};
static protocol_packet_t txPacket = {0};
static uint8_t packetBuffer[300] = {0};  // ��춷���M�b�ľ��n�^

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
//

//The tx buffer and rx buffer for app_drv_fifo
//length should be a power of 2
static uint8_t app_uart_tx_buffer[APP_UART_TX_BUFFER_LENGTH] = {0};
static uint8_t app_uart_rx_buffer[APP_UART_RX_BUFFER_LENGTH] = {0};


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PROFILE CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      app_uart_process
 *
 * @brief   process uart data
 *
 * @return  NULL
 */
void app_uart_process(void)
{
    UINT32 irq_status;
    static uint8_t uartRxBuffer[300];
    static uint16_t uartRxLen = 0;
    
    // ̎���؂�
    handle_retransmission();
    
    SYS_DisableAllIrq(&irq_status);
    if(uart_rx_flag)
    {
        // �xȡUART����
        uint16_t read_len = 0;
        app_drv_fifo_read(&app_uart_rx_fifo, &uartRxBuffer[uartRxLen], &read_len);
        uartRxLen += read_len;
        
        // �����f�h���
        if (uartRxLen >= 5) { // ��С����L�Ȟ�5 (��ʼ��ӛ+ָ��ID+�L��+У��+�Y����ӛ)
            // ���ҷ����ʼ��ӛ
            for (uint16_t i = 0; i < uartRxLen; i++) {
                if (uartRxBuffer[i] == PKT_START_MARK) {
                    if (i > 0) {
                        // �G��ǰ��ğoЧ����
                        memmove(uartRxBuffer, &uartRxBuffer[i], uartRxLen - i);
                        uartRxLen -= i;
                    }
                    break;
                }
            }
            
            // ��������L�����̎��һ���������
            if (uartRxLen >= 5) {
                uint8_t dataLen = uartRxBuffer[2];
                uint16_t packetLen = dataLen + 5; // ���L�� = ��ʼ��ӛ(1) + ָ��ID(1) + �L��(1) + ����(dataLen) + У��(1) + �Y����ӛ(1)
                
                // �z���Ƿ����������
                if (uartRxLen >= packetLen && uartRxBuffer[packetLen-1] == PKT_END_MARK) {
                    // ̎����
                    process_packet_from_mcu(uartRxBuffer, packetLen);
                    
                    // �Ƴ���̎��ķ��
                    if (uartRxLen > packetLen) {
                        memmove(uartRxBuffer, &uartRxBuffer[packetLen], uartRxLen - packetLen);
                    }
                    uartRxLen -= packetLen;
                }
            }
        }
        
        uart_rx_flag = false;
    }
    SYS_RecoverIrq(irq_status);

    //tx process
    if(R8_UART3_TFC < UART_FIFO_SIZE)
    {
        app_drv_fifo_read_to_same_addr(&app_uart_tx_fifo, (uint8_t *)&R8_UART3_THR, UART_FIFO_SIZE - R8_UART3_TFC);
    }
}

/*********************************************************************
 * @fn      app_uart_init
 *
 * @brief   init uart
 *
 * @return  NULL
 */
void app_uart_init()
{
    //tx fifo and tx fifo
    //The buffer length should be a power of 2
    app_drv_fifo_init(&app_uart_tx_fifo, app_uart_tx_buffer, APP_UART_TX_BUFFER_LENGTH);
    app_drv_fifo_init(&app_uart_rx_fifo, app_uart_rx_buffer, APP_UART_RX_BUFFER_LENGTH);

    //uart tx io
    GPIOA_SetBits(bTXD3);
    GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);

    //uart rx io
    GPIOA_SetBits(bRXD3);
    GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);

    //uart3 init
    UART3_DefInit();

    //enable interupt
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART3_IRQn);
}

/*********************************************************************
 * @fn      app_uart_tx_data
 *
 * @brief   app_uart_tx_data
 *
 * @return  NULL
 */
void app_uart_tx_data(uint8_t *data, uint16_t length)
{
    uint16_t write_length = length;
    app_drv_fifo_write(&app_uart_tx_fifo, data, &write_length);
}

/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   Not every uart reception will end with a UART_II_RECV_TOUT
 *          UART_II_RECV_TOUT can only be triggered when R8_UARTx_RFC is not 0
 *          Here we cannot rely UART_II_RECV_TOUT as the end of a uart reception
 *
 * @return  NULL
 */
__INTERRUPT
__HIGH_CODE
void UART3_IRQHandler(void)
{
    uint16_t error;
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT:
            UART3_GetLinSTA();
            break;

        case UART_II_RECV_RDY:
        case UART_II_RECV_TOUT:
            error = app_drv_fifo_write_from_same_addr(&app_uart_rx_fifo, (uint8_t *)&R8_UART3_RBR, R8_UART3_RFC);
            if(error != APP_DRV_FIFO_RESULT_SUCCESS)
            {
                for(uint8_t i = 0; i < R8_UART3_RFC; i++)
                {
                    //fifo full,put to fifo black hole
                    for_uart_rx_black_hole = R8_UART3_RBR;
                }
            }
            uart_rx_flag = true;
            break;

        case UART_II_THR_EMPTY:
            break;
        case UART_II_MODEM_CHG:
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      on_bleuartServiceEvt
 *
 * @brief   ble uart service callback handler
 *
 * @return  NULL
 */
void on_bleuartServiceEvt(uint16_t connection_handle, ble_uart_evt_t *p_evt)
{
    switch(p_evt->type)
    {
        case BLE_UART_EVT_TX_NOTI_DISABLED:
            PRINT("%02x:bleuart_EVT_TX_NOTI_DISABLED\r\n", connection_handle);
            break;
            
        case BLE_UART_EVT_TX_NOTI_ENABLED:
            PRINT("%02x:bleuart_EVT_TX_NOTI_ENABLED\r\n", connection_handle);
            break;
            
        case BLE_UART_EVT_BLE_DATA_RECIEVED:
            PRINT("BLE RX DATA len:%d\r\n", p_evt->data.length);

            // ̎��APP�l�͵��{������
            if (p_evt->data.p_data[0] == PKT_START_MARK && 
                p_evt->data.length >= 5 && 
                p_evt->data.p_data[p_evt->data.length-1] == PKT_END_MARK) {
                // ̎��APP�l�͵ąf�h���
                process_packet_from_app((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            } else {
                // �oЧ�ķ����ʽ���l���e�`�ظ�
                send_error_packet_to_app(0, ERR_DATA_FORMAT);
            }
            break;
            
        default:
            break;
    }
}

/*********************************************************************
 * �f�h̎�����P����
 */

/**
 * @brief �@ȡϵ�y�r�g (���HAL_GetTick����)
 * @return ��ǰϵ�y�r�g(����)
 */
static uint32_t HAL_GetTick(void)
{
    // CH582F�]��HAL_GetTick������ʹ��TMOS�r�g���
    return TMOS_GetSystemClock();
}

/**
 * @brief Ӌ��У��
 * @param data ����ָ�
 * @param len �����L��
 * @return У�ͽY��
 */
static uint8_t calculate_checksum(uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}

/**
 * @brief ���b�f�h���
 * @param cmdId ָ��ID
 * @param data ����ָ�
 * @param dataLen �����L��
 * @return ������L��
 */
static uint16_t pack_protocol_packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint16_t packetLen = 0;
    
    // ��ʼ��ӛ
    packetBuffer[packetLen++] = PKT_START_MARK;
    
    // ָ��ID
    packetBuffer[packetLen++] = cmdId;
    
    // �����L��
    packetBuffer[packetLen++] = dataLen;
    
    // �}�u����
    if (dataLen > 0 && data != NULL) {
        tmos_memcpy(&packetBuffer[packetLen], data, dataLen);
        packetLen += dataLen;
    }
    
    // Ӌ��У��
    packetBuffer[packetLen] = calculate_checksum(&packetBuffer[1], packetLen - 1);
    packetLen++;
    
    // �Y����ӛ
    packetBuffer[packetLen++] = PKT_END_MARK;
    
    return packetLen;
}

/**
 * @brief �l�ͷ����MCU
 * @param cmdId ָ��ID
 * @param data ����ָ�
 * @param dataLen �����L��
 */
static void send_packet_to_mcu(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // ���b���
    uint16_t packetLen = pack_protocol_packet(cmdId, data, dataLen);
    
    // �l�͵�UART
    app_uart_tx_data(packetBuffer, packetLen);
    
    // ���f�h��B
    protocolState.curCmdId = cmdId;
    protocolState.lastTransmitTime = HAL_GetTick();
    protocolState.waitForResponse = true;
    
    PRINT("Send packet to MCU: CMD=0x%02X, Len=%d\n", cmdId, dataLen);
}

/**
 * @brief �l���e�`�ظ������APP
 * @param originalCmdId ԭʼָ��ID
 * @param errorCode �e�`���a
 */
static void send_error_packet_to_app(uint8_t originalCmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    // ���b�e�`�ظ����
    uint16_t packetLen = pack_protocol_packet(CMD_ERROR_ACK, data, 2);
    
    // �l�͵�APP
    attHandleValueNoti_t noti;
    noti.len = packetLen;
    noti.pValue = packetBuffer;
    
    // ʹ��tmos_start_task������ֱ���{��notify����
    tmos_memcpy(to_test_buffer, packetBuffer, packetLen);
    tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
    
    PRINT("Send error packet to APP: Orig CMD=0x%02X, Error=0x%02X\n", originalCmdId, errorCode);
}

/**
 * @brief �l�ͻظ������APP
 * @param cmdId ָ��ID
 * @param data ����ָ�
 * @param dataLen �����L��
 */
static void send_response_to_app(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // ���b�ظ����
    uint16_t packetLen = pack_protocol_packet(cmdId, data, dataLen);
    
    // �}�u��ȫ�־��n�^���Ա�ͨ�^�΄հl��
    tmos_memcpy(to_test_buffer, packetBuffer, packetLen);
    
    // ʹ��tmos_start_task���Ӱl���¼�������ֱ���{��notify����
    tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
    
    PRINT("Send response to APP: CMD=0x%02X, Len=%d\n", cmdId, dataLen);
}

/**
 * @brief ̎���MCU���յ��ąf�h���
 * @param data ����ָ�
 * @param len �����L��
 */
static bool process_packet_from_mcu(uint8_t *data, uint16_t len)
{
    // �z����ʼ�ͽY����ӛ
    if (data[0] != PKT_START_MARK || data[len-1] != PKT_END_MARK) {
        PRINT("Invalid packet markers\n");
        return false;
    }
    
    // �z���L��һ����
    uint8_t dataLen = data[2];
    if (len != dataLen + 5) { // 5 = ��ʼ��ӛ(1) + ָ��ID(1) + �L��(1) + У��(1) + �Y����ӛ(1)
        PRINT("Invalid packet length\n");
        return false;
    }
    
    // �z��У��
    uint8_t calculatedChecksum = calculate_checksum(&data[1], dataLen + 2); // +2 ��ָ��ID���L�ș�λ
    if (calculatedChecksum != data[len-2]) {
        PRINT("Checksum error\n");
        return false;
    }
    
    // ��ȡָ��ID���Y��
    uint8_t cmdId = data[1];
    uint8_t* pData = (dataLen > 0) ? &data[3] : NULL;
    
    // �����؂�Ӌ����
    protocolState.retransmitCount = 0;
    protocolState.waitForResponse = false;
    
    // ����ָ��ID�D�l��APP
    send_response_to_app(cmdId, pData, dataLen);
    
    return true;
}

/**
 * @brief ̎���APP���յ��ąf�h���
 * @param data ����ָ�
 * @param len �����L��
 */
static bool process_packet_from_app(uint8_t *data, uint16_t len)
{
    // �z����ʼ�ͽY����ӛ
    if (data[0] != PKT_START_MARK || data[len-1] != PKT_END_MARK) {
        PRINT("Invalid packet markers from APP\n");
        return false;
    }
    
    // �z���L��һ����
    uint8_t dataLen = data[2];
    if (len != dataLen + 5) { // 5 = ��ʼ��ӛ(1) + ָ��ID(1) + �L��(1) + У��(1) + �Y����ӛ(1)
        PRINT("Invalid packet length from APP\n");
        return false;
    }
    
    // �z��У��
    uint8_t calculatedChecksum = calculate_checksum(&data[1], dataLen + 2); // +2 ��ָ��ID���L�ș�λ
    if (calculatedChecksum != data[len-2]) {
        PRINT("Checksum error from APP\n");
        send_error_packet_to_app(data[1], ERR_CHECKSUM);
        return false;
    }
    
    // ��ȡָ��ID���Y��
    uint8_t cmdId = data[1];
    uint8_t* pData = (dataLen > 0) ? &data[3] : NULL;
    
    // ���殔ǰ̎���ָ��ID
    protocolState.curCmdId = cmdId;
    
    // ����ָ������M������̎��
    switch (cmdId) {
        case CMD_SET_CODE_EVENT:
            if (dataLen >= 3) {
                // ����CODE��EVENTֵ
                protocolState.stripCode = pData[0];
                protocolState.eventType = BUILD_UINT16(pData[1], pData[2]);
            }
            break;
        
        case CMD_BLOOD_STATUS:
            // ��ӛ�y���_ʼ
            protocolState.measureInProgress = true;
            break;
            
        case CMD_REQ_RESULT:
            // �y���Y��Ո��̎��
            break;
            
        default:
            break;
    }
    
    // �D�l��MCU
    send_packet_to_mcu(cmdId, pData, dataLen);
    
    return true;
}

/**
 * @brief ̎��f�h�؂�
 */
static void handle_retransmission(void)
{
    // �Д��Ƿ��ڵȴ��ظ�
    if (!protocolState.waitForResponse) {
        return;
    }
    
    // �@ȡ��ǰ�r�g
    uint32_t currentTime = HAL_GetTick();
    
    // �Д��Ƿ񳬕r
    if ((currentTime - protocolState.lastTransmitTime) >= RETRANSMIT_TIMEOUT) {
        // ���r�؂�
        if (protocolState.retransmitCount < MAX_RETRANSMIT_COUNT) {
            // �ذl��һ�����
            app_uart_tx_data(packetBuffer, 0); // packetBuffer������һ�ΰl�͵ķ��
            
            // �����؂���B
            protocolState.retransmitCount++;
            protocolState.lastTransmitTime = currentTime;
            
            PRINT("Retransmit packet, count=%d\n", protocolState.retransmitCount);
        } else {
            // ���^����؂��Δ���֪ͨAPPͨӍ�e�`
            send_error_packet_to_app(protocolState.curCmdId, ERR_COMM_ERROR);
            
            // �����؂���B
            protocolState.waitForResponse = false;
            protocolState.retransmitCount = 0;
            
            PRINT("Retransmit failed, max count reached\n");
        }
    }
}

/*********************************************************************
*********************************************************************/
