/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.1.0
 * Date               : 2025/04/28
 * Description        : CH582F主程式
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "ble_uart_protocol.h"

/* 全局定義 */
#define STRIP_DETECT_3_PIN     GPIO_Pin_11      /* PB11 試片插入檢測 */
#define T3_IN_SEL_PIN          GPIO_Pin_10      /* PB10 T3電極量測致能控制 */
#define STRIP_DETECT_5_PIN     GPIO_Pin_15      /* PA15 試片插入檢測 */
#define V2P5_ENABLE_PIN        GPIO_Pin_14      /* PA14 供電給CH32V203致能控制 */
#define V_BACK_C_PIN           GPIO_Pin_13      /* PA13 外部UART設備Rx Level Shift電源選擇控制 */
#define VBUS_GET_PIN           GPIO_Pin_12      /* PA12 量測外部設備是否供電 */

/* UART相關緩衝區 */
#define UART_RX_BUF_SIZE       128
static uint8_t uart1_rx_buf[UART_RX_BUF_SIZE];
static uint16_t uart1_rx_len = 0;

/* BLE相關參數 */
#define BLE_DEVICE_NAME_BASE   "P14-"
static uint8_t ble_mac[6];
static uint8_t ble_device_name[20];

/* 函數聲明 */
void SystemInit(void);
void GPIO_Init(void);
void GPIO_INT_Init(void);
void UART_Init(void);
void BLE_Init(void);
void BLE_ProcessEvent(void);
void Timer_Init(void);
void Process_UART_Data(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   主函數
 *
 * @return  none
 */
int main(void)
{
    /* 系統基本初始化 */
    SystemInit();
    
    /* GPIO初始化 */
    GPIO_Init();
    
    /* GPIO中斷初始化 */
    GPIO_INT_Init();
    
    /* UART初始化 */
    UART_Init();
    
    /* 協議初始化 */
    BLE_Protocol_Init();
    
    /* BLE初始化 */
    BLE_Init();
    
    /* 定時器初始化 */
    Timer_Init();
    
    /* 基本測試輸出 */
    PRINT("P14_V2.1 BLE 初始化完成\n");
    PRINT("系統時鐘: %d MHz\n", GetSysClock() / 1000000);
    PRINT("設備名稱: %s\n", ble_device_name);
    
    /* 主循環 */
    while(1)
    {
        /* 處理BLE事件 */
        BLE_ProcessEvent();
        
        /* 處理UART接收數據 */
        Process_UART_Data();
        
        /* 系統休眠 */
        __WFI();
    }
}

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   系統時鐘初始化
 *
 * @return  none
 */
void SystemInit(void)
{
    /* 解除外設複位 */
    PERIPHERAL_RESET();
    
    /* 系統時鐘初始化 */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* 系統中斷初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* 開啟時鐘 */
    GPIOA_ModeCfg(PIN_WAKEUP_GPIO_ALL, Pin_ModeIN_PD);
    LowPower_Idle();
}

/*********************************************************************
 * @fn      GPIO_Init
 *
 * @brief   GPIO初始化配置
 *
 * @return  none
 */
void GPIO_Init(void)
{
    /* 試片插入檢測引腳 - PB11 */
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    
    /* T3電極量測致能控制 - PB10 (高電平致能) */
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 試片插入檢測引腳 - PA15 */
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    
    /* 供電給CH32V203致能控制 - PA14 (高電平致能) */
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 外部UART設備Rx Level Shift電源選擇控制 - PA13 */
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);
    
    /* 量測外部設備是否供電 - PA12 */
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);
    
    /* 初始設定輸出狀態 */
    GPIOB_ResetBits(T3_IN_SEL_PIN);     /* T3電極量測禁用(低電平) */
    GPIOA_SetBits(V2P5_ENABLE_PIN);     /* 供電給CH32V203致能(高電平) */
    GPIOA_ResetBits(V_BACK_C_PIN);      /* UART Level Shift電源選擇(低電平) */
}

/*********************************************************************
 * @fn      GPIO_INT_Init
 *
 * @brief   GPIO中斷初始化配置
 *
 * @return  none
 */
void GPIO_INT_Init(void)
{
    /* 啟用試片插入檢測引腳中斷 - PB11 */
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    
    /* 啟用試片插入檢測引腳中斷 - PA15 */
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    
    /* 啟用GPIO中斷 */
    PFIC_EnableIRQ(GPIO_IRQn);
}

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   UART初始化配置
 *
 * @return  none
 */
