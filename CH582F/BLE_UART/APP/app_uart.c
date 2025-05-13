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
#include "StripDetect.h"

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

// 函數前置聲明
static void process_uart_mcu_protocol(uint8_t *data, uint16_t len);

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
    if(R8_UART1_TFC < UART_FIFO_SIZE)
    {
        app_drv_fifo_read_to_same_addr(&app_uart_tx_fifo, (uint8_t *)&R8_UART1_THR, UART_FIFO_SIZE - R8_UART1_TFC);
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
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);

    //uart rx io
    GPIOA_SetBits(bRXD1);
    GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);

    //uart1 init
    UART1_DefInit();

    //enable interupt
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART1_IRQn);
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
 * @fn      UART1_IRQHandler
 *
 * @brief   Not every uart reception will end with a UART_II_RECV_TOUT
 *          UART_II_RECV_TOUT can only be triggered when R8_UARTx_RFC is not 0
 *          Here we cannot rely UART_II_RECV_TOUT as the end of a uart reception
 *
 * @return  NULL
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    uint16_t error;
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT:
            UART1_GetLinSTA();
            break;

        case UART_II_RECV_RDY:
        case UART_II_RECV_TOUT:
            error = app_drv_fifo_write_from_same_addr(&app_uart_rx_fifo, (uint8_t *)&R8_UART1_RBR, R8_UART1_RFC);
            if(error != APP_DRV_FIFO_RESULT_SUCCESS)
            {
                for(uint8_t i = 0; i < R8_UART1_RFC; i++)
                {
                    //fifo full,put to fifo black hole
                    for_uart_rx_black_hole = R8_UART1_RBR;
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

            // //for notify back test
            // //to ble
            // uint16_t to_write_length = p_evt->data.length;
            // app_drv_fifo_write(&app_uart_rx_fifo, (uint8_t *)p_evt->data.p_data, &to_write_length);
            // tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
            // //end of nofify back test

            //ble to uart
            app_uart_tx_data((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            
            // 處理接收到的BLE數據
            process_uart_mcu_protocol((uint8_t *)p_evt->data.p_data, p_evt->data.length);

            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      send_to_uart_mcu
 *
 * @brief   發送數據到MCU (CH32V203G8R6)
 *
 * @param   buf - 待發送的數據緩衝區
 * @param   len - 數據長度
 *
 * @return  發送結果
 */
uint8_t send_to_uart_mcu(uint8_t *buf, uint16_t len)
{
    app_drv_fifo_write(&app_uart_tx_fifo, buf, &len);
    UART1_SendString(buf, len);
    return APP_DRV_FIFO_RESULT_SUCCESS;
}

/*********************************************************************
 * @fn      process_uart_mcu_protocol
 *
 * @brief   處理來自MCU的通訊協議數據
 *
 * @param   data - 接收到的數據
 *          len - 數據長度
 *
 * @return  none
 */
static void process_uart_mcu_protocol(uint8_t *data, uint16_t len)
{
    // 最基本的通訊協議處理
    if (len < 5) return; // 包頭(1) + 指令(1) + 長度(1) + 數據(N) + 校驗(1) + 包尾(1)
    
    if (data[0] != 0xAA || data[len-1] != 0x55) return; // 判斷包頭包尾
    
    uint8_t cmd = data[1];       // 獲取指令
    uint8_t dataLen = data[2];   // 獲取數據長度
    
    // 簡單的校驗和檢查
    uint8_t checksum = 0;
    for(uint8_t i = 1; i < len-2; i++) {
        checksum += data[i];
    }
    
    if (checksum != data[len-2]) {
        PRINT("Checksum Error\n");
        return;
    }
    
    // 根據不同指令進行處理
    switch(cmd) {
        case 0xA0: // 試片類型回應指令
            if (dataLen == 1) {
                uint8_t stripType = data[3]; // 獲取試片類型
                StripDetect_SetStripType(stripType);
                PRINT("MCU set Strip Type: %d\n", stripType);
            }
            break;
            
        default:
            PRINT("Unknown command: 0x%02X\n", cmd);
            break;
    }
}

/*********************************************************************
*********************************************************************/
