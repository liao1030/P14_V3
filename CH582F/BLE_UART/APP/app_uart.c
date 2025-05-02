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

// 宣告外部變數
extern uint8_t Peripheral_TaskID;

/*********************************************************************
 * 通訊協議常量定義
 */
#define PKT_START_MARK       0xAA    // 起始標記
#define PKT_END_MARK         0x55    // 結束標記

// 指令ID定義
#define CMD_SYNC_TIME        0x01    // 同步時間
#define CMD_SYNC_TIME_ACK    0x81    // 時間同步確認
#define CMD_REQ_STATUS       0x02    // 請求裝置狀態
#define CMD_STATUS_ACK       0x82    // 裝置狀態回覆
#define CMD_SET_CODE_EVENT   0x03    // 設定CODE和EVENT
#define CMD_CODE_EVENT_ACK   0x83    // CODE/EVENT設定確認
#define CMD_BLOOD_STATUS     0x04    // 檢測狀態請求
#define CMD_BLOOD_NOTIFY     0x84    // 血液檢測通知
#define CMD_REQ_RESULT       0x05    // 請求測量結果
#define CMD_RESULT_ACK       0x85    // 測量結果回覆
#define CMD_REQ_RAW_DATA     0x06    // 請求RAW DATA
#define CMD_RAW_DATA_ACK     0x86    // RAW DATA回覆
#define CMD_ERROR_ACK        0xFF    // 錯誤回覆

// 錯誤代碼
#define ERR_BATTERY_LOW      0x01    // 電池電量過低
#define ERR_TEMP_HIGH        0x02    // 溫度過高
#define ERR_TEMP_LOW         0x03    // 溫度過低
#define ERR_STRIP_EXPIRED    0x04    // 試片過期或損壞
#define ERR_STRIP_USED       0x05    // 試片已使用
#define ERR_STRIP_ERROR      0x06    // 試片插入錯誤
#define ERR_BLOOD_NOTENOUGH  0x07    // 血液樣本不足
#define ERR_MEASURE_TIMEOUT  0x08    // 測量超時
#define ERR_CALIBRATION      0x09    // 校準錯誤
#define ERR_HARDWARE         0x0A    // 硬體錯誤
#define ERR_COMM_ERROR       0x0B    // 通訊錯誤
#define ERR_DATA_FORMAT      0x0C    // 資料格式錯誤
#define ERR_CHECKSUM         0x0D    // 校驗和錯誤
#define ERR_CMD_NOTSUPPORT   0x0E    // 指令不支持
#define ERR_RESULT_OUTRANGE  0x0F    // 測量結果超出範圍

// 重傳相關參數
#define RETRANSMIT_TIMEOUT   500     // 重傳超時時間 (ms)
#define MAX_RETRANSMIT_COUNT 3       // 最大重傳次數

// 封包結構定義
typedef struct {
    uint8_t startMark;           // 起始標記 (0xAA)
    uint8_t cmdId;               // 指令ID
    uint8_t dataLen;             // 資料長度
    uint8_t data[255];           // 資料欄位 (可變長度)
    uint8_t checksum;            // 校驗和
    uint8_t endMark;             // 結束標記 (0x55)
} __attribute__((packed)) protocol_packet_t;

// 協議狀態管理
typedef struct {
    uint8_t curCmdId;                // 當前指令ID
    uint8_t retransmitCount;         // 重傳次數
    uint32_t lastTransmitTime;       // 最後傳送時間
    bool waitForResponse;            // 等待回應標誌
    uint8_t stripCode;               // 當前試片CODE值
    uint16_t eventType;              // 當前事件類型
    bool measureInProgress;          // 測量進行中標誌
} protocol_state_t;

/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * 函數宣告
 */