void UART_Init(void)
{
    /* UART1初始化 - 與CH32V203通訊 */
    GPIOA_SetBits(GPIO_Pin_8);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);         /* RX (PA8) */
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);    /* TX (PA9) */
    
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    /* 清空接收緩衝區 */
    uart1_rx_len = 0;
    
    /* 啟用UART1中斷接收 */
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
}

/*********************************************************************
 * @fn      Timer_Init
 *
 * @brief   定時器初始化
 *
 * @return  none
 */
void Timer_Init(void)
{
    /* 初始化定時器3, 10ms中斷 */
    TMR3_TimerInit(GetSysClock() / 100);
    TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR3_IRQn);
}

/*********************************************************************
 * @fn      Process_UART_Data
 *
 * @brief   處理UART接收到的數據
 *
 * @return  none
 */
void Process_UART_Data(void)
{
    /* 如果有數據需要處理 */
    if (uart1_rx_len > 0) {
        /* 通過協議處理 */
        BLE_Protocol_Parse(uart1_rx_buf, uart1_rx_len);
        
        /* 清空緩衝區 */
        uart1_rx_len = 0;
    }
}

/*********************************************************************
 * @fn      BLE_Init
 *
 * @brief   BLE初始化
 *
 * @return  none
 */
void BLE_Init(void)
{
    uint8_t i;
    
    /* 獲取MAC地址 */
    GetMACAddress(ble_mac);
    
    /* 生成設備名稱 P14-XXXX (MAC地址最後3字節) */
    sprintf((char*)ble_device_name, "%s%02X%02X%02X", BLE_DEVICE_NAME_BASE, 
            ble_mac[3], ble_mac[4], ble_mac[5]);
    
    /* BLE配置初始化 */
    BLE_LibInit();
    
    /* 廣播初始化 */
    GAPRole_PeripheralInit();
    
    /* 設置設備名稱 */
    GAP_SetDeviceName((uint8_t *)ble_device_name);
    
    /* 設置廣播參數 */
    {
        uint8_t adv_data[31];
        uint8_t adv_len = 0;
        
        adv_data[adv_len++] = 0x02;     // 長度
        adv_data[adv_len++] = 0x01;     // 類型: Flags
        adv_data[adv_len++] = 0x06;     // 值: LE General Discoverable
        
        adv_data[adv_len++] = strlen((char*)ble_device_name) + 1; // 長度
        adv_data[adv_len++] = 0x09;     // 類型: Complete Local Name
        for (i = 0; i < strlen((char*)ble_device_name); i++) {
            adv_data[adv_len++] = ble_device_name[i];
        }
        
        adv_data[adv_len++] = 0x03;     // 長度
        adv_data[adv_len++] = 0x02;     // 類型: UUID List
        adv_data[adv_len++] = BLE_SERVICE_UUID & 0xFF;
        adv_data[adv_len++] = (BLE_SERVICE_UUID >> 8) & 0xFF;
        
        /* 設置廣播數據 */
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, adv_len, adv_data);
    }
    
    /* 設置廣播間隔 */
    {
        uint16_t adv_interval = 160;    // 100ms (160 * 0.625ms)
        GAPRole_SetParameter(GAPROLE_ADV_EVENT_INTERVAL, sizeof(adv_interval), &adv_interval);
    }
    
    /* 設置服務 */
    {
        /* 自定義服務 */
        uint8_t service_uuid[2] = {BLE_SERVICE_UUID & 0xFF, (BLE_SERVICE_UUID >> 8) & 0xFF};
        uint8_t rx_uuid[2] = {BLE_CHAR_RX_UUID & 0xFF, (BLE_CHAR_RX_UUID >> 8) & 0xFF};
        uint8_t tx_uuid[2] = {BLE_CHAR_TX_UUID & 0xFF, (BLE_CHAR_TX_UUID >> 8) & 0xFF};
        
        /* 注冊自定義服務 */
        BLE_RegisterService(service_uuid);
        
        /* 注冊特性 */
        BLE_RegisterCharacteristic(rx_uuid, GATT_PERMIT_WRITE);
        BLE_RegisterCharacteristic(tx_uuid, GATT_PERMIT_READ | GATT_PROP_NOTIFY);
    }
    
    /* 開始廣播 */
    GAPRole_PeripheralStartDiscovery();
    
    PRINT("BLE initialized: %s\n", ble_device_name);
}

