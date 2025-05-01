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
#include "app_uart.h"
#include "protocol_parser.h"
#include <stdlib.h>

/*********************************************************************
 * MACROS
 */
//The buffer length should be a power of 2
#define APP_UART_TX_BUFFER_LENGTH    512U
#define APP_UART_RX_BUFFER_LENGTH    2048U

/*********************************************************************
 * CONSTANTS
 */

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

// ��ǰ�O���B
static DeviceStatusData_t deviceStatus = {
    .testItem = TEST_ITEM_GLU,  // �A�O�yԇ�Ŀ��Ѫ��
    .stripStatus = 0,           // ԇ����B����
    .batteryLevel = 3300,       // 늳�늉� 3.3V
    .temperature = 250          // �ض� 25.0 ��
};

// ���殔ǰ��CODE��EVENT
static uint8_t current_code = 0;
static uint16_t current_event = EVENT_NONE;

// �f�h������
static ProtocolParser_t bleProtocolParser;
static ProtocolParser_t uartProtocolParser;

// ���n�^��춘����ؑ����
static uint8_t responseBuffer[256];

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

// ̎���BLE���յ��Ĕ���
static void process_ble_command(uint8_t *data, uint16_t length);

// ̎���UART���յ��Ĕ���
static void process_uart_command(uint8_t *data, uint16_t length);

// ̎��f�h����
static void handle_protocol_command(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t fromBle);

