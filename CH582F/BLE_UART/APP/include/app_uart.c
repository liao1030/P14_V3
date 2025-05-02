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
#include "protocol_handler.h"

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
 * @brief   初始化UART
 *
 * @return  NULL
 */
void app_uart_init()
{
    //tx fifo and tx fifo
    //The buffer length should be a power of 2
    app_drv_fifo_init(&app_uart_tx_fifo, app_uart_tx_buffer, APP_UART_TX_BUFFER_LENGTH);
    app_drv_fifo_init(&app_uart_rx_fifo, app_uart_rx_buffer, APP_UART_RX_BUFFER_LENGTH);

    // 配置UART1硬體連接
    // CH32V203G8R6使用UART2通訊
    // 配置PA2/ADC2/OP2O0 (Pin 11) 作為MCU_TX，連接到UART 1 Tx
    GPIOA_SetBits(bTXD3);
    GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);

    // 配置PA3/ADC3/OP1O0 (Pin 12) 作為MCU_RX，連接到UART 1 Rx
    GPIOA_SetBits(bRXD3);
    GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);

    // 初始化UART3，配置波特率為115200
    UART3_DefInit();
    UART3_BaudRateCfg(115200);

    // 啟用中斷
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART3_IRQn);
    
    // 初始化協議處理模塊
    protocol_init();
    
    PRINT("UART初始化完成，波特率115200\n");
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
 * @brief   藍牙UART服務回調處理
 *
 * @return  NULL
 */
void on_bleuartServiceEvt(uint16_t connection_handle, ble_uart_evt_t *p_evt)
{
    switch(p_evt->type)
    {
        case BLE_UART_EVT_TX_NOTI_DISABLED:
            PRINT("%02x:藍牙通知已禁用\n", connection_handle);
            break;
            
        case BLE_UART_EVT_TX_NOTI_ENABLED:
            PRINT("%02x:藍牙通知已啟用\n", connection_handle);
            break;
            
        case BLE_UART_EVT_BLE_DATA_RECIEVED:
            PRINT("接收藍牙數據: %d字節\n", p_evt->data.length);
            
            // 將藍牙數據轉發至協議處理模塊
            protocol_handle_ble_data((uint8_t *)p_evt->data.p_data, p_evt->data.length);
            
            break;
            
        default:
            break;
    }
}

/*********************************************************************
 * @fn      uart_to_ble_send
 *
 * @brief   處理從UART到BLE的數據發送
 *
 * @param   None
 *
 * @return  None
 */
void uart_to_ble_send(void)
{
    uint8_t data[BLE_BUFF_MAX_LEN - 4];
    uint16_t read_len = 0;
    
    // 從UART接收FIFO讀取數據
    app_drv_fifo_read(&app_uart_rx_fifo, data, &read_len);
    
    if (read_len > 0) {
        // 將數據交給協議處理模塊
        protocol_process_uart_data(data, read_len);
        
        // 記錄接收到的UART數據
        PRINT("UART->BLE: ");
        for (uint16_t i = 0; i < read_len && i < 16; i++) {
            PRINT("%02X ", data[i]);
        }
        if (read_len > 16) PRINT("...");
        PRINT("\n");
    }
}

/*********************************************************************
*********************************************************************/