/*********************************************************************
 * @fn      BLE_ProcessEvent
 *
 * @brief   處理BLE事件
 *
 * @return  none
 */
void BLE_ProcessEvent(void)
{
    /* 處理BLE事件 */
    BLE_LibProcessEvents();
}

/*********************************************************************
 * @fn      TMR3_IRQHandler
 *
 * @brief   TMR3中斷處理函數 (10ms)
 *
 * @return  none
 */
__INTERRUPT void TMR3_IRQHandler(void)
{
    /* 清除中斷標誌 */
    TMR3_ClearITFlag(TMR0_3_IT_CYC_END);
    
    /* 處理重傳超時等定時任務 */
    static uint16_t retry_counter = 0;
    
    retry_counter++;
    
    /* 檢測10ms任務 */
    
    /* 檢測100ms任務 */
    if (retry_counter % 10 == 0) {
        
    }
    
    /* 檢測1000ms任務 */
    if (retry_counter % 100 == 0) {
        /* 每秒閃爍LED或其他定期任務 */
    }
}

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中斷處理函數
 *
 * @return  none
 */
__INTERRUPT void UART1_IRQHandler(void)
{
    uint8_t rx_data;
    
    /* 接收中斷 */
    if (UART1_GetITFlag(RB_IER_RECV_RDY)) {
        /* 讀取接收數據 */
        rx_data = UART1_RecvByte();
        
        /* 存入緩衝區 */
        if (uart1_rx_len < UART_RX_BUF_SIZE) {
            uart1_rx_buf[uart1_rx_len++] = rx_data;
        }
        
        /* 清除中斷標誌 */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO中斷處理函數
 *
 * @return  none
 */
__INTERRUPT void GPIO_IRQHandler(void)
{
    /* 檢測試片插入 - PB11 */
    if (GPIOB_ReadITFlagBit(STRIP_DETECT_3_PIN)) {
        PRINT("Strip detected on PB11!\n");
        
        /* 清除中斷標誌 */
        GPIOB_ClearITFlagBit(STRIP_DETECT_3_PIN);
    }
    
    /* 檢測試片插入 - PA15 */
    if (GPIOA_ReadITFlagBit(STRIP_DETECT_5_PIN)) {
        PRINT("Strip detected on PA15!\n");
        
        /* 清除中斷標誌 */
        GPIOA_ClearITFlagBit(STRIP_DETECT_5_PIN);
    }
}

/*********************************************************************
 * @fn      BLE_GAPRole_EventCallBack
 *
 * @brief   BLE GAP角色事件回調
 *
 * @param   event - 事件類型
 * @param   data - 事件數據
 *
 * @return  none
 */
void BLE_GAPRole_EventCallBack(uint8_t event, uint8_t *data)
{
    switch (event) {
        case GAPROLE_CONNECTED:
            /* BLE已連接 */
            PRINT("BLE Connected\n");
            BLE_SetConnectedState(1);
            break;
            
        case GAPROLE_DISCONNECTED:
            /* BLE已斷開 */
            PRINT("BLE Disconnected\n");
            BLE_SetConnectedState(0);
            
            /* 重新開始廣播 */
            GAPRole_PeripheralStartDiscovery();
            break;
            
        default:
            break;
    }
}

/*********************************************************************
 * @fn      BLE_Characteristic_EventCallBack
 *
 * @brief   BLE特性事件回調
 *
 * @param   char_uuid - 特性UUID
 * @param   event - 事件類型
 * @param   data - 事件數據
 * @param   length - 數據長度
 *
 * @return  none
 */
void BLE_Characteristic_EventCallBack(uint16_t char_uuid, uint8_t event, uint8_t *data, uint16_t length)
{
    switch (char_uuid) {
        case BLE_CHAR_RX_UUID:
            if (event == GATT_CHAR_WRITE_EVENT) {
                /* 接收到BLE寫入數據 */
                PRINT("BLE RX Data: %d bytes\n", length);
                
                /* 解析數據 */
                BLE_Protocol_Parse(data, length);
            }
            break;
            
        default:
            break;
    }
} 