// RTC���P׃��
static uint16_t rtc_year = 2023;
static uint8_t rtc_month = 1;
static uint8_t rtc_day = 1;
static uint8_t rtc_hour = 0;
static uint8_t rtc_minute = 0;
static uint8_t rtc_second = 0;

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
    SYS_DisableAllIrq(&irq_status);
    if(uart_rx_flag)
    {
        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
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
    
    // ��ʼ���f�h������
    protocol_parser_init(&bleProtocolParser);
    protocol_parser_init(&uartProtocolParser);
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
    static uint8_t uart_temp_buffer[64]; // ����UART���յ��Y��
    static uint16_t uart_temp_len = 0;
    
    uint16_t error;
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT:
            UART3_GetLinSTA();
            break;

        case UART_II_RECV_RDY:
        case UART_II_RECV_TOUT:
            // ���殔ǰ�����L��
            uart_temp_len = R8_UART3_RFC;
            
            // ֱ�ӏ�UART�xȡ�Y�ϵ��R�r���n�^
            for(uint8_t i = 0; i < uart_temp_len; i++) {
                if (i < sizeof(uart_temp_buffer)) {
                    uart_temp_buffer[i] = R8_UART3_RBR;
                } else {
                    // ���n�^���
                    for_uart_rx_black_hole = R8_UART3_RBR;
                }
            }
            
            // ̎����յ����Y��
            if (uart_temp_len > 0) {
                process_uart_command(uart_temp_buffer, uart_temp_len);
            }
            
            // �D�浽ϵ�yFIFO
            error = app_drv_fifo_write(&app_uart_rx_fifo, uart_temp_buffer, &uart_temp_len);
            if(error != APP_DRV_FIFO_RESULT_SUCCESS) {
                // FIFO�M���o������
            }
            
            uart_rx_flag = true;
            break;

        case UART_II_THR_EMPTY:
            break;
        case UART_II_MODEM_CHG:
            break;
        default:
            {
                // ���Д�̎������̎��֧Ԯ���Д����
                // �����κ�̎��ֱ������
            }
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
            
            // ̎���{�����յ����Y��
            process_ble_command((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      process_ble_command
 *
 * @brief   ̎���{�����յ�������
 *
 * @param   data - �Y��ָ�
 *          length - �Y���L��
 *
 * @return  NULL
 */
static void process_ble_command(uint8_t *data, uint16_t length)
{
    // ���ֹ��������յ����{���Y��
    for (uint16_t i = 0; i < length; i++) {
        int result = protocol_parser_parse(&bleProtocolParser, data[i]);
        
        // ����յ�һ�������ķ��
        if (result == 1) {
            uint8_t cmdId = bleProtocolParser.cmdId;
            uint8_t dataLength = bleProtocolParser.dataLength;
            
            // �@ȡ�Y�ϲ���ָ�
            uint8_t *cmdData = NULL;
            if (dataLength > 0) {
                cmdData = &bleProtocolParser.rxBuffer[3]; // 3 = ��ʼ��ӛ(1) + ����ID(1) + �L��(1)
            }
            
            // ̎������
            handle_protocol_command(cmdId, cmdData, dataLength, 1);
            
            // ���Åf�h���������ʂ������һ�����
            protocol_parser_reset(&bleProtocolParser);
        }
        // �����e�`
        else if (result == -1) {
            // ̎������e�`�����Է����e�`ӍϢ
            ErrorResponseData_t errData;
            errData.originalCmd = 0; // δ֪����
            errData.errorCode = ERR_DATA_FORMAT_ERROR;
            
            int respLen = protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errData, 
                                              sizeof(ErrorResponseData_t), responseBuffer, sizeof(responseBuffer));
            
            if (respLen > 0) {
                // �l���e�`�ؑ����{��
                uint16_t to_write_length = respLen;
                app_drv_fifo_write(&app_uart_rx_fifo, responseBuffer, &to_write_length);
                tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
            }
        }
    }
}

/*********************************************************************
 * @fn      process_uart_command
 *
 * @brief   ̎��UART���յ�������
 *
 * @param   data - �Y��ָ�
 *          length - �Y���L��
 *
 * @return  NULL
 */
static void process_uart_command(uint8_t *data, uint16_t length)
{
    // ���ֹ��������յ���UART�Y��
    for (uint16_t i = 0; i < length; i++) {
        int result = protocol_parser_parse(&uartProtocolParser, data[i]);
        
        // ����յ�һ�������ķ��
        if (result == 1) {
            uint8_t cmdId = uartProtocolParser.cmdId;
            uint8_t dataLength = uartProtocolParser.dataLength;
            
            // �@ȡ�Y�ϲ���ָ�
            uint8_t *cmdData = NULL;
            if (dataLength > 0) {
                cmdData = &uartProtocolParser.rxBuffer[3]; // 3 = ��ʼ��ӛ(1) + ����ID(1) + �L��(1)
            }
            
            // ̎������
            handle_protocol_command(cmdId, cmdData, dataLength, 0);
            
            // ���Åf�h���������ʂ������һ�����
            protocol_parser_reset(&uartProtocolParser);
        }
        // �����e�`
        else if (result == -1) {
            // ̎������e�`��������CH32V�l���e�`ӍϢ
            ErrorResponseData_t errData;
            errData.originalCmd = 0; // δ֪����
            errData.errorCode = ERR_DATA_FORMAT_ERROR;
            
            int respLen = protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errData, 
                                              sizeof(ErrorResponseData_t), responseBuffer, sizeof(responseBuffer));
            
            if (respLen > 0) {
                // �l���e�`�ؑ���UART
                app_uart_tx_data(responseBuffer, respLen);
            }
        }
    }
}

/*********************************************************************
 * @fn      handle_protocol_command
 *
 * @brief   ̎��f�h����
 *
 * @param   cmdId - ����ID
 *          data - �Y��ָ�
 *          length - �Y���L��
 *          fromBle - 1:�����{��, 0:����UART
 *
 * @return  NULL
 */
static void handle_protocol_command(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t fromBle)
{
    int respLen = 0;
    
    switch(cmdId)
    {
        case CMD_SYNC_TIME:
            {
                if (length == sizeof(SyncTimeData_t)) {
                    SyncTimeData_t *timeData = (SyncTimeData_t *)data;
                    // ����ͬ���r�g
                    rtc_year = timeData->year;
                    rtc_month = timeData->month;
                    rtc_day = timeData->day;
                    rtc_hour = timeData->hour;
                    rtc_minute = timeData->minute;
                    rtc_second = timeData->second;
                    
                    // �����_�J�ؑ�
                    uint8_t status = 0x00; // �ɹ�
                    respLen = protocol_build_packet(CMD_SYNC_TIME_CONFIRM, &status, 1, responseBuffer, sizeof(responseBuffer));
                } else {
                    // �Y���L���e�`
                    ErrorResponseData_t errData;
                    errData.originalCmd = cmdId;
                    errData.errorCode = ERR_DATA_FORMAT_ERROR;
                    respLen = protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errData, sizeof(ErrorResponseData_t), 
                                                 responseBuffer, sizeof(responseBuffer));
                }
            }
            break;
            
        case CMD_REQUEST_DEVICE_STATUS:
            {
                // �����O���B
                deviceStatus.batteryLevel = 3300; // ����: 3.3V
                deviceStatus.temperature = 250;   // ����: 25.0��C
                
                // �����O���B�ؑ�
                respLen = protocol_build_packet(CMD_DEVICE_STATUS_RESPONSE, (uint8_t *)&deviceStatus, 
                                             sizeof(DeviceStatusData_t), responseBuffer, sizeof(responseBuffer));
            }
            break;
            
        case CMD_SET_CODE_EVENT:
            {
                if (length == sizeof(CodeEventData_t)) {
                    CodeEventData_t *codeEventData = (CodeEventData_t *)data;
                    // ����CODE��EVENT
                    current_code = codeEventData->code;
                    current_event = codeEventData->event;
                    
                    // �����_�J�ؑ�
                    uint8_t status = 0x00; // �ɹ�
                    respLen = protocol_build_packet(CMD_CODE_EVENT_CONFIRM, &status, 1, responseBuffer, sizeof(responseBuffer));
                } else {
                    // �Y���L���e�`
                    ErrorResponseData_t errData;
                    errData.originalCmd = cmdId;
                    errData.errorCode = ERR_DATA_FORMAT_ERROR;
                    respLen = protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errData, sizeof(ErrorResponseData_t), 
                                                 responseBuffer, sizeof(responseBuffer));
                }
            }
            break;
            
        case CMD_BLOOD_STATUS_REQUEST:
            {
                // ����ѪҺ��B֪ͨ�����O����5��
                BloodStatusData_t bloodStatus;
                bloodStatus.countdown = 5;
                respLen = protocol_build_packet(CMD_BLOOD_STATUS_NOTIFY, (uint8_t *)&bloodStatus, 
                                             sizeof(BloodStatusData_t), responseBuffer, sizeof(responseBuffer));
            }
            break;
            
        case CMD_REQUEST_RESULT:
            {
                // �����yԇ�Y��
                TestResultData_t testResult;
                testResult.resultStatus = 0x0000; // �ɹ�
                testResult.testValue = 123;       // �yԇֵ����
                testResult.testItem = deviceStatus.testItem;
                testResult.event = current_event;
                testResult.code = current_code;
                testResult.year = rtc_year;
                testResult.month = rtc_month;
                testResult.day = rtc_day;
                testResult.hour = rtc_hour;
                testResult.minute = rtc_minute;
                testResult.second = rtc_second;
                testResult.batteryLevel = deviceStatus.batteryLevel;
                testResult.temperature = deviceStatus.temperature;
                
                respLen = protocol_build_packet(CMD_RESULT_RESPONSE, (uint8_t *)&testResult, 
                                             sizeof(TestResultData_t), responseBuffer, sizeof(responseBuffer));
            }
            break;
            
        case CMD_REQUEST_RAW_DATA:
            {
                // ����RAW DATA����
                uint8_t rawDataBuffer[50];
                RawDataHeader_t *rawHeader = (RawDataHeader_t *)rawDataBuffer;
                rawHeader->dataLength = 40; // ���ORAW DATA�L�Ȟ�40�ֹ�
                
                // ���һЩʾ������
                for (int i = 0; i < 40; i++) {
                    rawDataBuffer[sizeof(RawDataHeader_t) + i] = i & 0xFF;
                }
                
                respLen = protocol_build_packet(CMD_RAW_DATA_RESPONSE, rawDataBuffer, 
                                             sizeof(RawDataHeader_t) + 40, responseBuffer, sizeof(responseBuffer));
            }
            break;
            
        // ̎��������APP��CH32V����ąf�h����
        default:
            {
                // ��֧Ԯ������
                ErrorResponseData_t errData;
                errData.originalCmd = cmdId;
                errData.errorCode = ERR_COMMAND_NOT_SUPPORTED;
                respLen = protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errData, sizeof(ErrorResponseData_t), 
                                             responseBuffer, sizeof(responseBuffer));
            }
            break;
    }
    
    // �l�ͻؑ�
    if (respLen > 0) {
        if (fromBle) {
            // ���{���յ���������ؑ�ͨ�^UART�l��
            app_uart_tx_data(responseBuffer, respLen);
        } else {
            // ��UART�յ���������ؑ�ͨ�^�{���l��
            uint16_t to_write_length = respLen;
            app_drv_fifo_write(&app_uart_rx_fifo, responseBuffer, &to_write_length);
            tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
        }
    }
}

/*********************************************************************
*********************************************************************/