// 獲取系統時間的函數
static uint32_t HAL_GetTick(void);
// 計算校驗和
static uint8_t calculate_checksum(uint8_t *data, uint16_t len);
// 封裝協議封包
static uint16_t pack_protocol_packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// 發送封包到MCU
static void send_packet_to_mcu(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// 發送錯誤回覆封包到APP
static void send_error_packet_to_app(uint8_t originalCmdId, uint8_t errorCode);
// 發送回覆封包到APP
static void send_response_to_app(uint8_t cmdId, uint8_t *data, uint8_t dataLen);
// 處理從MCU接收到的協議封包
static bool process_packet_from_mcu(uint8_t *data, uint16_t len);
// 處理從APP接收到的協議封包
static bool process_packet_from_app(uint8_t *data, uint16_t len);
// 處理協議重傳
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

// 協議處理相關變量
static protocol_state_t protocolState = {0};
static protocol_packet_t rxPacket = {0};
static protocol_packet_t txPacket = {0};
static uint8_t packetBuffer[300] = {0};  // 用於封包組裝的緩衝區

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
    
    // 處理重傳
    handle_retransmission();
    
    SYS_DisableAllIrq(&irq_status);
    if(uart_rx_flag)
    {
        // 讀取UART數據
        uint16_t read_len = 0;
        app_drv_fifo_read(&app_uart_rx_fifo, &uartRxBuffer[uartRxLen], &read_len);
        uartRxLen += read_len;
        
        // 解析協議封包
        if (uartRxLen >= 5) { // 最小封包長度為5 (起始標記+指令ID+長度+校驗和+結束標記)
            // 尋找封包起始標記
            for (uint16_t i = 0; i < uartRxLen; i++) {
                if (uartRxBuffer[i] == PKT_START_MARK) {
                    if (i > 0) {
                        // 丟棄前面的無效數據
                        memmove(uartRxBuffer, &uartRxBuffer[i], uartRxLen - i);
                        uartRxLen -= i;
                    }
                    break;
                }
            }
            
            // 如果數據長度足夠處理一個完整封包
            if (uartRxLen >= 5) {
                uint8_t dataLen = uartRxBuffer[2];
                uint16_t packetLen = dataLen + 5; // 總長度 = 起始標記(1) + 指令ID(1) + 長度(1) + 數據(dataLen) + 校驗和(1) + 結束標記(1)
                
                // 檢查是否有完整封包
                if (uartRxLen >= packetLen && uartRxBuffer[packetLen-1] == PKT_END_MARK) {
                    // 處理封包
                    process_packet_from_mcu(uartRxBuffer, packetLen);
                    
                    // 移除已處理的封包
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

            // 處理APP發送的藍牙數據
            if (p_evt->data.p_data[0] == PKT_START_MARK && 
                p_evt->data.length >= 5 && 
                p_evt->data.p_data[p_evt->data.length-1] == PKT_END_MARK) {
                // 處理APP發送的協議封包
                process_packet_from_app((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            } else {
                // 無效的封包格式，發送錯誤回覆
                send_error_packet_to_app(0, ERR_DATA_FORMAT);
            }
            break;
            
        default:
            break;
    }
}

/*********************************************************************
 * 協議處理相關函數
 */

/**
 * @brief 獲取系統時間 (替代HAL_GetTick函數)
 * @return 當前系統時間(毫秒)
 */
static uint32_t HAL_GetTick(void)
{
    // CH582F沒有HAL_GetTick函數，使用TMOS時間替代
    return TMOS_GetSystemClock();
}

/**
 * @brief 計算校驗和
 * @param data 數據指針
 * @param len 數據長度
 * @return 校驗和結果
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
 * @brief 封裝協議封包
 * @param cmdId 指令ID
 * @param data 數據指針
 * @param dataLen 數據長度
 * @return 封包總長度
 */
static uint16_t pack_protocol_packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint16_t packetLen = 0;
    
    // 起始標記
    packetBuffer[packetLen++] = PKT_START_MARK;
    
    // 指令ID
    packetBuffer[packetLen++] = cmdId;
    
    // 數據長度
    packetBuffer[packetLen++] = dataLen;
    
    // 複製數據
    if (dataLen > 0 && data != NULL) {
        tmos_memcpy(&packetBuffer[packetLen], data, dataLen);
        packetLen += dataLen;
    }
    
    // 計算校驗和
    packetBuffer[packetLen] = calculate_checksum(&packetBuffer[1], packetLen - 1);
    packetLen++;
    
    // 結束標記
    packetBuffer[packetLen++] = PKT_END_MARK;
    
    return packetLen;
}

/**
 * @brief 發送封包到MCU
 * @param cmdId 指令ID
 * @param data 數據指針
 * @param dataLen 數據長度
 */
static void send_packet_to_mcu(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // 封裝封包
    uint16_t packetLen = pack_protocol_packet(cmdId, data, dataLen);
    
    // 發送到UART
    app_uart_tx_data(packetBuffer, packetLen);
    
    // 更新協議狀態
    protocolState.curCmdId = cmdId;
    protocolState.lastTransmitTime = HAL_GetTick();
    protocolState.waitForResponse = true;
    
    PRINT("Send packet to MCU: CMD=0x%02X, Len=%d\n", cmdId, dataLen);
}

/**
 * @brief 發送錯誤回覆封包到APP
 * @param originalCmdId 原始指令ID
 * @param errorCode 錯誤代碼
 */
static void send_error_packet_to_app(uint8_t originalCmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    // 封裝錯誤回覆封包
    uint16_t packetLen = pack_protocol_packet(CMD_ERROR_ACK, data, 2);
    
    // 發送到APP
    attHandleValueNoti_t noti;
    noti.len = packetLen;
    noti.pValue = packetBuffer;
    
    // 使用tmos_start_task而不是直接調用notify函數
    tmos_memcpy(to_test_buffer, packetBuffer, packetLen);
    tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
    
    PRINT("Send error packet to APP: Orig CMD=0x%02X, Error=0x%02X\n", originalCmdId, errorCode);
}

/**
 * @brief 發送回覆封包到APP
 * @param cmdId 指令ID
 * @param data 數據指針
 * @param dataLen 數據長度
 */
static void send_response_to_app(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // 封裝回覆封包
    uint16_t packetLen = pack_protocol_packet(cmdId, data, dataLen);
    
    // 複製到全局緩衝區，以便通過任務發送
    tmos_memcpy(to_test_buffer, packetBuffer, packetLen);
    
    // 使用tmos_start_task啟動發送事件而不是直接調用notify函數
    tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
    
    PRINT("Send response to APP: CMD=0x%02X, Len=%d\n", cmdId, dataLen);
}

/**
 * @brief 處理從MCU接收到的協議封包
 * @param data 數據指針
 * @param len 數據長度
 */
static bool process_packet_from_mcu(uint8_t *data, uint16_t len)
{
    // 檢查起始和結束標記
    if (data[0] != PKT_START_MARK || data[len-1] != PKT_END_MARK) {
        PRINT("Invalid packet markers\n");
        return false;
    }
    
    // 檢查長度一致性
    uint8_t dataLen = data[2];
    if (len != dataLen + 5) { // 5 = 起始標記(1) + 指令ID(1) + 長度(1) + 校驗和(1) + 結束標記(1)
        PRINT("Invalid packet length\n");
        return false;
    }
    
    // 檢查校驗和
    uint8_t calculatedChecksum = calculate_checksum(&data[1], dataLen + 2); // +2 為指令ID和長度欄位
    if (calculatedChecksum != data[len-2]) {
        PRINT("Checksum error\n");
        return false;
    }
    
    // 提取指令ID和資料
    uint8_t cmdId = data[1];
    uint8_t* pData = (dataLen > 0) ? &data[3] : NULL;
    
    // 重置重傳計數器
    protocolState.retransmitCount = 0;
    protocolState.waitForResponse = false;
    
    // 根據指令ID轉發到APP
    send_response_to_app(cmdId, pData, dataLen);
    
    return true;
}

/**
 * @brief 處理從APP接收到的協議封包
 * @param data 數據指針
 * @param len 數據長度
 */
static bool process_packet_from_app(uint8_t *data, uint16_t len)
{
    // 檢查起始和結束標記
    if (data[0] != PKT_START_MARK || data[len-1] != PKT_END_MARK) {
        PRINT("Invalid packet markers from APP\n");
        return false;
    }
    
    // 檢查長度一致性
    uint8_t dataLen = data[2];
    if (len != dataLen + 5) { // 5 = 起始標記(1) + 指令ID(1) + 長度(1) + 校驗和(1) + 結束標記(1)
        PRINT("Invalid packet length from APP\n");
        return false;
    }
    
    // 檢查校驗和
    uint8_t calculatedChecksum = calculate_checksum(&data[1], dataLen + 2); // +2 為指令ID和長度欄位
    if (calculatedChecksum != data[len-2]) {
        PRINT("Checksum error from APP\n");
        send_error_packet_to_app(data[1], ERR_CHECKSUM);
        return false;
    }
    
    // 提取指令ID和資料
    uint8_t cmdId = data[1];
    uint8_t* pData = (dataLen > 0) ? &data[3] : NULL;
    
    // 儲存當前處理的指令ID
    protocolState.curCmdId = cmdId;
    
    // 根據指令類型進行特殊處理
    switch (cmdId) {
        case CMD_SET_CODE_EVENT:
            if (dataLen >= 3) {
                // 儲存CODE和EVENT值
                protocolState.stripCode = pData[0];
                protocolState.eventType = BUILD_UINT16(pData[1], pData[2]);
            }
            break;
        
        case CMD_BLOOD_STATUS:
            // 標記測量開始
            protocolState.measureInProgress = true;
            break;
            
        case CMD_REQ_RESULT:
            // 測量結果請求處理
            break;
            
        default:
            break;
    }
    
    // 轉發到MCU
    send_packet_to_mcu(cmdId, pData, dataLen);
    
    return true;
}

/**
 * @brief 處理協議重傳
 */
static void handle_retransmission(void)
{
    // 判斷是否在等待回覆
    if (!protocolState.waitForResponse) {
        return;
    }
    
    // 獲取當前時間
    uint32_t currentTime = HAL_GetTick();
    
    // 判斷是否超時
    if ((currentTime - protocolState.lastTransmitTime) >= RETRANSMIT_TIMEOUT) {
        // 超時重傳
        if (protocolState.retransmitCount < MAX_RETRANSMIT_COUNT) {
            // 重發上一個封包
            app_uart_tx_data(packetBuffer, 0); // packetBuffer中是上一次發送的封包
            
            // 更新重傳狀態
            protocolState.retransmitCount++;
            protocolState.lastTransmitTime = currentTime;
            
            PRINT("Retransmit packet, count=%d\n", protocolState.retransmitCount);
        } else {
            // 超過最大重傳次數，通知APP通訊錯誤
            send_error_packet_to_app(protocolState.curCmdId, ERR_COMM_ERROR);
            
            // 重置重傳狀態
            protocolState.waitForResponse = false;
            protocolState.retransmitCount = 0;
            
            PRINT("Retransmit failed, max count reached\n");
        }
    }
}

/*********************************************************************
*********************************************************************/
